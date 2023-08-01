/* tb_a.c: Address tables for Address Lines in ADDR Control Files */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_a.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_a.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_a.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "tb_a.h"
#include 	"adr.h"
#include        <isode/cmd_srch.h>



/* ------------------------------------------------------------------------ */



CMD_TABLE  atbl_ctrl_addrs[] = {  /* Env-ctrl-addresses */
	"Origs",        AD_ORIGINATOR,
	"Recip",        AD_RECIPIENT,
	0,              -1
	};


/* sorted - do not reorder */
CMD_TABLE  atbl_addr[] = {  /* address-lines */
	"content",      AD_CONTENT,
	"content_integrity",	AD_CONTENT_INTEGRITY,
	"content_integrity_crit",	AD_CONTENT_INTEGRITY_CRIT,
	"dn",		AD_DN,
	"eits",         AD_EITS,
	"end-of-addr",  AD_END,
	"explicit",     AD_EXPLICITCONVERSION,
	"ext-id",       AD_EXTENSION_ID,
	"extension",	AD_EXTENSION,
	"message_token",	AD_MESSAGE_TOKEN,
	"message_token_crit",	AD_MESSAGE_TOKEN_CRIT,
	"mta-rreq",     AD_MTA_REP_REQ,
	"orig",         AD_ORIG,
	"orig-req-alt",	AD_ORIG_REQ_ALT,
	"orig-req-alt-crit", AD_ORIG_REQ_ALT_CRIT,
	"outchan",      AD_OUTCHAN,
	"outmta",      	AD_OUTHOST,
	"pd_report_request",	AD_PD_REPORT_REQUEST,
	"pd_report_request_crit",	AD_PD_REPORT_REQUEST_CRIT,
	"phys-forward",	AD_PHYS_FORWARD,
	"phys_forward_crit",	AD_PHYS_FORWARD_CRIT,
	"phys_fw_ad",	AD_PHYS_FW_AD,
	"phys_fw_ad_crit",	AD_PHYS_FW_AD_CRIT,
	"phys_modes",	AD_PHYS_MODES,
	"phys_modes_crit",	AD_PHYS_MODES_CRIT,
	"phys_rendition",	AD_PHYS_RENDITION,
	"phys_rendition_crit",	AD_PHYS_RENDITION_CRIT,
	"proof_delivery",	AD_PROOF_DELIVERY,
	"proof_delivery_crit",	AD_PROOF_DELIVERY_CRIT,
	"recip_number_advice",	AD_RECIP_NUMBER_ADVICE,
	"recip_number_advice_crit",	AD_RECIP_NUMBER_ADVICE_CRIT,
	"redirection_history",	AD_REDIRECTION_HISTORY,
	"redirection_history_crit",	AD_REDIRECTION_HISTORY_CRIT,
	"reform-done",  AD_REFORM_DONE,		/* DO NOT CHANGE */
	"reform-list",  AD_REFORM_LIST,
	"reg_mail",	AD_REG_MAIL,
	"reg_mail_crit",	AD_REG_MAIL_CRIT,
	"req_del",	AD_REQ_DEL,
	"req_del_crit",	AD_REQ_DEL_CRIT,
	"resp",         AD_RESPONSIBILITY,
	"rfc",          AD_822,
	"rfcdr",	AD_822DR,
	"rno",          AD_RECIP_NO,		/* DO NOT CHANGE */
	"status",       AD_STATUS,		/* DO NOT CHANGE */
	"subtype",      AD_SUBTYPE,
	"type",         AD_ADDTYPE,
	"usr-rreq",     AD_USR_REP_REQ,
	"x400",         AD_X400,
	"x400dr",	AD_X400DR,
	"x400orig",	AD_X400ORIG,
	0,              -1
	};
int n_atbl_addr = ((sizeof(atbl_addr)/sizeof(CMD_TABLE)) - 1);


CMD_TABLE  atbl_status[] = {  /* recipient-status */
	"pend",         AD_STAT_PEND,
	"done",         AD_STAT_DONE,
	"drrq",         AD_STAT_DRREQUIRED,
	"dliv",         AD_STAT_DRWRITTEN,
	"errs",         AD_STAT_UNKNOWN,
	0,              -1
	};



CMD_TABLE  atbl_mtarreq[] = {  /* mta-report-request */
	"undefined",            AD_MTA_NONE,
	"basic",                AD_MTA_BASIC,
	"confirmed",            AD_MTA_CONFIRM,
	"audit-&-confirmed",    AD_MTA_AUDIT_CONFIRM,
	0,                      -1
	};



