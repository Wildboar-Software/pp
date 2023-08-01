/* ap_norm.c: normali[zs]e an address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_norm.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_norm.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_norm.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "chan.h"
#include "ap.h"

extern char *loc_dom_site;
extern int ap_lex_percent;

static void ap_ptinit ();
/* ---------------------  Begin  Routines  -------------------------------- */


/*
Normalize a parse tree, to fill-in host references, etc.
	Returns:
		0  if ok
		-1 if error
*/

int	all_domain_norm = FALSE;

ap_norm_all_domains()
{
	all_domain_norm = TRUE;
}

ap_norm_first_domain()
{
	all_domain_norm = FALSE;
}

AP_ptr ap_normalize (thetree, order_pref)
	AP_ptr          thetree;        /* the parse tree */
	int             order_pref;
{
	struct ap_node  base_node;      /* first node in routing chain */
	AP_ptr          r822_prefptr,
			person_ptr,
			mbx_prefptr,
			dom_prefptr,
			lst_comment_prefptr,
			last_ptr,
			group_ptr,
			ap;
	int		oneNorm = FALSE;

	ap_ninit (&base_node);

	(void) ap_sqinsert (&base_node, AP_PTR_MORE, thetree);

	ap_ptinit (&base_node, &person_ptr, &r822_prefptr, &mbx_prefptr,
		   &dom_prefptr, &lst_comment_prefptr, &last_ptr, &group_ptr);

	/*
	Remember "list: ;"...
	*/

	if (mbx_prefptr == NULLAP) {
		PP_DBG (("Lib/addr/ap_normalize/No mailbox found!"));
		return (base_node.ap_next);
	}


	/*
	Normalize all refs in source route
	*/

	if (r822_prefptr != NULLAP) {
		ap = r822_prefptr;
		for(;;) {
			switch (ap -> ap_ptrtype) {
			case AP_PTR_NIL:
				break;
			case AP_PTR_NXT:
				break;
			case  AP_PTR_MORE:
				/* -- next is part of this address -- */
				ap = ap -> ap_next;

				switch (ap -> ap_obtype) {
				case AP_DOMAIN:
					(void) ap_dmnormalize (ap, order_pref);
					if (all_domain_norm == TRUE)
						continue;
					oneNorm = TRUE;
					break;
				case AP_DOMAIN_LITERAL:
				case AP_COMMENT:
					continue;
				}
			}
			break;
		}
	}



	if (ap_lex_percent != TRUE) {
		/*
		  if JNT mail, % is treated as lexically equivalent
		  to @, and CSNET style routes ignored.  Might accept
		  CSNET routes if someone wants this...
		  */

		if (dom_prefptr == NULLAP) {
			/* -- no domain, so add default and leave -- */
			PP_DBG (("Lib/addr/ap_normalize/no domain"));
			ap_locnormalize (&base_node, &r822_prefptr,
					 &mbx_prefptr, &dom_prefptr);
		}
	}


	if ((dom_prefptr != NULLAP) &&
	    (oneNorm == FALSE) &&
	    (ap_dmnormalize (dom_prefptr -> ap_next, order_pref) == OK)) {
			PP_DBG (("Lib/addr/ap_normali/Local reference"));
			if (ap_lex_percent != TRUE)
				ap_locnormalize (&base_node, &r822_prefptr,
						 &mbx_prefptr, &dom_prefptr);
	}


	if (dom_prefptr == NULLAP && r822_prefptr == NULLAP) {
		/* -- no host references anywhere -- */

		if (mbx_prefptr == NULLAP ||
		    mbx_prefptr -> ap_next -> ap_obtype == AP_GROUP_NAME ||
		    mbx_prefptr -> ap_next -> ap_obtype == AP_GROUP_START)
				return (base_node.ap_next);

		(void) ap_append (mbx_prefptr -> ap_next, AP_DOMAIN, loc_dom_site);

		PP_DBG (("Lib/addr/ap_normalize/appending %s domain",
			loc_dom_site));

		(void) ap_dmnormalize (mbx_prefptr -> ap_next -> ap_next,
				       order_pref);
	}

	return (base_node.ap_next);
}



