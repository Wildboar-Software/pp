/* wr_prm.c: send the parameter structure to submit */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_prm.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_prm.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: wr_prm.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"



/* ---------------------  Begin  Routines  -------------------------------- */



int wr_prm (pp, fp)
struct prm_vars         *pp;
FILE                    *fp;
{
	int             retval;

	PP_DBG (("Lib/io/wr_prm()"));

	retval = prm2txt (fp, pp);
	if (retval == NOTOK)
		return (RP_FIO);
	return (RP_OK);
}
