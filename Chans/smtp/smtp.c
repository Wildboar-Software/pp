/* smtp: as invoked by qmgr to deliver smtp stuff*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/smtp/RCS/smtp.c,v 6.0 1991/12/18 20:12:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/smtp/RCS/smtp.c,v 6.0 1991/12/18 20:12:19 jpo Rel $
 *
 * $Log: smtp.c,v $
 * Revision 6.0  1991/12/18  20:12:19  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "chan.h"
#include "prm.h"
#include "q.h"
#include "dr.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include <pwd.h>
#include "sys.file.h"
#include <signal.h>
#include <isode/internet.h>
#ifdef SYS5
#include <sys/time.h>
#endif

extern  char    *quedfldir;
extern void	rd_end(), chan_init(), err_abrt(), timer_start(), timer_end();
FILE            *msg_fp;
CHAN            *mychan;
char            *this_msg;
int		smtpport = 0;

extern struct sm_rstruct {              /* save last reply obtained */
	int     sm_rval;                /* rp.h value for reply */
	int     sm_rlen;                /* current lengh of reply string */
	char    sm_rgot;                /* true, if have a reply */
	char    sm_rstr[LINESIZE];      /* human-oriented reply text */
} sm_rp;
#define smtp_error(def) (sm_rp.sm_rgot ? sm_rp.sm_rstr : (def))

extern int data_bytes;
extern time_t time();


#define SM_RTIME        15              /* Time allowed for a RSET command */
#define SM_TTIME        180             /* Time allowed for a block of text */


static struct type_Qmgr_DeliveryStatus *process();
static void dirinit();
static int chaninit();
static int endproc ();
static int dotext ();
static int copy ();
void deliver();

/* ---------------------  Begin  Routines  -------------------------------- */




main (argc, argv)
int     argc;
char    **argv;
{
	char    *p;
	int	opt;
	extern char *optarg;

	if ((p = rindex (argv[0], '/')) != NULLCP)
		p ++;
	if (p == NULLCP || *p == NULL)
		p = argv[0];

	chan_init (p);   /* init the channel - and find out who we are */

	while ((opt = getopt (argc, argv, "p:")) != EOF) {
		switch (opt) {
		    case 'p':
			smtpport = htons (atoi (optarg));
			break;

		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Bad argument -%c", opt));
			break;
		}
	}


	dirinit();              /* get to the right directory */
	(void) signal (SIGPIPE, SIG_IGN);
#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0)
		debug_channel_control (argc, argv, chaninit, process, endproc);
	else
#endif
		channel_control (argc, argv, chaninit, process, endproc);
	exit (0);
}



static int chaninit (arg)
struct type_Qmgr_Channel *arg;
{
	char    *p = qb2str (arg);

	if ((mychan = ch_nm2struct (p)) == (CHAN *)0)
		err_abrt (RP_PARM, "Channel '%s' not known", p);

	rename_log(p); 
	sm_init (mychan);
	PP_NOTICE (("starting %s (%s)", mychan -> ch_name, mychan -> ch_show));
	free (p);
	return OK;
}

char    *cur_host;
char    *open_host;
int	open_state;		/* defines meaning of open_host */
#define STATE_INITIAL 	0
#define STATE_OPEN    	1
#define STATE_BAD_HOST	2
#define STATE_TIMED_OUT	3
char    *sender;
char    *formatdir;

