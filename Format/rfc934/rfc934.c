/* rfc934.c: rfc934 filter channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc934/RCS/rfc934.c,v 6.0 1991/12/18 20:21:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc934/RCS/rfc934.c,v 6.0 1991/12/18 20:21:02 jpo Rel $
 *
 * $Log: rfc934.c,v $
 * Revision 6.0  1991/12/18  20:21:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "head.h"
#include "qmgr.h"
#include "q.h"
#include "prm.h"
#include "chan.h"
#include "dr.h"
#include <sys/stat.h>
#include "sys.file.h"
#include "Qmgr-types.h"

extern char	*quedfldir;
extern char 	*chndfldir;
extern void	sys_init(), err_abrt(), rd_end();
CHAN 		*mychan;
char		*this_msg = NULL, *this_chan = NULL;
extern int	err_fatal;
int		first_failureDR;

static struct type_Qmgr_DeliveryStatus *process ();
static int initialise ();
static int security_check ();
static void dirinit ();
static struct type_Qmgr_DeliveryStatus *new_DeliveryStatus();
static ADDR *getnthrecip ();
static int filterMsg();
static int doFilter();
/*  */
/* main routine */
 
main (argc, argv)
int 	argc;
char	**argv;
{
	sys_init(argv[0]);
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control(argc,argv,initialise,process,NULLIFP);
	else
#endif
		channel_control (argc, argv, initialise, process,NULLIFP);
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
		       ("Chans/rfc934 : Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}

	/* check if a rfc934 channel */
	if (name != NULL) free(name);
	return OK;
}

/*  */
/* routine to check if allowed to rfc934 filter this message */
static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
	char *msg_file = NULL, *msg_chan = NULL;
	int result;

	result = TRUE;
	msg_file = qb2str (msg->qid);
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp(msg_chan,mychan->ch_name) !=0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/rfc934 channel err: '%s'",msg_chan));
		result = FALSE;
	}
	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	return result;
}

/*  */
/* routine called to do rfc934 filter */

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars	prm;
	Q_struct	que;
	ADDR		*sender = NULL;
	ADDR		*recips = NULL;
	int 		rcount;
	int		retval;
	struct type_Qmgr_UserList 	*ix;
	ADDR				*adr;
	char		*error;
	bzero((char *)&prm,sizeof(prm));
	bzero((char *)&que,sizeof(que));
	first_failureDR = TRUE;

	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_messageFailure);
	
	if (security_check(arg) != TRUE)
		return deliverystate;

	if (this_msg != NULL) free(this_msg);
	if (this_chan != NULL) free(this_chan);

	this_msg = qb2str(arg->qid);
	this_chan = qb2str(arg->channel);

	PP_LOG(LLOG_NOTICE,
	       ("filtering msg '%s' through '%s'",this_msg, this_chan));

	if (rp_isbad(rd_msg(this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/rfc934 rd_msg err: '%s'",this_msg));
		rd_end();
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Can't read message");
	}

	/* check each recipient for processing */
	for (ix = arg->users; ix; ix = ix->next) {
		error = NULLCP;
		if ((adr = getnthrecip(&que,ix->RecipientId->parm)) == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
				("Chans/rfc934 : failed to find recipient %d of msg '%s'",ix->RecipientId->parm, this_msg));

 			delivery_setstate(ix->RecipientId->parm,
					  int_Qmgr_status_messageFailure,
					  "Unable to find specified recipient");
			continue;
		}

		switch (chan_acheck(adr, mychan, 1, NULL)) {
		    case OK:
			if (filterMsg(this_msg,adr, &error) == NOTOK) {
				if (err_fatal == TRUE) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("failed to flatten msg '%s' (FATAL)", 
						this_msg));
					set_1dr(&que, adr->ad_no, this_msg,
						DRR_CONVERSION_NOT_PERFORMED,
						DRD_CONTENT_SYNTAX_ERROR,
						(error == NULLCP) ? "Unable to flatten the message" : error);
					delivery_set(adr->ad_no,
						    (first_failureDR == TRUE) ? int_Qmgr_status_negativeDR : int_Qmgr_status_failureSharedDR);
					first_failureDR = FALSE;
				} else {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Chans/rfc934 : failed to filter msg '%s' for recip '%d' on channel '%s'",this_msg, adr->ad_no, this_chan));
					delivery_setstate(adr->ad_no,
							  int_Qmgr_status_messageFailure,
							  (error == NULLCP) ? "Failed to flatten message" : error);
				}
			} else {
				/* CHANGE update adr->ad_rcnt in struct and in file */
				adr->ad_rcnt++;
				wr_ad_rcntno(adr,adr->ad_rcnt);
				delivery_set(adr->ad_no,
					     int_Qmgr_status_success);
			}
			break;
		    default:
			break;
		}
		if (error != NULLCP)
			free(error);
	}
	if (rp_isbad(retval = wr_q2dr(&que, this_msg))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure '%d'",mychan->ch_name,retval));
		delivery_resetDRs(int_Qmgr_status_messageFailure);
	}
	rd_end();
	q_free (&que);
	prm_free(&prm);
	return deliverystate;
}

