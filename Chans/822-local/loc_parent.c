/* loc_parent.c: local delivery channel - parent process */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/822-local/RCS/loc_parent.c,v 6.0 1991/12/18 20:04:31 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/822-local/RCS/loc_parent.c,v 6.0 1991/12/18 20:04:31 jpo Rel $
 *
 * $Log: loc_parent.c,v $
 * Revision 6.0  1991/12/18  20:04:31  jpo
 * Release 6.0
 *
 */



/* 
 Works as parent & child. New child for each message. Child does
 actual delivery of message, parent interacts with PP.
*/

#include "util.h"
#include "retcode.h"
#include "prm.h"
#include "q.h"
#include "dr.h"
#include "qmgr.h"
#include <isode/logger.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "loc_user.h"

#ifndef USRPATH
#define USRPATH "."
#endif

extern  char            *quedfldir;
extern  CHAN            *ch_nm2struct ();
extern  char            *delim1;
extern  char            *delim2;
extern	char		*hdr_822_bp;
extern	char		*ad_getlocal ();
extern void		chan_init(), err_abrt(), rd_end();
CHAN             *mychan;
static char             *this_msg;

static int              initproc ();
static int		douser ();
static int 		deliver ();
static void             dirinit ();
static struct           type_Qmgr_DeliveryStatus *process ();
Q_struct         Qstruct;

static char	env_user[LINESIZE];
static char	env_shell[LINESIZE];
static char	env_home[LINESIZE];
char	env_path[LINESIZE];
char	*envp[] = {
	env_user, env_shell, env_home, env_path, NULLCP
};

long	message_size;
int	firstSuccessDR, firstFailDR;
char	*local_user;

/* ---------------------  Begin Routines  --------------------------------- */

main (argc, argv)
int     argc;
char    **argv;
{
	char    *p;

	if ((p = rindex(argv[0], '/')) != NULLCP)
		p++;
	else	p = argv[0];

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
	/* NOTREACHED */
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

	(void) signal (SIGCHLD, SIG_DFL);

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
	firstFailDR = TRUE;
	firstSuccessDR = TRUE;
	if (this_msg)
		free (this_msg);

	this_msg = qb2str (arg -> qid);

	PP_NOTICE (("Processing msg %s", this_msg));

	delivery_init (arg -> users);

	retval = rd_msg (this_msg, &prm, qp, &ad_sendr, &ad_recip, &ad_count);
	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("rd_msg err: %s", this_msg));
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					"Can't read message");
	}

	for (up = arg -> users; up; up = up -> next)
		douser ((int)up -> RecipientId -> parm, ad_recip);

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

static int douser (rno, ad_recip)
int     rno;
ADDR    *ad_recip;
{
	ADDR    *ap;

	for (ap = ad_recip; ap ; ap = ap->ad_next) {
		if (rno != ap -> ad_no)
			continue;
		if (lchan_acheck (ap, mychan, 1, NULLVP) == NOTOK)
			return;
		else {
			deliver (ap);
			return;
		}
	}

	PP_LOG (LLOG_EXCEPTIONS,
		("user not recipient of %s", this_msg));

	delivery_setstate (rno, int_Qmgr_status_messageFailure,
			   "user is not a recipient");
}


static char    *formatdir = NULLCP;

