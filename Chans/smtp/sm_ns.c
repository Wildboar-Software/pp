/* sm_ns.c */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/smtp/RCS/sm_ns.c,v 6.0 1991/12/18 20:12:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/smtp/RCS/sm_ns.c,v 6.0 1991/12/18 20:12:19 jpo Rel $
 *
 * $Log: sm_ns.c,v $
 * Revision 6.0  1991/12/18  20:12:19  jpo
 * Release 6.0
 *
 */



/* Steve Kille, based on code from Phil Cockcroft and Craig Partridge
	September 1989 */



#include "util.h"
#include "retcode.h"
#include "chan.h"
#include <signal.h>
#ifdef NAMESERVER
#include <isode/internet.h>

#include <arpa/nameser.h>
#ifdef XOS_2
#include <arpa/resolv.h>
#else
#include <resolv.h>
#endif


/* T_UNSPEC was defined only in more recent versions of BIND */

#ifdef T_UNSPEC
#define BSD4_3
#define	getshort _getshort
#endif T_UNSPEC


#ifndef MAXADDR
#define MAXADDR		30
#endif

#ifndef MAXADDR_PER
#define MAXADDR_PER	4
#endif

#ifndef MAXDATA
#define MAXDATA (4 * PACKETSZ)		/* tcp tried after udp */
#endif

#ifndef MAXMX
#define MAXMX	20			/* shouldn't be < MAXADDR */
#endif


union ansbuf {			/* potentially huge */
    HEADER ab1;
    char ab2[MAXDATA];
}; 

union querybuf {		/* just for outbound stuff */
    HEADER qb1;			/* didn't want to clobber stack */
    char qb2[2 * MAXDNAME];
}; 

extern char *loc_dom_mta;

static  char dn_name[MAXDNAME];

static struct hostent hval;
static struct in_addr addrbuf[MAXADDR+1];
static char *addrptrs [MAXADDR+1];
static char addrarray[MAXADDR + 1][sizeof(struct in_addr) + 1];
extern int h_errno;

