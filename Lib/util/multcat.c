/* multcat: concatenate strings together */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/multcat.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/multcat.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: multcat.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <varargs.h>

/* VARARGS 1*/
char *
multcat (va_alist)
va_dcl
{
	register va_list ap;
	register char  *oldstr, *ptr;
	char    *newstr;
	unsigned  newlen;

	va_start(ap);
	for (newlen = 1; oldstr = va_arg(ap, char *);)
		newlen += strlen (oldstr);
	va_end(ap);

	ptr = newstr = smalloc ((int)newlen);

	va_start(ap);
	for (; oldstr = va_arg(ap, char *); ptr--)
		while(*ptr++ = *oldstr++);
	va_end(ap);

	return (newstr);
}
