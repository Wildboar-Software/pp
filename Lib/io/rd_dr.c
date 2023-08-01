/* rd_dr.c: read a DR file from q/msg/dir/dr.999 into a DR struct */
/* see man page QUEUE (5) */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_dr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_dr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: rd_dr.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "dr.h"




/* ---------------------  Begin  Routines  -------------------------------- */

int rd_dr (dp, fp)
register DRmpdu         *dp;
FILE                    *fp;
{

	PP_DBG (("Lib/io/rd_dr()"));
	dr_init (dp);
	return (txt2dr (dp, fp));
}
