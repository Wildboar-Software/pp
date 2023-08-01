/* submit.c: the mail enqueuer */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/submit.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/submit.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: submit.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "q.h"
#include "or.h"
#include <varargs.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <pwd.h>
#include <isode/cmd_srch.h>



/* -- externals -- */
extern char	*msg_unique,	/* unique part of msg queue name  */
		*mgt_inhost,
		*qmgr_hostname,
		*rcmd_srch(),
		*quedfldir,	/* dir w/mail queue directories */
		auth2submit_msg[];

extern int	       mgt_adtype,
		protocol_mode;

ADDR		*ad_originator,
		*ad_recip;

extern struct passwd	*getpwuid();
extern long		msg_size;
extern CHAN		*ch_inbound;
extern CMD_TABLE	qtbl_mt_type[];




/* -- globals -- */
char	*username;	/* login name of usr running me */
int	effecid,	/* id providing access priviliges */
	privileged,	/* am I a priveledged user ? */
	userid,		/* id of user running me */
	accept_all,
	submit_debug = 0,
	notrace;
Q_struct		Qstruct, *qptr;




/* -- static variables	-- */
static struct prm_vars	Prm;
static int		qmgr_fd = NOTOK;
static int		log_msgtype;
static int		adr_count;

extern void fmt_init ();
extern int validate_recip ();
extern void auth_start ();
extern void gen_warnings ();
extern void gen_probedr ();
extern void gen_ndr ();
extern void auth_log ();
extern void ad_init ();
extern void check_splitter ();
extern void check_conversions ();
extern void check_report_level ();
extern void check_crits ();
extern void check_dr_crits ();
extern void msg_mgt ();
extern void time_mgt ();
extern void clear_q ();
extern int move_q ();
extern int winit_q ();
extern int write_q ();
extern void txt_input ();
extern void ad_log_print ();
extern ADDR *read_sender ();
extern ADDR *read_recipient ();
extern int 		server_start ();

/* -- local routines -- */
void			err_abrt();
void			pro_reply();

static int		priv_user();
static void		_pro_reply();
static void		dir_init();
static void		done();
static void		msg_init();
static void		user_init();



/*  --------------------  Main	Routine	 ---------------------------------- */



