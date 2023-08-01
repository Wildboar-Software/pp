/* pptox400.c: convert PP structures to X.400 88 structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/pptox400.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/pptox400.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: pptox400.c,v $
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
#include "rtsparams.h"
#include "x400_ub.h"

#define STR2QB(s)	str2qb(s, strlen(s), 1)

static int build_envelope ();
static int build_msgid ();
int build_addr ();
static int build_addrdn ();
static int build_eits ();
static int build_content ();
static int build_pmf ();
static int build_time ();
static int build_recips ();
static int build_p3_recips ();
static int build_ri ();
static int build_p3_ri ();
static int build_trace ();
static int build_gdi ();
static int build_pm_extensions ();
static int build_prf_ext ();
static int build_drenvelope ();
static int build_drcontent ();
static int build_drc_pr_fields ();
static int build_lasttrace ();
static int build_report ();
static int build_fullname ();
static int add_eeit ();
static int setuberror ();
struct type_IOB_ORName *orn2orname();

static int trace_type;

static int build_probe_envelope(), build_dehl(), build_drc_extensions(),
	build_drc_pr_extensions(), build_dre_extensions();

char ub_error_string[BUFSIZ];
int ub_error_set;

int build_p1 (qp, adl, p1, tt)
Q_struct *qp;
ADDR	*adl;
struct type_Trans_MtsAPDU **p1;
int tt;
{
	ub_error_set = NOTOK;
	trace_type = tt;
	*p1 = (struct type_Trans_MtsAPDU *) smalloc (sizeof **p1);
	bzero ((char *)*p1, sizeof **p1);

	(*p1) -> offset = type_Trans_MtsAPDU_message;
	(*p1) -> un.message = (struct type_Trans_MessageAPDU *)
		smalloc (sizeof *(*p1) -> un.message);
	(*p1) -> un.message -> content = NULL;
	if (build_envelope (qp, adl, &(*p1) -> un.message -> envelope) == OK)
		return OK;

	free_Trans_MtsAPDU (p1);
	if (ub_error_set != NOTOK)
		return DONE;
	return NOTOK;
}

int build_dr (qp, recip, dr, qb, p1dr, tt)
Q_struct *qp;
ADDR	*recip;
DRmpdu	*dr;
struct type_MTA_Content *qb;
struct type_Trans_MtsAPDU **p1dr;
int tt;
{

	ub_error_set = NOTOK;
	trace_type = tt;
	*p1dr = (struct type_Trans_MtsAPDU *) smalloc (sizeof **p1dr);
	bzero ((char *)*p1dr, sizeof **p1dr);

	(*p1dr) -> offset = type_Trans_MtsAPDU_report;
	(*p1dr) -> un.report = (struct type_Trans_ReportAPDU *)
		smalloc (sizeof *(*p1dr) -> un.report);
	if (build_drenvelope (qp, dr,
			      &(*p1dr) -> un.report -> envelope) == OK &&
	    build_drcontent (qp, recip, dr, qb,
			     &(*p1dr) -> un.report -> content) == OK)
		return OK;
	free_Trans_MtsAPDU (p1dr);
	if (ub_error_set != NOTOK)
		return DONE;
	return NULL;
}


int build_probe (qp, recip, p1, tt)
Q_struct *qp;
ADDR	*recip;
struct type_Trans_MtsAPDU **p1;
int tt;
{
	ub_error_set = NOTOK;
	trace_type = tt;
	*p1 = (struct type_Trans_MtsAPDU *) smalloc (sizeof **p1);
	bzero ((char *)*p1, sizeof **p1);

	(*p1) -> offset = type_Trans_MtsAPDU_probe;
	if (build_probe_envelope (qp, recip, &(*p1) -> un.probe) == OK)
		return OK;

	free_Trans_MtsAPDU (*p1);
	if (ub_error_set != NOTOK)
		return DONE;
	return NOTOK;
}

static int build_envelope (qp, adl, envp)
Q_struct *qp;
ADDR	*adl;
struct type_MTA_MessageTransferEnvelope **envp;
{
	struct type_MTA_MessageTransferEnvelope *env;

	*envp = env = (struct type_MTA_MessageTransferEnvelope *)
		smalloc (sizeof *env);
	bzero ((char *)env, sizeof *env);

	if (build_msgid (&qp->msgid, &env -> message__identifier) == NOTOK)
		return NOTOK;

	if (build_addrdn (qp -> Oaddress -> ad_r400adr,
			qp -> Oaddress -> ad_dn,
			&env -> originator__name) == NOTOK)
		return NOTOK;


	if (build_eits (&qp -> encodedinfo,
			&env -> original__encoded__information__types) == NOTOK)
		return NOTOK;

	if (build_content (adl -> ad_content ?
			   adl -> ad_content :
			   qp -> cont_type,
			   &env -> member_MTA_13) == NOTOK)
		return NOTOK;


	if (qp -> ua_id) {
		if ((int) strlen(qp -> ua_id) > UB_CONTENT_ID_LENGTH)
			return setuberror ("Content Id", strlen(qp -> ua_id),
					   UB_CONTENT_ID_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		if ((env -> content__identifier =
		     STR2QB (qp -> ua_id)) == NULL)
			return NOTOK;
	}

	env -> priority = (struct type_MTA_Priority *)
		smalloc (sizeof *env -> priority);
	env -> priority -> parm = qp -> priority;

	if (build_pmf (qp,
		       &env -> per__message__indicators) == NOTOK)
		return NOTOK;

	if (qp -> defertime &&
	    build_time (qp -> defertime,
			&env -> deferred__delivery__time) == NOTOK)
		return NOTOK;

	env -> per__domain__bilateral__information = NULL;

	if (build_pm_extensions (qp,1,1,
				 &env -> extensions) == NOTOK)
		return NOTOK;

	if (build_trace (qp->trace,
			 &env -> trace__information) == NOTOK)
		return NOTOK;

	if (build_recips (qp -> disclose_recips ?
			  qp -> Raddress : adl, adl,
			  qp->disclose_recips,
			  &env -> per__recipient__fields) == NOTOK)
			  return NOTOK;

	return OK;
}




int build_p3_envelope (qp, envp, tt)
Q_struct *qp;
struct type_MTA_MessageSubmissionEnvelope **envp;
int	tt;
{
	struct type_MTA_MessageSubmissionEnvelope *env;

	trace_type = tt;
	*envp = env = (struct type_MTA_MessageSubmissionEnvelope *)
		smalloc (sizeof *env);
	bzero ((char *)env, sizeof *env);


	if (build_addrdn (qp -> Oaddress -> ad_r400adr,
			qp -> Oaddress -> ad_dn,
			&env -> originator__name) == NOTOK)
		return NOTOK;


	if (build_eits (&qp -> orig_encodedinfo,
			&env -> original__eits) == NOTOK)
		return NOTOK;

	if (build_content (qp -> cont_type,
			   &env -> member_MTA_6) == NOTOK)
		return NOTOK;

	if (qp -> ua_id) {
		if ((int) strlen(qp -> ua_id) > UB_CONTENT_ID_LENGTH)
			return setuberror ("Content Id", strlen(qp -> ua_id),
					   UB_CONTENT_ID_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		if ((env -> content__identifier =
		     STR2QB (qp -> ua_id)) == NULL)
			return NOTOK;
	}
	env -> priority = (struct type_MTA_Priority *)
		smalloc (sizeof *env -> priority);

	env -> priority -> parm =
		qp -> priority ? qp -> priority : PRIO_NORMAL;

	if (build_pmf (qp,
		       &env -> per__message__indicators) == NOTOK)
		return NOTOK;

	if (qp -> defertime &&
	    build_time (qp -> defertime,
			&env -> deferred__delivery__time) == NOTOK)
		return NOTOK;

	if (build_pm_extensions (qp,0,1,
				 &env -> extensions) == NOTOK)
		return NOTOK;

	if (build_p3_recips (qp -> Raddress,
			     &env -> per__recipient__fields) == NOTOK)
		return NOTOK;

	return OK;
}


static int build_msgid (mid, p1mp)
MPDUid *mid;
struct type_MTA_MTSIdentifier **p1mp;
{
	struct type_MTA_MTSIdentifier *p1msgid;

	*p1mp = p1msgid = (struct type_MTA_MTSIdentifier *)
		smalloc (sizeof *p1msgid);
	bzero ((char *)p1msgid, sizeof *p1msgid);

	if (mid -> mpduid_string) {
		if ((int) strlen(mid -> mpduid_string) > UB_LOCAL_ID_LENGTH)
			return setuberror ("LocalId",
					   strlen(mid -> mpduid_string),
					   UB_LOCAL_ID_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		if ((p1msgid -> local__identifier =
		     STR2QB (mid -> mpduid_string)) == NULL)
			return NOTOK;
	}
	if (build_gdi (&mid -> mpduid_DomId,
		       &p1msgid -> global__domain__identifier) == NOTOK)
		return NOTOK;
	return OK;
}

static int build_addrdn (cp, dn, ornp)
char	*cp;
char *dn;
struct type_MTA_ORName **ornp;
{
	OR_ptr or;
	ORName *orn;
	struct type_IOB_ORName *orname;

	orn = (ORName *) calloc (1, sizeof *orn);
	if ((or = or_std2or (cp)) == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't convert %s", cp));
		ORName_free (orn);
		return NOTOK;
	}
	orn -> on_or = or;
	if (dn)
		orn -> on_dn = str2dn (dn);
	orname = orn2orname (orn);
	ORName_free (orn);
	*ornp = (struct type_MTA_ORName *)orname;
	return  OK;
}

int build_addr (cp, orap)
char	*cp;
struct type_MTA_ORAddress **orap;
{
	OR_ptr or;
	extern struct type_MTA_ORAddress *ora2oradr();

	if ((or = or_std2or (cp)) == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't convert %s", cp));
		or_free (or);
		return NOTOK;
	}
	*orap = ora2oradr (or);
	or_free (or);
	return  OK;
}


static int build_eits (eit, p1eitp)
EncodedIT *eit;
struct type_MTA_EncodedInformationTypes **p1eitp;
{
	struct type_MTA_EncodedInformationTypes *p1eit;
	LIST_BPT				*ep;
	int					n;
	PE					pe;


	if (eit == NULL) return NULL;

	if ((pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_BITS))
	    == NULLPE)
		return NOTOK;

	*p1eitp = p1eit = (struct type_MTA_EncodedInformationTypes *)
		smalloc (sizeof *p1eit);
	bzero ((char *)p1eit, sizeof *p1eit);
	p1eit -> built__in__encoded__information__types = NULL;
	p1eit -> external__encoded__information__types = NULL;


	for (ep = eit -> eit_types; ep; ep = ep -> li_next) {
		switch (n = bodypart2value (ep -> li_name)) {
		case NOTOK:
			/* -- unrecognised -- */
			break;
		case 10:
			/* -- object id -- */
			if (add_eeit (&p1eit->external__encoded__information__types,
				      (ep -> li_name + strlen("oid."))) == NOTOK)
				return NOTOK;
			break;
		default:
			if (n > UB_BUILT_IN_ENCODED_INFORMATION_TYPES)
				return setuberror ("BuiltInEit",
						   n, UB_BUILT_IN_ENCODED_INFORMATION_TYPES,
						   DRD_SIZE_CONSTRAINT_VIOLATION);
			/* -- recognised eit -- */
			if (bit_on(pe,n) == NOTOK)
				return NOTOK;
			break;	
		}
	}

	p1eit -> built__in__encoded__information__types = pe;

	return OK;
}



