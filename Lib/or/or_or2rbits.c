/* or_or2rbits.c: break or into bits */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2rbits.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2rbits.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_or2rbits.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"
#include "table.h"

extern char     *loc_dom_site;
extern char     or_error[];
extern char     *or2rfc_tbl;
static Table    *tb_or2rfc;
static int	or_or2rbits_ckdmn(), or_isOKforRHS();
extern void	fixup_space();

/* --- definitions --- */
#define NO_MATCH		0
#define OU_O_MATCH		1
#define PRMD_ADMD_C_MATCH	2


/* -- local routines -- */
int		or_or2rbits();
static		get_psORasc();




/* ---------------------  Begin  Routines  -------------------------------- */




/* --- Get local + domain from or --- */

#ifdef NOTNEEDED
static char	*at_str = "@";
#endif

int or_or2rbits (or, local, domain)
OR_ptr  or;
char    *local;
char    *domain;
{
	OR_ptr          ptr,
			dptr,
			tptr;
	char            buf[LINESIZE],
			tbuf[LINESIZE],
			tmp[LINESIZE],
			loc_ps[LINESIZE], /* --- Printable String --- */
			dom_ps[LINESIZE]; /* --- Printable String --- */
	int		cont, or_form;
#ifdef NOTNEEDED
	int		atmatch = FALSE;
#endif

	PP_DBG (("Lib/or_or2rbits()"));

	if (tb_or2rfc == NULLTBL)
		if ((tb_or2rfc = tb_nm2struct (or2rfc_tbl)) == NULLTBL)
			return or_lose ("No or2rfc table!");

	or_form = or_form_type(or);
	domain[0] = '\0';
	if (or_add_atsigns (&or) == NOTOK)
		return NOTOK;


	/* --- See how much we can domain check against --- */

	for (ptr = or; ptr -> or_next != NULLOR; ptr = ptr -> or_next)
		if (ptr -> or_type > OR_OU)
			break;


	/* --- pointing to the first non-dom compt --- */

	if (ptr != or) {
		dptr = ptr;
		ptr = ptr -> or_prev;

		/* --- move until we find a match --- */

		for (; ptr != NULLOR; ptr = ptr->or_prev) {
			or_or2dmn (NULLOR, ptr, buf);

			if (tb_k2val (tb_or2rfc, buf, tbuf, TRUE) != OK)
				continue;

			/* got a match */

			/* --- check that tbuf is a valid domain syntax --- */
			if (or_or2rbits_ckdmn (tbuf) == NOTOK)
				goto doexplicit;

			switch (or_form) {
			    case OR_FORM_TERM:
			    case OR_FORM_POST:
			    case OR_FORM_NUMR:
				/* fully lhs encode */
				(void) strcpy (dom_ps, tbuf);
				get_psORasc (dom_ps, domain);
				if (index(domain, '.') == NULLCP)
					/* only one component to domain */
					/* so LHS encode with loc_dom_site */
					domain[0] = '\0';
				goto doexplicit;
			    default:
			    case OR_FORM_MNEM:
				break;
			}

			/* ptr is ones we've matched */
			/* dptr is end of components that may be added to RHS */
			
			ptr = ptr -> or_next;
			cont = TRUE;
			
			(void) strcpy (dom_ps, tbuf);

			while (cont == TRUE
			       && ptr != dptr) {
				if (lexequ (ptr -> or_value, "@") == 0) {
					/* missing next component */
					/* lhs encode from here */
					cont = FALSE;
					continue;
				}
				
				fixup_space(ptr -> or_value, tmp);
				if (or_isOKforRHS(tmp)) {
					(void) sprintf(tbuf, "%s.%s", tmp, dom_ps);
					(void) strcpy(dom_ps, tbuf);
					ptr = ptr -> or_next;
				} else 
					cont = FALSE;
			}
			
			/* appended what could to rhs */
			/* lhs encode from here */
			
			tptr = ptr -> or_prev;
			ptr -> or_prev = NULLOR;
			
			if (or_delete_atsigns (&ptr) == NOTOK)
				return NOTOK;

			or_or2std (ptr, loc_ps, TRUE);
			if (loc_ps[strlen (loc_ps) - 1]  == '/')
				or_or2std (ptr, loc_ps, FALSE);


			ptr -> or_prev = tptr;
			tptr -> or_next = ptr;

			get_psORasc (dom_ps, domain);

			if (index(domain, '.') == NULLCP) {
				/* only one component to domain */
				/* so LHS encode */
				domain[0] = '\0';
				goto doexplicit;
			}
			get_psORasc (loc_ps, local);

			PP_DBG ((
			"Lib/or_or2rbits DMN encoded local='%s', dmn='%s'",
			local, domain
			));

			if (or_delete_atsigns (&or) == NOTOK)
				return NOTOK;

			return OK;

		}   /* end of for */

	} /* end of if */


doexplicit:
	/* --- Do full LHS encoding --- */
	if (or_delete_atsigns (&or) == NOTOK)
		return NOTOK;

	if (domain[0] == '\0')
		(void) strcpy (domain, loc_dom_site);
	or_or2std (or, loc_ps, FALSE);
	get_psORasc (loc_ps, local);	

	PP_DBG (("Lib/or_or2rbits/Default setting local='%s' domain='%s'",
		local, domain));

	return OK;
}




