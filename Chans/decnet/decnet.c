/* decnet.c: main decnet program */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/decnet/RCS/decnet.c,v 6.0 1991/12/18 20:06:35 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/decnet/RCS/decnet.c,v 6.0 1991/12/18 20:06:35 jpo Rel $
 *
 * $Log: decnet.c,v $
 * Revision 6.0  1991/12/18  20:06:35  jpo
 * Release 6.0
 *
 */

/*
 * Mail11 channel for PP
 */

#include "head.h"
#include "chan.h"
#include "prm.h"
#include "q.h"
#include "dr.h"
#include "qmgr.h"
#include <sys/stat.h>

/* Externs */
extern  char    *quedfldir;
extern void     rd_end(), chan_init(), err_abrt(), timer_start(), timer_end();
FILE            *msg_fp;
extern char     *rfc_de();
extern char     de_status_text[];

/* Local variables */
CHAN *mychan;                           /* Our channel structure */
char *sender;                           /* Message sender */
char *this_msg;                         /* Message id */
char *cur_host = NULLCP;                /* Current host */
char *open_host = NULLCP;
char bp_name[MAXPATHLENGTH];                 /* Body part file name */
FILE *bp;                               /* Body part file descriptor */
int strip = 0;                          /* If true then strip headers */
int cc_ok = 0;                          /* If true then may send cc line */
int mrgate = 0;                         /* If true then talking to MRGATE */
int map_space = 0;                      /* If true then map <space_char> to ' ' */
char space_char = '_';                  /* Character which will be substituted by ' ' */
int  de_fd;                             /* File descriptor for decnet */
int     open_state;                     /* defines meaning of open_host */
#define STATE_INITIAL   0
#define STATE_OPEN      1
#define STATE_BAD_HOST  2
#define STATE_TIMED_OUT 3

/* Local Procedures */
char                            *fix_header();
void                            dirinit();
int                             chaninit(),
				endproc();
struct type_Qmgr_DeliveryStatus *process();
static int data_bytes;

main(argc, argv)
int argc;
char **argv;
{

	/* First initialise the channel */

	chan_init(argv[0]);

	/* Go to correct directory */

	dirinit();

#ifdef PP_DEBUG
	if (argc > 1 && strcmp(argv[1], "debug") == 0)
		debug_channel_control(argc, argv, chaninit, process, endproc);
	else
#endif
		channel_control(argc, argv, chaninit, process, endproc);
	exit(0);
}

static int chaninit(arg)
struct type_Qmgr_Channel *arg;
{
	extern char *strstr();
	char *p = qb2str(arg);

	/* Check channel OK */
	if ((mychan = ch_nm2struct(p)) == (CHAN *)0)
		err_abrt(RP_PARM, "Channel `%s' not known", p);
	/* Do any initialising */

	rename_log(p);
	PP_NOTICE (("starting %s (%s)", mychan -> ch_name, mychan -> ch_show));

	/* Check the config options */
	strip = (strstr(mychan->ch_out_info, "strip") != NULLCP);
	mrgate = (strstr(mychan->ch_out_info, "mrgate") != NULLCP);
	map_space = (strstr(mychan->ch_out_info, "map_space") != NULLCP);

	PP_TRACE (("options: strip(%d) mrgate(%d) map_space(%d)", strip, mrgate, map_space));

	free(p);
	return (OK);
}

static int endproc()
{
	PP_TRACE(("endproc()"));
}

