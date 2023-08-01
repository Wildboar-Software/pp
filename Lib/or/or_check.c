/* or_check.c: check and normalise or names */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_check.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_check.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_check.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "or.h"
#include "table.h"
#include <isode/cmd_srch.h>

extern CMD_TABLE ortbl_rfc822ddas[];
extern char     *or_tbl;
static Table    *tb_or;
char            or_error[BUFSIZ] = "";


/* ---------------------  Begin  Routines  -------------------------------- */


/* --- *** ---
or_check (or, buf, buf_type, locname)  where:
	or              -       what is being checked
	buf             -       what is returned
	buf_type        -       Channel or local
	lsubdom		-	local proferred table
	replace		-	if synonym'd place to put replacement or
--- *** --- */

#define	MAX_NUM_REPLACES	25
#define WILDCARD		"*"

static int or_fill_wilds_and_swap();

static int allRFC822DDA (or)
OR_ptr	or;
{
	while (or != NULLOR
	       && or->or_type == OR_DD
	       && cmd_srch(or->or_ddname, ortbl_rfc822ddas) != -1)
		or = or->or_next;
	if (or == NULLOR)
		return TRUE;
	return FALSE;
}

int or_check (or, buf, buf_type, lsubdom, replace, lastMatch)
OR_ptr          *or;
char            *buf;
int             *buf_type;
char		**lsubdom;
char		**replace;
OR_ptr		*lastMatch;
{
	OR_ptr          tptr,
			sptr,
			current_ptr,
			tree;
	char            current_val[LINESIZE],
			tbuf[LINESIZE],
			tracebuf[LINESIZE],
			*oargv[10];
	int             oargc;
	char		first_lookup[BUFSIZ];
	int		postReplace = FALSE,
			num_replaces = 0,
			aliased = FALSE;

	PP_DBG (("Lib/or_chk()"));

	first_lookup[0] ='\0';
	if (replace != (char **) NULL)
		*replace = NULLCP;

	*lastMatch = NULLOR;
	*lsubdom = NULLCP;
	if ((tptr = loc_ortree) == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS, ("Lib/or_chk: loc_ortree missing"));
		return NOTOK;
	}

	if ((current_ptr = tree = or_tdup(*or)) == NULLOR)
		return or_lose ("No OR specified for checking");

	
	if (tb_or == NULLTBL) {
		if ((tb_or = tb_nm2struct (or_tbl)) == NULLTBL)
			return or_lose ("No or table!!!");
	}

	or_or2std (tree, tracebuf, FALSE);
	PP_TRACE (("Lib/or_chk's tracebuf: '%s'", tracebuf));

	buf[0] = '\0';

	for (;;) {
		or_or2dmn (NULLOR, current_ptr, current_val);
		PP_DBG (("Lib/or_chk: Checking part: '%s'", current_val));


		if (tb_k2val (tb_or, current_val, tbuf, TRUE) != OK) {
			char	*holder = NULLCP;
			char	prev_current_val[LINESIZE];
			(void) strcpy(prev_current_val, current_val);
			if (current_ptr != NULLOR) {
				holder = current_ptr -> or_value;
				current_ptr -> or_value = strdup(WILDCARD);
				or_or2dmn (NULLOR, current_ptr, current_val);
			}
			if (current_ptr != NULLOR
			    && tb_k2val (tb_or, current_val, tbuf, TRUE) == OK)  {
				if (holder)
					free(holder);
			} else {
				if (current_ptr) {
					free(current_ptr -> or_value);
					current_ptr -> or_value = holder;
				}
				if (buf[0] == '\0' && postReplace == FALSE) {
					char	*orargv[10], tmp[BUFSIZ];
					char    *dollar;
					(void) strcpy(current_val, prev_current_val);
					PP_DBG (("Lib/or_chk: Check failed '%s' '%s'",
						 current_val, tracebuf));
					if (first_lookup[0] != '\0')
						or_or2std(or_lastpart(tree), tmp, FALSE);
					else
						tmp[0] = '\0';

					if (sstr2arg (current_val, 9, orargv, ".")) {
						if ((dollar = index (orargv[0], '$')) != NULLCP)
							*dollar = ' ';
						if (first_lookup[0] == '\0'
						    || lexequ(first_lookup, tmp) == 0) 
							(void) or_lose (
									"Unknown '%s' in '%s'",
									orargv[0], tracebuf);
						else
							(void) or_lose (
									"Unknown '%s' in '%s' (aka '%s')",
									orargv[0], tmp, first_lookup);
					}
					else {
						if (first_lookup[0] == '\0'
						    || lexequ(first_lookup, tmp) == 0)
							(void) or_lose (
									"Unknown '%s' in '%s'",
									current_val, tracebuf);
						else
							(void) or_lose (
									"Unknown '%s' in '%s' (aka '%s')",
									current_val, tmp, first_lookup);
					}
					return NOTOK;
				}
				else if (buf[0] == '\0' && postReplace == TRUE) {
					/* no direct match with replacement */
					/* rewind checking and start again */
					current_ptr = tree;
					postReplace = FALSE;
					continue;
				}
				else {
					if (or_fill_wilds_and_swap(or, tree) != OK) {
						(void) or_lose ("Local wildcard OR problems for '%s'",
							 tracebuf);
						return NOTOK;
					}
					or_chk_admd(or);
					return OK;
				}
			}
		}

		postReplace = FALSE;

		PP_DBG (("Lib/or_chk: Hit: '%s'", tbuf));

		/* --- Have found a pointer --- */
		oargc = sstr2arg (tbuf, 9, oargv, " \t\n,");

		if (lexequ (oargv[0], "valid") == 0) {
			buf[0] = '\0';
			current_ptr = current_ptr -> or_next;
			continue;
		}


		if (lexequ (oargv[0], "local") == 0) {
			/* --- This is local - map rest to buf --- */
			if (current_ptr -> or_next == NULLOR) 
				return or_lose (
						"local entry found - but no more components '%s'",
						tracebuf);
			else if (allRFC822DDA(current_ptr -> or_next) == TRUE) {	
				tptr = current_ptr->or_next;
				tptr->or_prev = NULLOR;
				or_or2std(tptr, buf, TRUE);
				tptr->or_prev = current_ptr;
				return or_lose(
					       "unknown local user '%s'",
					       buf);
			}

			*lastMatch = tptr = current_ptr;
			current_ptr = current_ptr -> or_next;
			current_ptr ->  or_prev = NULLOR;

			if (current_ptr -> or_type <= OR_OU
			    || (or_getpn (current_ptr, buf) == FALSE))
				or_or2std (current_ptr, buf, TRUE);
				
			current_ptr -> or_prev = tptr;
			*buf_type = OR_ISLOCAL;

			PP_TRACE (("Lib/or_chk: Returning local: '%s' '%s' ",
				buf, tracebuf));
			if (oargc > 1)
				*lsubdom = strdup (oargv[1]);
			continue;
		}  /* --- end of if local --- */


		if (oargc < 2) 
			return or_lose ("Format error in OR table '%s' '%s'",
					current_val, tracebuf);


		if (lexequ (oargv[0], "mta") == 0) {
			(void) strcpy (buf, oargv[1]);
			*buf_type = OR_ISMTA;
			*lastMatch = current_ptr;
			current_ptr = current_ptr -> or_next;
			continue;
		}


		if (lexequ (oargv[0], "realalias") == 0) {
			/* --- invalidate previous hits --- */
			buf[0] = '\0';
			postReplace = TRUE;
			aliased = TRUE;
			if (++num_replaces > MAX_NUM_REPLACES)
				return or_lose ("Too many aliases/synonyms when resolving '%s'",
						tracebuf);

			if ((tptr = or_dmn2or (oargv[1])) == NULLOR)
				return or_lose ("Invalid syntax alias '%s' '%s'",
						oargv[1], tracebuf);
			
			if (first_lookup[0] == '\0')
				or_or2std(or_lastpart(tree), first_lookup, FALSE);

			/* --- free off old head --- */
			if ((sptr = current_ptr -> or_next) != NULLOR) {
				sptr -> or_prev         = NULLOR;
				current_ptr -> or_next  = NULLOR;
			} 
			or_free (tree);

			/* --- add in the new stuff --- */
			tree                     = tptr;
			current_ptr             = or_lastpart (tptr);

			if (sptr != NULLOR) {
				/* --- add back in the rest of old stuff */
				if (sptr -> or_type < current_ptr -> or_type)
					return or_lose ("Alias out of order '%s' '%s'",
							oargv[1], tracebuf);

				current_ptr -> or_next  = sptr;
				sptr -> or_prev         = current_ptr;
			}
			continue;
		}  /* --- end of if alias --- */

		if (lexequ (oargv[0], "synonym") == 0
		    || lexequ (oargv[0], "alias") == 0) {
			/* --- invalidate previous hits --- */
			buf[0] = '\0';
			postReplace = TRUE;

			if (++num_replaces > MAX_NUM_REPLACES)
				return or_lose ("Too many aliases/synonyms when resolving '%s'",
					tracebuf);

			if ((tptr = or_dmn2or (oargv[1])) == NULLOR)
				return or_lose (
					"Invalid syntax alias '%s' '%s'",
					oargv[1], tracebuf);
			
			if (first_lookup[0] == '\0')
				or_or2std(or_lastpart(tree), first_lookup, FALSE);

			/* --- free off old head --- */
			if ((sptr = current_ptr -> or_next) != NULLOR) {
				sptr -> or_prev         = NULLOR;
				current_ptr -> or_next  = NULLOR;
			} 
			or_free (tree);

			/* --- add in the new stuff --- */
			tree                     = tptr;
			current_ptr             = or_lastpart (tptr);

			if (sptr != NULLOR) {
				/* --- add back in the rest of old stuff */
				if (sptr -> or_type < current_ptr -> or_type)
					return or_lose (
							"Alias out of order '%s' '%s'",
							oargv[1], tracebuf);

				current_ptr -> or_next  = sptr;
				sptr -> or_prev         = current_ptr;
			}
			if (aliased == FALSE
			    && replace != (char **) NULL) {
				char	tmp[BUFSIZ];
				if (*replace != NULLCP)
					free(*replace);
				or_or2std(or_lastpart(tree), tmp, FALSE);
				*replace = strdup(tmp);
			}
			continue;
		}  /* --- end of if synonym --- */

		return or_lose ("Unknown alias type '%s' '%s'",
				oargv[1], tracebuf);
	}  /* end of for */
}

