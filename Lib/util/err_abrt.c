/* err_abrt: abort from an error */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/err_abrt.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/err_abrt.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: err_abrt.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include <stdarg.h>

void err_abrt (int code, char *fmt, ...)
{
	va_list	ap;
	char    tbuf[BUFSIZ];
	va_start (ap, fmt);
	_asprintf (tbuf, NULLCP, fmt, ap);
	PP_OPER (NULLCP, ("err_abrt (%s) [%s]", tbuf, rp_valstr(code)));
	fprintf(stderr, "err_abrt -> %s [%s]\n", tbuf, rp_valstr(code));
	va_end (ap);
	exit(code);
}
