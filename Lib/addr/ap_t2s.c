/* ap_t2s.c: Address tree to String */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_t2s.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_t2s.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_t2s.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



/*
Format one address from parse tree, into a string

    Returns:    pointer to next node, after address
		 0 if end of tree
		NOTOK if error
*/


#include "util.h"
#include "ap.h"




/* ---------------------  Begin  Routines  -------------------------------- */


AP_ptr ap_t2s (thetree, str_pptr)
AP_ptr          thetree;        /* -- the parse tree -- */
char            **str_pptr;     /* -- where to stuff the string -- */
{
	AP_ptr  loc_ptr,        /* -- in case fake personal name needed -- */
		group_ptr,
		name_ptr,
		dom_ptr,
		route_ptr,
		return_tree;

	return_tree = ap_t2p (thetree, &group_ptr, &name_ptr,
				  &loc_ptr, &dom_ptr, &route_ptr);

	if (return_tree == BADAP) {
		PP_LOG (LLOG_EXCEPTIONS,
				("Lib/addr/ap_t2s: error from ap_t2p()"));
		*str_pptr = strdup ("(PP Error!)");
		return (BADAP);
	}

	*str_pptr = ap_p2s (group_ptr, name_ptr, loc_ptr, dom_ptr, route_ptr);

	if (*str_pptr == (char *)NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
				("Lib/addr/ap_t2s: error from ap_p2s()"));
		*str_pptr = strdup ("(PP Error!)");
		return (BADAP);
	}

	return (return_tree);
}
