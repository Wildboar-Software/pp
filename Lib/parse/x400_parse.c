/* x400_parse.c: x400 address parsing */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/x400_parse.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/x400_parse.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: x400_parse.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"

extern char or_error[];

int x400_parse (ad)
register ADDR	*ad;
{
	if (ad->aparse->x400_str) 
		ad->aparse->orname->on_or =
			or_std2or(ad->aparse->x400_str);
	else {
		if ((ad->aparse->orname->on_or =
			or_std2or(ad->ad_value)) != NULLOR) {
			ad->aparse->orname->on_or =
				or_default(ad->aparse->orname->on_or);
		}
		
	}

	if (ad->aparse->x400_dn)
		/* do something with dn ? */
		;

	if (ad->aparse->orname->on_or == NULLOR) {
		ad->aparse->x400_error = strdup(or_error);
		return RP_PARSE;
	}
	
	fillin_400_str(ad);

	return RP_AOK;
}
