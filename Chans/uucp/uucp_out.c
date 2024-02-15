/* uucp_out.c: uucp outbound channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/uucp/RCS/uucp_out.c,v 6.0 1991/12/18 20:13:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/uucp/RCS/uucp_out.c,v 6.0 1991/12/18 20:13:06 jpo Rel $
 *
 * $Log: uucp_out.c,v $
 * Revision 6.0  1991/12/18  20:13:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "head.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "q.h"
#include "dr.h"
#include "prm.h"
#include <isode/cmd_srch.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<sys/wait.h>
#include	<signal.h>

#ifdef __STDC__
#define VOLATILE volatile
#else
#define VOLATILE
#endif

#ifndef WCOREDUMP
#define WCOREDUMP(x) (((union wait *)&(x))->w_coredump)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(x) (((union wait *)&(x)) -> w_retcode)
#endif
#ifndef WTERMSIG
#define WTERMSIG(x) (((union wait *)&(x)) -> w_termsig)
#endif

extern char	*quedfldir,
		*chndfldir,
		*logdfldir,
		*hdr_822_bp;

extern CHAN 	*ch_nm2struct();
extern	char	*loc_dom_mta;
extern int	rfc8222uu();
CHAN 		*mychan;
char		*this_msg = NULL, *this_chan = NULL;
char		*myhostname;
char		*uuxstr;
int		first_successDR;

static struct type_Qmgr_DeliveryStatus *process();
static void set_success();
static void dirinit();
static ADDR *getnthrecip();
static jmp_buf	toplevel;
static int   pipe_signaled;
static int data_bytes;
static int initialise();

static int processMsg(), doProcess(), send_to_child();
/* ARGSUSED */
static SFD sig_pipe(sig)
int sig;
{
	pipe_signaled = TRUE;
	longjmp (toplevel, DONE);
}

/*  */
/* main routine */
 
main (argc, argv)
int 	argc;
char	**argv;
{
	sys_init (argv[0]);
	dirinit();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp (argv[1],"debug") == 0))
		debug_channel_control (argc,argv,initialise,process, NULLIFP);
	else
#endif
		channel_control (argc, argv, initialise, process, NULLIFP);
}

/*  */
/* routine to move to correct place in file system */

static void dirinit()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, " Unable to change directory to '%s'",
			  quedfldir);
}

/*  */

static CMD_TABLE uucptbl[] = {
#define TBL_UUXCMD 1
	"uux",	TBL_UUXCMD,
#define TBL_HOST 2
	"host",	TBL_HOST,
	0, -1
	};

/* channel initialise routine */

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char *name, *p;
	char	*argv[20];
	int	i, argc;
	char	buffer[BUFSIZ];

	if (uuxstr) free (uuxstr);
	uuxstr = NULLCP;
	if (myhostname) free (myhostname);
	myhostname = NULLCP;

	name = qb2str (arg);

	if ((mychan = ch_nm2struct (name)) == NULLCHAN) {
		PP_OPER (NULLCP, ("Chan/uucp_out : Channel '%s' not known",name));
		if (name != NULL) free (name);
		return NOTOK;
	}

	if (mychan -> ch_out_info == NULLCP) {
		PP_OPER (NULLCP, ("%s channel (%s) has no info parameter",
				  mychan -> ch_name, mychan -> ch_show));
		return NOTOK;
	}
	(void) strcpy (buffer, mychan -> ch_out_info);
	if ((argc = sstr2arg (buffer, 20, argv, ",")) <= 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("Info string unparseable"));
		return NOTOK;
	}

	for (i = 0; i < argc; i++) {
		if ((p = index (argv[i], '=')) == NULL) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Info string for %s not in key=val format",
				 mychan -> ch_name));
			return NOTOK;
		}
		*p++ = '\0';
		switch (cmd_srch (argv[i], uucptbl)) {
		    case TBL_UUXCMD:
			uuxstr = strdup (p);
			break;

		    case TBL_HOST:
			myhostname = strdup (p);
			break;

		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown key in chan info for %s '%s'",
				 mychan -> ch_name,
				 argv[i]));
			return NOTOK;
		}
	}
	if (uuxstr == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, ("uux not set in info"));
		return NOTOK;
	}
	if (myhostname == NULLCP)
		myhostname = strdup (loc_dom_mta);

	(void) signal (SIGCHLD, SIG_DFL);
	if (name != NULL) free (name);
	return OK;
}

