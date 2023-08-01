/* compress: compress out redundant, & strip trailing linear white space */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/compress.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/compress.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: compress.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"


char *compress (fromptr, toptr)
register char   *fromptr;
register char   *toptr;
{
	register char   chr;
	char		*in_toptr = toptr;
	/*
	init to skip leading spaces
	*/
	chr = ' ';

	while ((*toptr = *fromptr++) != '\0') {
		/*
		convert newlines and tabs to only save first space
		*/
		if (isspace (*toptr)) {
			if (chr != ' ')
				*toptr++ = chr = ' ';
		}
		else
			chr = *toptr++;
	}


	/*
	remove trailing space if any
	*/
	if ((chr == ' ')
	    && (toptr != in_toptr))
		*--toptr = '\0';
	return (toptr);
}
