/* x400topp.c: X400(1988) protocol to submit format - inbound */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400topp.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400topp.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: x400topp.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */


#include "Trans-types.h"
#include "Ext-types.h"
#include "util.h"
#include "chan.h"
#include "q.h"
#include "adr.h"
#include "or.h"
#include "prm.h"
#include "dr.h"
#include "retcode.h"
#include <isode/rtsap.h>

#define bit_ison(x,y)	(bit_test(x,y) == 1)
extern char *undefined_bodyparts;
extern ORName *orname2orn ();
extern char *dn2str ();

static int load_msgid(), load_addr(), load_eits(), load_84eit(),
	load_content(), load_pmf(), load_time(), load_recips(),
	load_p3_recips(), load_prf_ext(), load_trace(),
	make_trace(), load_gdi(), load_reportenv(), load_reportcont(),
	load_drc_pr_fields(), load_lasttrace(), load_report(),
	load_repe_ext(), load_cont_ext(), load_prr_ext(),
	load_fullname(), load_pm_extensions(), do_local_extension(),
	ext_decode_dlh(), do_global_extension(), ext_ora(),
	ext_inttrace(), decode_1redir(), do_global_ext_prf(),
	do_global_repe_prf(), do_global_cont_prf(), do_global_rr_prf();

static int rebuild_eits (eits, orig, trace)
EncodedIT *eits, *orig;
Trace	*trace;
{
	Trace	*tp;
	LIST_BPT *lasteit = NULL;

	for (tp = trace; tp; tp = tp -> trace_next)
		if (tp -> trace_DomSinfo.dsi_converted.eit_types)
			lasteit = tp -> trace_DomSinfo.dsi_converted.eit_types;
	if (lasteit)
		list_bpt_add (&eits -> eit_types, list_bpt_dup (lasteit));
	else
		list_bpt_add (&eits -> eit_types,
			      list_bpt_dup (orig -> eit_types));
	return OK;
}

static int rebuild_dreits (eits, dr)
EncodedIT *eits;
DRmpdu *dr;
{
	Trace	*tp;
	Rrinfo *rr;
	LIST_BPT *ep;

	for (rr = dr -> dr_recip_list; rr; rr = rr-> rr_next)  {
		if (rr -> rr_report.rep_type == DR_REP_FAILURE &&
		    rr -> rr_converted)
			for (ep = rr -> rr_converted->eit_types; ep;
			     ep = ep->li_next)
				if (list_bpt_find(eits -> eit_types, ep -> li_name) == NULL)
					list_bpt_add (&eits -> eit_types,
						      list_bpt_new (ep->li_name));
	}
	return OK;
}

int load_qstruct (qp, mte)
Q_struct *qp;
struct type_MTA_MessageTransferEnvelope *mte;
{
	int retval;

	if ((retval = load_msgid (&qp->msgid, mte -> message__identifier)) != OK)
		return retval;

	if ((retval = load_addr (&qp->Oaddress, mte -> originator__name)) != OK)
		return retval;

	if (mte -> original__encoded__information__types &&
	    (retval = load_eits (&qp -> orig_encodedinfo,
		       mte -> original__encoded__information__types)) != OK)
		return retval;

	if ((retval = load_content (qp, mte -> member_MTA_13)) != OK)
		return retval;

	if ( mte -> content__identifier &&
	    (qp -> ua_id = qb2str (mte -> content__identifier)) == NULL)
		return NOTOK;

	if (mte -> priority)
		qp -> priority = mte -> priority -> parm;

	if (mte -> per__message__indicators &&
	    (retval = load_pmf (qp, mte -> per__message__indicators)) != OK)
		return retval;

	if (mte -> deferred__delivery__time &&
	    (retval = load_time (&qp -> defertime, mte -> deferred__delivery__time)) != OK)
		return retval;

	if (mte -> per__domain__bilateral__information)
		PP_LOG (LLOG_EXCEPTIONS,
			("Bilateral Info supplied (ignored)"));

	if ((retval = load_trace (&qp -> trace, mte -> trace__information)) != OK)
		return retval;


	if ((retval = load_recips (&qp -> Raddress,
			 mte -> per__recipient__fields)) != OK)
		return retval;

	if (mte -> extensions &&
	    (retval = load_pm_extensions (qp, mte -> extensions,1,1)) != OK)
		return retval;
	if ((retval = rebuild_eits (&qp -> encodedinfo,
				    &qp -> orig_encodedinfo,
				    qp -> trace)) != OK)
		return retval;

	return OK;
}

int load_probe (qp, probe)
Q_struct *qp;
struct type_Trans_ProbeAPDU *probe;
{
	int retval;

	if ((retval = load_msgid (&qp -> msgid,
				  probe -> probe__identifier)) != OK ||
	    (retval = load_addr (&qp -> Oaddress,
				 probe -> originator__name)) != OK)
		return retval;

	if (probe -> original__encoded__information__types &&
	    (retval = load_eits (&qp -> orig_encodedinfo,
		       probe -> original__encoded__information__types))
	    != OK)
		return retval;

	if ((retval = load_content (qp, probe -> member_MTA_15)) != OK)
		return retval;

	if (probe -> content__identifier &&
	    (qp -> ua_id = qb2str (probe -> content__identifier)) == NULL)
		return NOTOK;

	if (probe -> content__length)
		qp -> msgsize = probe -> content__length -> parm;
	
	if (probe -> per__message__indicators &&
	    (retval = load_pmf (qp, probe -> per__message__indicators)) != OK)
		return retval;

	if (probe -> per__domain__bilateral__information)
		PP_LOG (LLOG_EXCEPTIONS,
			("Bilateral Info supplied (ignored)"));

	if ((retval = load_trace (&qp -> trace,
				  probe -> trace__information)) != OK)
		return retval;

	if ((retval = load_recips (&qp -> Raddress,
				   probe -> per__recipient__fields))
	    != OK)
		return retval;

	if ((retval = load_pm_extensions (qp,
					  probe -> extensions,1,0)) != OK ||
	    (retval = rebuild_eits (&qp -> encodedinfo,
				    &qp -> orig_encodedinfo,
				    qp -> trace)) != OK)
		return retval;
	return OK;
}

int load_dr (qp, dr, report)
Q_struct *qp;
DRmpdu *dr;
struct type_Trans_ReportAPDU *report;
{
	int retval;

	qp -> msgtype = MT_DMPDU;
	if ((retval = load_reportenv (qp, dr, report -> envelope)) != OK ||
	    (retval = load_reportcont (qp, dr, report -> content)) != OK ||
	    (rebuild_dreits (&qp -> encodedinfo, dr) != OK))
		return retval;
	return OK;
}

int load_p3_qstruct (qp, mte)
Q_struct *qp;
struct type_MTA_MessageSubmissionEnvelope *mte;
{
	int retval;

	if ((retval = load_addr (&qp->Oaddress,
				 mte -> originator__name)) != OK)
		return retval;

	if (mte -> original__eits) {
		if ((retval = load_eits (&qp -> orig_encodedinfo,
			       mte -> original__eits)) != OK ||
		    (retval = load_eits (&qp -> encodedinfo,
					 mte -> original__eits)) != OK)
			return retval;
	}

	if ((retval = load_content (qp, mte -> member_MTA_6)) != OK)
		return retval;

	if ( mte -> content__identifier &&
	    (qp -> ua_id = qb2str (mte -> content__identifier)) == NULL)
		return NOTOK;

	if (mte -> priority)
		qp -> priority = mte -> priority -> parm;

	if (mte -> per__message__indicators &&
	    (retval = load_pmf (qp, mte -> per__message__indicators)) != OK)
		return retval;

	if (mte -> deferred__delivery__time &&
	    (retval = load_time (&qp -> defertime,
		       mte -> deferred__delivery__time)) != OK)
		return retval;

	if (mte -> extensions &&
	    (retval = load_pm_extensions (qp, mte -> extensions,0,1)) != OK)
		return retval;

	if ((retval = load_p3_recips (&qp -> Raddress,
			    mte -> per__recipient__fields)) != OK)
		return retval;
	return OK;
}

