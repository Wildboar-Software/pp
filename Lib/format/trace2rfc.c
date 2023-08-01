/* trace2rfc.c - Converts a Trace struct into a RFC string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/trace2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/trace2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: trace2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "mta.h"

extern int globalid2rfc ();
extern int domsinfo2rfc ();

/* ---------------------  Begin  Routines  -------------------------------- */


int trace2rfc (trace, buffer)  /* TraceInformation -> RFC */
Trace   *trace;
char    *buffer;
{
	char    *cp;
	Trace   *tp = trace;

	if (tp == (Trace *) NULL)
		return DONE;

	if (globalid2rfc (&tp -> trace_DomId, buffer) == NOTOK)
		return NOTOK;
	cp = buffer + strlen(buffer);

	if (domsinfo2rfc (&tp -> trace_DomSinfo, cp) == NOTOK)
		return NOTOK;

	PP_DBG (("Lib/trace2rfc returns (%s)", buffer));

	return OK;
}
