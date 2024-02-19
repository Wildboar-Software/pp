/* msg_clean.c: message cleanout channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/msg-clean/RCS/msg-clean.c,v 6.0 1991/12/18 20:11:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/msg-clean/RCS/msg-clean.c,v 6.0 1991/12/18 20:11:18 jpo Rel $
 *
 * $Log: msg-clean.c,v $
 * Revision 6.0  1991/12/18  20:11:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "head.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "q.h"
#include "prm.h"
#include <sys/param.h>
#include <sys/stat.h>
#include "sys.file.h"
#include <isode/usr.dirent.h>


extern char     *quedfldir;
extern char     *chndfldir;
extern CHAN     *ch_nm2struct();
extern void	getfpath(), rd_end(), sys_init(), err_abrt();
/* CHANGE LATER */
CHAN            *mychan;

static struct type_Qmgr_DeliveryStatus *process ();
static int initialise ();
static int security_check ();
static void dirinit ();
int	error;
/*  */
/* main routine */

main (argc, argv)
int     argc;
char    **argv;
{
	sys_init(argv[0]);
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control(argc,argv,initialise, process, NULLIFP);
	else
#endif
		channel_control (argc, argv, initialise, process, NULLIFP);
}

/*  */
/* routine to move to correct place in file system */

static void dirinit ()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, " Unable to change directory to '%s'",
			  quedfldir);
}

/*  */
/* channel initialise routine */

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char *name;

	name = qb2str(arg);

	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP,
			("Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}
	/* check if a cleanout channel */
	if (mychan -> ch_chan_type != CH_DELETE) {
		PP_OPER (NULLCP,
			 ("Channel '%s' is not a msg deletion channel",
			  name));
		if (name) free (name);
		return NOTOK;
	}
	if (name != NULL) free(name);
	return OK;
}

/*  */
/* routine called to do clean out */
static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	char    *this_msg = NULL;
	struct stat statbuf;

	dirinit ();
	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_messageFailure);

	if (security_check(arg) != TRUE)
		return deliverystate;

	/* ok-you asked for it-remove message */
	this_msg = qb2str (arg->qid);

	PP_LOG(LLOG_NOTICE,
	       ("Cleaning out msg %s", this_msg));


	error = FALSE;

	/*
	 * This stuff here to try and win efficiency.
	 * In a crowded queue, the chdir business should cut down
	 * the time spent in kernel namei routine.
	 */
	if (chdir (this_msg) == 0) {
		(void) recrm (".");
		dirinit ();
		if (rmdir (this_msg) == NOTOK)
			error = TRUE;
	}
	else if (stat(this_msg, &statbuf) == OK) {
		if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
			if (unlink (this_msg) == NOTOK)
				error = TRUE;
		}
	}
	if (error == TRUE)
		PP_SLOG (LLOG_EXCEPTIONS, this_msg,
			 ("Unable to remove directory"));

	if (this_msg != NULL) free(this_msg);
	/* create return values */
	if (error == FALSE)
		delivery_setall(int_Qmgr_status_success);
	return deliverystate;

}

/*  */
/* routine to check if allowed to remove this message */
static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
	char            *msg_file = NULL, *msg_chan = NULL;
	struct prm_vars prm;
	Q_struct        que;
	ADDR            *sender = NULL;
	ADDR            *recips = NULL;
	int             rcount,
			result;
	ADDR            *ix = NULL;
	char    	msgname[MAXPATHLENGTH];
	struct stat	statbuf;
	extern char 	*aquefile;

	prm_init (&prm);
	q_init (&que);

	result = TRUE;
	msg_file = qb2str (msg->qid);
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp(msg_chan,mychan->ch_name) != 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Wrong channel name '%s'",msg_chan));
		result = FALSE;
	}

	if (rp_isbad(rd_msg_file(msg_file,&prm,&que,
			    &sender,&recips,&rcount,
			    RDMSG_RDONLY))) {
		(void) sprintf (msgname, "%s/%s", msg_file, aquefile);
		if (stat(msgname, &statbuf) != OK) {
			/* does not exist so effectively removed */
			if (msg_file != NULL) free(msg_file);
			if (msg_chan != NULL) free(msg_chan);
			return TRUE;
		}
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Can't read message '%s'",msg_file));
		result = FALSE;
	}

	/* unlock file */
	rd_end();

	/* now check the recips */
	if (result == TRUE) {
		ix = que.Raddress;
		while ((ix != NULL) && (ix->ad_status == AD_STAT_DONE))
			ix = ix->ad_next;
		if (ix != NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("recipient %d status not DONE: '%s'",
				ix->ad_no,msg_file));
			result = FALSE;
		}
	}

	/* free all storage used */
	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	q_free (&que);
	prm_free(&prm);
	return result;
}
