/* local: local delivery channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/slocal/RCS/local.c,v 6.0 1991/12/18 20:12:09 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/slocal/RCS/local.c,v 6.0 1991/12/18 20:12:09 jpo Rel $
 *
 * $Log: local.c,v $
 * Revision 6.0  1991/12/18  20:12:09  jpo
 * Release 6.0
 *
 */



/*********************************************************************\
*                                                                     *
* local channel - deliver to people                                   *
*                                                                     *
\*********************************************************************/


#include "head.h"
#include "util.h"
#include "prm.h"
#include "q.h"
#include "dr.h"
#include "qmgr.h"
#include <isode/logger.h>
#include <sys/stat.h>
#include <pwd.h>
#include "sys.file.h"

#define TESTVERSION

extern  char            *quedfldir;
extern  CHAN            *ch_nm2struct ();
extern  char            *mboxname;
extern  char            *delim1;
extern  char            *delim2;

static CHAN             *mychan;
static char             *this_msg;
static struct passwd    *pwd;

static int              initproc ();
static int		douser ();
static int 		deliver ();
static void             dirinit ();
static struct           type_Qmgr_DeliveryStatus *process ();
static Q_struct         Qstruct;
static int 		copy ();



/* ---------------------  Begin Routines  --------------------------------- */




main (argc, argv)
int     argc;
char    **argv;
{
	char    *p;

	p = argv[0];
	if (*p == '/') {
		p = rindex (p, '/');
		p++;
	}


	/* -- Init the channel - and find out who we are -- */
	chan_init (p);


	dirinit ();
#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0)
		debug_channel_control (argc, argv,
					initproc, process, NULLIFP);
	else
#endif
		channel_control (argc, argv, initproc, process, NULLIFP);
	exit (0);
}



static int initproc (arg)
struct type_Qmgr_Channel *arg;
{
	char    *p;

	p = qb2str (arg);

	PP_TRACE (("initproc (%s)", p));
	if ((mychan = ch_nm2struct (p)) == NULLCHAN)
		err_abrt (RP_PARM, "Channel '%s' not known", p);

	PP_NOTICE (("Starting %s (%s)",
			mychan -> ch_name, mychan -> ch_show));

	if (p)
		free (p);

	return OK;
}




static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct type_Qmgr_UserList *up;
	struct prm_vars         prm;
	Q_struct                *qp = &Qstruct;
	ADDR                    *ad_sendr = NULL,
				*ad_recip = NULL;
	int                     ad_count,
				retval;


	bzero ((char *)&prm, sizeof prm);
	bzero ((char *)qp, sizeof *qp);

	if (this_msg)
		free (this_msg);

	this_msg = qb2str (arg -> qid);

	PP_NOTICE (("Processing msg %s", this_msg));

	delivery_init (arg -> users);

	retval = rd_msg (this_msg, &prm, qp, &ad_sendr, &ad_recip, &ad_count);
	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("local/rd_msg err: %s", this_msg));
		return delivery_setall (int_Qmgr_status_messageFailure);
	}

	for (up = arg -> users; up; up = up -> next) {
		retval = douser (up -> RecipientId -> parm, ad_recip,
				 this_msg);
		if (rp_isbad (retval))
			break;
	}

	rd_end();
	prm_free (&prm);
	q_free (qp);

	return deliverystate;
}



/*
Change into pp queue space
*/

static void dirinit ()
{
	PP_TRACE (("dirinit ()"));
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, "Uanble to change directory to '%s'",
						quedfldir);
}


/*
 * process one extension-id for this message
 */

static int douser (rno, ad_recip, msgd)
int     rno;
ADDR    *ad_recip;
char    *msgd;
{
	ADDR    *ap;

	PP_TRACE (("douser (%d, ad_recip, %s)", rno, msgd));

	for (ap = ad_recip; ap ; ap = ap->ad_next) {
		if (ap-> ad_status == AD_STAT_DONE)
			continue;
		if ( rno == ap -> ad_no)
			return (deliver (ap, msgd));
	}

	PP_LOG (LLOG_EXCEPTIONS,
		("local/user not recipient of %s", this_msg));

	delivery_set (rno, int_Qmgr_status_messageFailure);
	return RP_MECH;
}


static char    *formatdir = NULLCP;

