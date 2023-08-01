/* mta2txt.c: handle text encoding of various mta structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/mta2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/mta2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: mta2txt.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include	"mta.h"
#include        "tb_com.h"
#include 	"tb_a.h"
#include        "tb_p1.h"

extern  char *genstore ();
char            *utctime ();
char            *int2txt ();
extern	int	argv2fp ();

static char *P1_rreq2txt ();
#define crit2txt(n)	rcmd_srch ((n), tbl_crit)

extern CMD_TABLE
		p1tbl_mpduid [/* mpdu-identifier */],
		p1tbl_trace [/* trace-information */],
		p1tbl_globaldomid [/* global-domain-identifier */],
		p1tbl_domsinfo [/* domain-supplied-info */],
		p1tbl_action [/* domain-supplied-info-action */],
		p1tbl_encinfoset [/* encode-info-types-set */],
		tbl_crit [],
		atbl_addr [/* address-lines */],
		tbl_bool [],
		atbl_mtarreq [/* mta-report-request */],
		atbl_usrreq [/* user-report-request */];


/* ---------------------  Memory->Text File  ---------------------------- */

void	globaldomid2txt ();
void	encodedinfo2txt ();

void mpduid2txt (mp, argv, argcp)  /* MPDUIdentifier->Txt */
MPDUid  *mp;
char    *argv[];
int     *argcp;
{
	int     n = *argcp;
	PP_DBG (("Lib/pp/mpduid2txt()"));

	if (mp->mpduid_string != NULL) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (MPDUID_STRING, p1tbl_mpduid);
		argv[n++] = mp->mpduid_string;
		*argcp = n;
	}

	globaldomid2txt (&mp->mpduid_DomId, argv, argcp);
}


void globaldomid2txt (gp, argv, argcp)  /* GlobalDomainIdentifier->Txt */
GlobalDomId     *gp;
char    *argv[];
int     *argcp;
{
	int     n = *argcp;

	PP_DBG (("Lib/pp/golbaldomid2txt (c=%s, a=%s, p=%s)",
		gp->global_Country ? gp->global_Country : "notspecified", 
		gp->global_Admin ? gp->global_Admin : "notspecified", 
		gp->global_Private && isstr (gp->global_Private) 
				? gp->global_Private : "notspecified"));

	if (gp->global_Country != NULLCP) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (GLOBAL_COUNTRY, p1tbl_globaldomid);
		argv[n++] = gp->global_Country;
	}

	if (gp->global_Admin != NULLCP) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (GLOBAL_ADMIN, p1tbl_globaldomid);
		argv[n++] = gp->global_Admin;
	}

	if (gp->global_Private != NULLCP) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (GLOBAL_PRIVATE, p1tbl_globaldomid);
		argv[n++] = gp->global_Private;
	}
	*argcp = n;
}

extern char *time2txt ();

void domsinfo2txt (sp, argv, argcp)  /* DomainSuppliedInfo->Txt */
DomSupInfo      *sp;
char    *argv[];
int     *argcp;
{
	int     n = *argcp;

	PP_DBG (("Lib/pp/domsinfo2txt()"));

	if (sp->dsi_time != 0) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (DSI_TIME, p1tbl_domsinfo);
		argv[n++] = time2txt (sp->dsi_time);
	}

	if (sp->dsi_deferred != 0) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (DSI_DEFERRED, p1tbl_domsinfo);
		argv[n++] = time2txt (sp->dsi_deferred);
	}

	if (sp->dsi_action == ACTION_RELAYED ||
	    sp->dsi_action == ACTION_ROUTED)
	{
		argv[n++] = "=";
		argv[n++] = rcmd_srch (DSI_ACTION, p1tbl_domsinfo);
		argv[n++] = int2txt (sp->dsi_action);
	}

	if (sp->dsi_converted.eit_types != NULL ||
	    sp->dsi_converted.eit_g3parms != NULL  ||
	    sp->dsi_converted.eit_tTXparms != NULL ||
	    sp->dsi_converted.eit_presentation != NULL)
	{
		argv[n++] = rcmd_srch (DSI_CONVERTED, p1tbl_domsinfo);
		encodedinfo2txt (&sp->dsi_converted, argv, &n);
	}


	if (sp->dsi_attempted_md.global_Country != NULLCP ||
	    sp->dsi_attempted_md.global_Admin   != NULLCP ||
	    sp->dsi_attempted_md.global_Private != NULLCP) {
		argv[n++] = rcmd_srch (DSI_ATTEMPTED, p1tbl_domsinfo);
		globaldomid2txt (&sp->dsi_attempted_md, argv, &n);
	}
	*argcp = n;
}


