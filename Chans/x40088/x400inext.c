/* x400inext.c: X400(1988) extensions handling - inbound */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400inext.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400inext.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: x400inext.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */



#include "Ext-types.h"
#include "Trans-types.h"
#include "util.h"
#include "chan.h"
#include "q.h"
#include "adr.h"
#include "or.h"
#include "prm.h"
#include "dr.h"
#include "retcode.h"
#include <isode/rtsap.h>

static void	decode_ext_time ();

#ifndef PEPSY_VERSION
extern int print_Ext_LatestDeliveryTime ();
extern int decode_Ext_LatestDeliveryTime ();
extern int print_Ext_PhysicalRenditionAttributes ();
extern int decode_Ext_PhysicalRenditionAttributes ();
extern int print_Ext_RecipientNumberForAdvice ();
extern int decode_Ext_RecipientNumberForAdvice ();
extern int print_Ext_RequestedDeliveryMethod ();
extern int decode_Ext_RequestedDeliveryMethod ();
extern int print_Ext_PhysicalDeliveryModes ();
extern int decode_Ext_PhysicalDeliveryModes ();
extern int print_Ext_RecipientReassignmentProhibited();
extern int decode_Ext_RecipientReassignmentProhibited();
extern int print_Ext_DLExpansionProhibited();
extern int decode_Ext_DLExpansionProhibited();
extern int print_Ext_ConversionWithLossProhibited();
extern int decode_Ext_ConversionWithLossProhibited();
extern int print_Ext_SequenceNumber();
extern int decode_Ext_SequenceNumber();
extern int print_Ext_ProofOfSubmissionRequest();
extern int decode_Ext_ProofOfSubmissionRequest();
extern int print_Ext_ProofOfDeliveryRequest();
extern int decode_Ext_ProofOfDeliveryRequest();
extern int print_Ext_PhysicalForwardingProhibited();
extern int decode_Ext_PhysicalForwardingProhibited();
extern int print_Ext_PhysicalForwardingAddressRequest();
extern int decode_Ext_PhysicalForwardingAddressRequest();
extern int print_Ext_RegisteredMailType();
extern int decode_Ext_RegisteredMailType();
extern int print_Ext_PhysicalDeliveryReportRequest();
extern int decode_Ext_PhysicalDeliveryReportRequest();
#endif


void	do_local_extension ();
void	do_global_extension ();

static X400_Extension *flatten_ext ();
static void	ext_recip_reassign ();
static void	ext_dlxp ();
static void	ext_conv ();
static void	ext_ora ();
static void	ext_dlexph ();
static void 	ext_inttrace ();

static int	pe2crit ();

static DLHistory *ext_decode_dlh  ();

#ifdef PEPSY_VERSION

#define decode_singleint(type, ip, critp, defcrit, magic_num, mod, label, ext) \
	{ \
	struct type *genp; \
\
	if (pp_log_norm -> ll_events & LLOG_PDUS) \
		pvpdu (pp_log_norm, (magic_num), (mod), \
		       (ext) -> value, label, PDU_READ); \
	if (dec_f ((magic_num), (mod), (ext) -> value, 1, NULLIP, NULLVP, &genp) == NOTOK) \
		adios (NULLCP, "Can't parse %s value [%s]", label, PY_pepy); \
\
	(ip) = genp -> parm; \
	(critp) = pe2crit ((ext) -> criticality, defcrit); \
	fre_obj ((char *) (genp), (mod)->md_dtab[(magic_num)], (mod)); \
	}

static void decode_extension (value, crit, defcrit, magic_num, mod,
			      label, decoder, ext)
caddr_t	value;
char	*crit;
int	defcrit;
int	magic_num;
modtyp	*mod;
VFP	decoder;
char	*label;
struct type_MTA_ExtensionField *ext;
{
	caddr_t *genp;

#if PP_DEBUG > 0
	if (pp_log_norm -> ll_events & LLOG_PDUS)
		pvpdu (pp_log_norm, magic_num, mod, 
			ext -> value, label, PDU_READ);
#endif
	if (dec_f(magic_num, mod, ext -> value, 1, NULLIP, NULLVP, &genp) == NOTOK)
		adios (NULLCP, "Can't parse %s value [%s]", label, PY_pepy);