static int add_eeit (eep, oidn)
struct type_MTA_ExternalEncodedInformationTypes **eep;
char	*oidn;
{
	int n;
	if (*eep == NULL) {
		*eep = (struct type_MTA_ExternalEncodedInformationTypes *)
			calloc (1, sizeof **eep);
		(*eep) -> ExternalEncodedInformationType = str2oid (oidn);
	}
	else {
		struct type_MTA_ExternalEncodedInformationTypes *ep;

		for (n = 0, ep = *eep; ep -> next; ep = ep -> next, n++)
			continue;
		if (n > UB_ENCODED_INFORMATION_TYPES)
			return setuberror ("EncodedInfoTypes",
					   n, UB_ENCODED_INFORMATION_TYPES,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		ep -> next = (struct type_MTA_ExternalEncodedInformationTypes *)
			calloc (1, sizeof *ep);
		ep -> next -> ExternalEncodedInformationType =
			str2oid (oidn);
	}
	return OK;
}

static int build_content (str, bictp)
char	*str;
struct type_MTA_ContentType **bictp;
{
	struct type_MTA_ContentType *bict;

	*bictp = bict = (struct type_MTA_ContentType *)
		smalloc (sizeof *bict);

	bzero ((char *)bict, sizeof *bict);
	if (lexnequ (str, "oid.", 4) == 0) {
		bict -> offset = type_MTA_ContentType_external;
		bict -> un.external = str2oid(str+4);
		return OK;
	}
	bict -> offset = type_MTA_ContentType_built__in;
	bict -> un.built__in = (struct type_MTA_BuiltInContentType *)
		smalloc (sizeof *bict->un.built__in);
	if (lexequ (str, "p2") == 0)
		bict -> un.built__in -> parm =
			int_MTA_BuiltInContentType_interpersonal__messaging__1984;
	else if (lexequ (str, "p22") == 0)
		bict -> un.built__in -> parm =
			int_MTA_BuiltInContentType_interpersonal__messaging__1988;
	else if (lexequ (str, "external") == 0)
		bict -> un.built__in -> parm =
			int_MTA_BuiltInContentType_external;
	else
		bict -> un.built__in -> parm =
			int_MTA_BuiltInContentType_unidentified;

	return OK;
}

static int build_pmf (qp, pep)
Q_struct *qp;
PE	*pep;
{
	PE pe;

	if ((pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_BITS))
	    == NULLPE)
		return NOTOK;
	*pep = pe;

	if (qp -> disclose_recips &&
	    bit_on(pe, bit_MTA_PerMessageIndicators_disclosure__of__recipients)
	    == NOTOK)
		return NOTOK;
	if (qp -> implicit_conversion_prohibited &&
	    bit_on(pe, bit_MTA_PerMessageIndicators_implicit__conversion__prohibited)
	    == NOTOK)
		return NOTOK;
	if (qp -> alternate_recip_allowed &&
	    bit_on(pe, bit_MTA_PerMessageIndicators_alternate__recipient__allowed)
	    == NOTOK)
		return NOTOK;
	if (qp -> content_return_request &&
	    bit_on(pe, bit_MTA_PerMessageIndicators_content__return__request)
	    == NOTOK)
		return NOTOK;

	return OK;
}

static int build_time (utc, utp)
UTC	utc;
struct type_UNIV_UTCTime **utp;
{
	char	*str;

	str = utct2str (utc);
	
	if((*utp = STR2QB (str)) == NULL)
		return NOTOK;
	return OK;
}

static int build_recips (ad, adl, disclose, rlp)
ADDR	*ad;
ADDR	*adl;
int	disclose;
struct element_MTA_4 **rlp;
{
	struct element_MTA_4 *rl = NULL;
	struct type_MTA_PerRecipientMessageTransferFields *prmtf;
	ADDR	*ap, *ap2;
	int	resp;
	int 	naddr = 0;

	*rlp = NULL;
	for (ap = ad; ap; ap = ap -> ad_next) {
		for (ap2 = adl; ap2; ap2 = ap2 -> ad_next) {
			if (ap2 -> ad_no == ap -> ad_no)
				break;
		}
		if (ap2 == NULLADDR)
			resp = FALSE;
		else	
			resp = ap2 -> ad_resp;
				

		if (*rlp == NULL) 
			*rlp = rl = (struct element_MTA_4 *)
				smalloc (sizeof *rl);
		else {
			rl -> next = (struct element_MTA_4 *)
				smalloc (sizeof *rl);
			rl = rl -> next;
		}
		bzero ((char *)rl, sizeof *rl);
		prmtf = rl -> PerRecipientMessageTransferFields =
			(struct type_MTA_PerRecipientMessageTransferFields *)
			smalloc (sizeof *prmtf);
		bzero ((char *)prmtf, sizeof *prmtf);

		if (build_addrdn (ap -> ad_r400adr,
				ap -> ad_dn,
				&prmtf -> recipient__name) == NOTOK)
			return NOTOK;

		prmtf -> originally__specified__recipient__number =
			(struct type_MTA_OriginallySpecifiedRecipientNumber *)
			smalloc (sizeof *prmtf ->
				 originally__specified__recipient__number);
		prmtf -> originally__specified__recipient__number -> parm =
			ap -> ad_extension;

		if (ap -> ad_explicitconversion != AD_EXP_NONE) {
			if (ap -> ad_explicitconversion > UB_INTEGER_OPTIONS)
				return setuberror("ExplicitConversion", 
						  ap -> ad_explicitconversion,
						  UB_INTEGER_OPTIONS,
						  DRD_SIZE_CONSTRAINT_VIOLATION);
			prmtf -> explicit__conversion =

				(struct type_MTA_ExplicitConversion *)
					smalloc (sizeof *prmtf -> explicit__conversion);
			prmtf -> explicit__conversion -> parm =
				ap -> ad_explicitconversion;
		}
		else prmtf -> explicit__conversion = NULL;

		if (build_ri (resp,
			      ap -> ad_mtarreq,
			      ap -> ad_usrreq,
			      &prmtf -> per__recipient__indicators) == NOTOK)
			return NOTOK;

		if (build_prf_ext (ap,1,1,
				   &prmtf -> extensions) == NOTOK)
			return NOTOK;
		if (naddr++ > UB_RECIPIENTS)
			return setuberror("Max number of recipients", naddr,
					  UB_RECIPIENTS, DRD_TOO_MANY_RECIPIENTS);
	}
	return OK;
}


static int build_p3_recips (ad, rlp)
ADDR	*ad;
struct element_MTA_0 **rlp;
{
	struct element_MTA_0 *rl = NULL;
	struct type_MTA_PerRecipientMessageSubmissionFields *prmtf;
	ADDR	*ap;
	int naddr = 0;

	*rlp = NULL;
	for (ap = ad; ap; ap = ap -> ad_next) {

		if (*rlp == NULL) 
			*rlp = rl = (struct element_MTA_0 *)
				smalloc (sizeof *rl);
		else {
			rl -> next = (struct element_MTA_0 *)
				smalloc (sizeof *rl);
			rl = rl -> next;
		}
		bzero ((char *)rl, sizeof *rl);
		prmtf = rl -> PerRecipientMessageSubmissionFields =
			(struct type_MTA_PerRecipientMessageSubmissionFields *)
			smalloc (sizeof *prmtf);
		if (build_addrdn (ap -> ad_r400adr, ap -> ad_dn,
				&prmtf -> recipient__name) == NOTOK)
			return NOTOK;

		if (ap -> ad_explicitconversion != AD_EXP_NONE) {
			prmtf -> explicit__conversion =
				(struct type_MTA_ExplicitConversion *)
					smalloc (sizeof *prmtf -> explicit__conversion);
			prmtf -> explicit__conversion -> parm =
				ap -> ad_explicitconversion;
		}
		else prmtf -> explicit__conversion = NULL;

		if (build_p3_ri (ap -> ad_usrreq,
				 &prmtf	 -> originator__report__request)
		    == NOTOK)
			return NOTOK;
		if (build_prf_ext (ap,0,1,
				   &prmtf -> extensions) == NOTOK)
			return NOTOK;
		if (naddr++ > UB_RECIPIENTS)
			return setuberror("Max number of recipients", naddr,
					  UB_RECIPIENTS,
					  DRD_TOO_MANY_RECIPIENTS);
	}
	return OK;
}

static int build_ri (resp, mta, usr, pep)
int	resp, mta, usr;
PE	*pep;
{
	PE	pe;

	if ((pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_BITS))
	    == NULLPE)
		return NOTOK;
	*pep = pe;
	if (resp && bit_on(pe,
			   bit_MTA_PerRecipientIndicators_responsibility) == NOTOK)
		return NOTOK;

	switch (mta) {
	    case AD_MTA_NONE:
		break;
	    case AD_MTA_BASIC:
		if (bit_on(pe,
			   bit_MTA_PerRecipientIndicators_originating__MTA__non__delivery__report) == NOTOK)
			return NOTOK;
		break;
	    case AD_MTA_CONFIRM:
		if (bit_on(pe,
			bit_MTA_PerRecipientIndicators_originating__MTA__report) == NOTOK)
			return NOTOK;
		break;
	    case AD_MTA_AUDIT_CONFIRM:
		if (bit_on(pe,
			   bit_MTA_PerRecipientIndicators_originating__MTA__non__delivery__report) == NOTOK ||
		    bit_on(pe,
			   bit_MTA_PerRecipientIndicators_originating__MTA__report) == NOTOK)
			return NOTOK;
		break;
	}

	switch (usr) {
	    case AD_USR_NOREPORT:
		break;
	    case AD_USR_BASIC:
		if (bit_on(pe,
			   bit_MTA_PerRecipientIndicators_originator__non__delivery__report) == NOTOK)
			return NOTOK;
		break;
	    case AD_USR_CONFIRM:
		if (bit_on(pe,
			bit_MTA_PerRecipientIndicators_originator__report) == NOTOK)
			return NOTOK;
		break;
	}
	if (bit_test (pe, 8) == NOTOK)
		(void) bit_off (pe, 8);
	return OK;
}


static int build_p3_ri (usr, pep)
int     usr;
PE	*pep;
{
	PE      pe;

	if ((pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_BITS))
	    == NULLPE)
		return NOTOK;
	*pep = pe;

	switch (usr) {
		case AD_USR_NOREPORT:
		case AD_USR_BASIC:
			if (bit_on(pe,
				   bit_MTA_OriginatorReportRequest_non__delivery__report) == NOTOK)
				return NOTOK;
			break;
		case AD_USR_CONFIRM:
			if (bit_on(pe,bit_MTA_OriginatorReportRequest_report)
			    == NOTOK)
				return NOTOK;
			break;
	}
	return OK;
}

static int build_traceelement (tp, tiep)
Trace	*tp;
struct type_MTA_TraceInformationElement **tiep;
{
	struct type_MTA_TraceInformationElement *tie;
	struct type_MTA_DomainSuppliedInformation *dsi;
	DomSupInfo *dsp;