int load_p3_probe (qp, probe)
Q_struct *qp;
struct type_MTA_ProbeSubmissionEnvelope  *probe;
{
	int retval;

	if ((retval = load_addr (&qp -> Oaddress,
				 probe -> originator__name)) != OK)
		return retval;

	if (probe -> original__eits) {
		if ((retval = load_eits (&qp -> orig_encodedinfo,
			       probe -> original__eits)) != OK ||
		    (retval = load_eits (&qp -> encodedinfo, /* necessary */
					 probe -> original__eits)) != OK)
			return retval;
	}

	if ((retval = load_content (qp, probe -> member_MTA_7)) != OK)
		return retval;

	if (probe -> content__identifier &&
	    (qp -> ua_id = qb2str (probe -> content__identifier)) == NULL)
		return NOTOK;

	if (probe -> content__length)
		qp -> msgsize = probe -> content__length -> parm;
	
	if (probe -> per__message__indicators &&
	    (retval = load_pmf (qp, probe -> per__message__indicators)) != OK)
		return retval;

	if ((retval = load_p3_recips (&qp -> Raddress,
			    probe -> per__recipient__fields)) != OK ||
	    (retval = load_prf_ext(&qp, probe -> extensions,0,0)) != OK)
		return retval;
	return OK;
}


static int load_msgid (mid, p1msgid)
MPDUid *mid;
struct type_MTA_MTSIdentifier *p1msgid;
{
	int retval;

	if ((retval = load_gdi (&mid -> mpduid_DomId,
		      p1msgid -> global__domain__identifier)) != OK)
		return retval;

	if((mid -> mpduid_string =
	    qb2str (p1msgid -> local__identifier)) == NULL)
		return NOTOK;
	return OK;

}

static int decode_ext_orn (value, genp)
char **value;
struct type_MTA_ORName *genp;
{
	ORName *orn;
	char	buf[BUFSIZ];

	if ((orn = orname2orn (genp)) == NULL)
		return NOTOK;

	or_or2std (orn -> on_or, buf, 0);
	*value = strdup(buf);
	ORName_free (orn);
	return OK;
}	


static int load_addr (app, orname)
ADDR	**app;
struct type_MTA_ORName *orname;
{
	ORName *orn;
	char	buf[BUFSIZ];
	ADDR	*ap;

	if ((orn = orname2orn (orname)) == NULL)
		return NOTOK;

	or_or2std (orn -> on_or, buf, 0);
	*app = ap  = adr_new (buf, AD_X400_TYPE, 1);
	ap -> ad_subtype = AD_X400_88;

	if (orn -> on_dn)
		ap -> ad_dn = dn2str (orn -> on_dn);
	ORName_free (orn);
	return OK;
}

static int load_eits (eit, encinfo)
EncodedIT *eit;
struct type_MTA_EncodedInformationTypes *encinfo;
{
	struct type_MTA_ExternalEncodedInformationTypes *eeit;
	LIST_BPT	*bpt;
	char	*str;
	int retval;

	if ((retval = load_84eit (eit,
			encinfo -> built__in__encoded__information__types)) != OK)
		return retval;

	for (eeit = encinfo -> external__encoded__information__types;
	     eeit; eeit = eeit -> next) {
		str = strdup (sprintoid (eeit ->
					 ExternalEncodedInformationType));
		bpt = list_bpt_new (str);
		free (str);
		list_bpt_add (&eit->eit_types, bpt);
	}
	return OK;
}

static int load_84eit (eit, bieit)		/* XXX */
EncodedIT *eit;
struct type_MTA_BuiltInEncodedInformationTypes *bieit;
{
	char	*bs;
	int	n;

	if (eit == NULL) {
		eit = (EncodedIT *) smalloc (sizeof *eit);
		bzero ((char *)eit, sizeof *eit);
	}

	if ((bs = bitstr2strb (bieit, &n)) == NULLCP)
		return NOTOK;

	n = strb2int (bs, n);
	free (bs);
	enctypes2mem (n, undefined_bodyparts, &eit -> eit_types);
	return OK;
}


static int load_content (qp, ct)
Q_struct *qp;
struct type_MTA_ContentType *ct;
{
	extern char *cont_p2, *cont_p22, *hdr_p2_bp, *hdr_p22_bp;
	struct type_MTA_BuiltInContentType *bict;
	LIST_BPT	*new;

	if (ct -> offset == type_MTA_ContentType_external) {
		qp -> cont_type = strdup(sprintoid (ct -> un.external));
		return OK;
	}

	bict = ct -> un.built__in;

	switch (bict -> parm) {
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown content type (%d)"));
		/* fall */
	    case int_MTA_BuiltInContentType_unidentified:
		qp -> cont_type = strdup ("unidentified");
		break;
	    case int_MTA_BuiltInContentType_external:
		qp -> cont_type = strdup ("external");
		break;
	    case int_MTA_BuiltInContentType_interpersonal__messaging__1984:
		qp -> cont_type = strdup (cont_p2);
		if ((new = list_bpt_new (hdr_p2_bp)) == NULLIST_BPT)
			return NOTOK;
		list_bpt_add (&(qp -> encodedinfo.eit_types), new);
		break;
	    case int_MTA_BuiltInContentType_interpersonal__messaging__1988:
		qp -> cont_type = strdup (cont_p22);
		if ((new = list_bpt_new (hdr_p22_bp)) == NULLIST_BPT)
			return NOTOK;
		list_bpt_add (&(qp -> encodedinfo.eit_types), new);
		break;
	}
	return OK;
}

static int load_pmf (qp, pe)
Q_struct *qp;
PE	pe;
{
	if (bit_ison (pe,
		    bit_MTA_PerMessageIndicators_disclosure__of__recipients))
		qp -> disclose_recips = TRUE;
	else	qp -> disclose_recips = FALSE;

	if (bit_ison (pe,
		    bit_MTA_PerMessageIndicators_implicit__conversion__prohibited))
		qp -> implicit_conversion_prohibited = TRUE;
	else qp -> implicit_conversion_prohibited = FALSE;

	if (bit_ison (pe, bit_MTA_PerMessageIndicators_alternate__recipient__allowed))
		qp -> alternate_recip_allowed = TRUE;
	else qp -> alternate_recip_allowed = FALSE;

	if (bit_ison (pe, bit_MTA_PerMessageIndicators_content__return__request))
		qp -> content_return_request = TRUE;
	else qp -> content_return_request = FALSE;

	return OK;
}

static int load_time (utc, utcstr)
UTC	*utc;
struct type_UNIV_UTCTime *utcstr;
{
	char *str = qb2str (utcstr);
	if (str == NULLCP)
		return NOTOK;
	*utc = str2utct (str, strlen(str));
	if (*utc == NULLUTC)
		return NOTOK;
	free (str);
	*utc = utcdup (*utc);
	return *utc ? OK : NOTOK;
}

static int load_recips (app, prf)
ADDR	**app;
struct element_MTA_4 *prf;
{
	ADDR	*ap, *lastap = NULL;
	struct type_MTA_PerRecipientMessageTransferFields *prmtf;
	int	ad_no = 1;
	int retval;