	(*decoder) (value, genp);

	*crit = pe2crit (ext -> criticality, defcrit);

	fre_obj((char *) genp, mod->md_dtab[magic_num], mod);
}
#else

#define decode_singleint(type, ip, critp, defcrit, pfnx, dfnx, ffnx, label, ext) \
	{ \
	struct type *genp; \
\
	PP_PDU (pfnx, ext -> value, label, PDU_READ); \
	if (dfnx (ext -> value, 1, NULLIP, NULLVP, &genp) == NOTOK) \
		adios (NULLCP, "Can't parse %s value [%s]", label, PY_pepy); \
\
	(ip) = genp -> parm; \
	(critp) = pe2crit (ext -> criticality, defcrit); \
	ffnx (genp); \
	}

static void decode_extension (value, crit, defcrit, pfnx, dfnx, ffnx,
			      label, decoder, ext)
caddr_t	value;
char	*crit;
int	defcrit;
IFP	pfnx, dfnx, ffnx;
VFP	decoder;
char	*label;
struct type_MTA_ExtensionField *ext;
{
	caddr_t *genp;

#if PP_DEBUG > 0
	if (pfnx && pp_log_norm -> ll_events & LLOG_PDUS)
		_vpdu (pp_log_norm, pfnx, ext -> value, label, PDU_READ);
#endif
	if ((*dfnx) (ext -> value, 1, NULLIP, NULLVP, &genp) == NOTOK)
		adios (NULLCP, "Can't parse %s value [%s]", label, PY_pepy);

	(*decoder) (value, genp);

	*crit = pe2crit (ext -> criticality, defcrit);

	(*ffnx) (genp);
}
#endif

extern OR_ptr do_or_address ();

#define bit_ison(x,y)	(bit_test(x,y) == 1)

load_pm_extensions (qp, ext,p1,msg)
Q_struct *qp;
struct type_MTA_Extensions *ext;
int p1,msg;
{
	struct type_MTA_Extensions *ep;

	for (ep = ext; ep; ep = ep -> next) {
		switch (ep -> ExtensionField -> type -> offset) {
		    case type_MTA_ExtensionType_local:
			do_local_extension (&qp -> per_message_extensions,
					    ep -> ExtensionField);
			break;

		    case type_MTA_ExtensionType_global:
			do_global_extension (ep -> ExtensionField ->
					     type -> un.global,
					     qp, ep -> ExtensionField,p1,msg);
			break;

		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown extension type %d",
				 ep -> ExtensionField -> type -> offset));
			break;
		}
	}
}


squash_ext (qbp, value)
struct qbuf **qbp;
PE	value;
{
	char	*cp;
	int	len;
	PS	ps;

	if ((ps = ps_alloc (str_open)) == NULLPS)
		adios (NULLCP, "Can't allocate PS");

	len = ps_get_abs (value);
	cp = smalloc (len);

	if (str_setup (ps, cp, len, 1) == NOTOK)
		adios (NULLCP, "Can't setup stream [%s]",
		       ps_error (ps -> ps_errno));

	if (pe2ps (ps, value) == NOTOK)
		adios (NULLCP, "pe2ps failed [%s]", ps_error (ps -> ps_errno));
	
	ps_free (ps);

	*qbp = str2qb (cp, len, 1);
	free (cp);
}

static X400_Extension *flatten_ext (ext)
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep;

	ep = (X400_Extension *)smalloc (sizeof *ep);
	if (ext -> type -> offset == type_MTA_ExtensionType_global) {
		ep -> ext_int = ext -> type -> un.global;
		ep -> ext_oid = NULLOID;
	}
	else {
		ep -> ext_int = EXT_OID_FORM;
		ep -> ext_oid = oid_cpy (ext -> type -> un.local);
	}
		
	squash_ext (&ep -> ext_value, ext -> value);
	ep -> ext_criticality = pe2crit (ext -> criticality,
					 CRITICAL_NONE);
	ep -> ext_next = NULL;
	return ep;
}

