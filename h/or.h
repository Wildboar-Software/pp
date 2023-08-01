/* or.h: o/r name package */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/or.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: or.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_OR
#define _H_OR

#include <isode/psap.h>
#include <isode/cmd_srch.h>
#include <isode/quipu/name.h>
/*
Definitions for OR Name structure

NOTE - that the trees are strucutred with the MOST significant part at the top
*/

/* SEK temp notes:

    OR/STD  will have a matrix of or_part/valid encodings.
    If a Char* is present, this identifies the valid key

    84/88 distinction will be done as address subtyping.
    std2or  needs to have a flag to reflect this.   

    Needs to be a routine to downgrade addresses 88 -> 84.  
    First cut can work by discard.
*/

/* upperbound on number of organizational units allowed in an address */
#define	MAX_OU	4

/* upperbound on number of domain defined attributes allowed in an address */
#define MAX_DDA	4

struct or_part
{
int		   or_type;
#define OR_C		1
#define OR_ADMD		2
#define OR_PRMD		3
#define OR_O		4
#define OR_OU		5
#define OR_OU1		6
#define OR_OU2		7
#define OR_OU3		8
#define OR_OU4		9
#define OR_X121		10
#define OR_TID		11
#define OR_UAID		12
#define OR_S		13
#define OR_G		14
#define OR_I		15
#define OR_GQ		16
#define OR_CN		17
#define OR_PDSNAME	18
#define OR_PD_C		19
#define OR_POSTCODE	20
#define OR_PDO_NAME	21
#define OR_PDO_NUM	22
#define OR_OR_COMPS	23
#define OR_PD_PN	24
#define OR_PD_O		25
#define OR_PD_COMPS	26
#define OR_UPA_PA	27
#define OR_STREET	28
#define OR_PO_BOX	29
#define OR_PRA		30
#define OR_UPN		31
#define OR_LPA		32
#define OR_ENA_N	33
#define OR_ENA_S	34
#define OR_ENA_P	35
#define OR_TT		36
#define OR_DD		99
int		   or_encoding;
#define OR_ENC_PS	1
#define OR_ENC_TTX	2
#define OR_ENC_NUM	3
#define OR_ENC_TTX_AND_OR_PS	4 /* use 987 string encoding */
#define OR_ENC_INT	5
#define OR_ENC_PSAP	6	/* ISODE/SEK string form */
#define OR_ENC_PP	7	/* internal PP encoding used for */
				/* missing OR components in rfc1148 */
				/* conversion tables */
char		   *or_ddname;
char		   *or_value;
struct or_part	   *or_prev;
struct or_part	   *or_next;
};

#define or_or2rfc(a,b)	or_or2rfc_aux ((a), (b), 0)	/* backwards compat */
#define or_rfc2or(a,b)	or_rfc2or_aux ((a), (b), 1)

#define OR_ISLOCAL	01
#define OR_ISMTA	02

#define CHR_PS		01	 /* Printable String */
#define CHR_NS		02	 /* Numeric String */

typedef struct or_part	*OR_ptr;

#define NULLOR		((OR_ptr)0)

typedef struct ORName {
	OR_ptr	on_or;
	DN	on_dn;
} ORName;

#define NULLORName	((ORName *) 0)

ORName *pe2orn ();
ORName	*orname2orn ();
OR_ptr	oradr2ora ();
PE	orn2pe ();
PE	orname2pe ();
OR_ptr	pe2ora ();
PE	ora2pe ();
PE	oradr2pe ();
void	ORName_free ();

extern char _pstable[];

#define or_isps(c)	(_pstable[c]&CHR_PS)
#define or_isns(c)	(_pstable[c]&CHR_NS)

typedef struct tpstruct {
    char	    *ty_string;
    int		    ty_int;
    int		    ty_charset;
} typestruct;

typedef struct or_upperbound {
	int	or_type;
	int	or_upperbound;
/* 0 == not valid, -1 == no upperbound */
} OR_upperbound;

