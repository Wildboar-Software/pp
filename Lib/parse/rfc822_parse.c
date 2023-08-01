/* rfc822_parse.c: 822 address parsing and normalisation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_parse.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_parse.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: rfc822_parse.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"

extern int ap_lex_percent;
extern char *loc_dom_site;
static int normalise_and_strip_excess_locals();

int rfc822_parse(ad)
register ADDR	*ad;
{
	int	old_ap_lex = ap_lex_percent;
	char	buf[BUFSIZ];
	ap_lex_percent =  ad->aparse->percents;
	switch ((int) (ad->aparse->ap_tree =
		       ap_s2t ((ad->aparse->r822_str) ?
			       ad->aparse->r822_str : 
			       ad->ad_value))) {
	    case 0:
		/* -- could parse it and found it to be duff -- */
		if (ad->aparse->r822_error)
			free(ad->aparse->r822_error);
		(void) sprintf(buf, "Unparseable address '%s'", 
			(ad->aparse->r822_str) ? ad->aparse->r822_str : ad->ad_value);

		ad->aparse->r822_error = strdup(buf);
		ap_lex_percent = old_ap_lex;
		return RP_NO;
		
	    case NOTOK:
		/* -- couldn't parse it (x400 ?) -- */
		if (ad->aparse->r822_error)
			free(ad->aparse->r822_error);
		ad->aparse->r822_error = strdup("822 parse error");
		ap_lex_percent = old_ap_lex;
		return RP_PARSE;
		
	    default:
		ap_lex_percent = old_ap_lex;
		break;
	}

	/* -- split into parts -- */

	(void) ap_t2p(ad->aparse->ap_tree,
		      &ad->aparse->ap_group,
		      &ad->aparse->ap_name,
		      &ad->aparse->ap_local,
		      &ad->aparse->ap_domain,
		      &ad->aparse->ap_route);

	if (rp_isbad(normalise_and_strip_excess_locals(ad)))
		return RP_PARSE;

	fillin_822_str(ad);

	return RP_OK;
}

static AP_ptr	get_next_route (ad, startfrom)
register ADDR	*ad;
AP_ptr		startfrom;
{
	AP_ptr	ret;
	
	if (startfrom == NULLAP)
		return NULLAP;
	
	if (startfrom->ap_obtype == AP_DOMAIN
	    || startfrom->ap_obtype == AP_DOMAIN_LITERAL)
		return startfrom;

	ret = startfrom;
	for (;;) {
		switch (ret->ap_ptrtype) {
		    case AP_PTR_MORE:
			/* -- next is part of this address -- */
			ret = ret -> ap_next;
			if (ret == ad->aparse->ap_group ||
			    ret == ad->aparse->ap_name ||
			    ret == ad->aparse->ap_local ||
			    ret == ad->aparse->ap_domain)
				return NULLAP;

			switch (ret -> ap_obtype) {
			    case AP_DOMAIN_LITERAL:
			    case AP_DOMAIN:
				return ret;
			    default:
				break;
			}
			break;
		    default:
			return NULLAP;
		}
	}
}

static int normalise_and_strip_excess_locals(ad)
register ADDR	*ad;
{
	AP_ptr	rout;
	
	if (ad->aparse->normalised == APARSE_NORM_ALL) {
		/* normalise all domains */
		rout = ad->aparse->ap_route;
		while (rout && 
		       (rout = get_next_route(ad, rout)) != NULLAP) {
			(void) rfc822_norm_dmn(rout, ad->aparse->dmnorder);
			rout = rout->ap_next;
		}
		if (ad->aparse->ap_domain)
			(void) rfc822_norm_dmn(ad->aparse->ap_domain, 
					ad->aparse->dmnorder);
		rout = get_next_route (ad, ad->aparse->ap_route);
	} else {
		/* normalise first domain */
		if ((rout = get_next_route(ad, ad->aparse->ap_route))
		    != NULLAP)
			(void) rfc822_norm_dmn(rout,
					ad->aparse->dmnorder);
		else if (ad->aparse->ap_domain)
			(void) rfc822_norm_dmn(ad->aparse->ap_domain,
					ad->aparse->dmnorder);
	}

	while (rout && rout->ap_islocal == TRUE) {
		/* local so remove from route */
		ad->aparse->ap_route = rout = get_next_route(ad, rout->ap_next);
		if (rout && rout->ap_normalised != TRUE)
			/* normalise ad->aparse->ap_route */
			(void) rfc822_norm_dmn(rout,
					ad->aparse->dmnorder);
	}

	if (rout == NULLAP) {
		/* no route */
		if (ad->aparse->ap_domain == NULLAP) {
			/* -- no domain either so add default -- */
			AP_ptr	ix;
			if (ad->aparse->ap_local == NULLAP) {
				/* -- no local either so fail */
				char	tmp[BUFSIZ];
				if (ad->aparse->r822_error)
					free(ad->aparse->r822_error);
				(void) sprintf(tmp, 
					"No route, domain or local part to '%s'",
					ad->ad_value);
				ad->aparse->r822_error = strdup(tmp);
				return RP_PARSE;
			}
			for (ix = ad->aparse->ap_local;
			     ix -> ap_next != NULLAP;
			     ix = ix -> ap_next)
				continue;
			if (loc_dom_site == NULLCP
			    || *loc_dom_site == '\0') {
				char	tmp[BUFSIZ];
				if (ad->aparse->r822_error)
					free(ad->aparse->r822_error);
				(void) sprintf(tmp, 
					"loc_dom_site is invalid");
				ad->aparse->r822_error = strdup(tmp);
				return RP_PARSE;
			} 
			(void) ap_append (ix, AP_DOMAIN, loc_dom_site);
			ad->aparse->ap_domain = ix->ap_next;
			(void) rfc822_norm_dmn(ad->aparse->ap_domain,
					ad->aparse->dmnorder);

		} else if (ad->aparse->ap_domain->ap_normalised != TRUE)
			/* normalise ad->aparse->ap_domain */
			(void) rfc822_norm_dmn(ad->aparse->ap_domain,
					ad->aparse->dmnorder);
	}
	return RP_OK;
}		
		

