/* ad_getlocal.c: get the local part of the address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/ad_getlocal.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/ad_getlocal.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: ad_getlocal.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */
#include "head.h"
#include "adr.h"

extern char *rfc822_getlocal(), *x400_getlocal();

char	*ad_getlocal (addr, type, resp)
char	*addr;
int	type,
	resp;
{
	if (type == AD_X400_TYPE)
		return x400_getlocal(addr, resp);
	else
		return rfc822_getlocal(addr, resp);
}	

/*  */

char	*rfc822_getlocal(addr, resp)
char	*addr;
int	resp;
{
	RP_Buf	rp;
	char	*retstr;
	ADDR	*ad = adr_new(addr, AD_822_TYPE, 1);
	ad->ad_resp = resp;

	if (!rp_isbad (rfc822_parse(ad))
	    && !rp_isbad(rfc822_validate(ad, &rp))
	    && ad->aparse->r822_local)
		retstr = strdup(ad->aparse->r822_local);
	else
		retstr = NULLCP;

	adr_free(ad);
	return retstr;
}

/*  */

char 	*x400_getlocal(addr, resp)
char	*addr;
int	resp;
{
	RP_Buf	rp;
	char	*retstr;
	ADDR	*ad = adr_new(addr, AD_X400_TYPE, 1);
	ad->ad_resp = resp;

	if (!rp_isbad(x400_parse(ad))
	    && !rp_isbad(x400_validate(ad, &rp))
	    && ad->aparse->x400_local)
		retstr = strdup(ad->aparse->x400_local);
	else
		retstr = NULLCP;

	adr_free(ad);
	return retstr;
}
