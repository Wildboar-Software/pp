/* or_util.c: or-name utility routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_util.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_util.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_util.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



/*
OR Name handling Utilities
*/

#include "util.h"
#include "or.h"

char *or_type2name (type)
int             type;
{
    typestruct  *ptr;

    PP_DBG (("or_util.c/or_type2name()"));

    for (ptr = typetab88; (ptr -> ty_string) != NULLCP; ptr++)
	if (type == ptr->ty_int)
		return (ptr->ty_string);

    PP_LOG(LLOG_EXCEPTIONS,
	   ("BOGUS Unknown type '%d'", type));
    return NULLCP;
}

int or_name2type (name)
char            *name;
{
    typestruct  *ptr;

    PP_DBG (("or_util.c/or_name2type (%s)", name ? name : "null"));

    if (!isstr (name))
		return NOTOK;

    for (ptr = typetab88; (ptr -> ty_string) != NULLCP; ptr++)
	if (lexequ (name, ptr->ty_string) == 0)
		return (ptr->ty_int);

    return NOTOK;
}

/*
Strcat + dot quoting
*/

void dstrcat (s1, s2)
char    *s1;
char    *s2;
{
     register char *p, *q;

     p = s1 + strlen (s1);
     for (q = s2; *q != '\0';)
	switch (*q) {
	case '.':
	case '"':
	case '\\':
		/*
		Shouldn't be any backslashes tho...
		*/
		*p++ = '\\';
	default:
		*p++ = *q++;
		break;
	}
     *p = '\0';

     PP_DBG (("or_util.c/dstrcat(%s, %s)", s1, s2));
}

/*
Strcat + dollar quoting
*/

void qstrcat (s1, s2)
char    *s1;
char    *s2;
{
    register char       *p,
			*q;

    PP_DBG (("qstrcat()"));

    p = s1 + strlen (s1);
    for (q = s2; *q != '\0';)
	switch (*q) {
	case '/':
	case '=':
		*p++ = '$';
	default:
		*p++ = *q++;
		break;
	}
    *p = '\0';
}

int or_gettoken (str, delim, buf)
char    **str;
char    delim;
char    *buf;
{

    char        *ind = *str,
		*x;

    x = buf;
    if (*ind == '\0')
	return NOTOK;
    for (; (*ind != delim) && (*ind != '\0'); ind++, buf++)
	 *buf = *ind;
    if (*ind != '\0')  ind++;
    *str = ind;
    *buf = '\0';

    PP_DBG (("or_util.c/or_gettoken (%s)", x));

    return OK;
}

int or_ddvalid_chk (key, buf)
char    *key;
char    *buf;
{
	int     retval;

	switch (retval = cmd_srch (key, ortbl_ddvalid)) {
	    case OR_DDVALID_RFC822:
	    case OR_DDVALID_X40088:
	    case OR_DDVALID_JNT:
	    case OR_DDVALID_UUCP:
	    case OR_DDVALID_LIST:
	    case OR_DDVALID_ROLE:
	    case OR_DDVALID_FAX:
	    case OR_DDVALID_ATTN:
		(void) strcpy (buf, rcmd_srch (retval, ortbl_ddvalid));
		return OK;
	    default:
		return NOTOK;
	}

	/*NOTREACHED*/
}

int or_str_isps (str)
char *str;
{
	while (*str && or_isps (*str))
		str ++;
	return *str == '\0';
}

int or_str_isns (str)
char *str;
{
	int seen_digit = 0;
	while (*str) {
		if (!or_isns(*str))
			return 0;
		if (isdigit (*str))
			seen_digit = 1;
		str ++;
	}
	return seen_digit;
}

int or_type2charset84 (type)
int type;
{
    typestruct  *ptr;

    PP_DBG (("or_util.c/or_type2charset()"));

    for (ptr = typetab; (ptr -> ty_string) != NULLCP; ptr++)
	if (type == ptr->ty_int)
		return (ptr->ty_charset);

    return 0;
}

int or_type2charset (type)
int type;
{
    typestruct  *ptr;

    PP_DBG (("or_util.c/or_type2charset()"));

    for (ptr = typetab88; (ptr -> ty_string) != NULLCP; ptr++)
	if (type == ptr->ty_int)
		return (ptr->ty_charset);

    return 0;
}

int or_checktypes (or, buf)
OR_ptr or;
char *buf;
{
	char	*name;
	for (; or != NULLOR; or = or -> or_next) {
		switch (or -> or_type) {
		    case OR_OU1:
		    case OR_OU2:
		    case OR_OU3:
		    case OR_OU4:
			or -> or_type = OR_OU;
		    default:
			break;
		}

		switch (or -> or_encoding) {
		    case OR_ENC_PS:
			if (!or_str_isps (or -> or_value)) {
				if ((name = or_type2name (or -> or_type)) != NULLCP)
					(void) sprintf (buf, "Attribute %s=%s is not a Printable String",
							name,
							or -> or_value);
				else
					(void) sprintf (buf, "Attribute %d=%s is not a Printable String",
							or -> or_type,
							or -> or_value);
				return NOTOK;
			}
			break;
		    case OR_ENC_NUM:
			if (!or_str_isns (or -> or_value)) {
				if ((name = or_type2name (or -> or_type)) != NULLCP)
					(void) sprintf (buf, "Attribute %s=%s in not a Numeric String",
							name,
							or -> or_value);
				else
					(void) sprintf (buf, "Attribute %d=%s in not a Numeric String",
							or -> or_type,
							or -> or_value);
				return NOTOK;
			}
			break;
		    case OR_ENC_TTX:
		    case OR_ENC_TTX_AND_OR_PS:
		    case OR_ENC_INT:
		    case OR_ENC_PSAP:
		    default:
			break;
		}
	}
	return OK;
}

/*  */

void or_chk_admd (or)
OR_ptr	*or;
{
	OR_ptr	ptr;
	
	if ((ptr = or_find (*or, OR_ADMD, NULLCP)) == NULLOR) {
		if (or_find (*or, OR_C, NULLCP) != NULLOR
		    && or_find (*or, OR_PRMD, NULLCP) != NULLOR)
			*or = or_add (*or,
				      or_new (OR_ADMD, " ", " "),
				      TRUE);
	} else if (ptr -> or_value == NULLCP
		   || *(ptr -> or_value) == '\0') {
		if (ptr -> or_value)
			free (ptr -> or_value);
		ptr -> or_value = strdup(" ");
	}
}