	*tiep = tie = (struct type_MTA_TraceInformationElement *)
		smalloc (sizeof *tie);
	bzero ((char *)tie, sizeof *tie);

	if (build_gdi (&tp -> trace_DomId,
		       &tie -> global__domain__identifier) == NOTOK)
		return NOTOK;

	dsp = &tp -> trace_DomSinfo;
	dsi = tie -> domain__supplied__information =
		(struct type_MTA_DomainSuppliedInformation *)
			smalloc (sizeof *tie ->
				 domain__supplied__information);
	bzero ((char *)dsi, sizeof *dsi);
	if (dsp -> dsi_time &&
	    build_time (dsp -> dsi_time,
			&dsi -> arrival__time) == NOTOK)
		return NOTOK;

	dsi -> routing__action =
		(struct type_MTA_RoutingAction *)
			smalloc (sizeof *dsi -> routing__action);
	if (dsp -> dsi_action)
		dsi -> routing__action -> parm = dsp -> dsi_action;
	else	dsi -> routing__action -> parm = 
		int_MTA_RoutingAction_relayed;

	if (dsp -> dsi_attempted_md.global_Country != NULLCP &&
	    dsp -> dsi_attempted_md.global_Admin != NULLCP)
		if (build_gdi (&dsp -> dsi_attempted_md,
			       &dsi -> attempted__domain) == NOTOK)
			return NOTOK;

	if (dsp -> dsi_deferred &&
	    build_time (dsp -> dsi_deferred,
			&dsi -> deferred__time) == NOTOK)
		return NOTOK;

	if (dsp -> dsi_converted.eit_types &&
	    build_eits (&dsp -> dsi_converted,
			&dsi -> converted__encoded__information__types) == NOTOK)
		return NOTOK;

	if (dsp -> dsi_other_actions) {
		if ((dsi -> other__actions =
		     pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM,
			       PE_PRIM_BITS)) == NULLPE)
			return NOTOK;
		if (dsp -> dsi_other_actions & ACTION_REDIRECTED &&
		    bit_on(dsi -> other__actions,
			   bit_MTA_OtherActions_redirected) == NOTOK)
			return NOTOK;
		if (dsp -> dsi_other_actions & ACTION_EXPANDED &&
		    bit_on(dsi -> other__actions,
			   bit_MTA_OtherActions_dl__operation) == NOTOK)
			return NOTOK;
	}
	return OK;
}

int same_prmd (tp1, tp2)
Trace	*tp1, *tp2;
{
	GlobalDomId *d1, *d2;

	d1 = &tp1 -> trace_DomId;
	d2 = &tp2 -> trace_DomId;

	if (d1 -> global_Country && d2 -> global_Country &&
	    lexequ (d1 -> global_Country, d2 -> global_Country) == 0 &&
	    d1 -> global_Admin && d2 -> global_Admin &&
	    lexequ (d1 -> global_Admin, d2 -> global_Admin) == 0) {
		if (d1 -> global_Private == NULL &&
		    d2 -> global_Private == NULL)
			return 1;
		if (d1 -> global_Private && d2 -> global_Private &&
		    lexequ (d1 -> global_Private, d2 -> global_Private) == 0)
			return 1;
	}
	return 0;
	    
}

static int build_trace (trp, tracep)
Trace	*trp;
struct type_MTA_TraceInformation **tracep;
{
	Trace	*tp, *tlast;
	struct type_MTA_TraceInformation *trace = NULL;
	int	ntrace = 0;

	*tracep = NULL;

	for (tlast = trp; tlast && tlast -> trace_next;
	     tlast = tlast -> trace_next)
		continue;

	if (trace_type == RTSP_TRACE_ADMD) {
		if (tlast != trp && same_prmd (tlast, trp))
			tp = trp;
		else	tp = tlast;
		*tracep = trace = (struct type_MTA_TraceInformation *)
			smalloc (sizeof *trace);
		trace -> next = NULL;

		if (build_traceelement (tp,
					&trace -> TraceInformationElement)
		    == NOTOK)
			return NOTOK;
		return OK;
	}
			
	for (tp = trp; tp; tp = tp -> trace_next) {
		if (*tracep == NULL)
			*tracep = trace =
				(struct type_MTA_TraceInformation *)
					smalloc (sizeof *trace);
		else {
			trace -> next =
				(struct type_MTA_TraceInformation *)
					smalloc (sizeof *trace);
			trace = trace -> next;
		}
		bzero ((char *)trace, sizeof *trace);

		if (build_traceelement(tp,
				       &trace -> TraceInformationElement)
		    == NOTOK)
			return NOTOK;
		if (++ntrace > UB_TRANSFERS)
			return setuberror ("Trace",
					   ntrace, UB_TRANSFERS,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
	}

	return OK;
}

static int build_gdi (gdi, p1gdip)
GlobalDomId *gdi;
struct type_MTA_GlobalDomainIdentifier **p1gdip;
{
	struct type_MTA_GlobalDomainIdentifier *p1gdi;
	struct type_MTA_AdministrationDomainName *admd;
	struct type_MTA_CountryName *co;
	struct type_MTA_PrivateDomainIdentifier *prmd;

	*p1gdip = p1gdi = (struct type_MTA_GlobalDomainIdentifier *)
		smalloc (sizeof *p1gdi);
	bzero ((char *)p1gdi, sizeof *p1gdi);
	
	if (gdi -> global_Country) {
		p1gdi -> country__name = co =
			(struct type_MTA_CountryName *)
				smalloc (sizeof *co);
		if (or_str_isns(gdi -> global_Country)) {
			if ((int) strlen(gdi -> global_Country) >
			    UB_COUNTRY_NAME_NUMERIC_LENGTH)
				return setuberror ("NumericCountryLength",
						   strlen(gdi -> global_Country),
						   UB_COUNTRY_NAME_NUMERIC_LENGTH,
						   DRD_SIZE_CONSTRAINT_VIOLATION);
			co -> offset = type_MTA_CountryName_x121__dcc__code;
			if ((co -> un.x121__dcc__code =
			     STR2QB (gdi -> global_Country)) == NULL)
				return NOTOK;
		} else {
			if ((int) strlen(gdi -> global_Country) >
			    UB_COUNTRY_NAME_ALPHA_LENGTH)
				return setuberror ("Country",
						   strlen(gdi -> global_Country),
						   UB_COUNTRY_NAME_ALPHA_LENGTH,
						   DRD_SIZE_CONSTRAINT_VIOLATION);
			co -> offset = type_MTA_CountryName_iso__3166__alpha2__code;
			if ((co -> un.iso__3166__alpha2__code =
			     STR2QB (gdi -> global_Country)) == NULL)
				return NOTOK;
		}
	}
	else {
		PP_LOG (LLOG_EXCEPTIONS, ("Country missing from GDI"));
		return NOTOK;
	}

