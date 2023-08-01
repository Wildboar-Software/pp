/* pptsapd.c: pp version of the tsap daemon */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/pptsapd/RCS/pptsapd-stat.c,v 6.0 1991/12/18 20:27:31 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/pptsapd/RCS/pptsapd-stat.c,v 6.0 1991/12/18 20:27:31 jpo Rel $
 *
 * $Log: pptsapd-stat.c,v $
 * Revision 6.0  1991/12/18  20:27:31  jpo
 * Release 6.0
 *
 */



#include <errno.h>
#include <signal.h>
#include "util.h"
#include "chan.h"
#include <varargs.h>
#include <isode/manifest.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "sys.file.h"
#ifndef X_OK
#define X_OK    1
#endif
#include <sys/wait.h>
#include <sys/resource.h>

#include <isode/tpkt.h>

#ifdef  TCP
#include <isode/internet.h>
#endif
#ifdef  X25
#include <isode/x25.h>
#endif
#ifdef  CONS
#include <isode/cons.h>
#endif
#ifdef  TP4
#include <isode/tp4.h>
#endif
#include <isode/isoservent.h>
#include <isode/logger.h>
#include <isode/tailor.h>

/*  */

static int  debug = 1;
static int  nbits = FD_SETSIZE;

static char *myname = "pptsapd";
static char myhost[BUFSIZ];

#define NTADDRS         FD_SETSIZE

static struct TSAPaddr *tz;


static void    adios (), advise ();

static char *getchanpgm ();
static void tsapd ();
static void ts_adios ();
static void ts_advise ();
static void arginit ();
static void envinit ();
static SFD childserver ();
static int childhit;

extern int  errno;
extern char	*pptsapd_addr;
/*  */

/* ARGSUSED */

main (argc, argv, envp)
int     argc;
char  **argv,
      **envp;
{
    int	   vecp;
    char   *vec[4];
    register struct NSAPaddr  *na;
    struct TSAPdisconnect   tds;
    register struct TSAPdisconnect  *td = &tds;

    arginit (argv);
    envinit ();

    for (na = tz -> ta_addrs; na < &tz->ta_addrs[tz->ta_naddr]; na++) {
	    switch (na -> na_type) {
		case NA_TCP:
		    advise (LLOG_NOTICE, NULLCP, "listening on TCP %s %d",
			    na2str (na), ntohs (na -> na_port));
		    break;

		case NA_X25:
		    advise (LLOG_NOTICE, NULLCP, "listening on X.25 %s %s",
			    na2str (na),
			    sel2str (na -> na_pid, na -> na_pidlen, 1));
		    break;

		case NA_BRG:
		    advise (LLOG_NOTICE, NULLCP,
			    "listening on X.25 (BRIDGE) %s %s", na2str (na),
			    sel2str (na -> na_pid, na -> na_pidlen, 1));
		    break;

		case NA_NSAP:
		    advise (LLOG_NOTICE, NULLCP, "listening on NS %s",
			    na2str (na));
		    break;

		default:
		    adios (NULLCP, "unknown network type 0x%x", na -> na_type);
		    /* NOTREACHED */
	    }
    }
    (void) TNetAccept (&vecp, vec, 0, NULLFD, NULLFD, NULLFD, OK, td);

    (void) signal (SIGCHLD, childserver);

    if (tz -> ta_selectlen)
	advise (LLOG_NOTICE, NULLCP, "   %s",
			sel2str (tz -> ta_selector, tz -> ta_selectlen, 1));
    if (TNetListen (tz, td) == NOTOK)
         ts_adios (td, LLOG_EXCEPTIONS, "listen failed");

    for (;;) {
        childhit = 0;
	if (TNetAccept (&vecp, vec, 0, NULLFD, NULLFD, NULLFD, NOTOK, td)
		== NOTOK) {
		if (childhit == 0)
			ts_advise (td, LLOG_EXCEPTIONS, "accept failed");
		continue;
	}

	if (vecp <= 0)
	    continue;

	switch (TNetFork (vecp, vec, td)) {
	    case OK:
		tsapd (vecp, vec);
		exit (1);
		/* NOTREACHED */

	    case NOTOK:
		ts_advise (td, LLOG_EXCEPTIONS, "fork failed");
	    default:
		break;
	}
    }
}

/*  */

static char buffer1[4096];
static char buffer2[32768];