	for (; prf; prf = prf -> next) {
		prmtf = prf -> PerRecipientMessageTransferFields;
		if ((retval = load_addr (&ap, prmtf -> recipient__name)) != OK)
			return retval;
		if (*app == NULL) 
			*app = lastap = ap;
		else	{
			lastap -> ad_next = ap;
			lastap = ap;
		}
		ap -> ad_no = ad_no ++;

		ap -> ad_extension =
			prmtf -> originally__specified__recipient__number ->
				parm;
		if (prmtf -> explicit__conversion)
			ap -> ad_explicitconversion =
				prmtf -> explicit__conversion -> parm;

		if (bit_ison (prmtf -> per__recipient__indicators,
			    bit_MTA_PerRecipientIndicators_responsibility))
			ap -> ad_resp = 1;
		else	ap -> ad_resp = 0;

		ap -> ad_mtarreq = AD_MTA_NONE;
		if (bit_ison (prmtf -> per__recipient__indicators,
			    bit_MTA_PerRecipientIndicators_originating__MTA__report))
			ap -> ad_mtarreq |= AD_MTA_CONFIRM;

		if (bit_ison (prmtf -> per__recipient__indicators,
			    bit_MTA_PerRecipientIndicators_originating__MTA__non__delivery__report))
			ap -> ad_mtarreq |= AD_MTA_BASIC;

		ap -> ad_usrreq = AD_USR_NOREPORT;
		if (bit_ison (prmtf -> per__recipient__indicators,
			    bit_MTA_PerRecipientIndicators_originator__non__delivery__report))
			ap -> ad_usrreq = AD_USR_BASIC;
			
		if (bit_ison (prmtf -> per__recipient__indicators,
			    bit_MTA_PerRecipientIndicators_originator__report))
			ap -> ad_usrreq = AD_USR_CONFIRM;
		
		
		if ((retval = load_prf_ext (ap, prmtf -> extensions,1,1)) != OK)
			return retval;
	}
	return OK;
}


static int load_p3_recips (app, prf)
ADDR	**app;
struct element_MTA_0 *prf;
{
	ADDR	*ap, *lastap = NULL;
	struct type_MTA_PerRecipientMessageSubmissionFields *prmtf;
	int	ad_no = 1;
	int retval;

	for (; prf; prf = prf -> next) {
		prmtf = prf -> PerRecipientMessageSubmissionFields;
		if ((retval = load_addr (&ap, prmtf -> recipient__name)) != OK)
			return retval;
		if (*app == NULL) 
			*app = lastap = ap;
		else	{
			lastap -> ad_next = ap;
			lastap = ap;
		}
		ap -> ad_no = ad_no ++;

		if (prmtf -> explicit__conversion)
			ap -> ad_explicitconversion =
				prmtf -> explicit__conversion -> parm;

		if (bit_ison (prmtf -> originator__report__request, bit_MTA_OriginatorReportRequest_non__delivery__report))
			ap -> ad_usrreq = AD_USR_BASIC;

		if (bit_ison (prmtf -> originator__report__request, bit_MTA_OriginatorReportRequest_report))
			ap -> ad_usrreq = AD_USR_CONFIRM;
		
		if ((retval = load_prf_ext (ap, prmtf -> extensions,0,1)) != OK)
			return retval;
	}
	return OK;
}


static int load_prf_ext (ap, ext,p1,msg)
ADDR	*ap;
struct type_MTA_Extensions *ext;
int p1,msg;
{
	struct type_MTA_Extensions *ep;
	int retval;

	for (ep = ext; ep; ep = ep -> next) {
		switch (ep -> ExtensionField -> type -> offset) {
		    case type_MTA_ExtensionType_local:
			if ((retval = do_local_extension
			     (&ap -> ad_per_recip_ext_list,
			      ep -> ExtensionField)) != OK)
				return retval;
			break;
		    case type_MTA_ExtensionType_global:
			if ((retval = do_global_ext_prf
			     (ep -> ExtensionField -> type ->
			      un.global, ap,
			      ep -> ExtensionField,
			      p1,msg)) != OK)
				return retval;
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown extension type %d",
				 ep -> ExtensionField -> type -> offset));
			break;
		}
	}
	return OK;
}


static int load_trace (tpp, trace)
Trace	**tpp;
struct type_MTA_TraceInformation *trace;
{
	Trace	*tp;
	int retval;

	for ( ; trace; trace = trace -> next) {
		if((retval = make_trace (trace -> TraceInformationElement,
					 &tp)) != OK)
			return retval;
		if (tp)
			trace_add (tpp, tp);
	}
	return OK;
}

static int make_trace (ti, tpp)
struct type_MTA_TraceInformationElement *ti;
Trace **tpp;
{
	Trace 	*tp;
	struct type_MTA_GlobalDomainIdentifier *gdi;
	struct type_MTA_DomainSuppliedInformation *dsi;
	DomSupInfo *dsp;
	int retval;

	*tpp = tp = (Trace *) smalloc (sizeof *tp);
	bzero ((char *)tp, sizeof *tp);

	if ((gdi = ti -> global__domain__identifier) != NULL &&
	    (retval = load_gdi (&tp -> trace_DomId, gdi)) != OK)
		return retval;

	dsp = &tp -> trace_DomSinfo;
	if ((dsi = ti -> domain__supplied__information) != NULL) {
		if (dsi -> arrival__time &&
		    (retval = load_time (&dsp -> dsi_time,
			       dsi -> arrival__time)) != OK)
			return retval;

		if (dsi -> routing__action)
			dsp -> dsi_action = dsi -> routing__action -> parm;

		if (dsi -> attempted__domain &&
		    (retval = load_gdi (&dsp -> dsi_attempted_md,
			      dsi -> attempted__domain)) != OK)
			return retval;

		if (dsi -> deferred__time &&
		    (retval = load_time (&dsp -> dsi_deferred,
			       dsi -> deferred__time)) != OK)
			return retval;

		if (dsi -> converted__encoded__information__types &&
		    (retval = load_eits (&dsp -> dsi_converted,
			       dsi -> converted__encoded__information__types))
		    != OK)
			return retval;

		if (dsi -> other__actions) {
			if (bit_ison (dsi -> other__actions,
				      bit_MTA_OtherActions_redirected))
				dsp -> dsi_other_actions |= ACTION_REDIRECTED;
			if (bit_ison (dsi -> other__actions,
				      bit_MTA_OtherActions_dl__operation))
				dsp -> dsi_other_actions |= ACTION_EXPANDED;
		}
	}
	return OK;
}

static int load_gdi (gdi, gdip1)
GlobalDomId *gdi;
struct type_MTA_GlobalDomainIdentifier *gdip1;
{
	struct type_MTA_AdministrationDomainName *admd;
	struct type_MTA_CountryName *co;
	struct type_MTA_PrivateDomainIdentifier *prmd;

	if ((co = gdip1 -> country__name) != NULL &&
	    (gdi -> global_Country =
	     qb2str (co -> offset ==
		     type_MTA_CountryName_x121__dcc__code ?
		     co -> un.x121__dcc__code :
		     co -> un.iso__3166__alpha2__code)) == NULL)
		return NOTOK;
	if ((admd = gdip1 -> administration__domain__name) != NULL &&
	    (gdi -> global_Admin =
	     qb2str (admd -> offset ==
		     type_MTA_AdministrationDomainName_numeric ?
		     admd -> un.numeric : admd -> un.printable)) == NULL)
		return NOTOK;
	if ((prmd = gdip1 -> private__domain__identifier) != NULL &&
	    (gdi -> global_Private =
	     qb2str (prmd -> offset ==
		     type_MTA_PrivateDomainIdentifier_numeric ?
		     prmd -> un.numeric :
		     prmd -> un.printable)) == NULL)
		return NOTOK;
	return OK;
}

static int load_reportenv (qp, dr, envelope)
Q_struct *qp;
DRmpdu *dr;
struct type_MTA_ReportTransferEnvelope *envelope;
{
	int retval;

	dr -> dr_mpduid = (MPDUid *)
		smalloc (sizeof *dr -> dr_mpduid);
	bzero ((char *)dr -> dr_mpduid, sizeof *dr -> dr_mpduid);

	if ((retval = load_msgid (dr -> dr_mpduid,
			envelope -> report__identifier)) != OK)
		return retval;

	if ((retval = load_addr (&qp -> Oaddress,
				 envelope -> report__destination__name)) != OK ||
	    (retval = load_trace (&dr -> dr_trace,
				  envelope -> trace__information)) != OK ||
	    (retval = load_repe_ext (dr, envelope -> extensions)) != OK)
		return retval;
	return OK;
}

static int load_reportcont (qp, dr, content)
Q_struct *qp;
DRmpdu *dr;
struct type_MTA_ReportTransferContent *content;
{
	int retval;

	if ((retval = load_msgid (&qp -> msgid, content -> subject__identifier)) != OK)
		return retval;

