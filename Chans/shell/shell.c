/* shell.c: shell or program channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/shell/RCS/shell.c,v 6.0 1991/12/18 20:11:52 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/shell/RCS/shell.c,v 6.0 1991/12/18 20:11:52 jpo Rel $
 *
 * $Log: shell.c,v $
 * Revision 6.0  1991/12/18  20:11:52  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "expand.h"
#include "head.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "q.h"
#include "dr.h"
#include "prm.h"
#include "prog.h"
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

extern void	sys_init(), err_abrt(), rd_end();
extern struct type_Qmgr_DeliveryStatus	*delivery_resetDRs();
extern char 	*quedfldir;
extern char	*chndfldir;
extern char	*logdfldir;
extern char	*ad_getlocal();
extern CHAN 	*ch_nm2struct();
extern char	*expand_dyn ();
extern void	or_myinit ();
CHAN 		*mychan;
char		*this_msg = NULL, *this_chan = NULL;
static int	first_successDR, first_failDR, err_fatal = FALSE;
static int 	data_bytes;
static char	buf[BUFSIZ], *fatalbuf = NULLCP;

static struct type_Qmgr_DeliveryStatus *process ();
static int toProcess ();
static int processMsg ();
static int doProcess ();
static int write_to_child ();
static int send_to_child();
static int do_child_logging();
static void set_success();
static int initialise ();
static char *get_adrstr();
static int security_check ();
static void dirinit ();
static ADDR *getnthrecip ();
static int	pipe_signaled;
static jmp_buf	toplevel;

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
	sys_init(argv[0]);
	or_myinit();
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control(argc,argv,initialise,process, NULLIFP);
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
		PP_OPER(NULLCP, ("Chan/shell : Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}

	(void) signal (SIGCHLD, SIG_DFL);
	if (name != NULL) free(name);
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

	if ((mychan == NULLCHAN) || (strcmp(msg_chan,mychan->ch_name) != 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/shell channel err: '%s'",msg_chan));
		result = FALSE;
	}

	/* free all storage used */
	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	return result;
}

/*  */
/* routine called to do shell */

