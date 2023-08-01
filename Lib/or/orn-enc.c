/* orn-enc.c: routines to encode ORNames */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/orn-enc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/orn-enc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: orn-enc.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "or.h"
#include "IOB-types.h"
#include "MTA-types.h"
#include "Ext-types.h"
#include "extension.h"
#include <isode/isoaddrs.h>

static int build_std_or(), build_dd_or(), build_ext_or();

#define STR2QB(s)	str2qb(s, strlen(s), 1)

struct type_IOB_ORName	*orn2orname(orn)
ORName	*orn;
{
	struct type_IOB_ORName *orname;
	OR_ptr or;
	int ret;


	orname = (struct type_IOB_ORName *) smalloc (sizeof *orname);
	bzero ((char *)orname, sizeof *orname);

	for (or = orn -> on_or; or; or = or -> or_next) {
		switch (or -> or_type) {
		    case OR_C:
		    case OR_ADMD:
		    case OR_PRMD:
		    case OR_X121:
		    case OR_TID:
		    case OR_UAID:
			ret = build_std_or (&orname -> standard__attributes,
					    or);
			break;

		    case OR_O:
		    case OR_OU:
		    case OR_S:
		    case OR_G:
		    case OR_I:
		    case OR_GQ:
			if (or -> or_encoding == OR_ENC_PS)
				ret = build_std_or (&orname -> standard__attributes,
						    or);
			else if (or -> or_encoding == OR_ENC_TTX)
				ret = build_ext_or (&orname -> extension__attributes,
						    or);
			else {
				ret = build_std_or (&orname -> standard__attributes,
						    or);
				if (ret == OK)
					ret = build_ext_or (&orname -> extension__attributes,
							    or);
			}
			break;
		    case OR_DD:
			if (or -> or_encoding == OR_ENC_PS)
				ret = build_dd_or (&orname -> domain__defined,
						    or);
			else if (or -> or_encoding == OR_ENC_TTX)
				ret = build_ext_or (&orname -> extension__attributes,
						    or);
			else {
				ret = build_dd_or (&orname -> domain__defined,
						    or);
				if (ret == OK)
					ret = build_ext_or (&orname -> extension__attributes,
							    or);
			}
			break;
		    case OR_CN:
		    case OR_PDSNAME:
		    case OR_PD_C:
		    case OR_POSTCODE:
		    case OR_PDO_NAME:
		    case OR_PDO_NUM:
		    case OR_OR_COMPS:
		    case OR_PD_PN:
		    case OR_PD_O:
		    case OR_PD_COMPS:
		    case OR_UPA_PA:
		    case OR_STREET:
		    case OR_PO_BOX:
		    case OR_PRA:
		    case OR_UPN:
		    case OR_LPA:
		    case OR_ENA_N:
		    case OR_ENA_S:
		    case OR_ENA_P:
		    case OR_TT:
			ret = build_ext_or (&orname -> extension__attributes,
					    or);
			break;
		}
		if (ret == NOTOK) {
			free_IOB_ORName (orname);
			return NULL;
		}
	}
	if (orname -> standard__attributes == NULL)
	   	orname -> standard__attributes = 
		       (struct type_MTA_StandardAttributes *) calloc
		       (1, sizeof (struct type_MTA_StandardAttributes));

	if (orn -> on_dn) {
		PE pet;
		if (encode_IF_Name (&orname -> directory__name, 1, NULLCP, 0,
				orn -> on_dn) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS, ("Can't encode DN [%s]",
						  PY_pepy));
			free_IOB_ORName (orname);
			return NULL;
		}
		if ((pet = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 0)) ==
		    NULLPE) {
			PP_LOG (LLOG_EXCEPTIONS, ("Can't allocate PE"));
			free_IOB_ORName (orname);
			return NULL;
		}
		pet -> pe_cons = orname->directory__name;
		orname->directory__name = pet;
	} else
		orname->directory__name = NULL;
	return orname;
}

PE orname2pe(orname)
struct type_IOB_ORName	*orname;
{
	PE	pe;
	if (encode_IOB_ORName (&pe, 1, NULLCP, 0, orname) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't encode DN [%s]",
					  PY_pepy));
		return NULLPE;
	}
	return pe;
}