	if (content -> subject__intermediate__trace__information &&
	    (retval = load_trace (&dr -> dr_subject_intermediate_trace,
			content -> subject__intermediate__trace__information))
	    != OK)
		return retval;

	if (content -> original__encoded__information__types &&
	    (retval = load_eits (&qp -> orig_encodedinfo,
		       content -> original__encoded__information__types))
	    != OK)
		return retval;

	if (content -> member_MTA_17 &&
	    (retval = load_content (qp, content -> member_MTA_17)) != OK)
		return retval;

	if (content -> content__identifier &&
	    (qp -> ua_id = qb2str (content -> content__identifier)) == NULL)
		return NOTOK;

	/* content -> returned__contents handled directly. */

	/* content -> additional__information XXX */

	if (content -> extensions &&
	    (retval = load_cont_ext (dr, content -> extensions)) != OK)
		return retval;

	if ((retval = load_drc_pr_fields (&qp -> Raddress, &dr -> dr_recip_list,
				content -> per__recipient__fields, 1)) != OK)
		return retval;
	return OK;
}

static int load_drc_pr_fields (app, rrp, prfl, ad_no)
ADDR	**app;
Rrinfo **rrp;
struct element_MTA_9 *prfl;
int	ad_no;
{
	Rrinfo *rr;
	struct type_MTA_PerRecipientReportTransferFields *prf;
	int retval;

	if ((prf = prfl -> PerRecipientReportTransferFields) == NULL)
		return OK;

	*rrp = rr = (Rrinfo *) smalloc (sizeof *rr);
	bzero ((char *)rr, sizeof (*rr));

	if ((retval = load_addr (app, prf -> actual__recipient__name)) != OK)
		return retval;

	rr -> rr_recip = (*app) -> ad_no = ad_no ++;
	(*app) -> ad_status = AD_STAT_DRWRITTEN;

	(*app) -> ad_extension =
		prf -> originally__specified__recipient__number -> parm;

	if (bit_ison (prf -> per__recipient__indicators,
		      bit_MTA_PerRecipientIndicators_responsibility))
		(*app) -> ad_resp = 1;
	else	(*app) -> ad_resp = 0;

	(*app) -> ad_mtarreq = AD_MTA_NONE;
	if (bit_ison (prf -> per__recipient__indicators,
		      bit_MTA_PerRecipientIndicators_originating__MTA__report))
		(*app) -> ad_mtarreq |= AD_MTA_CONFIRM;

	if (bit_ison (prf -> per__recipient__indicators,
		      bit_MTA_PerRecipientIndicators_originating__MTA__non__delivery__report))
		(*app) -> ad_mtarreq |= AD_MTA_BASIC;

	(*app) -> ad_usrreq = AD_USR_NOREPORT;
	if (bit_ison (prf -> per__recipient__indicators,
		      bit_MTA_PerRecipientIndicators_originator__non__delivery__report))
		(*app) -> ad_usrreq = AD_USR_BASIC;
			
	if (bit_ison (prf -> per__recipient__indicators,
		      bit_MTA_PerRecipientIndicators_originator__report))
		(*app) -> ad_usrreq = AD_USR_CONFIRM;


	if ((retval = load_lasttrace (rr, prf -> last__trace__information)) != OK)
		return retval;

	if (prf -> originally__intended__recipient__name &&
	    (retval = load_fullname (&rr -> rr_originally_intended_recip,
			   prf -> originally__intended__recipient__name))
	    != OK)
		return retval;

	if (prf -> supplementary__information &&
	    (rr -> rr_supplementary =
	     qb2str (prf -> supplementary__information)) == NULL)
		return NOTOK;

	if (prf -> extensions &&
	    (retval = load_prr_ext (rr, prf -> extensions)) != OK)
		return retval;

	if (prfl -> next)
		return load_drc_pr_fields (&(*app) -> ad_next,
					   &rr -> rr_next,
					   prfl -> next, ad_no);
	return OK;
}
	
static int load_lasttrace(rr, lti)
Rrinfo *rr;
struct type_MTA_LastTraceInformation *lti;
{
	int retval;

	if ((retval = load_time (&rr -> rr_arrival,
				 lti -> arrival__time)) != OK)
		return retval;

	if (lti -> converted__encoded__information__types) {
		rr -> rr_converted = (EncodedIT *)
			calloc (1, sizeof *rr -> rr_converted);
		if ((retval = load_eits (rr -> rr_converted,
			       lti -> converted__encoded__information__types))
		    != OK)
			return retval;
	}

	return load_report (&rr -> rr_report, lti -> report);
}

static int load_report (rp, report)
Report *rp;
struct type_MTA_Report *report;
{
	int retval;

	if (report -> offset == type_MTA_Report_delivery) {
		rp -> rep_type = DR_REP_SUCCESS;

		if ((retval = load_time (&rp -> rep.rep_dinfo.del_time,
			       report -> un.delivery ->
			       message__delivery__time)) != OK)
			return retval;

		rp -> rep.rep_dinfo.del_type =
			report -> un.delivery -> type__of__MTS__user -> parm;
	}
	else {
		rp -> rep_type = DR_REP_FAILURE;

		rp -> rep.rep_ndinfo.nd_rcode =
			report -> un.non__delivery ->
				non__delivery__reason__code -> parm;
		if (report -> un.non__delivery -> 
			non__delivery__diagnostic__code)
			rp -> rep.rep_ndinfo.nd_dcode =
				report -> un.non__delivery ->
					non__delivery__diagnostic__code ->
						parm;
		else
			rp -> rep.rep_ndinfo.nd_dcode = -1;
	}
	return OK;
}

			   



static int load_repe_ext (dr, ext)
DRmpdu *dr;
struct type_MTA_Extensions *ext;
{
	struct type_MTA_Extensions *ep;
	int retval;

	for (ep = ext; ep; ep = ep -> next) {
		switch (ep -> ExtensionField -> type -> offset) {
		    case type_MTA_ExtensionType_local:
			if ((retval = do_local_extension (&dr -> dr_per_envelope_extensions,
						ep -> ExtensionField)) != OK)
				return retval;
			break;
		    case type_MTA_ExtensionType_global:
			if ((retval = do_global_repe_prf (ep -> ExtensionField -> type ->
						un.global, dr,
						ep -> ExtensionField)) != OK)
				return retval;
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown extension type %d",
				 ep -> ExtensionField -> type -> offset));
			break;
		}
	}
	return OK;
	
}

static int load_cont_ext (dr, ext)
DRmpdu *dr;
struct type_MTA_Extensions *ext;
{
	struct type_MTA_Extensions *ep;
	int retval;

	for (ep = ext; ep; ep = ep -> next) {
		switch (ep -> ExtensionField -> type -> offset) {
		    case type_MTA_ExtensionType_local:
			if ((retval = do_local_extension (&dr -> dr_per_report_extensions,
						ep -> ExtensionField)) != OK)
				return retval;
			break;
		    case type_MTA_ExtensionType_global:
			if ((retval = do_global_cont_prf (ep -> ExtensionField -> type ->
						un.global, dr,
						ep -> ExtensionField)) != OK)
				return retval;
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown extension type %d",
				 ep -> ExtensionField -> type -> offset));
			break;
		}
	}
	return OK;
}

static int load_prr_ext (rr, ext)	
Rrinfo *rr;
struct type_MTA_Extensions *ext;
{
	struct type_MTA_Extensions *ep;
	int retval;

	for (ep = ext; ep; ep = ep -> next) {
		switch (ep -> ExtensionField -> type -> offset) {
		    case type_MTA_ExtensionType_local:
			if ((retval = do_local_extension (&rr -> rr_per_recip_extensions,
						ep -> ExtensionField)) != OK)
				return retval;
			break;

		    case type_MTA_ExtensionType_global:
			if ((retval = do_global_rr_prf (ep -> ExtensionField -> type ->
					      un.global, rr,
					      ep -> ExtensionField)) != OK)
				return retval;
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown extension type %d",
				 ep -> ExtensionField -> type -> offset));
			break;
		}
	}
	return OK;
}

static int load_fullname (fn, orname)
FullName **fn;
struct type_MTA_ORName *orname;
{
	ADDR	*ad;
	int retval;

