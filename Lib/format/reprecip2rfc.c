/* reprecip2rfc.c - Converts a Rrseq struct into a RFC string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/reprecip2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/reprecip2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: reprecip2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include	"dr.h"


extern CMD_TABLE
		rr_rcode [/* reason-codes */],
		rr_dcode [/* diagnostic-codes */];

extern UTC	utclocalise();
extern int	x400eits2rfc();
static int	lastrace2rfc ();

/* ---------------------  Begin  Routines  -------------------------------- */




int reprecip2rfc (rp, ap, buffer)  /* ReportedRecipientInfo -> RFC */
Rrinfo	*rp;
ADDR	*ap;
char    *buffer;
{
	int     i;
	char 	*cp;

	if (rp == NULL)
		return OK;

	cp = buffer;

	if (rp -> rr_recip) {
		(void) sprintf (cp, "%s; ", ap -> ad_r822adr);
		cp += strlen(cp);
	}
	else {
		*buffer = '\0';
		return OK;
	}

	if (lastrace2rfc (rp, cp) == NOTOK)
		return NOTOK;

	cp += strlen(cp);

	(void) sprintf (cp, "ext %d ", ap -> ad_extension);
	cp += strlen(cp);

	i = mem2prf (ap -> ad_resp, ap -> ad_mtarreq, ap -> ad_usrreq);
	(void) sprintf (cp, "flags %02d", i);
	cp += strlen(cp);

	if (rp -> rr_originally_intended_recip &&
	    rp -> rr_originally_intended_recip->fn_addr) {
		(void) sprintf (cp, " intended %s",
				rp -> rr_originally_intended_recip -> fn_addr);
		cp += strlen(cp);
	}

	if (rp -> rr_supplementary) {
		(void) sprintf (cp, " info %s", rp->rr_supplementary);
		cp += strlen(cp);
	}
	else if (rp -> rr_report.rep_type == DR_REP_FAILURE) {
		(void) sprintf (cp, " info %s",
			"Extra information on the failure was not supplied");
		cp += strlen(cp);
	}
	PP_DBG (("Lib/rri2rfc returns (%s)", buffer));

	return OK;
}

static int lastrace2rfc (rp, buffer)   /* LastTraceInformation -> RFC */
Rrinfo	*rp;
char            *buffer;
{
	Delinfo         *dliv;
	NonDelinfo      *ndliv;
	char            tbuf [BUFSIZ];
	UTC		lut;
	char	*cp = buffer;

	switch (rp -> rr_report.rep_type) {
	case DR_REP_SUCCESS:
		dliv = &rp -> rr_report.rep.rep_dinfo;
		if (dliv->del_time == NULLUTC) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("lastrace2rfc: missing deliv time"));
			return NOTOK;
		}
		lut = utclocalise(dliv -> del_time);
		if(UTC2rfc (lut, tbuf) == NOTOK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("lastrace2rfc: bad deliv time"));
			return NOTOK;
		}
		free((char *) lut);
		(void) sprintf (cp, "SUCCESS %s; %d; ",
				tbuf, dliv -> del_type);
		break;
	case DR_REP_FAILURE:
		ndliv = &rp -> rr_report.rep.rep_ndinfo;
		(void) sprintf (cp, "FAILURE %d (%s); %d (%s); ",
			ndliv -> nd_rcode,
			rcmd_srch (ndliv -> nd_rcode, rr_rcode),
			ndliv -> nd_dcode,
			rcmd_srch (ndliv -> nd_dcode, rr_dcode));
		break;
	default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/lastrace2rfc/Unknown report type %d",
			rp -> rr_report.rep_type));
		*buffer = '\0';
		return NOTOK;
	}
	cp += strlen(cp);


	if (rp -> rr_arrival == NULLUTC) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("lastrace2rfc: missing arrival time"));
		return NOTOK;
	}
	lut = utclocalise (rp -> rr_arrival);
	if(UTC2rfc (lut, tbuf) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("lastrace2rfc: bad arrival time"));
		return NOTOK;
	}
	free ((char *) lut);

	(void) sprintf (cp, "%s;", tbuf);
	cp += strlen(cp);

	if (rp -> rr_converted) {
		tbuf[0] = NULL;
		if (x400eits2rfc (rp -> rr_converted,
				  tbuf) == NOTOK)
			return NOTOK;
		if (tbuf[0])
			(void) sprintf (cp, " converted(%s);", tbuf);
	}
	PP_DBG (("Lib/lastrace2rfc returns (%s)", buffer));

	return OK;
}
