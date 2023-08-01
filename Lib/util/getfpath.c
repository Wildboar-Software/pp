/* getfpath: fix up pathnames */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/getfpath.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/getfpath.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: getfpath.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"

/*
 * convert a relative pathname into a full pathname - using base
 * as the secret ingredient
 */


void getfpath (base, prog, buf)
char    *base, *prog, *buf;
{
	if (*prog == '/' || strncmp (prog, "./", 2) == 0 ||
	    strncmp (prog, "../", 3) == 0)
		 (void) strcpy (buf, prog);
	else
		 (void) sprintf (buf, "%s/%s", base, prog);
}

/*
 * as above, but no buffer supplied, grab space off the heap
 */

char *dupfpath (base, prog)
char    *base, *prog;
{
	char    tmpbuf[MAXPATHLENGTH];

	getfpath (base, prog, tmpbuf);
	return strdup (tmpbuf);
}