void action2txt (fp, pvkey, action)  /*DomainSuppliedInfo (action)->Txt */
FILE    *fp;
char    *pvkey;
int     action;
{
	char    *keywd;

	PP_DBG (("Lib/pp/action2txt(%s %d)", pvkey, action));

	keywd = rcmd_srch (action, p1tbl_action);
	(void) fprintf (fp, "%s%s\n", pvkey, keywd);
}


void encodedinfo2txt (ep, argv, argcp)  /* EncodedInformationTypes->Txt */
EncodedIT               *ep;
char    *argv[];
int     *argcp;
{
	char    buf[LINESIZE];
	int     n = *argcp;
	extern char *listbpt2txt ();

	PP_DBG (("Lib/encodedinfo2txt('%d' '%d' '%d')",
		 ep->eit_g3parms, ep->eit_tTXparms,
		 ep->eit_presentation));

	(void) listbpt2txt (ep -> eit_types, buf);
	if (buf[0]) {
		PP_DBG (("Lib/listbpt2txt ('%s')", buf));
		argv[n++] = "=";
		argv[n++] = rcmd_srch (EI_BIT_STRING, p1tbl_encinfoset);
		argv[n++] = genstore(buf);
	}

	if (ep->eit_g3parms != 0){
		argv[n++] = "=";
		argv[n++] = rcmd_srch (EI_G3NONBASIC, p1tbl_encinfoset);
		argv[n++] = int2txt ((int)ep -> eit_g3parms);
	}
	if (ep->eit_tTXparms != 0){
		argv[n++] = "=";
		argv[n++] = rcmd_srch (EI_TELETEXNONBASIC, p1tbl_encinfoset);
		argv[n++] = int2txt ((int)ep -> eit_tTXparms);
	}
	if (ep->eit_presentation != 0){
		argv[n++] = "=";
		argv[n++] = rcmd_srch (EI_PRESENTATION, p1tbl_encinfoset);
		argv[n++] = int2txt ((int)ep->eit_presentation);
	}
	*argcp = n;
}


char *listbpt2txt (bpt, buf)
LIST_BPT	*bpt;
char	*buf;
{
	LIST_BPT	*list;

	PP_DBG (("Lib/pp/listbpt2txt()"));
	buf[0] = 0;
	for (list = bpt; list; list = list->li_next) {
		if (list->li_name == NULLCP)
			continue;
		(void) strcat (buf, list->li_name);
		if (list->li_next)
			(void) strcat (buf, ",");
	}
	return buf;
}

void trace2txt (tp, argv, argcp)  /* TraceInformation->Txt */
Trace   *tp;
char    *argv[];
int     *argcp;
{
	int     n = *argcp;
	PP_DBG (("Lib/pp/trace2txt()"));

	if (tp->trace_mta != NULLCP) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (TRACE_MTA, p1tbl_trace);
		argv[n++] = tp->trace_mta;
		*argcp = n;
	}
	globaldomid2txt (&tp->trace_DomId, argv, argcp);
	domsinfo2txt (&tp->trace_DomSinfo, argv, argcp);
	n = *argcp;
	argv[n++] = rcmd_srch (EOB, p1tbl_trace);
	*argcp = n;
}


