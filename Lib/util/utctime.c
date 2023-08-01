/* utctime: utctime functions */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/utctime.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/utctime.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: utctime.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/psap.h>

UTC	time_t2utc ();
time_t	utc2time_t ();
UTC	utcdup ();
time_t	time ();

UTC	time_t2utc(t)
time_t	t;
{
	struct tm *tm;
	struct UTCtime uts;

	tm = gmtime (&t);
	tm2ut (tm, &uts);
	return utcdup(&uts);
}


UTC 	utclocalise(utc)
UTC	utc;
{
	time_t	ti;
	struct tm	*tm;
	struct UTCtime	uts;

	ti = utc2time_t(utc);
	tm = localtime(&ti);
	tm2ut (tm, &uts);
#ifdef sun
	if (tm -> tm_gmtoff != 0) {
		uts.ut_flags |= UT_ZONE;
		uts.ut_zone = tm -> tm_gmtoff / 60;
	}
#endif
	return utcdup(&uts);
}

UTC	utcdup (utc)
UTC	utc;
{
	UTC	ut;

	ut = (UTC) smalloc (sizeof *ut);
	*ut = *utc;
	return ut;
}

time_t utc2time_t(utc)
UTC	utc;
{
	struct tm *tm;
	if (utc == NULLUTC)
		return (time_t) 0;
	if (utc -> ut_mon <= 0) utc -> ut_mon = 1; /* hack around isode bug */
	tm = ut2tm (utc);
	return gtime (tm);
}

UTC	utcnow ()
{
	time_t	now;

	(void) time (&now);
	return time_t2utc (now);
}

int utcequ (ut1, ut2)
UTC	ut1, ut2;
{
	return bcmp ((char *)ut1, (char *)ut2, sizeof *ut1);
}
