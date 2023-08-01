/* auth_tb.c: authorisation tables */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/auth_tb.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/auth_tb.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: auth_tb.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include	"q.h"
#include	"tb_auth.h"
#include        <isode/cmd_srch.h>




CMD_TABLE authtbl_reasons[] = {
	"unknown error", 				AUR_UNKNOWN,
	"This route is prohibited: (policy none)",	AUR_CH_NONE,
	"Authorisation approved: (policy free)",	AUR_CH_FREE,
	"in MTA regex : %s does not include sender",	AUR_IMTA_MISSING_SNDR,
	"in MTA regex : %s excludes sender",		AUR_IMTA_EXCLUDES_SNDR,
	"out MTA regex : %s does not include recip",	AUR_OMTA_MISSING_RECIP,
	"out MTA regex : %s excludes recip",		AUR_OMTA_EXCLUDES_RECIP,
	"msg size %s exceeds channel limit of %s",	AUR_MSGSIZE_FOR_CHAN,
	"msg size %s exceeds mta limit of %s",		AUR_MSGSIZE_FOR_MTA,
	"msg size %s exceeds user limit of %s",		AUR_MSGSIZE_FOR_USR,
	"bad body part : %s (mta restriction)",		AUR_MTA_BPT,
	"bad body part : %s (user restriction)",	AUR_USR_BPT,
	"channel bind failed",				AUR_CHBIND,
	"duplicate recip removed",			AUR_DUPLICATE_REMOVED,
	0,						-1
	};




CMD_TABLE authtbl_loglevel[] = {
	"low", 			AUTH_LOGLEVEL_LOW,
	"medium", 		AUTH_LOGLEVEL_MEDIUM,
	"high", 		AUTH_LOGLEVEL_HIGH,
	0, 			-1
	};



CMD_TABLE authtbl_statmsgs [] = {
	"ok", 			AUTH_OK,
	"failed",		AUTH_FAILED,
	"ok-testfailed",	AUTH_FAILED_TEST,
	"dr_ok",		AUTH_DR_OK,
	"dr_failed",		AUTH_DR_FAILED,
	0, 			-1
	};
