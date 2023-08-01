/* smtpd: daemon for smtp */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/smtp/RCS/smtpd.c,v 6.0 1991/12/18 20:12:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/smtp/RCS/smtpd.c,v 6.0 1991/12/18 20:12:19 jpo Rel $
 *
 * $Log: smtpd.c,v $
 * Revision 6.0  1991/12/18  20:12:19  jpo
 * Release 6.0
 *
 */



/*
 * new version - detects if it is running under inetd.
 */


#include "util.h"
#include <signal.h>
#include "sys.file.h"
#ifdef  SYS5
#ifdef vfork
#undef vfork
#endif
#include <unistd.h>
#endif
#include <sys/signal.h>
#include <isode/internet.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#ifdef SYS5
#include <sys/resource.h>
#endif

static char usage[] = "usage: smptd [-p port] [-i maxconn] \
program channel";

static int smtpport;
static int	maxconn = 5;
static struct sockaddr_in rmtaddr, laddr;
static int	numconnections = 0;
static char thishost[LINESIZE];
static char *Channel, Smtpserver[BUFSIZ];
static char *nstimeout;

static void envinit ();
static SFD reaper ();

#if sparc && defined(__GNUC__)	/* work around bug in gcc 1.37 sparc version */
#define inet_ntoa myinet_ntoa

static char *myinet_ntoa (in)
struct in_addr in;
{
	static char buf[80];

	(void) sprintf (buf, "%d.%d.%d.%d",
			(in.s_addr >> 24) & 0xff,
			(in.s_addr >> 16) & 0xff,
			(in.s_addr >> 8 ) & 0xff,
			(in.s_addr	) & 0xff);
	return buf;
}
#else
extern char	*inet_ntoa ();
#endif

extern int errno;

char	*myname;
int	debug;

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	struct servent *sp;
	int	alen, i, skt;

	myname = argv[0];
	sys_init (myname);

	if ((sp = getservbyname ("smtp", "tcp")) == NULL)
		smtpport = htons (25);
	else	smtpport = sp -> s_port;

	while((opt = getopt(argc, argv, "p:i:t:")) != EOF)
		switch (opt) {
		    case 'i':
			maxconn = atoi (optarg);
			break;
		    case 'p':
			smtpport = htons ((short)atoi (optarg));
			break;
		    case 't':
			nstimeout = optarg;
			break;
		    default:
			fprintf (stderr, "bad switch -%c: %s\n", opt, usage);
			PP_OPER (NULLCP, ("Bad argument -%c: %s", opt, usage));
			exit (1);
		}
	argc -= optind;
	argv += optind;

	initialise (argc, argv);

	alen = sizeof rmtaddr;
	if (getpeername (0, (struct sockaddr *)&rmtaddr,
			 &alen) == 0) { /* under inetd */
		PP_TRACE (("Running under inetd..."));
		doit (0);
		_exit (0);
	}

	envinit ();

	laddr.sin_family = AF_INET;
	laddr.sin_addr.s_addr = INADDR_ANY;
	laddr.sin_port = smtpport;

	for (i = 0; i < 10; sleep ((unsigned) 2 * ++i)) {
		if ((skt = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
			PP_SLOG (LLOG_EXCEPTIONS, "socket",
				 ("can't create "));
			continue;
		}

		if (bind (skt, (struct sockaddr *)&laddr,
			  sizeof laddr) == -1) {
			PP_SLOG (LLOG_EXCEPTIONS, "bind",
				 ("socket can't "));
			(void) close (skt);
			continue;
		}

		if (listen (skt, 5) == -1) {
			PP_SLOG (LLOG_EXCEPTIONS, "listen",
				 ("socket can't "));
			(void) close (skt);
			continue;
		}
		break;
	}

	PP_NOTICE (("%s listening on port %d", myname, ntohs(smtpport)));
#define tcp_still_alive_and_kicking 1

	while (tcp_still_alive_and_kicking) {
		int	snew;
		int	pid;

		alen = sizeof (rmtaddr);
		if ((snew = accept (skt, (struct sockaddr *)&rmtaddr,
				    &alen)) == NOTOK) {
			if (errno != EINTR)
				PP_SLOG (LLOG_EXCEPTIONS, "failed",
					 ("accept "));
			continue;
		}
		PP_TRACE (("Accepted new connection from %s",
			   inet_ntoa (rmtaddr.sin_addr)));
		if ((pid = tryfork ()) == NOTOK) {
			PP_SLOG (LLOG_EXCEPTIONS, "failed", ("fork "));
			sleep (10);
			continue;
		}
		if (pid == 0) {	/* child */
			doit (snew);
			_exit(0);
		}
		(void) close (snew);

		numconnections ++;

		while (numconnections >= maxconn) {
			PP_TRACE (("Too many connections -- waiting"));
#ifdef SYS5
			reaper(0);
#else
			sigpause (0);
#endif
			PP_TRACE (("Finished waiting"));
		}
	}
}

