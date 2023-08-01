/* tb_getchan.c - maps a Next Hop hostname into its outgoing channel pairs */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getchan.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getchan.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_getchan.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "list_rchan.h"



static Table            *Channel = NULLTBL;
static int		CH_str2rchan ();
static int 		CH_doparse ();

/* ---------------------  Begin  Routines --------------------------------- */




/* --- *** Start Description *** ---

tb_getchan (key, rp)
	char            *key;
	LIST_RCHAN      **rp;

Parameters required:
	- The Next Hop hostname (key)
	- A pointer to the list of channel pairs used to get to the
	  Next Hop hostname (rp)

Example:
	key     =  torch.co.uk
	rp      =  li_mta = ukc.ac.uk     li_chan = JANET
		   li_mta = stl.stc.co.uk li_chan = PSS

Routine:
	- Reads the channel table
	- Updates rp
	- Returns OK or NOTOK.


Note:
	- This routine always fills in ad_outchan with hostnames in US order.


--- *** End Description *** --- */




/* ------------------------------------------------------------------------ */


extern	char	*channel_tbl;

int tb_getchan (key, rp)
char                    *key;
LIST_RCHAN              **rp;
{
	char            tbuf[BUFSIZ];

	PP_DBG (("Lib/table/tb_getchan ('%s')", key));

	if (Channel == NULLTBL)
		if ((Channel = tb_nm2struct (channel_tbl)) == NULLTBL) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Lib/tb_getchan channel table not found"));
			return NOTOK ;
		}

	if (tb_k2val (Channel, key, tbuf, TRUE) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/tb_getchan key '%s' not found.POSSIBLE INTERNAL INCONSISTENCY BETWEEN TABLES", key));
		return NOTOK ;
	}

	if (*rp != NULLIST_RCHAN) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/tb_getchan list is already set!"));
		return NOTOK ;
	}

	/* -- free all memory -- */
	if (CH_str2rchan (rp, tbuf) == NOTOK) {
		PP_DBG (("Lib/table/tb_getchan free list!"));
		list_rchan_free (*rp);
		*rp = NULLIST_RCHAN;
		return NOTOK ;
	}

	return OK;
}




/* ---------------------  Static  Routines  ------------------------------- */



static int CH_str2rchan (rp, str)
LIST_RCHAN              **rp;
char                    *str;
{
	LIST_RCHAN      *list;
	char            *host = NULLCP,
			*chan = NULLCP,
			*next = NULLCP;
	int             retval;


	PP_DBG (("Lib/table/CH_str2rchan ('%s')", str));

	*rp = NULLIST_RCHAN;

	while ((retval = CH_doparse (str, &host, &chan, &next)) == OK) {
		if ((list = list_rchan_new(host, chan)) != NULLIST_RCHAN)
			list_rchan_add (rp, list);

		if (next) 
			str = next; 
		else 
			break;
	}

	if (retval == NOTOK || *rp == NULLIST_RCHAN)
		return NOTOK;
	return OK;
}



static int CH_doparse (str, host, chan, next)
char            *str;
char            **host;
char            **chan;
char            **next;
{
	char *cp;

	PP_DBG (("Lib/table/CH_doparse ('%s')", str));

	if ((cp = index (str, ',')) == NULLCP) 
		*next = NULLCP;  
	else 
		*next = ++cp; 

	*host = str;
	if ((cp = index (str, '(')) == NULLCP)
		return NOTOK;
	if (*host == cp)
		*host = NULLCP;
	else
		*cp = '\0';
	cp++;
	*chan = cp;

	if ((cp = index (*chan, ')')) == NULLCP)
		return NOTOK;
	*cp  = '\0';

	return OK;
}
