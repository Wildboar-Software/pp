/* ap_s2s: converts a string into a normalise string with given order pref */
/*	ap_s2t + ap_normalise + ap_t2s */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_s2s.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_s2s.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_s2s.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"ap.h"

ap_s2s (orig, str_pptr, order_pref)
char	*orig,
	**str_pptr;
int	order_pref;
{
	AP_ptr	tree,
		rettree;

	tree = ap_s2t(orig);
	tree = ap_normalize(tree, order_pref);
	rettree = ap_t2s(tree, str_pptr);
	ap_free(tree);
	ap_free(rettree);
}
