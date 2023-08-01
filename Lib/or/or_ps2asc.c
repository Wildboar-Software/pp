/* or_ps2asc.c: convert printable string to ascii */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_ps2asc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_ps2asc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_ps2asc.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "util.h"


/*
Map printable string to ASCII
*/

void or_ps2asc (ps, ascii)
char    *ps;
char    *ascii;
{
    register char       *p, *q;
    int                 i;

    PP_DBG (("Lib/or_ps2asc('%s')", ps));

    p = ps;
    q = ascii;
    for (;*p != '\0';) {

	if (*p != '(')
		*q++ = *p++;
	else {
		if (*(p + 2) == ')') {
			switch (*(p + 1)) {
			    case 'a':
			    case 'A':
				*q++ = '@';
				break;
			    case 'p':
			    case 'P':
				*q++ = '%';
				break;
			    case 'b':
			    case 'B':
				*q++ = '!';
				break;
			    case 'q':
			    case 'Q':
				*q++ = '"';
				break;
			    case 'u':
			    case 'U':
				*q++ = '_';
				break;
			    case 'l':
			    case 'L':
				*q++ = '(';
				break;
			    case 'r':
			    case 'R':
				*q++ = ')';
				break;

			    default:
				PP_LOG (LLOG_EXCEPTIONS, 
					("Unknown Printable Char '%s'", ps));
				(void) strcpy (ascii, ps);
				return;
			}
			p = p + 3;
		}

		else {

			if (isdigit (*(p + 1)) && isdigit (*(p + 2))
			    && (*(p + 3)) && *(p + 4) == ')')
			{
				(void) sscanf (p + 1, "%3d", &i);
				p = p + 5;
			}
			else
			{
				PP_LOG (LLOG_EXCEPTIONS, 
					("Unkown digit '%s'", ps)); 
				(void) strcpy (ascii, ps);
				return;
			}


			if (i <= 0 || i >= 127) {
				PP_LOG (LLOG_EXCEPTIONS, 
					("Digit '%d' out of range '%s'", i, ps)); 
				(void) strcpy (ascii, ps);
				return;
			}

			*q++ = (char) i;

		}  /* end of if */

	}  /* end of if */

    }  /* end of for */

    *q = '\0';
}
