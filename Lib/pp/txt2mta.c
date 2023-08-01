/* txt2p1.c: handle text to varios mta structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2mta.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2mta.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: txt2mta.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include	"mta.h"
#include        "tb_com.h"
#include        "tb_p1.h"
#include        "tb_a.h"

#define	txt2int(n)	atoi(n)

extern  char *genstore ();
char            *utctime ();
extern	struct qbuf *hex2qb ();
extern  LIST_BPT        *bodies_all, *headers_all;

extern CMD_TABLE
		tbl_crit [],
		p1tbl_mpduid [/* mpdu-identifier */],
		p1tbl_trace [/* trace-information */],
		p1tbl_globaldomid [/* global-domain-identifier */],
		p1tbl_domsinfo [/* domain-supplied-info */],
		p1tbl_action [/* domain-supplied-info-action */],
		p1tbl_encinfoset [/* encode-info-types-set */],
		atbl_addr [/* address-lines */],
		tbl_bool [],
		atbl_mtarreq [/* mta-report-request */],
		atbl_usrreq [/* user-report-request */];
extern int n_atbl_addr;

/* ---------------------  Text File->Memory  ---------------------------- */




int txt2trace (trp, argv, argc)   /* Txt->TraceInformation */
Trace   **trp;
char    **argv;
int     argc;
{
	int             key;
	int     count = 0, n;
	register Trace  *new,
			*tp;

	PP_DBG (("Lib/pp/txt2trace(%s)", argv[0]));


	if (--argc < 1)
		return (NOTOK);


	/* -- create a new Trace struct -- */
	new = (Trace*) smalloc (sizeof (*new));
	bzero ((char*)new, sizeof (*new));


	/* -- put the new Trace struct into the appropriate place -- */
	if ((tp = *trp) == (Trace *)NULL)
		*trp = tp = new;
	else {
		/* -- point to current and put new in trace_next -- */
		for (; tp->trace_next; tp=tp->trace_next);
		tp->trace_next = new;
		tp = tp -> trace_next;
	}


	while (argc > 1) {
		if (*argv[0]  != '=') {
			if (cmd_srch(argv[0], p1tbl_trace) == EOB)
				count ++;
			break;
		}
		key = cmd_srch (argv[1], p1tbl_trace);

		switch (key) {
		    case TRACE_MTA:
			tp->trace_mta = strdup (argv[2]);
			n = 3;
			break;

		    case EOB:
		    case EOU:
			return count > 0 ? count : 0;

		    default:
			n = txt2globaldomid (&tp->trace_DomId,
					&argv[0], argc);
			if (n != NOTOK) {
				break;
			}
			n = txt2domsinfo (&tp->trace_DomSinfo,
					&argv[0], argc);
			if (n == NOTOK)
				return count > 0 ? count : NOTOK;
			break;
		}
		argc -= n;
		argv += n;
		count += n;
	}
	return count > 0 ? count : NOTOK;
}


int txt2globaldomid (gp, argv, argc)   /* Txt->GlobalDomainIdentifier */
GlobalDomId     *gp;
char            **argv;
int             argc;
{
	int     count = 0;
	int     n;

	PP_DBG (("Lib/pp/txt2globaldomid(%s %d)",
		argv[1], argc));

	while (argc > 0) {
		if (*argv[0] != '=')
			break;

		switch (cmd_srch (argv[1], p1tbl_globaldomid)) {
		    case GLOBAL_COUNTRY:
			gp->global_Country = strdup (argv[2]);
			n = 3;
			break;

		    case GLOBAL_ADMIN:
			gp->global_Admin = strdup (argv[2]);
			n = 3;
			break;
		    case GLOBAL_PRIVATE:
			gp->global_Private = strdup (argv[2]);
			n = 3;
			break;
		    default:
			return count > 0 ? count : NOTOK;
		}
		count += n;
		argc -= n;
		argv += n;
	}
	return (count > 0 ? count : NOTOK);
}