int ap_dmnormalize (ap, order_pref)
AP_ptr          ap;
int             order_pref;
{
	char    official[LINESIZE], *subdom;
	int     retval;


	if (ap == NULL)
		return NOTOK;

	retval = tb_getdomain (ap->ap_obvalue, NULLCP, official, 
			       order_pref, &subdom);
	if (subdom != NULLCP) free(subdom);

	if (retval == OK) {
		if (official[0]) {
			free (ap -> ap_obvalue);
			ap -> ap_obvalue = strdup (official);
			PP_DBG (("Lib/addr/ap_dmnormalize/Official = '%s'",
				official));
		}
	}
	else {
		PP_LOG (LLOG_EXCEPTIONS, 
			("%s is not a known domain reference",
			 ap -> ap_obvalue));

		return (OK);
		/*  This is not worth choking on */
		/* SEK */
	}


	return (retval);

}



/* ---------------------  Static  Routines  ------------------------------- */



static void  ap_ptinit (base_prefptr, person_ptr, r822_prefptr, mbx_prefptr,
		  dom_prefptr, lst_comment_prefptr, last_ptr, group_ptr)

	AP_ptr  base_prefptr,
		*person_ptr,
		*r822_prefptr,
		*mbx_prefptr,
		*dom_prefptr,
		*lst_comment_prefptr,
		*last_ptr,
		*group_ptr;
{
	AP_ptr  ap;


	*person_ptr             = NULLAP;
	*r822_prefptr           = NULLAP;
	*mbx_prefptr            = NULLAP;
	*dom_prefptr            = NULLAP;
	*lst_comment_prefptr    = NULLAP;
	*last_ptr               = NULLAP;
	*group_ptr              = NULLAP;


	/*
	Need switch here to catch leading mbox or domain
	*/
	if (base_prefptr == NULLAP 
	    || base_prefptr -> ap_next == NULLAP)
		return;
	switch (base_prefptr -> ap_next -> ap_obtype) {
	case AP_MAILBOX:
		*mbx_prefptr = base_prefptr;
		break;
	case AP_DOMAIN_LITERAL:
	case AP_DOMAIN:
		*r822_prefptr = base_prefptr;
		break;
	}


	for (ap = base_prefptr -> ap_next; ap -> ap_obtype != AP_NIL;
	     ap = ap -> ap_next) {

		*last_ptr = ap;

		PP_DBG (("Lib/addr/ap_ptinit/val '%s'", ap -> ap_obvalue));

		switch (ap -> ap_obtype) {
		case AP_PERSON_NAME:
			PP_DBG (("Lib/addr/ap_ptinit/person_ptr"));
			*person_ptr = ap;
			break;
		case AP_GROUP_NAME:
			PP_DBG (("Lib/addr/ap_ptinit/group_ptr"));
			*group_ptr = ap;
			break;
		}


		if (ap -> ap_ptrtype == AP_PTR_NXT)     break;
		if (ap -> ap_ptrtype == AP_PTR_NIL)     break;
		if (ap -> ap_next == NULLAP)            break;


		switch (ap -> ap_next -> ap_obtype) {
		case AP_COMMENT:
			PP_DBG (("Lib/addr/ap_ptinit/got comment"));
			*lst_comment_prefptr = ap;
			break;

		case AP_GENERIC_WORD:
			PP_DBG (("Lib/addr/ap_ptinit/got word"));
			if (*mbx_prefptr != NULLAP)
			break;

		case AP_MAILBOX:
			PP_DBG (("Lib/addr/ap_ptinit/got mbox pref"));
			/* -- one before the mbox -- */
			*mbx_prefptr = ap;
			break;

		case AP_DOMAIN_LITERAL:
		case AP_DOMAIN:
			if(*r822_prefptr == NULLAP && *mbx_prefptr == NULLAP){
				PP_DBG (("Lib/addr/ap_ptinit/got r822_prefptr"
				       ));
				*r822_prefptr = ap;
			}
			else if ((*dom_prefptr == NULLAP) &&
				 (*mbx_prefptr != NULLAP)) {
					/* -- need mailbox befor domain -- */
					PP_DBG (("%s%s",
						"Lib/addr/ap_ptinit/",
						"got dom_prefptr"));
					*dom_prefptr = ap;
			     }

			break;
		}
	}



    if (*last_ptr == NULLAP)
	PP_DBG (("Lib/addr/ap_ptinit/no last_ptr"));
    else
	PP_DBG (("Lib/addr/ap_ptinit/last_ptr '%s'",
		(*last_ptr) -> ap_obvalue));
}



