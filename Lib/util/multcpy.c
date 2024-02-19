/* multcpy: copy several strings */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/multcpy.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/multcpy.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: multcpy.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include <stdarg.h>

/* VARARGS 2 */
char *
multcpy (char *to, ...)
{
	va_list ap;
	char   *from;

	va_start(ap, to);

	while(from = va_arg(ap, char *)){
		while(*to++ = *from++);
		to--;
	}
	va_end(ap);
	return (to);            /* return pointer to end of str         */
}