/*  */
/* routine to check if allowed to filter this message through this channel */
static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
 	char 		*msg_file = NULL, *msg_chan = NULL;
	int		result;

	result = TRUE;
	msg_file = qb2str (msg->qid);	
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp (msg_chan,mychan->ch_name) != 0)) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("Chans/shell channel err: '%s'",msg_chan));
		result = FALSE;
	}

	/* free all storage used */
	if (msg_file != NULL) free (msg_file);
	if (msg_chan != NULL) free (msg_chan);
	return result;
}

/*  */
/* routine called to do uucp */

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars	prm;
	Q_struct	que;
	ADDR		*sender = NULL;
	ADDR		*recips = NULL;
	int 		rcount, retval;
	struct type_Qmgr_UserList 	*ix;
	ADDR				*adr;
	char		*realfrom = NULL;
	bzero ((char *) &que,sizeof (que));
	bzero ((char *) &prm,sizeof (prm));

	delivery_init (arg->users);
	delivery_setall (int_Qmgr_status_messageFailure);
	first_successDR = TRUE;

	if (security_check (arg) != TRUE)
		return deliverystate;

	if (this_msg != NULL) free (this_msg);
	if (this_chan != NULL) free (this_chan);

	this_msg = qb2str (arg->qid);
	this_chan = qb2str (arg->channel);

	PP_LOG (LLOG_NOTICE,
	       ("%s processing msg '%s'",mychan->ch_name,this_msg));

	if (rp_isbad (rd_msg (this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s rd_msg err: '%s'",mychan->ch_name,this_msg));
		rd_end();
		return deliverystate;
	}

		
	if (getrealfrom (sender, &realfrom) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s unable to construct real sender message for orig of message '%s'",mychan->ch_name,this_msg));
		rd_end();
		return deliverystate;
	}

	if (arg->users == NULL)
		PP_LOG (LLOG_NOTICE,
			("%s : passed a NULL user list for message '%s'",
			mychan->ch_name,
			this_msg));

	/* check each recipient for processing */
	for (ix = arg->users; ix; ix = ix->next) {
		if ((adr = getnthrecip (&que,(int)ix->RecipientId->parm)) == NULL) {
			PP_LOG (LLOG_EXCEPTIONS,
			      ("%s : failed to find recipient %d of msg '%s'",mychan->ch_name,ix->RecipientId->parm, this_msg));
			delivery_set ((int)ix->RecipientId->parm, int_Qmgr_status_messageFailure);
			continue;
		}

		switch (chan_acheck (adr, mychan, 1, NULLVP)) {
		    case OK:
			if (processMsg (this_msg,adr, realfrom) == NOTOK) {
				PP_LOG (LLOG_EXCEPTIONS,
				       ("%s : failed to process msg '%s' for recip '%d' on channel '%s'",mychan->ch_name,this_msg, adr->ad_no, this_chan));
				delivery_set (adr->ad_no, int_Qmgr_status_messageFailure);
			} else {
				PP_LOG (LLOG_NOTICE,
					 ("%s : processed '%s' for recipient %d",mychan->ch_name,this_msg,adr->ad_no));
				set_success (adr, &que, this_msg);
			}
			break;
		    default:
			break;
		}
	}

	if (rp_isbad (retval = wr_q2dr (&que, this_msg))) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure '%d'",mychan->ch_name,retval));
		delivery_resetDRs (int_Qmgr_status_messageFailure);
	}

	rd_end();
	if (realfrom != NULL) free (realfrom);
	return deliverystate;
}

static void set_success (recip, que, msg)
ADDR		*recip;
Q_struct	*que;
char		*msg;
{
	if (recip->ad_usrreq == AD_USR_CONFIRM ||
	    recip->ad_mtarreq == AD_MTA_CONFIRM ||
	    recip->ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
		set_1dr (que, recip->ad_no, this_msg,
			 DRR_NO_REASON, -1, NULLCP);
		delivery_set (recip->ad_no, 
			     (first_successDR == TRUE) ? int_Qmgr_status_positiveDR : int_Qmgr_status_successSharedDR);
	} else {
		(void) wr_ad_status (recip, AD_STAT_DONE);
		(void) wr_stat (recip, que, this_msg, data_bytes);
		delivery_set (recip->ad_no, int_Qmgr_status_success);
	}
}