	if (gdi -> global_Admin) {
		if ((int) strlen(gdi -> global_Admin) > UB_DOMAIN_NAME_LENGTH)
			return setuberror ("ADMD", strlen(gdi -> global_Admin),
					   UB_DOMAIN_NAME_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		p1gdi -> administration__domain__name = admd =
			(struct type_MTA_AdministrationDomainName *)
				smalloc (sizeof *admd);

		if (or_str_isns (gdi -> global_Admin)) {
			admd -> offset = type_MTA_AdministrationDomainName_numeric;
			if ((admd -> un.numeric =
			     STR2QB(gdi -> global_Admin)) == NULL)
				return NOTOK;
		} else {
			admd -> offset = type_MTA_AdministrationDomainName_printable;
			if ((admd -> un.printable = STR2QB (gdi -> global_Admin))
			    == NULL)
				return NOTOK;
		}
	}
	else {
		PP_LOG (LLOG_EXCEPTIONS, ("ADMD missing from GDI"));
		return NOTOK;
	}

	if (gdi -> global_Private) {
		if ((int) strlen(gdi -> global_Private) > UB_DOMAIN_NAME_LENGTH)
			return setuberror ("PrivateDomainIdentifier",
					   strlen(gdi -> global_Private),
					   UB_DOMAIN_NAME_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		p1gdi -> private__domain__identifier = prmd =
			(struct type_MTA_PrivateDomainIdentifier *)
				smalloc (sizeof *prmd);
		if (or_str_isns (gdi -> global_Private)) {
			prmd -> offset = type_MTA_PrivateDomainIdentifier_numeric;
			if ((prmd -> un.numeric = STR2QB (gdi -> global_Private))
			    == NULL)
				return NOTOK;
		} else {
			prmd -> offset = type_MTA_PrivateDomainIdentifier_printable;
			if ((prmd -> un.printable = STR2QB (gdi -> global_Private))
			    == NULL)
				return NOTOK;
		}
	}

	return OK;
}


static int build_drenvelope (qp, dr, envp)
Q_struct *qp;
DRmpdu	*dr;
struct type_MTA_ReportTransferEnvelope **envp;
{
	struct type_MTA_ReportTransferEnvelope *env;

	*envp = env = (struct type_MTA_ReportTransferEnvelope *)
		smalloc (sizeof *env);
	bzero ((char *)env, sizeof *env);


	if (build_msgid (dr -> dr_mpduid, &env -> report__identifier) == NOTOK)
		return NOTOK;

	if (build_addrdn (qp -> Oaddress -> ad_r400adr,
			qp -> Oaddress -> ad_dn,
			&env -> report__destination__name) == NOTOK)
		return NOTOK;

	if (build_trace (dr -> dr_trace,
			 &env -> trace__information) == NOTOK)
		return NOTOK;

	if (build_dre_extensions (dr, &env -> extensions) == NOTOK)
		return NOTOK;

	return OK;
}

static int build_drcontent (qp, recip, dr, qb, drcp)
Q_struct *qp;
ADDR	*recip;
DRmpdu	*dr;
struct type_MTA_Content *qb;
struct type_MTA_ReportTransferContent **drcp;
{
	struct type_MTA_ReportTransferContent *drc;

	*drcp = drc = (struct type_MTA_ReportTransferContent *)
		smalloc (sizeof *drc);

	bzero ((char *)drc, sizeof *drc);


	if (build_msgid (&qp -> msgid, &drc -> subject__identifier) == NOTOK)
		return NOTOK;


	if (build_trace (dr -> dr_subject_intermediate_trace,
			 &drc -> subject__intermediate__trace__information) == NOTOK)
		return NOTOK;

	if (build_eits (&qp -> encodedinfo,
			&drc -> original__encoded__information__types) == NOTOK)
		return NOTOK;

	if ((qp -> cont_type || recip -> ad_content) &&
	    build_content (qp -> cont_type,
			   &drc -> member_MTA_17) == NOTOK)
		return NOTOK;

	if (qp -> ua_id) {
		if ((int) strlen(qp -> ua_id) > UB_CONTENT_ID_LENGTH)
			return setuberror ("Content ID", qp -> msgsize,
					   UB_CONTENT_ID_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		if ((drc -> content__identifier =
		     STR2QB (qp -> ua_id)) == NULL)
		return NOTOK;
	}
	drc -> returned__content = qb;

	drc -> additional__information = NULLPE;

	if (build_drc_extensions (qp, recip, dr,
			   &drc -> extensions) == NOTOK)
		return NOTOK;


	if (build_drc_pr_fields (qp, recip, dr -> dr_recip_list,
				 &drc -> per__recipient__fields) == NOTOK)
		return NOTOK;

	return OK;
}

static int build_drc_pr_fields (qp, recip, rr, prp)
Q_struct *qp;
ADDR	*recip;
Rrinfo	*rr;
struct element_MTA_9 **prp;
{
	struct element_MTA_9 *prf;
	struct type_MTA_PerRecipientReportTransferFields *pr;
	ADDR	*ap;

	if (rr == NULL)
		return NULL;
	for (ap = qp -> Raddress; ap; ap = ap -> ad_next)
		if (ap -> ad_no == rr -> rr_recip)
			break;
	if (ap == NULLADDR)
		return build_drc_pr_fields (qp, recip, rr -> rr_next, prp);

	*prp = prf = (struct element_MTA_9 *) smalloc (sizeof *prf);

	prf -> next = NULL;
	prf -> PerRecipientReportTransferFields = pr =
		(struct type_MTA_PerRecipientReportTransferFields *)
			smalloc (sizeof *pr);
	bzero ((char *)pr, sizeof *pr);

	if (build_addrdn (ap -> ad_r400adr, ap -> ad_dn,
			&pr -> actual__recipient__name) == NOTOK)
		return NOTOK;

	pr -> originally__specified__recipient__number =
		(struct type_MTA_OriginallySpecifiedRecipientNumber *)
			smalloc (sizeof *pr ->
				 originally__specified__recipient__number);
	pr -> originally__specified__recipient__number -> parm =
		ap -> ad_extension;

	if (build_ri (ap -> ad_resp,
		      ap -> ad_mtarreq,
		      ap -> ad_usrreq,
		      &pr -> per__recipient__indicators) == NOTOK)
		return NOTOK;

	if (build_lasttrace (rr,
			     &pr -> last__trace__information) == NOTOK)
		return NOTOK;

	if (rr -> rr_originally_intended_recip &&
	    build_fullname (rr -> rr_originally_intended_recip,
			    &pr -> originally__intended__recipient__name) == NOTOK)
		return NOTOK;

	if (rr -> rr_supplementary) {
		if ((int) strlen(rr -> rr_supplementary) >
		    UB_SUPPLEMENTARY_INFO_LENGTH)
			return setuberror ("Supplementary Information",
					   strlen(rr -> rr_supplementary),
					   UB_SUPPLEMENTARY_INFO_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
			
		if ((pr -> supplementary__information =
		     STR2QB (rr -> rr_supplementary)) == NULL)
			return NOTOK;
	}
	if (build_drc_pr_extensions (qp, rr, &pr -> extensions) == NOTOK)
		return NOTOK;

	if (build_drc_pr_fields (qp, recip, rr -> rr_next,
				 &prf -> next) == NOTOK)
		return NOTOK;

	return OK;
}

static int build_lasttrace (rr, ltip)
Rrinfo *rr;
struct type_MTA_LastTraceInformation **ltip;
{
	struct type_MTA_LastTraceInformation *lti;

	*ltip = lti = (struct type_MTA_LastTraceInformation *)
		smalloc (sizeof *lti);
	bzero ((char *)lti, sizeof *lti);

	if (build_time (rr -> rr_arrival,
			&lti -> arrival__time) == NOTOK)
		return NOTOK;

	if (rr -> rr_converted &&
	    build_eits (rr -> rr_converted,
			&lti -> converted__encoded__information__types) == NOTOK)
		return NOTOK;

	if (build_report (&rr -> rr_report,
			  &lti -> report) == NOTOK)
		return NOTOK;

	return OK;
}

static int build_report (rp, repp)
Report *rp;
struct type_MTA_Report **repp;
{
	struct type_MTA_Report *rep;

	*repp = rep = (struct type_MTA_Report *)
		smalloc (sizeof *rep);

	if (rp -> rep_type == DR_REP_SUCCESS) {
		struct type_MTA_DeliveryReport *drp;

		rep -> offset = type_MTA_Report_delivery;

		rep -> un.delivery = drp =
			(struct type_MTA_DeliveryReport *)
				smalloc (sizeof *drp);

		if (build_time (rp->rep.rep_dinfo.del_time,
				&drp -> message__delivery__time) == NOTOK)
			return NOTOK;

		if (rp -> rep.rep_dinfo.del_type > UB_MTS_USER_TYPES)
			return setuberror ("TypeOfMtsUser",
				    rp -> rep.rep_dinfo.del_type,
				    UB_MTS_USER_TYPES,
				    DRD_SIZE_CONSTRAINT_VIOLATION);
		drp -> type__of__MTS__user =
			(struct type_MTA_TypeOfMTSUser *)
				smalloc (sizeof *drp -> type__of__MTS__user);
		drp -> type__of__MTS__user -> parm =
			rp -> rep.rep_dinfo.del_type;
	}
	else if (rp -> rep_type == DR_REP_FAILURE) {
		struct type_MTA_NonDeliveryReport *ndr;

		rep -> offset = type_MTA_Report_non__delivery;

		rep -> un.non__delivery = ndr =
			(struct type_MTA_NonDeliveryReport *)
				smalloc (sizeof *ndr);
		if (rp -> rep.rep_ndinfo.nd_rcode > UB_REASON_CODES) 
			return setuberror ("Reason Code",
					   rp -> rep.rep_ndinfo.nd_rcode,
					   UB_REASON_CODES,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		ndr -> non__delivery__reason__code =
			(struct type_MTA_NonDeliveryReasonCode *)
				smalloc (sizeof *ndr -> non__delivery__reason__code);
		ndr -> non__delivery__reason__code -> parm =
			rp -> rep.rep_ndinfo.nd_rcode;

		if (rp -> rep.rep_ndinfo.nd_dcode > UB_DIAGNOSTIC_CODES)
			return setuberror ("Diagnostic code",
					   rp -> rep.rep_ndinfo.nd_dcode,
					   UB_DIAGNOSTIC_CODES,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		ndr -> non__delivery__diagnostic__code =
			(struct type_MTA_NonDeliveryDiagnosticCode *)
				smalloc (sizeof *ndr -> non__delivery__diagnostic__code);
		ndr -> non__delivery__diagnostic__code -> parm =
			rp -> rep.rep_ndinfo.nd_dcode;
	}
	else {
		PP_LOG (LLOG_EXCEPTIONS, ("Bad delivery report type %d",
					  rp -> rep_type));
		return NOTOK;
	}

	return OK;
}

static int build_fullname (fn, ornp)
FullName	*fn;
struct type_MTA_ORName **ornp;
{
	if (build_addrdn (fn -> fn_addr, fn -> fn_dn, ornp) == NOTOK)
		return NOTOK;
	return OK;
}


static int build_prb_recips (ad, adl, disclose, rlp)
ADDR	*ad;
ADDR	*adl;
int	disclose;
struct element_MTA_7 **rlp;
{
	struct element_MTA_7 *rl = NULL;
	struct type_MTA_PerRecipientProbeTransferFields *prmtf;
	ADDR	*ap, *ap2;
	int	resp;
	int naddr = 0;

	*rlp = NULL;
	for (ap = ad; ap; ap = ap -> ad_next) {
		for (ap2 = adl; ap2; ap2 = ap2 -> ad_next) {
			if (ap2 -> ad_no == ap -> ad_no)
				break;
		}
		if (ap2 == NULLADDR)
			resp = FALSE;
		else
			resp = ap2 -> ad_resp;
				

		if (*rlp == NULL) 
			*rlp = rl = (struct element_MTA_7 *)
				smalloc (sizeof *rl);
		else {
			rl -> next = (struct element_MTA_7 *)
				smalloc (sizeof *rl);
			rl = rl -> next;
		}
		bzero ((char *)rl, sizeof *rl);
		prmtf = rl -> PerRecipientProbeTransferFields =
			(struct type_MTA_PerRecipientProbeTransferFields *)
			smalloc (sizeof *prmtf);

		if (build_addrdn (ap -> ad_r400adr, ap -> ad_dn,
				&prmtf -> recipient__name) == NOTOK)
			return NOTOK;

		prmtf -> originally__specified__recipient__number =
			(struct type_MTA_OriginallySpecifiedRecipientNumber *)
			smalloc (sizeof *prmtf ->
				 originally__specified__recipient__number);
		prmtf -> originally__specified__recipient__number -> parm =
			ap -> ad_extension;

		if (ap -> ad_explicitconversion != AD_EXP_NONE) {
			prmtf -> explicit__conversion =
				(struct type_MTA_ExplicitConversion *)
					smalloc (sizeof *prmtf -> explicit__conversion);
			prmtf -> explicit__conversion -> parm =
				ap -> ad_explicitconversion;
		}
		else prmtf -> explicit__conversion = NULL;

		if (build_ri (resp,
			      ap -> ad_mtarreq,
			      ap -> ad_usrreq,
			      &prmtf -> per__recipient__indicators) == NOTOK)
			return NOTOK;
		if (build_prf_ext (ap,1,0,
				   &prmtf -> extensions) == NOTOK)
			return NOTOK;
		if (naddr++ > UB_RECIPIENTS)
			return setuberror("Max number of recipients", naddr,
					  UB_RECIPIENTS,
					  DRD_TOO_MANY_RECIPIENTS);
	}
	return OK;
}

static int build_p3_prb_recips (ad, rlp)
ADDR	*ad;
struct element_MTA_1 **rlp;
{
	struct element_MTA_1 *rl = NULL;
	struct type_MTA_PerRecipientProbeSubmissionFields *prmtf;
	ADDR	*ap;

	*rlp = NULL;
	for (ap = ad; ap; ap = ap -> ad_next) {

		if (*rlp == NULL) 
			*rlp = rl = (struct element_MTA_1 *)
				smalloc (sizeof *rl);
		else {
			rl -> next = (struct element_MTA_1 *)
				smalloc (sizeof *rl);
			rl = rl -> next;
		}
		bzero ((char *)rl, sizeof *rl);
		prmtf = rl -> PerRecipientProbeSubmissionFields =
			(struct type_MTA_PerRecipientProbeSubmissionFields *)
			smalloc (sizeof *prmtf);

		if (build_addrdn (ap -> ad_r400adr, ap -> ad_dn,
				&prmtf -> recipient__name) == NOTOK)
			return NOTOK;

		if (ap -> ad_explicitconversion != AD_EXP_NONE) {
			prmtf -> explicit__conversion =
				(struct type_MTA_ExplicitConversion *)
					smalloc (sizeof *prmtf -> explicit__conversion);
			prmtf -> explicit__conversion -> parm =
				ap -> ad_explicitconversion;
		}
		else prmtf -> explicit__conversion = NULL;

		if (build_p3_ri (ap -> ad_usrreq,
				 &prmtf	 -> originator__report__request) == NOTOK)
			return NOTOK;
		if (build_prf_ext (ap,0,0,
				   &prmtf -> extensions) == NOTOK)
			return NOTOK;
	}
	return OK;
}

static int build_probe_envelope (qp, adl, prbp)
Q_struct *qp;
ADDR *adl;
struct type_MTA_ProbeTransferEnvelope **prbp;
{
	struct type_MTA_ProbeTransferEnvelope *prb;

	*prbp = prb = (struct type_MTA_ProbeTransferEnvelope *)
		smalloc (sizeof *prb);
	bzero ((char *)prb, sizeof *prb);

	if (build_msgid (&qp -> msgid, &prb -> probe__identifier ) == NOTOK)
		return NOTOK;


	if (build_addrdn (qp -> Oaddress -> ad_r400adr,
			  qp -> Oaddress -> ad_dn,
			  &prb -> originator__name) == NOTOK)
		return NOTOK;

	if (build_eits (&qp -> encodedinfo,
			&prb -> original__encoded__information__types) == NOTOK)
		return NOTOK;

	if (build_content (adl -> ad_content ?
			   adl -> ad_content :
			   qp -> cont_type,
			   &prb -> member_MTA_15) == NOTOK)
		return NOTOK;

	if (qp -> ua_id) {
		if ((int) strlen(qp -> ua_id) > UB_CONTENT_ID_LENGTH)
			return setuberror ("Content Id", strlen(qp -> ua_id),
				    UB_CONTENT_ID_LENGTH,
				    DRD_SIZE_CONSTRAINT_VIOLATION);
		if ((prb -> content__identifier =
		     STR2QB (qp -> ua_id)) == NULL)
			return NOTOK;
	}
	prb -> content__length = (struct type_MTA_ContentLength *)
		smalloc (sizeof *prb -> content__length);
	if (qp -> msgsize > UB_CONTENT_LENGTH)
		return setuberror ("MessageSize", qp -> msgsize,
				   UB_CONTENT_LENGTH,
				   DRD_SIZE_CONSTRAINT_VIOLATION);

	prb -> content__length -> parm = qp -> msgsize;

	if (build_pmf (qp,
		       &prb -> per__message__indicators) == NOTOK)
		return NOTOK;

	prb -> per__domain__bilateral__information = NULL;

	if (build_trace (qp -> trace,
			 &prb -> trace__information) == NOTOK)
		return NOTOK;

	if (build_pm_extensions (qp,1,0,
				 &prb -> extensions) == NOTOK)
		return NOTOK;

	if (build_prb_recips (qp -> disclose_recips ? qp -> Raddress : adl,
			      adl, qp -> disclose_recips,
			      &prb -> per__recipient__fields) == NOTOK)
		return NOTOK;

	return OK;
}

int build_p3_probe_envelope (qp, prbp)
Q_struct *qp;
struct type_MTA_ProbeSubmissionEnvelope **prbp;
{
	struct type_MTA_ProbeSubmissionEnvelope *prb;

	*prbp = prb = (struct type_MTA_ProbeSubmissionEnvelope *)
		smalloc (sizeof *prb);
	bzero ((char *)prb, sizeof *prb);


	if (build_addrdn (qp -> Oaddress -> ad_r400adr,
			qp -> Oaddress -> ad_dn,
			&prb -> originator__name) == NOTOK)
		return NOTOK;

	if (build_eits (&qp -> orig_encodedinfo,
			&prb -> original__eits) == NOTOK)
		return NOTOK;

	if (build_content (qp -> cont_type,
			   &prb -> member_MTA_7) == NOTOK)
		return NOTOK;

	if (qp -> ua_id) {
		if ((int) strlen(qp -> ua_id) > UB_CONTENT_ID_LENGTH)
			return setuberror ("Content Id", strlen(qp -> ua_id),
					   UB_CONTENT_ID_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);

		if ((prb -> content__identifier =
		     STR2QB (qp -> ua_id)) == NULL)
		return NOTOK;
	}
	prb -> content__length = (struct type_MTA_ContentLength *)
		smalloc (sizeof *prb -> content__length);
	prb -> content__length -> parm = qp -> msgsize;

	if (build_pmf (qp,
		       &prb -> per__message__indicators) == NOTOK)
		return NOTOK;

	if (build_pm_extensions (qp,0,0,
				 &prb -> extensions) == NOTOK)
		return NOTOK;

	if (build_p3_prb_recips (qp -> Raddress,
				 &prb -> per__recipient__fields) == NOTOK)
		return NOTOK;

	return OK;
}


/*  EXTENSIONS */

#define wrap_up_ext(e, n)	\
	(e) = (struct type_MTA_Extensions *) smalloc (sizeof *(e)); \
	(e) -> next = NULL; (e) -> ExtensionField = (n);
#define STR2QB(s)	str2qb(s, strlen(s), 1)


static int build_ext_type (ext_int, ext_oid, typep)
int	ext_int;
OID	ext_oid;
struct type_MTA_ExtensionType **typep;
{
	struct type_MTA_ExtensionType *type;

	*typep = type = (struct type_MTA_ExtensionType *)
		smalloc (sizeof *type);

	if (ext_int == EXT_OID_FORM) {
		type -> offset = type_MTA_ExtensionType_local;
		if ((type -> un.local = oid_cpy (ext_oid)) == NULLOID)
			return NOTOK;
	} else {
		type -> offset = type_MTA_ExtensionType_global;
		if (ext_int > UB_EXTENSION_TYPES) 
			return setuberror ("Extension types",
					   ext_int, UB_EXTENSION_TYPES,
					   DRD_SIZE_CONSTRAINT_VIOLATION);

		type -> un.global = ext_int;
	}
	return OK;
}

static int build_crit (cr, crp)
int	cr;
struct type_MTA_Criticality **crp;
{
	char	*p;
	int nbits = 3;

	if (cr > (1<<UB_BIT_OPTIONS))
		return setuberror ("Criticality", cr, UB_BIT_OPTIONS,
				   DRD_SIZE_CONSTRAINT_VIOLATION);

	if (cr > 7) {
		nbits = UB_BIT_OPTIONS;
	}
	p = int2strb (cr, nbits);
	*crp = strb2bitstr (p, nbits, PE_CLASS_UNIV, PE_PRIM_BITS);
	if (*crp == NULL)
		return NOTOK;
	return OK;
}

static int	build_ext_value (qb, pep)
struct qbuf *qb;
PE	*pep;
{
	char	*cp;
	PS	ps;

	cp = qb2str(qb);

	if ((ps = ps_alloc (str_open)) == NULLPS) {
		PP_LOG (LLOG_EXCEPTIONS, ("PS alloc failed"));
		return NOTOK;
	}

	if (str_setup (ps, cp, qb -> qb_len, 1) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't setup PS stream"));
		return NOTOK;
	}

	if ((*pep = ps2pe (ps)) == NULLPE) {
		PP_LOG (LLOG_EXCEPTIONS, ("ps2pe failed [%s]",
					  ps_error (ps -> ps_errno)));
		return NOTOK;
	}
	ps_free (ps);
	return OK;
}	

static int build_ext_generic (parm, crit, defcrit, type, oid, mod_idx,
			      mod, fnx, label, extlp)
caddr_t parm;
int crit;
int type;
int defcrit;
OID	oid;
int mod_idx;
modtyp *mod;
IFP	fnx;
char *label;
struct type_MTA_Extensions **extlp;
{
	struct type_MTA_ExtensionField *ext;
	caddr_t value;

	ext = (struct type_MTA_ExtensionField *)
		smalloc (sizeof *ext);
	if (build_ext_type (type, oid, &ext -> type) == NOTOK)
		return NOTOK;
	if (crit != defcrit) {
		if (build_crit (crit, &ext -> criticality) == NOTOK)
			return NOTOK;
	}
	else ext -> criticality = NULL;

	if ((*fnx) (&value, parm) == NOTOK)
		return NOTOK;

	if (enc_f (mod_idx, mod, &ext -> value, 1, 0, NULLCP, value)
	    == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't encode value %s: %s",
					  label, PY_pepy));
		return NOTOK;
	}

#if PP_DEBUG > 0
	if(pp_log_norm -> ll_events & LLOG_PDUS) 
		pvpdu (pp_log_norm, mod_idx, mod,
		       ext -> value, label, PDU_WRITE);
#endif
	fre_obj (value, mod->md_dtab[mod_idx], mod, 1);

	*extlp = (struct type_MTA_Extensions *) smalloc (sizeof **extlp);
	(*extlp) -> next = NULL;
	(*extlp) -> ExtensionField = ext;
	return OK;
}

static int build_ext_int (value, parm)
caddr_t *value;
caddr_t parm;
{
	int *ip = (int *)parm;
	struct type_Ext_RecipientReassignmentProhibited **val =
		(struct type_Ext_RecipientReassignmentProhibited **) value;

	*val = (struct type_Ext_RecipientReassignmentProhibited *)
		smalloc (sizeof **val);
	(*val) -> parm = *ip;
	return OK;
}

static int build_ext_char (value, parm)
caddr_t *value;
caddr_t parm;
{
	char *cp = (char *)parm;
	struct type_Ext_RecipientReassignmentProhibited **val =
		(struct type_Ext_RecipientReassignmentProhibited **) value;

	*val = (struct type_Ext_RecipientReassignmentProhibited *)
		smalloc (sizeof **val);
	(*val) -> parm = *cp;
	return OK;
}

static int build_ext_time (value, parm)
caddr_t	*value;
caddr_t parm;
{
	UTC utc = (UTC)parm;
	struct type_UNIV_UTCTime **ut = (struct type_UNIV_UTCTime **)value;
	return build_time (utc, ut);
}

static int build_ext_dlh (value, parm)
caddr_t *value;
caddr_t parm;
{
	DLHistory *dlp = (DLHistory *)parm;
	struct type_Ext_DLExpansionHistory **val =
		(struct type_Ext_DLExpansionHistory **)value;
	return build_dehl (dlp, val);
}

static int build_aext_oraa (value, parm)
caddr_t *value;
caddr_t parm;
{
	char *ora = (char *)parm;
	struct type_MTA_ORName **val = (struct type_MTA_ORName **)value;

	return build_addrdn (ora, NULLCP, val);
}

static int build_aext_rdm (value, parm)
caddr_t *value;
caddr_t parm;
{
	char *rdm = (char *)parm;
	struct type_Ext_RequestedDeliveryMethod **val =
		(struct type_Ext_RequestedDeliveryMethod **)value;
	int	i;

	*val = NULL;

	for (i = 0; i < AD_RDM_MAX && rdm[i] != AD_RDM_NOTUSED; i++) {
		*val = (struct type_Ext_RequestedDeliveryMethod *)
			smalloc (sizeof **val);
		(*val) -> element_Ext_0 = parm[i];
		(*val) -> next = NULL;
		val = &(*val) -> next;
	}

	return OK;
}
static int build_aext_pra (value, parm)
caddr_t *value;
caddr_t parm;
{
	struct type_Ext_PhysicalRenditionAttributes **val =
		(struct type_Ext_PhysicalRenditionAttributes **)value;
	OID pra = (OID)parm;

	if ((*val = oid_cpy (pra)) == NULLOID)
		return NOTOK;
	return OK;
}

static int build_aext_pdm (value, parm)
caddr_t *value;
caddr_t parm;
{
	int *pdm = (int *)parm;
	struct type_Ext_PhysicalDeliveryModes **val =
		(struct type_Ext_PhysicalDeliveryModes **)value;
	char	*p;

	p = int2strb (*pdm, 8);
	if ((*val = strb2bitstr (p, 8, PE_CLASS_UNIV, PE_PRIM_BITS)) == NULLPE)
		return NOTOK;
	return OK;
}

static int build_aext_rnfa (value, parm)
caddr_t *value;
caddr_t parm;
{
	char *rnfa = (char *)parm;
	struct type_Ext_RecipientNumberForAdvice **val =
		(struct type_Ext_RecipientNumberForAdvice **)value;;

	if ((int) strlen(rnfa) > UB_RECIPIENT_NUMBER_FOR_ADVICE_LENGTH)
		return setuberror ("RecipientNumberForAdvice",
			    strlen(rnfa),
			    UB_RECIPIENT_NUMBER_FOR_ADVICE_LENGTH,
			    DRD_SIZE_CONSTRAINT_VIOLATION);

	if ((*val = STR2QB (rnfa)) == NULL)
		return NOTOK;
	return OK;
}

static int build_aext_redir (value, parm)
caddr_t *value;
caddr_t parm;
{
	Redirection *redir = (Redirection *)parm;
	struct type_Ext_RedirectionHistory  **rp =
		(struct type_Ext_RedirectionHistory **)value;
	struct type_Ext_Redirection *rd;
	int nredir = 0;

	*rp = NULL;
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
		
		if (build_addrdn (redir -> rd_addr, redir -> rd_dn,
				&rd -> intended__recipient__name -> address) == NOTOK)
			return NOTOK;
		
		if (build_time (redir -> rd_time,
				&rd -> intended__recipient__name -> redirection__time) == NOTOK)
			return NOTOK;

		rd -> redirection__reason =
			(struct type_Ext_RedirectionReason *)
				smalloc (sizeof *rd -> redirection__reason);
		rd -> redirection__reason -> parm = redir -> rd_reason;
		
		if (++nredir > UB_REDIRECTIONS)
			return setuberror ("RedirectionHistory",
				    nredir, UB_REDIRECTIONS,
				    DRD_SIZE_CONSTRAINT_VIOLATION);
	}
	return OK;
}

static int build_ext_ora (value, parm)
caddr_t *value;
caddr_t parm;
{
	struct type_Ext_OriginatorReturnAddress **val =
		(struct type_Ext_OriginatorReturnAddress **)value;
	char *ora = (char *)parm;

	return build_addr (ora, val);
}

static int build_ext_fn (value, parm)
caddr_t *value;
caddr_t parm;
{
	FullName *fn = (FullName *)parm;
	struct type_MTA_ORName **orn =
		(struct type_MTA_ORName **)value;
	return build_fullname (fn, orn);
}

static int build_ext_secure (parm, crit, defcrit, type, oid,
			     modid, mod, label, extlp)
struct qbuf	*parm;
int	crit;
int defcrit;
int type;
OID oid;
int modid;
modtyp *mod;
char *label;
struct type_MTA_Extensions **extlp;
{
	struct type_MTA_ExtensionField *ext;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	if (build_ext_type (type, oid, &ext -> type) == NOTOK)
		return NOTOK;

	if (crit != defcrit) {
		if (build_crit (crit, &ext -> criticality) == NOTOK)
			return NOTOK;
	}
	else
		ext -> criticality = NULL;

	
	if (build_ext_value (parm, &ext -> value) == NOTOK)
		return NOTOK;

#if PP_DEBUG > 0
	if(pp_log_norm -> ll_events & LLOG_PDUS) 
		pvpdu (pp_log_norm, modid, mod,
		       ext -> value, label, PDU_WRITE);
#endif
	*extlp = (struct type_MTA_Extensions *) smalloc (sizeof **extlp);
	(*extlp) -> next = NULL;
	(*extlp) -> ExtensionField = ext;
	return OK;
}



static int build_dehl (dlh, dlp)
DLHistory *dlh;
struct type_Ext_DLExpansionHistory **dlp;
{
	struct type_Ext_DLExpansionHistory **dp;
	struct type_Ext_DLExpansion *d;
	int ndlexp = 0;

	*dlp = NULL;
	dp = dlp;
	for (;dlh; dlh = dlh -> dlh_next) {
		*dp = (struct type_Ext_DLExpansionHistory *)
			smalloc (sizeof **dp);
		(*dp) -> next = NULL;
		d = (*dp) -> DLExpansion =
			(struct type_Ext_DLExpansion *)
				smalloc (sizeof *d);

		if (build_addrdn (dlh -> dlh_addr,
				  dlh -> dlh_dn,
				  &d -> address) == NOTOK)
			return NOTOK;

		if (build_time (dlh -> dlh_time,
				&d -> dl__expansion__time) == NOTOK)
			return NOTOK;

		dp = &(*dp) -> next;
		if (++ ndlexp > UB_DL_EXPANSIONS)
			return setuberror ("DLexpansionHistory",
				    ndlexp, UB_DL_EXPANSIONS,
				    DRD_SIZE_CONSTRAINT_VIOLATION);
	}
	return OK;
}
	

static int build_msi (dp, msip)
DomSupInfo *dp;
struct type_Ext_MTASuppliedInformation **msip;
{
	struct type_Ext_MTASuppliedInformation *msi;

	*msip = msi = (struct type_Ext_MTASuppliedInformation *)
		smalloc(sizeof *msi);
	bzero ((char *)msi, sizeof *msi);

	if (dp -> dsi_time)
		if (build_time (dp -> dsi_time,
				&msi -> arrival__time) == NOTOK)
			return NOTOK;
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
			if ((msi -> attempted -> un.mta =
			     STR2QB (dp -> dsi_attempted_mta)) == NULL)
				return NOTOK;
		}
		else {
			msi -> attempted -> offset =
				choice_Ext_1_domain;
			
			if (build_gdi (&dp -> dsi_attempted_md,
				       &msi -> attempted -> un.domain) == NOTOK)
				return NOTOK;
		}
	}

	if (dp -> dsi_deferred &&
	    build_time (dp -> dsi_deferred,
			&msi -> deferred__time) == NOTOK)
		return NOTOK;
	if (dp -> dsi_other_actions) {
		if ((msi -> other__actions =
		     pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM,
			       PE_PRIM_BITS)) == NULLPE)
			return NOTOK;
		if ((dp -> dsi_other_actions & ACTION_REDIRECTED) &&
		    bit_on (msi -> other__actions,
			    bit_MTA_OtherActions_redirected) == NOTOK)
			return NOTOK;
		if ((dp -> dsi_other_actions & ACTION_EXPANDED) &&
		    bit_on (msi -> other__actions,
			    bit_MTA_OtherActions_dl__operation) == NOTOK)
			return NOTOK;
	}
	return OK;
}


static int build_inttrace (tp, tlp)
Trace	*tp;
struct type_Ext_InternalTraceInformation **tlp;
{
	struct type_Ext_InternalTraceInformation **tpp;
	struct type_Ext_InternalTraceInformationElement *ti;
	Trace	*tlast;