static int endproc ()
{
	if (cur_host)
		sm_nclose (OK);
}

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars prm;
	struct type_Qmgr_UserList *up;
	Q_struct        Qstruct, *qp = &Qstruct;
	int     retval;
	ADDR    *ap,
		*ad_sendr = NULLADDR,
		*ad_recip = NULLADDR,
		*alp = NULLADDR,
		*ad_list = NULLADDR;
	int     ad_count;

	if (this_msg) free (this_msg);

	this_msg = qb2str (arg -> qid);

	bzero ((char *)&prm, sizeof prm);
	bzero ((char *)qp, sizeof *qp);

	(void) delivery_init (arg -> users);

	retval = rd_msg (this_msg, &prm, qp, &ad_sendr, &ad_recip, &ad_count);

	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("rd_msg err: %s", this_msg));
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Can't read message");
	}

	sender = ad_sendr -> ad_r822adr;

	for (ap = ad_recip; ap; ap = ap -> ad_next) {
		for (up = arg ->users; up; up = up -> next) {
			if (up -> RecipientId -> parm != ap -> ad_no)
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
			ad_list = alp = (ADDR *) calloc (1, sizeof *alp);
		else {
			alp -> ad_next = (ADDR *) calloc (1, sizeof *alp);
			alp = alp -> ad_next;
		}
		*alp = *ap;
		alp -> ad_next = NULLADDR;
	}

	if (ad_list == NULLADDR) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipients in user list"));
		rd_end ();
		q_free (qp);
		return deliverystate;
	}

	PP_NOTICE (("processing msg %s to %s", this_msg, cur_host));

	deliver (ad_list, qp);

	q_free (qp);
	rd_end();

	return deliverystate;
}

static void dirinit()       /* Change into pp queue space */
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, "Unable to change directory to '%s'",
						quedfldir);
}

void deliver (ad_list, qp)
ADDR    *ad_list;
Q_struct *qp;
{
	ADDR    *ap;
	int     naddrs;
	char	buf [LINESIZE];

	PP_TRACE (("deliver ()"));


	if (lexequ (cur_host, open_host) != 0) { 
					/* Brand new host */

		if (open_host != NULLCP) {
			free (open_host);	
			if (open_state == STATE_OPEN) 
				sm_nclose (OK);
		}
		open_host = strdup (cur_host);

		switch (sm_nopen (cur_host, buf))
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
			break;		/* just carry on */
		case STATE_BAD_HOST:
			PP_NOTICE ((buf));
			set_all_dr (qp, ad_list, this_msg,
				 DRR_UNABLE_TO_TRANSFER,
				 DRD_UNRECOGNISED_OR,
				 buf);
			if (rp_isgood(wr_q2dr (qp, this_msg)))
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

	if (do_sender (sender) == NOTOK) {
		if (rp_isbad(wr_q2dr (qp, this_msg)))
			delivery_resetDRs(int_Qmgr_status_messageFailure);
		return;
	}

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
		if (rp_isbad(wr_q2dr (qp, this_msg)))
			delivery_resetDRs (int_Qmgr_status_messageFailure);
		PP_NOTICE ((">>> Message %s transfered to no recipients",
			    this_msg, naddrs));
		return;
	}

	if (dotext (qp, ad_list, naddrs) == NOTOK) {
		if (rp_isbad(wr_q2dr (qp, this_msg)))
			delivery_resetDRs(int_Qmgr_status_messageFailure);
		return;
	}

	for (ap = ad_list; ap; ap = ap -> ad_next) {
		if (ap -> ad_resp) {
			if (ap -> ad_usrreq == AD_USR_CONFIRM ||
			    ap -> ad_mtarreq == AD_MTA_CONFIRM ||
			    ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
				set_1dr (qp, ap -> ad_no, this_msg,
					 DRR_NO_REASON, -1, NULLCP);
				delivery_set (ap -> ad_no,
					      int_Qmgr_status_positiveDR);
			}
			else {
				(void) wr_ad_status (ap, AD_STAT_DONE);
				(void) wr_stat (ap, qp, this_msg, data_bytes);
				delivery_set (ap -> ad_no,
					      int_Qmgr_status_success);
			}
		}
	}
	if (rp_isbad(wr_q2dr (qp, this_msg)))
		delivery_resetDRs(int_Qmgr_status_messageFailure);
	PP_NOTICE ((">>> Message %s transfered to %d recipients",
		    this_msg, naddrs));
}       