/* upperbounds as defined in x.411 */
#define OR_UB_C_ALPHA	2
#define OR_UB_C_NUM	3
#define OR_UB_MD	16
#define OR_UB_O		64
#define OR_UB_OU	32
#define OR_UB_X121	15
#define OR_UB_TID	24
#define OR_UB_UAID	32
#define OR_UB_S		40
#define OR_UB_G		16
#define OR_UB_I		5
#define OR_UB_GQ	3
#define OR_UB_CN	64
#define OR_UB_PDSNAME	16
#define	OR_UB_PDS_PARAM	30
#define OR_UB_POSTCODE	16
#define OR_UB_OR_COMPS	256
#define OR_UB_UPA_PA	180
#define OR_UB_ENA_N	15
#define OR_UB_ENA_S	40
#define OR_UB_INT_OPTS	246
#define OR_UB_DDA_VALUE	128
#define OR_UB_DDA_TYPE	8

extern typestruct typetab[];
extern typestruct typetab88[];

/* valid DD's */
#define	 OR_DDVALID_RFC822		1
#define	 OR_DDVALID_JNT			2
#define	 OR_DDVALID_UUCP		3
#define	 OR_DDVALID_LIST		4
#define	 OR_DDVALID_ROLE		5
#define	 OR_DDVALID_FAX			6
#define	 OR_DDVALID_ATTN		7
#define	 OR_DDVALID_X40088		8

/* forms of address */
#define OR_FORM_NONE			0
#define OR_FORM_MNEM			1
#define OR_FORM_NUMR			2
#define OR_FORM_POST			3
#define OR_FORM_TERM			4

extern CMD_TABLE ortbl_ddvalid[];
extern OR_ptr	loc_ortree;

extern	OR_ptr	pe2or();		/* convert PE to OR struct */
extern	PE	or2pe();		/* convert OR struct to PE */
extern OR_ptr	or_add();		/* add or_part to place in tree */
extern OR_ptr	or_add_t61 ();		/* add in a T61 address attribute */
extern OR_ptr	or_default();		/* add def vals to partial OR Name */
extern OR_ptr	or_default_aux();	/* add given vals to partial OR Name */
extern OR_ptr	or_dmn2or();		/* OR dmn string -> structure */
extern OR_ptr	or_dup();		/* duplicate or_part */
extern OR_ptr	or_tdup();		/* duplicate or tree */
extern OR_ptr	or_find();		/* find or_part of type in tree */
extern OR_ptr	or_lastpart();		/* point to last component */
extern OR_ptr	or_locate();		/* find 1st part of any component */
extern OR_ptr	or_new();		/* create and fill new or_part */
extern OR_ptr	or_new_aux();		/* create and fill new or_part */
extern OR_ptr	or_std2or();		/* OR std string -> structure */
extern char	*or_value();		/* find a type and return it's val */
extern char*	or_type2name();		/* get string of part type */
extern int	or_cmp();		/* test if two or_parts are equal */
extern void	or_free();		/* free structure recursively */
extern int	or_init();		/* various global inits */
extern void	or_myinit();		/* various global inits */
extern int	or_name2type();		/* get type of string */
extern void	or_or2dmn();		/* structure to OR dmn string */
extern int	or_or2rfc_aux ();
extern int	or_rfc2or_aux ();	/* map 822 form to tree */
extern void	or_or2std();		/* structure to OR std string */
extern int	or_utilinit();		/* various global inits */
extern int	or_blank ();
extern OR_ptr	or_buildpn ();		/* construct a personal name */
extern int	or_getpn ();
extern void	qstrcat ();
extern void	dstrcat ();
extern void	or_myinit ();
extern void	or_t61encode ();
extern struct qbuf *or_t61decode ();
extern int	or_ddname_chk ();
extern int	or_ddvalid_chk ();
extern int	or_gettoken ();
extern int	or_isdomsyntax ();
extern void	or_ps2asc ();
extern void	or_downgrade();		/* downgrades 88 to 84 */
extern int	or_or2rbits ();
extern int	rfc_space ();
extern int	or_domain2or();
extern int	or_local2or();
extern int	or_asc2ps ();
extern int	or_append ();
extern struct qbuf *or_getps ();
extern struct qbuf *or_getttx ();
extern void or_chk_admd ();
extern int	or_check_upper();
extern int	or_chk_ubs();
#endif
