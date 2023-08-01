/* tb_misc.c: msic tables */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_misc.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_misc.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_misc.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include 	"util.h"
#include	<isode/cmd_srch.h>


/* ------------------------------------------------------------------------ */

CMD_TABLE tbl_bool[] = {	/* boolean values */
	"true",		TRUE,
	"yes",		TRUE,
	"false",	FALSE,
	"no",		FALSE,
	0,		0
};

CMD_TABLE tbl_month[] = { /* -- used for rfc-utc time calculations -- */
   "Jan",					0,
   "Feb",					1,
   "Mar",					2,
   "Apr",					3,
   "May",					4,
   "Jun",					5,
   "Jul",					6,
   "Aug",					7,
   "Sep",					8,
   "Oct",					9,
   "Nov",					10,
   "Dec",					11,
   0,						-1
   };
