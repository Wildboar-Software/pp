/* submit_srvr.c: submit running as a server */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_srvr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_srvr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: submit_srvr.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include <isode/tpkt.h>
#include <fcntl.h>
#include <signal.h>
#include <isode/internet.h>

extern char *submit_addr;
extern int  errno;

#ifdef BSD42
#include <sys/wait.h>

/* ARGSUSED */
static SFD	reaper (sig)
int sig;
{
#ifdef UNIONWAIT
	union wait status;
#else
	int status;
#endif

	while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0)
		continue;
}
#endif

int server_start (host, statusp)
char *host;
int *statusp;
{
	struct servent *sp;
	u_short port;
	int	sd;
	struct sockaddr_in s_in;
	int newsd, qfd;
	int argc;
	char *argv[20];
	char *subp;

	argc = sstr2arg (submit_addr, 50, argv, ",");
	if (argc <= 0 || (subp = index(argv[0], ':')) == NULL) {
		PP_OPER (NULLCP, ("Bad submit port specification"));
		exit (1);
	}
	*subp ++ = '\0';
		
	if (isdigit (*subp))
		port = htons ((u_short)atoi(subp));
        else if ((sp = getservbyname (subp, "tcp")) != NULL)
		port = sp -> s_port;
	else {
		PP_OPER (NULLCP, ("Can't locate port %s", subp));
		exit (1);
	}
	if (tryfork ()) exit(0);
	if ((sd = open ("/dev/null", O_RDWR)) != NOTOK) {
		if (sd != 0)
			(void) dup2 (sd, 0), (void) close (sd);
		(void) dup2 (0, 1);
		(void) dup2 (0, 2);
	}
#ifdef SETSID
	(void) setsid ();
#endif
#ifdef  TIOCNOTTY
{	int i;
        if ((i = open ("/dev/tty", O_RDWR)) != NOTOK) {
                (void) ioctl (i, TIOCNOTTY, NULLCP);
                (void) close (i);
        }
}
#else
#ifdef  SYS5
        (void) setpgrp ();
        (void) signal (SIGINT, SIG_IGN);
        (void) signal (SIGQUIT, SIG_IGN);
#endif
#endif
        isodexport (NULLCP);    /* re-initialize logfiles */

	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == NOTOK) {
		PP_OPER ("socket", ("Can't create "));
		exit (1);
	}

	bzero ((char *)&s_in, sizeof s_in);
	s_in.sin_family = AF_INET;
	s_in.sin_port = port;

	if (bind (sd, (struct sockaddr *)&s_in, sizeof s_in) == NOTOK ||
	    listen(sd, 5) == NOTOK) {
		PP_OPER ("bind/listen", ("failed to "));
		exit (1);
	}

#ifdef BSD42
	(void) signal (SIGCHLD, reaper);
#else
	(void) signal (SIGCLD, SIG_IGN);
#endif

	PP_NOTICE (("Submit started"));
	for (;;) {
		struct tsapblk *tb;
		qfd = qmgr_start (host, statusp, 0);

		PP_TRACE (("qmgr on ad %d, status = %d", qfd, *statusp));
		while ((newsd = accept (sd, (struct sockaddr *)0, NULLIP)) == NOTOK) {
			if (errno != EINTR) {
				PP_SLOG (LLOG_EXCEPTIONS, "accept",
					 ("problem with"));
				sleep (1);
			}
		}
		PP_TRACE (("new association on %d", newsd));
		switch (tryfork()) {
		    case NOTOK:
			(void) close (newsd);
			break;

		    case 0: /* child */
			isodexport (NULLCP);
			(void) dup2 (newsd, 0);
			(void) dup2 (newsd, 1);
			(void) close (newsd);
			(void) close (sd);
			ll_close (pp_log_stat);
			ll_close (pp_log_oper);
			ll_close (pp_log_norm);
			return qfd;

		    default:
			if ((tb = findtblk (qfd)) != (struct tsapblk *) NULL) {
				if ((*tb -> tb_closefnx) (tb -> tb_fd) == NOTOK)
					PP_LOG (LLOG_EXCEPTIONS,
						("close failed on %d",
						 tb -> tb_fd));

				PP_TRACE (("Closed qmgr %d", tb -> tb_fd));
				tb -> tb_fd = NOTOK;
				freetblk (tb);
			}
			if (close (newsd) == NOTOK)
				PP_LOG (LLOG_EXCEPTIONS, ("close failed on %d",
							  newsd));
			PP_TRACE (("Closed newsd %d", newsd));
			break;
		}
	}
}