do_recip (ap, qp)
ADDR    *ap;
Q_struct *qp;
{
	int     retval;
	char	errbuf[BUFSIZ];

	PP_NOTICE (("Recipient %s", ap -> ad_r822adr));

	if (rp_isbad (retval = sm_wto (ap ->ad_r822adr))) {
		switch (retval) {
		    case RP_DHST:
			PP_LOG (LLOG_EXCEPTIONS, ("Host died"));
			(void) delivery_setallstate (int_Qmgr_status_mtaAndMessageFailure,
						"Host died");
			return DONE;

		    case RP_AOK:
			ap -> ad_resp = 1;
			return OK;

		    case RP_AGN:
			(void) delivery_setstate (ap -> ad_no,
				      int_Qmgr_status_messageFailure,
					     smtp_error ("temporary failure"));
			PP_LOG (LLOG_EXCEPTIONS, ("Temporary failure: %s",
						  smtp_error ("cause not known")));
			ap -> ad_resp = 0;
			return NOTOK;

		    case RP_USER:
			(void) delivery_set (ap -> ad_no,
				      int_Qmgr_status_negativeDR);
			(void) sprintf (errbuf,
					"MTA '%s' gives error message %s",
					cur_host, smtp_error ("problem with user"));
			PP_LOG (LLOG_EXCEPTIONS, ("User error: %s", errbuf));
			set_1dr (qp, ap -> ad_no, this_msg,
				 DRR_UNABLE_TO_TRANSFER,
				 DRD_UNRECOGNISED_OR, 
				 errbuf);
			ap -> ad_resp = 0;
			return NOTOK;

		    case RP_PARM:
			ap -> ad_resp = 0;
			PP_LOG (LLOG_EXCEPTIONS, ("Parameter problem: %s",
						  smtp_error("bad paramter")));
			(void) delivery_setstate (ap -> ad_no,
				      int_Qmgr_status_messageFailure,
					     smtp_error ("bad parameter"));
			return NOTOK;

		    case RP_RPLY:
		    default:
			PP_LOG (LLOG_EXCEPTIONS, ("Message failure: %s",
						  smtp_error ("bad reply")));
			(void) delivery_setallstate (int_Qmgr_status_messageFailure,
						smtp_error ("bad reply"));
			(void) reset ();
			return DONE;
		}
	}
	return OK;
}

do_sender (sndr)
char    *sndr;
{
	int     retval;

	PP_NOTICE (("sender %s", sndr));

	if (rp_isbad (retval = sm_wfrom (sndr))) {
		switch (retval) {
		    case RP_DHST:
			PP_LOG (LLOG_EXCEPTIONS, ("Host connection died"));
			(void) delivery_setallstate (int_Qmgr_status_mtaAndMessageFailure,
						"host connection failed");
			sm_nclose (NOTOK);
			return NOTOK;

		    case RP_PARM:
			PP_LOG (LLOG_EXCEPTIONS, ("Parameter problem: %s",
						  smtp_error("bad param")));
			(void) delivery_setallstate (int_Qmgr_status_mtaAndMessageFailure,
						smtp_error ("bad param"));
			sm_nclose (NOTOK);
			return NOTOK;

		    case RP_AGN:
			(void) delivery_setallstate (int_Qmgr_status_messageFailure,
						smtp_error ("temp error"));
			PP_LOG (LLOG_EXCEPTIONS, ("Temporary error: %s",
						  smtp_error ("temp error")));
			(void) reset ();
			return NOTOK;

		    case RP_OK:
			break;

		    default:
			PP_LOG (LLOG_EXCEPTIONS, ("Protocol error: %s",
						  smtp_error("")));
			(void) delivery_setallstate (int_Qmgr_status_mtaAndMessageFailure,
						smtp_error ("protocol problem"));
			sm_nclose (NOTOK);
			return NOTOK;
		}
	}
	return OK;
}


