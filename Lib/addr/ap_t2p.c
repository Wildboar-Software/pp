/* ap_t2p.c: split tree into major address parts */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_t2p.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_t2p.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_t2p.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



/*
Record beginnings of major sections of an address

    Returns:    pointer to next node, after address
		 0 if end of tree
		-1 if error

if both rroute and nroute are requested, and the tree has routing
in reverse-path form, then these will point to the same node.
*/



#include "util.h"
#include "ap.h"


static int      person_level,
		group_level;



/* ---------------------  Begin  Routines  -------------------------------- */



AP_ptr ap_t2p (tree, group, name, local, domain, route)
register AP_ptr         tree;           /* -- the parse tree -- */
AP_ptr                  *group,         /* -- start of group name -- */
			*name,          /* -- start of person name -- */
			*local,         /* -- start of local-part -- */
			*domain,        /* -- basic domain reference -- */
			*route;         /* -- start of 822 reverse route -- */
{
	AP_ptr          return_ptr,
			start_ptr = tree;
	int             got_local, got_first;

	PP_DBG (("Lib/addr/ap_t2p()"));
	
	group_level = 0;
	person_level = 0;

	if (group  != (AP_ptr *) 0)             *group   = NULLAP;
	if (name   != (AP_ptr *) 0)             *name    = NULLAP;
	if (local  != (AP_ptr *) 0)             *local   = NULLAP;
	if (domain != (AP_ptr *) 0)             *domain  = NULLAP;
	if (route  != (AP_ptr *) 0)             *route   = NULLAP;

	/* -- ignore null stuff -- */
	if (tree == NULLAP || tree -> ap_obtype == AP_NIL)
		return ((AP_ptr) OK);

	got_first = FALSE;

	for (got_local = FALSE; ; tree = tree -> ap_next) {
		/* -- print munged addr -- */
		switch (tree -> ap_obtype) {
		    case AP_NIL:
			return_ptr = NULLAP;
			goto endit;

		    case AP_PERSON_NAME:
			person_level++;
			if (name != (AP_ptr *) 0 && *name == NULLAP) { 
				*name = start_ptr;
				got_first = TRUE;
			}
			start_ptr = tree -> ap_next;
			break;

		    case AP_PERSON_START:
			start_ptr = tree -> ap_next;
			break;

		    case AP_PERSON_END:
			person_level--;
			start_ptr = tree -> ap_next;
			break;

		    case AP_GROUP_NAME:
			group_level++;
			if (group != (AP_ptr *) 0 && *group == NULLAP) {
				*group = start_ptr;
				got_first = TRUE;
			}
			start_ptr = tree -> ap_next;
			break;

		    case AP_GROUP_START:
			start_ptr = tree -> ap_next;
			break;

		    case AP_GROUP_END:
			group_level--;
			start_ptr = tree -> ap_next;
			break;

		    case AP_GENERIC_WORD:
		    case AP_MAILBOX:
			if (local != (AP_ptr *) 0 && *local == NULLAP) {
				*local = start_ptr;
				got_first = TRUE;
			}
			got_local = TRUE;
			start_ptr = tree -> ap_next;

			break;

		    case AP_DOMAIN_LITERAL:
		    case AP_DOMAIN:
			if (got_local) {
				/* -- reference after local -- */
				if (domain != (AP_ptr *) 0 &&
				    *domain == NULLAP) {
					*domain = start_ptr;
					got_first = TRUE;
				}
			}
			else {
				/* -- must be 822 route -- */
				/* -- domain precedes local-part -- */
				if (route != (AP_ptr *) 0 && *route == NULLAP) {
					*route = start_ptr;
					got_first = TRUE;
				}
			}
			start_ptr = tree -> ap_next;

			break;
		
		    case AP_COMMENT:
			/* -- output value as comment -- */
			if (got_first == FALSE)
				/* cannot lose preceding comments */
				break;
		    default:
			start_ptr = tree -> ap_next;
			break;
		}



		switch (tree -> ap_ptrtype) {
		case AP_PTR_NXT:
			if (tree -> ap_next -> ap_obtype != AP_NIL) {
				return_ptr = (AP_ptr) tree -> ap_next;
				goto endit;
			}

			/* -- else drop on through -- */

		case AP_PTR_NIL:
			return_ptr = (AP_ptr) OK;
			goto endit;
		}
	}

endit:

	return (return_ptr);
}
