/* rfc822_validate.c: validate a syntatic rfc 822 address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_val.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_val.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: rfc822_val.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"

static int rfc822_val_domain(), rfc822_val_local();

int	rfc822_validate (ad, rp)
register ADDR	*ad;
RP_Buf		*rp;
{
	/* -- some debugging statements -- */
	AP_ptr	dom_ptr, hptr;
	int	cont;
	char	tmp[BUFSIZ];

	if (ad->aparse->ap_route) {
		for (hptr = ad->aparse->ap_route, cont = TRUE;
		     cont == TRUE;
		     hptr = hptr->ap_next) {
			PP_DBG(("rfc822_validate (route=%s, type=%d)",
				hptr->ap_obvalue,
				hptr->ap_obtype));
			switch (hptr -> ap_ptrtype) {
			    case AP_PTR_MORE:
				break;
			    default:
				/* -- no more route -- */
				cont = FALSE;
				break;
			}
		}
	}

	if (ad->aparse->ap_domain)
		PP_DBG(("rfc822_validate (dmn=%s, type=%d)",
			ad->aparse->ap_domain->ap_obvalue,
			ad->aparse->ap_domain->ap_obtype));
	
	if (ad->aparse->ap_local)
		PP_DBG(("rfc822_validate (dmn=%s, type=%d)",
			ad->aparse->ap_local->ap_obvalue,
			ad->aparse->ap_local->ap_obtype));

	if (ad->aparse->ap_local == NULLAP) {
		(void) sprintf (tmp,
				"No local address in %s",
				ad->ad_value);
		ad->aparse->r822_error = strdup(tmp);
		fillin_822_str(ad);
		return RP_BAD;
	}

	/* assume have stripped excess local domains from route */
	
	dom_ptr = (ad->aparse->ap_route) ? 
		ad->aparse->ap_route :
			ad->aparse->ap_domain;

	if (dom_ptr && dom_ptr->ap_normalised != TRUE)
		/* -- shouldn't happen --*/
		/* normalise dom_ptr */
		(void) rfc822_norm_dmn(dom_ptr, ad->aparse->dmnorder);
	
	fillin_822_str(ad);

	if (dom_ptr && dom_ptr->ap_islocal == FALSE)
		return rfc822_val_domain(ad, dom_ptr, rp);
	else
		return rfc822_val_local(ad, dom_ptr, rp);
}
						
/*  */

static int rfc822_val_domain(ad, dom_ptr, rp)
register ADDR	*ad;
AP_ptr		dom_ptr;
RP_Buf		*rp;
{
	char	tmp[BUFSIZ];
	LIST_RCHAN	*rlp;
	/* not local so remove local if there */
	if (ad->aparse->r822_local) {
		free(ad->aparse->r822_local);
		ad->aparse->r822_local = NULLCP;
	}

	if (dom_ptr->ap_recognised == FALSE) {
		/* unrecgonised domain */
		/* dom_ptr->ap_error should be filled */
		/* in when normalise was done */
		if (dom_ptr->ap_error) {
			if (ad->aparse->r822_error)
				free(ad->aparse->r822_error);
			ad->aparse->r822_error = dom_ptr->ap_error;
			dom_ptr->ap_error = NULLCP;
		}
		return RP_BAD;
	}

	/* -- look up the channel table -- */
	if (ad->ad_outchan == NULLIST_RCHAN)
		if (dom_ptr->ap_chankey == NULLCP) {
			/* just norm in domain table */
			if (ad->aparse->r822_error)
				free(ad->aparse->r822_error);
			PP_NOTICE(("No routing information associated with domain '%s'",
				   dom_ptr->ap_obvalue));
			(void) sprintf (tmp,
					"Unrouteable domain '%s'",
					ad->aparse->ap_domain->ap_obvalue);
			ad->aparse->r822_error = strdup(tmp);
			return parselose (rp, RP_BAD, "%s", tmp);
		} else if (tb_getchan (dom_ptr->ap_chankey,
				       &(ad->ad_outchan)) != OK) {
			if (ad->aparse->r822_error)
				free(ad->aparse->r822_error);
			(void) sprintf (tmp,
					"Unknown domain '%s'",
					ad->aparse->ap_domain->ap_obvalue);
			PP_NOTICE(("No 822 channel registered for '%s'",
				   dom_ptr->ap_chankey));
			ad->aparse->r822_error = strdup(tmp);
			return parselose (rp, RP_BAD, "%s", tmp);
		}
	/* --- *** ---
	   Go thru list of channels. if li_mta (i.e. mta) is not set
	   insert the official.
	   --- *** --- */
	for (rlp=ad->ad_outchan; rlp; rlp = rlp->li_next)
		if (rlp->li_mta == NULLCP)
			rlp->li_mta = strdup (dom_ptr->ap_obvalue);

	if (ad->ad_outchan == NULL) 
		return RP_BAD;
	return RP_AOK;
}

/*  */

static int rfc822_val_local(ad, dom_ptr, rp)
register ADDR	*ad;
AP_ptr		dom_ptr;
RP_Buf		*rp;
{
	if (ad->aparse->r822_local) free(ad->aparse->r822_local);
	ad->aparse->r822_local = ap_p2s(NULLAP, NULLAP,
					  ad->aparse->ap_local,
					  NULLAP, NULLAP);

	if (dom_ptr
	    && dom_ptr->ap_localhub) {
		if (ad->aparse->local_hub_name)
			free(ad->aparse->local_hub_name);
		ad->aparse->local_hub_name = strdup(dom_ptr->ap_localhub);
	}
		
	return ad_local(ad->aparse->r822_local, ad, rp);

}

/*  */

int fillin_822_str(ad)
register ADDR	*ad;
{
	if (ad->aparse->r822_str) free(ad->aparse->r822_str);
	if (ad->aparse->full == OK)
		ad->aparse->r822_str = 
			ap_p2s(ad->aparse->ap_group, ad->aparse->ap_name,
			       ad->aparse->ap_local, ad->aparse->ap_domain,
			       ad->aparse->ap_route);
	else
		ad->aparse->r822_str = 
			ap_p2s_nc(NULLAP, NULLAP,
				  ad->aparse->ap_local, ad->aparse->ap_domain,
				  ad->aparse->ap_route);
		
}