/* ReportRequest->Txt */
void repreq2txt (resp, mta, usr, argv, argcp)
int     resp;
int     mta;
int     usr;
char    *argv[];
int     *argcp;
{
	char    *p;
	int     n = *argcp;

	if ((p = P1_rreq2txt (AD_RESPONSIBILITY, resp)) != NULLCP) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (AD_RESPONSIBILITY, atbl_addr);
		argv[n++] = p;
	}
	if ((p = P1_rreq2txt (AD_MTA_REP_REQ, mta)) != NULLCP) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (AD_MTA_REP_REQ, atbl_addr);
		argv[n++] = p;
	}
	if ((p = P1_rreq2txt (AD_USR_REP_REQ, usr)) != NULLCP) {
		argv[n++] = "=";
		argv[n++] = rcmd_srch (AD_USR_REP_REQ, atbl_addr);
		argv[n++] = p;
	}
	*argcp = n;
}


static char* P1_rreq2txt (type, val)  /* ReportRequest->Txt */
int     type;
int     val;
{
	PP_DBG (("Lib/pp/P1_rreq2txt(%d  %d)", type, val));

	switch (type) {
	    case AD_RESPONSIBILITY:
		switch (val) {
		    case YES:
		    case NO:
			return rcmd_srch (val, tbl_bool);
		    default:
			return (NULLCP);
		}
	    case AD_MTA_REP_REQ:
		switch (val) {
		    case AD_MTA_NONE:
		    case AD_MTA_BASIC:
		    case AD_MTA_CONFIRM:
		    case AD_MTA_AUDIT_CONFIRM:
			return rcmd_srch (val, atbl_mtarreq);
		    default:
			return (NULLCP);
		}
	    case AD_USR_REP_REQ:
		switch (val) {
		    case AD_USR_NONE:
		    case AD_USR_NOREPORT:
		    case AD_USR_BASIC:
		    case AD_USR_CONFIRM:
			return rcmd_srch (val, atbl_usrreq);
		    default:
			return (NULLCP);
		}
	}
	return (NULLCP);
}


char    *time2txt (t)
UTC t;
{
	static char	buffer[LINESIZE];
	if (UTC2rfc (t, buffer) == NOTOK) {
		UTC ut = utcnow ();
		(void) UTC2rfc (ut, buffer);
		free ((char *)ut);
	}
	return buffer;
}


char    *int2txt (val)
int    val;
{
	char    buf[LINESIZE];

	(void) sprintf (buf, "%ld", val);
	return genstore (buf);
}

char	*qb2hex (qb)
struct qbuf *qb;
{
	char	*hexbuf;
	char	*str;

	hexbuf = smalloc (2 *qb -> qb_len + 1);
	str = qb2str(qb);

	hexbuf[explode (hexbuf, (u_char *)str, qb -> qb_len)] = 0;
	free (str);

	return hexbuf;
}

void dlexp2txt (dlp, argv, argcp)
DLHistory *dlp;
char	*argv[];
int	*argcp;
{
	int	n = *argcp;

	argv[n++] = dlp -> dlh_addr ? dlp -> dlh_addr : "";
	argv[n++] = dlp -> dlh_dn ? dlp -> dlh_dn : "";
	argv[n++] = dlp -> dlh_time ? time2txt (dlp -> dlh_time) : "";
	*argcp = n;
}

void fname2txt (fn, crt, argv, argcp)
FullName *fn;
int	crt;
char	*argv[];
int	*argcp;
{
	int	n = *argcp;

	argv[n++] = fn -> fn_addr ? fn -> fn_addr : "";
	argv[n++] = fn -> fn_dn ? fn -> fn_dn : "";
	if (crt)
		argv[n++] = crit2txt (crt);
	*argcp = n;
}

int extension2txt (ext, fp, argv, argcp)
X400_Extension *ext;
FILE	*fp;
char	*argv[];
int	*argcp;
{
	int	n = *argcp;

	argv[n++] = int2txt (ext -> ext_int);
	argv[n++] = ext -> ext_oid ? sprintoid (ext -> ext_oid) : "";
	argv[n++] = qb2hex (ext -> ext_value);
	argv[n++] = rcmd_srch (ext -> ext_criticality, tbl_crit);
	argv[n++] = NULLCP;

	if (argv2fp (fp, argv) == NOTOK)
		return NOTOK;
	free (argv[2]);
	return OK;
}