Expand expand_macros[] = {
	"SENDER", NULLCP,	/* 0 */
	"822SENDER", NULLCP,	/* 1 */
	"400SENDER", NULLCP,	/* 2 */
	"QID", NULLCP,		/* 3 */
	"UA-ID", NULLCP,	/* 4 */
	"P1-ID", NULLCP,	/* 5 */
	"RECIP", NULLCP,	/* 6 */
	"822RECIP", NULLCP,	/* 7 */
	"400RECIP", NULLCP,	/* 8 */
	"RECIPIENT", NULLCP,	/* 9 */
	"USERID", NULLCP,	/* 10 */
	"GROUPID", NULLCP,	/* 11 */
	"SIZE",	NULLCP,		/* 12 */
	"CHANNELNAME", NULLCP,	/* 13 */
	NULLCP, NULLCP};

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars	prm;
	Q_struct	que;
	char	sizebuf[20];
	char	uidbuf[20], gidbuf[20];
	ADDR		*sender = NULL;
	ADDR		*recips = NULL;
	int 		rcount, retval;
	struct type_Qmgr_UserList 	*ix;
	ADDR				*adr;
	static char		*local = NULLCP;
	char *key;
	ProgInfo	*info = NULLPROG;

	bzero((char *) &que,sizeof(que));
	bzero((char *) &prm,sizeof(prm));

	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_messageFailure);
	first_successDR = TRUE;
	first_failDR = TRUE;
	if (security_check(arg) != TRUE)
		return deliverystate;

	if (this_msg != NULL) free(this_msg);
	if (this_chan != NULL) free(this_chan);

	this_msg = qb2str(arg->qid);
	this_chan = qb2str(arg->channel);

	PP_LOG(LLOG_NOTICE,
	       ("%s processing msg '%s'",mychan->ch_name,this_msg));

	if (rp_isbad(rd_msg(this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s rd_msg err: '%s'",mychan->ch_name,this_msg));
		rd_end();
		return delivery_setallstate(int_Qmgr_status_messageFailure,
					    "Can't read message");
	}

	expand_macros[0].expansion = que.Oaddress->ad_value;
	expand_macros[1].expansion = que.Oaddress->ad_r822adr;
	expand_macros[2].expansion = que.Oaddress->ad_r400adr;
	expand_macros[3].expansion = this_msg;
	expand_macros[4].expansion = que.ua_id;
	expand_macros[5].expansion = que.msgid.mpduid_string;
	/* 6-11 filled in below */
	(void) sprintf (sizebuf, "%d", que.msgsize);
	expand_macros[12].expansion = sizebuf;
	expand_macros[13].expansion = mychan -> ch_name;

	if (arg->users == NULL)
		PP_LOG(LLOG_NOTICE,
			("%s : passed a NULL user list for message '%s'",
			mychan->ch_name,
			this_msg));

	/* check each recipient for processing */
	for (ix = arg->users; ix; ix = ix->next) {
		err_fatal = FALSE;
		if (fatalbuf != NULLCP) {
			free(fatalbuf);
			fatalbuf = NULLCP;
		}
		if ((adr = getnthrecip(&que,ix->RecipientId->parm)) == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s : failed to find recipient %d of msg '%s'",
				mychan->ch_name,ix->RecipientId->parm,
				this_msg));
			delivery_setstate(ix->RecipientId->parm, 
					  int_Qmgr_status_messageFailure,
					  "Unable to find specified recipient");
			continue;
		}
		buf[0] = '\0';
		key = get_adrstr(adr);
		if (local) {
			free (local);
			local = NULLCP;
		}
		if ((local = ad_getlocal(key, adr -> ad_type)) != NULLCP)
			info = tb_getprog(local, mychan->ch_table);

		if (info == NULLPROG)
			info = tb_getprog (key, mychan -> ch_table);
		
		if (info == NULLPROG) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("table lookup failed for '%s' in '%s'",
				key, mychan -> ch_table -> tb_name));
			(void) sprintf (buf,
					"Failed to find entry for '%s' in table '%s'",
					key, mychan -> ch_table -> tb_name);
			delivery_setstate(ix -> RecipientId -> parm,
				     int_Qmgr_status_messageFailure,
				     buf);
			continue;
		}

		expand_macros[6].expansion = adr -> ad_value;
		expand_macros[7].expansion = adr -> ad_r822adr;
		expand_macros[8].expansion = adr -> ad_r400adr;
		expand_macros[9].expansion = local;
		if (info -> username)
			expand_macros[10].expansion = info -> username;
		else {
			(void) sprintf (uidbuf, "%d", info -> uid);
			expand_macros[10].expansion = uidbuf;
		}
		(void) sprintf (gidbuf, "%d", info -> gid);
		expand_macros[11].expansion = gidbuf;

		
		switch (lchan_acheck (adr, mychan, 1, NULLVP)) {
		    default:
		    case NOTOK:
			break;
		    case OK:
			if (processMsg(this_msg, adr, info) == NOTOK) {
				if (err_fatal == TRUE) {
					/* DR it */
					set_1dr (&que, adr->ad_no, this_msg,
						 DRR_TRANSFER_FAILURE,
						 DRD_UA_UNAVAILABLE,
						 fatalbuf);
					delivery_set (adr->ad_no,
						      (first_failDR == TRUE) ? int_Qmgr_status_negativeDR : int_Qmgr_status_failureSharedDR);
					first_failDR = FALSE;
				} else {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("%s : failed to process msg '%s' for recip '%d' on channel '%s'",
						mychan->ch_name,this_msg, adr->ad_no, this_chan));
					delivery_setstate(adr->ad_no, 
							  int_Qmgr_status_messageFailure,
							  buf);
				}
			} else {
				PP_LOG(LLOG_NOTICE,
				       ("%s : processed '%s' for recipient %d",
					mychan->ch_name,this_msg,adr->ad_no));
				set_success(adr, &que);
			}
		}
		prog_free (info);
		info = NULLPROG;
	}
	if (fatalbuf != NULLCP) {
		free(fatalbuf);
		fatalbuf = NULLCP;
	}

	if (rp_isbad(retval = wr_q2dr(&que, this_msg))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure '%d'",mychan->ch_name,retval));
		delivery_resetDRs(int_Qmgr_status_messageFailure);
	}

	rd_end();
	return deliverystate;
}

