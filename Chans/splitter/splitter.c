/* splitter.c: resubmit messages so as to have indivual processes for recips */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/splitter/RCS/splitter.c,v 6.0 1991/12/18 20:12:36 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/splitter/RCS/splitter.c,v 6.0 1991/12/18 20:12:36 jpo Rel $
 *
 * $Log: splitter.c,v $
 * Revision 6.0  1991/12/18  20:12:36  jpo
 * Release 6.0
 *
 */


#include "util.h"
#include "qmgr.h"
#include "q.h"
#include "prm.h"
#include "retcode.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "sys.file.h"

static void dirinit();
static int initialise();
static int endfunc();
static struct type_Qmgr_DeliveryStatus *process();
static ADDR *getnthrecip ();
static int submit_error ();
static void set_success();
static int processAddr();

CHAN	*mychan;
char	*this_msg = NULLCP, *this_chan = NULLCP;
int	first_successDR, first_failureDR, start_submit;
int	linked = TRUE;

main (argc, argv)
int	argc;
char	**argv;
{
	sys_init (argv[0]);
	dirinit();

#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0)
		debug_channel_control (argc,argv,initialise,process,endfunc);
	else
#endif
		channel_control (argc,argv,initialise,process,endfunc);
}

/*  */
/* move to correct place in file system */

extern char	*quedfldir;

static void dirinit ()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO,
			  "Unable to change directory to '%s'",
			  quedfldir);
}

/*  */
/* channel initialisation routine */

static void parse_ch_info();

static int initialise (arg)
struct type_Qmgr_Channel	*arg;
{
	char	*name;
	name = qb2str (arg);

	if ((mychan = ch_nm2struct (name)) == NULLCHAN) {
		PP_OPER (NULLCP, ("Channel '%s' not known", name));
		if (name != NULLCP) free(name);
		return NOTOK;
	}

	if (name != NULLCP) free(name);
	if (mychan->ch_out_info != NULLCP)
		parse_ch_info (mychan->ch_out_info);

	start_submit = TRUE;
	return OK;
}

/* parse the info string and set the global variables */
static void parse_ch_info(info)
char	*info;
{
	char	*info_copy, *margv[20];
	int	margc, ix;

	if (info == NULLCP) return;

	info_copy = strdup(info);

	if ((margc = sstr2arg (info_copy, 20, margv, ",")) > 0) {
		for (ix = 0; ix < margc; ix++) {
			/* check each entry in info string */
			if (lexequ(margv[ix], "notlinked") == 0)
				linked = FALSE;
			else if (lexequ(margv[ix], "linked") == 0)
				linked = TRUE;
			else 
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown ch_info flag '%s'",
					margv[ix]));
		}
	}
	free(info_copy);
}

/*  */
/* channel termination routine */

static int endfunc (arg)
struct type_Qmgr_Channel *arg;
{
	if (start_submit == FALSE)
		io_end (OK);
	start_submit = TRUE;
}

/*  */
/* channel work routine */

static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
	if (this_msg) free(this_msg);
	this_msg = qb2str (msg->qid);
	
	if (this_chan) free(this_chan);
	this_chan = qb2str (msg->channel);
	
	if (mychan == NULLCHAN
	    || strcmp (this_chan, mychan->ch_name) != 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("channel error: '%s'",
			 this_chan));
		return FALSE;
	}

	return TRUE;
}

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars			prm;
	Q_struct			que;
	ADDR				*sender = NULLADDR;
	ADDR				*recips = NULLADDR;
	ADDR				*adr;
	int				rcount, retval;
	struct type_Qmgr_UserList	*ix;
	RP_Buf				*reply;

	bzero ((char *)&prm, sizeof (prm));
	bzero ((char *)&que, sizeof (que));

	delivery_init (arg->users);
	delivery_setall (int_Qmgr_status_messageFailure);
	first_failureDR = first_successDR = TRUE;

	if (security_check (arg) != TRUE)
		return deliverystate;

	PP_LOG (LLOG_NOTICE,
		("processing msg '%s' through '%s'",
		 this_msg, this_chan));

	/* read in message */
	if (rp_isbad (rd_msg (this_msg, &prm, &que,
			      &sender, &recips, &rcount))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("rd_msg error: '%s'", this_msg));
		rd_end();
		return delivery_setallstate(int_Qmgr_status_messageFailure,
					    "Can't read message");
	}

	/* process recipients individually */
	for (ix = arg->users; ix; ix = ix->next) {
		if ((adr = getnthrecip (&que, ix->RecipientId->parm)) == NULLADDR) {
			PP_LOG (LLOG_EXCEPTIONS,
				("failed to find recipient %d of msg '%s'",
				 ix->RecipientId->parm, this_msg));
			delivery_setstate (ix->RecipientId->parm,
					   int_Qmgr_status_messageFailure,
					   "Unable to find specified recipient");
			continue;
		}

		if (start_submit == TRUE && rp_isbad (io_init (&reply))) {
			submit_error (adr, "io_init", reply);
			rd_end();
			return delivery_setallstate (int_Qmgr_status_messageFailure,
						     "Unable to start submit");
		} else
			start_submit = FALSE;
		
		switch (chan_acheck (adr, mychan, 1, (char **)NULL)) {
		    default:
		    case NOTOK:
			break;
		    case OK:
			processAddr (this_msg, &prm, &que, adr);
			break;
		}
	}

	/* write results of processing */

	if (rp_isbad (retval = wr_q2dr (&que, this_msg))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("%s wr_q2dr failure '%d'",
			 mychan->ch_name, retval));
		(void) delivery_resetDRs (int_Qmgr_status_messageFailure);
	}

	rd_end ();
	q_free(&que);
	prm_free (&prm);
	
	return deliverystate;
}