/* ARGSUSED */
main (argc, argv)
int		argc;
char		**argv;
{
	int	retval;
	int	qmgr_status;
	int	server = 0;
	ADDR	*ad;
	ADDR	**app;
	RP_Buf rps, *rp = &rps;
	int opt;
	extern char *optarg, *pptailor;

	while ((opt = getopt (argc, argv, "dst:")) != EOF) {
		switch (opt) {
		    case 'd':
			submit_debug = 1;
			break;
		    case 's':
			server = 1;
			break;
		    case 't':
			pptailor = optarg;
			break;
		}
	}

	(void) umask (0);
	sys_init (argv[0]);

	qptr = &Qstruct;
	q_init (qptr);

	user_init();
	dir_init();
	or_myinit();
	fmt_init();

	(void) signal (SIGPIPE, done);
	if (server)
		qmgr_fd = server_start (qmgr_hostname, &qmgr_status);
	else
		qmgr_fd = qmgr_start (qmgr_hostname, &qmgr_status, 1);

#define pp_could_go_faster 1
	while (pp_could_go_faster) { /* infinite loop! */

		/*
			Reset various parameters
		*/
		msg_init();

		protocol_mode = 1;


		/*
			Read the parameter settings for a Message
		*/
		if (rp_isbad (retval = rd_prm_info (&Prm))) {
			if (retval == RP_EOF)
				break; /* we're done */
			err_abrt (retval, "bad parameters");
		}
		else
			pro_reply (RP_OK, "Got the parameters");

		if (qmgr_status != DONE && qmgr_fd != NOTOK)
			qmgr_fd = qmgr_retry (qmgr_fd, &qmgr_status);
		/*
			Read the queue structure
		*/
		if (rp_isbad (retval = rd_q_info (qptr)))
			err_abrt (retval, "Bad queue read");

		/*
			Perform management functions on parameters
		*/
		if (rp_isbad (retval = cntrl_mgt (&Prm, qptr)))
			err_abrt (retval, "Bad parameter settings");

		/*
			Perform management functions on the queue
		*/
		if (rp_isbad (retval = q_mgt (qptr)))
			err_abrt (retval, "Bad queue parameter settings");

		pro_reply (RP_OK, "Starting fine, sender please");

		/*
		 * read and validate the sender
		 */

		ad = read_sender (qptr);
		if (validate_sender(qptr, ad, username, rp) != RP_OK)
			err_abrt (rp -> rp_val, "%s", rp -> rp_line);
		ad_originator = ad;

		pro_reply (RP_AOK, "Continuing fine, addresses please");

		if (qmgr_status != DONE && qmgr_fd != NOTOK)
			qmgr_fd = qmgr_retry (qmgr_fd, &qmgr_status);

		/*
		 * Read and validate the recipients.
		 */

		app = &ad_recip;
		while ((ad = read_recipient (qptr)) != NULLADDR) {
			if (rp_isbad(validate_recip(ad, qptr, rp))) {
				if (rp_gbval (rp -> rp_val) == RP_BTNO &&
				    retval != RP_BHST &&
				    retval != RP_DHST)
					err_abrt (rp -> rp_val, "%s",
						  rp -> rp_line);
				pro_reply (rp -> rp_val, "%s", rp -> rp_line);
				adr_tfree (ad);
				continue;
			}
			else
				pro_reply (rp -> rp_val, "%s", rp -> rp_line);
			(*app) = ad; /* add to recipient list */
			app = &(*app) -> ad_next;
			adr_count ++;
		}
		if (ad_recip == NULLADDR)
			err_abrt (RP_USER, "No valid addresses found");
		ad_log_print (ad_recip);

		/*
			Authorisation - main routines
		*/

		auth_start(qptr, ad_originator, ad_recip);

		/*
			Sets the Qstruct's Originator and Recipient fields
		*/
		qptr -> Oaddress = ad_originator;
		qptr -> Raddress = ad_recip;
		time_mgt(qptr);
		/*
			Initialize to the queue directory for output
		*/

		if (rp_isbad(winit_q(rp)))
			err_abrt (rp -> rp_val, "%s", rp -> rp_line);

		/*
			if (DMPDU)
				parse DR from incoming channel
				then get body (if any)
			else if probe
				nothing much
			else (normal message)
				splat the body into a file
		*/

		log_msgtype = qptr -> msgtype;

		switch (qptr -> msgtype) {
		case MT_DMPDU:
			pro_reply (RP_AOK, "Addresses ok, DR info please");
			if (rp_isbad (retval = gen_io2dr(qptr))) {
				err_abrt (retval, "Bad DR struct");
			}
			pro_reply (RP_AOK, "DR OK");
			txt_input (qptr, notrace == 0);
			break;

		case MT_PMPDU:
			msg_size = qptr -> msgsize; 
			break;

		default:
			pro_reply (RP_AOK, "Addresses ok, text please");
			txt_input (qptr, notrace == 0);
			break;
		}

		if (qmgr_status != DONE && qmgr_fd != NOTOK)
			qmgr_fd = qmgr_retry (qmgr_fd, &qmgr_status);

		protocol_mode = 0;	/* end of protocol phase */

		/*
			finish authorisation tests on message
			size/body parts and perform channel binding
		*/

		if (rp_isbad (retval = auth_finish(qptr, ad_originator,
						   ad_recip)))
			if (!accept_all)
				err_abrt (retval, "%s : %s",
				"At least one recipient fails authorisation",
				auth2submit_msg);
		
		check_splitter(qptr);
		check_crits (qptr);
		check_conversions (qptr);
		check_report_level (qptr);
		setup_directories (qptr);

		/*
			Generate the appropriate Delivery Notifications 
		*/

		switch (qptr -> msgtype) {
		case MT_PMPDU:
			gen_probedr(qptr);
			break;
		default:
			gen_ndr(qptr);
			break;
		}

		if (rp_isbad (write_q (qptr, &Prm, rp)) ||
		    rp_isbad (move_q(rp)))
			err_abrt (rp -> rp_val, "%s", rp -> rp_line);


		pro_reply (RP_MOK, "Submitted & queued (%s)", msg_unique);


		/*
			generate authorisation statistics logs
			now that msg_unique is known
		*/

		auth_log(qptr, ad_originator, ad_recip, msg_unique);

		/* Tell qmgr */

		if (qmgr_fd != NOTOK)
			tell_qmgr (msg_unique, &Prm, qptr, ad_originator,
				   ad_recip, adr_count);
		if (ch_inbound != NULL && ch_inbound->ch_access == CH_MTS)
			PP_NOTICE (("<<< local %s %s %s %s %s",
				    rcmd_srch(log_msgtype, qtbl_mt_type),
				    msg_unique,
				    ch_inbound -> ch_name,
				    username, 
				    mgt_inhost));
		else
			PP_NOTICE (("<<< remote %s %s %s %s", 
				    rcmd_srch(log_msgtype, qtbl_mt_type),
				    msg_unique,
				    ch_inbound -> ch_name,
				    mgt_inhost == NULLCP ? "<unknown-host>" :
				    mgt_inhost));

		gen_warnings (qptr, ad_originator, ad_recip);
	}

	if (qmgr_fd != NOTOK)
		(void) qmgr_end (qmgr_fd);

	PP_TRACE (("Submit normal end"));
	exit (0); /* NOTREACHED */
}


