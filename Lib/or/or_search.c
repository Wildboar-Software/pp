/* or_search.c: misc functions to do with locating or bits */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_search.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_search.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_search.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"




/* ---------------------  Begin  Routines  -------------------------------- */




OR_ptr or_lastpart (or)
OR_ptr or;
{
    PP_DBG (("or_util.c/or_lastpart()"));

    if (or == NULLOR)
	return NULLOR;

    while (or -> or_next != NULLOR)
	or = or -> or_next;

    return or;
}




OR_ptr or_find (tree, type, ddname)
OR_ptr          tree;
int             type;
char            *ddname;
{
    OR_ptr      ptr;

    PP_DBG (("or_util.c/or_find()"));

    for (ptr = tree; ptr != NULLOR; ptr = ptr -> or_next)
	if (ptr -> or_type == type)  
		if (ptr -> or_type != OR_DD || ddname == NULLCP)
			return ptr;
		else 
			if (lexequ (ptr -> or_ddname, ddname) == 0)
				return ptr;
    return NULLOR;
}




int or_cmp (or1, or2)
OR_ptr          or1;
OR_ptr          or2;
{

    PP_DBG (("or_util.c/or_cmp:  ('%d', '%s', '%s')  ('%d', '%s', '%s')",
	  or1 -> or_type, or1 -> or_ddname, or1 -> or_value,
	  or2 -> or_type, or2 -> or_ddname, or2 -> or_value));

    if (or1 -> or_type != or2 -> or_type
	|| lexequ (or1 -> or_value, or2 -> or_value) != 0)
		return FALSE;

    if (or1 -> or_type == OR_DD
	&& lexequ (or1 -> or_ddname, or2 -> or_ddname) != 0)
		return FALSE;

    return TRUE;
}




/* --- *** ---
or_locate: given a or tree, find the first component
	   of the specified type.
--- *** --- */

OR_ptr  or_locate (or, type)
OR_ptr          or;
int             type;
{
    register OR_ptr   orp;

    PP_DBG (("Lib/or/or_locate (%d)", type));
    for (orp = or; orp != NULLOR && or -> or_type <= type;
	 orp = orp -> or_next)
		if (orp -> or_type == type)
			return orp;
    return NULLOR;
}




/* --- *** ---
or_value: given an or tree, return the value of the first element of a
	  given type.
--- *** --- */

char*   or_value (or, type)
OR_ptr  or;
int     type;
{
    register OR_ptr  orp;

    PP_DBG (("Lib/or/or_value (%d)", type));
    for (orp = or; orp != NULLOR && or -> or_type <= type;
	 orp = orp -> or_next)
		if (type == orp -> or_type)
			return (orp -> or_value);
    return NULLCP;
}




/* --- *** ---
or_add_atsigns: For any missing OR Components insert that
		component as a NULL ("@") attribute
--- *** --- */

int or_add_atsigns (orp)
OR_ptr  *orp;
{
	OR_ptr  or,
		old,
		new;
	int     type;

	PP_DBG (("Lib/or_add_atsigns()"));

	old = *orp;
	or = new = NULLOR;

	if (old == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/or_add_atsigns - No OR specified"));
		return NOTOK;
	}


	for (type = OR_C; type < OR_OU; type++) {

		/* -- add the attribute as NULL ("@") -- */
		if ((or = or_find (old, type, NULLCP)) == NULLOR)
			or = or_new_aux (type, NULLCP, "@", OR_ENC_PP);
		else
			old = old -> or_next;

		if ((new = or_add (new, or, TRUE)) == NULLOR)
			return NOTOK;

		if (old == NULLOR)
			break;
	}


	for (;;) {
		if (old == NULLOR)
			break;
		if (old -> or_type < OR_OU)
			continue;
		or = old;
		old = old -> or_next;
		if (or_add (new, or, FALSE) == NULLOR)
			return NOTOK;
	}

	*orp = new;
	return OK;
}




/* --- *** ---
or_delete_atsigns: For any NULL OR Components delete
		   that component
--- *** --- */

int or_delete_atsigns (ptr)
OR_ptr  *ptr;
{
	OR_ptr  old, top, or,
		bk, fw, rm;

	PP_DBG (("Lib/or_delete_atsigns()"));

	old = top = *ptr;

	if (old == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/or_delete_atsigns - No OR specified"));
		return NOTOK;
	}


	for (or = old; or; or = fw) {
		bk = or -> or_prev;
		fw = or -> or_next;
		rm = or;
		if (or -> or_type >= OR_OU)
			break;
		if (lexequ (or -> or_value, "@") == 0) {
			if (bk) {
				bk -> or_next = fw;
				if (or == *ptr ||
				    (lexequ (top -> or_value, "@") == 0))
						top = fw;
			}
			else
				top = fw;
			fw -> or_prev = bk;
			rm -> or_next = rm -> or_prev = NULLOR;
			or_free (rm);
		}
	}


	*ptr = top;
	return OK;
}
