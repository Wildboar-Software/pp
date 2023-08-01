/* tb_p1.c: p1 tables of other P1 keywds found in ADDR Control Files */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_p1.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_p1.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_p1.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include 	"util.h"
#include	"adr.h"
#include        "tb_com.h"
#include        "tb_p1.h"
#include	"extension.h"
#include        <isode/cmd_srch.h>


/* ----------------------------------------------------------------------- */


CMD_TABLE  p1tbl_mpduid[] = { /* mpdu-identifier */
	"string",                       MPDUID_STRING,
	0,                              -1
	};



CMD_TABLE  p1tbl_trace[] = { /* trace-information */
	"mta",                          TRACE_MTA,
	EOUNIT,                         EOU,
	EOBLOCK,                        EOB,
	0,                              -1
	};



CMD_TABLE  p1tbl_globaldomid[] = { /* global-domain-identifier */
	"Country",                      GLOBAL_COUNTRY,
	"Admin",                        GLOBAL_ADMIN,
	"Prmd",                         GLOBAL_PRIVATE,
	0,                              -1
	};



CMD_TABLE  p1tbl_domsinfo[] = { /* domain-supplied-info */
	"arrival",                      DSI_TIME,
	"deferred",                     DSI_DEFERRED,
	"action",                       DSI_ACTION,
	"converted",                    DSI_CONVERTED,
	"attempted",                    DSI_ATTEMPTED,
	0,                              -1
	};



CMD_TABLE  p1tbl_action[] = { /* domain-supplied-info-action */
	"Relayed",                      ACTION_RELAYED,
	"Rerouted",                     ACTION_ROUTED,
	0,                              -1
	};


CMD_TABLE  p1tbl_encinfoset[] = { /* encode-info-types-set */
	"EncTypes",                     EI_BIT_STRING,
	"G3NonBasic",                   EI_G3NONBASIC,
	"TeletexNonBasic",              EI_TELETEXNONBASIC,
	"PresCapabilities",             EI_PRESENTATION,
	0,                              -1
	};

CMD_TABLE tbl_crit[] = {
	"none",			CRITICAL_NONE,
	"submission",		CRITICAL_SUBMISSION,
	"transfer",		CRITICAL_TRANSFER,
	"delivery",		CRITICAL_DELIVERY,
	0,			-1
	};

CMD_TABLE  tbl_redir[] = {
	"recip-assigned",	RDR_RECIP_ASSIGNED,
	"orig-assigned",	RDR_ORIG_ASSIGNED,
	"md-assigned",		RDR_MD_ASSIGNED,
	0,			-1
};