int txt2domsinfo (di, argv, argc)   /* Txt->DomainSuppliedInfo */
DomSupInfo      *di;
char            **argv;
int             argc;
{
	int     n, count = 0;
	char    *p;
	PP_DBG (("Lib/pp/txt2domsinfo(%s)", argv[0]));

	while (argc > 0) {
		if (*argv[0] == '=')
			p = argv[1];
		else    p = argv[0];
		switch (cmd_srch (p, p1tbl_domsinfo)) {
		    case DSI_TIME:
			if (txt2time (argv[2], &di->dsi_time) == NOTOK)
				di -> dsi_time = utcnow ();
			n = 3;
			break;
		    case DSI_DEFERRED:
			if (txt2time (argv[2], &di->dsi_deferred) == NOTOK)
				di -> dsi_deferred = utcnow ();
			n = 3;
			break;
		    case DSI_ACTION:
			di->dsi_action = txt2int (argv[2]);
			n = 3;
			break;
		    case DSI_CONVERTED:
			n = txt2encodedinfo(&di->dsi_converted, &argv[1],
					    argc - 1);
			if (n == NOTOK)
				return n;
			n ++;
			break;
		    case DSI_ATTEMPTED:
			n = txt2globaldomid (&di->dsi_attempted_md, &argv[1],
					     argc - 1);
			if (n == NOTOK)
				return n;
			n ++;
			break;
		    default:
			return count > 0 ? count : NOTOK;
		}
		argc -= n;
		argv += n;
		count += n;
	}
	return (count > 0 ? count : NOTOK);
}


int txt2encodedinfo (ep, argv, argc)  /* Txt->EncodedInformationTypes */
EncodedIT               *ep;
char                    **argv;
int                     argc;
{
	int     count = 0, n;


	PP_DBG (("Lib/pp/txt2encodedinfo('%s', %d)", argv[0], argc));

	while (argc > 0) {
		if (*argv[0] != '=')
			return NOTOK;

		switch (cmd_srch (argv[1], p1tbl_encinfoset)) {
		    case EI_G3NONBASIC:
			ep->eit_g3parms = txt2int (argv[2]);
			n = 3;
			break;
		    case EI_TELETEXNONBASIC:
			ep->eit_tTXparms = txt2int (argv[2]);
			n = 3;
			break;
		    case EI_PRESENTATION:
			ep->eit_presentation = txt2int (argv[2]);
			n = 3;
			break;
		    case EI_BIT_STRING:
			if (txt2listbpt (&ep->eit_types, argv[2]) == NOTOK)
				return NOTOK;
			n = 3;
			break;
		    default:
			return count > 0 ? count : NOTOK;
		}
		argc -= n;
		argv += n;
		count += n;
	}
	return (count > 0 ? count : NOTOK);
}


int txt2listbpt (bpt, str)  /* Txt -> A List of BodyPartTypes */
LIST_BPT                **bpt;
char                    *str;
{
	LIST_BPT        *np;
	int             i,
			j;
	char            *buf[30];

	PP_DBG (("Lib/pp/txt2listbpt(%s)", str));

	/*
	read string which is delimitted by   ','
	example:  tlx,ia5, etc...
	*/

	j = sstr2arg (str, 30, buf, ",");

	for (i = 0; i < j; i++) {
#ifdef notdef
		if (list_bpt_find(bodies_all, buf[i]) == NULLIST_BPT) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unknown bodypart type '%s'", buf[i]));
			return NOTOK;
		}
#endif
		np = list_bpt_new (buf[i]);
		list_bpt_add (bpt, np);
	}

	return (OK);

}

extern char	*hdr_prefix;

int txthdr2listbpt (bpt, str)  /* Txt -> A List of BodyPartTypes */
LIST_BPT                **bpt;
char                    *str;
{
	LIST_BPT        *np;
	int             i,
			j;
	char            *buf[30];
	char		buffer[BUFSIZ];

	PP_DBG (("Lib/pp/txthdr2listbpt(%s)", str));

	/*
	read string which is delimitted by   ','
	example:  822,ipn, etc..
	*/

	j = sstr2arg (str, 30, buf, ",");

	for (i = 0; i < j; i++) {
		(void) strcpy (buffer, hdr_prefix);
		(void) strcat (buffer, buf[i]);
#ifdef notdef
		if (list_bpt_find(headers_all, buffer) == NULLIST_BPT) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unknown header type '%s' ('%s')", 
				buf[i], buffer));
			return NOTOK;
		}
