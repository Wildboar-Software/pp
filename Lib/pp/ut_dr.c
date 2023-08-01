/* ut_dr.c: DR utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_dr.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include 	"dr.h"



/* ---------------------  Begin  Routines  -------------------------------- */




void dr_init (dreport)
register DRmpdu         *dreport;
{
        PP_DBG (("Lib/pp/dr_init()"));
        bzero ((char *)dreport, sizeof(*dreport));
}




void dr_free (dr)
register DRmpdu         *dr;
{
        PP_DBG (("Lib/pp/dr_free()"));

        /* -- the envelope part is set from Q_struct - so not freed -- */

        if (dr->dr_mpduid)
		MPDUid_free (dr->dr_mpduid);
        if (dr->dr_trace)
		trace_free (dr->dr_trace);
        if (dr->dr_subject_intermediate_trace)
		trace_free (dr->dr_subject_intermediate_trace);
	if (dr -> dr_dl_history)
		dlh_free (dr -> dr_dl_history);
	if (dr -> dr_reporting_dl_name)
		fn_free (dr -> dr_reporting_dl_name);
	if (dr -> dr_security_label)
		qb_free (dr -> dr_security_label);
	if (dr -> dr_reporting_mta_certificate)
		qb_free (dr -> dr_reporting_mta_certificate);
	if (dr -> dr_report_origin_auth_check)
		qb_free (dr -> dr_report_origin_auth_check);
	if (dr -> dr_per_report_extensions)
		extensions_free (dr -> dr_per_report_extensions);
        if (dr->dr_recip_list)
		rrinfo_free(dr -> dr_recip_list);

        dr_init (dr);
}

void rrinfo_free (rr)  /* -- ReportedRecipientInfo -- */
Rrinfo   *rr;
{
        if (rr == (Rrinfo *) NULL)  return;

        rrinfo_free (rr->rr_next);

	if (rr -> rr_originally_intended_recip)
		fn_free (rr -> rr_originally_intended_recip);
        if (rr->rr_supplementary != NULLCP)
		free (rr->rr_supplementary);
	if (rr -> rr_redirect_history)
		redirection_free (rr -> rr_redirect_history);
	if (rr -> rr_physical_fwd_addr)
		fn_free (rr -> rr_physical_fwd_addr);
	if (rr -> rr_recip_certificate)
		qb_free (rr -> rr_recip_certificate);
	if (rr -> rr_report_origin_authentication_check)
		qb_free (rr -> rr_report_origin_authentication_check);
	if (rr -> rr_arrival)
		free ((char *)rr->rr_arrival);
	if (rr -> rr_per_recip_extensions)
		extensions_free (rr -> rr_per_recip_extensions);
	free ((char *) rr);
}