	if ((retval = load_addr (&ad, orname)) != OK)
		return retval;

	*fn = fn_new (ad -> ad_value, ad -> ad_dn);
	adr_free (ad);
	return OK;
}

/*  EXTENSIONS */
static int	pe2crit ();

static int decode_extension (value, crit, defcrit, magic_num, mod,
			     label, decoder, ext)
caddr_t	value;
char	*crit;
int	defcrit;
int	magic_num;
modtyp	*mod;
IFP	decoder;
char	*label;
struct type_MTA_ExtensionField *ext;
{
	caddr_t *genp;
	int retval;

#if PP_DEBUG > 0
	if (pp_log_norm -> ll_events & LLOG_PDUS)
		pvpdu (pp_log_norm, magic_num, mod, 
			ext -> value, label, PDU_READ);
#endif
	if (dec_f(magic_num, mod, ext -> value, 1,
		  NULLIP, NULLVP, &genp) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't parse %s value [%s]", label, PY_pepy));
		return DONE;
	}

	if ((retval = (*decoder) (value, genp)) != OK)
		return retval;

	*crit = pe2crit (ext -> criticality, defcrit);

	fre_obj((char *) genp, mod->md_dtab[magic_num], mod, 1);
	return OK;
}

static int load_pm_extensions (qp, ext,p1,msg)
Q_struct *qp;
struct type_MTA_Extensions *ext;
int p1,msg;
{
	struct type_MTA_Extensions *ep;
	int retval;

	for (ep = ext; ep; ep = ep -> next) {
		switch (ep -> ExtensionField -> type -> offset) {
		    case type_MTA_ExtensionType_local:
			if ((retval = do_local_extension (&qp -> per_message_extensions,
						ep -> ExtensionField)) != OK)
				return retval;
			break;

		    case type_MTA_ExtensionType_global:
			if ((retval = do_global_extension (ep -> ExtensionField ->
						 type -> un.global,
						 qp, ep -> ExtensionField,
						 p1,msg)) != OK)
				return retval;
			break;

		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown extension type %d",
				 ep -> ExtensionField -> type -> offset));
			break;
		}
	}
	return OK;
}


static int squash_ext (qbp, value)
struct qbuf **qbp;
PE	value;
{
	char	*cp;
	int	len;
	PS	ps;

	if ((ps = ps_alloc (str_open)) == NULLPS) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't allocate PS"));
		return NOTOK;
	}

	len = ps_get_abs (value);
	cp = smalloc (len);

	if (str_setup (ps, cp, len, 1) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't setup stream [%s]",
					  ps_error (ps -> ps_errno)));
		return NOTOK;
	}

	if (pe2ps (ps, value) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("pe2ps failed [%s]", ps_error (ps -> ps_errno)));
		return NOTOK;
	}
	
	ps_free (ps);

	if ((*qbp = str2qb (cp, len, 1)) == NULL)
		return NOTOK;
	free (cp);
	return OK;
}

static int flatten_ext (ext, epp)
struct type_MTA_ExtensionField *ext;
X400_Extension **epp;
{
	X400_Extension	*ep;

	*epp = ep = (X400_Extension *)smalloc (sizeof *ep);
	if (ext -> type -> offset == type_MTA_ExtensionType_global) {
		ep -> ext_int = ext -> type -> un.global;
		ep -> ext_oid = NULLOID;
	}
	else {
		ep -> ext_int = EXT_OID_FORM;
		if ((ep -> ext_oid = oid_cpy (ext -> type -> un.local))
		    == NULLOID)
			return NOTOK;
	}
		
	if (squash_ext (&ep -> ext_value, ext -> value) == NOTOK)
		return NOTOK;
	ep -> ext_criticality = pe2crit (ext -> criticality,
					 CRITICAL_NONE);
	ep -> ext_next = NULL;
	return OK;
}

static int do_local_extension (elist, ext)
X400_Extension **elist;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;
	int retval;

	PP_LOG (LLOG_EXCEPTIONS, ("Unknown local extension %s",
				  sprintoid (ext -> type -> un.local)));

	if ((retval = flatten_ext (ext, &ep)) != OK)
		return retval;
	for (epp = elist; *epp;
	     epp = &(*epp) -> ext_next)
		continue;
	*epp = ep;
	return OK;
}

static int ext_security (qbp, critp, defcrit, ext)
struct qbuf **qbp;
char	*critp;
int	defcrit;
struct type_MTA_ExtensionField *ext;
{
	int retval;

	if ((retval = squash_ext (qbp, ext -> value)) != OK)
		return retval;

	*critp = pe2crit (ext -> criticality, defcrit);
	return OK;
}

static int decode_ext_time (utc, genp)
UTC	*utc;
struct type_UNIV_UTCTime *genp;
{
	return load_time (utc, genp);
}

static int decode_ext_char (cp, ptr)
char *cp;
struct type_Ext_DLExpansionProhibited *ptr; /* any integer type will do */
{
	*cp = ptr -> parm;
	return OK;
}

static int decode_ext_int (ip, ptr)
int *ip;
struct type_Ext_DLExpansionProhibited *ptr; /* any integer type will do */
{
	*ip = ptr -> parm;
	return OK;
}

static int ext_decode_dlexph (dpp, genp)
DLHistory **dpp;
struct type_Ext_DLExpansionHistory *genp;
{
	struct type_Ext_DLExpansionHistory *dlhp;
	DLHistory *dp;
	int retval;

	for (dlhp = genp; dlhp; dlhp = dlhp -> next) {
		if ((retval = ext_decode_dlh (dlhp -> DLExpansion, &dp)) != OK)
			return retval;
		dlh_add (dpp, dp);
	}
	return OK;
}

static int ext_decode_dlh (dlp, dpp)
struct type_Ext_DLExpansion *dlp;
DLHistory **dpp;
{
	UTC	ut;
	ADDR	*ad;
	int retval;

	if ((retval = load_time (&ut, dlp -> dl__expansion__time)) != OK)
		return retval;
	if ((retval = load_addr (&ad, dlp -> address)) != OK)
		return retval;

	*dpp = dlh_new (ad -> ad_value, ad -> ad_dn , ut);
	adr_free (ad);

	free ((char *)ut);
	return OK;
}


