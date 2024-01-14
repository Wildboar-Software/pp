/* or_misc.c: misc or-name handling routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_misc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_misc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_misc.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "table.h"
#include "or.h"
#include "ap.h"
#include <stdarg.h>
#include <isode/cmd_srch.h>

extern CMD_TABLE ortbl_ddvalid[];

extern char             *loc_dom_site;
extern char             *loc_or;

OR_ptr                  loc_ortree;




/* ---------------------  Start  Routines  -------------------------------- */




int or_init()
{
    char        tbuf[LINESIZE];


    PP_DBG (("Lib/or_init()"));

    if (loc_or == NULLCP || *loc_or == '\0') {
	    PP_LOG (LLOG_EXCEPTIONS, ("Lib/or_init: loc_or NULL !"));
	    return NOTOK;
    }
    (void) strcpy (tbuf, loc_or);
    loc_ortree = or_std2or (tbuf);

    if (loc_ortree == NULLOR) {
	PP_LOG (LLOG_EXCEPTIONS, ("Lib/or_init: loc_ortree NULL! %s", loc_or));
	return NOTOK;
    }

    return OK;
}




/*
domain syntax is valid if it contains  
one or more ASCII chars EXCEPT FOR  specials spaces and ctrls
*/

int or_isdomsyntax (s)
register char   *s;
{

	PP_DBG (("Lib/or_isdomsyntax('%s')", s));

	for (; *s != '\0'; s++) {
		switch (*s) {

		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '\\':
		case '"':
		case '.':
		case '[':
		case ']':
		case ' ':
			/* --- specials --- */
			return FALSE;

		default:
			if (!isascii (*s))	return FALSE;
			if (iscntrl (*s))	return FALSE;
			break;
		}
	}

    return TRUE;
}


int or_ddname_chk (key)
char    *key;
{
	int	value = cmd_srch(key, ortbl_ddvalid);

	if (value == OR_DDVALID_RFC822
	    || value == OR_DDVALID_X40088
	    || value == OR_DDVALID_JNT
	    || value == OR_DDVALID_UUCP)
		return TRUE;
    return FALSE;
}




int rfc_space (s)
char  *s;
{
    int    retval = TRUE;

    for (; *s != '\0'; s++)
	if (*s != ' ') {
		retval = FALSE;
		break;
	}
    PP_DBG (("Lib/rfc_space ('%d')", retval));
    return retval;
}

#ifdef lint
/* VARARGS1 */
int or_lose (str)
char *str;
{
	return or_lose (str);
}
#else
int or_lose (va_alist)
va_dcl
{
	va_list ap;
	extern char or_error[];

	va_start (ap);

	_asprintf (or_error, NULLCP, ap);
	
	PP_LOG (LLOG_EXCEPTIONS, ("or_lose: %s", or_error));

	va_end(ap);

	return NOTOK;
}
#endif
