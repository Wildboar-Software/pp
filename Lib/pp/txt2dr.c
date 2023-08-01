/* txt2dr.c: converts text encoding back to dr structure */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: txt2dr.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include 	"dr.h"
#include        "tb_dr.h"
#include	"retcode.h"


#define	txt2crit(n)	cmd_srch ((n), tbl_crit)
#define txt2int(n)	atoi(n)

extern struct qbuf	*hex2qb ();
extern X400_Extension	*txt2extension ();

extern CMD_TABLE
		tbl_redir [],
		rr_rcode[],
		rr_dcode [],
		tbl_crit [],
		rrtbl [],
		drtbl [/* delivery-notification */];

extern int n_drtbl, n_rrtbl, n_rr_rcode, n_rr_dcode;


/* -------------------  Text File -> Memory  ------------------------------ */



int	txt2dr (dr, fp)
DRmpdu	*dr;
FILE	*fp;
{
	int	retval;
	char	buf[10*BUFSIZ];
	int	argc;
	char	*argv[100];

	for (;;) {
		if (rp_isbad (retval = txt2buf (buf, sizeof buf, fp))) {
			PP_LOG (LLOG_EXCEPTIONS,
				("txt2buf failed"));
			return retval;
		}
		if (retval == RP_DONE)
			return RP_DONE;

		PP_DBG (("Lib/pp/txt2dr buf='%s'", buf));

		if ((argc = str2arg (buf, 100, argv)) == 0)
			return RP_PARM;

		switch (cmdbsrch (argv[0], drtbl, n_drtbl)) {
		    case DR_MSGID:
			dr -> dr_mpduid = (MPDUid *)
				smalloc ( sizeof *dr -> dr_mpduid);
			bzero ((char *)dr -> dr_mpduid,
			       sizeof *dr -> dr_mpduid);
			if (txt2mpduid (dr -> dr_mpduid, &argv[1], argc - 1)
			    == NOTOK)
				return RP_BAD;
			break;

		    case DR_TRACE:
			if (txt2trace (&dr -> dr_trace, &argv[1], argc - 1)
			    == NOTOK)
				return RP_BAD;
			break;

		    case DR_SI_TRACE:
			if (txt2trace (&dr -> dr_subject_intermediate_trace,
				       &argv[1], argc - 1) == NOTOK)
				return RP_BAD;
			break;

		    case DR_DL_EXP_HIST:
			if (txt2dlexp (&dr -> dr_dl_history, &argv[1],
				       argc - 1) == NOTOK)
				return RP_BAD;
			break;

		    case DR_DL_EXP_HIST_CRIT:
			dr -> dr_dl_history_crit = txt2int (argv[1]);
			break;

		    case DR_REPORTING_DL_NAME:
			if (txt2fname (&dr -> dr_reporting_dl_name,
				       &dr -> dr_reporting_dl_name_crit,
				       &argv[1], argc - 1) == NOTOK)
				return RP_BAD;
			break;

		    case DR_SECURITY_LABEL:
			dr -> dr_security_label = hex2qb(argv[1]);
			if (argc < 3) break;
			dr -> dr_security_label_crit = txt2crit (argv[2]);
			break;

		    case DR_REPORTING_MTA_CERTIFICATE:
			dr -> dr_reporting_mta_certificate = hex2qb (argv[1]);
			if (argc < 3) break;
			dr -> dr_reporting_mta_certificate_crit =
				txt2crit (argv[2]);
			break;

		    case DR_REPORT_ORIGIN_AUTH_CHECK:
			dr -> dr_report_origin_auth_check = hex2qb (argv[1]);
			if (argc < 3) break;
			dr -> dr_report_origin_auth_check_crit =
				txt2crit (argv[2]);
			break;

		    case DR_PER_REPORT_EXTENSIONS:
			{
				X400_Extension *ext, **extp;

				ext = txt2extension (&argv[1], argc - 1);

				for (extp = &dr -> dr_per_report_extensions;
				     *extp; extp = &(*extp) -> ext_next)
					continue;
				*extp = ext;
			}
			break;
		    case DR_PER_ENVELOPE_EXTENSIONS:
			{
				X400_Extension *ext, **extp;

				ext = txt2extension (&argv[1], argc - 1);

				for (extp = &dr -> dr_per_envelope_extensions;
				     *extp; extp = &(*extp) -> ext_next)
					continue;
				*extp = ext;
			}
			break;

		    case DR_END_HDR:
			return rr2txt (&dr -> dr_recip_list, fp);

		    case DR_END:
			return RP_DONE;
		}
	}
}

