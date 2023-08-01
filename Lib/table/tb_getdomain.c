/*
tb_getdomain.c - maps an alias domain into a normalised one and optionally,
	    maps a normalised domain into its next hop out.
*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getdomain.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getdomain.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_getdomain.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "list_rchan.h"
#include <isode/cmd_srch.h>

#define MASK_PREFBIT		1
char		*bits2str();
int		str2bits();
static int	scorebits();
static int 	dom_lookup ();
static int	fillin_results();


/* ---------------------  Begin	 Routines --------------------------------- */



/* --- *** Start Description *** ---

tb_getdomain (dom_key, chan_key, normalised, order_pref, ppsubdom)

	dom_key		-      Could be either an Alias, NRS Short from,
					or a Normalised domain name.
	chan_key	-	The Next Hop hostname. This is used
					as a key to the channel table.
	normalised	-	Normalised version of (dom_key)
	order_pref	-	UK only, USA only, UK pref, USA pref.
	ppsubdom	-	is this a sub domain - if so which

Example:
	dom_key		=	blah.torch.co
	chan_key	=	torch.co.uk
	normalised	=	blah.torch.co.uk
	order_pref	=	CH_UK_PREF

Routine:
	- Reads the domain table
	- Updates normalised.
	- Optionally updates chan_key if ptr given.
	- Returns OK or NOTOK.

Note:
	- This Routine always returns the char* normalised in USA order

--- *** End Description *** --- */



/* ------------------------------------------------------------------------ */

static int loopCount; /* stop infinite recursion for synonyms */
#define MAXLOOPS	10

static int	hitTable;
/*  */
/* interface for general parsing */

int tb_getdomain (dom_key, chan_key, normalised, order_pref, ppsubdom)
char			*dom_key;
char			*chan_key;
char			*normalised;
int			order_pref;
char			**ppsubdom;
{
	Table	*Domain;
	int	retval;

	if ((Domain = tb_nm2struct ("domain")) == NULLTBL) {
		PP_LOG (LLOG_FATAL, 
			("No table 'domain' defined"));
		return (NOTOK);
	}
	
	hitTable = FALSE;
	loopCount = 0;
	if ((retval = tb_getdomain_aux(Domain, dom_key, order_pref,
				       chan_key, normalised, 
				       ppsubdom)) == NOTOK
	    && hitTable == FALSE)
		/* try for default */
		if ((retval = tb_getdomain_aux(Domain, "*", order_pref,
					       chan_key, normalised,
					       ppsubdom)) == OK) {
			(void) strcpy(normalised, dom_key);
			PP_NOTICE(("Using default route for '%s'",
				   dom_key));
		}
	return retval;
}

/*  */
/* interface for mtatable identification */

int tb_getdomain_table (table, dom_key, chan_key,
			normalised, order_pref, ppsubdom)
Table			*table;
char			*dom_key;
char			*chan_key;
char			*normalised;
int			order_pref;
char			**ppsubdom;
{
	loopCount = 0;
	return tb_getdomain_aux(table, dom_key, order_pref,
				chan_key, normalised, ppsubdom);
}

/*  */

int tb_getdomain_aux (table, dom_key, order_pref,
		      chan_key, normalised,
		      ppsubdom)