	*tlp = NULL;
	if (trace_type == RTSP_TRACE_ADMD ||
	    trace_type == RTSP_TRACE_NOINT)
		return NULL;

	for (tlast = tp; tlast && tlast -> trace_next;
	     tlast = tlast -> trace_next)
		continue;

	tpp = tlp;
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
		
		if (build_gdi (&tp -> trace_DomId,
			       &ti -> global__domain__identifier) == NOTOK)
			return NOTOK;
		if ((ti -> mta__name = STR2QB (tp -> trace_mta)) == NULL)
			return NOTOK;

		
		if (build_msi (&tp -> trace_DomSinfo,
			       &ti -> mta__supplied__information) == NOTOK)
			return NOTOK;
		tpp = &(*tpp) -> next;
	}
	return OK;
}

static int build_ext_iti (parm, crit, extlp)
Trace	*parm;
char	crit;
struct type_MTA_Extensions **extlp;
{
	struct type_MTA_ExtensionField *ext;
	struct type_Ext_InternalTraceInformation *value;

	if(build_inttrace (parm, &value) == NOTOK)
		return NOTOK;

	ext = (struct type_MTA_ExtensionField *) smalloc (sizeof *ext);

	if (build_ext_type (EXT_INTERNAL_TRACE_INFORMATION,
			    NULLOID,
			    &ext -> type) == NOTOK)
		return NOTOK;

