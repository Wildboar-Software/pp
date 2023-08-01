/* signals: catch some disatorous signals */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/siginit.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/siginit.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: siginit.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <signal.h>

#ifndef sigmask
#define sigmask(m) (1<< ((m) - 1))
#endif

static SFD onsig ();

void siginit ()
{
	int     mask;

	mask = sigblock (sigmask(SIGILL) |
			 sigmask(SIGFPE) |
			 sigmask(SIGBUS) |
			 sigmask(SIGSEGV) |
			 sigmask(SIGSYS) |
			 sigmask(SIGPIPE) );
	(void) signal (SIGILL, onsig);
	(void) signal (SIGFPE, onsig);
	(void) signal (SIGBUS, onsig);
	(void) signal (SIGSEGV, onsig);
	(void) signal (SIGSYS, onsig);
	(void) signal (SIGPIPE, onsig);
	(void) sigsetmask (mask);
}

/* ARGSUSED */
#ifdef SYS5
static SFD onsig (sig)
int sig;
#else
static SFD onsig (sig, code, context)
int     sig, code;
struct sigcontext *context;
#endif
{
#ifdef BSD43
	extern char *sys_siglist[];
#endif

	(void) signal (sig, SIG_DFL);    /* to stop recursion */
	(void) signal (SIGIOT, SIG_DFL); /* for abort */
	(void) signal (SIGILL, SIG_DFL); /* for abort too */
#ifdef SIGABRT
	(void) signal (SIGABRT, SIG_DFL);
#endif
	(void) sigsetmask (sigblock (0) & ~ (sigmask(SIGILL) | sigmask(SIGIOT)
#ifdef SIGABRT
					     | sigmask(SIGABRT)
#endif
					     ));
#ifndef BSD43
	PP_OPER (NULLCP,
		("Process dying on signal %d", sig));
#else
	PP_OPER (NULLCP,
		("Process dying on signal %d (%s)", sig, sys_siglist[sig]));
#endif
	(void) abort ();
	/* NOTREACHED */
}