/*  */
/* do processing for specified recip */

static int processAddr (msg, prm, qp, recip)
char		*msg;
struct prm_vars	*prm;
Q_struct	*qp;
ADDR		*recip;
{
	ADDR	*sender = adr_new((qp->Oaddress->ad_type == AD_X400_TYPE) ?
				  qp->Oaddress->ad_r400adr :
				  qp->Oaddress->ad_r822adr,
				  qp->Oaddress->ad_type,
				  0);

	ADDR	*recipient = adr_new((recip->ad_type == AD_X400_TYPE) ?
				  recip->ad_r400adr :
				  recip->ad_r822adr,
				  recip->ad_type,
				  1);
	Q_struct	qs;
	struct timeval data_time;
	struct stat st;
	RP_Buf	reply;
	char	*msgdir = NULLCP,
		file[MAXPATHLENGTH],
		buf[BUFSIZ],
		*strippedname;
	int	dirlen,
		size,
		fd_in, n;

	if (qid2dir (msg, recip, TRUE, &msgdir) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("msg dir not found for recip %d of msg '%s'",
			 recip->ad_no, msg));
		delivery_setstate (recip->ad_no,
				   int_Qmgr_status_messageFailure,
				   "source directory not found");
		return 0;
	}
	
	q_init (&qs);
	q_almost_dup (&qs, qp);
	qs.encodedinfo.eit_types = list_bpt_dup(qp->encodedinfo.eit_types);
	
	qs.inbound = list_rchan_new (qp->inbound->li_mta,
				     qp->inbound->li_chan->ch_name);
	prm->prm_opts = prm->prm_opts | PRM_NOTRACE | PRM_ACCEPTALL;

	/* now resubmit */
	timer_start(&data_time);

	if (rp_isbad (io_wprm (prm, &reply)))
		return submit_error (recip, "io_wprm", &reply);

	if (rp_isbad (io_wrq (&qs, &reply)))
		return submit_error (recip, "io_wrq", &reply);

	if (rp_isbad (io_wadr (sender, AD_ORIGINATOR, &reply))) {
		(void) sprintf (buf, "io_wadr(%s)", sender->ad_value);
		return submit_error(recip, buf, &reply);
	}

	if (rp_isbad (io_wadr (recipient, AD_RECIPIENT, &reply))) {
		(void) sprintf (buf, "io_wadr(%s)", recipient->ad_value);
		return submit_error(recip, buf, &reply);
	}

	if (rp_isbad (io_adend (&reply)))
		return submit_error (recip, "io_adend", &reply);

	/* submit body of message */

	if (rp_isbad (io_tinit (&reply)))
		return submit_error (recip, "io_tinit", &reply);
	dirlen = strlen (msgdir) +1;

	msg_rinit (msgdir);

	size = 0;
	while (msg_rfile (file) != RP_DONE) {

		/* --- transmit file --- */
		strippedname = file + dirlen;
		if (stat (file, &st) != NOTOK)
			size += st.st_size;
		if (linked == TRUE) {
			(void) sprintf(buf, "%s %s",strippedname, file);
			if (rp_isbad (io_tpart (buf, TRUE, &reply))) 
				return submit_error (recip,"io_tpart",&reply);
		} else {
			if (rp_isbad (io_tpart (strippedname, FALSE, &reply))) 
				return submit_error (recip,"io_tpart",&reply);

			if ((fd_in = open (file, O_RDONLY)) == -1) {
				(void) strcpy (reply.rp_line,file);
				return submit_error (recip,"open",&reply);
			}
			while ((n = read (fd_in, buf, BUFSIZ)) > 0) {
				if (rp_isbad (io_tdata (buf, n))) {
					(void) strcpy (reply.rp_line,"???");
					return submit_error (recip,"io_tdata",&reply);
				}
			}

			close (fd_in);
			if (rp_isbad (io_tdend (&reply)))
				return submit_error (recip,"io_tdend", &reply);
		}
	}
	msg_rend();
	if (rp_isbad (io_tend (&reply)))
		return submit_error (recip,"io_tend", &reply);
	set_success (recip,qp, size);

	q_free (&qs);

	timer_end (&data_time, size, "Data submitted");

	return 0;
}

/*  */

static void set_success (recip, que, size)
ADDR		*recip;
Q_struct	*que;
int		size;
{
	(void) wr_ad_status (recip, AD_STAT_DONE);
	(void) wr_stat (recip, que, this_msg, size);
	delivery_set (recip->ad_no, int_Qmgr_status_success);
}

static ADDR *getnthrecip(que, num)
Q_struct	*que;
int		num;
{
	ADDR *ix = que->Raddress;

	if (num == 0)
		return que->Oaddress;
	while ((ix != NULL) && (ix->ad_no != num))
		ix = ix->ad_next;
	return ix;
}

static int submit_error (recip, proc, reply)
ADDR	*recip;
char	*proc;
RP_Buf	*reply;
{
	char	buf[BUFSIZ];
	PP_LOG (LLOG_EXCEPTIONS,
	       ("Chans/list %s failure [%s]", proc, reply->rp_line));

	if (recip != NULLADDR) {
		(void) sprintf (buf,
				"'%s' failure for '%s' [%s]",
				proc,
				this_msg,
				reply -> rp_line);
		PP_OPER(NULLCP,("%s", buf));
		delivery_setstate (recip->ad_no, 
				   int_Qmgr_status_messageFailure,
				   buf);
	}

	start_submit = TRUE;
	io_end (NOTOK);

	return OK;
}