PE orn2pe (orn)
ORName *orn;
{
	struct type_IOB_ORName	*orname;
	PE	pe;
	
	if ((orname = orn2orname(orn)) == NULL)
		return NULLPE;
	pe = orname2pe(orname);
	free_IOB_ORName(orname);
	return pe;
}

struct type_MTA_ORAddress *ora2oradr (ortree)
OR_ptr ortree;
{
	struct type_MTA_ORAddress *ora;
	OR_ptr or;
	int ret;

	ora = (struct type_MTA_ORAddress *) smalloc (sizeof *ora);
	bzero ((char *)ora, sizeof *ora);

	for (or = ortree; or; or = or -> or_next) {
		switch (or -> or_type) {
		    case OR_C:
		    case OR_ADMD:
		    case OR_PRMD:
		    case OR_X121:
		    case OR_TID:
		    case OR_UAID:
			ret = build_std_or (&ora -> standard__attributes,
					    or);
			break;

		    case OR_O:
		    case OR_OU:
		    case OR_S:
		    case OR_G:
		    case OR_I:
		    case OR_GQ:
			if (or -> or_encoding == OR_ENC_PS)
				ret = build_std_or (&ora -> standard__attributes,
						    or);
			else if (or -> or_encoding == OR_ENC_TTX)
				ret = build_ext_or (&ora -> extension__attributes,
						    or);
			else {
				ret = build_std_or (&ora -> standard__attributes,
						    or);
				if (ret == OK)
					ret = build_ext_or (&ora -> extension__attributes,
							    or);
			}
			break;
		    case OR_DD:
			if (or -> or_encoding == OR_ENC_PS)
				ret = build_dd_or (&ora -> domain__defined__attributes,
						    or);
			else if (or -> or_encoding == OR_ENC_TTX)
				ret = build_ext_or (&ora -> extension__attributes,
						    or);
			else {
				ret = build_dd_or (&ora -> domain__defined__attributes,
						    or);
				if (ret == OK)
					ret = build_ext_or (&ora -> extension__attributes,
							    or);
			}
			break;
		    case OR_CN:
		    case OR_PDSNAME:
		    case OR_PD_C:
		    case OR_POSTCODE:
		    case OR_PDO_NAME:
		    case OR_PDO_NUM:
		    case OR_OR_COMPS:
		    case OR_PD_PN:
		    case OR_PD_O:
		    case OR_PD_COMPS:
		    case OR_UPA_PA:
		    case OR_STREET:
		    case OR_PO_BOX:
		    case OR_PRA:
		    case OR_UPN:
		    case OR_LPA:
		    case OR_ENA_N:
		    case OR_ENA_S:
		    case OR_ENA_P:
		    case OR_TT:
			ret = build_ext_or (&ora -> extension__attributes,
					    or);
			break;
		}
		if (ret == NOTOK) {
			free_MTA_ORAddress (ora);
			return NULL;
		}
	}
	return ora;
}

PE oradr2pe(ora)
struct type_MTA_ORAddress *ora;
{
	PE pe;
	if (encode_MTA_ORAddress (&pe, 1, NULLCP, 0, ora) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't encode ORAddress [%s]",
					  PY_pepy));
		return NULLPE;
	}
	
	return pe;
}

PE ora2pe(ortree)
OR_ptr	ortree;
{
	struct type_MTA_ORAddress	*ora;
	PE	pe;
	if ((ora = ora2oradr(ortree)) == NULL)
		return NULLPE;
	pe = oradr2pe(ora);
	free_MTA_ORAddress(ora);
	return pe;
}

