/* rfc2encinfo.c - Converts a RFC string into a EncodedIT struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2encinfo.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2encinfo.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2encinfo.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "mta.h"
#include "list_bpt.h"



/* ---------------------  Begin  Routines  -------------------------------- */

int rfc2encinfo (ep, str)
EncodedIT	*ep;
char		*str;
{
	LIST_BPT        *base = NULLIST_BPT,
			*new;
	char            *cp = str,
			*bp = str;


	PP_DBG (("Lib/rfc2listbpt (%s)", str));

	/* -- read string which delimitted by ',' and an optional ' '  -- */
	while (cp != NULLCP) {
		if (cp = index (bp, ',')) {
			*cp++ = '\0';
			if (*cp == ' ') cp++;
		}
		new = list_bpt_new (bp);
		list_bpt_add (&base, new);
		bp = cp;
	}

	ep -> eit_types = base;

	return OK;
}