Table			*table; 	/* domain table */
char			*dom_key;	/* what we're looking up */
int			order_pref;	/* which dmn ordering we allow */
char			*chan_key;	/* routing pointer */
char			*normalised;	/* normalised dmn */
char			**ppsubdom;	/* pointer to local tables */
{
	char	tbuf[BUFSIZ],
		usa_norm[BUFSIZ],
		usa_chan[BUFSIZ],
		uk_norm[BUFSIZ],
		uk_chan[BUFSIZ],
		*usa_ord[LINESIZE],
		*uk_ord[LINESIZE],
		*str;
	char	*uksd = NULLCP, *ussd = NULLCP;
	int	count,
		usa_score,
		uk_score;

	if (dom_key == NULLCP)
		return NOTOK;

	if (*dom_key == '.' || dom_key [strlen (dom_key) - 1] == '.')
		return (NOTOK);

	*ppsubdom = NULLCP;
	*normalised = '\0';
	if (chan_key)
		*chan_key = '\0';

	PP_DBG (("Lib/table/tb_getdomain (domain='%s', order_pref=%d)",
		 dom_key, order_pref));

	/* -- do not destroy input -- */
	(void) strcpy (tbuf, dom_key);


	/* -- get things into known order and count parts of name -- */
	if ((order_pref & CH_UK_ONLY) != 0)
		count = str2bits (tbuf, '.', uk_ord, usa_ord);
	else
		count = str2bits (tbuf, '.', usa_ord, uk_ord);


	/* -- look up the usa order name - exact match -- */

	str = bits2str (usa_ord, '.');
	/* try for foo.bar */
	if (dom_lookup (table, str, order_pref,
			usa_chan, usa_norm, ppsubdom, 
			0, TRUE, usa_ord, count) == OK ||
	    /* try for *.foo.bar */
	    dom_lookup (table, str, order_pref,
			usa_chan, usa_norm, ppsubdom, 
			0, FALSE, usa_ord, count) == OK) {
		fillin_results(usa_norm, usa_chan, normalised, chan_key);
		return OK;
	}
	/* -- ok, if the other way around is allowed - try it -- */
	if ((order_pref & MASK_PREFBIT) != 0) {
		str = bits2str (uk_ord, '.');
		/* try for bar.foo */
		if (dom_lookup (table, str, order_pref, 
				uk_chan, uk_norm, ppsubdom, 
				0, TRUE, uk_ord, count) == OK ||
		    /* try for *.bar.foo */
		    dom_lookup(table, str, order_pref, 
			       uk_chan, uk_norm, ppsubdom,
			       0, FALSE, uk_ord, count) == OK) {
			fillin_results(uk_norm, uk_chan,
				       normalised, chan_key);
			return OK;
		}
	}

	uksd = ussd = NULLCP;

	/* -- score usa order -count the number of matching bits -- */
	usa_score = scorebits (table, usa_ord, count, 
			       usa_norm, usa_chan, 
			       &ussd, order_pref);

	/* -- also score the reverse order if appropriate -- */
	if ((order_pref & MASK_PREFBIT) != 0)
		uk_score = scorebits (table, uk_ord, count, 
				      uk_norm, uk_chan, 
				      &uksd, order_pref);
	else
		uk_score = -1;

	/* -- no matches - can't normalise this thing -- */
	if (uk_score == -1 && usa_score == -1) {
		if (uksd) free(uksd);
		if (ussd) free(ussd);
		return NOTOK;
	}


	/* -- usa order is the best fit -- */
	if (usa_score >= uk_score) {
		*ppsubdom = ussd;
		if (uksd) free(uksd);
		fillin_results(usa_norm, usa_chan, normalised, chan_key);
	} else {
		/* -- uk order is the best fit -- */
		*ppsubdom = uksd;
		if (ussd) free(ussd);
		fillin_results(uk_norm, uk_chan, normalised, chan_key);
	}
	return OK;
}

/*  */
/* find best match ( -1 == no match ) */

static int scorebits (table, comps, numComps, norm, chan, subdom, order_pref)
Table	*table;
char	*comps[];
int	numComps;
char	norm[];
char	chan[];
char	**subdom;
int	order_pref;
{
	int	i;
	char	*tbuf[LINESIZE], *str, **ptr;

	PP_DBG (("Lib/table/scorebits()"));

	for (i = 0, ptr = comps; *ptr != NULL; ptr++)
		tbuf[i++] = *ptr;

	tbuf[i] = NULLCP;

	/* numComps -1 cos already looked up whole domain */
	for (i = numComps-1; i > 0; i--) {
		str = bits2str (&tbuf[numComps - i], '.');
		switch (dom_lookup(table, str, order_pref,
				   chan, norm, subdom, 
				   numComps - i, FALSE,
				   comps, numComps)) {
		    case OK:
		    case DONE:
			/* decreasing i so best hit first hit */
			return i;
		    default:
			break;
		}
	}

	/* no hits */
	norm[0] = '\0';
	if (chan)
		chan[0] = '\0';
	return NOTOK;
}

/*  */

static int check_rule();

static int dom_lookup (table, name, order_pref,
		       chan, norm, subdom, nbits,
		       full, comps, numComps)