static void set_success(recip, que)
ADDR		*recip;
Q_struct	*que;
{
	if (recip->ad_usrreq == AD_USR_CONFIRM ||
	    recip->ad_mtarreq == AD_MTA_CONFIRM ||
	    recip->ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
		set_1dr(que, recip->ad_no, this_msg,
			DRR_NO_REASON, -1, NULLCP);
		delivery_set(recip->ad_no, 
			     (first_successDR == TRUE) ?
			     int_Qmgr_status_positiveDR :
			     int_Qmgr_status_successSharedDR);
	} else {
		(void) wr_ad_status(recip, AD_STAT_DONE);
		(void) wr_stat (recip, que, this_msg, data_bytes);
		delivery_set(recip->ad_no, int_Qmgr_status_success);
	}
}

/*  */
static int toProcess (msg, recip)
/* returns true if should process msg for recip on mychan */
char	*msg;
ADDR 	*recip;
{
	LIST_RCHAN	*chan;
	if ((chan = recip->ad_outchan) == NULL) {
		PP_OPER(NULLCP,
			("%s : no outbound channel specified for recipient %d of msg '%s'",
			 mychan -> ch_name,
			 recip->ad_no,
			 msg));
		exit(1);
	}
			
	if (strcmp(chan->li_chan->ch_name, mychan->ch_name) == 0)
		return TRUE;
	else {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Channel mismatch for recip %d of msg '%s'",
			recip->ad_no,
			msg));
		return FALSE;
	}
}	

/*  */
static int processMsg (msg,recip, info)
/* returns OK if managed to process msg for recip on mychan */
char	*msg;
ADDR 	*recip;
ProgInfo *info;
{
	char 	*origdir = NULL;
	int result = OK;
	char	file[MAXPATHLENGTH];
	
	if (qid2dir(msg, recip, TRUE, &origdir) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s no message directory for recip %d of message '%s'",
			mychan->ch_name,recip->ad_no, msg));
		(void) sprintf(buf,
			       "Unable to find source directory");
		result = NOTOK;
	}

	if (info->solo == TRUE) {
		msg_rinit(origdir);
		while (result == OK && msg_rfile(file) == RP_OK)
			if (doProcess(file, info) != OK)
				result = NOTOK;
		msg_rend();
	} else {
		if ((result == OK) && (doProcess(origdir, info) != OK))
			result = NOTOK;
	}

	if (origdir != NULL) free(origdir);
	return result;
}

/*  */

static int sigpiped, sigalarmed;
static SFD alarmed(), piped();
static jmp_buf pipe_alrm_jmp;
static char env_home[LINESIZE], env_user[LINESIZE], env_shell[LINESIZE];
static char *env[] = {
	env_home, env_user, env_shell, NULLCP
	};