void do_local_extension (elist, ext)
X400_Extension **elist;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;

	PP_LOG (LLOG_EXCEPTIONS, ("Unknown local extension %s",
				  sprintoid (ext -> type -> un.local)));

	ep = flatten_ext (ext);
	for (epp = elist; *epp;
	     epp = &(*epp) -> ext_next)
		continue;
	*epp = ep;
}

static void ext_security (qbp, critp, defcrit, ext)
struct qbuf **qbp;
char	*critp;
int	defcrit;
struct type_MTA_ExtensionField *ext;
{
	squash_ext (qbp, ext -> value);

	*critp = pe2crit (ext -> criticality, defcrit);
}

static void do_global_extension (n, qp, ext, p1, msg)
int	n;
Q_struct *qp;
struct type_MTA_ExtensionField *ext;
int p1,msg;
{
	X400_Extension	*ep, **epp;

	switch (n) {
	    case EXT_RECIPIENT_REASSIGNMENT_PROHIBITED:
#ifdef PEPSY_VERSION
		decode_singleint (type_Ext_RecipientReassignmentProhibited,
			       qp -> recip_reassign_prohibited,
			       qp -> recip_reassign_crit,
			       EXT_RECIPIENT_REASSIGNMENT_PROHIBITED_DC,
			       _ZRecipientReassignmentProhibitedExt,
			       &_ZExt_mod,
			       "Extension.RecipientReassignmentProhibited",
			       ext);
#else
		decode_singleint (type_Ext_RecipientReassignmentProhibited,
			       qp -> recip_reassign_prohibited,
			       qp -> recip_reassign_crit,
			       EXT_RECIPIENT_REASSIGNMENT_PROHIBITED_DC,
			       print_Ext_RecipientReassignmentProhibited,
			       decode_Ext_RecipientReassignmentProhibited,
			       free_Ext_RecipientReassignmentProhibited,
			       "Extension.RecipientReassignmentProhibited",
			       ext);
#endif
		break;

	    case EXT_DL_EXPANSION_PROHIBITED:
#ifdef PEPSY_VERSION
		decode_singleint (type_Ext_DLExpansionProhibited,
			       qp -> dl_expansion_prohibited,
			       qp -> dl_expansion_crit,
			       EXT_DL_EXPANSION_PROHIBITED_DC,
			       _ZDLExpansionProhibitedExt,
			       &_ZExt_mod,
			       "Extension.DLExpansionProhibited",
			       ext);
#else
		decode_singleint (type_Ext_DLExpansionProhibited,
			       qp -> dl_expansion_prohibited,
			       qp -> dl_expansion_crit,
			       EXT_DL_EXPANSION_PROHIBITED_DC,
			       print_Ext_DLExpansionProhibited,
			       decode_Ext_DLExpansionProhibited,
			       free_Ext_DLExpansionProhibited,
			       "Extension.DLExpansionProhibited",
			       ext);
#endif
		break;

	    case EXT_CONVERSION_WITH_LOSS_PROHIBITED:
#ifdef PEPSY_VERSION
		decode_singleint (type_Ext_ConversionWithLossProhibited,
			       qp -> conversion_with_loss_prohibited,
			       qp -> conversion_with_loss_crit,
			       EXT_CONVERSION_WITH_LOSS_PROHIBITED_DC,
			       _ZConversionWithLossProhibitedExt,
			       &_ZExt_mod,
			       "Extensions.ConversionWithLossProhibited",
			       ext);
#else
		decode_singleint (type_Ext_ConversionWithLossProhibited,
			       qp -> conversion_with_loss_prohibited,
			       qp -> conversion_with_loss_crit,
			       EXT_CONVERSION_WITH_LOSS_PROHIBITED_DC,
			       print_Ext_ConversionWithLossProhibited,
			       decode_Ext_ConversionWithLossProhibited,
			       free_Ext_ConversionWithLossProhibited,
			       "Extensions.ConversionWithLossProhibited",
			       ext);
#endif
		break;

	    case EXT_ORIGINATOR_CERTIFICATE:
		ext_security (&qp -> originator_certificate,
			      &qp -> originator_certificate_crit,
			      EXT_ORIGINATOR_CERTIFICATE_DC,
			      ext);
		break;

	    case EXT_MESSAGE_SECURITY_LABEL:
		ext_security (&qp -> security_label,
			      &qp -> security_label_crit,
			      EXT_MESSAGE_SECURITY_LABEL_DC,
			      ext);
		break;
	    case EXT_CONTENT_CORRELATOR:
		ext_security (&qp -> general_content_correlator,
			      &qp -> content_correlator_crit,
			      EXT_CONTENT_CORRELATOR_DC,
			      ext);
		break;

	    case EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK:
		if (!msg)
			ext_security (&qp -> message_origin_auth_check,
				      &qp -> message_origin_auth_check_crit,
				      EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK_DC,
				      ext);
		break;

	    case EXT_DL_EXPANSION_HISTORY:
		if (p1)
			ext_dlexph (qp, ext);
		break;

	    case EXT_INTERNAL_TRACE_INFORMATION:
		if (p1)
			ext_inttrace (&qp -> trace, ext);
		break;

	    case EXT_LATEST_DELIVERY_TIME:
		if (msg)
			decode_extension (&qp -> latest_time,
					  &qp -> latest_time_crit,
					  EXT_LATEST_DELIVERY_TIME_DC,
#ifdef PEPSY_VERSION
					  _ZLatestDeliveryTimeExt,
					  &_ZExt_mod,
#else
					  print_Ext_LatestDeliveryTime,
					  decode_Ext_LatestDeliveryTime,
					  free_Ext_LatestDeliveryTime,
#endif
					  "Extensions.LatestDeliveryTime",
					  decode_ext_time,
					  ext);
		break;

	    case EXT_ORIGINATOR_RETURN_ADDRESS:
		if (msg)
			ext_ora (qp, ext);
		break;

	    case EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER:
		if (msg)
			ext_security (&qp -> algorithm_identifier,
				      &qp -> algorithm_identifier_crit,
				      EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER_DC,
				      ext);
		break;
	    case EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK:
		if (msg)
			ext_security (&qp -> message_origin_auth_check,
				      &qp -> message_origin_auth_check_crit,
				      EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK_DC,
				      ext);
		break;
	    case EXT_FORWARDING_REQUEST:
		if (msg && (!p1))
#ifdef PEPSY_VERSION
			decode_singleint (type_Ext_SequenceNumber,
			       qp -> forwarding_request,
			       qp -> forwarding_request_crit,
			       EXT_FORWARDING_REQUEST_DC,
			       _ZSequenceNumberExt,
			       &_ZExt_mod,
			       "Extension.ForwardingRequest.SequenceNumber",
			       ext);
#else
			decode_singleint (type_Ext_SequenceNumber,
			       qp -> forwarding_request,
			       qp -> forwarding_request_crit,
			       EXT_FORWARDING_REQUEST_DC,
			       print_Ext_SequenceNumber,
			       decode_Ext_SequenceNumber,
			       free_Ext_SequenceNumber,
			       "Extension.ForwardingRequest.SequenceNumber",
			       ext);
#endif
		break;
	    case EXT_PROOF_OF_SUBMISSION_REQUEST:
		if (msg && (!p1))
#ifdef PEPSY_VERSION
			decode_singleint (type_Ext_ProofOfSubmissionRequest,
			       qp -> proof_of_submission_request,
			       qp -> proof_of_submission_crit,
			       EXT_PROOF_OF_SUBMISSION_REQUEST_DC,
			       _ZProofOfSubmissionRequestExt,
			       &_ZExt_mod,
			       "Extension.ProofOfSubmissionRequest",
			       ext);
#else
			decode_singleint (type_Ext_ProofOfSubmissionRequest,
			       qp -> proof_of_submission_request,
			       qp -> proof_of_submission_crit,
			       EXT_PROOF_OF_SUBMISSION_REQUEST_DC,
			       print_Ext_ProofOfSubmissionRequest,
			       decode_Ext_ProofOfSubmissionRequest,
			       free_Ext_ProofOfSubmissionRequest,
			       "Extension.ProofOfSubmissionRequest",
			       ext);
#endif
		break;
	    default:
		PP_NOTICE (("Unknown global extension type %d", n));
		/* fall */
		ep = flatten_ext (ext);
		for (epp = &qp -> per_message_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;

	}
}


static void ext_ora (qp, ext)
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
						       &ora) == NOTOK)
		adios (NULLCP, "Can't parse originator return address [%s]",
		       PY_pepy);

	or = do_or_address (ora -> standard__attributes,
			    ora -> domain__defined__attributes,
			    ora -> extension__attributes);
	or_or2std (or, buf, 0);
	or_free (or);

	qp -> originator_return_address = fn_new (buf, NULLCP);

	qp -> originator_return_address_crit = pe2crit (ext -> criticality,
							EXT_ORIGINATOR_RETURN_ADDRESS_DC);
	free_Ext_OriginatorReturnAddress (ora);
}

