/* or_tables.c: various tables of values */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_tables.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_tables.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_tables.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */


#include "config.h"
#include "or.h"
#include <isode/cmd_srch.h>

typestruct typetab[] = {
    "C",        OR_C,           OR_ENC_PS,
    "ADMD",     OR_ADMD,        OR_ENC_PS,
    "A",     	OR_ADMD,        OR_ENC_PS,
    "PRMD",     OR_PRMD,        OR_ENC_PS,
    "P",     	OR_PRMD,        OR_ENC_PS,
    "X121",     OR_X121,        OR_ENC_NUM,
    "X.121",    OR_X121,        OR_ENC_NUM,
    "T-ID",     OR_TID,         OR_ENC_PS,
    "O",        OR_O,           OR_ENC_PS,
    "OU",       OR_OU,          OR_ENC_PS,
    "UA-ID",    OR_UAID,        OR_ENC_NUM,
    "N-ID",    OR_UAID,        OR_ENC_NUM,
    "S",        OR_S,           OR_ENC_PS,
    "G",        OR_G,           OR_ENC_PS,
    "I",        OR_I,           OR_ENC_PS,
    "GQ",       OR_GQ,          OR_ENC_PS,
    "Q",       OR_GQ,          OR_ENC_PS,
    "DD",	OR_DD,		OR_ENC_PS,
    0,		0,		0
};

typestruct typetab88[] = {
    "C",        OR_C,           OR_ENC_PS,
    "ADMD",     OR_ADMD,        OR_ENC_PS,
    "A",     	OR_ADMD,        OR_ENC_PS,
    "PRMD",     OR_PRMD,        OR_ENC_PS,
    "P",	OR_PRMD,        OR_ENC_PS,
    "X121",     OR_X121,        OR_ENC_NUM,
    "X.121",     OR_X121,        OR_ENC_NUM,
    "T-ID",     OR_TID,         OR_ENC_PS,
    "O",        OR_O,           OR_ENC_TTX_AND_OR_PS,
    "OU",       OR_OU,          OR_ENC_TTX_AND_OR_PS,
    "OU1",	OR_OU1,		OR_ENC_TTX_AND_OR_PS,
    "OU2",	OR_OU2,		OR_ENC_TTX_AND_OR_PS,
    "OU3",	OR_OU3,		OR_ENC_TTX_AND_OR_PS,
    "OU4",	OR_OU4,		OR_ENC_TTX_AND_OR_PS,
    "UA-ID",    OR_UAID,        OR_ENC_NUM,
    "N-ID",     OR_UAID,        OR_ENC_NUM,
    "S",        OR_S,           OR_ENC_TTX_AND_OR_PS,
    "G",        OR_G,           OR_ENC_TTX_AND_OR_PS,
    "I",        OR_I,           OR_ENC_TTX_AND_OR_PS,
    "GQ",       OR_GQ,          OR_ENC_TTX_AND_OR_PS,
    "Q",       OR_GQ,          OR_ENC_TTX_AND_OR_PS,
    "CN",	OR_CN,		OR_ENC_TTX_AND_OR_PS,
    "PD-SYSTEM",OR_PDSNAME,	OR_ENC_PS,
    "PD-C",	OR_PD_C,	OR_ENC_PS,
    "POSTCODE",	OR_POSTCODE,	OR_ENC_PS,
    "PD-OFFICE",OR_PDO_NAME,	OR_ENC_TTX_AND_OR_PS,
    "PD-OFFICE-NUM",OR_PDO_NUM,	OR_ENC_TTX_AND_OR_PS,
    "PD-OFFICE-NUMBER",OR_PDO_NUM,	OR_ENC_TTX_AND_OR_PS,
    "PD-OFFICE NUMBER",OR_PDO_NUM,	OR_ENC_TTX_AND_OR_PS,
    "PD-EXT-D",	OR_OR_COMPS,	OR_ENC_TTX_AND_OR_PS,
    "PD-PN", 	OR_PD_PN,	OR_ENC_TTX_AND_OR_PS,
    "PD-O",	OR_PD_O,	OR_ENC_TTX_AND_OR_PS,
    "PD-EXT-LOC",OR_PD_COMPS,	OR_ENC_TTX_AND_OR_PS,
    "PD-ADDRESS",OR_UPA_PA,	OR_ENC_TTX_AND_OR_PS,
    "STREET",	OR_STREET,	OR_ENC_TTX_AND_OR_PS,
    "PO-BOX",	OR_PO_BOX,	OR_ENC_TTX_AND_OR_PS,
    "POSTE-RESTANTE",OR_PRA,	OR_ENC_TTX_AND_OR_PS,
    "PD-UNIQUE",OR_UPN,		OR_ENC_TTX_AND_OR_PS,
    "PD-LOCAL",	OR_LPA,		OR_ENC_TTX_AND_OR_PS,
    "NET-NUM",	OR_ENA_N,	OR_ENC_NUM,
    "NET-SUB",	OR_ENA_S,	OR_ENC_NUM,
    "NET-PSAP",	OR_ENA_P,	OR_ENC_PSAP,
    "NET-TTYPE",OR_TT,		OR_ENC_INT,
    "DD",	OR_DD,		OR_ENC_PS,
    0,          0,              0
    };