	if (crit != EXT_INTERNAL_TRACE_INFORMATION_DC) {
		if (build_crit (crit,
				&ext -> criticality) == NOTOK)
			return NOTOK;
	}
	else
		ext -> criticality = NULL;


	if (encode_Ext_InternalTraceInformation
	    (&ext -> value, 1, 0, NULLCP, value) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't encode InternalTraceInformation value [%s]",
			 PY_pepy));
		return NOTOK;
	}

	PP_PDUP (Ext_InternalTraceInformation,
		ext -> value, "Extensions.InternalTraceInformation",
		PDU_WRITE);

	free_Ext_InternalTraceInformation (value);

	wrap_up_ext (*extlp, ext);
	return OK;
}




static int build_extension (ext, epp)
X400_Extension *ext;
struct type_MTA_ExtensionField **epp;
{
	struct type_MTA_ExtensionField *ep;

	*epp = ep = (struct type_MTA_ExtensionField *) smalloc (sizeof *ep);
	if (build_ext_type (ext -> ext_int, ext -> ext_oid,
			    &ep -> type) == NOTOK)
		return NOTOK;
	if (build_crit (ext -> ext_criticality, &ep -> criticality) == NOTOK)
		return NOTOK;
	if (build_ext_value (ext -> ext_value, &ep -> value) == NOTOK)
		return NOTOK;
	return OK;
}

static int build_gen_extensions (xp, extp)
X400_Extension *xp;
struct type_MTA_Extensions **extp;
{
	for (;xp; xp = xp -> ext_next) {
		*extp = (struct type_MTA_Extensions *)
				smalloc (sizeof **extp);
		(*extp) -> next = NULL;
		if (build_extension (xp,
				     &(*extp) -> ExtensionField) == NOTOK)
			return NOTOK;
		extp = &(*extp) -> next;
	}
	return OK;
}

static int build_pm_extensions (qp,p1,msg, extp)
Q_struct *qp;
int p1,msg;
struct type_MTA_Extensions **extp;
{
	struct type_MTA_Extensions **ep;

	*extp = NULL;

	if (build_gen_extensions (qp -> per_message_extensions, extp) == NOTOK)
		return NOTOK;

	for (ep = extp; *ep; ep = &(*ep) -> next)
		continue;