static struct type_Qmgr_DeliveryStatus *process(arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars prm;
	struct type_Qmgr_UserList *up;
	Q_struct Qstruct, *qp = &Qstruct;
	int retval;
	ADDR    *ap,
		*ad_sendr = NULLADDR,
		*ad_recip = NULLADDR,
		*alp,
		*ad_list = NULLADDR;
	int ad_count;

	PP_TRACE(("process()"));
	if (this_msg) free(this_msg);

	this_msg = qb2str (arg -> qid);

	PP_TRACE(("process msg %s", this_msg));

	memset((char *)&prm, 0, sizeof(prm));
	memset((char *)qp, 0, sizeof(*qp));

	(void) delivery_init(arg->users);

	retval = rd_msg(this_msg, &prm, qp, &ad_sendr, &ad_recip, &ad_count);

	if (rp_isbad(retval))
	{
		PP_LOG(LLOG_EXCEPTIONS,("rd_msg err: %s", this_msg));
		return delivery_setall(int_Qmgr_status_messageFailure);
	}

	sender = ad_sendr->ad_r822adr;
	PP_TRACE(("sender: %s", sender));

	for (ap = ad_recip; ap; ap = ap->ad_next)
	{
		for (up = arg->users; up; up = up->next)
		{
			if (up->RecipientId->parm != ap->ad_no)
				continue;
			switch (chan_acheck (ap, mychan,
					     ad_list == NULLADDR, &cur_host)) {
			    default:
			    case NOTOK:
				continue;

			    case OK:
				break;
			}
			break;
		}
		if (up == NULL)
			continue;

		if (ad_list == NULLADDR)
		{
			PP_DBG(("allocating ad_list and alp"));
			ad_list = alp = (ADDR *)calloc(1, sizeof(*alp));
		}
		else
		{
			PP_DBG(("allocating alp"));
			alp->ad_next = (ADDR *)calloc(1, sizeof(*alp));
			alp = alp->ad_next;
		}
		*alp = *ap;
		alp->ad_next = NULLADDR;
	}

	if (ad_list == NULLADDR)
	{
		PP_LOG(LLOG_EXCEPTIONS, ("No recipients in user list"));
		rd_end();
		q_free (qp);
		return deliverystate;
	}

	PP_NOTICE (("processing msg %s to %s", this_msg, cur_host));

	deliver(ad_list, qp);

	q_free (qp);
	rd_end();

	return deliverystate;
}

/* Do delivery of message */

deliver(ad_list, qp)
ADDR *ad_list;
Q_struct *qp;
{
	ADDR *ap;
	int     naddrs, good_cnt, bad_cnt;
	char    buf [LINESIZE];
	struct timeval data_time;

	PP_TRACE(("deliver()"));

	if (lexequ (cur_host, open_host) != 0)
	{
		/* Brand new host */

		if (open_host != NULLCP)
		{
			free (open_host);
			if (open_state == STATE_OPEN)
				de_nclose (OK);
		}
		open_host = strdup (cur_host);

		switch (de_nopen (cur_host, buf))
		{
			case RP_OK:
				open_state = STATE_OPEN;
				break;
			case RP_NO:
				open_state = STATE_BAD_HOST;
				break;
			default:
				open_state = STATE_TIMED_OUT;
				break;
		}
	}

	switch (open_state)
	{
		case STATE_OPEN:
			break;          /* just carry on */
		case STATE_BAD_HOST:
			PP_NOTICE ((buf));
			set_all_dr (qp, ad_list, this_msg,
				 DRR_UNABLE_TO_TRANSFER,
				 DRD_UNRECOGNISED_OR,
				 buf);
			wr_q2dr (qp, this_msg);
			(void) delivery_setall (int_Qmgr_status_negativeDR);
			return;
		case STATE_TIMED_OUT:
			PP_NOTICE (("Connection failed to '%s': %s",
				    cur_host, buf));
			(void) delivery_setallstate
				(int_Qmgr_status_mtaFailure,
				 buf);
			return;
	}

	naddrs = 0;

	if (do_sender (ad_list, sender) == NOTOK)
		return;

	for (ap = ad_list; ap; ap = ap -> ad_next) {
		switch (do_recip (ap, qp) ){
		    case OK:
			naddrs ++;
			break;

		    case NOTOK:
			break;

		    default:
			return;
		}
	}

	if (naddrs == 0) {
		(void) reset ();
		wr_q2dr (qp, this_msg);
		PP_NOTICE ((">>> Message %s transfered to no recipients",
			    this_msg, naddrs));
		delivery_setall (int_Qmgr_status_negativeDR);
		return;
	}
	/* Terminate list of addressees */
	de_mark();

	data_bytes = 0;
	timer_start (&data_time);

	/* Send To: Cc: and Subj: headers */
	if (rp_isbad(de_headers(ad_list)))
	{
		PP_NOTICE((">>> Message %s failed during header transfer",
			    this_msg));
		msg_rend();
		return;
	}

	/* Send message body */
	if (rp_isbad(de_txt()))
	{
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
			"failure during body transfer");
		PP_NOTICE((">>> Message %s failed during body transfer",
				this_msg));
		(void) reset();
		msg_rend();
		return;
	}

	/* Sit and receive the status codes */
	bad_cnt = good_cnt = 0;
	for (ap = ad_list; ap; ap = ap->ad_next)
	{
		if (ap->ad_resp)
		{
			switch (de_status())
			{
			case RP_NIO:
				delivery_setstate (ap -> ad_no, int_Qmgr_status_mtaAndMessageFailure, "channel failure during final status phase");
				bad_cnt++;
				break;
			case RP_OK:
				(void) wr_ad_status (ap, AD_STAT_DONE);
				if (ap -> ad_usrreq == AD_USR_CONFIRM ||
					ap -> ad_mtarreq == AD_MTA_CONFIRM ||
					ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM)
				{
					set_1dr (qp, ap -> ad_no, this_msg,
						DRR_NO_REASON, -1, NULLCP);
					delivery_set (ap -> ad_no,
						int_Qmgr_status_positiveDR);
				}
				else {
					delivery_set (ap -> ad_no, int_Qmgr_status_success);
					wr_stat (ap, qp, this_msg, data_bytes);
				}
				good_cnt++;
				break;
			case RP_NO:
				set_1dr (qp, ap->ad_no, this_msg,
					DRR_TRANSFER_FAILURE,
					DRD_MTA_CONGESTION,
					de_status_text);
				delivery_setstate (ap -> ad_no,
						   int_Qmgr_status_negativeDR,
						   "message transfer failed during final status phase");
				bad_cnt++;
				break;
			default:
				set_1dr (qp, ap->ad_no, this_msg,
					DRR_TRANSFER_FAILURE,
					DRD_MTA_CONGESTION,
					"final status failure (unknown reason)");
				delivery_setstate (ap -> ad_no, int_Qmgr_status_negativeDR, "internal MTA failure during final status phase");
				bad_cnt++;
				break;
			}
		}
	}
	timer_end (&data_time, data_bytes, "Data Transfered");

	wr_q2dr (qp, this_msg);
	PP_NOTICE ((">>> Message %s transferred to %d recipients, failed on %d recipients",
		    this_msg, good_cnt, bad_cnt));
	/* Unlock the message files */
	msg_rend();
}