/*  */
static int processMsg (msg,recip, realfrom)
/* returns OK if managed to process msg for recip on mychan */
char	*msg;
ADDR 	*recip;
char	*realfrom;
{
	char 	*origdir = NULL,
		*cmdline = NULL;
	int result = OK;

	if (qid2dir (msg, recip, TRUE, &origdir) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s original directory not found for recipient %d of message '%s'",mychan->ch_name,recip->ad_no, msg));
		result = NOTOK;
	}

	if (result == OK &&
	    mychan->ch_out_info == NULL) {
		PP_OPER (NULLCP,
			 ("%s uux pathname not given in info string",mychan->ch_name));
		result = NOTOK;
	}

	if (result == OK && 
	    rfc8222uu (recip->ad_outchan->li_mta,recip->ad_r822adr, &cmdline) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s unable to convert to uucp format address of recip %d of message '%s'",mychan->ch_name,recip->ad_no, msg));
		result = NOTOK;
	}
		
	if ((result == OK) && (doProcess (origdir,msg, cmdline, realfrom) != OK))
		result = NOTOK;

	if (origdir != NULL) free (origdir);
	if (cmdline != NULL) free (cmdline);
	return result;
}

/*  */

static int sigpiped, sigalarmed;
static SFD alarmed(), piped();
static jmp_buf pipe_alrm_jmp;

static int doProcess (orig, msg, cmdline, realfrom)
/* processes orig directory contents through mychan */
char	*orig,	/* original directory */
	*msg,	/* message */
	*cmdline,/* uucp cmdline */
	*realfrom;/* real sender */
{
	int	fd[2],
		result = OK,
		pid,
		pgmresult,
		margc;
	char	*program = NULL,
		*margv[20];
	SFP	 oldalarm, oldpipe;
	VOLATILE int	killed = 0;
	struct stat statbuf;

	if (pipe (fd) != 0) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s pipe failed",mychan->ch_name));
		return NOTOK;
	}

	if ((margc = sstr2arg (cmdline, 20, margv, " \t")) < 1) 
		return NOTOK;
		
	if (margv[0][0] == '/')
		/* given full path name */
		program = strdup (margv[0]);
	else {
		program = (char *) malloc ((unsigned int) (strlen (chndfldir) + 1 + strlen (margv[0]) + 1));
		(void) sprintf (program,"%s/%s",chndfldir,margv[0]);
	}
	
	if (stat (program, &statbuf) != OK) {
		PP_OPER (NULLCP,
			 ("%s : missing uux program '%s'",mychan -> ch_name, program));
		return NOTOK;
	}
	if ((pid = tryfork()) == 0) {
		/* in child so redirect in- and out-put */
		(void) dup2 (fd[0], 0);
		(void) close (fd[0]);
		(void) close (fd[1]);
		(void) setpgrp ();

		(void) execv (program,margv);
		_exit (1);
	} else if (pid == -1) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s: tryfork failed for msg %s",
		       mychan->ch_name,this_msg));
		(void) close (fd[1]);
		result = NOTOK;
	} else {
#ifndef UNIONWAIT
		int w;
#else
		union wait w;
#endif

		oldalarm = signal (SIGALRM, alarmed);
		oldpipe = signal (SIGPIPE, piped);
		sigpiped = 0;
		sigalarmed = 0;
		data_bytes = 0;

		if (setjmp (pipe_alrm_jmp) != DONE) {
			if (send_to_child (orig, fd[1], realfrom) == NOTOK)
				result = NOTOK;
			(void) close (fd[1]);
		} else {
			if (sigalarmed)
				PP_TRACE (("alarm went off"));
			if (sigpiped)
				PP_TRACE (("pipe went off"));
		}

		if (sigpiped) { /* pipe died - reset for timeout */
			sigpiped = 0;
			if (setjmp (pipe_alrm_jmp) == DONE)
				PP_TRACE (("Timeout"));
		}

		if (sigalarmed) { /* alarm went off */
			PP_NOTICE (("Process taking too long ... killing"));
			killed = 1;
			(void) killpg (pid, SIGTERM);
			sleep (2);
			(void) killpg (pid, SIGKILL);
		}

#ifndef UNIONWAIT
		while ((pgmresult = wait (&w)) != pid &&
		       pgmresult != -1)
#else
		while ((pgmresult = wait (&w.w_status)) != pid &&
		       pgmresult != -1)
#endif
			PP_TRACE (("process %d returned", pgmresult));

		PP_TRACE (("pid %d returned term=%d, retcode=%d core = %d killed = %d",
			  pid, WTERMSIG(w), WEXITSTATUS(w),
			  WCOREDUMP(w), killed));
		(void) alarm (0);

		(void) signal (SIGPIPE, oldpipe);
		(void) signal (SIGALRM, oldalarm);

		if ((pgmresult == pid) && (WEXITSTATUS(w) == 0) && (killed == 0))
			result = OK;
		else 
			result = NOTOK;
	}
	(void) close (fd[0]);

	return result;
}

