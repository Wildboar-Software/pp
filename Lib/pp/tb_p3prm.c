/* tb_p3prm.c: P3 paramter keywords */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_p3prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_p3prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_p3prm.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"tb_p3prm.h"
#include	<isode/cmd_srch.h>


/* ------------------------------------------------------------------------ */



CMD_TABLE  p3prm_tbl[] = { /* message-management-parameters */
	"P3mpduid",	P3PRM_MPDUID,
	"P3subtime",	P3PRM_STIME,
	"P3content",	P3PRM_CONTENT,
	"P3origcert",	P3PRM_ORIG_MTA_CERT,
	"P3proofofsub",	P3PRM_PROOF_OF_SUB,
	"P3end",	P3PRM_END,
	0,				-1
	};
