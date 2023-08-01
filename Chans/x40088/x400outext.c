/* x400outext.c: extension attributes for X400out88 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400outext.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400outext.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: x400outext.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */



#include "Trans-types.h"
#include "Ext-types.h"
#include "util.h"
#include "q.h"
#include "adr.h"
#include "dr.h"
#include "or.h"
#include <isode/cmd_srch.h>
#include <rtsparams.h>

struct type_UNIV_UTCTime *build_time ();
struct type_MTA_ORName *build_fullname ();
struct type_MTA_ORName	*build_addr ();
struct type_MTA_GlobalDomainIdentifier *build_gdi ();
extern int trace_type;

#define wrap_up_ext(e, n)	\
	(e) = (struct type_MTA_Extensions *) smalloc (sizeof *extl); \
	(e) -> next = NULL; (e) -> ExtensionField = (n);
#define STR2QB(s)	str2qb(s, strlen(s), 1)


static struct type_MTA_ExtensionType *build_ext_type (ext_int, ext_oid)
int	ext_int;
OID	ext_oid;
{
	struct type_MTA_ExtensionType *type;

	type = (struct type_MTA_ExtensionType *) smalloc (sizeof *type);

	if (ext_int == EXT_OID_FORM) {
		type -> offset = type_MTA_ExtensionType_local;
		type -> un.local = oid_cpy (ext_oid);
	} else {
		type -> offset = type_MTA_ExtensionType_global;
		type -> un.global = ext_int;
	}
	return type;
}

static struct type_MTA_Criticality *build_crit (cr)
int	cr;
{
	char	*p;

	p = int2strb (cr, 3);
	return strb2bitstr (p, 3, PE_CLASS_UNIV, PE_PRIM_BITS);
}

static PE	build_ext_value (qb)
struct qbuf *qb;
{
	char	*cp;
	PS	ps;
	PE	pe;

	cp = qb2str(qb);

	if ((ps = ps_alloc (str_open)) == NULLPS)
		adios (NULLCP, "PS alloc failed");

	if (str_setup (ps, cp, qb -> qb_len, 1) == NOTOK)
		adios (NULLCP, "Can't setup PS stream");

	if ((pe = ps2pe (ps)) == NULLPE)
		adios (NULLCP, "ps2pe failed [%s]",
		       ps_error (ps -> ps_errno));
	ps_free (ps);
	return pe;
}	

