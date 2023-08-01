/* dr2txt.c: convert dr structure to text encooding */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/dr2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/dr2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: dr2txt.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include 	"dr.h"
#include        "tb_dr.h"

#define	crit2txt(n)	rcmd_srch ((n), tbl_crit)
static int rr2txt ();
static void drredir2txt ();

extern	char	*qb2hex ();
extern	char	*int2txt ();
extern	char	*time2txt ();
extern	void	genreset ();
extern	void	mpduid2txt ();
extern	int	argv2fp ();
extern	void	trace2txt ();
extern	void	dlexp2txt ();
extern	void	fname2txt ();
extern	int	extension2txt ();
extern	void	encodedinfo2txt ();

extern CMD_TABLE
		tbl_redir [],
		rr_rcode[],
		rr_dcode [],
		tbl_crit[],
		rrtbl [],
		drtbl [];


/* -------------------  Memory -> Text File  ------------------------------ */


int dr2txt (fp, dr)   /* DeliveryReport -> Txt */
FILE    *fp;
DRmpdu  *dr;
{
	char	*argv[100];
	int	argc;
	Trace	*trace;
	DLHistory	*dlh;
	X400_Extension	*ext;
	Rrinfo		*rr;

	PP_DBG (("Lib/pp/dr2txt()"));

	genreset ();

	if (dr -> dr_mpduid) {
		argv[0] = rcmd_srch (DR_MSGID, drtbl);
		argc = 1;
		mpduid2txt (dr -> dr_mpduid, argv, &argc);
		if (argc > 1) {
			argv[argc] = NULLCP;
			if (argv2fp (fp, argv) == NOTOK)
				return NOTOK;
		}
	}

	argv[0] = rcmd_srch (DR_TRACE, drtbl);
	for (trace = dr -> dr_trace; trace; trace = trace -> trace_next) {
		argc = 1;
		trace2txt (trace, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (DR_SI_TRACE, drtbl);
	for (trace = dr -> dr_subject_intermediate_trace; trace;
	     trace = trace -> trace_next) {
		argc = 1;
		trace2txt (trace, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (DR_DL_EXP_HIST, drtbl);
	for (dlh = dr -> dr_dl_history; dlh; dlh = dlh -> dlh_next) {
		argc = 1;
		dlexp2txt (dlh, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (dr -> dr_dl_history_crit) {
		argv[0] = rcmd_srch (DR_DL_EXP_HIST_CRIT, drtbl);
		argv[1] = crit2txt (dr -> dr_dl_history_crit);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (dr -> dr_reporting_dl_name) {
		argv[0] = rcmd_srch (DR_REPORTING_DL_NAME, drtbl);
		argc = 1;
		fname2txt (dr -> dr_reporting_dl_name,
			   dr -> dr_reporting_dl_name_crit, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (dr -> dr_security_label) {
		argv[0] = rcmd_srch (DR_SECURITY_LABEL, drtbl);
		argv[1] = qb2hex (dr -> dr_security_label);
		argv[2] = crit2txt (dr -> dr_security_label_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (dr -> dr_reporting_mta_certificate) {
		argv[0] = rcmd_srch (DR_REPORTING_MTA_CERTIFICATE, drtbl);
		argv[1] = qb2hex (dr -> dr_reporting_mta_certificate);
		argv[2] = crit2txt (dr -> dr_reporting_mta_certificate_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (dr -> dr_report_origin_auth_check) {
		argv[0] = rcmd_srch (DR_REPORT_ORIGIN_AUTH_CHECK, drtbl);
		argv[1] = qb2hex (dr -> dr_report_origin_auth_check);
		argv[2] = crit2txt (dr -> dr_report_origin_auth_check_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	argv[0] = rcmd_srch (DR_PER_ENVELOPE_EXTENSIONS, drtbl);
	for (ext = dr -> dr_per_envelope_extensions; ext;
	     ext = ext -> ext_next) {
		argc = 1;
		if (extension2txt (ext, fp, argv, &argc) == NOTOK)
			return NOTOK;
	}


	argv[0] = rcmd_srch (DR_PER_REPORT_EXTENSIONS, drtbl);
	for (ext = dr -> dr_per_report_extensions; ext;
	     ext = ext -> ext_next) {
		argc = 1;
		if (extension2txt (ext, fp, argv, &argc) == NOTOK)
			return NOTOK;
	}
	(void) fprintf (fp, "%s\n", rcmd_srch (DR_END_HDR, drtbl));
	
	for (rr = dr -> dr_recip_list; rr; rr = rr -> rr_next)
		if (rr2txt (rr, fp) == NOTOK)
			return NOTOK;
	(void) fprintf (fp, "%s\n", rcmd_srch (RR_END, rrtbl));
	(void) fflush (fp);
	return ferror(fp) ? NOTOK : OK;
}

static int rr2txt (rr, fp)
Rrinfo *rr;
FILE	*fp;
{
	char	*argv[30];
	int	argc;
	Redirection	*redir;
	X400_Extension	*ext;

	genreset ();

	argv[0] = rcmd_srch (RR_RECIP, rrtbl);
	argv[1] = int2txt (rr -> rr_recip);
	argv[2] = NULLCP;
	if (argv2fp (fp, argv) == NOTOK)
		return NOTOK;

	if (rr -> rr_report.rep_type == DR_REP_SUCCESS) {
		argv[0] = rcmd_srch (RR_SUCCESS, rrtbl);
		argv[1] = time2txt (rr -> rr_report.rep.rep_dinfo.del_time);
		argv[2] = int2txt (rr -> rr_report.rep.rep_dinfo.del_type);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (rr -> rr_report.rep_type == DR_REP_FAILURE) {
		argv[0] = rcmd_srch (RR_FAILURE, rrtbl);
		argv[1] = rcmd_srch (rr -> rr_report.rep.rep_ndinfo.nd_rcode,
				     rr_rcode);
		argv[2] = rcmd_srch (rr -> rr_report.rep.rep_ndinfo.nd_dcode,
				     rr_dcode);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (rr -> rr_converted) {
		argv[0] = rcmd_srch (RR_CONVERTED, rrtbl);
		argc = 1;
		encodedinfo2txt (rr -> rr_converted, argv, &argc);
		argv[argc] = NULLCP;
		if (argc != 1)
			if (argv2fp (fp, argv) == NOTOK)
				return NOTOK;
	}

	if (rr -> rr_originally_intended_recip) {
		argv[0] = rcmd_srch (RR_ORIGINALLY_INTENDED_RECIP, rrtbl);
		argc = 1;
		fname2txt (rr -> rr_originally_intended_recip, 0,
			   argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (rr -> rr_supplementary) {
		argv[0] = rcmd_srch (RR_SUPPLEMENTARY, rrtbl);
		argv[1] = rr -> rr_supplementary;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (RR_REDIRECT_HISTORY, rrtbl);
	for (redir = rr -> rr_redirect_history; redir;
	     redir = redir -> rd_next) {
		argc = 1;
		drredir2txt (redir, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (rr -> rr_redirect_history_crit) {
		argv[0] = rcmd_srch (RR_REDIRECT_HISTORY_CRIT, rrtbl);
		argv[1] = crit2txt (rr -> rr_redirect_history_crit);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (rr -> rr_physical_fwd_addr) {
		argv[0] = rcmd_srch (RR_PHYSICAL_FWD, rrtbl);
		fname2txt (rr -> rr_physical_fwd_addr,
			   rr -> rr_physical_fwd_addr_crit, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (rr -> rr_recip_certificate) {
		argv[0] = rcmd_srch (RR_RECIP_CERTIFICATE, rrtbl);
		argv[1] = qb2hex (rr -> rr_recip_certificate);
		argv[2] = crit2txt (rr -> rr_recip_certificate_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (rr -> rr_report_origin_authentication_check) {
		argv[0] = rcmd_srch (RR_REPORT_ORIGIN_CHECK, rrtbl);
		argv[1] = qb2hex (rr -> rr_report_origin_authentication_check);
		argv[2] =
			crit2txt (rr -> rr_report_origin_authentication_check_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (rr -> rr_arrival) {
		argv[0] = rcmd_srch (RR_ARRIVAL, rrtbl);
		argv[1] = time2txt (rr -> rr_arrival);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (RR_PER_RECIP_EXTENSIONS, rrtbl);
	for (ext = rr -> rr_per_recip_extensions; ext; ext = ext -> ext_next) {
		argc = 1;
		if (extension2txt (ext, fp, argv, &argc) == NOTOK)
			return NOTOK;
	}
	(void) fprintf (fp, "%s\n", rcmd_srch (RR_END_RECIP, rrtbl));
	return ferror (fp) ? NOTOK : OK;
}

static void drredir2txt (rp, argv, argcp)
Redirection *rp;
char	*argv[];
int	*argcp;
{
	int	n = *argcp;
	argv[n++] = rp -> rd_addr ? rp -> rd_addr : "";
	argv[n++] = rp -> rd_dn ? rp -> rd_dn : "";
	argv[n++] = rcmd_srch (rp -> rd_reason, tbl_redir);
	*argcp = n;
}