static void ext_dlexph (qp, ext)
Q_struct	*qp;
struct type_MTA_ExtensionField *ext;
{
	struct type_Ext_DLExpansionHistory *dlh, *dlhp;
	DLHistory *dp;

	PP_PDU (print_Ext_DLExpansionHistory, ext -> value,
		"Extensions.DLExpansionHistory", PDU_READ);

	if (decode_Ext_DLExpansionHistory (ext -> value, 1,
						  NULLIP, NULLVP,
						  &dlh) == NOTOK)
		adios (NULLCP, "Can't parse DLExpansionHistory value [%s]",
		       PY_pepy);
	for (dlhp = dlh; dlhp; dlhp = dlhp -> next) {
		dp = ext_decode_dlh (dlhp -> DLExpansion);
		dlh_add (&qp -> dl_expansion_history, dp);
	}
	qp -> dl_expansion_history_crit = pe2crit (ext -> criticality,
						   EXT_DL_EXPANSION_HISTORY_DC);
	free_Ext_DLExpansionHistory (dlh);
}

static DLHistory *ext_decode_dlh (dlp)
struct type_Ext_DLExpansion *dlp;
{
	char	buf[BUFSIZ];
	UTC	ut;
	OR_ptr	or;
	DLHistory *dp;

	load_time (&ut, dlp -> dl__expansion__time);
	or = do_or_address (dlp -> address -> standard__attributes,
			    dlp -> address -> domain__defined,
			    dlp -> address -> extension__attributes);
	or_or2std (or, buf, 0);
	or_free (or);

