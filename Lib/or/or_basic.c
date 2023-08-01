/* or_basic.c: basic low level or structure manipulation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_basic.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_basic.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_basic.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"


OR_ptr or_new (type, ddname, value)
int type;
char *ddname;
char *value;
{
	int enc = or_type2charset (type);
	char *cp;
	
	if (strcmp(value, "@") == 0) 
		return or_new_aux(type, ddname, value, OR_ENC_PP);

	switch (enc) {
	    case OR_ENC_TTX_AND_OR_PS:
		for (cp = value; *cp; cp ++) {
			if (!or_isps (*cp)) {
				if (*cp == '*') {
					if(cp == value)
						enc = OR_ENC_TTX;
					/* else stay as you were */
				}
				(void) or_lose ("Bad string syntax %s",
						value);
				return NULLOR;
			}
		}
		if (*cp == '\0')
			enc = OR_ENC_PS;
		break;
	    case OR_ENC_PS:
	    case OR_ENC_NUM:
	    case OR_ENC_TTX:
	    case OR_ENC_INT:
	    case OR_ENC_PSAP:
	    case OR_ENC_PP:
		break;
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Unknown encoding for type %d val %s",
			 type, value));
	}
	return or_new_aux (type, ddname, value, enc);
}
			
OR_ptr or_new_aux (type, ddname, value, encoding)
int     type;
char    *ddname;
char    *value;
int 	encoding;
{
    OR_ptr      or;

    PP_DBG (("or_util.c/or_new ('%d', '%s', '%s')",
	     type, ddname ? ddname : "<none>",
	     value ? value : "<none>"));

    switch (encoding) { /* check the easy ones */
	case OR_ENC_PS:
	    if (!or_str_isps (value)) {
		    (void) or_lose ("Not a printable string %s", value);
		    return NULLOR;
	    }
	    if (type == OR_DD && !or_str_isps (ddname)) {
		    (void) or_lose ("DD type %s not a printable string",
				    ddname);
		    return NULLOR;
	    }
	    break;
	case OR_ENC_NUM:
	    if (!or_str_isns (value)) {
		    (void) or_lose ("Not a numeric string %s", value);
		    return NULLOR;
	    }
	    break;
	default:
	    break;
    }
    or = (OR_ptr) smalloc (sizeof (struct or_part));
    if (or == NULLOR)
	return NULLOR;

    or -> or_type = type;
    or -> or_encoding = encoding;
    or -> or_next = NULLOR;
    or -> or_prev = NULLOR;

    if (type != OR_DD ) {
	or -> or_ddname = NULLCP;
	or -> or_value = strdup (value);
    }
    else {
	or -> or_ddname = strdup (ddname);
	or -> or_value = strdup (value);
    }
    return (or);
}

OR_ptr or_dup (or)
OR_ptr or;
{
    return (or_new_aux (or -> or_type, or -> or_ddname,
			or -> or_value, or -> or_encoding));
}

OR_ptr or_tdup (or)
OR_ptr or;
{
    OR_ptr new = NULLOR;

    if (or == NULLOR)
	return NULLOR;
    new = or_dup (or);
    new -> or_next = or_tdup (or -> or_next);
    if (new -> or_next)
	new -> or_next -> or_prev = new;
    return new;
}

void or_free (tree)
OR_ptr  tree;
{
	if (tree == NULLOR)
		return;

	if (tree -> or_next)
		or_free (tree -> or_next);
	if (tree -> or_value)
		free (tree -> or_value);
	if (tree -> or_ddname)
		free (tree -> or_ddname);
	free ((char *)tree);
}
