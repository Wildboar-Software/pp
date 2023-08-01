/* or_add.c: add an orname component to the list */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_add.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_add.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_add.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "or.h"

OR_ptr or_add (tree, or, before)
OR_ptr          tree;
OR_ptr          or;
int             before;    /* if true then add equal values at front */
{
    OR_ptr      current;
    char	*name;
    
    /* this routine is horrible */
    /* MUST be able to rewrite more efficiently */

    PP_DBG (("or_util.c/or_add ('%d', '%s')",
	  or -> or_type, or -> or_value));

    if (or == NULLOR)
	    return tree;

    if (tree == NULLOR) {
	or -> or_next = NULLOR;
	or -> or_prev = NULLOR;
	return (or);
    }

    if (or -> or_type == OR_OU) {
	    int	ou_count = 1;
	    /* check for exceding upper limit */
	    for (current = tree; 
		 current != NULLOR && current->or_type <= OR_OU; 
		 current = current -> or_next) 
		    if (current->or_type == OR_OU)
			    ou_count++;
	    if (ou_count > MAX_OU) {
		    (void) or_lose("Exceeds upperbound on the number of OrganizataionalUnits (%d OUs)",
			    MAX_OU);
		    return NULLOR;
	    }
    } else if (or -> or_type == OR_DD) {
	    int	dda_count = 1;
	    /* check for exceding upper limit */
	    for (current = tree; 
		 current != NULLOR && current->or_type <= OR_DD;
		 current = current -> or_next) 
		    if (current->or_type == OR_DD)
			    dda_count++;
	    if (dda_count > MAX_DDA) {
		    (void) or_lose("Exceeds upperbound on the number of Domain Defined Attributes (%d DDAs)",
			    MAX_DDA);
		    return NULLOR;
	    }
    }


    if (or -> or_type < tree -> or_type) {
	or -> or_next = tree;
	tree -> or_prev = or;
	or -> or_prev = NULLOR;
	return (or);
    }


    if (or -> or_type == tree -> or_type) {

	if (or -> or_type != OR_OU && or -> or_type != OR_DD) {
		if ((name = or_type2name (or -> or_type)) == NULLCP) 
			(void) or_lose("Illegal duplicate component type '%d' (%s & %s)",
				 or -> or_type,
				 or -> or_value, tree -> or_value);
		else
			(void) or_lose("Illegal duplicate component type '%s' (%s & %s)",
				 name,
				 or -> or_value, tree -> or_value);

		return NULLOR;
	}

	if (before) {
		or -> or_next = tree;
		tree -> or_prev = or;
		or -> or_prev = NULLOR;
		return (or);
	}
    }


    for (current = tree; current != NULLOR; current = current -> or_next) {

	if (current -> or_next == NULLOR) {
		current -> or_next = or;
		or -> or_prev = current;
		or -> or_next = NULLOR;
		return (tree);
	}

	if (or -> or_type < current -> or_next -> or_type) {
		or -> or_next = current -> or_next;
		current -> or_next -> or_prev = or;
		or -> or_prev = current;
		current -> or_next = or;
		return (tree);
	}

	if (or -> or_type == current -> or_next -> or_type) {

		if (or -> or_type != OR_OU && or -> or_type != OR_DD) {
			if ((name = or_type2name (or -> or_type)) == NULLCP) 
				(void) or_lose("Illegal duplicate type '%d' (%s & %s)",
					or->or_type,
					or->or_value, current->or_value);
			else
				(void) or_lose ("Illegal duplicate type '%s' (%s & %s)",
					 name,
					 or -> or_value, 
					 current -> or_value);
			return NULLOR;
		}

		if (before) {
			or -> or_next = current -> or_next;
			current -> or_next -> or_prev = or;
			or -> or_prev = current;
			current -> or_next = or;
			return (tree);
		}

	}			/* end of if */

    }  /* end of for */


   PP_LOG (LLOG_EXCEPTIONS, ("or_util.c/or_add () - serious problem"));
   return NULLOR;
}

OR_ptr or_add_t61 (tree, type, value,len, before)
OR_ptr tree;
int	type;
unsigned char	*value;
int before;
{
	OR_ptr or;
	char buf[BUFSIZ];
	char buf2[BUFSIZ];

	or_t61encode (buf, value, len);
	if ((or = or_locate (tree, type)) == NULLOR) {
		(void) sprintf (buf2, "*%s", buf);
		or = or_new_aux (type, NULLCP, buf2, OR_ENC_TTX);
		return or_add (tree, or, before);
	}

	if (or -> or_encoding != OR_ENC_PS) {
		PP_LOG(LLOG_EXCEPTIONS, ("Encoding not plain PS"));
		return NULLOR;
	}

	(void) sprintf (buf2, "%s*%s", or -> or_value, buf);
	or -> or_encoding = OR_ENC_TTX_AND_OR_PS;
	if (or -> or_value)
		free (or -> or_value);
	or -> or_value = strdup (buf2);
	return tree;
}

void or_t61encode (buf, str, len)
char	*buf;
unsigned char *str;
int len;
{
	int n;
	int inbracket = 0;
	char *p = buf;

	for (n = 0; n < len; n++) {
		if (*str < 128 && or_isps(*str)) {
			if (inbracket) {
				*p ++ = '}';
				inbracket = 0;
			}
			*p ++ = *str ++;
		}
		else {
			if (! inbracket) {
				inbracket = 1;
				*p++ = '{';
			}
			*p++ = (*str) / 100 + '0';
			*p++ = (unsigned char) ((*str) % 100) / 10 + '0';
			*p++ = (*str++) % 10 + '0';
		}
	}
	if (inbracket)
		*p++ = '}';
	*p = 0;
}

struct qbuf *or_t61decode (str)
char	*str;
{
	unsigned char buf[BUFSIZ];
	unsigned char *p;
	int n;
	int inbracket = 0;

	for (n = 0, p = buf; *str; n++) {
		if (inbracket) {
			if (*str == '}') {
				inbracket = 0;
				str ++;
			}
			else {
				*p = (*str++ - '0') * 100;
				*p += (*str++ - '0') * 10;
				*p++ += (*str++ - '0');
			}
		}
		else {
			if (*str == '{') {
				inbracket = 1;
				str ++;
			}
			else
				*p++ = *str++;
		}
	}
	p = 0;
	return str2qb ((char *)buf, n, 1);
}