/* ---------------------  Not JNTMAIL   ----------------------------------- */


ap_locnormalize (obase_ptr, or822_prefptr, ombx_prefptr, odom_prefptr)
AP_ptr                          obase_ptr,
				*or822_prefptr,
				*ombx_prefptr,
				*odom_prefptr;
{
	/* -- tear local-part apart -- */

	struct ap_node          base_node;
	AP_ptr                  cur_ptr;
	AP_ptr                  r822_prefptr,
				person_ptr,
				mbx_prefptr,
				dom_prefptr,
				lst_comment_prefptr,
				last_ptr,
				group_ptr;

	PP_DBG (("Lib/addr/ap_locnormalize/parsing '%s'",
			(*ombx_prefptr) -> ap_next -> ap_obvalue));

	ap_ninit (&base_node);

	(void) ap_sqinsert (&base_node, AP_PTR_MORE,
			    ap_s2t ((*ombx_prefptr) -> ap_next -> ap_obvalue));

	ap_ptinit (&base_node, &person_ptr, &r822_prefptr,
		   &mbx_prefptr, &dom_prefptr, &lst_comment_prefptr,
		   &last_ptr, &group_ptr);


	if (dom_prefptr != NULLAP) {
		/* -- actually have some stuff -- */

		/* -- replace old reference -- */
		free ((*ombx_prefptr) -> ap_next -> ap_obvalue);

		(*ombx_prefptr) -> ap_next -> ap_obvalue =
				strdup (mbx_prefptr -> ap_next -> ap_obvalue);

		PP_DBG (("Lib/addr/ap_locnormalize/newlocal '%s'",
			      (*ombx_prefptr) -> ap_next -> ap_obvalue));

		if (r822_prefptr != NULLAP || *odom_prefptr != NULLAP)
			if (*or822_prefptr == NULLAP)
				/* -- initialize route pointer -- */
				*or822_prefptr = obase_ptr;


		if (*odom_prefptr == NULLAP)
			*odom_prefptr = mbx_prefptr -> ap_next;
		else
			/* -- get rid of old domain reference -- */
			(void) ap_move (*or822_prefptr, *odom_prefptr);


		if (r822_prefptr != NULLAP) {
			/* -- put new chain at end of old -- */
			PP_DBG (("%s%s",
				 "Lib/addr/ap_locnormalize/",
				 "adding new routing to end info"));

			for (cur_ptr = *or822_prefptr;
			     cur_ptr -> ap_next -> ap_obtype
					== AP_DOMAIN ||
			     cur_ptr -> ap_next -> ap_obtype
					== AP_DOMAIN_LITERAL ||
			     cur_ptr -> ap_next -> ap_obtype
					== AP_COMMENT;
			     cur_ptr = cur_ptr -> ap_next)
				continue;

			(void) ap_sqmove (cur_ptr, r822_prefptr, AP_DOMAIN);
		}

		(void) ap_move (*odom_prefptr, dom_prefptr);
	}
}
