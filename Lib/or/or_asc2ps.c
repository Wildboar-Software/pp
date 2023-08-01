/* or_asc2ps.c: convert ascii to printable string  */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_asc2ps.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_asc2ps.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_asc2ps.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "or.h"

int	real987	= NOTOK;

int or_asc2ps (ascii, ps)
char    *ascii;
char    *ps;
{
    register char       *p, *q;
    int			converted;
    PP_DBG (("Lib/or_asc2ps()"));

    q = ps;
    converted = OK;
    for (p = ascii; *p != '\0'; p++) {
	    
	if (or_isps (*p) && *p != ')' && *p != '(')
		*q++ = *p;
	else {
		converted = NOTOK;
		*q++ = '(';
		switch (*p) {
		    case '@':
			*q++ = 'a';
			break;
		    case '%':
			*q++ = 'p';
			break;
		    case '!':
			*q++ = 'b';
			break;
		    case '"':
			*q++ = 'q';
			break;
		    case '_':
			*q++ = 'u';
			break;
		    case '(':
			if (real987 == NOTOK) {
				*q++ = 'l';
				break;
			}
		    case ')':
			if (real987 == NOTOK) {
				*q++ = 'r';
				break;
			}
		    default:
			(void) sprintf (q, "%03d", *p);
			q = q + 3;
			break;

		}		/* end of switch */

		*q++ = ')';

	}  /* end of if */

    }  /* end of for */

    *q = '\0';
    return converted;
}
