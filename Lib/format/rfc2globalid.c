/* rfc2globalid.c - Converts a RFC string into a GlobalDomId struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2globalid.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2globalid.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2globalid.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "mta.h"




/* ---------------------  Begin  Routines  -------------------------------- */

#define PRMD	"/PRMD="
#define ADMD	"/ADMD="
#define COUNTRY	"/C="

int rfc2x400globalid (gp, str)
GlobalDomId	*gp;
char		*str;
{
	char	*start, *end;
	int len;

	len = strlen(PRMD);
	if (lexnequ (str, PRMD, len) == 0) {
		start = str + len;
		if ((end = index(start, '/')) == NULLCP)
			return NOTOK;
		*end = '\0';
		gp -> global_Private = strdup(start);
		*end = '/';
		str = end;
	}
	len = strlen (ADMD);
	if (lexnequ (str, ADMD, len) == 0) {
		start = str + len;
		if ((end = index(start, '/')) == NULLCP) {
			if (gp -> global_Private != NULLCP) {
				free (gp -> global_Private);
				gp -> global_Private = NULLCP;
			}
			return NOTOK;
		}
		*end = '\0';
		gp -> global_Admin = strdup(start);
		*end = '/';
		str = end;
	}
	len = strlen(COUNTRY);
	if (lexnequ (str, COUNTRY, strlen(COUNTRY)) == 0) {
		start = str + len;
		if ((end = index(start, '/')) == NULLCP) {
			if (gp -> global_Private != NULLCP) {
				free (gp -> global_Private);
				gp -> global_Private = NULLCP;
			}
			if (gp -> global_Admin != NULLCP) {
				free (gp -> global_Admin);
				gp -> global_Admin = NULLCP;
			}
			return NOTOK;
		}
		*end = '\0';
		gp -> global_Country = strdup(start);
		*end = '/';
		str = end;
	}
	return OK;
}
	

int rfc2globalid (gp, str)
GlobalDomId     *gp;
char            *str;
{

	char    *cp = str,
		*bp = str;

	PP_DBG (("Lib/rfc2globalid (%s)", str));


	if (*cp == '*')
		return NOTOK;

	/* -- Country -- */
	cp = index (bp, '*');
	if (cp == NULLCP)
		return NOTOK;
	*cp++ = '\0';
	gp->global_Country = strdup (bp);
	bp = cp;

	/* -- Admin -- */
	cp = index (bp, '*');
	if (cp != NULLCP)
		*cp++ = '\0';
	gp->global_Admin = strdup (bp);
	bp = cp;

	/* -- Prmd -- */
	if (cp == NULLCP)
		return OK;
	gp->global_Private = strdup (bp);

	return OK;
}
