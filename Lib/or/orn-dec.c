/* orn-dec.c: routines to decode ORNames */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/orn-dec.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/orn-dec.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: orn-dec.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */

#include "MTA-types.h"
#include "Ext-types.h"

#include "util.h"
#include "or.h"
#include "extension.h"
#include "IOB-types.h"
#include <isode/isoaddrs.h>

static int do_or_address (), do_addr_ext(), decode_adr_ext();

struct type_IOB_ORName *pe2orname (pe)
PE	pe;
{
	struct type_IOB_ORName *orn;
	
	PP_PDUP (IOB_ORName, pe, "ORName", PDU_READ);

	if (decode_IOB_ORName (pe, 1, NULLIP, NULLVP, &orn) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't decode ORName [%s]",
					  PY_pepy));
		return NULL;
	}
	return orn;
}

ORName *orname2orn(orn)
struct type_IOB_ORName *orn;
{
	ORName	*orname;

	if (orn == (struct type_IOB_ORName *) NULL)
		return NULLORName;

	orname = (ORName *)malloc (sizeof *orname);
	if (do_or_address (&orname->on_or,
			   orn -> standard__attributes,
			   orn -> domain__defined,
			   orn -> extension__attributes) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS, ("Failed to decipher ORaddress"));
		free ((char *)orname);
		return NULLORName;
	}

	if (orn->directory__name) {
		PE dn = orn -> directory__name;

		if (dn -> pe_form != PE_FORM_CONS){
			PP_LOG (LLOG_EXCEPTIONS, ("DN not a constructor form"));
			ORName_free (orname);
			return NULLORName;
		}
		if (decode_IF_Name (dn -> pe_cons, 1,
				    NULLVP, NULLIP,
				    &orname -> on_dn) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS, ("Can't decode DN [%s]",
						  PY_pepy));
			ORName_free(orname);
			return NULLORName;
		}
	} else
		orname -> on_dn = NULL;
	return orname;
}

ORName	*pe2orn(pe)
PE	pe;
{
	struct type_IOB_ORName	*orn;
	ORName	*orname;

	if ((orn = pe2orname(pe)) == NULL)
		return NULLORName;

	orname = orname2orn(orn);
	free_IOB_ORName (orn);
	return orname;
}

struct type_MTA_ORAddress *pe2oradr (pe)
PE	pe;
{
	struct type_MTA_ORAddress	*ora;

	PP_PDUP (MTA_ORAddress, pe, "ORAddress", PDU_READ);

	if (decode_MTA_ORAddress (pe, 1, NULLIP, NULLVP, &ora) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't decode ORAddress [%s]",
					  PY_pepy));
		return NULL;
	}
	return ora;
}

OR_ptr oradr2ora (ora)
struct type_MTA_ORAddress	*ora;
{
	OR_ptr	or;
	if(do_or_address (&or,
			  ora -> standard__attributes,
			  ora -> domain__defined__attributes,
			  ora -> extension__attributes) == NOTOK)
		or = NULLOR;
	return or;
}

OR_ptr pe2ora (pe)
PE	pe;
{
	struct type_MTA_ORAddress *ora;
	OR_ptr or;
	
	if ((ora = pe2oradr(pe)) == NULL)
		return NULLOR;
	or = oradr2ora(ora);
	free_MTA_ORAddress (ora);
	return or;
}

static int do_or_address (orp, std, dd, ext)
OR_ptr *orp;
struct type_MTA_StandardAttributes *std;
struct type_MTA_DomainDefinedAttributes *dd;
struct type_MTA_ExtensionAttributes *ext;
{
	struct type_MTA_OrganizationalUnitNames *ou;
	OR_ptr	tree = NULLOR, or;
	char	*p;
	struct qbuf *qb;

	*orp = NULLOR;
#define do_component(x,y,enc)	\
	if (x) {\
		or = or_new_aux (y, NULLCP, p = qb2str (x), (enc));\
		free (p); \
		if (or == NULLOR) return NOTOK; \
		if (or_append (&tree, or) != OK)\
			return NOTOK; \
	}

