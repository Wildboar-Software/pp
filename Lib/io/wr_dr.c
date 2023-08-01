/*  wr_dr.c: writes out a DR struct from an incomming chan  */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_dr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_dr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: wr_dr.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "dr.h"


/* ---------------------  Begin  Routines  -------------------------------- */



int wr_dr (dp, fp)
register DRmpdu	*dp;
FILE	*fp;
{
	int             retval;


	PP_DBG (("Lib/io/wr_dr()"));

	retval = dr2txt (fp, dp);
	if (retval == NOTOK)
		return (RP_FIO);
	return (RP_OK);
}
