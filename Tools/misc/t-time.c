/* t-time.c: tests the time routines rfc2UTC and UTC2rfc */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-time.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-time.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-time.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include "util.h"


main ()
{
	char    buf[BUFSIZ];
	UTC     ut;

	while (fputs ("Time> ", stdout), fflush(stdout), gets (buf)) {
		if (rfc2UTC (buf, &ut) == NOTOK)
			printf ("rfc2UTC failed\n");
		printf ("UTC time = %s\n", utct2str (ut));
		printf ("GEN time = %s\n", gent2str (ut));
		if (UTC2rfc (ut, buf) == NOTOK)
			printf ("UTC2rfc failed\n");
		printf ("new rfc = %s\n", buf);
	}
	exit (0);
}