tell_qmgr (msg, prm, qp, sndr, recip, cnt)
char	*msg;
struct prm_vars *prm;
Q_struct *qp;
ADDR	*sndr, *recip;
int cnt;
{
	if (qmgr_fd != NOTOK)
		(void) message_send (qmgr_fd, msg, prm, qp, sndr, recip, cnt);
}


/*  --------------------  Static  Routines  ------------------------------- */




/*
Get information on the incomming channel calling submit
*/

static void user_init()
{
	register struct passwd *pw;

	PP_LOG (LLOG_DEBUG, ("submit/user_init()"));

	privileged = FALSE;
	userid = getuid();
	effecid = geteuid();

	if ((pw = getpwuid (userid)) == NULL)
		err_abrt (RP_LIO,
			 "No login for UID %d, call Support.", userid);
	endpwent();

	username = strdup (pw->pw_name);

	if (priv_user (userid, effecid) == TRUE)
		privileged = TRUE;

	if (pp_setuserid() == NOTOK)
		err_abrt (RP_LIO, "Unable to set user id");
}

static int priv_user (id, eid)
int	id, eid;
{
	struct passwd *pwd;
	extern char *pplogin;

	if (id == 0 || eid == id)
		return TRUE;

	if ((pwd = getpwnam (pplogin)) == (struct passwd *)0)
		return FALSE;
	if (id == pwd -> pw_uid)
		return TRUE;
	return FALSE;
}



/*
chdir
*/

static void dir_init()
{
	/*
	cd into the queue directory
	*/
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, 
			"Unable to change directory. '%s'", quedfldir);
}




/*
Initialise a msg
*/

static void msg_init()
{
	msg_unique = NULLCP;	/* for logging failed messages */

	/*
	  address and qstruct 
	  done in two stages in case we filled in recipients but not
	  assigned to qstruct 
	 */
	adr_count = 0;
	if (ad_recip != NULLADDR)
		adr_tfree (ad_recip);
	if (ad_originator != NULLADDR)
		adr_tfree (ad_originator);
	ad_originator = NULLADDR;
	ad_recip = NULLADDR;
	qptr -> Oaddress = NULLADDR;
	qptr -> Raddress = NULLADDR;
	q_free (qptr);
	msg_mgt();
}

/*  --------------------  Reply	 Routines  -------------------------------- */




#ifdef lint
/*VARARGS2*/
void pro_reply (code, fmt)
int code;
char *fmt;
{
	pro_reply (code, fmt);
}
#else
void pro_reply (va_alist)
va_dcl
{
	va_list ap;
	int	code;

	va_start(ap);

	code = va_arg (ap, int);

	_pro_reply (code, ap);

	va_end (ap);
}

static void _pro_reply (code, ap)
int	code;
va_list ap;
{
	char	buffer[4*BUFSIZ];
	register char  *errchar = NULLCP;

	if (rp_isbad (code))
		switch (code) {
		    case RP_HUH: /* not if it was a user error */
		    case RP_PARM:
		    case RP_USER:
		    case RP_PARSE:
		    case RP_BHST:
		    case RP_NAUTH:
		    case RP_EOF:
			break;

		    default:
			errchar = " ";
			break;
		}

	_asprintf (buffer, NULLCP, ap);

	if (rp_isbad (code))
	    PP_SLOG (LLOG_EXCEPTIONS, errchar,
		     ("submit/pro_reply(%s) '%s'", rp_valstr (code),
		      buffer));
	else
	    PP_LOG (LLOG_PDUS|LLOG_TRACE,
		    ("submit/pro_reply(%s) '%s'", rp_valstr (code),
		     buffer));

	if (submit_debug)
		printf ("%s:", rp_valstr(code));
	else
		(void) putc (code, stdout);

	(void) fputs (buffer, stdout);
	(void) putc ('\n', stdout);
	(void) fflush (stdout);
}
#endif

/* ---------------  (err_)  GENERAL ERROR-HANDLING  ----------------------- */


#ifdef lint
/*VARARGS2*/
void err_abrt (code, fmt)
int	code;
char	*fmt;
{
	clear_q();

	pro_reply (code, fmt);
}
#else
void err_abrt (va_alist)   /* terminate the process		 */
va_dcl
{
	va_list ap;
	int	code;

	va_start (ap);

	code = va_arg (ap, int);

	PP_TRACE (("err_abrt (%d)", code));

	_pro_reply (code, ap);

	va_end (ap);

	done (-1);	      /* pass the reply code to caller	    */
}
#endif

static void done (n)
int	n;
{
	PP_TRACE (("done (%d)", n));
	clear_q();
	if (qmgr_fd != NOTOK)
		qmgr_end (qmgr_fd);
	exit (n);
}