Table	*table;		/* table */
char	*name;		/* what we're looking up */
int	order_pref;	/* which dmn ordering we allow */
char	*chan;		/* routing pointer */
char	*norm;		/* normalised dmn */
char	**subdom;	/* pointer to local tables */
int	nbits;		/* number of non matched subdomains */
int	full;		/* looking for * or exact */
char	*comps[];	/* components of dmn */
int	numComps;	/* number of components */
{
	char *vec[10];
	int	vecp, i;
	int	first = TRUE;
	char	buf[BUFSIZ], pnorm[BUFSIZ];
	char	value[BUFSIZ];

	norm[0] = '\0';
	chan[0] = '\0';
	*subdom = NULLCP;

	/* fillin * if not full */	
	if (full == TRUE) 
		(void) strcpy(buf, name);
	else if (name != NULLCP)
		(void) sprintf (buf, "*.%s", name);
	else 	
		/* default route */
		(void) strcpy(buf, "*");

	for (;;) {
		if (tb_k2val (table, buf, value, first) == NOTOK)
			return NOTOK;

		first = 0;
		hitTable = TRUE;

		/* split into different rules */
		if ((vecp = sstr2arg (value, 10, vec, "|")) < 1)
			return NOTOK;
		
		for (i = 0; i < vecp; i++) {
			switch (check_rule(table, vec[i], buf,
				       pnorm, chan, subdom,
				       nbits, full)) {
			    case OK:
				reconstruct_dom(norm, pnorm, numComps - nbits,
						comps, numComps);
				return OK;
			    case DONE:
				if (++loopCount > MAXLOOPS) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("possible synonym loop '%s' in table '%s'",
						buf, table->tb_name));
					return NOTOK;
				}
				if (pnorm[0] == '*') {
					char	*ix;
					ix = pnorm + strlen("*.");
					reconstruct_dom(buf, ix, 
							numComps - nbits,
							comps, numComps);
				} else 
					(void) strcpy(buf, pnorm);
				if (tb_getdomain_aux(table, buf, 
						     order_pref,
						     chan, norm, 
						     subdom) == OK)
					return OK;
				break;
			    default:
				break;
			}
		}
	}
}

/*  */

/* check one rule */

#define	KEY_NORM 1
#define KEY_KEY	2
#define KEY_NORMKEY	3
#define KEY_LOCAL 4
#define KEY_VALID 5
#define KEY_MAX	6
#define KEY_MIN 7
#define KEY_SYNONYM 8

static CMD_TABLE	tbl_domainKeys[] = {
	"norm",		KEY_NORM,
	"mta",		KEY_KEY,
	"norm+mta",	KEY_NORMKEY,
	"mta+norm",	KEY_NORMKEY,
	"local",	KEY_LOCAL,
	"valid",	KEY_VALID,
	"max",		KEY_MAX,
	"min",		KEY_MIN,
	"synonym",	KEY_SYNONYM,
	0,		-1
	};

static int check_rule (table, rule, lhs, norm, chan, subdom, nbits, full)
Table	*table;
char	*rule, *lhs,
	*norm, *chan, **subdom;
int	nbits, full;
{
	int	maxsub, minsub;
	int	vecp;
	char	*vec[10], *val;
	int	synon, i, canUse, normSet;

	synon = FALSE;
	canUse = FALSE;
	normSet = FALSE;

	/* set default upper and lower bounds */
	if (full == TRUE)
		maxsub = minsub = 0;
	else {
		maxsub = NOTOK;
		minsub = 1;
	}

	if ((vecp = sstr2arg (rule, 10, vec, " \t")) < 1)
		return NOTOK;
	
	for (i = 0; i < vecp; i++) {
		
		if (!isstr(vec[i]))
			continue;

		if ((val = index(vec[i], '=')) != NULLCP)
			*val++ = '\0';

		switch (cmd_srch(vec[i], tbl_domainKeys)) {
		    case KEY_SYNONYM:
			if (val == NULLCP || *val == '\0') 
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Illegal format in table '%s' for '%s': No value given with key '%s'",
					table->tb_name, lhs, vec[i]));
			else {
				(void) strcpy(norm, val);
				normSet = TRUE;
				synon = TRUE;
				canUse = TRUE;
			}
			break;
				
		    case KEY_NORM:
		    case KEY_VALID:
			if (val != NULLCP && *val != '\0') {
				(void) strcpy(norm, val);
				normSet = TRUE;
			}
			canUse = TRUE;
			break;

		    case KEY_NORMKEY:
			if (val != NULLCP && *val != '\0') {
				(void) strcpy(norm, val);
				(void) strcpy(chan, val);
			} else {
				/* use lhs as key */
				char	*ix;
				if (lhs[0] == '*'
				    && lhs[1] == '.')
					ix = &(lhs[2]);
				else
					ix = &(lhs[0]);
				(void) strcpy(norm, ix);
				(void) strcpy(chan, ix);
			}
			normSet = TRUE;
			canUse = TRUE;
			break;
				
		    case KEY_KEY:
			if (val == NULLCP || *val == '\0') {
				/* use lhs as key */
				char	*ix;
				if (lhs[0] == '*'
				    && lhs[1] == '.')
					ix = &(lhs[2]);
				else
					ix = &(lhs[0]);
				(void) strcpy(chan, ix);
			} else 
				(void) strcpy(chan, val);
			canUse = TRUE;
			break;

		    case KEY_LOCAL:
			if (val == NULLCP || *val == '\0')
				*subdom = strdup("");
			else
				*subdom = strdup(val);
			canUse = TRUE;
			break;
				
		    case KEY_MAX:
			if (full == TRUE) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Illegal format in table '%s' for '%s': invalid specification of 'max' with exact match",
					table->tb_name, lhs));
				norm[0] = '\0';
				chan[0] = '\0';
				*subdom = NULLCP;
				return NOTOK;
			}
			if (val == NULLCP || *val == '\0')
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Illegal format in table '%s' for '%s': No value given with key '%s'",
					table->tb_name, lhs, vec[i]));
			else {
				if (val[0] == '*')
					maxsub = NOTOK;
				else if (isdigit(val[0]))
					maxsub = atoi(val);
				else
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Illegal format in table '%s' for '%s': Non numeric string as value for key '%s'",
						table->tb_name, lhs, vec[i]));
			}
			break;

		    case KEY_MIN:
			if (full == TRUE) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Illegal format in table '%s' for '%s': invalid specification of 'min' with exact match",
					table->tb_name, lhs));
				norm[0] = '\0';
				chan[0] = '\0';
				*subdom = NULLCP;
				return NOTOK;
			}
			if (val == NULLCP || *val == '\0')
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Illegal format in table '%s' for '%s': No value given with key '%s'",
					table->tb_name, lhs, vec[i]));
			else {
				if (val[0] == '*')
					minsub = NOTOK;
				else if (isdigit(val[0]))
					minsub = atoi(val);
				else
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Illegal format in table '%s' for '%s': Non numeric string as value for key '%s'",
						table->tb_name, lhs, vec[i]));
			}
			break;
				
		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unknown key '%s' in table '%s' for entry '%s'",
				vec[i], table->tb_name, lhs));
			break;
		}
		
	}
	if (canUse == FALSE) 
		PP_LOG(LLOG_EXCEPTIONS,
		       ("entry '%s' in table '%s' has no useful keys (mta,key,local)",
			lhs, table->tb_name));
	else if ((maxsub == NOTOK || maxsub >= nbits)
		 && (minsub == NOTOK || minsub <= nbits)) {

		if (normSet == FALSE) {
			/* use lhs */
			char	*ix;
			if (lhs[0] == '*'
			    && lhs[1] == '.')
				ix = &(lhs[2]);
			else
				ix = &(lhs[0]);
			(void) strcpy(norm, ix);
		}
		
		if (synon == TRUE)
			return DONE;
		else
			return OK;
	}
	return NOTOK;
}