static int build_std_or (std, or)
struct type_MTA_StandardAttributes **std;
OR_ptr or;
{
	if (*std == NULL) {
		*std = (struct type_MTA_StandardAttributes *)
			smalloc (sizeof **std);
		bzero ((char *)*std, sizeof **std);
	}
	
	switch (or -> or_type) {
	    case OR_C:
		{
			struct type_MTA_CountryName *co;
			if ((*std) -> country__name) {
				PP_LOG (LLOG_EXCEPTIONS,
					("two country attributes"));
				return NOTOK;
			}
			co = (struct type_MTA_CountryName *)
				smalloc (sizeof *co);
			if (or_str_isns (or -> or_value)) {
				co -> offset = type_MTA_CountryName_x121__dcc__code;
				co -> un.x121__dcc__code =
					STR2QB(or->or_value);
			}
			else {
				co -> offset = type_MTA_CountryName_iso__3166__alpha2__code;
				co -> un.iso__3166__alpha2__code =
					STR2QB (or -> or_value);
			}
			(*std) -> country__name = co;
		}
		break;
	    case OR_ADMD:
		{
			struct type_MTA_AdministrationDomainName *admd;

			if ((*std) -> administration__domain__name != NULL) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Already got an ADMD"));
				return NOTOK;
			}
			admd = (*std) -> administration__domain__name =
				(struct type_MTA_AdministrationDomainName *)
					smalloc (sizeof *admd);
			if (or_str_isns (or -> or_value)) {
				admd -> offset = type_MTA_AdministrationDomainName_numeric;
				admd -> un.numeric = STR2QB (or -> or_value);
			}
			else {
				admd -> offset =
					type_MTA_AdministrationDomainName_printable;
				admd -> un.printable = STR2QB (or -> or_value);
		}
		}
		break;
			
	    case OR_PRMD:
		{
			struct type_MTA_PrivateDomainName *prmd;

			if ((*std) -> private__domain__name != NULL ){
				PP_LOG (LLOG_EXCEPTIONS,
					("Already got a PRMD"));
				return NOTOK;
			}
			prmd = (*std) -> private__domain__name =
				(struct type_MTA_PrivateDomainName *)
					smalloc (sizeof *prmd);
			if (or_str_isns (or -> or_value)) {
				prmd -> offset = type_MTA_PrivateDomainName_numeric;
				prmd -> un.numeric = STR2QB (or-> or_value);
			}
			else {
				prmd -> offset = type_MTA_PrivateDomainName_printable;
				prmd -> un.printable = STR2QB (or-> or_value);
			}
		}
		break;

	    case OR_O:
		{
			if (or -> or_encoding == OR_ENC_TTX)
				break;
			if ((*std) -> organization__name != NULL) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Already got an Organization"));
				return NOTOK;
			}
			(*std) -> organization__name =
				or_getps (or -> or_value, or->or_encoding);
		}
		break;
	    case OR_OU:
		{
			struct type_MTA_OrganizationalUnitNames *ou, **oup;

			if (or -> or_encoding == OR_ENC_TTX)
				break;
			for (oup = &(*std) -> organizational__unit__names;
			     *oup; oup = &(*oup) -> next)
				continue;
			ou = (struct type_MTA_OrganizationalUnitNames *)
				smalloc (sizeof *ou);
			ou -> OrganizationUnitName =
				or_getps (or -> or_value, or -> or_encoding);
			ou -> next = NULL;
			*oup = ou;
		}
		break;
				
	    case OR_X121:
		{
			if ((*std) -> network__address != NULL) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Extra X121 id"));
				return NOTOK;
			}
			(*std) -> network__address = STR2QB (or -> or_value);
		}
		break;

	    case OR_TID:
		{
			if ((*std) -> terminal__identifier != NULL) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Extra terminal identifier"));
				return NOTOK;
			}
			(*std) -> terminal__identifier = STR2QB (or -> or_value);
		}
		break;

	    case OR_UAID:
		{
			if ((*std) -> numeric__user__identifier != NULL) {
				PP_LOG (LLOG_EXCEPTIONS,
					("extra numeric user id"));
				return NOTOK;
			}
			(*std) -> numeric__user__identifier =
				STR2QB (or -> or_value);
		}
		break;
				
	    case OR_S:
	    case OR_G:
	    case OR_I:
	    case OR_GQ:
		{
			struct type_MTA_PersonalName *pn;

			if ((*std) -> personal__name == NULL) {
				pn = (*std) -> personal__name =
					(struct type_MTA_PersonalName *)
						smalloc (sizeof *pn);
				bzero ((char *)pn, sizeof *pn);
			}
			else	pn = (*std) -> personal__name;

			switch (or -> or_type) {
			    case OR_S:
				pn -> surname = or_getps (or -> or_value,
							  or -> or_encoding);
				break;

			    case OR_G:
				pn -> given__name = or_getps (or -> or_value,
							      or -> or_encoding);
				break;

			    case OR_I:
				pn -> initials = or_getps (or -> or_value,
							   or -> or_encoding);
				break;

			    case OR_GQ:
				pn -> generation__qualifier =
					or_getps (or -> or_value,
						  or -> or_encoding);
				break;
			}
		}
		break;
				
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Bad type %d", or -> or_type));
		return NOTOK;
	}
	return OK;
}

