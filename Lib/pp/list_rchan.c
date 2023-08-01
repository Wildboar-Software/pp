/* list_rchan.c: utility routines for struct LIST_REFORMAT */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/list_rchan.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/list_rchan.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: list_rchan.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "list_rchan.h"



/* ---------------------  Begin  Routines  -------------------------------- */


/*
Creates a new list structure
*/

LIST_RCHAN*  list_rchan_new (site, chan)
char    *site;
char    *chan;
{
	register LIST_RCHAN      *list;

	PP_DBG (("Lib/pp/list_rchan_new (%s, %s)", 
		 site ? site : "<none>",
		 chan ? chan : "<none>"));

	if (site == NULLCP && chan == NULLCP)
		return (NULLIST_RCHAN);

	list = (LIST_RCHAN *) smalloc (sizeof *list);

	if (chan) {
		list -> li_chan  = ch_nm2struct (chan);
		if (list -> li_chan == NULLCHAN) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Chan %s not known", chan));
			free ((char *)list);
			return (NULLIST_RCHAN);
		}
		PP_DBG (("Channel '%s' added", list -> li_chan -> ch_name));
	}
	else list -> li_chan = NULLCHAN;

	if (site)
		list -> li_mta = strdup (site);
	else	list -> li_mta = NULLCP;
	list -> li_auth = NULL;
	list -> li_next = NULL;

	return (list);
}



/*
Sets the channel part in the list structure
*/

int list_rchan_schan (list, chan)
LIST_RCHAN      *list;
char            *chan;
{

	PP_DBG (("Lib/pp/list_rchan_schan (%s)", chan));

	if (chan == NULLCP)  return (NOTOK);

	if ((list -> li_chan  = ch_nm2struct (chan)) == NULLCHAN)
		return (NOTOK);

	return (OK);
}



/*
Sets the site/mta info of the list structure
*/


int list_rchan_ssite (list, site)
LIST_RCHAN      *list;
char            *site;
{

	PP_DBG (("Lib/pp/list_rchan_ssite (%s)", site));

	if (site == NULLCP)  return (NOTOK);
	list -> li_mta = strdup (site);
	return (OK);
}



/*
Adds item onto the end of base
*/

void list_rchan_add (base, new)
LIST_RCHAN      **base,
		*new;
{
	register LIST_RCHAN      *rp = *base;

	if (rp == NULLIST_RCHAN)
		*base = new;
	else {
		while (rp->li_next != NULLIST_RCHAN)
			rp = rp->li_next;
		rp->li_next = new;
	}
}

/*
Do not free the CHAN ptr because set by ch_all[]
*/

void list_rchan_free (list)
LIST_RCHAN      *list;
{
	if (list == NULLIST_RCHAN)  return;
	if (list -> li_next)
		list_rchan_free (list->li_next);
	if (list -> li_mta)
		free (list -> li_mta);

	free ((char *)list);
}
