/* rfc822_x400.c: address conversion from 822 to x400 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_x400.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_x400.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: rfc822_x400.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"

extern char or_error[];

int rfc822_x400(ad)
register ADDR	*ad;
{
	OR_ptr or;
	char	*p = ad->aparse->r822_str ? 
		ad->aparse->r822_str : ad->ad_value;
	PP_DBG(("rfc822_x400('%s')", p));

	aparse_rewindx400(ad->aparse);

	if (or_rfc2or_aux (p, &(ad->aparse->orname->on_or), ad->ad_no) == NOTOK) {
		or_free (ad->aparse->orname->on_or);
		ad->aparse->orname->on_or = NULLOR;
		PP_DBG(("rfc822_x400 conversion failed (%s)", or_error));
		return RP_PARSE;
	}

	if ((or = or_default (ad->aparse->orname->on_or)) == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS, ("or_default failed"));
		return RP_PARSE;
	}
	ad -> aparse -> orname -> on_or = or;

	fillin_400_str(ad);

	return RP_AOK;
}

void x400_add(ad)
register ADDR	*ad;
{
	(void) rfc822_x400(ad);
	
	if (ad->aparse->x400_str) {
		if (ad->ad_r400adr)
			free (ad->ad_r400adr);

		ad->ad_r400adr = strdup (ad->aparse->x400_str);
	}
	PP_DBG (("x400_add returns (x400_str=%s)", ad->aparse->x400_str));
}	