static int do_global_extension (n, qp, ext, p1, msg)
int	n;
Q_struct *qp;
struct type_MTA_ExtensionField *ext;
int p1,msg;
{
	X400_Extension	*ep, **epp;
	int retval;

	switch (n) {
	    case EXT_RECIPIENT_REASSIGNMENT_PROHIBITED:
		return decode_extension
			((caddr_t)&qp -> recip_reassign_prohibited,
			 &qp -> recip_reassign_crit,
			 EXT_RECIPIENT_REASSIGNMENT_PROHIBITED_DC,
			 _ZRecipientReassignmentProhibitedExt,
			 &_ZExt_mod,
			 "Extension.RecipientReassignmentProhibited",
			 decode_ext_char,
			 ext);

	    case EXT_DL_EXPANSION_PROHIBITED:
		return decode_extension
			((caddr_t)&qp -> dl_expansion_prohibited,
			 &qp -> dl_expansion_crit,
			 EXT_DL_EXPANSION_PROHIBITED_DC,
			 _ZDLExpansionProhibitedExt,
			 &_ZExt_mod,
			 "Extension.DLExpansionProhibited",
			 decode_ext_char,
			 ext);

	    case EXT_CONVERSION_WITH_LOSS_PROHIBITED:
		return decode_extension
			((caddr_t)&qp -> conversion_with_loss_prohibited,
			 &qp -> conversion_with_loss_crit,
			 EXT_CONVERSION_WITH_LOSS_PROHIBITED_DC,
			 _ZConversionWithLossProhibitedExt,
			 &_ZExt_mod,
			 "Extensions.ConversionWithLossProhibited",
			 decode_ext_char,
			 ext);

	    case EXT_ORIGINATOR_CERTIFICATE:
		return ext_security (&qp -> originator_certificate,
				     &qp -> originator_certificate_crit,
				     EXT_ORIGINATOR_CERTIFICATE_DC,
				     ext);

	    case EXT_MESSAGE_SECURITY_LABEL:
		return ext_security (&qp -> security_label,
				     &qp -> security_label_crit,
				     EXT_MESSAGE_SECURITY_LABEL_DC,
				     ext);

	    case EXT_CONTENT_CORRELATOR:
		return ext_security (&qp -> general_content_correlator,
				     &qp -> content_correlator_crit,
				     EXT_CONTENT_CORRELATOR_DC,
				     ext);


	    case EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK:
		if (!msg)
			return ext_security
				(&qp -> message_origin_auth_check,
				 &qp -> message_origin_auth_check_crit,
				 EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK_DC,
				 ext);
		break;

	    case EXT_DL_EXPANSION_HISTORY:
		if (p1)
			return decode_extension
				((caddr_t)&qp -> dl_expansion_history,
				 &qp -> dl_expansion_crit,
				 EXT_DL_EXPANSION_HISTORY_DC,
				 _ZDLExpansionHistoryExt,
				 &_ZExt_mod,
				 "Extensions.DLExpansionHistory",
				 ext_decode_dlexph,
				 ext);
		break;

	    case EXT_INTERNAL_TRACE_INFORMATION:
		if (p1)
			return ext_inttrace (&qp -> trace, ext);
		break;

	    case EXT_LATEST_DELIVERY_TIME:
		if (msg)
			return decode_extension
				((caddr_t)&qp -> latest_time,
				 &qp -> latest_time_crit,
				 EXT_LATEST_DELIVERY_TIME_DC,
				 _ZLatestDeliveryTimeExt,
				 &_ZExt_mod,
				 "Extensions.LatestDeliveryTime",
				 decode_ext_time,
				 ext);
		break;

	    case EXT_ORIGINATOR_RETURN_ADDRESS:
		if (msg)
			return ext_ora (qp, ext);
		break;

	    case EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER:
		if (msg)
			return ext_security
				(&qp -> algorithm_identifier,
				 &qp -> algorithm_identifier_crit,
				 EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER_DC,
				 ext);
		break;
	    case EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK:
		if (msg)
			return ext_security
				(&qp -> message_origin_auth_check,
				 &qp -> message_origin_auth_check_crit,
				 EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK_DC,
				 ext);
		break;
	    case EXT_FORWARDING_REQUEST:
		if (msg && (!p1))
			return decode_extension
				((caddr_t)&qp -> forwarding_request,
				 &qp -> forwarding_request_crit,
				 EXT_FORWARDING_REQUEST_DC,
				 _ZSequenceNumberExt,
				 &_ZExt_mod,
				 "Extension.ForwardingRequest.SequenceNumber",
				 decode_ext_int,
				 ext);
		break;
	    case EXT_PROOF_OF_SUBMISSION_REQUEST:
		if (msg && (!p1))
			return decode_extension
				((caddr_t)&qp -> proof_of_submission_request,
				 &qp -> proof_of_submission_crit,
				 EXT_PROOF_OF_SUBMISSION_REQUEST_DC,
				 _ZProofOfSubmissionRequestExt,
				 &_ZExt_mod,
				 "Extension.ProofOfSubmissionRequest",
				 decode_ext_int,
				 ext);
		break;
	    default:
		PP_NOTICE (("Unknown global extension type %d", n));
		/* fall */
		if ((retval = flatten_ext (ext, &ep)) != OK)
			return retval;
		for (epp = &qp -> per_message_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}
	return OK;
}


static int ext_ora (qp, ext)
Q_struct *qp;
struct type_MTA_ExtensionField *ext;
{
	struct type_Ext_OriginatorReturnAddress *ora;
	OR_ptr or;
	char	buf[BUFSIZ];

	PP_PDU (print_Ext_OriginatorReturnAddress,
		ext -> value, "Extensions.OriginatorReturnAddress",
		PDU_READ);

	if (decode_Ext_OriginatorReturnAddress (ext -> value, 1,
						   NULLIP, NULLVP,
						   &ora) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't parse originator return address [%s]",
			 PY_pepy));
		return DONE;
	}

	or = oradr2ora (ora);
	or_or2std (or, buf, 0);
	or_free (or);

	qp -> originator_return_address = fn_new (buf, NULLCP);

	qp -> originator_return_address_crit =
		pe2crit (ext -> criticality,
			 EXT_ORIGINATOR_RETURN_ADDRESS_DC);
	free_Ext_OriginatorReturnAddress (ora);
	return OK;
}

static int same_gdi (gdi1, gdi2)
GlobalDomId *gdi1, *gdi2;
{
	int vset;

	vset = (gdi1 -> global_Country ? 1 : 0) +
		(gdi2 -> global_Country ? 1 : 0);
	if (vset == 1 || ( vset == 2 &&
			  lexequ (gdi1 -> global_Country,
				  gdi2 -> global_Country) != 0))
		return 0;
	vset = (gdi1 -> global_Admin ? 1 : 0) +
		(gdi2 -> global_Admin ? 1 : 0);

	if (vset == 1 || (vset == 2 &&
			  lexequ (gdi1 -> global_Admin,
				  gdi2 -> global_Admin) != 0))
		return 0;

	vset = (gdi1 -> global_Private ? 1 : 0) +
		(gdi2 -> global_Private ? 1 : 0);
	if (vset == 1 || (vset == 2 &&
			  lexequ (gdi1 -> global_Private,
				  gdi2 -> global_Private) != 0))
		return 0;
	return 1;
}

static int same_dsi (dsi1, dsi2)
DomSupInfo *dsi1, *dsi2;
{
	int vset;

	vset = (dsi1 -> dsi_time ? 1 : 0) +
		(dsi2 -> dsi_time ? 1 : 0);
	if (vset == 1 || (vset == 2 &&
			  utcequ (dsi1->dsi_time, dsi2 -> dsi_time) != 0))
		return 0;

	vset = (dsi1 -> dsi_deferred ? 1 : 0) +
		(dsi2 -> dsi_deferred ? 1 : 0);
	if (vset == 1 || (vset == 2 &&
			  utcequ (dsi1 -> dsi_deferred,
				  dsi2 -> dsi_deferred) != 0)) 
		return 0;

	if (dsi1 -> dsi_action != dsi2 -> dsi_action)
		return 0;
	if (dsi1 -> dsi_other_actions != dsi2 -> dsi_other_actions)
		return 0;

	if (same_gdi (&dsi1 -> dsi_attempted_md,
		      &dsi2 -> dsi_attempted_md) == 0)
		return 0;

	vset = (dsi1 -> dsi_converted.eit_types ? 1 : 0) +
		(dsi2 -> dsi_converted.eit_types ? 1 : 0);
	if (vset == 1)		/* should check all eit's too. */
		return 0;
	
	vset = (dsi1 -> dsi_attempted_mta ? 1 : 0) +
		(dsi2 -> dsi_attempted_mta ? 1 : 0);
	if (vset == 1 || (vset == 2 &&
			  lexequ (dsi1 -> dsi_attempted_mta,
				  dsi2 -> dsi_attempted_mta) != 0))
		return 0;
	return 1;
}


static void merge_trace (base, new_tr)
Trace **base, *new_tr;
{
	Trace **tp;
	Trace *tmp;

	for (tp = base; *tp; tp = &(*tp) -> trace_next) {
		if (same_gdi (&((*tp) -> trace_DomId),
			      &(new_tr -> trace_DomId)) == 0 ||
		    same_dsi (&((*tp) -> trace_DomSinfo),
			      &(new_tr -> trace_DomSinfo)) == 0)
			continue;
		tmp = *tp;
		new_tr -> trace_next = (*tp) -> trace_next;
		*tp = new_tr;
		tmp -> trace_next = NULL;
		trace_free (tmp);
		return;
	}
	*tp = new_tr;
	new_tr -> trace_next = NULL;
}

static int ext_inttrace (trace, ext)
Trace	**trace;
struct type_MTA_ExtensionField *ext;
{
	struct type_Ext_InternalTraceInformation *intt, *ip;
	int retval;

