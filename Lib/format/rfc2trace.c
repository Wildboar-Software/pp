/* rfc2trace.c - Converts a RFC string into a Trace struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2trace.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2trace.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2trace.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "mta.h"




/* ---------------------  Begin  Routines  -------------------------------- */




int rfc2trace (tp, str)
Trace   *tp;
char    *str;
{
	char    *cp = str,
		*bp = str;


	PP_DBG (("Lib/rfc2trace (%s)", str));

	cp = index (str, ';');

	if (cp == NULLCP)
		return NOTOK;
	else
		*cp++ = '\0';

	if (*cp == ' ')
		cp++;

	if (rfc2globalid (&tp->trace_DomId, bp) == NOTOK)
		return NOTOK;

	if (rfc2domsinfo (&tp->trace_DomSinfo, cp) == NOTOK)
		return NOTOK;

	return OK;
}
