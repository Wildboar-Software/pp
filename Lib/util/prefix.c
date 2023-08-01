/* prefix: Determine if str1 is the prefix of str2 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/prefix.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/prefix.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: prefix.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"

extern  char    chrcnv[];

int prefix(str1, str2)
register char   *str1;
register char   *str2;
{
	while(*str1)
		if(chrcnv[*str1++] != chrcnv[*str2++])
			return (FALSE);

	return (TRUE);
}