/* ARGSUSED */
static SFD alarmed(sig)
int sig;
{
	sigalarmed = 1;
	longjmp (pipe_alrm_jmp, DONE);
}

/* ARGSUSED */
static SFD piped(sig)
int sig;
{
	sigpiped = 1;
	longjmp (pipe_alrm_jmp, DONE);
}

/*  */
static int write_to_child (file, fd_out)
char	*file;
int	fd_out;
{
	char	buf[BUFSIZ];
	int	fd_in,
		num;

	if ((fd_in = open (file, O_RDONLY, 0666)) == -1) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s input file err '%s'",mychan->ch_name, file));
		return NOTOK;
	}

	while ((num = read (fd_in, buf, BUFSIZ)) > 0) {
		if (write (fd_out, buf, num) == -1) {
			close (fd_out);
			return NOTOK;
		}
		data_bytes += num;
	}
	close (fd_in);
	return OK;
}
		       
static ADDR *getnthrecip (que, num)
Q_struct	*que;
int		num;
{
	ADDR *ix = que->Raddress;

	if (num == 0)
		return que->Oaddress;
	while ((ix != NULL) && (ix->ad_no < num))
		ix = ix->ad_next;
	return ix;
}

static LIST_RCHAN *getnthchannel (chans,num)
LIST_RCHAN	*chans;
int		num;
{
	LIST_RCHAN	*ix = chans;
	int		icount = 0;

	while ((ix != NULL) && (icount++ < num)) 
		ix = ix->li_next;

	return ix;
}

static char	*get_adrstr (adr)
ADDR	*adr;
{
	char	*key;
	switch (adr->ad_type) {
	    case AD_X400_TYPE:
		key = adr->ad_r400adr;
		break;
	    case AD_822_TYPE:
		key = adr->ad_r822adr;
		break;
	    default:
		key = adr->ad_value;
		break;
	}
	return key;
}

static int is822hdr (file)
char	*file;
{
	char *ix = rindex (file, '/');

	if (ix == NULL
	    || (strncmp ((ix+1),hdr_822_bp,strlen (hdr_822_bp)) != 0))
		return FALSE;
	else
		return TRUE;
}

static int send_to_child (orig, fd, realfrom)
char	*orig;
int	fd;
char	*realfrom;
{
	char	file[MAXPATHLENGTH];
	SFP	old_sig;
	char	buf[BUFSIZ];
	time_t	timenow;
	int	len;

	(void) time (&timenow);
	
	msg_rinit (orig);

	pipe_signaled = FALSE;

	/* set signal handler */
	old_sig = signal (SIGPIPE, sig_pipe);

	/* set long jump */
	setjmp (toplevel);
	(void) sprintf (buf, "From %s %.24s remote from %s\n",
			realfrom, ctime (&timenow), myhostname);
	len = strlen (buf);
	if (write (fd, buf, len) != len)
		return NOTOK;
	data_bytes += len;

	while (pipe_signaled == FALSE
	       && msg_rfile (file) == RP_OK) {
		if (write_to_child (file, fd) == NOTOK) {
			msg_rend ();
			return NOTOK;
		}
		if (is822hdr (file) == TRUE) {
			if (write (fd, "\n", 1) != 1)
				return NOTOK;
			data_bytes ++;
		}
	}
	
	msg_rend();

	/* unset signal handler */
	(void) signal (SIGPIPE, old_sig);
	return OK;
}