static struct type_MTA_Extensions *build_ext_rrp (parm, crit)
int	parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_RecipientReassignmentProhibited *value;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_RECIPIENT_REASSIGNMENT_PROHIBITED,
				      NULLOID);

	if (crit != EXT_RECIPIENT_REASSIGNMENT_PROHIBITED_DC)
		ext -> criticality = build_crit (crit);
	else ext -> criticality = NULL;

	value = (struct type_Ext_RecipientReassignmentProhibited *)
		smalloc (sizeof *value);
	value -> parm = parm;
	if (encode_Ext_RecipientReassignmentProhibited
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode RecipientReassignmentProhibited value [%s]", PY_pepy);

	PP_PDUP (Ext_RecipientReassignmentProhibited,
		ext -> value, "Extensions.RecipientReassignmentProhibited",
		PDU_WRITE);

	free_Ext_RecipientReassignmentProhibited (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_dep (parm, crit)
int	parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_DLExpansionProhibited *value;
	struct type_MTA_Extensions *extl;

	
	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_DL_EXPANSION_PROHIBITED,
				      NULLOID);

	if (crit != EXT_DL_EXPANSION_PROHIBITED_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = (struct type_Ext_DLExpansionProhibited *)
		smalloc (sizeof *value);
	value -> parm = parm;
	if (encode_Ext_DLExpansionProhibited
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode DLExpansionProhibited value [%s]", PY_pepy);

	PP_PDUP (Ext_DLExpansionProhibited,
		ext -> value, "Extensions.DLExpansionProhibited",
		PDU_WRITE);

	free_Ext_DLExpansionProhibited (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_cwlp (parm, crit)
int	parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_ConversionWithLossProhibited *value;
	struct type_MTA_Extensions *extl;
	
	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_CONVERSION_WITH_LOSS_PROHIBITED,
				      NULLOID);

	if (crit != EXT_CONVERSION_WITH_LOSS_PROHIBITED_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = (struct type_Ext_ConversionWithLossProhibited *)
		smalloc (sizeof *value);
	value -> parm = parm;
	if (encode_Ext_ConversionWithLossProhibited
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode ConversionWithLossProhibited value [%s]", PY_pepy);

	PP_PDUP (Ext_ConversionWithLossProhibited,
		ext -> value, "Extensions.ConversionWithLossProhibited",
		PDU_WRITE);

	free_Ext_ConversionWithLossProhibited (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_lt (parm, crit)
UTC	parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_LatestDeliveryTime *value;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_LATEST_DELIVERY_TIME,
				      NULLOID);

	if (crit != EXT_LATEST_DELIVERY_TIME_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value  = build_time (parm);

	if (encode_Ext_LatestDeliveryTime
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode LatestDeliveryTime value [%s]", PY_pepy);

	PP_PDUP (Ext_LatestDeliveryTime,
		ext -> value, "Extensions.LatestDeliveryTime",
		PDU_WRITE);

/*	free_Ext_LatestDeliveryTime (value); too early */

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_ora (parm, crit)
FullName	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_OriginatorReturnAddress *value;
	struct type_MTA_ORName *orn;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_ORIGINATOR_RETURN_ADDRESS,
				      NULLOID);

	if (crit != EXT_ORIGINATOR_RETURN_ADDRESS_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	
	orn = build_addr (parm -> fn_addr);
	value = (struct type_Ext_OriginatorReturnAddress *)
		smalloc (sizeof *value);
	value -> standard__attributes = orn -> standard__attributes;
	orn -> standard__attributes = NULL;
	value -> domain__defined__attributes = orn -> domain__defined;
	orn -> domain__defined = NULL;
	value -> extension__attributes = orn -> extension__attributes;
	orn -> extension__attributes = NULL;
	free_MTA_ORName (orn);

	if (encode_Ext_OriginatorReturnAddress
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode OriginatorReturnAddress value [%s]", PY_pepy);

	PP_PDUP (Ext_OriginatorReturnAddress,
		ext -> value, "Extensions.OriginatorReturnAddress",
		PDU_WRITE);

/*	free_Ext_OriginatorReturnAddress (value); too early */

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_oc (parm, crit)
struct qbuf	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_ORIGINATOR_CERTIFICATE,
				      NULLOID);

	if (crit != EXT_ORIGINATOR_CERTIFICATE_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	
	ext -> value = build_ext_value (parm);

	PP_PDUP (Ext_OriginatorCertificate,
		ext -> value, "Extensions.OriginatorCertificate",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_ai (parm, crit)
struct qbuf	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER,
				      NULLOID);

	if (crit != EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	
	ext -> value = build_ext_value (parm);

	PP_PDUP (Ext_ContentConfidentialityAlgorithmIdentifier,
		ext -> value, "Extensions.ContentConfidentialityAlgorithmIdentifier",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_moac (parm, crit)
struct qbuf	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK,
				      NULLOID);

	if (crit != EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	
	ext -> value = build_ext_value (parm);

	PP_PDUP (Ext_MessageOriginAuthenticationCheck,
		ext -> value, "Extensions.MessageOriginAuthenticationCheck",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_poac (parm, crit)
struct qbuf	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK,
				      NULLOID);

	if (crit != EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	
	ext -> value = build_ext_value (parm);

	PP_PDUP (Ext_ProbeOriginAuthenticationCheck,
		ext -> value, "Extensions.ProbeOriginAuthenticationCheck",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}


static struct type_MTA_Extensions *build_ext_sl (parm, crit)
struct qbuf	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_MESSAGE_SECURITY_LABEL,
				      NULLOID);

	if (crit != EXT_MESSAGE_SECURITY_LABEL_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	
	ext -> value = build_ext_value (parm);

	PP_PDUP (Ext_MessageSecurityLabel,
		ext -> value, "Extensions.MessageSecurityLabel",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_cc (parm, crit)
struct qbuf	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_CONTENT_CORRELATOR,
				      NULLOID);

	if (crit != EXT_CONTENT_CORRELATOR_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	
	ext -> value = build_ext_value (parm);

	PP_PDUP (Ext_ContentCorrelator,
		ext -> value, "Extensions.ContentCorrelator",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_Ext_DLExpansionHistory * build_dehl (dlh)
DLHistory *dlh;
{
	struct type_Ext_DLExpansionHistory *dlist = NULL, **dp;
	struct type_Ext_DLExpansion *d;

	dp = &dlist;
	for (;dlh; dlh = dlh -> dlh_next) {
		*dp = (struct type_Ext_DLExpansionHistory *)
			smalloc (sizeof **dp);
		(*dp) -> next = NULL;
		d = (*dp) -> DLExpansion =
			(struct type_Ext_DLExpansion *)
				smalloc (sizeof *d);
		d -> address = build_addr (dlh -> dlh_addr);
		d -> dl__expansion__time = build_time (dlh -> dlh_time);
		dp = &(*dp) -> next;
	}
	return dlist;
}
	
static struct type_MTA_Extensions *build_ext_dl (parm, crit)
DLHistory *parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_DLExpansionHistory *value;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_DL_EXPANSION_HISTORY,
				      NULLOID);

	if (crit != EXT_DL_EXPANSION_HISTORY_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = build_dehl (parm);

	if (encode_Ext_DLExpansionHistory
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode DLExpansionHistory value [%s]", PY_pepy);

	PP_PDUP (Ext_DLExpansionHistory,
		ext -> value, "Extensions.DLExpansionHistory",
		PDU_WRITE);

	free_Ext_DLExpansionHistory (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_Ext_MTASuppliedInformation * build_msi (dp)
DomSupInfo *dp;
{
	struct type_Ext_MTASuppliedInformation *msi;

	msi = (struct type_Ext_MTASuppliedInformation *)
		calloc (1, sizeof *msi);
	if (dp -> dsi_time)
		msi -> arrival__time = build_time (dp -> dsi_time);
	msi -> routing__action = (struct type_MTA_RoutingAction *)
		smalloc (sizeof *msi -> routing__action);
	msi -> routing__action -> parm = dp -> dsi_action;

	if (dp -> dsi_attempted_mta ||
	    (dp -> dsi_attempted_md.global_Country &&
	     dp -> dsi_attempted_md.global_Admin)) {
		msi -> attempted = (struct choice_Ext_1 *)
			calloc (1, sizeof *msi -> attempted);
		if (dp -> dsi_attempted_mta) {
			msi -> attempted -> offset =
				choice_Ext_1_mta;
			msi -> attempted -> un.mta =
				STR2QB (dp -> dsi_attempted_mta);
		}
		else {
			msi -> attempted -> offset =
				choice_Ext_1_domain;
			msi -> attempted -> un.domain =
				build_gdi (&dp -> dsi_attempted_md);
		}
	}

	if (dp -> dsi_deferred)
		msi -> deferred__time = build_time (dp -> dsi_deferred);
	if (dp -> dsi_other_actions) {
		msi -> other__actions =
			pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM,
					  PE_PRIM_BITS);
			if (dp -> dsi_other_actions & ACTION_REDIRECTED)
				bit_on (msi -> other__actions,
					bit_MTA_OtherActions_redirected);
			if (dp -> dsi_other_actions & ACTION_EXPANDED)
				bit_on (msi -> other__actions,
					bit_MTA_OtherActions_dl__operation);
	}
	return msi;
}


static struct type_Ext_InternalTraceInformation *build_inttrace (tp)
Trace	*tp;
{
	struct type_Ext_InternalTraceInformation *tlist = NULL, **tpp;
	struct type_Ext_InternalTraceInformationElement *ti;
	Trace	*tlast;

	if (trace_type == RTSP_TRACE_ADMD ||
	    trace_type == RTSP_TRACE_NOINT)
		return NULL;

	for (tlast = tp; tlast && tlast -> trace_next;
	     tlast = tlast -> trace_next)
		continue;

	tpp = &tlist;
	for (; tp; tp = tp -> trace_next) {
		if (tp -> trace_mta == NULLCP)
			continue;
		if (trace_type == RTSP_TRACE_LOCALINT &&
		    !same_prmd (tlast, tp))
			continue;
		*tpp = (struct type_Ext_InternalTraceInformation *)
			smalloc (sizeof **tpp);
		(*tpp) -> next = NULL;
		ti = (*tpp) -> InternalTraceInformationElement =
			(struct type_Ext_InternalTraceInformationElement *)
				calloc (1, sizeof *ti);
		ti -> global__domain__identifier =
			build_gdi (&tp -> trace_DomId);
		ti -> mta__name = STR2QB (tp -> trace_mta);

		ti -> mta__supplied__information =
			build_msi (&tp -> trace_DomSinfo);
		tpp = &(*tpp) -> next;
	}
	return tlist;
}

static struct type_MTA_Extensions *build_ext_iti (parm, crit)
Trace	*parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_InternalTraceInformation *value;
	struct type_MTA_Extensions *extl;
	PE	pe;

	value = build_inttrace (parm);
	if (value == NULL)
		return NULL;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_INTERNAL_TRACE_INFORMATION,
				      NULLOID);

	if (crit != EXT_INTERNAL_TRACE_INFORMATION_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;


	if (encode_Ext_InternalTraceInformation
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode InternalTraceInformation value [%s]", PY_pepy);

	PP_PDUP (Ext_InternalTraceInformation,
		ext -> value, "Extensions.InternalTraceInformation",
		PDU_WRITE);
	pe = pe_cpy (ext->value);
	pe_free (ext->value);
	ext -> value = pe;

	free_Ext_InternalTraceInformation (value);

	wrap_up_ext (extl, ext);
	return extl;
}


static struct type_MTA_Extensions *build_ext_posr (parm, crit)
int	parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_ProofOfSubmissionRequest *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PROOF_OF_SUBMISSION_REQUEST, NULLOID);

	ext -> criticality = build_crit (crit);

	value = (struct type_Ext_ProofOfSubmissionRequest *)
		smalloc (sizeof *value);
	value -> parm = parm;

	if (encode_Ext_ProofOfSubmissionRequest
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode ProofOfSubmissionRequest value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_ProofOfSubmissionRequest,
		ext -> value, "Extensions.ProofOfSubmissionRequest",
		PDU_WRITE);

	free_Ext_ProofOfSubmissionRequest (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_ext_forw_req (parm, crit)
int	parm;
char	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_SequenceNumber *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_FORWARDING_REQUEST, NULLOID);

	ext -> criticality = build_crit (crit);

	value = (struct type_Ext_SequenceNumber *)
		smalloc (sizeof *value);
	value -> parm = parm;

	if (encode_Ext_SequenceNumber
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode forwarding request value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_SequenceNumber,
		ext -> value, "Extensions.SequenceNumber",
		PDU_WRITE);

	free_Ext_SequenceNumber (value);

	wrap_up_ext (extl, ext);
	return extl;
}


static struct type_MTA_ExtensionField *build_extension (ext)
X400_Extension *ext;
{
	struct type_MTA_ExtensionField *ep;

	ep = (struct type_MTA_ExtensionField *) smalloc (sizeof *ep);
	ep -> type = build_ext_type (ext -> ext_int, ext -> ext_oid);
	ep -> criticality = build_crit (ext -> ext_criticality);
	ep -> value = build_ext_value (ext -> ext_value);
	return ep;
}

struct type_MTA_Extensions *build_pm_extensions (qp,p1,msg)
Q_struct *qp;
int p1,msg;
{
	struct type_MTA_Extensions *ext = NULL, **ep;
	X400_Extension *pp_exp;

	ep = &ext;
	for (pp_exp = qp -> per_message_extensions; pp_exp; pp_exp = pp_exp -> ext_next) {
		*ep = (struct type_MTA_Extensions *)
				smalloc (sizeof *ep);
		(*ep) -> next = NULL;
		(*ep) -> ExtensionField = build_extension (pp_exp);
		ep = &(*ep) -> next;
	}
	if (qp -> recip_reassign_prohibited) {
		*ep = build_ext_rrp (qp -> recip_reassign_prohibited,
				     qp -> recip_reassign_crit);
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> dl_expansion_prohibited) {
		*ep = build_ext_dep (qp -> dl_expansion_prohibited,
				     qp -> dl_expansion_crit);
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> conversion_with_loss_prohibited) {
		*ep = build_ext_cwlp (qp -> conversion_with_loss_prohibited,
				      qp -> conversion_with_loss_crit);
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> originator_certificate) {
		*ep = build_ext_oc (qp -> originator_certificate,
				    qp -> originator_certificate_crit);
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> security_label) {
		*ep = build_ext_sl (qp -> security_label,
				    qp -> security_label_crit);
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> general_content_correlator) {
		*ep = build_ext_cc (qp -> general_content_correlator,
				    qp -> content_correlator_crit);
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (msg) {
		if (qp -> latest_time) {
			*ep = build_ext_lt (qp -> latest_time,
					    qp -> latest_time_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> originator_return_address) {
			*ep = build_ext_ora (qp -> originator_return_address,
					     qp -> originator_return_address_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> algorithm_identifier) {
			*ep = build_ext_ai (qp -> algorithm_identifier,
					    qp -> algorithm_identifier_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> message_origin_auth_check) {
			*ep = build_ext_moac (qp -> message_origin_auth_check,
					      qp -> message_origin_auth_check_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	if (!msg) {
		if (qp -> message_origin_auth_check) {
			*ep = build_ext_poac (qp -> message_origin_auth_check,
					      qp -> message_origin_auth_check_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	if (p1) {
		if (qp -> dl_expansion_history) {
			*ep = build_ext_dl (qp -> dl_expansion_history,
					     qp -> dl_expansion_history_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
	
		if (qp -> trace) {
			*ep = build_ext_iti (qp -> trace,
					     EXT_INTERNAL_TRACE_INFORMATION_DC);
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	if ((!p1) && msg) {
		if (qp -> forwarding_request != NOTOK) { 
			*ep = build_ext_forw_req(qp -> forwarding_request,
										 qp -> forwarding_request_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> proof_of_submission_request) {
			*ep = build_ext_posr(qp -> proof_of_submission_request,qp -> proof_of_submission_crit);
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	return ext;
}



static struct type_MTA_Extensions *build_aext_oraa (parm, crit)
char	*parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_OriginatorRequestedAlternateRecipient *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_ORIGINATOR_RETURN_ADDRESS, NULLOID);

	if (crit != EXT_ORIGINATOR_RETURN_ADDRESS_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = build_addr (parm);

	if (encode_Ext_OriginatorRequestedAlternateRecipient
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode OriginatorRequestedAlternateRecipient value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_OriginatorRequestedAlternateRecipient,
		ext -> value, "Extensions.OriginatorRequestedAlternateRecipient",
		PDU_WRITE);

	free_Ext_OriginatorRequestedAlternateRecipient (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_rdm (parm, crit)
int	parm[AD_RDM_MAX];
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_RequestedDeliveryMethod *value, **vp;
	int	i;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_REQUESTED_DELIVERY_METHOD, NULLOID);

	if (crit != EXT_REQUESTED_DELIVERY_METHOD_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = NULL;
	vp = &value;

	for (i = 0; i < AD_RDM_MAX && parm[i] != AD_RDM_NOTUSED; i++) {
		*vp = (struct type_Ext_RequestedDeliveryMethod *)
			smalloc (sizeof **vp);
		(*vp) -> element_Ext_0 = parm[i];
		(*vp) -> next = NULL;
		vp = &(*vp) -> next;
	}

	if (encode_Ext_RequestedDeliveryMethod
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode RequestedDeliveryMethod value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_RequestedDeliveryMethod,
		ext -> value, "Extensions.RequestedDeliveryMethod",
		PDU_WRITE);

	free_Ext_RequestedDeliveryMethod (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_pf (parm, crit)
char	parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_PhysicalForwardingProhibited *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PHYSICAL_FORWARDING_PROHIBITED,
				      NULLOID);

	if (crit != EXT_PHYSICAL_FORWARDING_PROHIBITED_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = (struct type_Ext_PhysicalForwardingProhibited *)
		smalloc (sizeof *value);
	value -> parm = parm;

	if (encode_Ext_PhysicalForwardingProhibited
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode PhysicalForwardingProhibited value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_PhysicalForwardingProhibited,
		ext -> value, "Extend.PhysicalForwardingProhibited",
		PDU_WRITE);

	free_Ext_PhysicalForwardingProhibited (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_pfar (parm, crit)
int	parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_PhysicalForwardingAddressRequest *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST, NULLOID);

	if (crit != EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = (struct type_Ext_PhysicalForwardingAddressRequest*)
		smalloc (sizeof *value);

	value -> parm = parm;

	if (encode_Ext_PhysicalForwardingAddressRequest
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode PhysicalForwardingAddressRequest value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_PhysicalForwardingAddressRequest,
		ext -> value, "Extensions.PhysicalForwardingAddressRequest",
		PDU_WRITE);

	free_Ext_PhysicalForwardingAddressRequest (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_pdm (parm, crit)
int	parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_PhysicalDeliveryModes *value;
	char	*p;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PHYSICAL_DELIVERY_MODES, NULLOID);

	if (crit != EXT_PHYSICAL_DELIVERY_MODES_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	p = int2strb (parm, 8);
	value = strb2bitstr (p, 8, PE_CLASS_UNIV, PE_PRIM_BITS);
	
	if (encode_Ext_PhysicalDeliveryModes
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode PhysicalDeliveryModes value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_PhysicalDeliveryModes,
		ext -> value, "Extensions.PhysicalDeliveryModes",
		PDU_WRITE);

	free_Ext_PhysicalDeliveryModes (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_rmt (parm, crit)
int	parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_RegisteredMailType *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_REGISTERED_MAIL, NULLOID);

	if (crit != EXT_REGISTERED_MAIL_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = (struct type_Ext_RegisteredMailType *)
		smalloc (sizeof *value);
	value -> parm = parm;

	if (encode_Ext_RegisteredMailType
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode RegisteredMailType value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_RegisteredMailType,
		ext -> value, "Extensions.RegisteredMailType",
		PDU_WRITE);

	free_Ext_RegisteredMailType (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_rnfa (parm, crit)
char	*parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_RecipientNumberForAdvice *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_RECIPIENT_NUMBER_FOR_ADVICE, NULLOID);

	if (crit != EXT_RECIPIENT_NUMBER_FOR_ADVICE_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = STR2QB (parm);

	if (encode_Ext_RecipientNumberForAdvice
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode RecipientNumberForAdvice value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_RecipientNumberForAdvice,
		ext -> value, "Extensions.RecipientNumberForAdvice",
		PDU_WRITE);

	free_Ext_RecipientNumberForAdvice (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_pra (parm, crit)
OID	parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_PhysicalRenditionAttributes *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PHYSICAL_RENDITION_ATTRIBUTES,
				      NULLOID);

	if (crit != EXT_PHYSICAL_RENDITION_ATTRIBUTES_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = oid_cpy (parm);

	if (encode_Ext_PhysicalRenditionAttributes
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode PhysicalRenditionAttributes value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_PhysicalRenditionAttributes,
		ext -> value, "Extensions.PhysicalRenditionAttributes",
		PDU_WRITE);

	free_Ext_PhysicalRenditionAttributes (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_pdrr (parm, crit)
int	parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_PhysicalDeliveryReportRequest *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PHYSICAL_DELIVERY_REPORT_REQUEST, NULLOID);

	if (crit != EXT_PHYSICAL_DELIVERY_REPORT_REQUEST_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = (struct type_Ext_PhysicalDeliveryReportRequest *)
		smalloc (sizeof *value);
	value -> parm = parm;

	if (encode_Ext_PhysicalDeliveryReportRequest
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode PhysicalDeliveryReportRequest value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_PhysicalDeliveryReportRequest,
		ext -> value, "Extensions.PhysicalDeliveryReportRequest",
		PDU_WRITE);

	free_Ext_PhysicalDeliveryReportRequest (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_mt (parm, crit)
struct qbuf *parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_MESSAGE_TOKEN, NULLOID);

	if (crit != EXT_MESSAGE_TOKEN_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	ext -> value = build_ext_value (parm);
	PP_PDUP (Ext_MessageToken,
		ext -> value, "Extensions.MessageToken",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_cic (parm, crit)
struct qbuf	*parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_CONTENT_INTEGRITY_CHECK, NULLOID);

	if (crit != EXT_CONTENT_INTEGRITY_CHECK_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	ext -> value = build_ext_value (parm);

	PP_PDUP (Ext_ContentIntegrityCheck,
		ext -> value, "Extensions.ContentIntegrityCheck",
		PDU_WRITE);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_MTA_Extensions *build_aext_podr (parm, crit)
int	parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_ProofOfDeliveryRequest *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_PROOF_OF_DELIVERY_REQUEST, NULLOID);

	if (crit != EXT_PROOF_OF_DELIVERY_REQUEST_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = (struct type_Ext_ProofOfDeliveryRequest *)
		smalloc (sizeof *value);
	value -> parm = parm;

	if (encode_Ext_ProofOfDeliveryRequest
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode ProofOfDeliveryRequest value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_ProofOfDeliveryRequest,
		ext -> value, "Extensions.ProofOfDeliveryRequest",
		PDU_WRITE);

	free_Ext_ProofOfDeliveryRequest (value);

	wrap_up_ext (extl, ext);
	return extl;
}

static struct type_Ext_RedirectionHistory *build_redir (redir)
Redirection *redir;
{
	struct type_Ext_RedirectionHistory *rlist, **rp;
	struct type_Ext_Redirection *rd;

	rlist = NULL;
	rp = &rlist;
	for ( ; redir; redir = redir -> rd_next) {
		*rp = (struct type_Ext_RedirectionHistory *)
			smalloc (sizeof **rp);
		(*rp) -> next = NULL;
		(*rp) -> Redirection = rd =
			(struct type_Ext_Redirection *)
				smalloc (sizeof *rd);
		rd -> intended__recipient__name =
			(struct type_Ext_IntendedRecipientName *)
				smalloc (sizeof *rd -> intended__recipient__name);
		rd -> intended__recipient__name -> address =
			build_addr (redir -> rd_addr);
		rd -> intended__recipient__name -> redirection__time =
			build_time (redir -> rd_time);

		rd -> redirection__reason =
			(struct type_Ext_RedirectionReason *)
				smalloc (sizeof *rd -> redirection__reason);
		rd -> redirection__reason -> parm = redir -> rd_reason;
		
	}
	return rlist;
}

static struct type_MTA_Extensions *build_aext_rh (parm, crit)
Redirection	*parm;
int	crit;
{
	struct type_MTA_ExtensionField *ext;
	struct type_MTA_Extensions *extl;
	struct type_Ext_RedirectionHistory *value;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	ext -> type = build_ext_type (EXT_REDIRECTION_HISTORY, NULLOID);

	if (crit != EXT_REDIRECTION_HISTORY_DC)
		ext -> criticality = build_crit (crit);
	else
		ext -> criticality = NULL;

	value = build_redir (parm);

	if (encode_Ext_RedirectionHistory
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK)
		adios (NULLCP, "Can't encode RedirectionHistory value [%s]",
		       PY_pepy);
	PP_PDUP (Ext_RedirectionHistory,
		ext -> value, "Extensions.RedirectionHistory",
		PDU_WRITE);

	free_Ext_RedirectionHistory (value);

	wrap_up_ext (extl, ext);
	return extl;
}

struct type_MTA_Extensions *build_prf_ext (ad,p1,msg)
ADDR	*ad;
int	p1,msg; 
{
	X400_Extension *pp_ext;
	struct type_MTA_Extensions *extl, **ep;

#define bump(x)	if (*(x)) (x) = &(*x) -> next
	extl = NULL;
	ep = &extl;
	for (pp_ext = ad -> ad_per_recip_ext_list; pp_ext;
	     pp_ext = pp_ext -> ext_next) {
		*ep = (struct type_MTA_Extensions *)
			smalloc (sizeof *ep);
		(*ep) -> next = NULL;
		(*ep) -> ExtensionField = build_extension (pp_ext);
		bump(ep);
	}
	if (ad -> ad_orig_req_alt) {
		*ep = build_aext_oraa (ad -> ad_orig_req_alt,
				       ad -> ad_orig_req_alt_crit);
		bump(ep);
	}
	if (ad -> ad_req_del[0] != AD_RDM_NOTUSED) {
		*ep = build_aext_rdm (ad -> ad_req_del,
				      ad -> ad_req_del_crit);
		bump(ep);
	}
	if (ad -> ad_phys_rendition_attribs) {
		*ep = build_aext_pra (ad -> ad_phys_rendition_attribs,
				      ad -> ad_phys_rendition_attribs_crit);
		bump(ep);
	}
	if (msg) {
		if (ad -> ad_phys_forward) {
			*ep = build_aext_pf (ad -> ad_phys_forward,
					     ad -> ad_phys_forward_crit);
			bump(ep);
		}
		if (ad -> ad_phys_fw_ad_req) {
			*ep = build_aext_pfar (ad -> ad_phys_fw_ad_req,
					       ad -> ad_phys_fw_ad_crit);
			bump(ep);
		}
		if (ad -> ad_phys_modes) {
			*ep = build_aext_pdm (ad -> ad_phys_modes,
					      ad -> ad_phys_modes_crit);
			bump(ep);
		}

		if (ad -> ad_reg_mail_type) {
			*ep = build_aext_rmt (ad -> ad_reg_mail_type,
					      ad -> ad_reg_mail_type_crit);
			bump (ep);
		}
		if (ad -> ad_recip_number_for_advice) {
			*ep = build_aext_rnfa (ad -> ad_recip_number_for_advice,
					       ad -> ad_recip_number_for_advice_crit);
			bump (ep);
		}
		if (ad -> ad_pd_report_request) {
			*ep = build_aext_pdrr (ad -> ad_pd_report_request,
					      ad -> ad_pd_report_request_crit);
			bump (ep);
		}
		if (ad -> ad_message_token) {
			*ep = build_aext_mt (ad -> ad_message_token,
					     ad -> ad_message_token_crit);
			bump (ep);
		}
		if (ad -> ad_content_integrity) {
			*ep = build_aext_cic (ad -> ad_content_integrity,
					      ad -> ad_content_integrity_crit);
			bump(ep);
		}
		if (ad -> ad_proof_delivery) {
			*ep = build_aext_podr (ad -> ad_proof_delivery,
					       ad -> ad_proof_delivery_crit);
			bump(ep);
		}
	}
	if (p1) {
		if (ad -> ad_redirection_history) {
			*ep = build_aext_rh (ad -> ad_redirection_history,
					     ad -> ad_redirection_history_crit);
			bump (ep);
		}
	}
	return extl;
}