/* Change in PP queue space */

static void dirinit()
{
	if (chdir(quedfldir) < 0)
		err_abrt(RP_LIO, "Unable to change directory to `%s'", quedfldir);
}

/* Send the `sender field' */

do_sender(ap, sndr)
ADDR *ap;
char *sndr;
{
	char *formatdir;
	char buf[LINESIZE];
	char *from = NULLCP;

	PP_TRACE(("do_sender()"));

	if (qid2dir (this_msg, ap, TRUE, &formatdir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't locate message %s",
					  this_msg));
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					"Can't find message");
		(void) reset ();
		return NOTOK;
	}

	if (rp_isbad (msg_rinit (formatdir))) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't read message %s files",
					  this_msg));
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					"Can't read message");
		reset ();
		return NOTOK;
	}

	/* Get the first file (which will be the header) */

	if (rp_isbad (msg_rfile (bp_name))) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't initialise message %s files",
					  this_msg));
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					"Can't read message file name");
		reset ();
		return NOTOK;
	}

	PP_TRACE(("message file (%s)", bp_name));
	/* open the file */
	if ((bp = fopen(bp_name, "r")) == 0)
	{
		PP_LOG (LLOG_EXCEPTIONS, ("Can't open bp %s file",
					  this_msg));
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					"Can't open bp file");
		reset ();
		return NOTOK;
	}

	/* Now extract the From: line (removing the terminating
	   newline */
	while (fgets(buf, sizeof(buf), bp) != 0)
	{
		buf[strlen(buf) - 1] = '\0';
		if (strncmp(buf, "From:", 5) == 0)
		{
			from = strdup(fix_header(buf));
			break;
		}
	}

	/* Check if we are sending to a mail router. If so then we have
	to make the sender address look as VMSish as possible */
	if (mrgate)
	{
		return (de_send((from) ? rfc_de(from) : rfc_de(sndr)));
	}

	/* Check and see if the sender needs quotes to protect it from the
	depredations of VMS */

	if (strpbrk((from) ? from : sndr, "@%:,") != 0)
		sprintf(buf, "\"%s\"", (from) ? from : sndr);
	else
		strcpy(buf, (from) ? from: sndr);

	return (de_send(buf));
}

/* This routine passes over a single recipient address */

do_recip (ap, qp)
ADDR    *ap;
Q_struct *qp;
{
	char	errbuf[BUFSIZ];

	PP_NOTICE (("Recipient %s", ap -> ad_r822adr));

	switch (de_wto (ap ->ad_r822adr))
	{
	case RP_AOK:
		ap -> ad_resp = 1;
		return OK;

	case RP_NIO:
		(void) delivery_setstate (ap -> ad_no,
			int_Qmgr_status_messageFailure,
				"network error");
		PP_LOG (LLOG_EXCEPTIONS, ("Temporary failure: %s",
					"network error"));
		ap -> ad_resp = 0;
		return NOTOK;

	case RP_USER:
		(void) delivery_set (ap -> ad_no,
			int_Qmgr_status_negativeDR);
		(void) sprintf (errbuf,
			"MTA '%s' rejects address %s",
			cur_host, ap->ad_r822adr);
		PP_LOG (LLOG_EXCEPTIONS, ("User error: %s", errbuf));
		set_1dr (qp, ap -> ad_no, this_msg,
			DRR_UNABLE_TO_TRANSFER,
			DRD_UNRECOGNISED_OR,
			errbuf);
		ap -> ad_resp = 0;
		return NOTOK;

	default:
		PP_LOG (LLOG_EXCEPTIONS, ("Message failure: %s",
					"unknown error"));
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
				"unknown error");
		(void) reset ();
		return DONE;
	}
}

