/* ch_nm2struct.c: convert channel name into structure */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/ch_nm2struct.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/ch_nm2struct.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: ch_nm2struct.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "chan.h"
#include "list_bpt.h"

extern char chrcnv[];
#define LEXEQU(a,b)	(chrcnv[(a)[0]] == chrcnv[(b)[0]] && lexequ((a),(b)) ==0)

CHAN* ch_nm2struct (name)
register char    *name;
{
	register CHAN   **chp, *chan;

	if (ch_all == (CHAN **)0)
		return (NULLCHAN);

	for (chp = ch_all; (chan = *chp) != NULLCHAN; chp++)
		if (LEXEQU (name, chan->ch_name) ||
		    (chan->ch_key && list_bpt_find (chan->ch_key, name) != NULLIST_BPT))
			return (chan);
	return (NULLCHAN);
}

CHAN   	*ch_mta2struct (name, mta)
register char	*name;
char	*mta;
{
	register CHAN	**cpp, *chan;
	CHAN *good_match = NULLCHAN;
	/* not actually used just there cos needed */
	char	*subdom = NULLCP, chan_key[BUFSIZ], normalised[BUFSIZ];
	
	if (ch_all == (CHAN **)0)
		return (NULLCHAN);

	for (cpp = ch_all; (chan = *cpp) != NULLCHAN; cpp++) {
		if (! LEXEQU (name, chan->ch_name) &&
		    list_bpt_find (chan -> ch_key, name) == NULLIST_BPT)
			continue;

		if (chan -> ch_mta_table == NULL)
			good_match = chan;
		else if (mta &&
			 tb_getdomain_table (chan -> ch_mta_table,
					   mta, chan_key, normalised,
					   chan -> ch_ad_order, &subdom) == OK) {
			if (subdom) free(subdom);
			return chan;
		}
		if (subdom) {
			free(subdom);
			subdom = NULLCP;
		}
	}
	
	return good_match;
}