extern char *dupfpath ();
/* processes orig directory contents through mychan */
static int doProcess (orig, info)
char	*orig;	/* original directory */
ProgInfo *info;
{
	int	fd[2],
		fd_stderr[2],
		result = OK,
		pid,
		pgmresult;
	char	*expanded,
		*program = NULL,
		*margv[20];
	SFP	oldalarm, oldpipe;
	VOLATILE int	killed = 0;
	struct stat statbuf;

	if ((expanded = expand_dyn (info->cmd_line, expand_macros)) == NULLCP) {
		PP_OPER (NULLCP,
			("%s : failed to expand '%s'", info->cmd_line));
		(void) sprintf(buf,
			       "Failed to expand '%s'",
			       info -> cmd_line);
		return NOTOK;
	}

	PP_NOTICE(("%s",expanded));
	if (sstr2arg(expanded, 20, margv, " \t") < 1) {
		PP_OPER(NULLCP, ("%s : no arguments in info string",
			 mychan->ch_name));
		(void) sprintf (buf,
				"No arguments in info string '%s'",
				expanded);
		free(expanded);
		return NOTOK;
	}

	program = dupfpath (chndfldir, margv[0]);
	
	if (stat(program, &statbuf) != OK) {
		PP_OPER(NULLCP, ("%s : missing program '%s'",
			 mychan -> ch_name, program));
		(void) sprintf (buf,
				"Missing program '%s",
				program);
		free(expanded);
		return NOTOK;
	}
	(void) sprintf (env_home, "HOME=%s", info -> home);
	(void) sprintf (env_user, "USER=%s", info -> username ?
			info -> username : "anon");
	(void) sprintf (env_shell, "SHELL=%s", info -> shell);

	if (pipe(fd) != 0) {
		PP_SLOG(LLOG_EXCEPTIONS, "pipe",
		       ("pipe failed"));
		(void) sprintf(buf,
			       "Pipe failed");
		free(expanded);
		return NOTOK;
	}
	if (pipe(fd_stderr) != 0) {
		PP_SLOG(LLOG_EXCEPTIONS, "pipe",
		       ("stderr pipe failed"));
		free(expanded);
		(void) sprintf (buf,
				"standard error pipe failed");
		(void) close (fd[0]);
		(void) close (fd[1]);
		return NOTOK;
	}
	if ((pid = tryfork()) == 0) {
		/* in child so redirect input and stderr */
		(void) dup2 (fd[0], 0);
		(void) dup2 (fd_stderr[1], 1);
		(void) dup2 (fd_stderr[1], 2);
		(void) close(fd[0]);
		(void) close(fd[1]);
		(void) close(fd_stderr[0]);
		(void) close(fd_stderr[1]);
		(void) setpgrp();

		if(chdir (info -> home) == NOTOK)
			chdir ("/tmp");

		if (info -> username)
			(void) initgroups (info -> username, info -> gid);
#ifdef SYS5
		if (setgid (info->gid) == -1 ||
		    setuid (info->uid) == -1) {
#else
		if (setregid (info->gid, info->gid) == -1 ||
		    setreuid (info->uid, info->uid) == -1) {
#endif
			PP_SLOG (LLOG_EXCEPTIONS, "user",
				 ("Can't set uid/gid %d/%d",
				  info -> uid, info -> gid));
			_exit(1);
		}

		execve (program,margv, env);
		_exit (1);

	} else if (pid == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, this_msg,
		       ("tryfork failed"));
		(void) sprintf(buf,
			       "Tryfork failed");
		(void) close(fd_stderr[1]);
		(void) close(fd[1]);
		(void) close(fd_stderr[0]);
		(void) close(fd[0]);
		result = NOTOK;
	} else {
#ifndef UNIONWAIT
		int w;
#else
		union wait w;
#endif
		
		(void) close(fd_stderr[1]);
		(void) close(fd[0]);
		
		data_bytes = 0;
		oldalarm = signal(SIGALRM, alarmed);
		oldpipe = signal(SIGPIPE, piped);
		sigpiped = 0;
		sigalarmed = 0;
		
		if (info -> timeout_in_secs > 0) {
			alarm(info -> timeout_in_secs);
			PP_TRACE(("alarm set for %d secs",
				  info -> timeout_in_secs));
		}
		if (setjmp(pipe_alrm_jmp) != DONE) {
			send_to_child(orig, fd[1], info->solo);
			(void) close(fd[1]);
		} else {
			if (sigalarmed)
				PP_TRACE(("alarm went off"));
			if (sigpiped)
				PP_TRACE(("pipe went off"));
		}

		if (sigpiped) { /* pipe died - reset for timeout */
			sigpiped = 0;
			if (setjmp (pipe_alrm_jmp) == DONE)
				PP_TRACE(("Timeout"));
		}

		if (sigalarmed) { /* alarm went off */
			PP_NOTICE (("Process taking too long ... killing"));
			(void) sprintf(buf,
				       "Process took to long - killed");
			killed = 1;
			(void) killpg (pid, SIGTERM);
			sleep(2);
			(void) killpg (pid, SIGKILL);
		}
		do_child_logging (fd_stderr[0]);

		while ((pgmresult = wait(&w)) != pid && pgmresult != -1)
			PP_TRACE(("process %d returned", pgmresult));

		alarm(0);

		PP_TRACE(("pid %d term=%d retcode=%d core=%d killed=%d",
			  pid, WTERMSIG(w), WEXITSTATUS(w),
			  WCOREDUMP(w), killed));

		(void) signal (SIGPIPE, oldpipe);
		(void) signal (SIGALRM, oldalarm);
		do_child_logging(fd_stderr[0]);
		(void) close(fd[1]);
		(void) close (fd_stderr[0]);
		
		if (pgmresult == pid && !killed && WIFEXITED(w) && rp_gbval(WEXITSTATUS(w)) == RP_BNO)
			err_fatal = TRUE;

		if ((pgmresult == pid) && (WEXITSTATUS(w) == 0) && (killed == 0))
			result = OK;
		else 
			result = NOTOK;
	}

	free(expanded);
	return result;
}