	dp = dlh_new (buf, NULLCP, ut);
	free ((char *)ut);
	return dp;
}

static void decode_ext_time (utc, genp)
UTC	*utc;
struct type_UNIV_UTCTime *genp;
{
	load_time (utc, genp);
}

static void ext_inttrace (trace, ext)
Trace	**trace;
struct type_MTA_ExtensionField *ext;
{
	struct type_Ext_InternalTraceInformation *intt, *ip;

	PP_PDU (print_Ext_InternalTraceInformation,
		ext -> value, "Extension.InternalTraceInformation",
		PDU_READ);

	if (decode_Ext_InternalTraceInformation (ext -> value, 1,
							NULLIP, NULLVP,
							&intt) == NOTOK)
		adios (NULLCP, "Can't parse InternalTraceInformation [%s]",
		       PY_pepy);

	for (ip = intt; ip; ip = ip -> next) {
		struct type_Ext_InternalTraceInformationElement *te;
		struct type_Ext_MTASuppliedInformation *msi;
		Trace *tp;

		te = ip -> InternalTraceInformationElement;
		tp = trace_new ();
		load_gdi (&tp -> trace_DomId,
			  te -> global__domain__identifier);
		tp -> trace_mta = qb2str (te -> mta__name);
		msi = te -> mta__supplied__information;
		if (msi -> arrival__time)
			load_time (&tp -> trace_DomSinfo.dsi_time,
				   msi -> arrival__time);
		if (msi -> routing__action)
			tp -> trace_DomSinfo.dsi_action =
				msi -> routing__action -> parm;
		if (msi -> deferred__time)
			load_time (&tp -> trace_DomSinfo.dsi_deferred,
				   msi -> deferred__time);
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
				tp -> trace_DomSinfo.dsi_attempted_mta =
					qb2str (msi -> attempted -> un.mta);
				break;
			    case choice_Ext_1_domain:
				load_gdi (&tp -> trace_DomSinfo.dsi_attempted_md,
					  msi -> attempted -> un.domain);
				break;
				

			    default:
				break;
			}
		}
		trace_add (trace, tp);
	}
}

