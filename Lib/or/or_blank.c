/* or_blank.c: do strange things with blanks. */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_blank.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_blank.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_blank.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"

int or_blank (or)
OR_ptr          *or;
{
    OR_ptr      ptr;

    PP_DBG (("Lib/or_blankor (i.e no addr given)"));

    *or = or_new (OR_S, "", "");
    ptr = or_new (OR_OU, "", "");
    *or = or_add (*or, ptr, TRUE);

    if (*or == NULLOR)
	return (NOTOK);

    ptr = or_new (OR_O, "", "");
    *or = or_add (*or, ptr, TRUE);

    if (*or == NULLOR)
	return (NOTOK);

    ptr = or_new (OR_PRMD, NULLCP, "");
    *or = or_add (*or, ptr, TRUE);

    if (*or == NULLOR)
	return (NOTOK);

    ptr = or_new (OR_ADMD, NULLCP, "");
    *or = or_add (*or, ptr, TRUE);

    if (*or == NULLOR)
	return (NOTOK);

    ptr = or_new (OR_C, NULLCP, "");
    *or = or_add (*or, ptr, TRUE);

    if (*or == NULLOR)
	return (NOTOK);

    return (OK);
}