/*  */
static int filterMsg (msg,recip,perr)
/* return OK if managed to filter msg through mychan for recip */
char	*msg;
ADDR	*recip;
char	**perr;
{
	char	*origdir = NULL,
	        *newdir = NULL;
	int	result = OK;
	struct stat statbuf;

	if (qid2dir(msg, recip, TRUE, &origdir) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/rfc934 original directory not found for recipient %d of message '%s'",recip->ad_no, msg));
		*perr = strdup("Can't find source directory");
		result = NOTOK;
	}

	/* temporary change to get new directory name */
	recip->ad_rcnt++;
	if ((result == OK) 
	    && (qid2dir(msg, recip, FALSE, &newdir) != OK)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/rfc934 couldn't construct new directory for recipient '%d' of message '%s'",recip->ad_no, msg));
		*perr = strdup("Can't construct destination directory");
		result = NOTOK;
	}
	recip->ad_rcnt--;
	
	if ((result == OK)
	    && (stat(newdir, &statbuf) == OK)
	    && ((statbuf.st_mode & S_IFMT) == S_IFDIR)) {
		/* directory already exists and so filter already done */

		if (origdir != NULL) free(origdir);
		if (newdir != NULL) free(newdir);
		return OK;
	}

	if ((result == OK) && (doFilter(origdir,newdir,msg, perr) != OK))
		result = NOTOK;

	if (origdir != NULL) free(origdir);
	if (newdir != NULL) free(newdir);
	return result;
}

/*  */
static doFilter (orig, new, msg, perr)
/* filters orig directory through mychan to new directory */
char	*orig,
	*new,
	*msg,
	**perr;
{
	char		tmpdir[MAXPATHLENGTH], buf[BUFSIZ];
	int		result = OK;
	struct stat 	statbuf;

	(void) sprintf(tmpdir, "%s/tmp.%s", 
		       msg, mychan->ch_name);
	
	if (stat(tmpdir, &statbuf) == OK) {
		char	*cmd_line;
		/* exists so remove it */
		cmd_line = malloc((unsigned) (strlen("rm -rf ") +
						   strlen(tmpdir) + 1));
		sprintf(cmd_line, "rm -rf %s", tmpdir);
		system(cmd_line);
		if (cmd_line != NULL) free(cmd_line);
	}

	if (mkdir(tmpdir, 0777) != OK) {
		PP_SLOG(LLOG_EXCEPTIONS, tmpdir,
		       ("Can't make directory"));
		(void) sprintf (buf,
				"Failed to make temporary directory '%s'",
				tmpdir);
		*perr = strdup(buf);
		result = NOTOK;
	}

	if (result == OK) {
		/* filter from orig to tmpdir */
		result = do_rfc934(orig,tmpdir,perr);
		/* if success rename tmpdir to new */
		if ((result == OK) && (rename(tmpdir,new) == -1)) {
			PP_SLOG(LLOG_EXCEPTIONS, "rename",
				("Can't rename directory '%s' to '%s'",
				 tmpdir, new));
			(void) sprintf (buf,
					"Failed to rename '%s' to '%s'",
					tmpdir,new);
			*perr = strdup(buf);
			result = NOTOK;
		}

	}
	return result;
}
	   
/*  */
/* auxilary routines to extract from lists */
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

