/* timer.c: utilities to provide timing information */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/timer.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/timer.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: timer.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <sys/time.h>

#define NBBY 	8		/* no. bits / byte */

static void tvsub ();

void timer_start (tv)
struct timeval *tv;
{
#ifdef UNICORN
	(void) gettimeofday (tv);
#else
	(void) gettimeofday (tv, (struct timezone *)0);
#endif
}

void timer_end (tv, cc, string)
struct timeval *tv;
int	cc;
char	*string;
{
	long                    ms;
	float                   bs;
	struct timeval stoptime, td;
#ifdef UNICORN	/* funny gettimeofday */
	(void) gettimeofday (&stoptime);
#else
	(void) gettimeofday (&stoptime, (struct timezone *)0);
#endif

	tvsub (&td, &stoptime, tv);
	ms = (td.tv_sec * 1000) + (td.tv_usec / 1000);
	bs = (((float) cc * NBBY * 1000) / (float) (ms ? ms : 1)) / NBBY;

	PP_NOTICE (("%s: %d bytes in %d.%02d seconds (%.2f Kbytes/s)",
		    string, cc, td.tv_sec, td.tv_usec / 10000, bs / 1024));
}

static void tvsub (tdiff, t1, t0)
register struct timeval *tdiff,
			*t1,
			*t0;
{
	tdiff -> tv_sec = t1 -> tv_sec - t0 -> tv_sec;
	tdiff -> tv_usec = t1 -> tv_usec - t0 -> tv_usec;
	if (tdiff -> tv_usec < 0)
		tdiff -> tv_sec--, tdiff -> tv_usec += 1000000;
}

	