ns_gethost (key, hptr, errstr)
char *key;
struct hostent **hptr;
char	*errstr;
{
    register char *cp;
    register int i, j, n;
    HEADER *hp;
    struct hostent *he;
    union querybuf qbuf;
    union ansbuf abuf;
    u_short type, dsize;
    int pref, localpref;
    int count, mxcount;
    int sawmx;			/* are we actually processing mx's? */
    char *eom;
    char buf[MAXDNAME];		/* for expanding in dn_expand */
    char newkey[MAXDNAME]; 	/* in case we get a CNAME RR back... */
    struct {			/* intermediate table */
	char *mxname;
	u_short mxpref;
    } mx_list[MAXMX];

    int acount;

    extern char *ns_skiphdr();

    *errstr = '\0';

    PP_TRACE (("DNS resolve host %s", key));

    for (i = 0; i <= MAXADDR; i++)
	addrptrs[i] = (char *) addrarray[i];
    *hptr = &hval;   
    hval.h_addr_list = addrptrs;
    hval.h_name = key;
    hval.h_aliases = 0;


restart:

    mxcount = 0;
    acount = 0;
    localpref = -1;
    sawmx = 0;

    n = res_mkquery(QUERY, key, C_IN, T_MX, (char *)0, 0, (char *)0,
	(char *)&qbuf, sizeof(qbuf));

    /* what else can we do? */
    if (n < 0)
    {
	(void) sprintf (errstr, "Unable to send query");
	PP_NOTICE (("No answers from res_mkquery"));
	return(RP_NO);
    }

    PP_TRACE (("ns_gethost: sending ns query (%d bytes)",n));

    n = res_send((char *)&qbuf,n,(char *)&abuf, sizeof(abuf));

    if (n < 0)
    {
	(void) sprintf (errstr, "Unable to contact the DNS");
	PP_SLOG (LLOG_EXCEPTIONS, "res_send", ("DNS resolver error: "));
	return(RP_TIME);
    }

    hp = (HEADER *)&abuf;

    if (hp->rcode != NOERROR) 
	return(ns_error(hp, errstr, key));


    if (ntohs(hp->ancount) == 0)
    {
	mxcount = 1;
	mx_list[0].mxname = strdup(key);
	mx_list[0].mxpref = 0;
	goto doaddr;
    }

    /* read MX list */
    sawmx = 1;
    count = ntohs(hp->ancount);


    /* skip header */
    eom = ((char *)&abuf) + n;
    if ((cp = ns_skiphdr((char *)&abuf, hp, eom))==0)
    {
	(void) sprintf (errstr, "No useful answers to query of %s", key);
	PP_NOTICE (("ns_gethost: no useful answers to query"));
	return(RP_TIME);
    }

    PP_TRACE (("ns_gethost: %d answers to query",count));
    for (i = 0; i < MAXMX; i++)
	    mx_list[i].mxname = NULL;

    while ((cp < eom) && (count--))
    {
	n = dn_expand((char *)&abuf,eom, cp, buf, sizeof(buf));
	if (n < 0)
	    goto quit;

	cp += n;
	type = getshort(cp);
	/* get to datasize */
	cp += (2 * sizeof(u_short)) + sizeof(u_long);
	dsize = getshort(cp);
	cp += sizeof(u_short);

	/*
	 * is it an MX ? 
	 * it could be a CNAME
	 */

	if (type == T_CNAME)
	{
	    if (key == newkey) {
		/* CNAME -> CNAME: illegal */
		PP_LOG (LLOG_EXCEPTIONS, ("recursive CNAME on MX query of %s",
					  key));
		goto quit;
	    }
	    PP_TRACE (("ns_gethost: CNAME answer to MX query"));
	    n = dn_expand((char *)&abuf,eom, cp, newkey, sizeof(newkey));
	    cp += dsize;
	    if (n < 0)
		continue;	/* pray? */
	    PP_TRACE (("ns_gethost: `%s` -> `%s` (new query)",key,newkey));
	    key = newkey;
	    goto restart;
	}

	if (type != T_MX)
	{
	    PP_NOTICE (("ns_gethost: RR of type %d in response",type));
	    cp += dsize;
	    continue;       /* keep trying */
	}

	pref = getshort(cp);
	cp += sizeof(u_short);

	n = dn_expand((char *)&abuf,eom, cp, buf, sizeof(buf));
	if (n < 0)
	    goto quit;

	cp += n;

	/* is it local? */
	if ((lexequ(loc_dom_mta, buf) == 0) &&
	    ((localpref < 0) || (pref < localpref)))
	{
	    localpref = pref;
	    for(i=(mxcount-1); i >= 0; i--)
	    {
		if (mx_list[i].mxpref < localpref)
		    break;

		(void) free(mx_list[i].mxname);
		mxcount--;
	    }
	    continue;
	}

	/* now, see if we keep it */
	if ((localpref >= 0) && (pref >= localpref))
	    continue;

	/*  find where it belongs */
	for(i=0; i < mxcount; i++)
	    if (mx_list[i].mxpref > pref)
		break;

	/* not of interest */
	if (i == MAXMX)
	    continue;

	/* shift stuff to make space */
	for(j= min(mxcount,MAXMX-1); j > i; j--)
	{
	    if (j == (MAXMX-1) && mx_list[j].mxname)
		(void) free(mx_list[j].mxname);

	    mx_list[j].mxname = mx_list[j-1].mxname;
	    mx_list[j].mxpref = mx_list[j-1].mxpref;
	}

	mx_list[i].mxname = strdup(buf);
	mx_list[i].mxpref = pref;

	if (mxcount < MAXMX)
	    mxcount ++;
    }

    /*
     * should read additional RR section for addresses and cache them
     * but let's hold on that.
     */

doaddr:
    /* now build the address list */
    

    PP_TRACE (("ns_gethost: using %d mx hosts",mxcount));

    for(i=0,j=0; (i < mxcount) && (j < MAXADDR); i++)
    {
	/*
	 * note that gethostbyname() is slow -- we should cache so
	 * we don't ask for an address repeatedly
	 */

	he = gethostbyname(mx_list[i].mxname);

	if (he == 0)
	{
	    PP_NOTICE (("ns_gethost: no addresses for %s",
			mx_list[i].mxname));
	    /* nope -- were trying special case and no address */
	    if ((!sawmx) && (h_errno != TRY_AGAIN)) {
		sprintf (errstr, "No nameserver addresses for %s", 
				mx_list[i].mxname);
		return(RP_NO);
	    }

	    continue;
	}

	if (j == 0)
 	{
		hval.h_length = he -> h_length;
		hval.h_addrtype = he -> h_addrtype;
	}

	for(n=0; (j < MAXADDR) && (n < MAXADDR_PER); n++, j++)
	{
	    if (he->h_addr_list[n] == 0)
		break;

	    bcopy(he->h_addr_list[n],hval.h_addr_list[j],sizeof(struct in_addr));

	}
	PP_TRACE (("ns_gethost: %d addresses saved for %s",
		n, mx_list[i].mxname));
    }
    acount = j;
    hval.h_addr_list[j] = 0; 

quit:
    for(i=0; i < mxcount; i++)
	(void) free(mx_list[i].mxname);

    if (acount == 0) {
	(void) sprintf (errstr, "Nameserver lookup of %s: No address found", 				key);
	PP_NOTICE (("ns_gethost: No addresess derived from MX query"));
    }
    else
	PP_NOTICE (("DNS resolves %s to %d address%s", key, acount,
		    acount > 1 ? "es" : ""));
    /* if localpref is set, then we got an answer */
    return (acount == 0 ? (localpref < 0 ? RP_TIME : RP_NO) : RP_OK);
}

