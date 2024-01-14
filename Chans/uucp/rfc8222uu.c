/* rfc8222uu.c: routine to convert from rfc822 style address to a 
uucp style address via uucp chan table lookup - code adapted from mmdf 
original */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/uucp/RCS/rfc8222uu.c,v 6.0 1991/12/18 20:13:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/uucp/RCS/rfc8222uu.c,v 6.0 1991/12/18 20:13:06 jpo Rel $
 *
 * $Log: rfc8222uu.c,v $
 * Revision 6.0  1991/12/18  20:13:06  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"chan.h"
#include	"ap.h"
#include	"adr.h"
extern char	*strdup();
extern CHAN	*mychan;
extern char *uuxstr, *rindex();

extern int	ap_outtype;
static char     nextnode[LINESIZE];
static char     who[LINESIZE];
static char	*MakeUucpFrom();
static void	ScanUucpFrom();

extern char	*loc_dom_site, *loc_dom_mta;

int	rfc8222uu(host, orig, into)
char	*host,
	*orig,
	**into;
{
    char        *bangptr;
    char	linebuf[LINESIZE];
    char        *atp, *percentp, *lp;
    
    AP_ptr  loc_ptr,        /* -- in case fake personal name needed -- */
		dom_ptr,
		ap;
	char	adr[LINESIZE];

	ap_outtype = AP_PARSE_733;

	ap_norm_all_domains();
	ap_use_percent ();

	if ((ap = ap_s2t(orig)) == NULLAP)
	{
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Failure to parse address '%s'",orig));
		return NOTOK;
	}

	(void) ap_t2p(ap, (AP_ptr *) NULL, (AP_ptr *) NULL, /*&group_ptr, &name_ptr,*/
			     &loc_ptr, &dom_ptr, (AP_ptr *) NULL); /*&route_ptr);*/
	lp = ap_p2s_nc(NULLAP, NULLAP, loc_ptr, dom_ptr, NULLAP); /*route_ptr);*/
	
	atp = index(lp, '@');
	if (atp != NULLCP)
		*atp++ = '\0';
	if (!lexequ(atp, host))
		atp = NULLCP;	/* don't make path-to-foo!foo.uucp!user */
	percentp = rindex(lp, '%');
	if (percentp != NULLCP) {
		*percentp = '\0';
		if (atp)
			(void) sprintf(adr, "%s!%s!%s", atp, ++percentp, lp);
		else
			(void) sprintf(adr, "%s!%s", ++percentp, lp);
	} else if (atp) {
		(void) sprintf(adr, "%s!%s", atp, lp);
	} else 
		(void) strcpy(adr,lp);
	PP_TRACE(("address = '%s'",adr));
	if (!isstr(host))
		(void) strcpy(who, adr);
	else {
		if (tb_k2val(mychan->ch_table, host, nextnode, TRUE) == NOTOK) {
			return NOTOK;
		}
		(void) sprintf(who, nextnode, adr);
	}

	/* Extract first host name for destination */
	if ((bangptr=index(who, '!')) != NULLCP)
	{
		/* at least one realy machine */
		*bangptr++ = '\0';
		(void) strcpy(nextnode, who);
		(void) strcpy(who, bangptr);
	} else (void) strcpy(nextnode, "");

	(void) sprintf(linebuf, "%s %s!rmail \\(%s%s\\)",
		uuxstr, nextnode, *who=='~' ? "\\\\" : "", who);
	PP_TRACE(("Queuing UUCP mail for %s via %s...\n",
		  who, nextnode));
	*into = strdup(linebuf);
	return OK;
}

/*  */
/* routine to create realfrom string */

char *
findfrom (sender)
char *sender;
{
	int     aptypesav;
	char    *adr;
	AP_ptr  ap;
	AP_ptr  local,
		domain,
		route;

/* SEK have axed looking at top of file.  */
/* This may not be wise - but very much neater */
/* Delver has no business being given UUCP style messsages */

	if ((ap = ap_s2t (sender)) == NULLAP)
	{
	    PP_LOG(LLOG_EXCEPTIONS,
		   ("Failure to parse address '%s'", sender));
	    return (strdup (sender));
	}

	ap = ap_normalize (ap, CH_USA_PREF);
	if(ap == (AP_ptr)MAYBE)
	    return( (char *)MAYBE);
	ap_t2p (ap, (AP_ptr *) NULL, (AP_ptr *) NULL, &local, &domain, &route);
	aptypesav = ap_outtype;
	ap_outtype = AP_PARSE_733;
	adr = ap_p2s_nc (NULLAP, NULLAP, local, domain, route);
	if(adr == (char *)MAYBE){
	    ap_outtype = aptypesav;
	    return(adr);
	}
	if (route == NULLAP)
	{
		if (domain->ap_islocal == TRUE)
		{
			free (adr);
			adr = strdup (local -> ap_obvalue);
		}
	}
	PP_TRACE(("sender = '%s'", adr));
	ap_outtype = aptypesav;

	lowerfy(adr);
	return(MakeUucpFrom(adr));
}

/*
 * This added by pc (UKC) to generate correct 'From' lines with
 * `!' separated routes for uucp sites
 * the rules
 * 	a@b   -> b!a
 *	a%b@c -> c!b!a
 *	etc
 *	a%b%c%d%e@x -> x!e!d!c!b!a
 *	This is done by a call to a recursive routine which I hope is OK
 */
static
char *
MakeUucpFrom(adr)
char *adr;
{	char *new;
	register char *site;
	
	/*NOSTRICT*/
	if ((new = malloc((unsigned)strlen(adr)+1)) == (char *)0)
		return ((char *)NOTOK);
	/*
	 * Can we assume that this is a legal 733 address ?
	 * look for the first site
	 */
	site = rindex(adr, '@');
	if (site)
	{	*site++ = '\0';
		/*
		 * some input channels (notably ni_niftp) will add the
		 * name of the local machine into this address
		 * so we look for it and delete it if found
		 */
		/*
		 * if not the same then put back a % to let ScanUucpFrom work
		 */
		if (lexequ (loc_dom_site, site) != 0
		    && lexequ (loc_dom_mta, site) != 0)
			site[-1] = '%';
	}
	ScanUucpFrom(new, adr);
	free(adr);
#ifdef DEBUG
	PP_TRACE(("sender (From line) = '%s'", new));
#endif
  	return(new);
}

static void
ScanUucpFrom(new, adr)
register char *new;
register char *adr;
{	register char *site;

	/*
	 * This presumes that the address we are scanning is somewhat
	 * legal - but the @ has been replaced by a %
	 */
	site = rindex(adr, '%');
	if (site == (char *)0)
	{	(void) strcpy(new, adr);
		return;
	}
	*site++ = '\0';
	(void) strcpy(new, site);
	new += strlen(site);
	*new++ = '!';
	*new = '\0';
	ScanUucpFrom(new, adr);
}

int getrealfrom(orig, realfrom)
ADDR	*orig;
char	**realfrom;
{
	if ((*realfrom = findfrom(orig->ad_r822adr)) != NULLCP)
		return OK;
	return NOTOK;
}

/**/

/*
 *      LOWERFY()  -  convert string to lower case
 */
void lowerfy (char *strp)
{
	while (*strp = uptolow (*strp))
		strp++;
}
