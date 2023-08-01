/* globalid2rfc.c - Converts a GlobalDomId struct into a RFC string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/globalid2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/globalid2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: globalid2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "mta.h"

int globalid2rfc (gp, buffer)  /* GlobalDomainIdentifier -> RFC */
GlobalDomId     *gp;
char            *buffer;
{
	char *cp = buffer;

	if (gp -> global_Private) {
		(void) sprintf (cp, "/PRMD=%s", gp -> global_Private);
		cp += strlen(cp);
	}

	if (gp -> global_Admin) {
		(void) sprintf(cp,"/ADMD=%s", gp -> global_Admin);
		cp += strlen(cp);
	}

	if (gp -> global_Country) {
		(void) sprintf(cp, "/C=%s",gp -> global_Country);
		cp += strlen(cp);
	}
	if (cp != buffer) {
		*cp++ = '/';
		*cp = NULL;
	}

	PP_DBG (("Lib/globalid2rfc returns (%s)", buffer));

	return OK;
}
