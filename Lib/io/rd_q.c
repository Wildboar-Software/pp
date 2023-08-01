/* rd_q.c: reads the control file in Q/addr into memory */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_q.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_q.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: rd_q.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include <isode/cmd_srch.h>
#include "q.h"
#include "tb_q.h"


/* ---------------------  Begin  Routines  -------------------------------- */

static int R_umpdu ();

int rd_q (qp, fp)
register Q_struct       *qp;
FILE                    *fp;
{

	PP_DBG (("Lib/io/rd_q()"));
	q_init (qp);
	return (R_umpdu (qp, fp));
}



/* ---------------------  Static  Routines  ------------------------------- */



static int R_umpdu (qp, fp)
register Q_struct       *qp;
FILE                    *fp;
{
	char            buf[10*BUFSIZ],
			*argv[100];
	int             argc,
			retval;
	long		fp_startln;


	PP_DBG (("Lib/io/R_umpdu()"));

	q_init (qp);

	for (;;) {
		
		fp_startln = ftell(fp);

		retval = txt2buf (buf, sizeof buf, fp);

		if (rp_isbad (retval)) {
			PP_DBG (("Lib/io/R_umpdu txt2buf retval (%d - %s)",
				retval, rp_valstr (retval)));
			return (retval);
		}

		PP_DBG (("Lib/io/R_umpdu(buf='%.999s')", &buf[0]));

		if ((argc = str2arg (buf, 100, argv)) == 0)
			return (RP_PARM);

		retval = txt2q (qp, fp_startln, argv, argc);

		PP_DBG (("Lib/io/R_umpdu (txt2q=%d)", retval));
		if (retval == NOTOK)    return (RP_PARM);
		if (retval == Q_END)    return (RP_OK);
	}
}