static int do_child_logging(fd)
int	fd;
{
	fd_set	rfds, ifds;
	char	buf[80];	/* smallish */
	int	n;
	struct timeval timer;
	char	*newfatal;
	timerclear(&timer);

	PP_TRACE (("suckuperrors (%d)", fd));
	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);

#define the_universe_exists	1

	while (the_universe_exists) {
		ifds = rfds;

		if (select (fd + 1, &ifds, NULLFD, NULLFD, &timer) <= 0)
			break;

		PP_TRACE (("select fired"));
		if (FD_ISSET (fd, &ifds)) {
			if((n = read (fd, buf, sizeof buf - 1)) > 0) {
				buf[n] = 0;
				PP_LOG (LLOG_EXCEPTIONS,
					("Output from process '%s'", buf));
				if (fatalbuf == NULLCP)
					fatalbuf = strdup(buf);
				else {
					newfatal = malloc(strlen(fatalbuf) + strlen(buf) + 1);
					strcat (newfatal, fatalbuf);
					strcat (newfatal, buf);
					free(fatalbuf);
					fatalbuf = newfatal;
				}
			}
			else break;
		}
		else	break;
	}
}

static SFD alarmed(sig)
int sig;
{
	sigalarmed = 1;
	longjmp(pipe_alrm_jmp, DONE);
}

static SFD piped (sig)
int sig;
{
	sigpiped = 1;
	longjmp(pipe_alrm_jmp, DONE);
}

/*  */
static int write_to_child(file, fd_out)
char	*file;
int	fd_out;
{
	char	buf[BUFSIZ];
	int	fd_in,
		num;

	if ((fd_in = open(file, O_RDONLY, 0666)) == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, file,
		       ("Can't open file"));
		return NOTOK;
	}

	while ((num = read(fd_in, buf, BUFSIZ)) > 0) {
		if (write (fd_out, buf, num) == -1) {
			(void) close(fd_in);
			return NOTOK;
		}
		data_bytes += num;
	}
	(void) close(fd_in);
	return OK;
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


static char	*get_adrstr(adr)
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

static int is822hdr(file)
char	*file;
{
	char *ix = rindex(file, '/');

	if (ix == NULL
	    || (strncmp((ix+1),"hdr.822",strlen("hdr.822")) != 0))
		return FALSE;
	else
		return TRUE;
}

static int send_to_child(orig, fd, solo)
char	*orig;
int	fd;
int	solo;
{
	int	result = OK;
	char	file[MAXPATHLENGTH];
	SFP	old_sig;
		
	pipe_signaled = FALSE;

	/* set signal handler */
	old_sig = signal (SIGPIPE, sig_pipe);

	/* set long jump */
	setjmp(toplevel);

	if (solo == TRUE)
		result = write_to_child(orig, fd);
	else {
		msg_rinit(orig);

		while (pipe_signaled == FALSE
		       && msg_rfile(file) == RP_OK) {
			write_to_child(file, fd);
			if (is822hdr(file) == TRUE) {
				write(fd, "\n", 1);
				data_bytes ++;
			}
		}

		msg_rend();
	}
	/* unset signal handler */
	signal (SIGPIPE, old_sig);

	return result;
}	
