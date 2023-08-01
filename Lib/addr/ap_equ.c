/* ap_equ: check to see if to strings represent same address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_equ.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_equ.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_equ.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"ap.h"
#include	"adr.h"
#include	"chan.h"

extern char *loc_dom_site, *loc_dom_mta;
extern char *ad_getlocal();

ap_equ (one, two)
char	*one,
	*two;
{
	AP_ptr	one_tree, two_tree,
		loc1, dom1, loc2, dom2;
	int	equ = FALSE;

	one_tree = ap_s2t(one);
#ifdef UKORDER
	one_tree = ap_normalize(one_tree, CH_UK_PREF);
#else
	one_tree = ap_normalize(one_tree, CH_USA_PREF);
#endif
	two_tree = ap_s2t(two);
#ifdef UKORDER
	two_tree = ap_normalize(two_tree, CH_UK_PREF);
#else
	two_tree = ap_normalize(two_tree, CH_USA_PREF);
#endif


	(void) ap_t2p (one_tree, (AP_ptr *)0, (AP_ptr *)0,
		       &loc1, &dom1, (AP_ptr *)0);
	(void) ap_t2p (two_tree, (AP_ptr *)0, (AP_ptr *)0,
		       &loc2, &dom2, (AP_ptr *)0);
	
	if (dom1 && dom2 && lexequ(dom1->ap_obvalue, dom2->ap_obvalue) ==0) {
		/* same domains */
		if (dom1->ap_islocal == TRUE
		    || lexequ(dom1->ap_obvalue, loc_dom_site) == 0
		    || lexequ(dom1->ap_obvalue, loc_dom_mta) == 0) {
			/* local addresses */
			char	*oneloc, *twoloc;
			oneloc = ad_getlocal(one, AD_822_TYPE, YES);
			twoloc = ad_getlocal(two, AD_822_TYPE, YES);
			if (oneloc != NULLCP
			    && twoloc != NULLCP
			    && lexequ(oneloc, twoloc) == 0)
				equ = TRUE;
			if (oneloc == NULLCP
			    && twoloc == NULLCP
			    && loc1 && loc2
			    && lexequ(loc1->ap_obvalue, loc2->ap_obvalue) == 0)
				/* x400 encapsulated addrs */
				equ = TRUE;
			if (oneloc != NULLCP) free(oneloc);
			if (twoloc != NULLCP) free(twoloc);
		} else if (loc1 && loc2 && 
			   lexequ(loc1->ap_obvalue, loc2->ap_obvalue) == 0)
			equ = TRUE;

	}
	(void) ap_sqdelete(one_tree, NULLAP);
	(void) ap_sqdelete(two_tree, NULLAP);
	return equ;
}
		    
	    
				   