static void decode_ext_pdm (value, genp)
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
}

static void decode_ext_pra (value, gen)
OID	*value;
OID	gen;
{
	*value = oid_cpy (gen);
}
static void decode_ext_rnfa (value, genp)
char	**value;
struct qbuf *genp;
{
	*value = qb2str (genp);
}

static void decode_ext_rdm (value, genp)
int	value[AD_RDM_MAX];
struct type_Ext_RequestedDeliveryMethod *genp;
{
	int i;
	
	for (i = 0; i < AD_RDM_MAX && genp;
	     i++, genp = genp -> next)
		value[i] = genp -> element_Ext_0;
}

do_global_ext_prf (n, ad, ext, p1, msg)
int	n;
ADDR	*ad;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;

	switch (n) {
	    case EXT_MESSAGE_TOKEN:
		if (msg)
			ext_security (&ad -> ad_message_token,
			      &ad -> ad_message_token_crit,
			      EXT_MESSAGE_TOKEN_DC,
			      ext);
		break;
	    case EXT_CONTENT_INTEGRITY_CHECK:
		if (msg)
			ext_security (&ad -> ad_content_integrity,
			      &ad -> ad_content_integrity_crit,
			      EXT_CONTENT_INTEGRITY_CHECK_DC,
			      ext);
		break;
	    case EXT_PROOF_OF_DELIVERY_REQUEST:
		if (msg)
#ifdef PEPSY_VERSION
			decode_singleint (type_Ext_ProofOfDeliveryRequest,
			       ad -> ad_proof_delivery,
			       ad -> ad_proof_delivery_crit,
			       EXT_PROOF_OF_DELIVERY_REQUEST_DC,
			       _ZProofOfDeliveryRequestExt,
			       &_ZExt_mod,
			       "Extensions.ProofOfDeliveryRequest",
			       ext);
#else
			decode_singleint (type_Ext_ProofOfDeliveryRequest,
			       ad -> ad_proof_delivery,
			       ad -> ad_proof_delivery_crit,
			       EXT_PROOF_OF_DELIVERY_REQUEST_DC,
			       print_Ext_ProofOfDeliveryRequest,
			       decode_Ext_ProofOfDeliveryRequest,
			       free_Ext_ProofOfDeliveryRequest,
			       "Extensions.ProofOfDeliveryRequest",
			       ext);
#endif
		break;
	    case EXT_PHYSICAL_FORWARDING_PROHIBITED:
		if (msg)
#ifdef PEPSY_VERSION
			decode_singleint (type_Ext_PhysicalForwardingProhibited,
			       ad -> ad_phys_forward,
			       ad -> ad_phys_forward_crit,
			       EXT_PHYSICAL_FORWARDING_PROHIBITED_DC,
			       _ZPhysicalForwardingProhibitedExt,
			       &_ZExt_mod,
			       "Extensions.PhysicalForwardingProhibited",
			       ext);
#else
			decode_singleint (type_Ext_PhysicalForwardingProhibited,
			       ad -> ad_phys_forward,
			       ad -> ad_phys_forward_crit,
			       EXT_PHYSICAL_FORWARDING_PROHIBITED_DC,
			       print_Ext_PhysicalForwardingProhibited,
			       decode_Ext_PhysicalForwardingProhibited,
			       free_Ext_PhysicalForwardingProhibited,
			       "Extensions.PhysicalForwardingProhibited",
			       ext);
#endif
		break;
	    case EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST:
		if (msg)
#ifdef PEPSY_VERSION
			decode_singleint (type_Ext_PhysicalForwardingAddressRequest,
				  ad -> ad_phys_fw_ad_req,
				  ad -> ad_phys_fw_ad_crit,
				  EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST_DC,
				  _ZPhysicalForwardingAddressRequestExt,
			          &_ZExt_mod,
				  "Extensions.PhysicalForwardingAddressRequest",
				  ext);
#else
			decode_singleint (type_Ext_PhysicalForwardingAddressRequest,
				  ad -> ad_phys_fw_ad_req,
				  ad -> ad_phys_fw_ad_crit,
				  EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST_DC,
				  print_Ext_PhysicalForwardingAddressRequest,
				  decode_Ext_PhysicalForwardingAddressRequest,
				  free_Ext_PhysicalForwardingAddressRequest,
				  "Extensions.PhysicalForwardingAddressRequest",
				  ext);
#endif
		break;
	    case EXT_REGISTERED_MAIL:
		if (msg)
#ifdef PEPSY_VERSION
			decode_singleint (type_Ext_RegisteredMailType,
				  ad -> ad_reg_mail_type,
				  ad -> ad_reg_mail_type_crit,
				  EXT_REGISTERED_MAIL_DC,
			          _ZRegisteredMailTypeExt,
			          &_ZExt_mod,
				  "Extensions.RegisteredMailType",
				  ext);
#else
			decode_singleint (type_Ext_RegisteredMailType,
				  ad -> ad_reg_mail_type,
				  ad -> ad_reg_mail_type_crit,
				  EXT_REGISTERED_MAIL_DC,
				  print_Ext_RegisteredMailType,
				  decode_Ext_RegisteredMailType,
				  free_Ext_RegisteredMailType,
				  "Extensions.RegisteredMailType",
				  ext);
#endif
		break;
	    case EXT_PHYSICAL_DELIVERY_REPORT_REQUEST:
		if (msg)
#ifdef PEPSY_VERSION
			decode_singleint (type_Ext_PhysicalDeliveryReportRequest,
				  ad -> ad_pd_report_request,
				  ad -> ad_pd_report_request_crit,
				  EXT_PHYSICAL_DELIVERY_REPORT_REQUEST_DC,
			          _ZPhysicalDeliveryReportRequestExt,
			          &_ZExt_mod,
				  "Extensions.PhysicalDeliveryReportRequest",
				  ext);
#else
			decode_singleint (type_Ext_PhysicalDeliveryReportRequest,
				  ad -> ad_pd_report_request,
				  ad -> ad_pd_report_request_crit,
				  EXT_PHYSICAL_DELIVERY_REPORT_REQUEST_DC,
				  print_Ext_PhysicalDeliveryReportRequest,
				  decode_Ext_PhysicalDeliveryReportRequest,
				  free_Ext_PhysicalDeliveryReportRequest,
				  "Extensions.PhysicalDeliveryReportRequest",
				  ext);
#endif
		break;
	    case EXT_PHYSICAL_RENDITION_ATTRIBUTES:
		decode_extension (&ad -> ad_phys_rendition_attribs,
				  &ad -> ad_phys_rendition_attribs_crit,
				  EXT_PHYSICAL_RENDITION_ATTRIBUTES_DC,
#ifdef PEPSY_VERSION
				  _ZPhysicalRenditionAttributesExt,
				  &_ZExt_mod,
#else
				  print_Ext_PhysicalRenditionAttributes,
				  decode_Ext_PhysicalRenditionAttributes,
				  free_Ext_PhysicalRenditionAttributes,
#endif
				  "Extensions.PhysicalRenditionAttributes",
				  decode_ext_pra,
				  ext);
		break;
	    case EXT_RECIPIENT_NUMBER_FOR_ADVICE:
		if (msg)
			decode_extension (&ad -> ad_recip_number_for_advice,
				  &ad -> ad_recip_number_for_advice_crit,
				  EXT_RECIPIENT_NUMBER_FOR_ADVICE_DC,
#ifdef PEPSY_VERSION
				  _ZRecipientNumberForAdviceExt,
				  &_ZExt_mod,
#else
				  print_Ext_RecipientNumberForAdvice,
				  decode_Ext_RecipientNumberForAdvice,
				  free_Ext_RecipientNumberForAdvice,
#endif
				  "Extensions.RecipientNumberForAdvice",
				  decode_ext_rnfa,
				  ext);
		break;
	    case EXT_REQUESTED_DELIVERY_METHOD:
		decode_extension (ad -> ad_req_del,
				  &ad -> ad_req_del_crit,
				  EXT_REQUESTED_DELIVERY_METHOD_DC,
#ifdef PEPSY_VERSION
				  _ZRequestedDeliveryMethodExt,
				  &_ZExt_mod,
#else
				  print_Ext_RequestedDeliveryMethod,
				  decode_Ext_RequestedDeliveryMethod,
				  free_Ext_RequestedDeliveryMethod,
#endif
				  "Extensions.RequestedDeliveryMethod",
				  decode_ext_rdm,
				  ext);
		break;
	    case EXT_PHYSICAL_DELIVERY_MODES:
		if (msg)
			decode_extension (&ad -> ad_phys_modes,
				  &ad -> ad_phys_modes_crit,
				  EXT_PHYSICAL_DELIVERY_MODES_DC,
#ifdef PEPSY_VERSION
				  _ZPhysicalDeliveryModesExt,
				  &_ZExt_mod,
#else
				  print_Ext_PhysicalDeliveryModes,
				  decode_Ext_PhysicalDeliveryModes,
				  free_Ext_PhysicalDeliveryModes,
#endif
				  "Extensions.PhysicalDeliveryModes",
				  decode_ext_pdm,
				  ext);
		break;

	    case EXT_REDIRECTION_HISTORY:
		if (p1) {
			ep = flatten_ext (ext);
			for (epp = &ad -> ad_per_recip_ext_list; *epp;
			     epp = &(*epp) -> ext_next)
				continue;
			*epp = ep;
		}
		break;

	    default:
		PP_NOTICE (("Unknown global extension type %d", n));
		/* fall */
	    case EXT_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT:
		ep = flatten_ext (ext);
		for (epp = &ad -> ad_per_recip_ext_list; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}
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

do_global_repe_prf (n, dr, ext)
int	n;
DRmpdu *dr;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;

	switch (n) {
	    case EXT_INTERNAL_TRACE_INFORMATION:
		ext_inttrace (&dr -> dr_trace, ext);
		break;
	    case EXT_MESSAGE_SECURITY_LABEL:
		ext_security (&dr -> dr_security_label,
			      &dr -> dr_security_label_crit,
			      EXT_MESSAGE_SECURITY_LABEL_DC,
			      ext);
		break;
	    case EXT_REPORT_ORIGIN_AUTHENTICATION_CHECK:
		ext_security (&dr -> dr_report_origin_auth_check,
			      &dr -> dr_report_origin_auth_check_crit,
			      EXT_REPORT_ORIGIN_AUTHENTICATION_CHECK_DC,
			      ext);
		break;
	    case EXT_REPORTING_MTA_CERTIFICATE:
		ext_security (&dr -> dr_reporting_mta_certificate,
			      &dr -> dr_reporting_mta_certificate_crit,
			      EXT_REPORTING_MTA_CERTIFICATE_DC,
			      ext);
		break;
	    case EXT_ORIGINATOR_AND_DL_EXPANSION_HISTORY:
	    case EXT_REPORT_DL_NAME:
	    default:
		ep = flatten_ext (ext);
		for (epp = &dr -> dr_per_envelope_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}		
}

do_global_cont_prf (n, dr, ext)
int	n;
DRmpdu *dr;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;

	switch (n) {
	    case EXT_CONTENT_CORRELATOR:
	    default:
		ep = flatten_ext (ext);
		for (epp = &dr -> dr_per_envelope_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}		
}

do_global_rr_prf (n, rr, ext)
int	n;
Rrinfo *rr;
struct type_MTA_ExtensionField *ext;
{
	X400_Extension	*ep, **epp;

	switch (n) {
	    case EXT_RECIPIENT_CERTIFICATE:
		ext_security (&rr -> rr_report_origin_authentication_check,
			      &rr -> rr_report_origin_authentication_check_crit,
			      EXT_RECIPIENT_CERTIFICATE_DC,
			      ext);
		break;

	    case EXT_REDIRECTION_HISTORY:
	    case EXT_PHYSICAL_FORWARDING_ADDRESS:
	    case EXT_PROOF_OF_DELIVERY:
	    default:
		ep = flatten_ext (ext);
		for (epp = &rr -> rr_per_recip_extensions; *epp;
		     epp = &(*epp) -> ext_next)
			continue;
		*epp = ep;
		break;
	}		
}
