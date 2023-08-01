/* rfc2msgid - Converts a RFC string into a MPDUid struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2msgid.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2msgid.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2msgid.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "mta.h"
#include 	"x400_ub.h"




/* ---------------------  Begin  Routines  -------------------------------- */




int rfc2msgid (mp, str)
register MPDUid         *mp;
char                    *str;
{
	char            *cp, *start, *end, *temp;

	PP_DBG (("Lib/rfc2msgid (%s)", str));

	temp = strdup(str);
	start = temp;
	if (*start == '[') {
		start++;
		end = str+strlen(str);
		if (*end == ']')
			*end = '\0';
	}

	/* try parsing as dom;string */
	cp = rindex (start, ';');
	if (cp != NULLCP) {
		*cp++ = '\0';
		if (rfc2globalid (&mp->mpduid_DomId, start) == OK) {
			mp->mpduid_string = strdup (cp);
			free(temp);
			return OK;
		}
	}
	free(temp);
	GlobalDomId_free(&mp->mpduid_DomId);
	/* put in loc dom and string */
	if ((int) strlen(str) > UB_LOCAL_ID_LENGTH) {
		char tbuf[UB_LOCAL_ID_LENGTH + 1];
		(void) strncap (tbuf, str, UB_LOCAL_ID_LENGTH);
		tbuf[UB_LOCAL_ID_LENGTH] = 0;
		mp->mpduid_string = strdup (tbuf);
	}
	else
		mp->mpduid_string = strdup (str);
	GlobalDomId_new (&mp->mpduid_DomId);
	return OK;
}
