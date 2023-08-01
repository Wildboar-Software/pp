/* timeout.c: provide a timeout function */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/timeout.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/timeout.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: timeout.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include <signal.h>
#include "util.h"



/* __________________________________________________________________________


provide a timeout function for jobs that could potentially hang
Only one entry is given - timeout(value)
timeout(value) will be longjumped back to

____________________________________________________________________________*/




jmp_buf _timeobuf;
static SFP      oldalrm;

/* ARGSUSED */
SFD _tcatch(sig)
int sig;
{
	longjmp(_timeobuf,1);
}


void _timeout(val)
unsigned val;
{
	if(val)
		oldalrm = signal (SIGALRM, _tcatch);
	(void) alarm(val);
	if (!val)
		(void) signal (SIGALRM, oldalrm);
}
