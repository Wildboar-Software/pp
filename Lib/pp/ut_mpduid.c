/* ut_mpduid.c: utilities for mpduid's */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_mpduid.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_mpduid.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_mpduid.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "mta.h"

void MPDUid_new (mp)
MPDUid	*mp;
{
	extern char	*loc_dom_mta;
	time_t		t, time();
	struct tm	*tp;
	char		tbuf[LINESIZE];

	PP_DBG (("Lib/pp/new_MPDUid()"));

	t = time((time_t *)0);
	tp = gmtime(&t);
	(void) sprintf(tbuf, "%.10s.%03d:%02d.%02d.%02d.%02d.%02d.%02d",
			loc_dom_mta, (getpid()%1000),
			tp->tm_mday, tp->tm_mon, (tp->tm_year%100),
			tp->tm_hour, tp->tm_min, tp->tm_sec);
	mp->mpduid_string = strdup(tbuf);
	GlobalDomId_new (&mp->mpduid_DomId);
}

void MPDUid_free (mp)
MPDUid *mp;
{
	if (mp == NULL)
		return;

	if (mp -> mpduid_string)
		free (mp -> mpduid_string);
	GlobalDomId_free (&mp -> mpduid_DomId);
}