static int build_dd_or (domd, or)
struct type_MTA_DomainDefinedAttributes **domd;
OR_ptr or;
{
	struct type_MTA_DomainDefinedAttributes **ddp, *dp;

	for (ddp = domd; *ddp; ddp = &(*ddp) -> next)
		continue;

	dp = (struct type_MTA_DomainDefinedAttributes *)
		smalloc (sizeof *dp);
	if (dp == NULL)
		return NOTOK;
	dp -> DomainDefinedAttribute =
		(struct type_MTA_DomainDefinedAttribute *)
			smalloc (sizeof *dp -> DomainDefinedAttribute);
	dp -> next = NULL;
	dp -> DomainDefinedAttribute -> type = STR2QB (or -> or_ddname);
	dp -> DomainDefinedAttribute -> value = STR2QB (or -> or_value);

	*ddp = dp;
	return OK;
}


static int build_aext (or, mod, offset, label, fnx, type, ext)
OR_ptr or;
modtyp *mod;
int offset;
IFP	fnx;
char *label;
struct type_MTA_ExtensionAttributes **ext;
{
	struct type_MTA_ExtensionAttribute *ep;
	struct type_MTA_ExtensionAttributes *epa;
	caddr_t parm;
	PE pe;

	if ((*fnx) (or, &parm) == NOTOK)
		return NOTOK;

	ep = (struct type_MTA_ExtensionAttribute *) smalloc (sizeof *ep);
	ep -> type = type;

	if (enc_f (offset, mod, &pe, 1, 0, NULLCP, parm) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't encode value %s: %s",
					  label, PY_pepy));
		return NOTOK;
	}
	/* insert tag */
	ep -> value = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 1);
	ep -> value -> pe_cons = pe;
#if PP_DEBUG > 0
        if(pp_log_norm -> ll_events & LLOG_PDUS)
                pvpdu (pp_log_norm, offset, mod,
                       ep -> value, label, PDU_WRITE);
#endif
        (void) fre_obj (parm, mod->md_dtab[offset], mod, 1);

	epa = (struct type_MTA_ExtensionAttributes *) smalloc (sizeof *epa);
	epa -> next = *ext;
	*ext = epa;
	epa -> ExtensionAttribute = ep;

	return OK;
}

static int ext_ttx_str (or, qb)
OR_ptr or;
struct qbuf **qb;
{
	if ((*qb = or_getttx (or -> or_value, or -> or_encoding)) == NULL)
		return NOTOK;
	return OK;
}

static int ext_ps_str (or, qb)
OR_ptr or;
struct qbuf **qb;
{
	if ((*qb = or_getps (or -> or_value, or -> or_encoding)) == NULL)
		return NOTOK;
	return OK;
}

static int ext_pdsparm (or, parm)
OR_ptr or;
struct type_Ext_PDSParameter **parm;
{
	*parm = (struct type_Ext_PDSParameter *)
		smalloc (sizeof **parm);
	
	switch (or -> or_encoding) {
	    case OR_ENC_TTX_AND_OR_PS:
		(*parm) -> printable__string = or_getps (or -> or_value,
						      or -> or_encoding);
		/* fall */
	    case OR_ENC_TTX:
		(*parm) -> teletex__string = or_getttx (or -> or_value,
						     or -> or_encoding);
		return OK;

	    case OR_ENC_PS:
		(*parm) -> printable__string = or_getps (or -> or_value,
							 or -> or_encoding);
		return OK;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown encoding type %d for %s",
					  or -> or_encoding, or -> or_value));
		return NOTOK;
	}
}