	if (std) {
		if (std -> country__name) {
			qb = (std -> country__name -> offset ==
			      type_MTA_CountryName_x121__dcc__code) ?
				      std -> country__name -> un.x121__dcc__code :
			std -> country__name ->
				un.iso__3166__alpha2__code;
			do_component (qb, OR_C, OR_ENC_PS);
		}
		if (std -> administration__domain__name) {
			qb = (std -> administration__domain__name -> offset ==
			      type_MTA_AdministrationDomainName_numeric) ?
				      std -> administration__domain__name ->
					      un.numeric :
			std -> administration__domain__name ->
				un.printable;
			do_component (qb, OR_ADMD, OR_ENC_PS);
		}
		do_component (std -> network__address, OR_X121, OR_ENC_NUM);
		do_component (std -> terminal__identifier, OR_TID, OR_ENC_PS);
		if (std -> private__domain__name) {
			qb = (std -> private__domain__name -> offset ==
			      type_MTA_PrivateDomainName_numeric) ?
				      std -> private__domain__name ->
					      un.numeric :
				      std -> private__domain__name ->
					      un.printable;
			do_component (qb, OR_PRMD, OR_ENC_PS);
		}
		do_component (std -> organization__name, OR_O, OR_ENC_PS);
		do_component (std -> numeric__user__identifier, OR_UAID, OR_ENC_NUM);
		if (std -> personal__name) {
			do_component (std -> personal__name -> surname, OR_S, OR_ENC_PS);
			do_component (std -> personal__name -> given__name,
				      OR_G, OR_ENC_PS);
			do_component (std -> personal__name -> initials, OR_I, OR_ENC_PS);
			do_component (std -> personal__name ->
				      generation__qualifier, OR_GQ, OR_ENC_PS);
		}
		for (ou = std -> organizational__unit__names;
		     ou; ou = ou -> next) {
			do_component (ou -> OrganizationUnitName, OR_OU, OR_ENC_PS);
		}
	}
#undef do_component
	for ( ; dd; dd = dd -> next) {
		char *q;

		or = or_new_aux (OR_DD,
			     p = qb2str(dd -> DomainDefinedAttribute -> type),
			     q = qb2str (dd -> DomainDefinedAttribute -> value),
			     OR_ENC_PS);
		free (q);
		free (p);
		if (or == NULLOR) return NOTOK;
		if (or_append (&tree, or) != OK)
			return NOTOK;;
	}

	for (; ext; ext = ext -> next)
		if (do_addr_ext (&tree, ext -> ExtensionAttribute) == NOTOK)
			return NOTOK;
	*orp = tree;
	return OK;
}

static int do_addrext_ps (treep, type, qb)
OR_ptr *treep;
int type;
struct qbuf *qb;
{
	char *p;
	OR_ptr or;
	int retval;

	p = qb2str (qb);
	or = or_new_aux (type, NULLCP, p, OR_ENC_PS);
	if (or == NULLOR) return NOTOK;
	retval = or_append (treep, or);
	free (p);
	return retval;
}

static int do_addr_tt (treep, type, tt)
OR_ptr *treep;
int type;
struct type_Ext_TerminalType *tt;
{
	char buf[64];
	OR_ptr or;

	(void) sprintf (buf, "%d", tt -> parm);
	or = or_new_aux (type, NULLCP, buf, OR_ENC_INT);
	if (or == NULLOR) return NOTOK;
	return or_append (treep, or);
}

static int do_addrext_ttx (treep, type, qb)
OR_ptr *treep;
int type;
struct qbuf *qb;
{
	char *p;
	OR_ptr or;
	int retval = OK;

	p = qb2str (qb);
	if ((or = or_add_t61 (*treep, type, (unsigned char *)p,
			      qb -> qb_len, 0)) == NULL)
		retval = NOTOK;
	else
		*treep = or;
	free (p);
	return retval;
}

static int do_addrext_psorns (treep, type, parm)
OR_ptr *treep;
int type;
struct type_Ext_PostalCode *parm;
{
	char	*p;
	OR_ptr or;
	int retval = OK;

	switch (parm -> offset) {
	    case type_Ext_PostalCode_numeric__code:
		p = qb2str (parm -> un.numeric__code);
		or = or_new_aux (type, NULLCP, p, OR_ENC_NUM);
		if (or == NULLOR)
			return NOTOK;
		retval = or_append (treep, or);
		break;
	    case type_Ext_PostalCode_printable__code:
		p = qb2str (parm -> un.printable__code);
		or = or_new_aux (type, NULLCP, p, OR_ENC_PS);
		if (or == NULLOR)
			return NOTOK;
		retval = or_append (treep, or);
	    default:
		return NOTOK;
	}
	free (p);
	return retval;
}

