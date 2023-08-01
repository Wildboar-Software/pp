/* io_grey.c: These are IO routines for the greybook channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/grey/RCS/io_grey.c,v 6.0 1991/12/18 20:10:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/grey/RCS/io_grey.c,v 6.0 1991/12/18 20:10:19 jpo Rel $
 *
 * $Log: io_grey.c,v $
 * Revision 6.0  1991/12/18  20:10:19  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "prm.h"
#include "q.h"
#include "adr.h"
#include "ap.h"
#include "retcode.h"
#include "tb_q.h"
#include "tb_a.h"
#include "tb_prm.h"
#include <isode/cmd_srch.h>
#include <signal.h>


/*--- statics ---*/
static struct prm_vars          prm = { NULLCP, 0, PRM_ACCEPTALL };
static Q_struct                 Qstruct;


static int			submit_started = 0;
#define ST_OPENED		3
#define ST_DATAINIT		4
#define ST_JNTHDR               0
#define ST_MSGHDR               1
#define ST_BODY                 2
static int                      state = ST_OPENED;


static AP_ptr                   newdomain();
static char                     *compress();
static void			berror ();
static int                      add_via();
static int                      aprd_done();
static int                      aprd_init();
static int                      aprdchr();
static int                      fixup_sender();
static int                      hdr_done();
static int                      hdr_init();
static int                      hdr_read();
static int                      process_addrs();
static int			pp_error ();

static CHAN                     *mychan = NULLCHAN;
static char			**errstr = (char **) 0;
static char                     *theheader = NULLCP;
static char                     *hdrptr;
static char                     *apchptr;
static char                     *apcharacters = NULLCP;
static char			*fromhost = NULLCP;

static int data_bytes;
static struct timeval data_timer;

/*--- externals */
extern ADDR                     *adr_new();
extern char			*postmaster;
extern char	*ia5_bp, *hdr_822_bp;

#define GB_NOTOK	NOTOK
#define GB_OK		OK
#define GB_RECIPFAILED	-2
#define GB_SENDERFAILED	-3


/* -------------------  Begin  Routines  ---------------------------------- */


char	errbuffer[BUFSIZ];