static int ext_upa (or, parm) /* XXX BROKEN */
OR_ptr or;
struct type_Ext_UnformattedPostalAddress **parm;
{
	struct element_Ext_1 **pap;
	*parm = (struct type_Ext_UnformattedPostalAddress *)
		smalloc (sizeof **parm);
	pap = &(*parm)->printable__address;

	for (;or && or -> or_type == OR_UPA_PA; or = or -> or_next) {
		switch (or -> or_encoding) {
		    case OR_ENC_TTX:
			(*parm) -> teletex__string =
				or_getttx (or -> or_value,
					   or -> or_encoding);
			break;
		    case OR_ENC_PS:
			*pap = (struct element_Ext_1 *)
				smalloc (sizeof **pap);
			(*pap) -> PrintableString =
				or_getps (or -> or_value, or -> or_encoding);
			(*pap) -> next = NULL;
			pap = &(*pap) -> next;
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unexpected encoding in UFPA %s %d",
				 or -> or_value, or -> or_encoding));
			return NOTOK;
		}
	}
	return OK;
}

static int ext_psorns_str (or, parm)
OR_ptr or;
struct type_Ext_PostalCode **parm;
{
	*parm = (struct type_Ext_PostalCode *)
		smalloc (sizeof **parm);
	switch (or -> or_encoding) {
	    case OR_ENC_NUM:
		(*parm) -> offset = type_Ext_PostalCode_numeric__code;
		(*parm) -> un.numeric__code = STR2QB (or -> or_value);
		break;
	    case OR_ENC_PS:
		(*parm) -> offset = type_Ext_PostalCode_printable__code;
		(*parm) -> un.printable__code = STR2QB(or -> or_value);
		break;
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Unexpected postalcode encoding %d %s",
			 or -> or_encoding, or -> or_value));
		return NOTOK;
	}
	return OK;
}

static int ext_ttxous (or, parm)
OR_ptr or;
struct type_Ext_TeletexOrganizationalUnitNames **parm;
{
	for (;or && or -> or_type == OR_OU; or = or -> or_next) {
		if (or -> or_encoding != OR_ENC_TTX &&
		    or -> or_encoding != OR_ENC_TTX_AND_OR_PS)
			continue;
		*parm = (struct type_Ext_TeletexOrganizationalUnitNames *)
			smalloc (sizeof **parm);
		(*parm) -> next = NULL;
		(*parm) -> TeletexOrganizationalUnitName =
			or_getttx (or -> or_value,
				   or -> or_encoding);
		parm = &(*parm) -> next;
	}
	return OK;
}

static int ext_ttx_pn (or, parm)
OR_ptr or;
struct type_Ext_TeletexPersonalName **parm;
{
	*parm = (struct type_Ext_TeletexPersonalName *)
		smalloc (sizeof **parm);
	while (or) {
		if (or -> or_encoding != OR_ENC_TTX &&
		    or -> or_encoding != OR_ENC_TTX_AND_OR_PS)
			continue;
		switch (or -> or_type) {
		    case OR_S:
			(*parm) -> surname = or_getttx (or->or_value,
							or -> or_encoding);
			break;
		    case OR_GQ:
			(*parm) -> given__name = or_getttx (or -> or_value,
							    or -> or_encoding);
			break;
		    case OR_I:
			(*parm) -> initials = or_getttx (or -> or_value,
							 or -> or_encoding);
			break;
		    case OR_G:
			(*parm) -> generation__qualifier =
				or_getttx (or -> or_value,
					   or -> or_encoding);
			break;

		    default:
			return OK;
		}
	}
	return OK;
}

static int ext_ttx_dd (or, parm)
OR_ptr or;
struct type_Ext_TeletexDomainDefinedAttributes **parm;
{
	for (;or && or -> or_type == OR_DD; or = or -> or_next) {
		if (or -> or_encoding != OR_ENC_TTX &&
		    or -> or_encoding != OR_ENC_TTX_AND_OR_PS)
			continue;
		*parm = (struct type_Ext_TeletexDomainDefinedAttributes *)
			smalloc (sizeof **parm);
		(*parm) -> next = NULL;
		(*parm) -> TeletexDomainDefinedAttribute =
			(struct type_Ext_TeletexDomainDefinedAttribute *)
				smalloc (sizeof (struct type_Ext_TeletexDomainDefinedAttribute));
		(*parm) -> TeletexDomainDefinedAttribute -> type =
			or_getttx (or -> or_ddname, or -> or_encoding);
		(*parm) -> TeletexDomainDefinedAttribute -> value =
			or_getttx (or -> or_value, or -> or_encoding);
		parm = &(*parm) -> next;
	}
	return OK;
}