static char *prcode (n)
int n;
{
	static char tbuf[40];

	switch (n) {
	    case NOERROR:
		return "No Error (NOERROR)";

	    case FORMERR:
		return "Format Error (FORMERR)";

	    case SERVFAIL:
		return "Server failure (SERVFAIL)";

	    case NXDOMAIN:
		return "Non existant host/domain (NXDOMAIN)";

	    case NOTIMP:
		return "Not implemented (NOTIMP)";

	    case REFUSED:
		return "Query Refused (REFUSED)";

	    default:
		(void) sprintf (tbuf, "Unknown code %d", n);
		return tbuf;
	}
}
		
/*
 * figure out proper error code to return given an error
 */

static
ns_error(hp, errstr, key)
register HEADER *hp;
char	*errstr;
char *key;
{

    PP_NOTICE (("DNS error: %s",prcode(hp->rcode)));
    switch (hp->rcode)
    {
	case NXDOMAIN:
	    (void) sprintf (errstr, "Nameserver error for %s: %s", key,
			    prcode (hp -> rcode));
	    return(RP_NO); /* even if not authoritative */

	case SERVFAIL:
	    return(RP_TIME);

	default:
	    break;
    }
    (void) sprintf (errstr, "Nameserver error for %s: %s", 
		    key, prcode(hp->rcode));
    return(RP_NO);
}

/*
 * skip header of query and return pointer to first answer RR.
 */

static
char *ns_skiphdr(answer, hp, eom)
char *answer;
HEADER *hp;
register char *eom;
{
    register int qdcount;
    register char *cp;
    register int n;
    char tmp[MAXDNAME];

    qdcount = ntohs(hp->qdcount);

    cp = answer + sizeof(HEADER);

    while ((qdcount-- > 0) && (cp < eom))
    {
	n = dn_expand(answer,eom,cp,tmp,sizeof(tmp));
	if (n < 0)
	    return(0);
	cp += (n + QFIXEDSZ);
    }

    return((cp < eom)? cp : 0);
}

/*
 * routine to set the resolver timeouts
 * takes maximum number of seconds you are willing to wait
 */

ns_settimeo(ns_time)
int     ns_time;
{
    static int called = 0;
    static struct state oldres;

    if ((_res.options & RES_INIT) == 0)
	    res_init ();

    /* always start afresh */
    if (called)
    {
	bcopy((char *)&oldres,(char *)&_res,sizeof(oldres));
    }
    else
    {
	called = 1;
	bcopy((char *)&_res,(char *)&oldres,sizeof(oldres));
    }

    /*
     * bind uses an exponential backoff
     */

    _res.retrans = ns_time >> _res.retry;

    PP_TRACE (("ns_timeo: servers(%d), retrans(%d), retry(%d)",
	_res.nscount, _res.retrans, _res.retry));
}
#endif
