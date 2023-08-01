/* x400_rfc822.c: address conversion from x400 to 822 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/x400_rfc822.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/x400_rfc822.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: x400_rfc822.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"

extern char or_error[];

int	x400_rfc822 (ad)
register ADDR	*ad;
{
	char	tbuf[LINESIZE];

	PP_DBG(("x400_rfc822('%s')",	
		ad->aparse->x400_str ? ad->aparse->x400_str : ad->ad_value));
	
	if (ad->aparse->orname->on_or == NULLOR) {
		/* ??? shouldn't happen */
		return RP_PARSE;
	}

	aparse_rewindr822(ad->aparse);

	switch (or_or2rfc (ad->aparse->orname->on_or, tbuf)) {
	    case DONE:
		/* ??? */
		return RP_AOK;
	    case NOTOK:
		PP_DBG(("x400_rfc822 Conversion failed (%s)", or_error));
		return RP_PARSE;
	    default:
		break;
	}
	/* do something with DNs */

	ad->aparse->r822_str = strdup(tbuf);
	
	if (rp_isbad(rfc822_parse (ad)))
		return RP_PARSE;

	PP_DBG(("x400_rfc822 result ('%s')", tbuf));

	return (RP_AOK);
}

void rfc822_add(ad)
register ADDR	*ad;
{
	(void) x400_rfc822(ad);
	
	if (ad->aparse->r822_str) {
		if (ad->ad_r822adr) 
			free(ad->ad_r822adr);
		ad->ad_r822adr = strdup(ad->aparse->r822_str);
	}
	PP_DBG (("rfc822_add returns (r822_str=%s)", ad->aparse->r822_str));
}
	
