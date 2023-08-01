/* or_up_bnds.c: routine to check length of O/R components fit in with standards */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_up_bnds.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_up_bnds.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_up_bnds.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */

#include "or.h"

static int get_ub(or, ubs)
OR_ptr	or;
OR_upperbound	*ubs;
{
	int	i;
	
	for (i = 0; ubs[i].or_type != 0; i++) {
		if (ubs[i].or_type == or->or_type)
			return ubs[i].or_upperbound;
	}
	return 0;
}

int or_check_upper (or, ubs)
OR_ptr	or;
OR_upperbound	*ubs;
{
	int	ub;

	if (or == NULLOR)
		return NOTOK;

	if ((ub = get_ub(or, ubs)) != -1) {
		if (ub == 0) {
			(void) or_lose("Illegal x400 O/R component '%s'",
				or_type2name(or->or_type));
			return NOTOK;
		}
		if (ub < (int) strlen(or->or_value)) {
			(void) or_lose ("Exceeds the upperbound on length of '%s' components (%d chars)",
				 or_type2name(or->or_type),
				 ub);
			return NOTOK;
		}
	}

	if (or->or_type == OR_DD &&
	    (int) strlen (or->or_ddname) > OR_UB_DDA_TYPE) {
		(void) or_lose("'%s' exceeds the upperbound on length of domain defined types (%d chars)",
			or->or_ddname, OR_UB_DDA_TYPE);
		return NOTOK;
	}

	return OK;
}

int or_chk_ubs (or, ubs)
OR_ptr	or;
OR_upperbound	*ubs;
{

	while (or != NULLOR) {
		if (or_check_upper(or, ubs) == NOTOK)
			return NOTOK;
		or = or->or_next;
	}
	return OK;
}