static int ext_tt (or, parm)
OR_ptr or;
struct type_Ext_TerminalType **parm;
{
	(*parm) = (struct type_Ext_TerminalType *) smalloc (sizeof **parm);
	(*parm) -> parm = atoi(or -> or_value);
	return OK;
}

static int ext_ena (or, parm)
OR_ptr or;
struct type_Ext_ExtendedNetworkAddress **parm;
{
	PE pe;
	struct PSAPaddr *pa;

	*parm = (struct type_Ext_ExtendedNetworkAddress *)
		smalloc (sizeof **parm);
	bzero ((char *)*parm, sizeof **parm);

	while (or) {
		switch (or -> or_type) {
		    case OR_ENA_N:
			if ((*parm) -> offset == 0) {
				(*parm) -> offset = type_Ext_ExtendedNetworkAddress_e163__4__address;
				(*parm) -> un.e163__4__address =
					(struct element_Ext_2 *)
						smalloc (sizeof (struct element_Ext_2));
			}
			(*parm) -> un.e163__4__address ->
				number = STR2QB (or -> or_value);
			break;
			
		    case OR_ENA_S:
			if ((*parm) -> offset == 0) {
				(*parm) -> offset = type_Ext_ExtendedNetworkAddress_e163__4__address;
				(*parm) -> un.e163__4__address =
					(struct element_Ext_2 *)
						smalloc (sizeof (struct element_Ext_2));
			}
			(*parm) -> un.e163__4__address ->
				sub__address = STR2QB(or -> or_value);
			break;
				
		    case OR_ENA_P:
			if ((*parm) -> offset != 0) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Both PSAP & e163 addr in OR"));
				return NOTOK;
			}
			if ((pa = str2paddr (or -> or_value)) == NULLPA)
				return NOTOK;
			if (build_DSE_PSAPaddr (&pe, 1, 0, NULLCP, pa) == NOTOK)
				return NOTOK;
			(*parm) -> offset = type_Ext_ExtendedNetworkAddress_psap__address;
			(*parm) -> un.psap__address = pe;
			break;
		    default:
			break;
		}
		or = or -> or_next;
	}
	return OK;
}

static int find_ext (ext, type)
struct type_MTA_ExtensionAttributes **ext;
int type;
{
	struct type_MTA_ExtensionAttributes *ep;

	for (ep = *ext; ep; ep = ep -> next) {
		if (ep -> ExtensionAttribute -> type == type)
			return 1;
	}
	return 0;
}

