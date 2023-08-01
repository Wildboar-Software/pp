/* rfc2UTC.c - Converts an RFC string into UTC structure */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2UTC.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2UTC.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2UTC.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/cmd_srch.h>
#include "mta.h"


extern CMD_TABLE        tbl_month[];

/* ---------------------  Begin  Routines  -------------------------------- */




/*
This routine converts a RFC 822 standard time string like:
	"Tue, 16 Aug 88 10:23:21 BST"
into a UTC structure. Where "Tue," is optional.
*/

static void getzone ();

int rfc2UTC (str, utc)
char    *str;
UTC     *utc;
{
	UTC     ut;
	char    *cp, *cp2;
	char    tbuf[LINESIZE];
	int     n;


	PP_DBG (("Lib/rfc2UTC (%s)", str));

	ut = (UTC) smalloc (sizeof *ut);
	bzero ((char *)ut, sizeof *ut);
	*utc = NULL;

	cp = str;
#define skips(c)        while (isspace (*c)) c++
#define skipsp(c)       while (isspace (*c) || ispunct(*c)) c++;

	skips (cp);

	if (isalpha (*cp)) {
		while(isalpha (*cp))
			cp ++;
		skipsp (cp);
	}


	/* month day */
	if (isdigit (*cp)) {
		n = 0;
		while (isdigit (*cp))
			n = n * 10 + (*cp++ - '0');
		ut -> ut_mday = n;
		skips (cp);
	}
	else {
		PP_LOG (LLOG_EXCEPTIONS, ("Bad month day '%s'", str));
		free((char *)ut);
		return NOTOK;
	}

	/* Month */
	if (isalpha (*cp)) {
		for (cp2 = tbuf; isalpha(*cp); *cp2 ++ = *cp++)
			continue;
		*cp2 = NULL;
		if (cp2 - tbuf > 3)
			tbuf[3] = NULL;
		ut -> ut_mon = cmd_srch (tbuf, tbl_month);
		if (ut -> ut_mon == -1) {
			free ((char *)ut);
			return NOTOK;
		}
		ut -> ut_mon ++;
		skips (cp);
	}
	else    {
		free((char*)ut);
		return NOTOK;
	}

	/* -- Year -- */
	if (isdigit (*cp)) {
		n = 0;
		while (isdigit (*cp))
			n = n * 10 + (*cp ++ - '0');
		if (n < 100)
			ut -> ut_year = 1900 + n;
		else
			ut -> ut_year = n;
		skips (cp);
	}
	else {
		free ((char *)ut);
		return NOTOK;
	}

	if (isdigit (*cp)) {
		n = 0;
		while (isdigit(*cp))
			n = n * 10 + (*cp++ - '0');
		ut -> ut_hour = n;
		skips (cp);
	}

	/* -- Minute & Secs -- */
	if (*cp == ':') {
		cp ++;
		n = 0;
		while (isdigit (*cp))
			n = n * 10 + (*cp++ - '0');
		ut -> ut_min = n;
		skips (cp);
	}
	if (*cp == ':') {
		cp ++;
		n = 0;
		while (isdigit (*cp))
			n = n * 10 + (*cp ++ - '0');
		ut -> ut_sec = n;
		ut -> ut_flags |= UT_SEC;
		skips (cp);
	}

	getzone (cp, ut);

	*utc = ut;
	PP_DBG (("Lib/rfc2UTC returns (%.22s)", utct2str (ut)));

	return OK;
}



static CMD_TABLE        zone_tbl[] = {  /* some of the more popular zones... */
	"A",    -100,
	"AHST",	-1000,	/* Alaska-Hawaii Std Time */
	"AT",	-200,	/* Azores Time */
	"AST",	-400,	/* Atlantic Std Time */
	"B",    -200,
	"BST",  100,	/* British Summer Time, also Brasil Std Time (-300) and Bering Strait Time (-1100) */
	"BT",	-1100,	/* Bering Time also Baghdad Time (+200) */
	"C",    -300,
	"CAT",	-1000,	/* Central Alaska Time */
	"CCT",	800,	/* China Coast Time, USSR Zone 7 */
	"CDT",  -500,
	"CET",	100,	/* Central European Time */
	"CST",  -600,	/* Central Std Time */
	"D",    -400,
	"E",    -500,
	"EDT",  -400,
	"EET",	200,	/* Eastern European Time, USSR Zone 1 */
	"EST",  -500,	/* Eastern Std Time */
	"F",    -600,
	"G",    -700,
	"GMT",  0,
	"GST",	1000,	/* Guam Std Time, USSR Zone 9 */
	"H",    -800,
	"HST",	-1030,	/* Hawaiian Std Time */
	"I",    -900,
	"IDLE",	1200,	/* International Date Line, East */
	"IDLW",	-1200,	/* International Date Line, West */
	"IST",	530,	/* Indian Standard Time */
	"IT",	300,	/* Iran Time */
	"JST",	900,	/* Japan Std Time, USSR Zone 8 */
	"JT",	730,	/* Java Time */
	"K",    -1000,
	"L",    -1100,
	"M",    -1200,
	"MDT",  -600,
	"MET",	100,	/* Middle European Time */
	"MST",  -700,	/* Mountain Std Time */
	"MT",	830,	/* Moluccas Time */
	"N",    100,
	"NT",	-1100,	/* Nome Time */
        "NFT",	-330,	/* Newfoundland Time */
	"NST",	630,	/* North Sumatra Time */
	"NZT",	1130,	/* New Zealand Time */
	"O",    200,
	"P",    300,
	"PDT",  -700,
	"PST",  -800,	/* Paciific Std Time */
	"Q",    400,
	"R",    500,
	"S",    600,
	"SAST",	930,	/* South Australia Std Time */
	"SST",	700,	/* South Sumatra Time */
	"T",    700,
	"U",    800,
	"UT",   0,
	"V",    900,
	"W",    1000,
	"WAT",	-100,	/* West Africa Time */
	"WDT",  100,
	"WET",  0,	/* Western European Time */
	"X",    1100,
	"Y",    1200,
	"YST",	-900,	/* Yukon Std Time */
	"Z",    0,
	0,      -99
};
#define N_ZONES ((sizeof(zone_tbl)/sizeof(CMD_TABLE)) - 1)

static void getzone (bp, ut)
char    *bp;
UTC     ut;
{
	int     hours, mins;
	char    *cp, zbuf[64];

	while (isspace (*bp))
		bp ++;

	if (*bp == '+' || *bp == '-') { /* offset TZ */
		hours = (bp[1] - '0') * 10 + (bp[2] - '0');
		mins = (bp[3] - '0') * 10 + (bp[4] - '0');
		ut -> ut_zone = 60 * hours + mins;
		if (*bp == '-')
			ut -> ut_zone = - ut -> ut_zone;

		ut -> ut_flags |= UT_ZONE;
		return;
	}
	if (isalpha(*bp)) {     /* 'real' TZ (e.g. BST) */
		for (cp = zbuf; isalpha(*bp); )
		      *cp++ = *bp ++;
		*cp = 0;
		if ((hours = cmdbsrch (zbuf, zone_tbl, N_ZONES)) != -99) {
			ut -> ut_flags |= UT_ZONE;
			ut -> ut_zone = ((hours/100) * 60);
			/* add minutes */
			if (hours >= 0)
				ut -> ut_zone += (hours%100);
			else
				ut -> ut_zone -= (hours%100);
			return;
		}
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown timezone syntax %s", zbuf));
	}
	if (*bp != '\0')
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown timezone syntax %s", bp));
}
