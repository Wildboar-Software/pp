/* ut_free.c: Freeing utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_free.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_free.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_free.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include	"mta.h"


/* ---------------------  Begin  Routines  -------------------------------- */

void encodedinfo_free (ep)  /* EncodedInfoTypes */
EncodedIT       *ep;
{
	if (ep == (EncodedIT *)NULL)
		return;

	if (ep->eit_types)
		list_bpt_free (ep->eit_types);
	bzero ((char *)ep, sizeof *ep);
}

void DomSupInfo_free (dp)  /* DomainSuppliedInfo */
DomSupInfo      *dp;
{
	if (dp == NULL)
		return;

	if (dp->dsi_time)
		free ((char *)dp->dsi_time);

	if (dp->dsi_deferred)
		free ((char *)dp->dsi_deferred);

	encodedinfo_free  (&dp -> dsi_converted);
	GlobalDomId_free  (&dp->dsi_attempted_md);

	if (dp -> dsi_attempted_mta)
		free (dp -> dsi_attempted_mta);

	bzero ((char *)dp, sizeof *dp);
}

void redirection_free (rp)
Redirection *rp;
{
	Redirection *rq;

	for (; rp; rp = rq) {
		rq = rp -> rd_next;
		if (rp -> rd_addr)
			free (rp -> rd_addr);
		if (rp -> rd_dn)
			free (rp -> rd_dn);
		free ((char *)rp);
	}
}

void extensions_free (ext)
X400_Extension	*ext;
{
	if (ext == NULL)
		return;
	if (ext -> ext_next)
		extensions_free (ext -> ext_next);
	if (ext -> ext_oid)
		oid_free (ext -> ext_oid);
	if (ext -> ext_value)
		qb_free (ext -> ext_value);
	free ((char *) ext);
}


	