int rr2txt (rrp, fp)
Rrinfo **rrp;
FILE	*fp;
{
	Rrinfo *rr = NULL;
	int	retval;
	char	buf[10*BUFSIZ];
	int	argc;
	char	*argv[100];
	int	type;

	for (;;) {
		if (rp_isbad (retval = txt2buf (buf, sizeof buf, fp))) {
			PP_LOG (LLOG_EXCEPTIONS,
				("txt2buf failed"));
			return retval;
		}
		if (retval == RP_DONE)
			return RP_DONE;

		PP_DBG (("Lib/pp/txt2rr buf='%s'", buf));

		if ((argc = str2arg (buf, 100, argv)) == 0)
			return RP_PARM;

		type = cmdbsrch (argv[0], rrtbl, n_rrtbl);
		if (type == RR_END)
			return RP_DONE;

		if (rr == NULL) {
			rr = (Rrinfo *) smalloc (sizeof *rr);
			bzero ((char *)rr, sizeof *rr);
			*rrp = rr;
		}
		switch (type) {
		    case RR_RECIP:
			rr -> rr_recip = txt2int (argv[1]);
			break;
			
		    case RR_SUCCESS:
			rr -> rr_report.rep_type = DR_REP_SUCCESS;
			if (txt2time (argv[1],
				      &rr -> rr_report.rep.rep_dinfo.del_time) == NOTOK)
				return RP_BAD;
			if (argc < 3) break;
			rr -> rr_report.rep.rep_dinfo.del_type =
				txt2int (argv[2]);
			break;

		    case RR_FAILURE:
			rr -> rr_report.rep_type = DR_REP_FAILURE;
			rr -> rr_report.rep.rep_ndinfo.nd_rcode =
				cmdbsrch (argv[1], rr_rcode, n_rr_rcode);
			if (argc < 3) break;
			rr -> rr_report.rep.rep_ndinfo.nd_dcode =
				cmdbsrch (argv[2], rr_dcode, n_rr_dcode);
			break;

		    case RR_CONVERTED:
			rr -> rr_converted =
				(EncodedIT *)
					smalloc (sizeof *rr -> rr_converted);
			bzero ((char *)rr -> rr_converted,
			       sizeof *rr -> rr_converted);
			if (txt2encodedinfo (rr -> rr_converted, &argv[1],
					     argc - 1) == NOTOK)
				return RP_BAD;
			break;

		    case RR_ORIGINALLY_INTENDED_RECIP:
			{
			char junk;
			if (txt2fname (&rr -> rr_originally_intended_recip,
				       &junk, &argv[1], argc - 1) == NOTOK)
				return RP_BAD;
			}
			break;

		    case RR_SUPPLEMENTARY:
			rr -> rr_supplementary = strdup (argv[1]);
			break;

		    case RR_REDIRECT_HISTORY:
			{
				Redirection *rp, **rpp,
					*txt2redir ();

				rp = txt2redir (&argv[1], argc - 1);
				for (rpp = &rr -> rr_redirect_history;
				     *rpp; rpp = &(*rpp) -> rd_next)
					continue;
				*rpp = rp;
			}
			break;

		    case RR_REDIRECT_HISTORY_CRIT:
			rr -> rr_redirect_history_crit = txt2crit (argv[1]);
			break;

		    case RR_PHYSICAL_FWD:
			if (txt2fname (&rr -> rr_physical_fwd_addr,
				       &rr -> rr_physical_fwd_addr_crit,
				       &argv[1], argc - 1) == NOTOK)
				return RP_BAD;
			break;

		    case RR_RECIP_CERTIFICATE:
			rr -> rr_recip_certificate = hex2qb (argv[1]);
			if (argc < 3) break;
			rr -> rr_recip_certificate_crit = txt2crit (argv[2]);
			break;

		    case RR_REPORT_ORIGIN_CHECK:
			rr -> rr_report_origin_authentication_check =
				hex2qb (argv[1]);
			if (argc < 3) break;
			rr -> rr_report_origin_authentication_check_crit =
				txt2crit (argv[2]);
			break;

		    case RR_ARRIVAL:
			if (txt2time (argv[1], &rr -> rr_arrival) == NOTOK)
				return RP_BAD;
			break;

		    case RR_PER_RECIP_EXTENSIONS:
			{
				X400_Extension *ext, **extp;

				ext = txt2extension (&argv[1], argc - 1);
				for (extp = &rr -> rr_per_recip_extensions;
				     *extp; extp = &(*extp) -> ext_next)
					continue;
				*extp = ext;
			}
			break;

		    case RR_END_RECIP:
			return rr2txt (&rr -> rr_next, fp);

		    case RR_END:
			return RP_DONE;
		}
	}
}

Redirection *txt2redir (argv, argc)
char	*argv[];
int	argc;
{
	Redirection *rp;

	rp = (Redirection *) smalloc (sizeof *rp);
	bzero ((char *)rp, sizeof *rp);
	
	if (argc < 1) return rp;
	rp -> rd_addr = isstr (argv[0]) ? strdup (argv[0]) : NULLCP;
	rp -> rd_dn =isstr (argv[1]) ? strdup (argv[1]) : NULLCP;
	rp -> rd_reason = cmd_srch (argv[2], tbl_redir);
	return rp;
}