/*--- tell PP that a call is arriving ---*/
int pp_open (channame, hostname, perrstr)
char    *channame;
char    *hostname;
char	**perrstr;
{
	RP_Buf          reply;
	Q_struct        *qp = &Qstruct;
	LIST_BPT        *new;
	extern char	*quedfldir;
	static int onceonly = 1;

	errstr = perrstr;
	if (onceonly) {
		chan_init (channame);
		(void) chdir (quedfldir);
		ap_use_percent ();
		(void) signal(SIGPIPE, SIG_IGN);
		onceonly = 0;
	}

	if (state != ST_OPENED)
		return pp_error (errstr, GB_RECIPFAILED,
				 "Illegal call to ppp_open - internal botch");
	PP_TRACE (("pp_open (%s, %s)", channame, hostname));

	if ((mychan = ch_mta2struct (channame, hostname)) == NULLCHAN) {
		(void) sprintf (errbuffer,
				"Local error: Channel '%s' is not known",
				channame);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}
	rename_log (mychan -> ch_name);

	PP_NOTICE (("Incoming Call from %s to %s", hostname, mychan->ch_name));
	if (fromhost)
		free (fromhost);
	fromhost = strdup (hostname);

	if (submit_started == 0 && rp_isbad (io_init (&reply))) {
		berror (errbuffer, "Can't initialise mail system", &reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}
	submit_started = 1;

	if (rp_isbad (io_wprm (&prm, &reply))) {
		berror (errbuffer, "Can't set parameters", &reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}


	/*--- initialize queue structure ---*/

	q_init (qp);
	qp -> msgtype = MT_UMPDU;
	qp -> inbound = list_rchan_new (hostname, NULL);
	qp -> inbound -> li_chan = mychan;

	qp -> encodedinfo.eit_types = NULLIST_BPT;
	new = list_bpt_new (hdr_822_bp);
	list_bpt_add (&qp -> encodedinfo.eit_types, new);
	new = list_bpt_new (ia5_bp);
	list_bpt_add (&qp -> encodedinfo.eit_types, new);

	if (rp_isbad (io_wrq (qp, &reply))) {
		berror (errbuffer, "Can't write Q struct", &reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}
	data_bytes = 0;
	timer_start (&data_timer);
	state = ST_DATAINIT;
	return GB_OK;
}

/* ARGSUSED */
int pp_write (fd, buff, size)  /*--- write some data to PP ---*/
int	fd; 
char    buff[];
int     size;
{
	static int              linefeed = 0;
	static struct qbuf      *qb[2] = {NULL, NULL}, *qp = NULL;
	char                    *cp = NULLCP,
				*ep = NULLCP,
				*bp = NULLCP,
				*q;
	int	retval;

	PP_TRACE (("pp_write (%d bytes)", size));

	data_bytes += size;

	switch (state) {
	    case ST_DATAINIT:
		linefeed = 0;
		if (qb[0])
			qb_free (qb[0]), qb[0] = NULL;
		if (qb[1])
			qb_free (qb[1]), qb[1] = NULL;
		state = ST_JNTHDR;
		/* fall */
	    case ST_JNTHDR:
	    case ST_MSGHDR:
		for (bp = cp = buff, ep = buff + size; cp < ep; cp++) {
			switch (*cp) {
			    case '\r':
			    case ' ':
			    case '\t':
				continue;
			    case '\n':
				linefeed ++;
				if (linefeed < 2)
					continue;
				for (q = cp - 1; q > bp && *q == '\r'; q--)
					continue;

				if (qb[state] == NULL)
					qb[state] = str2qb (bp, q - bp + 1, 1);
				else {
					qp = str2qb (bp, q - bp + 1, 0);
					insque (qp, qb[state]->qb_back);
				}
				bp = cp + 1;
				state ++;
				if (state < ST_BODY)
					continue;
				break;
			    default:
				linefeed = 0;
				continue;
			}
			break;
		}
                if (state != ST_BODY && bp + 1 < ep) {
                        if (qb[state] == NULL)
                                qb[state] = str2qb (bp, ep - bp, 1);
                        else {
                                qp = str2qb (bp, ep - bp, 0);
                                insque (qp, qb[state]->qb_back);
                        }
                }

		if (qb[0]) {
			cp = NULLCP;
			PP_DBG (("pp_write/qb[0]='%.900s'",
				   cp = qb2str(qb[0])));
			if (cp)
				free (cp);
		}
		if (qb[1]) {
			cp = NULLCP;
			PP_DBG (("pp_write/qb[1]='%.900s'",
				   cp = qb2str (qb[1])));
			if (cp)
				free (cp);
		}

		if (state == ST_BODY) { /* GOT IT - lets go!! */
			if ((retval = process_addrs (qb[0], qb[1], errstr))
			    != GB_OK)
				return retval;
			qb_free (qb[0]); qb[0] = NULL;
			qb_free (qb[1]); qb[1] = NULL;
			if (bp + 1 < ep)
				if (rp_isbad (io_tdata(bp, size - (bp-buff))))
					return pp_error (errstr,
							 GB_RECIPFAILED,
							 "Data Copy failed");
		}
		break;


	    case ST_BODY:
		if (size <= 0) {
			PP_LOG (LLOG_EXCEPTIONS, ("Funny size to pp_write: %d",
						  size));
			return size;
		}
		if (rp_isbad (io_tdata (buff, size)))
			return pp_error (errstr, GB_RECIPFAILED,
					 "Data Copy Faield");
		return size;
	    default:
		return pp_error (errstr, GB_RECIPFAILED,
				 "Illegal call to pp_write - internal error");
	}
	return size;
}

/* ARGSUSED */
int pp_close(fd)  /*--- tell PP that all the data has arrived ---*/
int	fd; 
{
	RP_Buf  reply;

	PP_TRACE (("pp_close()"));

	if (state == ST_MSGHDR)
		if (pp_write (0, "\n", 1) != 1)
			return pp_error (errstr, GB_RECIPFAILED,
					 "pp_write (internal call) failed");

	if (state != ST_BODY)
		return pp_error (errstr, GB_RECIPFAILED,
				 "Illegal call to pp_close - internal error");

	timer_end (&data_timer, data_bytes, "Data Received");
	if (rp_isbad (io_tdend (&reply))) {
		berror (errbuffer, "data termination failed", &reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}

	if (rp_isbad (io_tend (&reply))) {
		berror (errbuffer, "data termination (2) failed", &reply);
		if (rp_gbval (reply.rp_val) == RP_BNO)
			return pp_error (errstr, GB_NOTOK, errbuffer);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}

	PP_NOTICE (("Message successfully transfered"));
	state = ST_OPENED;
	return GB_OK;
}




/* -------------------  Static Routines  ---------------------------------- */

static AP_ptr newdomain (domn)
char    *domn;
{
	PP_TRACE (("newdomain (%s)", domn));
	return ap_new ((*domn == '[') ? AP_DOMAIN_LITERAL : AP_DOMAIN, domn);
}




static process_addrs (qbjnt, qbhdr, errstr)
struct qbuf     *qbjnt;
struct qbuf     *qbhdr;
char	**errstr;
{
	RP_Buf          reply;
	char            *name,
			*contents,
			buffer[BUFSIZ],
			sbuf[BUFSIZ],
			reasonmsg[BUFSIZ],
			*p = NULLCP;
	AP_ptr          ap_sender = NULLAP,
			jntroute = NULLAP,
			local = NULLAP,
			domain = NULLAP,
			route = NULLAP,
			jntaddr = NULLAP,
			ackaddr = NULLAP,
			aptr;
	ADDR            *ap = NULLADDR;
	int             got_sender = 0,
			ack_flag = AD_USR_BASIC,
			retval;
	struct qbuf     *qp = NULL;
	int	n;


	PP_TRACE (("process_addrs()"));

	hdr_init (qbhdr);


	reasonmsg[0] = 0;
	while (hdr_read (&name, &contents) != NOTOK) {

		PP_DBG (("process_addrs (%s %s)", name, contents));

		if (lexequ ("sender", name) == 0) {
			if ((aptr = ap_s2t (contents)) == BADAP ||
			    aptr == NULLAP) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Bad sender field %s",
					 contents));
				(void) sprintf (reasonmsg,
						"sender %s not parseable", contents);
			}
			else {
				(void) strcpy (sbuf, contents);
				ap_sender = aptr;
				got_sender = 1;
			}
		}

		if (lexequ ("from", name) == 0 && !got_sender) {
			if ((aptr = ap_s2t (contents)) == BADAP ||
			    aptr == NULLAP) {
				PP_LOG (LLOG_EXCEPTIONS,
				       ("Bad from field %s",
					contents));
				(void) sprintf (reasonmsg,
						"From %s not parseable", contents);
			}
			else {
				ap_sender = aptr;
				(void) strcpy (sbuf, contents);
			}
		}

		if (lexequ ("via", name) == 0) {
			if (add_via (contents, &jntroute, buffer) == NOTOK) {
				return pp_error (errstr, GB_NOTOK,
						 buffer);
			}
		}

		if (lexequ ("acknowledge-to", name) == 0)
			if ((ackaddr = ap_s2t(contents)) == BADAP ||
			    aptr == NULLAP) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Bad acknowledge-to field %s",
					  contents));
				aptr = NULLAP;
			}
	}

	hdr_done();

	if (!ap_sender) {
		if (reasonmsg[0])
			return pp_error (errstr, GB_NOTOK,
					 reasonmsg);
		return pp_error (errstr, GB_NOTOK,
				 "No sender field present in Header");
	}

	if (fixup_sender (ap_sender, jntroute, buffer, ackaddr))
		ack_flag = AD_USR_CONFIRM;

	ap = adr_new (buffer, AD_822_TYPE, 0);

	PP_NOTICE (("Sender '%s', original sender '%s'", ap->ad_value, sbuf));

	if (rp_isbad (io_wadr (ap, AD_ORIGINATOR, &reply))) {
		(void) sprintf (buffer, "Bad sender '%s'", ap -> ad_value);
		adr_tfree (ap);
		berror (errbuffer, buffer, &reply);
		if (reply.rp_val == RP_BHST ||
		    rp_gbval(reply.rp_val) == RP_BNO)
			return pp_error (errstr, GB_NOTOK, errbuffer);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}

	adr_tfree (ap);

	aprd_init (qbjnt);

	n = 0;
	while ((jntaddr = ap_pinit (aprdchr)) != BADAP) {
		switch (retval = ap_1adr()) {
		    case NOTOK:
			ap_free (jntaddr);
			aprd_done ();
			return pp_error (errstr, GB_NOTOK,
				 "unparseable JNT address - BAD JNT HEADER");

		    case DONE:
			ap_free (jntaddr);
			break;
		}

		if (retval == DONE)
			break;

		ap_t2p (jntaddr, (AP_ptr *)0, (AP_ptr *)0, &local,
			    &domain, &route);

		if (route == NULLAP) {
			if (domain && domain -> ap_obtype == AP_DOMAIN_LITERAL) {
				ap_free (domain);
				domain = NULLAP;
			}
		}
		else {
			if (route -> ap_obtype == AP_DOMAIN_LITERAL) {
				aptr = route;
				route = aptr -> ap_next;
				if (route -> ap_obtype != AP_DOMAIN_LITERAL ||
				    route -> ap_obtype != AP_DOMAIN)
					route = NULLAP;
				ap_free (aptr);
			}
		}
		ap_dmnormalize (domain, mychan -> ch_ad_order);
		p = ap_p2s_nc (NULLAP, NULLAP, local, domain, route);
		ap = adr_new (p, AD_822_TYPE, 1);
		ap -> ad_usrreq = ack_flag;

		PP_NOTICE (("Recipient Address '%s'", ap->ad_value));

		if (rp_isbad (io_wadr (ap, AD_RECIPIENT, &reply))) {
			berror (errbuffer, "Recipient failed", &reply);
			return pp_error (errstr, GB_RECIPFAILED, errbuffer);
		}
		adr_tfree (ap);
		n ++;
	}
	if (n == 0)
		return pp_error (errstr, GB_NOTOK,
				 "No recipients in JNT Header");

	if (rp_isbad (io_adend (&reply))) {
		berror (errbuffer, "Address termination failed", &reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}

	aprd_done();

	if (rp_isbad (io_tinit (&reply)) ||
	    rp_isbad (io_tpart (hdr_822_bp, FALSE, &reply))) {
		berror (errbuffer, "Text initialisation", &reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}

	for (qp = qbhdr -> qb_forw; qp != qbhdr; qp = qp -> qb_forw) {
		if (qp -> qb_len <= 0)
			continue;
		if (rp_isbad (io_tdata (qp->qb_data, qp->qb_len))) {
			(void) sprintf (errbuffer, "Data copy Failed");
			return pp_error (errstr, GB_RECIPFAILED, errbuffer);
		}
	}

	if (rp_isbad (io_tdend (&reply))) {
		berror (errbuffer, "Text termination problem", &reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}

	if (rp_isbad (io_tpart ("1.ia5", FALSE, &reply))) {
		berror (errbuffer, "Text body initialiastion probelms",
			&reply);
		return pp_error (errstr, GB_RECIPFAILED, errbuffer);
	}

	return GB_OK;
}

static int fixup_sender (sender, jntroute, dest, ack_to)
AP_ptr  sender;
AP_ptr  jntroute;
char    *dest;
AP_ptr ack_to;
{
	extern  char    *loc_dom_site; 
	AP_ptr          ap, 
			route = NULLAP, 
			domain = NULLAP, 
			mbox = NULLAP;
	int             donevia = 0;
	int		ack = 0;
	char            *p;


	PP_TRACE (("fixup_sender()")); 


	ap_t2p (sender, (AP_ptr *)0, (AP_ptr *)0, &mbox, &domain, &route);

#define pp_is_burning_up_cpu_time 1

	while (pp_is_burning_up_cpu_time) {

		if (jntroute != NULLAP) {
			ap = jntroute;
			jntroute = jntroute -> ap_next;
		}
		else if (donevia)
			break;
		else {
			donevia = TRUE;
			if (fromhost == NULLCP || *fromhost == '\0')
				ap = newdomain (loc_dom_site);
			else
				ap = newdomain (fromhost);
		}


		if (domain ==NULLAP)
			domain = ap;
		else {
			ap -> ap_next = route;
			route = ap;
		}
	}


	/*--- normalise the domains ---*/
	ap_dmnormalize (domain, mychan -> ch_ad_order);

	for (ap = route; ap != NULLAP; ap = ap -> ap_next)
		switch (ap -> ap_obtype) {
		    case AP_DOMAIN:
			ap_dmnormalize (ap, mychan -> ch_ad_order);
			break;

		    default:
			break;
		}

	/*--- eliminating redundant entries ---*/
	while (route != NULLAP) {
		AP_ptr last = domain;

		for (ap = route; ap -> ap_next != NULLAP;) {
			if (ap -> ap_next ->ap_obtype == AP_NIL)
				break;
			last = ap;
			ap = ap -> ap_next;
		}
		PP_TRACE(("comparing %s & %s",
			  last -> ap_obvalue, ap -> ap_obvalue));
		if (lexequ (last -> ap_obvalue, ap -> ap_obvalue) == 0) {
			PP_TRACE (("Removing duplicate route %s (dom %s)",
				   ap -> ap_obvalue, domain -> ap_obvalue));
			ap -> ap_obtype = AP_NIL;
			free (ap -> ap_obvalue);
			ap -> ap_obvalue = NULLCP;
			if (ap == route) {
				ap_free (route);
				route = NULLAP;
				break;
			}
		}
		else
			break;
	}
	if (ack_to) {
		AP_ptr mb, dmn;

		ap_t2p (ack_to, NULLAPP, NULLAPP, &mb, &dmn, NULLAPP);
		if (dmn) ap_dmnormalize (dmn, mychan -> ch_ad_order);
		if (mb && dmn &&
		    lexequ (mb -> ap_obvalue, mbox -> ap_obvalue) == 0 &&
		    lexequ (dmn -> ap_obvalue, domain -> ap_obvalue) == 0)
			ack = 1;
	}
	p = ap_p2s_nc (NULLAP, NULLAP, mbox, domain, route);
	(void) strcpy (dest, p);
	free (p);
	return ack;
}

#define retsloppy() (mychan -> ch_strict_mode == CH_STRICT_CHECK ? NOTOK : OK)

static add_via (str, route, ebuf)
char    *str;
AP_ptr  *route;
char	*ebuf;
{
	char    *p, *q;
	AP_ptr  ap;


	PP_TRACE (("add_via (%s)", str));

	if ((p = rindex (str, ';')) != NULLCP)
		*p = '\0';
	else {
		(void) sprintf (ebuf, "Illegal Via: field, No ';' \"%s\"",
				str);
		PP_LOG (LLOG_EXCEPTIONS, ("%s", ebuf));
		return retsloppy();
	}

	compress (str, str);

	while ((p = index (str, '(')) != NULLCP) {
		if ((q = index (p, ')')) != NULLCP) {
			q++;
			while (*q)
				*p++ = *q++;
			*p = '\0';
		}
		else
			break;
	}

	compress (str, str);
	if (str == NULLCP || *str == NULL) {
		(void) sprintf (ebuf, "empty host component in via field");
		PP_LOG (LLOG_EXCEPTIONS, ("%s", ebuf));
		return retsloppy();
	}
	ap = newdomain (str);
	ap -> ap_next = *route;
	*route = ap;
	return OK;
}





static hdr_init (qb)
struct qbuf *qb;
{
	hdrptr = theheader = qb2str (qb);
	PP_TRACE (("hdr_init (%.999s)", hdrptr)); 
}




static hdr_read (name, contents)
char    **name, **contents;
{
	PP_DBG (("hdr_read (%.80s)", hdrptr)); 

	if (*hdrptr == '\0')
		return NOTOK;

	*name = hdrptr;
	for (*name = hdrptr; *hdrptr; hdrptr++) {
		if (*hdrptr == ':') {
			*hdrptr ++ = '\0';
			break;
		}
		if (*hdrptr == '\n') {
			*hdrptr = '\0';
			*contents = hdrptr;
			hdrptr ++;
			return OK;
		}
	}
	compress (*name, *name);

	if (*hdrptr == '\0')
		return OK;

	while (*hdrptr == ' ' || *hdrptr == '\t')
		hdrptr ++;

	for (*contents = hdrptr; *hdrptr; hdrptr++) {
		if (*hdrptr == '\n') {
			if (isspace (hdrptr[1])) {
				*hdrptr = ' ';
				continue;
			}
			*hdrptr ++ = '\0';
			break;
		}
	}
	return OK;
}




static hdr_done()
{
	PP_TRACE (("hdr_done()")); 
	if (theheader) {
		free (theheader);
		theheader = NULLCP; 
	} 
}



static aprd_init (qb)
struct qbuf *qb;
{
	apchptr= apcharacters = qb2str (qb);
	PP_TRACE (("aprd_init (%.999s)", apchptr)); 
}



static aprdchr()
{
	return *apchptr == '\0' ? EOF : *apchptr++;
}



static aprd_done()
{
	PP_TRACE (("aprd_done()"));
	if (apcharacters) {
		free (apcharacters);
		apcharacters = NULLCP;
	}
}



static char *compress (fromptr, toptr)
register char   *fromptr;
register char   *toptr;
{
	register char   chr;
	char            *in_toptr = toptr;


	PP_DBG (("compress (%s)", fromptr));


	/*--- init to skip leading spaces tabs and newlines --- */

	chr = ' ';


	while ((*toptr = *fromptr++) != '\0') {

		/*--- only save the spaces found in between words ---*/
		if (isspace (*toptr)) {
			if (chr != ' ')
				*toptr++ = chr = ' ';
		}
		else
			chr = *toptr++;
	}


	/*--- remove trailing spaces, tabs and newlines  if any ---*/
	if ((chr == ' ')
	    && (toptr != in_toptr))
		*--toptr = '\0';

	return (toptr);
}

static void berror (buffer, str, rp)
char	*buffer, *str;
RP_Buf *rp;
{
	(void) sprintf (buffer, "%s ([%s] %s)",
			str, rp_valstr (rp->rp_val),
			rp -> rp_line[0] ? rp -> rp_line :
			"No further information");
}

static int pp_error (err, code, str)
char	**err;
int	code;
char	*str;
{
	char	*kind;

	switch (code) {
	    case GB_OK:
		kind = "ok";
		break;
	    case GB_RECIPFAILED:
		kind = "failed - recipient temp";
		break;
	    case GB_NOTOK:
		kind = "failed - permanent";
		break;

	    case GB_SENDERFAILED:
		kind = "failed - sender temporary";
		break;
	    default:
		kind = "UNKNOWN";
		break;
	}
	if (err) *err = strdup (str);
	PP_LOG (LLOG_EXCEPTIONS, ("%s %s", str, kind));
	submit_started = 0;
	io_end (NOTOK);
	state = ST_OPENED;
	return code;
}