/*  */

static int or_fill_wilds_and_swap(por, tree)
OR_ptr	*por, tree;
{
	/* take WILDCARDS from por and replace in tree */
	/* then swap trees */

	OR_ptr	ixor, ixt;
	int	numOU1 = 0, numOU2 = 0;

	for (ixt = tree, ixor = *por; ixt != NULLOR; ixt = ixt->or_next) {
		if (lexequ(ixt->or_value, WILDCARD) == 0) {
			if (ixt->or_type == OR_OU) {
				while (ixor != NULLOR
				       && (ixor -> or_type != ixt -> or_type
					   || numOU1 != numOU2)) {
					if (ixor -> or_type == OR_OU)
						numOU2++;
					ixor = ixor -> or_next;
				}
			} else {
				while (ixor != NULLOR && 
				       ixor -> or_type != ixt -> or_type) {
					if (ixor -> or_type == OR_OU)
						numOU2++;
					ixor = ixor -> or_next;
				}
			}
			if (ixor == NULLOR) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Unable to find original valure for wildcard '%s'",
					 or_type2name(ixt -> or_type)));
				return NOTOK;
			}
			free (ixt -> or_value);
			ixt -> or_value = strdup(ixor -> or_value);
			ixor = ixor -> or_next;
		}
		if (ixt->or_type == OR_OU)
			numOU1++;
	}

	or_free(*por);

	*por = tree;
	return OK;
}