	PP_PDU (print_Ext_InternalTraceInformation,
		ext -> value, "Extension.InternalTraceInformation",
		PDU_READ);

	if (decode_Ext_InternalTraceInformation (ext -> value, 1,
							NULLIP, NULLVP,
							&intt) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't parse InternalTraceInformation [%s]",
			 PY_pepy));
		return DONE;
	}

	for (ip = intt; ip; ip = ip -> next) {
		struct type_Ext_InternalTraceInformationElement *te;
		struct type_Ext_MTASuppliedInformation *msi;
		Trace *tp;

		te = ip -> InternalTraceInformationElement;
		tp = trace_new ();
		if ((retval = load_gdi (&tp -> trace_DomId,
			      te -> global__domain__identifier)) != OK)
			return retval;
		if ((tp -> trace_mta = qb2str (te -> mta__name)) == NULL)
			return NOTOK;
		msi = te -> mta__supplied__information;
		if (msi -> arrival__time &&
		    (retval = load_time (&tp -> trace_DomSinfo.dsi_time,
			       msi -> arrival__time)) != OK)
			return retval;
		if (msi -> routing__action)
			tp -> trace_DomSinfo.dsi_action =
				msi -> routing__action -> parm;
		if (msi -> deferred__time &&
		    (retval = load_time (&tp -> trace_DomSinfo.dsi_deferred,
			       msi -> deferred__time)) != OK)
			return retval;
		if (msi -> other__actions) {
			if (bit_ison (msi -> other__actions,
				      bit_MTA_OtherActions_redirected))
				tp -> trace_DomSinfo.dsi_other_actions |=
					ACTION_REDIRECTED;
			if (bit_ison (msi -> other__actions,
				      bit_MTA_OtherActions_dl__operation))
				tp -> trace_DomSinfo.dsi_other_actions |=
					ACTION_EXPANDED;
		}
		if (msi -> attempted) {
			switch (msi -> attempted -> offset) {
			    case choice_Ext_1_mta:
				if ((tp -> trace_DomSinfo.dsi_attempted_mta =
				     qb2str (msi -> attempted -> un.mta)) == NULL)
					return NOTOK;
				break;
			    case choice_Ext_1_domain:
				if ((retval = load_gdi (&tp ->
					      trace_DomSinfo.dsi_attempted_md,
					      msi -> attempted -> un.domain))
				    != OK)
					return retval;
				break;
				

			    default:
				break;
			}
		}
		merge_trace (trace, tp);
	}
	return OK;
}

static int decode_ext_pdm (value, genp)
int	*value;
PE	genp;
{
#define setbit(t,v) \
	(*value) |= (bit_ison(genp, (t)) ? (v) : 0)

	setbit (bit_Ext_PhysicalDeliveryModes_ordinary__mail,
		AD_PM_ORD);
	setbit (bit_Ext_PhysicalDeliveryModes_special__delivery,
		AD_PM_SPEC);
	setbit (bit_Ext_PhysicalDeliveryModes_express__mail,
		AD_PM_EXPR);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection,
		AD_PM_CNT);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection__with__telephone__advice,
		AD_PM_CNT_PHONE);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection__with__telex__advice,
		AD_PM_CNT_TLX);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection__with__teletex__advice,
		AD_PM_CNT_TTX);
	setbit (bit_Ext_PhysicalDeliveryModes_bureau__fax__delivery,
		AD_PM_CNT_BUREAU);
#undef setbit
	return OK;
}

static int decode_ext_pra (value, gen)
OID	*value;
OID	gen;
{
	if ((*value = oid_cpy (gen)) != NULLOID)
		return OK;
	return NOTOK;
}

static int decode_ext_rnfa (value, genp)
char	**value;
struct qbuf *genp;
{
	if ((*value = qb2str (genp)) == NULL)
		return NOTOK;
	return OK;
}

static int decode_ext_rdm (value, genp)
int	value[AD_RDM_MAX];
struct type_Ext_RequestedDeliveryMethod *genp;
{
	int i;
	
	for (i = 0; i < AD_RDM_MAX && genp;
	     i++, genp = genp -> next)
		value[i] = genp -> element_Ext_0;
	return OK;
}

static int decode_ext_redir (value, genp)
Redirection **value;
struct type_Ext_RedirectionHistory *genp;
{
	int retval;

	while (genp) {
		if ((retval = decode_1redir (value, genp->Redirection)) != OK)
			return retval;
		value = &(*value) -> rd_next;
		genp = genp -> next;
	}
	return OK;
}

static int decode_1redir (rpp, redir)
Redirection **rpp;
struct type_Ext_Redirection *redir;
{
	ADDR *ad;
	int retval;

	*rpp = (Redirection *) smalloc (sizeof **rpp);
	if ((retval =
	     load_addr (&ad, redir -> intended__recipient__name -> address))
	    != OK)
		return retval;
	(*rpp) -> rd_addr = ad -> ad_value;
	ad -> ad_value = NULLCP;
	(*rpp) -> rd_dn = ad -> ad_dn;
	ad -> ad_dn = NULLCP;
	adr_free (ad);

	if ((retval = load_time (&(*rpp) -> rd_time,
				 redir -> intended__recipient__name -> redirection__time)) != OK)
		return retval;
	(*rpp) -> rd_reason = redir -> redirection__reason -> parm;
	return OK;
}