/*  */

static int fillin_results(innorm, inchan,
			  outnorm, outchan)
char	*innorm, *inchan,
	*outnorm, *outchan;
{
	if (outnorm) 
		(void) strcpy(outnorm, innorm);
	if (outchan) {
		if (inchan[0] != '\0')
			(void) strcpy(outchan, inchan);
		else
			/* no norm given */
			outchan[0] = '\0';
	}
}

/*  */

/* reconstruct full domain by adding unmatched components to norm'd */
reconstruct_dom(into, norm, hits, comps, numComps)
char	*into;
char	*norm;
int	hits;
char	*comps[];
int	numComps;
{
	int	i;

	into[0] = '\0';

	for (i = 0; i < numComps - hits; i++) {
		if (into[0] != '\0')
			(void) strcat (into, ".");
		(void) strcat (into, comps[i]);
	}
	if (into[0] != '\0')
		(void) strcat (into, ".");
	(void) strcat(into, norm);
}


/*  */

char *bits2str (ptr, sep)
register char	**ptr;
register char	sep;
{
	char		silly[2];
	static char	tbuf[LINESIZE];

	PP_DBG (("Lib/table/bits2str ('%s')", *ptr));

	silly[0] = sep;
	silly[1] = '\0';

	(void) strcpy (tbuf, *ptr++);

	while (*ptr != NULLCP) {
		(void) strcat (tbuf, silly);
		(void) strcat (tbuf, *ptr++);
	}

	return (tbuf);
}


/*
This routine takes a hostname in a string and returns two
arrays of pointers into the string. The string is "chopped up"
with '\0'. The number of elements in the hostname is returned.
*/

int str2bits (str, sep, forward, reverse)
register char	*str;
register char	sep;
register char	**forward;
register char	**reverse;
{
	register int	i,
			j;

	PP_DBG (("Lib/table/str2bits(%s)", str));

	*forward++ = str;
	for (i = 1; *str != '\0'; str++)
		if (*str == sep) {
			*str = '\0';
			*forward++ = &str[1];
			i++;
		}

	*forward-- = NULLCP;

	if (reverse != NULL) {
		for (j = 0; j < i; j++)
			*reverse++ = *forward--;
		*reverse = NULLCP;
	}

	return (i);
}
