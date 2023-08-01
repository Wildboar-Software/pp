/* tb_prm.c: MessageManagementParameter tables the parameter keywords
	     found in ADDR Control Files.  See manual page QUEUE (5).
*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_prm.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"tb_prm.h"
#include	"prm.h"
#include	<isode/cmd_srch.h>


/* ------------------------------------------------------------------------ */



CMD_TABLE  prmtbl_ln[] = { /* message-management-parameters */
	"logfile",			PRM_LOGFILE,
	"level",			PRM_LOGLEVEL,
	"options",			PRM_OPTS,
	"passwd",			PRM_PASSWD,
	"Start-of-MsgEnvPrm",		PRM_END,
	0,				-1
	};

CMD_TABLE  prmtbl_opts[] = { /* options */
	"acceptall",			PRM_ACCEPTALL,
	"notrace",			PRM_NOTRACE,
	0,			-1
	};