static int do_global_ext_prf (n, ad, ext, p1, msg)
int	n;
ADDR	*ad;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;
	int retval;

	switch (n) {
	    case EXT_MESSAGE_TOKEN:
		if (msg)
			return ext_security (&ad -> ad_message_token,
					     &ad -> ad_message_token_crit,
					     EXT_MESSAGE_TOKEN_DC,
					     ext);
		break;
	    case EXT_CONTENT_INTEGRITY_CHECK:
		if (msg)
			return ext_security (&ad -> ad_content_integrity,
					     &ad -> ad_content_integrity_crit,
					     EXT_CONTENT_INTEGRITY_CHECK_DC,
					     ext);
		break;
	    case EXT_PROOF_OF_DELIVERY_REQUEST:
		if (msg)
			return decode_extension
				((caddr_t)&ad -> ad_proof_delivery,
				 &ad -> ad_proof_delivery_crit,
				 EXT_PROOF_OF_DELIVERY_REQUEST_DC,
				 _ZProofOfDeliveryRequestExt,
				 &_ZExt_mod,
				 "Extensions.ProofOfDeliveryRequest",
				 decode_ext_int,
				 ext);
		break;
	    case EXT_PHYSICAL_FORWARDING_PROHIBITED:
		if (msg)
			return decode_extension
				((caddr_t)&ad -> ad_phys_forward,
				 &ad -> ad_phys_forward_crit,
				 EXT_PHYSICAL_FORWARDING_PROHIBITED_DC,
				 _ZPhysicalForwardingProhibitedExt,
				 &_ZExt_mod,
				 "Extensions.PhysicalForwardingProhibited",
				 decode_ext_char,
				 ext);
		break;
	    case EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST:
		if (msg)
			return decode_extension
				((caddr_t)&ad -> ad_phys_fw_ad_req,
				 &ad -> ad_phys_fw_ad_crit,
				 EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST_DC,
				 _ZPhysicalForwardingAddressRequestExt,
				 &_ZExt_mod,
				 "Extensions.PhysicalForwardingAddressRequest",
				 decode_ext_char,
				 ext);
		break;
	    case EXT_REGISTERED_MAIL:
		if (msg)
			return decode_extension
				((caddr_t)&ad -> ad_reg_mail_type,
				 &ad -> ad_reg_mail_type_crit,
				 EXT_REGISTERED_MAIL_DC,
				 _ZRegisteredMailTypeExt,
				 &_ZExt_mod,
				 "Extensions.RegisteredMailType",
				 decode_ext_int,
				 ext);
		break;
	    case EXT_PHYSICAL_DELIVERY_REPORT_REQUEST:
		if (msg)
			return decode_extension
				((caddr_t)&ad -> ad_pd_report_request,
				 &ad -> ad_pd_report_request_crit,
				 EXT_PHYSICAL_DELIVERY_REPORT_REQUEST_DC,
				 _ZPhysicalDeliveryReportRequestExt,
				 &_ZExt_mod,
				 "Extensions.PhysicalDeliveryReportRequest",
				 decode_ext_int,
				 ext);
		break;
	    case EXT_PHYSICAL_RENDITION_ATTRIBUTES:
		return decode_extension
			((caddr_t)&ad -> ad_phys_rendition_attribs,
			 &ad -> ad_phys_rendition_attribs_crit,
			 EXT_PHYSICAL_RENDITION_ATTRIBUTES_DC,
			 _ZPhysicalRenditionAttributesExt,
			 &_ZExt_mod,
			 "Extensions.PhysicalRenditionAttributes",
			 decode_ext_pra,
			 ext);

	    case EXT_RECIPIENT_NUMBER_FOR_ADVICE:
		if (msg)
			return decode_extension
				((caddr_t)&ad -> ad_recip_number_for_advice,
				 &ad -> ad_recip_number_for_advice_crit,
				 EXT_RECIPIENT_NUMBER_FOR_ADVICE_DC,
				 _ZRecipientNumberForAdviceExt,
				 &_ZExt_mod,
				 "Extensions.RecipientNumberForAdvice",
				 decode_ext_rnfa,
				 ext);
		break;
	    case EXT_REQUESTED_DELIVERY_METHOD:
		return decode_extension
			((caddr_t)ad -> ad_req_del,
			 &ad -> ad_req_del_crit,
			 EXT_REQUESTED_DELIVERY_METHOD_DC,
			 _ZRequestedDeliveryMethodExt,
			 &_ZExt_mod,
			 "Extensions.RequestedDeliveryMethod",
			 decode_ext_rdm,
			 ext);

	    case EXT_PHYSICAL_DELIVERY_MODES:
		if (msg)
			return decode_extension
				((caddr_t)&ad -> ad_phys_modes,
				 &ad -> ad_phys_modes_crit,
				 EXT_PHYSICAL_DELIVERY_MODES_DC,
				 _ZPhysicalDeliveryModesExt,
				 &_ZExt_mod,
				 "Extensions.PhysicalDeliveryModes",
				 decode_ext_pdm,
				 ext);
		break;

	    case EXT_REDIRECTION_HISTORY:
		if (p1)
			return decode_extension
				((caddr_t)&ad -> ad_redirection_history,
				 &ad -> ad_redirection_history_crit,
				 EXT_REDIRECTION_HISTORY_DC,
				 _ZRedirectionHistoryExt,
				 &_ZExt_mod,
				 "Extensions.RedirectionHistory",
				 decode_ext_redir,
				 ext);
		break;

	    case EXT_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT:
		return decode_extension
			((caddr_t)&ad -> ad_orig_req_alt,
			 &ad -> ad_orig_req_alt_crit,
			 EXT_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT_DC,
			 _ZOriginatorRequestedAlternateRecipientExt,
			 &_ZExt_mod,
			 "Extensions.OriginatorRequestedAlternateRecipient",
			 decode_ext_orn,
			 ext);
	    default:
		PP_NOTICE (("Unknown global extension type %d", n));
		/* fall */
		if ((retval = flatten_ext (ext, &ep)) != OK)
			return retval;
		for (epp = &ad -> ad_per_recip_ext_list; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}
	return OK;
}

static int	pe2crit (pe, defcrit)
PE	pe;
int	defcrit;
{
	char	*str;
	int	len, n;

	if (pe == NULLPE)
		return defcrit;
	str = bitstr2strb (pe, &len);
	n = strb2int(str, len);
	free (str);
	return n;
}

static int decode_ext_fn (value, genp)
FullName **value;
struct type_MTA_ORName *genp;
{
	return load_fullname (value, genp); 
}

static int do_global_repe_prf (n, dr, ext)
int	n;
DRmpdu *dr;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;
	int retval;

	switch (n) {
	    case EXT_INTERNAL_TRACE_INFORMATION:
		return ext_inttrace (&dr -> dr_trace, ext);
	    case EXT_MESSAGE_SECURITY_LABEL:
		return ext_security (&dr -> dr_security_label,
				     &dr -> dr_security_label_crit,
				     EXT_MESSAGE_SECURITY_LABEL_DC,
				     ext);
	    case EXT_REPORT_ORIGIN_AUTHENTICATION_CHECK:
		return ext_security (&dr -> dr_report_origin_auth_check,
				     &dr -> dr_report_origin_auth_check_crit,
				     EXT_REPORT_ORIGIN_AUTHENTICATION_CHECK_DC,
				     ext);

	    case EXT_REPORTING_MTA_CERTIFICATE:
		return ext_security (&dr -> dr_reporting_mta_certificate,
				     &dr -> dr_reporting_mta_certificate_crit,
				     EXT_REPORTING_MTA_CERTIFICATE_DC,
				     ext);

	    case EXT_REPORT_DL_NAME:
		return decode_extension
			((caddr_t)&dr -> dr_reporting_dl_name,
			 &dr -> dr_reporting_dl_name_crit,
			 EXT_REPORT_DL_NAME_DC,
			 _ZReportingDLNameExt,
			 &_ZExt_mod,
			 "Extension.ReportingDLName",
			 decode_ext_fn,
			 ext);

	    case EXT_ORIGINATOR_AND_DL_EXPANSION_HISTORY:
		return decode_extension
			((caddr_t)&dr -> dr_dl_history,
			 &dr -> dr_dl_history_crit,
			 EXT_ORIGINATOR_AND_DL_EXPANSION_HISTORY_DC,
			 _ZOriginatorAndDLExpansionHistoryExt,
			 &_ZExt_mod,
			 "Extension.OriginatorAndDLExpansion",
			 ext_decode_dlexph,
			 ext);
		
	    default:
		if ((retval = flatten_ext (ext, &ep)) != OK)
			return retval;
		for (epp = &dr -> dr_per_envelope_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}		
	return OK;
}

static int do_global_cont_prf (n, dr, ext)
int	n;
DRmpdu *dr;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;
	int retval;

	switch (n) {
	    case EXT_CONTENT_CORRELATOR:
	    default:
		if ((retval = flatten_ext (ext, &ep)) != OK)
			return retval;
		for (epp = &dr -> dr_per_envelope_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}		
	return OK;
}

static int do_global_rr_prf (n, rr, ext)
int	n;
Rrinfo *rr;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;
	int retval;

	switch (n) {
	    case EXT_RECIPIENT_CERTIFICATE:
		return ext_security
			(&rr -> rr_recip_certificate,
			 &rr -> rr_recip_certificate_crit,
			 EXT_RECIPIENT_CERTIFICATE_DC,
			 ext);

	    case EXT_REDIRECTION_HISTORY:
		return decode_extension
				((caddr_t)&rr -> rr_redirect_history,
				 &rr -> rr_redirect_history_crit,
				 EXT_REDIRECTION_HISTORY_DC,
				 _ZRedirectionHistoryExt,
				 &_ZExt_mod,
				 "Extensions.RedirectionHistory",
				 decode_ext_redir,
				 ext);
		
	    case EXT_PHYSICAL_FORWARDING_ADDRESS:
		return decode_extension
			((caddr_t)&rr -> rr_physical_fwd_addr,
			 &rr -> rr_physical_fwd_addr_crit,
			 EXT_PHYSICAL_FORWARDING_ADDRESS_DC,
			 _ZPhysicalForwardingAddressExt,
			 &_ZExt_mod,
			 "Extension.PhysicalForwardingAddress",
			 decode_ext_fn,
			 ext);

	    case EXT_PROOF_OF_DELIVERY:
		return ext_security (&rr -> rr_report_origin_authentication_check,
				     &rr -> rr_report_origin_authentication_check_crit,
				     EXT_PROOF_OF_DELIVERY_DC,
				     ext);
					 
	    default:
		if ((retval = flatten_ext (ext, &ep)) != OK)
			return retval;
		for (epp = &rr -> rr_per_recip_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}
	return OK;
}
