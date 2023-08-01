/* ut_ext.c: Common Function Routines for external body parts */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/ut_ext.c,v 6.0 1991/12/18 20:16:07 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/ut_ext.c,v 6.0 1991/12/18 20:16:07 jpo Rel $
 *
 * $Log: ut_ext.c,v $
 * Revision 6.0  1991/12/18  20:16:07  jpo
 * Release 6.0
 *
 */


#include "asn1/ExtDef-types.h"
#include "head.h"
#include "asn.h"
#include "oid.h"


/* -- stuff from pepsy -- */
#define EXT_BP			struct type_ExtDef_ExtDefBodyPart
#define EXT_UNIV		struct type_UNIV_EXTERNAL
#define EXT_UNIV_CHOICE		struct choice_UNIV_0
#define Choice_Asn1		choice_UNIV_0_single__ASN1__type



typedef struct oid_struct {
	int	type;
	char	*oid_prm;
	char	*oid_data;
	} TBL_OIDS;


static TBL_OIDS  tbl_oids [] = {/* -- oids for external body parts -- */
	GENERALTEXT,	"2.6.1.11.11",		"2.6.1.4.11",
	0,		0,			0
	};




/* ----------------------------  Begin Routines ----------------------------- */




pe2external (pptr, pe_prm, pe_data, type)
PE	*pptr;
PE	pe_prm;
PE	pe_data;
int	type;
{
	EXT_BP		*ext_bp;
	EXT_UNIV	*ext_prm, *ext_data;
	char		*oid_prm, *oid_data;
	int		retval;

	oid_srch (type, &oid_prm, &oid_data);

	PP_TRACE (("pe2external (%d, %s, %s)", type, oid_prm, oid_data));

	ext_bp = (EXT_BP *) smalloc (sizeof (EXT_BP));
	bzero ((char *) ext_bp, sizeof (EXT_BP));

	malloc_univ_external (&ext_prm, oid_prm, pe_prm);
	malloc_univ_external (&ext_data, oid_data, pe_data);

	ext_bp -> parameters = ext_prm;
	ext_bp -> data = ext_data;

	retval = encode_ExtDef_ExtDefBodyPart
			(pptr, 1, NULLIP, NULLVP, ext_bp);
	if (retval == NOTOK)	pe_done (*pptr);  /* -- exit -- */

	PP_PDUP (ExtDef_ExtDefBodyPart, *pptr,
			"External Body Part", PDU_WRITE);

	free_ExtDef_ExtDefBodyPart(ext_bp);
}




external2pe (pe, pe_prm, pe_data, type)
PE	pe;
PE	*pe_prm;
PE	*pe_data;
int	type;
{
	EXT_BP		*ext_bp;
	EXT_UNIV	*ext_prm, *ext_data;
	char		*oid_prm, *oid_data, *cp; 
	int		retval;

	oid_srch (type, &oid_prm, &oid_data);

	PP_TRACE (("pe2external (%d, %s, %s)", type, oid_prm, oid_data));

	PP_PDUP (ExtDef_ExtDefBodyPart, pe,
				"External Body Part", PDU_READ);

	retval = decode_ExtDef_ExtDefBodyPart
			(pe, 1, NULLIP, NULLVP, &ext_bp);
	if (retval == NOTOK)  pe_done (pe); /* -- exit -- */

	ext_prm = ext_bp -> parameters;
	ext_data = ext_bp -> data;

	if ((ext_prm -> encoding -> offset != Choice_Asn1) || 
	    (ext_data -> encoding -> offset != Choice_Asn1)) {
		PP_LOG (LLOG_EXCEPTIONS, ("Error: No External ASN1 found"));
		exit(1);
	}


	cp = sprintoid (ext_prm  -> direct__reference);
	if (lexequ (cp, oid_prm) != 0) { 
		PP_LOG (LLOG_EXCEPTIONS,
			("Error: Parameter OID mismatch  %s  %s", cp, oid_prm));
		exit (1);
	}

	cp = sprintoid (ext_data -> direct__reference);
	if (lexequ (cp, oid_data) != 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Error: Data OID mismatch  %s  %s", cp, oid_data));
		exit(1);
	}

	*pe_prm  = ext_prm -> encoding -> un.single__ASN1__type;
	*pe_data = ext_data -> encoding -> un.single__ASN1__type;

	ext_prm -> encoding -> un.single__ASN1__type = NULLPE;
	ext_data -> encoding -> un.single__ASN1__type = NULLPE;
	free (oid_prm);
	free (oid_data);
	free_ExtDef_ExtDefBodyPart(ext_bp);
}




/* ----------------------------- Static Routines ---------------------------- */




static oid_srch (type, prm, data)
int	type;
char	**prm;
char	**data;
{
	TBL_OIDS	*tbl = tbl_oids;

	PP_TRACE (("oid_srch (%d)", type));

	for(*prm = *data = NULLCP; tbl->type != NULL; tbl++)
		if (type == tbl->type) {
			*prm  = strdup (tbl->oid_prm);
			*data = strdup (tbl->oid_data);
		}


	if (*prm == NULLCP && *data == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, ("Error: Unknown OIDS for %d", type));
		exit(1);
	}
}




malloc_univ_external (pptr, oid, pe)
EXT_UNIV	**pptr;
char		*oid;
PE		pe;
{
	EXT_UNIV	*ExtUniv;
	int		n;

	n = sizeof (EXT_UNIV);
	ExtUniv = (EXT_UNIV *) smalloc (n);
	bzero ((char *) ExtUniv, n);

	n = sizeof (EXT_UNIV_CHOICE);
	ExtUniv -> encoding = (EXT_UNIV_CHOICE *) smalloc (n);
	bzero ((char *) ExtUniv -> encoding, n);

	ExtUniv -> direct__reference = oid_cpy (str2oid (oid));
	ExtUniv -> encoding -> offset = Choice_Asn1;
	ExtUniv -> encoding -> un.single__ASN1__type = pe;

	*pptr = ExtUniv;
}