static void  tsapd (vecp, vec)
int     vecp;
char  **vec;
{
    char    buffer[BUFSIZ];
    char    buf2[BUFSIZ];
    struct TSAPstart   tss;
    register struct TSAPstart *ts = &tss;
    struct TSAPdisconnect   tds;
    register struct TSAPdisconnect  *td = &tds;
    CHAN	*chan;
    char	*program;

/* begin UGLY */
    (void) strcpy (buffer1, vec[1]);
    (void) strcpy (buffer2, vec[2]);
/* end UGLY */

    if (TInit (vecp, vec, ts, td) == NOTOK) {
	ts_advise (td, LLOG_EXCEPTIONS, "T-CONNECT.INDICATION");
	return;
    }

    advise (LLOG_NOTICE, NULLCP,
	    "T-CONNECT.INDICATION: <%d, <%s, %s>, <%s, %s>, %d, %d>",
	    ts -> ts_sd,
	    na2str (ts -> ts_calling.ta_addrs),
	    sel2str (ts -> ts_calling.ta_selector,
		     ts -> ts_calling.ta_selectlen, 1),
	    na2str (ts -> ts_called.ta_addrs),
	    sel2str (ts -> ts_called.ta_selector,
		     ts -> ts_called.ta_selectlen, 1),
	    ts -> ts_expedited, ts -> ts_tsdusize);

     if (ts -> ts_called.ta_selectlen == 0) {
	(void) sprintf (buffer, "No TSEL present");
	goto out;
    }
    if ((chan = ch_nm2struct (ts -> ts_called.ta_selector)) == NULL) {
    	(void) sprintf (buffer, "PP Service %s not found",
		ts -> ts_called.ta_selector);
	goto out;
    }
    (void) strcpy (buf2, chan -> ch_progname);
    vecp = sstr2arg (buf2, 20, vec, " \t");
    if (vecp <= 0)
	    goto out;
    if ((program = getchanpgm (vec[0])) == NULLCP) {
    	(void) sprintf (buffer, "Can't locate program %s", vec[0]);
	goto out;
    }
    vec[vecp] = buffer1;
    vec[vecp+1] = buffer2;
    vec[vecp+2] = NULLCP;
    (void) execv (program, vec);
    (void) sprintf (buffer, "unable to exec %s (%d args): %s", program,
		 vecp + 2, sys_errname (errno));
    PP_OPER (NULLCP, ("%s", buffer));
out: ;
    advise (LLOG_EXCEPTIONS, NULLCP, "%s", buffer);
    if (strlen (buffer) >= TD_SIZE)
	buffer[0] = NULL;
    (void) TDiscRequest (ts -> ts_sd, buffer, strlen (buffer) + 1, td);

    exit (1);
}

static char	*getchanpgm (pgm)
char	*pgm;
{
	char	buf[BUFSIZ];
	static char 	name[BUFSIZ];
	extern char *cmddfldir, *chndfldir;

	getfpath (cmddfldir, chndfldir, buf);
	getfpath (buf, pgm, name);

	return name;
}
/*  */

static void  ts_adios (td, code, event)
register struct TSAPdisconnect *td;
int	code;
char	*event;
{
	ts_advise (td, code, event);

	exit (1);
}

static void  ts_advise (td, code, event)
register struct TSAPdisconnect *td;
int     code;
char   *event;
{
    char    buffer[BUFSIZ];

    if (td -> td_cc > 0)
	(void) sprintf (buffer, "[%s] %*.*s",
		TErrString (td -> td_reason),
		td -> td_cc, td -> td_cc, td -> td_data);
    else
	(void) sprintf (buffer, "[%s]", TErrString (td -> td_reason));

    advise (code, NULLCP, "%s: %s", event, buffer);
}

/*  */

static void arginit (vec)
char    **vec;
{
    register char  *ap;

    if (myname = rindex (*vec, '/'))
	myname++;
    if (myname == NULL || *myname == NULL)
	myname = *vec;

    sys_init (myname);
    isodetailor (myname, 0);

    (void) strcpy (myhost, TLocalHostName ());

    tz = str2taddr (pptsapd_addr);
    for (vec++; ap = *vec; vec++) {
	if (*ap == '-')
	    switch (*++ap) {
		case 'c': 
		    if ((ap = *++vec) == NULL || *ap == '-')
			adios (NULLCP, "Usage: %s -c address", myname);
		    tz = str2taddr (*vec);
		    break;
		default:
		   adios (NULLCP, "Usage: %s [-c address]", myname);
	    }
    }
}

static void envinit () {
    int     i,
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

	(void) chdir ("/");

	if ((sd = open ("/dev/null", O_RDWR)) == NOTOK)
	    adios ("/dev/null", "unable to read");
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
	pp_log_norm -> ll_stat |= LLOGTTY;

#ifndef sun             /* damn YP... */
    for (sd = 3; sd < nbits; sd++)
	(void) close (sd);
#endif

    (void) signal (SIGPIPE, SIG_IGN);

    ll_hdinit (pp_log_norm, myname);
    advise (LLOG_NOTICE, NULLCP, "starting");
}

/*    ERRORS */

#ifndef lint
static void    adios (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _ll_log (pp_log_norm, LLOG_FATAL, ap);

    va_end (ap);

    _exit (1);
}
#else
/* VARARGS */

static void    adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif


#ifndef lint
static void    advise (va_alist)
va_dcl
{
    int     code;
    va_list ap;

    va_start (ap);

    code = va_arg (ap, int);

    _ll_log (pp_log_norm, code, ap);

    va_end (ap);
}
#else
/* VARARGS */

static void    advise (code, what, fmt)
char   *what,
       *fmt;
int     code;
{
    advise (code, what, fmt);
}
#endif

static SFD childserver (sig, code, sc)
int	sig;
long	code;
struct sigcontext *sc;
{
	union wait status;
	struct rusage rusage;
	int	pid;

	childhit = 1;
	while ((pid = wait3 (&status, WNOHANG, &rusage)) > 0) {
		PP_NOTICE (("Pid %d: Resource usage ut %d.%d st %d.%d mrss %d \
ixrss %d idrss %d isrss %d minflt %d majflt %d nswap %d ib %d ob %d ms %d mr %d ns %d nvcs %d nics %d",
			    pid,
			    rusage.ru_utime.tv_sec, rusage.ru_utime.tv_usec,
			    rusage.ru_stime.tv_sec, rusage.ru_stime.tv_usec,
			    rusage.ru_maxrss, rusage.ru_ixrss, rusage.ru_idrss,
			    rusage.ru_isrss, rusage.ru_minflt, rusage.ru_majflt,
			    rusage.ru_nswap, rusage.ru_inblock,
			    rusage.ru_oublock, rusage.ru_msgsnd,
			    rusage.ru_msgrcv, rusage.ru_nsignals,
			    rusage.ru_nvcsw, rusage.ru_nivcsw));
	}
}

		