static int build_ext_or (ext, or)
struct type_MTA_ExtensionAttributes **ext;
OR_ptr or;
{
	switch (or -> or_type) {
	    case OR_O:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZTeletexOrganizationNameExt,
				   "TeletexOrganizationName",
				   ext_ttx_str,
				   AEXT_TTXORG,
				   ext);
				   
	    case OR_OU:
		if (find_ext (ext, AEXT_TTXOU))
			return OK;
		return build_aext (or,
				   &_ZExt_mod,
				   _ZTeletexOrganizationalUnitNamesExt,
				   "TeletexOrganizationalUnitNames",
				   ext_ttxous,
				   AEXT_TTXOU,
				   ext);
	    case OR_S: /* first one */
	    case OR_G:
	    case OR_I:
	    case OR_GQ:
		if (find_ext (ext, AEXT_TTXPN))
			return OK;
		return build_aext (or,
				   &_ZExt_mod,
				   _ZTeletexPersonalNameExt,
				   "TeletexPersonalName",
				   ext_ttx_pn,
				   AEXT_TTXPN,
				   ext);
	    case OR_DD:
		if (find_ext (ext, AEXT_TTXDD))
			return OK;
		return build_aext (or,
				   &_ZExt_mod,
				   _ZTeletexDomainDefinedAttributesExt,
				   "TeletexDomainDefinedAttributes",
				   ext_ttx_dd,
				   AEXT_TTXDD,
				   ext);
	    case OR_CN:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZCommonNameExt,
				   "CommonName",
				   ext_ps_str,
				   AEXT_CN,
				   ext);
	    case OR_PDSNAME:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPDSNameExt,
				   "PDSName",
				   ext_ps_str,
				   AEXT_PDSNAME,
				   ext);
	    case OR_PD_C:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPhysicalDeliveryCountryNameExt,
				   "PhysicalDeliveryCountryName",
				   ext_psorns_str,
				   AEXT_PDCN,
				   ext);
	    case OR_POSTCODE:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPostalCodeExt,
				   "PostalCode",
				   ext_psorns_str,
				   AEXT_POSTCODE,
				   ext);
	    case OR_PDO_NAME:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPhysicalDeliveryOfficeNameExt,
				   "PhysicalDeliveryOfficeName",
				   ext_pdsparm,
				   AEXT_PDONAME,
				   ext);
	    case OR_PDO_NUM:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPhysicalDeliveryOfficeNumberExt,
				   "PhysicalDeliveryOfficeNumber",
				   ext_pdsparm,
				   AEXT_PDONUMB,
				   ext);
	    case OR_OR_COMPS:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZExtensionORAddressComponentsExt,
				   "ExtensionORAddressComponents",
				   ext_pdsparm,
				   AEXT_ORAC,
				   ext);
	    case OR_PD_PN:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPhysicalDeliveryPersonalNameExt,
				   "PhysicalDeliveryPersonalName",
				   ext_pdsparm,
				   AEXT_PDPN,
				   ext);
	    case OR_PD_O:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPhysicalDeliveryOrganizationNameExt,
				   "PhysicalDeliveryOrganizationName",
				   ext_pdsparm,
				   AEXT_PDORG,
				   ext);
	    case OR_PD_COMPS:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZExtensionPhysicalDeliveryAddressComponentsExt,
				   "ExtensionPhysicalDeliveryAddressComponents",
				   ext_pdsparm,
				   AEXT_EPDAC,
				   ext);
	    case OR_UPA_PA:
		if (find_ext (ext, AEXT_UNFPA))
			return OK; /* already done it */
		return build_aext (or,
				   &_ZExt_mod,
				   _ZUnformattedPostalAddressExt,
				   "UnformattedPostalAddress",
				   ext_upa,
				   AEXT_UNFPA,
				   ext);

	    case OR_STREET:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZStreetAddressExt,
				   "StreetAddress",
				   ext_pdsparm,
				   AEXT_STREET,
				   ext);
	    case OR_PO_BOX:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPostOfficeBoxAddressExt,
				   "PostOfficeBoxAddress",
				   ext_pdsparm,
				   AEXT_POBOX,
				   ext);
	    case OR_PRA:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZPosteRestanteAddressExt,
				   "PosteRestanteAddress",
				   ext_pdsparm,
				   AEXT_POSTERES,
				   ext);
	    case OR_UPN:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZUniquePostalNameExt,
				   "UniquePostalName",
				   ext_pdsparm,
				   AEXT_UNIQPA,
				   ext);
	    case OR_LPA:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZLocalPostalAttributesExt,
				   "LocalPostalAttributes",
				   ext_pdsparm,
				   AEXT_LPA,
				   ext);
	    case OR_ENA_N:
	    case OR_ENA_S:
	    case OR_ENA_P:
		if (find_ext (ext, AEXT_NETADDR))
			return OK;
		return build_aext (or,
				   &_ZExt_mod,
				   _ZExtendedNetworkAddressExt,
				   "ExtendedNetworkAddress",
				   ext_ena,
				   AEXT_NETADDR,
				   ext);
	    case OR_TT:
		return build_aext (or,
				   &_ZExt_mod,
				   _ZTerminalTypeExt,
				   "TerminalType",
				   ext_tt,
				   AEXT_TT,
				   ext);
	}
	return OK;
}
