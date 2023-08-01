/* UTC2rfc.c - Converts a UTC struct into an RFC string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/UTC2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/UTC2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: UTC2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/cmd_srch.h>
#include <sys/time.h>


#define UYEAR(x)        ((x) >= 100 ? (x) - 1900 : (x))
#define YEAR(x)         ((x) >= 100 ? (x) : (x) + 1900)
#define dysize(x)       (((x) % 4) ? 365 : (((x) % 100) ? 366 : \
					    (((x) % 400) ? 365 : 366)))

static char  *Day []  = {
	"Sat",
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri"
	};

extern int dmsize[];
extern CMD_TABLE        tbl_month[];
extern time_t time ();
extern	int abs ();
static int makewkday ();


/* ---------------------  Begin  Routines  -------------------------------- */



/*
This routine produces a RFC 822 standard time string like:
	"Tue, 16 Aug 88 10:23:21 BST"
from a UTC structure.
*/


int	UTC2rfc (utc, buffer)
UTC     utc;
char    *buffer;
{
	char    zone[10];
	char secs[10];
	int     wday;

	*buffer = '\0';

	if (utc == NULLUTC) {
		PP_LOG (LLOG_EXCEPTIONS, ("NULL UTC time"));
		return NOTOK;
	}
	
	if (utc -> ut_mon <= 0 || utc -> ut_mon > 12) {
		PP_LOG (LLOG_EXCEPTIONS, ("Lib/UTC2rfc zero UTC specified"));
		return NOTOK;
	}

	if (utc -> ut_flags & UT_ZONE)
		(void) sprintf (zone, "%c%02d%02d",
				utc -> ut_zone < 0 ? '-' : '+',
				abs(utc -> ut_zone / 60),
				abs(utc -> ut_zone % 60));
	else
		(void) strcpy (zone, "+0000");

	if (utc -> ut_flags & UT_SEC)
		(void) sprintf (secs, ":%02d", utc -> ut_sec);
	else	secs[0] = 0;

	wday = makewkday (utc);
	if (wday < 0 || wday > 6) {
		PP_LOG (LLOG_EXCEPTIONS, ("Bad weekday calculated %d", wday));
		return NOTOK;
	}

	(void) sprintf (buffer, "%s, %d %s %d %02d:%02d%s %s",
			Day [wday],
			utc -> ut_mday,
			rcmd_srch (utc -> ut_mon - 1, tbl_month),
			YEAR(utc -> ut_year),
			utc -> ut_hour,
			utc -> ut_min,
			secs,
			zone);

	PP_DBG (("Lib/UTC2rfc returns (%s)", buffer));

	return OK;
}

static int makewkday (ut)
UTC     ut;
{
	int     year;
	int     mon;
	int     d;

	mon = ut -> ut_mon;
	year = YEAR (ut -> ut_year);
	d = 4 + year + (year+3)/4;

	if (year > 1800) {
		d -= (year - 1701)/100;
		d += (year - 1601)/400;
	}
	if (year > 1752)
		d += 3;
	if (dysize (year) == 366 && ut -> ut_mon >= 3)
		d ++;
	while (--mon)
		d += dmsize[mon - 1];
	d += ut -> ut_mday;
	return (d % 7);
}
