/* ap_dmflip.c: flip domains around */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_dmflip.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_dmflip.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_dmflip.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



#include "util.h"


/* ---------------------  Begin  Routines  -------------------------------- */


char *ap_dmflip(buf)
char    *buf;
{
	char            tbuf[LINESIZE];
	char            *cp;
	register char   *p,
			*q;

	/* -- don't flip if quoted or dlit -- */
	if (*buf == '"' || *buf == '[')
		return (strdup (buf));

	(void) strcpy (tbuf, buf);

	cp = smalloc (strlen (buf) + 1);

	for(q = cp; (p = rindex (tbuf, '.')) != NULLCP;  *(q-1) = '.') {
		*p++ = 0;
		while((*q++ = *p++) != '\0')
			continue;
	}

	p = tbuf;
	while((*q++ = *p++) != '\0')
		continue;
	return (cp);
}