CMD_TABLE  atbl_usrreq[] = {  /* user-report-request */
	"no-report",            AD_USR_NOREPORT,
	"basic",                AD_USR_BASIC,
	"confirmed",            AD_USR_CONFIRM,
	"undefined",            AD_USR_NONE,
	0,                      -1
	};


/* sorted - do not reorder */
CMD_TABLE  atbl_expconversion[] = {  /* explicit-conversion */
	"ia52Teletex",		AD_EXP_IA5_TEXT_TO_TELETEX,
	"ia52g3fax",		AD_EXP_IA5_TEXT_TO_G3_FACSIMILE,
	"ia52g4fax1",		AD_EXP_IA5_TEXT_TO_G4_CLASS_1,
	"ia52telex",		AD_EXP_IA5_TEXT_TO_TELEX,
	"ia5tovideo",		AD_EXP_IA5_TEXT_TO_VIDEOTEX,
	"teletex2Telex",        AD_EXP_TELETEX_TO_TELEX,
	"teletex2g3fax",	AD_EXP_TELETEX_TO_G3_FACSIMILE,
	"teletex2g4fax1",	AD_EXP_TELETEX_TO_G4_CLASS_1,
	"teletex2ia5",		AD_EXP_TELETEX_TO_IA5_TEXT,
	"teletex2video",	AD_EXP_TELETEX_TO_VIDEOTEX,
	"telex2g4fax1",		AD_EXP_TELEX_TO_G4_CLASS_1,
	"telex2ia5",		AD_EXP_TELEX_TO_IA5_TEXT,
	"telex2telex",		AD_EXP_TELEX_TO_TELETEX,
	"telex2video",		AD_EXP_TELEX_TO_VIDEOTEX,
	"texlex2g3fax",		AD_EXP_TELEX_TO_G3_FACSIMILE,
	"video2ia5",		AD_EXP_VIDEOTEX_TO_IA5_TEXT,
	"video2telex",		AD_EXP_VIDEOTEX_TO_TELETEX,
	"video2telex",		AD_EXP_VIDEOTEX_TO_TELEX,
	0,                      -1
	};
int n_atbl_expconversion = ((sizeof(atbl_expconversion)/sizeof(CMD_TABLE)) -1);


CMD_TABLE  atbl_types[] = {  /* address-type */
	"x400",                 AD_X400_TYPE,
	"822",                  AD_822_TYPE,
	"any",                  AD_ANY_TYPE,
	0,                      -1
	};



CMD_TABLE  atbl_subtypes[] = {  /* address-subtype */
	"jnt",                  AD_JNT,
	"real.rfc733",          AD_REAL733,
	"real.rfc822",          AD_REAL822,
	"x400-84",		AD_X400_84,
	"x400-88",		AD_X400_88,
	0,                      -1
	};


CMD_TABLE atbl_reg_mail[] = {
	"non-registered",	AD_RMT_NON_REG,
	"registered",		AD_RMT_REG,
	"personal",		AD_RMT_PERSON,
	0,			-1
};

/* sorted - do not reorder */
CMD_TABLE atbl_pd_modes[] = {
	"bureau",		AD_PM_CNT_BUREAU,
	"counter-collect",	AD_PM_CNT,
	"counter-collect+telephone",	AD_PM_CNT_PHONE,
	"counter-collect+teletex",	AD_PM_CNT_TTX,
	"counter-collect+telex",	AD_PM_CNT_TLX,
	"express",		AD_PM_EXPR,
	"ordinary",		AD_PM_ORD,
	"special",		AD_PM_SPEC,
	0,			-1
};
int n_atbl_pd_modes = ((sizeof(atbl_pd_modes)/sizeof(CMD_TABLE)) -1);

/* sorted - do not re-order */
CMD_TABLE atbl_rdm[] = {
	"any",			AD_RDM_ANY,
	"g3fax",		AD_RDM_G3,
	"g4fax",		AD_RDM_G4,
	"mhs",			AD_RDM_MHS,
	"pd",			AD_RDM_PD,
	"tlx",			AD_RDM_TLX,
	"ttx",			AD_RDM_TTX,
	"tty",			AD_RDM_TTY,
	"vtx",			AD_RDM_VTX,
	0,			-1
};
int n_atbl_rdm = ((sizeof(atbl_rdm)/sizeof(CMD_TABLE)) -1);
