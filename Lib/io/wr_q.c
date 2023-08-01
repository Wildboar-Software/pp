/* wr_q.c: write out a queue structure to a Control File in Q/ADDR */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_q.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_q.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: wr_q.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



/* __________________________________________________________________________

This routine is used so that there is only one entry point to q2txt()
which is located in Lib/pp/tx_q.c

____________________________________________________________________________*/



#include "head.h"
#include "q.h"




/* ---------------------  Begin  Routines  -------------------------------- */



int wr_q (qp, fp)
register Q_struct       *qp;
FILE                    *fp;
{
	int             retval;

	PP_DBG (("Lib/wr_q (type=%d, size=%d)", qp->msgtype, qp->msgsize));

	retval = q2txt (fp, qp);
	if (retval == NOTOK)
		return (RP_FIO);
	return (RP_OK);
}