static int do_addrext_pds (treep, type, parm)
OR_ptr *treep;
int type;
struct type_Ext_PDSParameter *parm;
{
	int retval;
	OR_ptr or;
	char *p;

	if (parm -> printable__string) {
		p = qb2str (parm -> printable__string);
		or = or_new_aux (type, NULLCP, p, OR_ENC_PS);
		free (p);
		if (or == NULLOR) return NOTOK;
		retval = or_append (treep, or);
		if (retval != OK)
			return retval;
	}
	if (parm -> teletex__string) {
		p = qb2str (parm -> teletex__string);
		or = or_add_t61 (*treep, type, (unsigned char *)p, 
				 parm -> teletex__string -> qb_len, 0);
		free (p);
		if (or == NULL)
			return NOTOK;
		*treep = or;
	}
	return OK;
}

/* ARGSUSED */
static int do_addrext_ena (treep, type, parm)
OR_ptr *treep;
int type;
struct type_Ext_ExtendedNetworkAddress *parm;
{
	OR_ptr or;
	struct PSAPaddr *pa;

	switch (parm -> offset) {
	    case type_Ext_ExtendedNetworkAddress_e163__4__address:
		if (parm -> un.e163__4__address -> number) {
			char *p = qb2str (parm -> un.e163__4__address -> number);
			or = or_new_aux (OR_ENA_N, NULLCP, p, OR_ENC_NUM);
			free (p);
			if (or == NULLOR) return NOTOK;
			if (or_append (treep, or) == NOTOK)
				return NOTOK;
		}
		if (parm -> un.e163__4__address -> sub__address) {
			char *p = qb2str (parm -> un.e163__4__address -> sub__address);
			or = or_new_aux (OR_ENA_S, NULLCP, p, OR_ENC_NUM);
			free (p);
			if (or == NULLOR) return NOTOK;
			if (or_append (treep, or) == NOTOK)
				return NOTOK;
		}
		break;

	    case type_Ext_ExtendedNetworkAddress_psap__address:
		if (parse_DSE_PSAPaddr (parm -> un.psap__address,
					1, NULLIP, NULLVP, &pa) == NOTOK)
			return NOTOK;
		or = or_new_aux (OR_ENA_P, NULLCP, 
				 paddr2str(pa, NULLNA), OR_ENC_PSAP);
		if (or == NULLOR) return NOTOK;
		if (or_append (treep, or) == NOTOK)
			return NOTOK;
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown ena type %d",
					  parm -> offset));
		return NOTOK;
	}
	return OK;
}

static int do_addr_ext (treep, ext)
OR_ptr	*treep;
struct type_MTA_ExtensionAttribute *ext;
{
	PY_pepy[0] = 0;

#define dec_axt(off, type, fnx, lab) \
	decode_adr_ext (ext -> value, &_ZExt_mod,\
			(off), (type), (fnx), (lab), treep)

