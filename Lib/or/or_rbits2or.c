/* or_rbits2or.c: maps bits into OR */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_rbits2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_rbits2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_rbits2or.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"
#include "table.h"

static Table    *tb_rfc2or = NULLTBL;

extern char     *rfc2or_tbl,
		or_error[];
extern OR_upperbound ortbl_88_ubs[];

/* ---------------------  Begin  Routines  -------------------------------- */



int or_domain2or (domain, or)
char	*domain;
OR_ptr	*or;
{
	OR_ptr      ptr, tmp;
	char        buf[LINESIZE],
	*p;

	PP_DBG (("Lib/or_domain2or (%s)", domain));

	if (tb_rfc2or == NULLTBL) {
		if ((tb_rfc2or = tb_nm2struct (rfc2or_tbl)) == NULLTBL) {
			PP_LOG (LLOG_EXCEPTIONS, ("Lib/or_rbits2or: rfc2or table NULL!"));
			return NOTOK;
		}
	}

	for (p = domain; *p != '\0';) {
		if (tb_k2val (tb_rfc2or, p, buf, TRUE) == OK)
			break;
		p = index (p, '.');
		if (p == NULLCP)
			break;
		else
			p++;
	}

	if (p == NULLCP || *p == '\0') {
		(void) sprintf (or_error,
				"No translation of '%s' to X400 address",
				domain);
		return NOTOK;
	}

	else {
		*or = or_dmn2or (buf);
		if (*or == NULLOR)
			return or_lose ("format error '%s':'%s'",
					p, buf);



		if (p != domain) {
			/* --- Must add more bits --- */
			int         i;
			int         tconst;
			int         type;
			int         oargc;
			char        *oargv[50];

			p--;
			*p = '\0';
			oargc = sstr2arg (domain, 49, oargv, ".");

			if (*or != NULLOR) {
				ptr = *or;
				tconst = (or_lastpart (ptr)) -> or_type;
			}
			else
				tconst = OR_OU;

			for (i = oargc - 1; i >= 0; i--) {
				tconst++;
				if (tconst > OR_OU)
					type = OR_OU;
				else
					type = tconst;

				if (!or_isdomsyntax (oargv[i]))
					return or_lose ("domain syntax error '%s'",
							oargv[i]);

				ptr = or_new (type, NULLCP, oargv[i]);
				if (or_check_upper (ptr, ortbl_88_ubs) == NOTOK) {
					if (ptr)
						or_free(ptr);
					return NOTOK;
				}
				tmp = or_add (*or, ptr, FALSE);
				if (tmp == NULLOR)
					return NOTOK;
				else
					*or = tmp;

			}	/* --- end of for --- */

		}		/* --- end of if --- */

	} 			/* --- end of else --- */
    
	return OK;

}
	
int or_local2or (local, or)
char	*local;
OR_ptr	*or;
{
	OR_ptr	ptr;
	/* --- Add in the local bits --- */

	PP_DBG (("Lib/or_local2or (%s)", local));

	if ((ptr = or_std2or (local)) == NULLOR)
		return or_lose ("local syntax error '%s'",
				local);

	*or = or_default_aux(ptr, *or);
	/* try adding in loc_or for missing bits */
	*or = or_default (*or);

	if (or_delete_atsigns (or) == NOTOK)
		return NOTOK;
	return OK;
}