/*
Do the delivery
*/
static int deliver (ap)
ADDR    *ap;
{
	char	buffer[BUFSIZ],
		filename[MAXPATHLENGTH];
	int	hdr_fd, body_fd,
		retval;
	LocUser	*loc;
	char	*cp;
	int size;
	struct stat st;

	if (local_user)
		free(local_user);
	if ((local_user = ad_getlocal (ap -> ad_r822adr, AD_822_TYPE, 1)) == NULLCP) {
		(void) sprintf (buffer, "User %s not local!",
				ap -> ad_r822adr);
		PP_LOG (LLOG_EXCEPTIONS, ("%s", buffer));
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER, DRD_UA_UNAVAILABLE, buffer);
		if (!rp_isbad(wr_q2dr (&Qstruct, this_msg)))
			delivery_set (ap -> ad_no,
				      int_Qmgr_status_negativeDR);
		return RP_USER;
	}

	if ((loc = tb_getlocal (local_user, mychan)) == NULL) {
		(void) sprintf (buffer,
				"User %s not registered in channel table %s",
				local_user, mychan -> ch_name);
		PP_LOG (LLOG_EXCEPTIONS, ("%s", buffer));
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER, DRD_UA_UNAVAILABLE, buffer);
		if (!rp_isbad(wr_q2dr (&Qstruct, this_msg))) {
			delivery_set (ap -> ad_no, 
				      (firstFailDR == TRUE) ? 
				      int_Qmgr_status_negativeDR :
				      int_Qmgr_status_failureSharedDR);
			firstFailDR = FALSE;
		}			
		return RP_USER;
	}
	if (loc -> searchpath == NULLCP)
		loc -> searchpath = strdup (USRPATH);

	PP_TRACE((
"Found entry for '%s' uid %d, gid %d, user '%s' dir '%s' mailbox '%s', \
shell %s, home %s, format %d, restrict %d, filter %s, sysfilter %s, \
PATH %s, opts `%s'",
		   ap -> ad_r822adr,
		loc ->	uid,
		loc ->	gid,
		(loc -> username) ? loc -> username : "<no username>",
		(loc -> directory) ? loc -> directory : "<no directory>",
		(loc -> mailbox) ? loc -> mailbox : "<no mailbox>",
		(loc -> shell) ? loc -> shell : "<no shell>",
		(loc -> home) ? loc -> home : "<no home>",
		  loc -> mailformat,
		loc -> restricted,
		(loc -> mailfilter) ? loc -> mailfilter : "<no mailfilter>",
		(loc -> sysmailfilter) ? loc -> sysmailfilter : "<no sysmailfilter>",
		(loc -> searchpath) ? loc -> searchpath : "<no searchpath>",
		(loc -> opts) ? loc -> opts : "<no opts>"));

	if (qid2dir (this_msg, ap, TRUE, &formatdir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't locate message %s", this_msg));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_messageFailure,
			      "Can't find the message");
		return RP_MECH;
	}
	if (rp_isbad (retval = msg_rinit (formatdir))) {
		PP_LOG (LLOG_EXCEPTIONS, ("msg_rinit can't init %s",
			      formatdir));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_messageFailure,
			      "can't read the body");
		return RP_MECH;
	}
	if (msg_rfile (filename) != RP_OK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't initialise hdr"));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_messageFailure,
				   "No message header");
		msg_rend ();
		return RP_MECH;
	}
	if ((cp = rindex (filename, '/')) != NULLCP)
		cp ++;
	else	cp = filename;
	if (lexnequ (cp, hdr_822_bp, strlen (hdr_822_bp)) != 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("Bad header type, %s", cp));
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER,
			 DRD_IMPLICITCONV_NOTREGISTERED,
			 "Header of message is not RFC-822 based");
		if (!rp_isbad(wr_q2dr (&Qstruct, this_msg))) 
			delivery_set (ap -> ad_no, int_Qmgr_status_negativeDR);
		msg_rend ();
		return RP_MECH;
	}
	    
	if ((hdr_fd = open (filename, O_RDONLY, 0)) < 0) {
		PP_SLOG (LLOG_EXCEPTIONS, filename,
			 ("Can't open file"));
		(void) sprintf (buffer, "Can't open file %s", filename);
		delivery_setstate (ap -> ad_no,
			      int_Qmgr_status_messageFailure,
			      buffer);
		return RP_MECH;
	}
	if (fstat (hdr_fd, &st) != NOTOK)
		size = st.st_size;

	switch (msg_rfile (filename)) {
	    case RP_OK:
		if ((body_fd = open (filename, O_RDONLY, 0)) < 0) {
			PP_SLOG (LLOG_EXCEPTIONS, filename,
				 ("Can't open file"));
			(void) sprintf (buffer, "Can't open file %s",
					filename);
			delivery_setstate (ap -> ad_no,
					   int_Qmgr_status_messageFailure,
					   buffer);
			(void) close (hdr_fd);
			msg_rend ();
			return RP_MECH;
		}
		if (fstat (body_fd, &st) != NOTOK)
			size += st.st_size;
		break;

	    case RP_DONE:
		PP_NOTICE (("Message with header part only"));
		if ((body_fd = open ("/dev/null", O_RDONLY, 0)) < 0) {
			PP_SLOG (LLOG_EXCEPTIONS, "/dev/null",
				 ("Can't open file"));
			delivery_set (ap -> ad_no,
				      int_Qmgr_status_messageFailure);
			(void) close (hdr_fd);
			msg_rend ();
			return RP_MECH;
		}
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Can't initialise body"));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_messageFailure,
				   "Can't initialise body");
		(void) close (hdr_fd);
		msg_rend ();
		return RP_MECH;
	}

	if (msg_rfile (filename) != RP_DONE) {
		PP_LOG (LLOG_EXCEPTIONS, ("Extra body parts in message %s",
					  this_msg));
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER,
			 DRD_IMPLICITCONV_NOTREGISTERED,
			 "Extra body part that has not been converted to 822");
		if (!rp_isbad(wr_q2dr (&Qstruct, this_msg)))
			delivery_set (ap -> ad_no, int_Qmgr_status_negativeDR);
		(void) close (hdr_fd);
		(void) close (body_fd);
		msg_rend ();
		return RP_USER;
	}
	msg_rend ();

	/* set up the env */
	(void) sprintf (env_home, "HOME=%s", loc -> home ? loc -> home :
			(loc -> directory ? loc -> directory : "."));
	(void) sprintf (env_shell, "SHELL=%s",
			loc -> shell ? loc -> shell : "/bin/sh");
	(void) sprintf (env_user, "USER=%s",
			loc -> username ? loc -> username : local_user);

	(void) sprintf (env_path, "PATH=%s", USRPATH);
	size += 1 + strlen (Qstruct.Oaddress -> ad_r822adr);
	message_size = size;
	retval = child_process (loc, hdr_fd, body_fd);
	(void) close (hdr_fd);
	(void) close (body_fd);
	switch (retval) {
	    case int_Qmgr_status_negativeDR:
		{
			char	buf[LINESIZE];

			(void) sprintf (buf, "Delivery to %s (%d) failed",
					loc -> username ?
					loc -> username : local_user,
					loc -> uid);
			
			set_1dr (&Qstruct, ap -> ad_no, this_msg,
				 DRR_TRANSFER_FAILURE,
				 DRD_UA_UNAVAILABLE, buf);
			if (!rp_isbad(wr_q2dr (&Qstruct, this_msg))) {
				delivery_set (ap -> ad_no, 
					      (firstFailDR == TRUE) ?
					      retval :
					      int_Qmgr_status_failureSharedDR);
				firstFailDR = FALSE;
			}
			free_loc_user (loc);
		}
		return RP_MECH;

	    case int_Qmgr_status_success:
		break;

	    case int_Qmgr_status_mtaFailure:
	    case int_Qmgr_status_messageFailure:
	    case int_Qmgr_status_mtaAndMessageFailure:
		delivery_set (ap -> ad_no, retval);
		free_loc_user (loc);
		return RP_MECH;

	    case NOTOK:
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("child process failed"));
		delivery_setstate (ap -> ad_no,
			      int_Qmgr_status_messageFailure,
			      "Child process failed");
		free_loc_user (loc);
		return RP_MECH;
	}

	if (loc -> username)
		PP_NOTICE ((">>> local : Message delivered to %s (%s)",
			    local_user, loc -> username));
	else
		PP_NOTICE ((">>> local: Message delivered to %s (id %d/%d)",
			    local_user, loc -> uid, loc -> gid));
	free_loc_user (loc);
	if (ap -> ad_usrreq == AD_USR_CONFIRM ||
	    ap -> ad_mtarreq == AD_MTA_CONFIRM ||
	    ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_NO_REASON,
			-1, NULLCP);
		if (!rp_isbad(wr_q2dr (&Qstruct, this_msg))) {
			delivery_set (ap -> ad_no, 
				      (firstSuccessDR == TRUE) ?
				      int_Qmgr_status_positiveDR :
				      int_Qmgr_status_successSharedDR);
			firstSuccessDR = FALSE;
		}
	}
	else {
		(void) wr_ad_status (ap, AD_STAT_DONE);
		(void) wr_stat (ap, &Qstruct, this_msg, (int)message_size);
		delivery_set (ap -> ad_no, int_Qmgr_status_success);
	}
	return RP_OK;
}