	switch (ext -> type) {
	    case AEXT_CN:	/* common-name */
		return dec_axt (_ZCommonNameExt, OR_CN, do_addrext_ps,
				"CommonName");

	    case AEXT_TTXCN:	/* teletex-common-name */
		return dec_axt (_ZTeletexCommonNameExt, OR_CN,
				do_addrext_ttx, "TelexCommonName");
				       
	    case AEXT_TTXORG:		/* teletex-organization */
		return dec_axt (_ZTeletexOrganizationNameExt, OR_O,
				do_addrext_ttx, "TeletexOrganizationName");

	    case AEXT_TTXPN:		/* teletex-personal-name */
	    case AEXT_TTXOU:		/* teletex-organizational-unit-names */
	    case AEXT_TTXDD:		/* teletex-domain-defined-attributes */
		break;
	    case AEXT_PDSNAME:		/* pds-name */
		return dec_axt (_ZPDSNameExt, OR_PDSNAME,
				do_addrext_ps, "PDSName");

	    case AEXT_PDCN:	/* physical-delivery-country-name */
		return dec_axt (_ZPhysicalDeliveryCountryNameExt, OR_PD_C,
				do_addrext_psorns,
				"PhysicalDeliveryCountryName");

	    case AEXT_POSTCODE:	/* postal-code */
		return dec_axt (_ZPostalCodeExt, OR_POSTCODE,
				do_addrext_psorns, "PostalCode");

	    case AEXT_PDONAME:	/* physical-delivery-office-name */
		return dec_axt (_ZPhysicalDeliveryOfficeNameExt,
				OR_PDO_NAME, do_addrext_pds,
				"PhysicalDeliveryOfficeName");
				       
	    case AEXT_PDONUMB:	/* physical-delivery-office-number */
		return dec_axt (_ZPhysicalDeliveryOfficeNumberExt,
				OR_PDO_NUM, do_addrext_pds,
				"PhysicalDeliveryOfficeNumber");

	    case AEXT_ORAC:	/* extension-OR-address-components */
		return dec_axt (_ZExtensionORAddressComponentsExt,
				OR_OR_COMPS, do_addrext_pds,
				"ExtensionORAddressComponents");
				       
	    case AEXT_PDPN:	/* physical-delivery-personal-name */
		return dec_axt (_ZPhysicalDeliveryPersonalNameExt,
				OR_PD_PN, do_addrext_pds,
				"PhysicalDeliveryPersonalName");

	    case AEXT_PDORG:	/* physical-delivery-organization-name */
		return dec_axt (_ZPhysicalDeliveryOrganizationNameExt,
				OR_PD_O, do_addrext_pds,
				"PhysicalDeliveryOrganizationName");

	    case AEXT_EPDAC:	/* extension-physical-delivery-address-components */
		return dec_axt (_ZExtensionPhysicalDeliveryAddressComponentsExt,
				OR_PD_COMPS, do_addrext_pds,
				"ExtensionPhysicalDeliveryAddressComponents");

	    case AEXT_UNFPA:	/* unformatted-postal-address */
		break;

	    case AEXT_STREET:	/* street-address */
		return dec_axt (_ZStreetAddressExt, OR_STREET,
				do_addrext_pds, "StreetAddress");

	    case AEXT_POBOX:	/* post-office-box-address */
		return dec_axt (_ZPostOfficeBoxAddressExt, OR_PO_BOX,
				do_addrext_pds, "PostOfficeBoxAddress");

	    case AEXT_POSTERES:	/* poste-restante-address */
		return dec_axt (_ZPosteRestanteAddressExt,
				OR_PRA, do_addrext_pds,
				"PosteRestanteAddress");

	    case AEXT_UNIQPA:	/* unique-postal-address */
		return dec_axt (_ZUniquePostalNameExt,
				OR_UPN, do_addrext_pds,
				"UniquePostalName");

	    case AEXT_LPA:	/* local-postal-attribute */
		return dec_axt (_ZLocalPostalAttributesExt,
				OR_LPA, do_addrext_pds,
				"LocalPostalAttributes");

	    case AEXT_NETADDR:	/* extended-network-address */
		return dec_axt (_ZExtendedNetworkAddressExt,
				OR_ENA_S, do_addrext_ena,
				"ExtendedNetworkAddress");

	    case AEXT_TT:	/* terminal-type */
		return dec_axt (_ZTerminalTypeExt, OR_TT,
				do_addr_tt, "TerminalType");
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown address extension %d",
					  ext -> type));
		break;
	}
	return OK;
}

static int decode_adr_ext (petag, mod, offset, type, fnx, label, treep)
PE	petag;
modtyp *mod;
int	offset;
int	type;
IFP	fnx;
char	*label;
OR_ptr	*treep;
{
	caddr_t genp;
	int retval;
	PE	pe;

	if (petag -> pe_class != PE_CLASS_CONT &&
	    petag -> pe_form != PE_FORM_CONS &&
	    petag -> pe_id != 1) {
		PP_LOG (LLOG_EXCEPTIONS, ("Missing [1] tag in adr extension"));
		return NOTOK;
	}
	pe = petag -> pe_cons;	/* strip off the tag */
#if PP_DEBUG > 0
        if (pp_log_norm -> ll_events & LLOG_PDUS)
                pvpdu (pp_log_norm, offset, mod,
		       pe, label, PDU_READ);
#endif
	if (dec_f (offset, mod, pe, 1, NULLIP, NULLVP, &genp) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't parse %s [%s]",
					  label, PY_pepy));
		return NOTOK;
	}

	if ((retval = (*fnx) (treep, type, genp)) != OK)
		return retval;

	(void) fre_obj ((char *)genp, mod -> md_dtab[offset], mod, 1);

	return OK;
}
