/* or_default: add suitable defaults for or structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_default.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_default.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_default.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"

extern void fixup_space();

OR_ptr or_default_aux ();

int or_form_type(or)
OR_ptr	or;
{
    int		or_components[OR_DD+1];	/* must be big enough ... */
    OR_ptr              defptr;

    bzero ((char *)or_components, sizeof or_components);
    for (defptr = or; defptr; defptr = defptr -> or_next) {
	    if (defptr -> or_type > OR_DD)
		    PP_LOG (LLOG_EXCEPTIONS, ("Unknown OR type %d",
					      defptr -> or_type));
	    else
		    or_components[defptr -> or_type] ++;
    }

    /* check for forms of OR address */
    if (or_components[OR_X121])
	    return OR_FORM_TERM;
    if (or_components[OR_ADMD] && or_components[OR_C] &&
	or_components[OR_PD_C] && or_components[OR_POSTCODE])
	    return OR_FORM_POST;
    if (or_components[OR_C] && or_components[OR_ADMD] &&
	or_components[OR_UAID])
	    return OR_FORM_NUMR;
    if (or_components[OR_C] && or_components[OR_ADMD])
	    return OR_FORM_MNEM;
    return OR_FORM_NONE;
}


OR_ptr or_default (or)
OR_ptr  or;
{
	return or_default_aux(or, loc_ortree);
}


OR_ptr or_default_aux (or, merge)
OR_ptr	or, merge;
{
    OR_ptr              defptr;
    OR_ptr              new_tree;
    OR_ptr              tptr = NULLOR;
    int			before;
    char		spaceless[BUFSIZ], defspless[BUFSIZ];

    PP_DBG (("Lib/or_default()"));

    if (or == NULLOR)
	    return or_tdup(merge);

    new_tree = NULLOR;
    switch (or_form_type(or)) {
	case OR_FORM_TERM:
	case OR_FORM_POST:
	case OR_FORM_NUMR:
	case OR_FORM_MNEM:
	    /* recognised form of OrAaddress */
	    /* don't merge */
	    return or_tdup(or);
	default:
	    break;
    }

    for (defptr = merge; defptr != NULLOR; defptr = defptr -> or_next) {
	if (or -> or_type <= defptr -> or_type)
		if (or->or_type == OR_OU) {
			fixup_space(or -> or_value, spaceless);
			fixup_space(defptr -> or_value, defspless);
			if (lexequ (spaceless, defspless) == 0)
				break;
		}
		else
			break;

	tptr = or_dup (defptr);

	if (tptr -> or_type == OR_OU) 
		before = FALSE; 
	else 
		before = TRUE;

	if ((new_tree = or_add (new_tree, tptr, before)) == NULLOR)
		return NULLOR;
    }

    /*
    As we know both are ordered, tptr will be at the end of the tree
    */

    if (new_tree == NULLOR)
	return (or);

    tptr -> or_next = or;
    or -> or_prev = tptr;

    return (new_tree);
}