/*
Do the delivery
*/
static int deliver (ap, msgd)
ADDR    *ap;
char    *msgd;
{
	char    *username = NULLCP,
		*diry = NULLCP,
		*mbox = NULLCP,
		mailbox[MAXPATHLENGTH],
		filename[MAXPATHLENGTH];
	int     first = 0,
		retval;
	RP_Buf  rps,
		*rp = &rps;


	PP_TRACE (("local/deliver msgd=%s", msgd));

	if (ap->ad_outchan->li_chan != mychan) {
		PP_LOG (LLOG_EXCEPTIONS,
			("local/this extension-id (%d) is not for this chan",
			ap->ad_extension));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_BAD;
	}

	if (tb_getlocal (ap -> ad_r822adr, mychan, &username,
			&diry, &mbox) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("User %s not registered in chan %s table",
			ap -> ad_r822adr, mychan -> ch_name));
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER,
			 DRD_UA_UNAVAILABLE,
			 "User not registered in delivery table");
		if (!rp_isbad(wr_q2dr (&Qstruct, this_msg)))
			delivery_set (ap -> ad_no, 
				      int_Qmgr_status_negativeDR);
		return RP_USER;
	}

	PP_TRACE (("Found entry  for '%s' uid '%s' directory '%s'",
		ap -> ad_r822adr, username, diry));

	if (pwd && strcmp (pwd -> pw_name, username) == 0)
	    PP_TRACE (("Same user"));
	else {
		if (pwd)
			(void) loc_end ();

		if ((pwd = getpwnam (username)) == NULL) {
			PP_LOG (LLOG_EXCEPTIONS,
				("local/user '%s' not in passwd file",
				 username));
			set_1dr (&Qstruct, ap -> ad_no, this_msg,
				DRR_UNABLE_TO_TRANSFER,
				DRD_UA_UNAVAILABLE,
				"user not registered in passwd file");
			if (!rp_isbad(wr_q2dr (&Qstruct, this_msg))) 
				delivery_set (ap -> ad_no,
					      int_Qmgr_status_negativeDR);
			return RP_USER;
		}

		if (rp_isbad (loc_init (pwd, diry ? diry : pwd->pw_dir))) {
			PP_LOG (LLOG_EXCEPTIONS, ("loc_init error"));
			delivery_set (ap -> ad_no,
					int_Qmgr_status_mtaFailure);
			return RP_MECH;
		}

	}

	/*
	Find the correctly formatted version of the message
	*/

	if (qid2dir (this_msg, ap, TRUE, &formatdir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't locate message %s", this_msg));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_MECH;
	}
        PP_DBG (("local: mboxname : '%s'", mboxname));
        PP_DBG (("local: delim1   : '%s'", delim1));
        PP_DBG (("local: delim2   : '%s'", delim2));

	(void) sprintf (mailbox, "%s/%s",
				diry ? diry : pwd->pw_dir,
				mbox ? mbox : mboxname);

	PP_TRACE (("delivering to mailbox='%s'", mailbox));

	if (loc_open (mailbox, "a", 1, rp) != RP_OK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't open mailbox %s",
					  rp -> rp_line));
		delivery_set (ap -> ad_no, int_Qmgr_status_mtaFailure);
		return RP_MECH;
	}

	if (rp_isbad (retval = msg_rinit (formatdir))) {
		PP_LOG (LLOG_EXCEPTIONS, ("local/msg_rinit can't init %s",
			      formatdir));
		(void) loc_close (rp);
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_MECH;
	}

	PP_TRACE (("Mailbox opened"));

	if (loc_write (delim1, strlen (delim1), rp) != RP_OK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("loc_write can't write delim1 [%s]",
			 rp -> rp_line));
		delivery_set (ap -> ad_no, int_Qmgr_status_mtaFailure);
		return rp -> rp_val;
	}

	first = 0;
	while ((retval = msg_rfile (filename)) == RP_OK) {
		if (rp_isbad (retval = copy (filename)))
			break;
		if (first++ == 0)
			loc_write ("\n", 1, rp);
	}

	if (loc_write (delim2, strlen (delim2), rp) != RP_OK)
		PP_LOG (LLOG_EXCEPTIONS, ("loc_write can't write delim2 [%s]",
			      rp -> rp_line));

	loc_close (rp);

	retval = msg_rend ();
	if (retval != RP_OK) {
		delivery_set (ap -> ad_no, int_Qmgr_status_mtaFailure);
		return retval;
	}

	PP_NOTICE ((">>> local : Message delivered to %s", username));
	if (ap -> ad_usrreq == AD_USR_CONFIRM ||
	    ap -> ad_mtarreq == AD_MTA_CONFIRM ||
	    ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_NO_REASON,
			-1, NULLCP);
		if (!rp_isbad(wr_q2dr (&Qstruct, this_msg)))
			delivery_set (ap -> ad_no, 
				      int_Qmgr_status_positiveDR);
	}
	else {
		(void) wr_ad_status (ap, AD_STAT_DONE);
		delivery_set (ap -> ad_no, int_Qmgr_status_success);
	}
	return RP_OK;
}


static int copy (fname)
char    *fname;
{
	FILE    *ifp;
	char    buf[BUFSIZ];
	int     n, retval = RP_OK;
	RP_Buf rps, *rp = &rps;

	PP_TRACE (("copy (%s, fp)", fname));

	if ((ifp = fopen (fname,"r")) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS,
			("local/can't read file '%s'", fname));
		return (RP_FOPN);
	}

	while ((n = fread (buf, sizeof (char), sizeof (buf), ifp)) > 0)
		if (loc_write (buf, n, rp) != RP_OK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("loc_write can't write error [%s]",
				 rp -> rp_line));
			retval = rp -> rp_val;
			break;
		}
	(void) fclose (ifp);
	return (retval);
}