char    _pstable[] = {
    0, 0, 0, 0,                         /* 000-003      nul             */
    0, 0, 0, 0,                         /* 004-007                      */
    0, 0, 0, 0,                         /* 010-013      bs tab lf       */
    0, 0, 0, 0,                         /* 014-017                      */
    0, 0, 0, 0,                         /* 020-023                      */
    0, 0, 0, 0,                         /* 024-027                      */
    0, 0, 0, 0,                         /* 030-033                      */
    0, 0, 0, 0,                         /* 034-037                      */
    CHR_PS|CHR_NS, 0, 0, 0,             /* 040-043      sp !  "  #      */
    0, 0, 0, CHR_PS,                    /* 044-047      $  %  &  '      */
    CHR_PS, CHR_PS, 0, CHR_PS,          /* 050-053      ( )  *  +       */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 054-057      ,  -  .  /      */
    CHR_PS|CHR_NS, CHR_PS|CHR_NS, CHR_PS|CHR_NS, CHR_PS|CHR_NS,
					/* 060-063      0 1  2  3       */
    CHR_PS|CHR_NS, CHR_PS|CHR_NS, CHR_PS|CHR_NS, CHR_PS|CHR_NS,     
					/* 014-067      4  5  6  7      */
    CHR_PS|CHR_NS, CHR_PS|CHR_NS, CHR_PS, 0,
					/* 070-073      8  9  :  ;      */
    0, CHR_PS, 0, CHR_PS,               /* 074-077      <  =  >  ?      */
    0, CHR_PS, CHR_PS, CHR_PS,          /* 100-103      @  A  B  C      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 014-107      D  E  F  G      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 110-114      H  I  J  K      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 115-117      L  M  N  O      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 120-123      P  Q  R  S      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 124-127      T  U  V  W      */
    CHR_PS, CHR_PS, CHR_PS, 0,          /* 130-133      X  Y  Z [      */
    0, 0, 0, 0,                         /* 134-137      \  ]  ^  _      */
    0, CHR_PS, CHR_PS, CHR_PS,          /* 140-143      `  a  b  c      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 144-147      d  e  f  g      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 150-153      h  i  j  k      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 154-157      l  m  n  o      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 160-163      p  q  r  s      */
    CHR_PS, CHR_PS, CHR_PS, CHR_PS,     /* 164-167      t  u  v  w      */
    CHR_PS, CHR_PS, CHR_PS, 0,          /* 170-173      x  y  z  {      */
    0, 0, 0, 0,                         /* 174-177      |  }  ~  del    */
    };





CMD_TABLE ortbl_ddvalid[] = { /* ddvalid keys */
	"RFC-822",      OR_DDVALID_RFC822,
	"X400-88",	OR_DDVALID_X40088,
#ifndef STRICT_1148
	"JNT-MAIL",      OR_DDVALID_JNT,
#endif
	"UUCP",         OR_DDVALID_UUCP,
	"LIST",         OR_DDVALID_LIST,
	"ROLE",         OR_DDVALID_ROLE,
	"FAX",		OR_DDVALID_FAX,
	"ATTN",		OR_DDVALID_ATTN,
	0,              -1
	};


OR_upperbound ortbl_88_ubs[] = { /* upperbounds on 88 components */
{OR_C, 		OR_UB_C_ALPHA},
{OR_ADMD, 	OR_UB_MD},
{OR_PRMD, 	OR_UB_MD},
{OR_O,		OR_UB_O},
{OR_OU,		OR_UB_OU},
{OR_OU1,	OR_UB_OU},
{OR_OU2,	OR_UB_OU},
{OR_OU3,	OR_UB_OU},
{OR_OU4,	OR_UB_OU},
{OR_X121,	OR_UB_X121},
{OR_TID,	OR_UB_TID},
{OR_UAID,	OR_UB_UAID},
{OR_S,		OR_UB_S},
{OR_G,		OR_UB_G},
{OR_I,		OR_UB_I},
{OR_GQ,		OR_UB_GQ},
{OR_CN,		OR_UB_CN},
{OR_PDSNAME,	OR_UB_PDSNAME},
{OR_PD_C,	OR_UB_PDS_PARAM},
{OR_POSTCODE,	OR_UB_POSTCODE},
{OR_PDO_NAME,	OR_UB_PDS_PARAM},
{OR_PDO_NUM,	OR_UB_PDS_PARAM},
{OR_OR_COMPS,	OR_UB_OR_COMPS},
{OR_PD_PN,	OR_UB_PDS_PARAM},
{OR_PD_O,	OR_UB_PDS_PARAM},
{OR_PD_COMPS,	OR_UB_PDS_PARAM},
{OR_UPA_PA,	OR_UB_UPA_PA},
{OR_STREET,	OR_UB_PDS_PARAM},
{OR_PO_BOX,	OR_UB_PDS_PARAM},
{OR_PRA,	OR_UB_PDS_PARAM},
{OR_UPN,	OR_UB_PDS_PARAM},
{OR_LPA,	OR_UB_PDS_PARAM},
{OR_ENA_N,	OR_UB_ENA_N},
{OR_ENA_S,	OR_UB_ENA_S},
{OR_ENA_P,	-1},
{OR_TT,		OR_UB_INT_OPTS},
{OR_DD,		OR_UB_DDA_VALUE},
{0,		-1}
};

OR_upperbound ortbl_84_ubs[] = { /* upperbounds on 84 components */
{OR_C, 		OR_UB_C_ALPHA},
{OR_ADMD, 	OR_UB_MD},
{OR_PRMD, 	OR_UB_MD},
{OR_O,		OR_UB_O},
{OR_OU,		OR_UB_OU},
{OR_OU1,	OR_UB_OU},
{OR_OU2,	OR_UB_OU},
{OR_OU3,	OR_UB_OU},
{OR_OU4,	OR_UB_OU},
{OR_X121,	OR_UB_X121},
{OR_TID,	OR_UB_TID},
{OR_UAID,	OR_UB_UAID},
{OR_S,		OR_UB_S},
{OR_G,		OR_UB_G},
{OR_I,		OR_UB_I},
{OR_GQ,		OR_UB_GQ},
{0,		-1}
};