static int dotext (qp, ap, naddrs)
Q_struct *qp;
ADDR    *ap;
int     naddrs;
{
	extern  FILE    *sm_wfp;
	int     retval;
	char    filename[MAXPATHLENGTH];
	struct timeval data_time;
	int	first = 1;
	char	errbuf[BUFSIZ];

	if (qid2dir (this_msg, ap, TRUE, &formatdir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't locate message %s",
					  this_msg));
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					"Cant find message");
		(void) reset ();
		return NOTOK;
	}


	if (rp_isbad (retval = msg_rinit (formatdir))) {
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					"Can't read message");
		reset ();
		return NOTOK;
	}

	if (rp_isbad (retval = sm_wrdata ())) {
		switch (retval) {
		    case RP_DHST:
			PP_LOG (LLOG_EXCEPTIONS, ("Host connection died"));
			(void) delivery_setallstate (int_Qmgr_status_mtaAndMessageFailure,
						"Connection died");

			break;

		    case RP_NDEL:
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_negativeDR);
			(void) sprintf (errbuf,
					"DATA transfer to host failed permanently: %s",
					smtp_error ("reason unknown"));
			PP_LOG (LLOG_EXCEPTIONS, ("DATA error: %s", errbuf));
			set_1dr (qp, ap -> ad_no, this_msg,
				 DRR_UNABLE_TO_TRANSFER,
				 DRD_UNRECOGNISED_OR, 
				 errbuf);
			break;

		    default:


			(void) delivery_setallstate (int_Qmgr_status_messageFailure,
						     smtp_error ("dead host?"));

			break;
		}
		sm_nclose (NOTOK);
		return NOTOK;
	}

	data_bytes = 0;
	timer_start (&data_time);

	while ((retval = msg_rfile (filename)) == RP_OK) {
		if (rp_isbad (copy (filename))) {
			(void) delivery_setallstate(int_Qmgr_status_mtaAndMessageFailure,
					       "Can't transfer data");
			sm_nclose (NOTOK);
			return NOTOK;
		}
		if (first) {
			sm_wstm ("\n", 1);
			first = 0;
		}
	}
	if (rp_isbad (retval)) {
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Message transfer failed");
		sm_nclose (NOTOK);
		return NOTOK;
	}

	if (rp_isbad (sm_wstm (NULLCP, 0))) {
		(void) delivery_setallstate (int_Qmgr_status_messageFailure,
					"Bad temination of data");
		sm_nclose (NOTOK);
		return NOTOK;
	}

	if (rp_isbad (retval = sm_wrdot (naddrs))) {
		switch (retval) {
		    case RP_DHST:
			PP_LOG (LLOG_EXCEPTIONS, ("Host connection died"));
			(void) delivery_setallstate (int_Qmgr_status_mtaAndMessageFailure,
						"Connection died");

			break;

		    case RP_NDEL:
			(void) delivery_setall (int_Qmgr_status_negativeDR);
			(void) sprintf (errbuf,
					"DATA transfer to host failed permanently");
			PP_LOG (LLOG_EXCEPTIONS, ("DATA error: %s", errbuf));
			set_1dr (qp, ap -> ad_no, this_msg,
				 DRR_UNABLE_TO_TRANSFER,
				 DRD_UNRECOGNISED_OR, 
				 errbuf);
			break;

		    default:
			PP_LOG (LLOG_EXCEPTIONS, ("Temporary failure: %s",
						  smtp_error ("bad data")));
			(void) delivery_setallstate (int_Qmgr_status_messageFailure,
						smtp_error ("bad data"));
			break;
		}
		return NOTOK;
	}
	timer_end (&data_time, data_bytes, "Data Transfered");
	msg_rend();

	return OK;
}

static int copy (fname)
char    *fname;
{
	FILE    *ifp;
	char    buf[BUFSIZ];
	int     n, retval = RP_OK;

	if ((ifp = fopen (fname,"r")) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't read file '%s'", fname));
		return RP_FOPN;
	}

	while ((n = fread (buf, sizeof (char), sizeof (buf), ifp)) > 0)
		if (rp_isbad (retval = sm_wstm (buf, n)))
			break;
	(void) fclose (ifp);
	return retval;
}

reset ()
{
	int     retval;

	if (rp_isbad (retval = sm_cmd ("RSET", SM_RTIME))) {
		switch (retval) {
		    default:
			(void) delivery_setallstate (int_Qmgr_status_mtaFailure,
						smtp_error ("reset failed"));
			sm_nclose (NOTOK);
			return NOTOK;

		    case RP_OK:
			break;
		}
	}
	return OK;
}