	if (qp -> recip_reassign_prohibited) {
		if (build_ext_generic (&qp -> recip_reassign_prohibited,
				       qp -> recip_reassign_crit,
				       EXT_RECIPIENT_REASSIGNMENT_PROHIBITED_DC,
				       EXT_RECIPIENT_REASSIGNMENT_PROHIBITED,
				       NULLOID,
				       _ZRecipientReassignmentProhibitedExt,
				       &_ZExt_mod,
				       build_ext_char,
				       "Extension.RecipientReassignmentProhibited",
				       ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> dl_expansion_prohibited) {
		if (build_ext_generic (&qp -> dl_expansion_prohibited,
				       qp -> dl_expansion_crit,
				       EXT_DL_EXPANSION_PROHIBITED_DC,
				       EXT_DL_EXPANSION_PROHIBITED,
				       NULLOID,
				       _ZDLExpansionProhibitedExt,
				       &_ZExt_mod,
				       build_ext_char,
				       "Extension.DLExpansionHistory",
				       ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> conversion_with_loss_prohibited) {
		if (build_ext_generic (&qp -> conversion_with_loss_prohibited,
				       qp -> conversion_with_loss_crit,
				       EXT_CONVERSION_WITH_LOSS_PROHIBITED_DC,
				       EXT_CONVERSION_WITH_LOSS_PROHIBITED,
				       NULLOID,
				       _ZConversionWithLossProhibitedExt,
				       &_ZExt_mod,
				       build_ext_char,
				       "Extension.ConversionWithLossProhibited",
				       ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> originator_certificate) {
		if (build_ext_secure (qp -> originator_certificate,
				      qp -> originator_certificate_crit,
				      EXT_ORIGINATOR_CERTIFICATE_DC,
				      EXT_ORIGINATOR_CERTIFICATE,
				      NULLOID,
				      _ZOriginatorCertificateExt,
				      &_ZExt_mod,
				      "Extensions.OriginatorCertificate",
				      ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> security_label) {
		if (build_ext_secure (qp -> security_label,
				      qp -> security_label_crit,
				      EXT_MESSAGE_SECURITY_LABEL_DC,
				      EXT_MESSAGE_SECURITY_LABEL,
				      NULLOID,
				      _ZMessageSecurityLabelExt,
				      &_ZExt_mod,
				      "Extensions.MessageSecurityLabel",
				      ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (qp -> general_content_correlator) {
		if (build_ext_secure (qp -> general_content_correlator,
					qp -> content_correlator_crit,
					EXT_CONTENT_CORRELATOR_DC,
					EXT_CONTENT_CORRELATOR,
					NULLOID,
					_ZContentCorrelatorExt,
					&_ZExt_mod,
					"Extensions.ContentCorrelator",
					ep) == NOTOK)
			return NOTOK;
		if (*ep &&
		    ps_get_abs ((*ep) -> ExtensionField -> value) >
		    UB_CONTENT_CORRELATOR_LENGTH)
			return setuberror ("ContentCorrelator",
				    ps_get_abs ((*ep) -> ExtensionField -> value),
				    UB_CONTENT_CORRELATOR_LENGTH,
				    DRD_SIZE_CONSTRAINT_VIOLATION);
		if (*ep)
			ep = &(*ep) -> next;
	}
	if (msg) {
		if (qp -> latest_time) {
			if (build_ext_generic ((caddr_t)qp -> latest_time,
					       qp -> latest_time_crit,
					       EXT_LATEST_DELIVERY_TIME_DC,
					       EXT_LATEST_DELIVERY_TIME,
					       NULLOID,
					       _ZLatestDeliveryTimeExt,
					       &_ZExt_mod,
					       build_ext_time,
					       "Extensions.LatestDeliveryTime",
					       ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> originator_return_address) {
			if (build_ext_generic
			    ((caddr_t)qp -> originator_return_address,
			     qp -> originator_return_address_crit,
			     EXT_ORIGINATOR_RETURN_ADDRESS_DC,
			     EXT_ORIGINATOR_RETURN_ADDRESS,
			     NULLOID,
			     _ZOriginatorReturnAddressExt,
			     &_ZExt_mod,
			     build_ext_ora,
			     "Extensions.OriginatorReturnAddress",
			     ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> algorithm_identifier) {
			if (build_ext_secure
			    (qp -> algorithm_identifier,
			     qp -> algorithm_identifier_crit,
			     EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER_DC,
			     EXT_CONTENT_CONFIDENTIALITY_ALGORITHM_IDENTIFIER,
			     NULLOID,
			     _ZContentConfidentialityAlgorithmIdentifierExt,
			     &_ZExt_mod,
			     "Extensions.ContentConfidentialityAlgorithmIdentifier",
			     ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> message_origin_auth_check) {
			if (build_ext_secure (qp -> message_origin_auth_check,
						qp -> message_origin_auth_check_crit,
						EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK_DC,
						EXT_MESSAGE_ORIGIN_AUTHENTICATION_CHECK,
						NULLOID,
						_ZMessageOriginAuthenticationCheckExt,
						&_ZExt_mod,
						"Extensions.MessageOriginAuthenticationCheck",
						ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	if (!msg) {
		if (qp -> message_origin_auth_check) {
			if (build_ext_secure
			    (qp -> message_origin_auth_check,
			     qp -> message_origin_auth_check_crit,
			     EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK_DC,
			     EXT_PROBE_ORIGIN_AUTHENTICATION_CHECK,
			     NULLOID,
			     _ZProbeOriginAuthenticationCheckExt,
			     &_ZExt_mod,
			     "Extensions.ProbeOriginAuthenticationCheck",
			     ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	if (p1) {
		if (qp -> dl_expansion_history) {
			if (build_ext_generic
			    ((caddr_t)qp -> dl_expansion_history,
			     qp -> dl_expansion_history_crit,
			     EXT_DL_EXPANSION_HISTORY_DC,
			     EXT_DL_EXPANSION_HISTORY,
			     NULLOID,
			     _ZDLExpansionHistoryExt,
			     &_ZExt_mod,
			     build_ext_dlh,
			     "Extensions.DLExpansionHistory",
			     ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
	
		if (qp -> trace) {
			if (build_ext_iti (qp -> trace,
					   EXT_INTERNAL_TRACE_INFORMATION_DC,
					   ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	if ((!p1) && msg) {
		if (qp -> forwarding_request != NOTOK) {
			if (build_ext_generic
			    ((caddr_t)&qp -> forwarding_request,
			     qp -> forwarding_request_crit,
			     EXT_FORWARDING_REQUEST_DC,
			     EXT_FORWARDING_REQUEST,
			     NULLOID,
			     _ZSequenceNumberExt,
			     &_ZExt_mod,
			     build_ext_int,
			     "Extensions.ForwardingRequest.SequenceNumber",
			     ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
		if (qp -> proof_of_submission_request) {
			if (build_ext_generic
			    ((caddr_t)&qp -> proof_of_submission_request,
			     qp -> proof_of_submission_crit,
			     EXT_PROOF_OF_SUBMISSION_REQUEST_DC,
			     EXT_PROOF_OF_SUBMISSION_REQUEST,
			     NULLOID,
			     _ZProofOfSubmissionRequestExt,
			     &_ZExt_mod,
			     build_ext_int,
			     "Extensions.ProofOfSubmissionRequest",
			     ep) == NOTOK)
				return NOTOK;
			if (*ep)
				ep = &(*ep) -> next;
		}
	}
	return OK;
}

static int build_drc_extensions (qp, dr, extp)
Q_struct *qp;
DRmpdu *dr;
struct type_MTA_Extensions **extp;
{
	struct type_MTA_Extensions **ep;

	*extp = NULL;

	if (build_gen_extensions (dr -> dr_per_report_extensions,
				  extp) == NOTOK)
		return NOTOK;

	for (ep = extp; *ep; ep = &(*ep) -> next)
		continue;

	if (qp -> general_content_correlator) {
		if (build_ext_secure (qp -> general_content_correlator,
				     qp -> content_correlator_crit,
				     EXT_CONTENT_CORRELATOR_DC,
				     EXT_CONTENT_CORRELATOR,
				     NULLOID,
				     _ZContentCorrelatorExt,
				     &_ZExt_mod,
				     "Extensions.ContentCorrelator",
				     ep) == NOTOK)
			return NOTOK;
		if (*ep &&
		    ps_get_abs ((*ep) -> ExtensionField -> value) >
		    UB_CONTENT_CORRELATOR_LENGTH)
			return setuberror ("ContentCorrelator",
					   ps_get_abs ((*ep) -> ExtensionField -> value),
					   UB_CONTENT_CORRELATOR_LENGTH,
					   DRD_SIZE_CONSTRAINT_VIOLATION);
		if (*ep)
			ep = &(*ep) -> next;
	}
	return OK;
}


static int build_drc_pr_extensions (qp, rr, extp)
Q_struct *qp;
Rrinfo *rr;
struct type_MTA_Extensions **extp;
{
	struct type_MTA_Extensions **ep;

	*extp = NULL;

	if (build_gen_extensions (rr -> rr_per_recip_extensions,
				  extp) == NOTOK)
		return NOTOK;

	for (ep = extp; *ep; ep = &(*ep) -> next)
		continue;

	if (rr -> rr_redirect_history) {
		if (build_ext_generic ((caddr_t)rr -> rr_redirect_history,
				       rr -> rr_redirect_history_crit,
				       EXT_REDIRECTION_HISTORY_DC,
				       EXT_REDIRECTION_HISTORY,
				       NULLOID,
				       _ZRedirectionHistoryExt,
				       &_ZExt_mod,
				       build_aext_redir,
				       "Extensions.RedirectionHistory",
				       ep) == NOTOK)
			return NOTOK;
		
		if (*ep)
			ep = &(*ep) -> next;
	}

	if (rr -> rr_physical_fwd_addr) {
		if (build_ext_generic ((caddr_t) rr -> rr_physical_fwd_addr,
				       rr -> rr_physical_fwd_addr_crit,
				       EXT_PHYSICAL_FORWARDING_ADDRESS_DC,
				       EXT_PHYSICAL_FORWARDING_ADDRESS,
				       NULLOID,
				       _ZPhysicalForwardingAddressExt,
				       &_ZExt_mod,
				       build_ext_fn,
				       "Extensions.PhysicalForwardingAddress",
				       ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}		

	if (rr -> rr_recip_certificate) {
		if (build_ext_secure (rr -> rr_recip_certificate,
				      rr -> rr_recip_certificate_crit,
				      EXT_RECIPIENT_CERTIFICATE_DC,
				      EXT_RECIPIENT_CERTIFICATE,
				      NULLOID,
				      _ZRecipientCertificateMTA,
				      &_ZMTA_mod,
				      "Extensions.RecipientCertificate",
				      ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}

	if (rr -> rr_report_origin_authentication_check) {
		if (build_ext_secure (rr -> rr_report_origin_authentication_check,
				      rr -> rr_report_origin_authentication_check_crit,
				      EXT_PROOF_OF_DELIVERY_DC,
				      EXT_PROOF_OF_DELIVERY,
				      NULLOID,
				      _ZProofOfDeliveryToks,
				      &_ZToks_mod,
				      "Extensions.ProofOfDelivery",
				      ep) == NOTOK)
			return NOTOK;
		if (*ep)
			ep = &(*ep) -> next;
	}

	return OK;
}

static int build_prf_ext (ad,p1,msg, extlp)
ADDR	*ad;
int	p1,msg; 
struct type_MTA_Extensions **extlp;
{
	struct type_MTA_Extensions **ep;

	if (build_gen_extensions (ad -> ad_per_recip_ext_list, extlp) == NOTOK)
		return NOTOK;

	for (ep = extlp; *ep; ep = &(*ep) -> next)
		continue;
#define bump(x)	if (*(x)) (x) = &(*x) -> next

	if (ad -> ad_orig_req_alt) {
		if (build_ext_generic
		    (ad -> ad_orig_req_alt,
		     ad -> ad_orig_req_alt_crit,
		     EXT_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT_DC,
		     EXT_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT,
		     NULLOID,
		     _ZOriginatorRequestedAlternateRecipientExt,
		     &_ZExt_mod,
		     build_aext_oraa,
		     "Extensions.OriginatorRequestedAlternateRecipient",
		     ep) == NOTOK)
			return NOTOK;
		bump(ep);
	}
	if (ad -> ad_req_del[0] != AD_RDM_NOTUSED) {
		if (build_ext_generic ((caddr_t)ad -> ad_req_del,
				       ad -> ad_req_del_crit,
				       EXT_REQUESTED_DELIVERY_METHOD_DC,
				       EXT_REQUESTED_DELIVERY_METHOD,
				       NULLOID,
				       _ZRequestedDeliveryMethodExt,
				       &_ZExt_mod,
				       build_aext_rdm,
				       "Extensions.RequestedDeliveryMethod",
				       ep) == NOTOK)
			return NOTOK;
		bump(ep);
	}

	if (ad -> ad_phys_rendition_attribs) {
		if (build_ext_generic
		    ((caddr_t)ad -> ad_phys_rendition_attribs,
		     ad -> ad_phys_rendition_attribs_crit,
		     EXT_PHYSICAL_RENDITION_ATTRIBUTES_DC,
		     EXT_PHYSICAL_RENDITION_ATTRIBUTES,
		     NULLOID,
		     _ZPhysicalRenditionAttributesExt,
		     &_ZExt_mod,
		     build_aext_pra,
		     "Extensions.PhysicalRenditionAttributes",
		     ep) == NOTOK)
			return NOTOK;
		bump(ep);
	}
	if (msg) {
		if (ad -> ad_phys_forward) {
			if (build_ext_generic
			    (&ad -> ad_phys_forward,
			     ad -> ad_phys_forward_crit,
			     EXT_PHYSICAL_FORWARDING_PROHIBITED_DC,
			     EXT_PHYSICAL_FORWARDING_PROHIBITED,
			     NULLOID,
			     _ZPhysicalForwardingProhibitedExt,
			     &_ZExt_mod,
			     build_ext_char,
			     "Extensions.PhysicalForwardingProhibited",
			     ep) == NOTOK)
				return NOTOK;
			bump(ep);
		}
		if (ad -> ad_phys_fw_ad_req) {
			if (build_ext_generic
			    (&ad -> ad_phys_fw_ad_req,
			     ad -> ad_phys_fw_ad_crit,
			     EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST_DC,
			     EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST,
			     NULLOID,
			     _ZPhysicalForwardingAddressRequestExt,
			     &_ZExt_mod,
			     build_ext_char,
			     "Extensions.PhysicalForwardingAddressRequest",
			     ep) == NOTOK)
				return NOTOK;
			bump(ep);
		}
		if (ad -> ad_phys_modes) {
			if (build_ext_generic
			    ((caddr_t)&ad -> ad_phys_modes,
			     ad -> ad_phys_modes_crit,
			     EXT_PHYSICAL_DELIVERY_MODES_DC,
			     EXT_PHYSICAL_DELIVERY_MODES,
			     NULLOID,
			     _ZPhysicalDeliveryModesExt,
			     &_ZExt_mod,
			     build_aext_pdm,
			     "Extensions.PhysicalDeliveryModes",
			     ep) == NOTOK)
				return NOTOK;
			bump(ep);
		}

		if (ad -> ad_reg_mail_type) {
			if (build_ext_generic
			    ((caddr_t)&ad -> ad_reg_mail_type,
			     ad -> ad_reg_mail_type_crit,
			     EXT_REGISTERED_MAIL_DC,
			     EXT_REGISTERED_MAIL,
			     NULLOID,
			     _ZRegisteredMailTypeExt,
			     &_ZExt_mod,
			     build_ext_int,
			     "Extensions.RegisteredMailType",
			     ep) == NOTOK)
				return NOTOK;
			bump (ep);
		}
		if (ad -> ad_recip_number_for_advice) {
			if (build_ext_generic
			    (ad -> ad_recip_number_for_advice,
			     ad -> ad_recip_number_for_advice_crit,
			     EXT_RECIPIENT_NUMBER_FOR_ADVICE_DC,
			     EXT_RECIPIENT_NUMBER_FOR_ADVICE,
			     NULLOID,
			     _ZRecipientNumberForAdviceExt,
			     &_ZExt_mod,
			     build_aext_rnfa,
			     "Extensions.RecipientNumberForAdvice",
			     ep) == NOTOK)
				return NOTOK;
			bump (ep);
		}
		if (ad -> ad_pd_report_request) {
			if (build_ext_generic
			    ((caddr_t)&ad -> ad_pd_report_request,
			     ad -> ad_pd_report_request_crit,
			     EXT_PHYSICAL_DELIVERY_REPORT_REQUEST_DC,
			     EXT_PHYSICAL_DELIVERY_REPORT_REQUEST,
			     NULLOID,
			     _ZPhysicalDeliveryReportRequestExt,
			     &_ZExt_mod,
			     build_ext_int,
			     "Extensions.PhysicalDeliveryReportRequest",
			     ep) == NOTOK)
				return NOTOK;
			bump (ep);
		}
		if (ad -> ad_message_token) {
			if (build_ext_secure (ad -> ad_message_token,
					      ad -> ad_message_token_crit,
					      EXT_MESSAGE_TOKEN_DC,
					      EXT_MESSAGE_TOKEN,
					      NULLOID,
					      _ZMessageTokenExt,
					      &_ZExt_mod,
					      "Extensions.MessageToken",
					      ep) == NOTOK)
				return NOTOK;
			bump (ep);
		}
		if (ad -> ad_content_integrity) {
			if (build_ext_secure (ad -> ad_content_integrity,
						ad -> ad_content_integrity_crit,
						EXT_CONTENT_INTEGRITY_CHECK_DC,
						EXT_CONTENT_INTEGRITY_CHECK,
						NULLOID,
						_ZContentIntegrityCheckExt,
						&_ZExt_mod,
						"Extensions.ContentIntegrityCheck",
						ep) == NOTOK)
				return NOTOK;
			bump(ep);
		}
		if (ad -> ad_proof_delivery) {
			if (build_ext_generic
			    ((caddr_t)&ad -> ad_proof_delivery,
			     ad -> ad_proof_delivery_crit,
			     EXT_PROOF_OF_DELIVERY_REQUEST_DC,
			     EXT_PROOF_OF_DELIVERY_REQUEST,
			     NULLOID,
			     _ZProofOfDeliveryRequestExt,
			     &_ZExt_mod,
			     build_ext_int,
			     "Extensions.ProofOfDeliveryRequest",
			     ep) == NOTOK)
				return NOTOK;
			bump(ep);
		}
	}
	if (p1) {
		if (ad -> ad_redirection_history) {
			if (build_ext_generic
			    ((caddr_t)ad -> ad_redirection_history,
			     ad -> ad_redirection_history_crit,
			     EXT_REDIRECTION_HISTORY_DC,
			     EXT_REDIRECTION_HISTORY,
			     NULLOID,
			     _ZRedirectionHistoryExt,
			     &_ZExt_mod,
			     build_aext_redir,
			     "Extensions.RedirectionHistory",
			     ep) == NOTOK)
				return NOTOK;
			bump (ep);
		}
	}
	return OK;
}

static int build_dre_extensions (dr, extp)
DRmpdu *dr;
struct type_MTA_Extensions **extp;
{
	struct type_MTA_Extensions **ep;

	if (build_gen_extensions (dr -> dr_per_envelope_extensions,
				  extp) == NOTOK)
		return NOTOK;

	for (ep = extp; *ep; ep = &(*ep) -> next)
		continue;

	if (dr -> dr_trace) {
		if (build_ext_iti (dr -> dr_trace,
				   EXT_INTERNAL_TRACE_INFORMATION_DC, ep) == NOTOK)
			return NOTOK;
		if (*ep) ep = &(*ep) -> next;
	}
	if (dr -> dr_security_label) {
		if (build_ext_secure (dr -> dr_security_label,
				      dr -> dr_security_label_crit,
				      EXT_MESSAGE_SECURITY_LABEL_DC,
				      EXT_MESSAGE_SECURITY_LABEL,
				      NULLOID,
				      _ZMessageSecurityLabelExt,
				      &_ZExt_mod,
				      "Extensions.MessageSecurityLabel",
				      ep) == NOTOK)
			return NOTOK;
		if (*ep) ep = &(*ep) -> next;
	}
	if (dr -> dr_report_origin_auth_check) {
		if (build_ext_secure
		    (dr -> dr_report_origin_auth_check,
		     dr -> dr_report_origin_auth_check_crit,
		     EXT_REPORT_ORIGIN_AUTHENTICATION_CHECK_DC,
		     EXT_REPORT_ORIGIN_AUTHENTICATION_CHECK,
		     NULLOID,
		     _ZReportOriginAuthenticationCheckExt,
		     &_ZExt_mod,
		     "Extensions.MessageOriginAuthenticationCheck",
		     ep) == NOTOK)
				return NOTOK;
		if (*ep) ep = &(*ep) -> next;
	}
	if (dr -> dr_reporting_mta_certificate) {
		if (build_ext_secure
		    (dr -> dr_reporting_mta_certificate,
		     dr -> dr_reporting_mta_certificate_crit,
		     EXT_REPORTING_MTA_CERTIFICATE_DC,
		     EXT_REPORTING_MTA_CERTIFICATE,
		     NULLOID,
		     _ZReportingMTACertificateExt,
		     &_ZExt_mod,
		     "Extensions.ReportingMTACertificate",
		     ep) == NOTOK)
			return NOTOK;
		if (*ep) ep = &(*ep) -> next;
	}
	if (dr -> dr_reporting_dl_name) {
		if (build_ext_generic ((caddr_t)dr -> dr_reporting_dl_name,
				       dr -> dr_reporting_dl_name_crit,
				       EXT_REPORT_DL_NAME_DC,
				       EXT_REPORT_DL_NAME,
				       NULLOID,
				       _ZReportingDLNameExt,
				       &_ZExt_mod,
				       build_ext_fn,
				       "Extension.ReportingDLName",
				       ep) == NOTOK)
			return NOTOK;
		if (*ep) ep = &(*ep) -> next;
	}
	if (dr -> dr_dl_history) {
		if (build_ext_generic
		    ((caddr_t)dr -> dr_dl_history,
		     dr -> dr_dl_history_crit,
		     EXT_ORIGINATOR_AND_DL_EXPANSION_HISTORY_DC,
		     EXT_ORIGINATOR_AND_DL_EXPANSION_HISTORY,
		     NULLOID,
		     _ZOriginatorAndDLExpansionHistoryExt,
		     &_ZExt_mod,
		     build_ext_dlh,
		     "Extensions.OriginatorAndDLExpansionHistory",
		     ep) == NOTOK)
			return NOTOK;
		if (*ep) ep = &(*ep) -> next;
	}
	return OK;
}

static int setuberror(msg, length, constrnt, error_code)
char *msg;
int length;
int constrnt;
{
	(void) sprintf (ub_error_string,
			"Upper bound constraint exceeded: %s (%d > %d)",
			msg, length, constrnt);
	PP_LOG(LLOG_EXCEPTIONS, ("%s", ub_error_string));
	ub_error_set = error_code;
	return NOTOK;
}
