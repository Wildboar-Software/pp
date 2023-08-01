/* list_bpt.c: utility routines for Body Part Types */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/list_bpt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/list_bpt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: list_bpt.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "list_bpt.h"




/* ---------------------  Begin  Routines  -------------------------------- */




/*
Creates a new list structure
*/

LIST_BPT* list_bpt_new (value)
char    *value;
{
	register LIST_BPT        *list;


	PP_DBG (("Lib/pp/list_bpt_new (%s)", value));

	if (value == NULLCP)
		return (NULLIST_BPT);

	list = (LIST_BPT *)smalloc (sizeof *list);

	list -> li_next = NULL;
	list -> li_name = strdup (value);

	return (list);

}




/*
Duplicates
*/

LIST_BPT  *list_bpt_dup (list)
LIST_BPT                *list;
{
	LIST_BPT *retlist = NULLIST_BPT;
	register LIST_BPT *new;

	for (; list != NULLIST_BPT; list = list->li_next) {
		new = (LIST_BPT *) smalloc (sizeof *list);
		new -> li_name = strdup (list->li_name);
		new -> li_next = NULL;
		list_bpt_add (&retlist, new);
	}

	return (retlist);
}

/*
Adds item onto the end of base
*/

void list_bpt_add (base, new)
LIST_BPT                **base,
			*new;
{
	register LIST_BPT        *ep = *base;

	if (ep == NULLIST_BPT)
		*base = new;
	else {
		while (ep->li_next != NULLIST_BPT)
			ep = ep->li_next;
		ep->li_next = new;
	}
}


/*
Finds a specified item within a list
*/

LIST_BPT  *list_bpt_find (list, item)
LIST_BPT        *list;
char            *item;
{
	register LIST_BPT        *lp;

	PP_DBG (("Lib/pp/list_bpt_find (%s)", item));

	for (lp = list; lp != NULLIST_BPT; lp = lp->li_next)
		if (lexequ (lp -> li_name, item) == 0)
			return (lp);

	return (NULLIST_BPT);
}

LIST_BPT  *list_bpt_nfind (list, item, len)
LIST_BPT        *list;
char            *item;
int		len;
{
	register LIST_BPT        *lp;

	PP_DBG (("Lib/pp/list_bpt_find (%s)", item));

	for (lp = list; lp != NULLIST_BPT; lp = lp->li_next)
		if (lexnequ (lp -> li_name, item, len) == 0)
			return (lp);

	return (NULLIST_BPT);
}

void list_bpt_free (list)
LIST_BPT    *list;
{
	if (list == NULLIST_BPT)  return;
	list_bpt_free (list->li_next);
	if (list -> li_name)
		free (list->li_name);
	free ((char *)list);
}