/* This routine gets and sends the header lines */

de_headers(ap)
ADDR *ap;
{
	char line[BUFSIZ];
	char    *to = NULLCP,
		*subject = NULLCP,
		*cc = NULLCP;

	PP_TRACE(("de_headers()"));

	/* Rewind the header file (opened by de_from) */
	rewind(bp);

	/* Now extract the To:, Cc: and Subject lines (removing the terminating
	   newline */
	while (fgets(line, sizeof(line), bp) != 0)
	{
		line[strlen(line) - 1] = '\0';
		if (strncmp(line, "To:", 3) == 0)
		{
			to = strdup(line);
		}
		else if (strncmp(line, "Subject:", 8) == 0)
		{
			subject = strdup(line);
		}
		else if (strncmp(line, "Cc:", 3) == 0)
		{
			cc = strdup(line);
		}
	}

	/* Send the various fields */
	if (rp_isbad(de_send((to) ? fix_header(to) : "(Unknown)"))
		|| (cc_ok && rp_isbad(de_send((cc) ? fix_header(cc) : "")))
		|| rp_isbad(de_send((subject) ? fix_header(subject) : "(None)")))
	{
		(void) delivery_setstate (ap -> ad_no,
			int_Qmgr_status_messageFailure,
				"network error");
		PP_LOG (LLOG_EXCEPTIONS, ("Temporary failure: %s",
					"network error"));
		return NOTOK;
	}

	/* Clean up */
	if (to) free(to);
	if (subject) free(subject);
	if (cc) free(cc);

	return OK;
}

/* This routine gets and sends the message text */

de_txt()
{
	char line[BUFSIZ];
	int n;

	PP_TRACE(("de_txt()"));
	/* If we have set header stripping then close the header */
	if (strip)
	{
		fclose(bp);
		/* If there is no more body parts, just wrap up */
		if (msg_rfile(bp_name) == RP_DONE)
		{
			de_mark();
			return OK;
		}
		/* other wise open the next body part */
		if ((bp = fopen(bp_name, "r")) == 0)
		{
			PP_LOG (LLOG_EXCEPTIONS, ("Can't open bp %s file",
						this_msg));
			(void) delivery_setallstate (int_Qmgr_status_messageFailure,
						"Can't open bp file");
			reset ();
			return NOTOK;
		}
	}
	else
	{
		rewind(bp);
	}

	/* Now iterate through the body parts */
	while (1)
	{
		/* Send a body part */
		while (fgets(line, sizeof(line), bp) != 0)
		{
			line[(n = strlen(line)) - 1] = '\0';
			n--;
			data_bytes += n;
			if (rp_isbad(de_send(line)))
			{
				return NOTOK;
			}
		}
		/* Send a blank line to seperate the body parts */
		de_send("");

		/* Close it and get the next one */
		fclose(bp);

		/* If there is no more body parts, just wrap up */
		if (msg_rfile(bp_name) == RP_DONE)
			break;

		/* other wise open the next body part */
		if ((bp = fopen(bp_name, "r")) == 0)
		{
			PP_LOG (LLOG_EXCEPTIONS, ("Can't open bp %s file",
						this_msg));
			(void) delivery_setallstate (int_Qmgr_status_messageFailure,
						"Can't open bp file");
			reset ();
			return NOTOK;
		}
	}
	de_mark();
	return OK;
}

/* This is the equivalent of the `reset' function. However, since mail11
   doesn't have the ability to reset, we have to close down the connection
 */

reset()
{
	PP_TRACE(("reset()"));

	if (open_host != NULLCP)
	{
		free (open_host);
		if (open_state == STATE_OPEN)
			de_nclose (OK);
		open_host = NULLCP;
	}
}

/* This trivial little function takes a header line and returns a pointer
   to the first non space character after the first `:'
 */

char *fix_header(h)
char *h;
{
	char *s;
	extern char *strchr();

	if ((s = strchr(h, ':')) != NULL)
	{
		while (*++s == ' ');
		return s;
	}
	return h;

}
