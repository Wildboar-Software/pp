/* msgid2rfc - Converts a MPDUid struct into a RFC string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/msgid2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/msgid2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: msgid2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "mta.h"

extern int globalid2rfc ();
int msgid2rfc_aux();

/* ---------------------  Begin  Routines  -------------------------------- */

int msgid2rfc (msgid, buffer)
MPDUid	*msgid;
char	*buffer;
{
	return msgid2rfc_aux (msgid, buffer, FALSE);
}

int msgid2rfc_aux (msgid, buffer, angled)
MPDUid  *msgid;
char    *buffer;
int	angled;
{
	char    tbuf[LINESIZE];

	if (globalid2rfc (&msgid -> mpduid_DomId, tbuf) == NOTOK)
		return NOTOK;

	if (msgid -> mpduid_string) 
		(void) sprintf(buffer, "%c%s;%s%c",
			       (angled == TRUE) ? '<' : '[',
			       tbuf, msgid -> mpduid_string,
			       (angled == TRUE) ? '>' : ']');
	else
		(void) sprintf(buffer, "%c%s%c",
			       (angled == TRUE) ? '<' : '[',
			       tbuf,
			       (angled == TRUE) ? '>' : ']');

	PP_DBG (("Lib/msgid2rfc returns (%s)", buffer));

	return OK;
}