/* --------------------------  Static  Routines  ---------------------------- */

static int or_isOKforRHS (dmn)
char	*dmn;
{
	char	asc[BUFSIZ], *ix;
	get_psORasc(dmn, asc);

	if (!isalnum(asc[0]) || 
	    !isalnum(asc[strlen(asc)-1]))
		return FALSE;
	    
	for (ix = &asc[0]; 
	     ix != NULL && *ix != '\0';
	     ix++)
		if (!isalnum(*ix) 
		    && *ix != '-')
			return FALSE;
	return TRUE;
}

static int or_or2rbits_ckdmn (dmn)
char	*dmn; 
{
	char	buf[LINESIZE];
	char	*ptr = dmn;

	PP_DBG (("Lib/or_or2rbits_ckdmn (%s)", dmn));

	buf[0] = '\0'; 

	while (or_gettoken (&ptr, '.', buf) == OK)
		if (!or_isOKforRHS (buf))
			return or_lose ("Bad style RFC-822 domain '%s'", dmn);
	return OK;
} 



#ifdef NOTNEEDED
static int or_or2rbits_match (start, dmn)
OR_ptr  start;
OR_ptr  *dmn;
{
	char	buf[LINESIZE];
	OR_ptr  curr;


	/* --- *** ---
	Build domain - match. If attribute value does not hold a valid domain 
	syntax, then move that part to the local part. This is only done if 
	no mapping for that attribute has been specified in the or2rfc table.
	Example:
		'/s=user/ou=org unit/prmd=valid/admd=gold 400/c=gb/'
	will be RFC987 mapped to:
		'"/s=user/ou=org unit/"@valid.gold-400.gb'
	--- *** --- */

	for (curr = *dmn; curr != start; curr = curr -> or_prev) {
		get_psORasc (curr -> or_value, buf);
		fixup_space(buf, buf);
		if ((!or_isOKforRHS (buf)) && (lexequ (buf, "@") != 0)) {
			if (curr -> or_prev)
				*dmn = curr -> or_prev;
			else
				return or_lose ("No OR prev '%s'",
						curr->or_value);
		}
	}

	return OK;
}

/* --- match for "@" signs in the or2rfc table --- */

static int or_or2rbits_atsigns (or)
OR_ptr	or;
{
	OR_ptr	next = or -> or_next;	/* save the value */
	int	type = or -> or_type;
	OR_ptr	at;
	char	key[LINESIZE], value[LINESIZE]; 
	int	retval;


	if (type < OR_OU)
		type++;
	else
		return NO_MATCH;

	at = or_new (type, NULLCP, "@");

	at -> or_prev = or;
	at -> or_next = NULLOR;
	or -> or_next = at;	


	or_or2dmn (NULLOR, at, key);	

	retval = tb_k2val (tb_or2rfc, key, value, TRUE);

	or -> or_next = next;

	at -> or_prev = at -> or_next = NULLOR;
	or_free (at);

	if (retval != OK)
		return NO_MATCH;

	if (type < OR_O)
		return PRMD_ADMD_C_MATCH;

	return OU_O_MATCH;
}
#endif


static get_psORasc (value, buf)
char	*value;
char	*buf;
{

#ifndef STRICT_1148
	if (!(value[0] == '/'
	    && index(value, '=') != NULLCP))
		(void) or_ps2asc (value, buf);
	else
#endif
	(void) strcpy (buf, value);
}
