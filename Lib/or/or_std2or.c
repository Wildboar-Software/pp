/* or_std2or.c: convert standard or representation to tree */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_std2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_std2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_std2or.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"

extern char     or_error[];
/*
Input standard to form orname
*/

static OR_ptr	or_std2or_aux();

OR_ptr or_std2or (str)
char            *str;
{
	OR_ptr	tree;
	char	*p, tmp[BUFSIZ];
	int	firstpn;
	PP_DBG (("or_util.c/or_std2or (%s)", str));

	if (str == NULLCP)
		return NULLOR;

	tree = NULLOR;

	if (str[0] == '/') {
		/*
		  No implicit PN
		  */
		firstpn = FALSE;
		p = &str[1];
	}
	else {
		firstpn = TRUE;
		p = str;
	}

	if ((tree = or_std2or_aux(p, '/', firstpn)) == NULLOR) {
		char	*colon, *equal;
		char	or_error_sav[BUFSIZ];
		
		(void) strcpy (or_error_sav, or_error);

		/* ignore leading semicolon */
		if (str[0] == ';')
			p = &str[1];
		else
			p = str;
		
		colon = index(p, ';');
		equal = index(p, '=');
		
		if (equal == NULLCP)
			/* no equals */
			firstpn = TRUE;
		else if (colon == NULLCP)
			/* no semicolon but equals */
			firstpn = FALSE;
		else if (colon < equal)
			/* semicolon before equals */

			firstpn = TRUE;
		else
			firstpn = FALSE;

		if ((tree = or_std2or_aux(p, ';', firstpn)) == NULLOR) {
			(void) strcpy (or_error, or_error_sav);
			return NULLOR;
		}
	}

	or_chk_admd(&tree);
	if (or_checktypes(tree, tmp) == NOTOK) {
		or_free(tree);
		(void) or_lose("Bad OR Address '%s'", tmp);
		return NULLOR;
	}

	return tree;
}

static OR_ptr	or_std2or_aux(str, sep, firstpn)
char	*str;
char	sep;
int	firstpn;
{
	OR_ptr	tree = NULLOR,
		or = NULLOR;
	char	keybuf[LINESIZE],
		valbuf[LINESIZE],
		tbuf[LINESIZE],
		*p,
		*r;
	int	ortype,
		before = TRUE;

	p = str;
    
	for (; *p != '\0';) {
		r = keybuf;
		for (; *p != '\0';) {
			if (*p == sep) {
				if (!firstpn) {
					(void) or_lose ("Bad AV Syntax '%s'",
							str);
					or_free (tree);
					return NULLOR;
				}
				p++;

			} else {
				switch (*p) {
				    case '=':
					if (firstpn) {
						(void) or_lose ("Bad PN syntax '%s'", str);
						return NULLOR;
					}
					p++;
					break;
				    case '$':
					p++;
				    default:
					*r++ = *p++;
					continue;
				}
			}
			break;
		}

		*r = '\0';

		if (firstpn) {
			tree = or_buildpn (keybuf);
			if (*p == '\0')
				return (tree);
			firstpn = FALSE;
			continue;
		}

		if (*p == '\0') {
			if (keybuf[0] == '\0')
				return (tree);
			(void) or_lose ("Prematurely terminated OR '%s'", str);
			or_free (tree);
			return NULLOR;
		}

		r = valbuf;

		for (; *p != '\0';) {
			if (*p != sep) {
				switch (*p) {
				    case '=':
					(void) or_lose("Bad AV syntax '%s'",str);
					or_free (tree);
					return NULLOR;
				    case '$':
					p++;
				    default:
					*r++ = *p++;
					continue;
				}
			}
			break;
		}

		*r = '\0';

		if (*p != sep) 
			PP_DBG (("Prematurely ended OR - no slash '%s'", str));
		else
			p++;

		PP_DBG ((
			 "or_util.c/or_std2or: Component '%s' = '%s'", keybuf, valbuf));

		/*
		  Process string from BACK to  optimise or-add
		  */

		switch (ortype = or_name2type (keybuf)) {
#ifdef NOTDEF
		    case OR_C:
		    case OR_ADMD:
		    case OR_PRMD:
#endif
		    case OR_O:
			before = FALSE;
		    default:
			break;
		}

		if (ortype != NOTOK) {
			or = or_new (ortype, NULLCP, valbuf);
			if (or == NULLOR) return NULLOR;
			if ((tree = or_add (tree, or, before)) == NULLOR)
				return NULLOR;
			continue;
		}

		if (lexequ (keybuf, "pn") == 0) {
			OR_ptr	pntree, orp, ix = NULLOR;
			pntree = or_buildpn (valbuf);
			for (orp = pntree; orp != NULLOR; orp = ix) {
				ix = orp -> or_next;
				orp -> or_next = NULLOR;
				if ((tree = or_add(tree, orp, before)) == NULLOR)
					return NULLOR;
			}
			continue;
		}

		(void) strncpy (tbuf, keybuf, 3);
		tbuf[3] = '\0';

		if (((int)strlen (keybuf) > 2) && lexequ (tbuf, "dd.") == 0) {
			if (!or_str_isps(valbuf)) {
				(void) or_lose("DDA '%s' is not a printable string",
					valbuf);
				or_free(tree);
				return NULLOR;
			}
			if (lexequ(&keybuf[3], "common") == 0) 
				or = or_new (OR_CN, NULLCP, valbuf);
			else
				or = or_new (OR_DD, &keybuf[3], valbuf);
			if (or == NULLOR) return NULLOR;
			if ((tree = or_add (tree, or, TRUE)) == NULLOR)
				return NULLOR;
			continue;
		}

		/*
		  Might need new table if this becomes assymetrical
		  */

		if (or_ddvalid_chk (keybuf, &tbuf[0]) == OK) {
			if (!or_str_isps(valbuf)) {
				(void) or_lose("DDA '%s' is not a printable string",
					valbuf);
				or_free(tree);
				return NULLOR;
			}
			or = or_new (OR_DD, tbuf, valbuf);
			if (or == NULLOR) return NULLOR;
			if ((tree = or_add (tree, or, TRUE)) == NULLOR)
				return NULLOR;
			continue;
		}

		(void) or_lose ("Unknown Key '%s'", keybuf);
		or_free (tree);
		return NULLOR;
	}
	return tree;
}