#endif
		np = list_bpt_new (buffer);
		list_bpt_add (bpt, np);
	}
	return (OK);

}

int txt2mpduid (mep, argv, argc)   /* Txt->MPDUIdentifier */
register MPDUid *mep;
char            **argv;
int             argc;
{
	int     n, count = 0;
	PP_DBG (("Lib/pp/txt2mpduid(%s)", argv[0]));

	while (argc > 0) {
		if (*argv[0] != '=')
			break;

		switch (cmd_srch (argv[1], p1tbl_mpduid)) {
		    case MPDUID_STRING:
			mep->mpduid_string = strdup (argv[2]);
			n = 3;
			break;
		    default:
			n = txt2globaldomid (&mep->mpduid_DomId,
					     &argv[0], argc);
			if (n == NOTOK)
				return count > 0 ? count : NOTOK;
			break;
		}
		argc -= n;
		argv += n;
		count += n;
	}
	return count > 0 ? count : NOTOK;
}


/* Txt->Responsibility, Mta-report-request & User-report-request */

int txt2repreq (field, argv, argc)
int     *field;
char    **argv;
int     argc;
{
	int                     val;

	PP_DBG (("Lib/pp/txt2repreq(%s %d)", argv[0], argc));

	if (--argc < 1)  return (NOTOK);
	if (*argv[0] != '=') return NOTOK;

	switch (cmdbsrch (argv[1], atbl_addr, n_atbl_addr)) {
	case AD_RESPONSIBILITY:
		if (P1_txt2resp (&val, argv[2]) == NOTOK)
		    return (NOTOK);
		break;
	case AD_MTA_REP_REQ:
		if (P1_txt2mtarreq (&val, argv[2]) == NOTOK)
		    return (NOTOK);
		break;
	case AD_USR_REP_REQ:
		if (P1_txt2usrreq (&val, argv[2]) == NOTOK)
		    return (NOTOK);
		break;
	default:
		return (NOTOK);
	}

	PP_DBG (("Lib/pp/txt2repreq(%s %d)", argv[1], val));

	*field = val;
	return (3);
}


int P1_txt2resp (val, keywd)   /* Txt->Responsibility */
int     *val;
char    *keywd;
{
	PP_DBG (("Lib/pp/P1_txt2resp(%s)", keywd));

	switch (*val = cmd_srch (keywd, tbl_bool)) {
	case YES:
	case NO:
		return (OK);
	default:
		return (NOTOK);
	}
}


int P1_txt2mtarreq (val, keywd)   /* Txt->Mta-report-request */
int     *val;
char    *keywd;
{
	PP_DBG (("Lib/pp/P1_txt2mtarreq(%s)", keywd));

	switch (*val = cmd_srch (keywd, atbl_mtarreq)) {
	case AD_MTA_NONE:
		*val = AD_MTA_BASIC;
	case AD_MTA_BASIC:
	case AD_MTA_CONFIRM:
	case AD_MTA_AUDIT_CONFIRM:
		return (OK);
	default:
		return (NOTOK);
	}
}


int P1_txt2usrreq (val, keywd)   /* Txt->Usr-report-request */
int     *val;
char    *keywd;
{
	PP_DBG (("Lib/pp/txt2usrreq(%s)", keywd));

	switch (*val = cmd_srch (keywd, atbl_usrreq)) {
	case AD_USR_NONE:
		*val = AD_USR_BASIC;
	case AD_USR_NOREPORT:
	case AD_USR_BASIC:
	case AD_USR_CONFIRM:
		return (OK);
	default:
		return (NOTOK);
	}
}


int txt2time (str, tp)
char    *str;
UTC	*tp;
{
	return (rfc2UTC (str, tp));
}

X400_Extension	*txt2extension (argv, argc)
char	*argv[];
int	argc;
{
	X400_Extension	*ext;

	if (argc <= 3)
		return NULL;
	ext = (X400_Extension *)smalloc (sizeof *ext);
	ext -> ext_int = txt2int (argv[0]);
	ext -> ext_oid = str2oid (argv[1]);
	ext -> ext_value = hex2qb (argv[2]);
	ext -> ext_criticality = cmd_srch (argv[3], tbl_crit);

	return ext;
}