/* ARGSUSED */
static SFD reaper (sig)
int sig;
{
#ifndef WNOHANG
	int status;
	alarm(2);
	while (wait(&status) != -1)
		numconnections --;
	alarm (0);
#else
#ifdef SYS5
	int status;
#else
	union wait status;
#endif
	while (wait3 (&status, WNOHANG, (caddr_t)0) > 0)
		numconnections --;
#endif
}

#ifdef SYS5
SFD alarmtrap (sig)
int sig;
{
	(void) signal (SIGALRM, alarmtrap);
}
#endif

doit (fd)
int	fd;
{
	char *argv[10];
	int argc = 0;

	if (fd != 0)
		(void) dup2 (fd, 0);
	if (fd != 1)
		(void) dup2 (fd, 1 );
	if (fd != 2)
		(void) dup2 (fd, 2 );
	if (fd > 2 )
		(void) close (fd);
	argv[argc++] = Channel; /* argv[0] */
	if (nstimeout) {
		argv[argc++] = "-t";
		argv[argc++] = nstimeout;
	}
	argv[argc++] = Channel;
	argv[argc] = NULLCP;
	execv (Smtpserver, argv);
	PP_OPER (Smtpserver, ("server exec "));
	_exit (99);
}

static void  envinit () 
{
	int     i,
		nbits,
		sd;

	nbits = getdtablesize ();

	if (!(debug = isatty (2))) {
		for (i = 0; i < 5; i++) {
			switch (fork ()) {
			    case NOTOK: 
				sleep (5);
				continue;

			    case OK: 
				break;

			    default: 
				_exit (0);
			}
			break;
		}

		if ((sd = open ("/dev/null", O_RDWR)) == NOTOK) {
			PP_OPER ("/dev/null", ("unable to read"));
			exit (3);
		}
		if (sd != 0)
			(void) dup2 (sd, 0), (void) close (sd);
		(void) dup2 (0, 1);
		(void) dup2 (0, 2);

#ifdef  TIOCNOTTY
		if ((sd = open ("/dev/tty", O_RDWR)) != NOTOK) {
			(void) ioctl (sd, TIOCNOTTY, NULLCP);
			(void) close (sd);
		}
#else
#ifdef  SYS5
		(void) setpgrp ();
		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGQUIT, SIG_IGN);
#endif
#endif
	}
	else
		ll_dbinit (pp_log_norm, myname);

#ifdef notdef			/* damn YP... */
	for (sd = 3; sd < nbits; sd++)
		(void) close (sd);
#endif

	(void) signal (SIGPIPE, SIG_IGN);

	ll_hdinit (pp_log_norm, myname);
#ifndef SYS5
	(void) signal (SIGCHLD, reaper);
#endif

	PP_TRACE (("starting"));
}

initialise (argc, argv)
int	argc;
char	**argv;
{
	int	i;
	struct hostent *hp;
	extern char *chndfldir;

	if (argc > 0)
	{
		getfpath (chndfldir, argv[0], Smtpserver);

		if (access (Smtpserver, X_OK) < 0 ) { /* execute privs? */
			PP_OPER (Smtpserver, ("Cannot access server program"));
			fprintf (stderr, "Cannot access server program %s\n",
				 Smtpserver);
			exit (99);
		}
		PP_TRACE (("server is '%s'", Smtpserver ));
	} else {
		fprintf (stderr, "Smtpserver program not specified! (%s)\n",
			 usage);
		PP_OPER (NULLCP, ("Smtpserver program not specified! (%s)",
				  usage));
		exit (99);
	}

	if( argc > 1 ) {
		Channel = argv[1];
		PP_TRACE (("channel is '%s'", Channel));
	} else {
		PP_OPER (NULLCP, ("Channel not specified! (%s)", usage));
		fprintf (stderr, "Channel not specified! (%s)\n", usage);
		exit (99);
	}

        /*
         * must get full name for this host from nameserver (or file)
         */

	if (gethostname (thishost, sizeof thishost) == -1)
		PP_SLOG (LLOG_EXCEPTIONS, "gethostname",
			 ("Can't find out who I am"));

        for(i = 0 ; i < 10 ; i++){
                hp = gethostbyname(thishost);
                if(hp != NULL)
                        break;
                if(i > 2)
                        sleep((unsigned)i*2);
        }
        if(hp == NULL) {
		PP_OPER (NULLCP, 
                  ("Cannot find 'official' name of host '%s'\n",
						thishost));
                fprintf(stderr,
		  "Cannot find 'official' name of host '%s'\n",
                                                   thishost);
                exit(-1);
        }
        (void) strcpy(thishost, hp->h_name);
#ifdef SYS5
	(void) signal (SIGALRM, alarmtrap);
#endif
}
