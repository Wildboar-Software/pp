/* ut_dlh.c: utility routines for distribution list history */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_dlh.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_dlh.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_dlh.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"mta.h"

DLHistory *dlh_new (addr, dn, utc)
char	*addr, *dn;
UTC	utc;
{
	DLHistory *dlh;

	dlh = (DLHistory *) smalloc (sizeof *dlh);

	dlh -> dlh_addr = addr ? strdup (addr) : NULLCP;
	dlh -> dlh_dn = dn ? strdup (dn) : NULLCP;
	dlh -> dlh_time = utc ? utcdup(utc) : utcnow ();
	dlh -> dlh_next = NULL;
	return dlh;
}

void	dlh_add (base, dlh)
DLHistory **base, *dlh;
{
	DLHistory **dlp;

	for (dlp = base; *dlp; dlp = &(*dlp) -> dlh_next)
		continue;
	*dlp = dlh;
}

DLHistory *dlh_dup (dlh)
DLHistory *dlh;
{
	DLHistory *new = NULL;
	DLHistory *dlp;

	for (dlp = dlh; dlp; dlp = dlp->dlh_next) {
		dlh_add (&new, dlh_new (dlp -> dlh_addr,
					dlp -> dlh_dn,
					dlp -> dlh_time));
	}
	return new;
}

void dlh_free (dlh)
DLHistory *dlh;
{
	if (dlh == NULL)
		return;
	if (dlh -> dlh_next)
		dlh_free (dlh -> dlh_next);
	if (dlh -> dlh_addr)
		free (dlh -> dlh_addr);
	if (dlh -> dlh_dn)
		free (dlh -> dlh_dn);
	if (dlh -> dlh_time)
		free ((char *) dlh -> dlh_time);
	free ((char *)dlh);
}

