/* submit_prm.c: read and process control switches */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_prm.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_prm.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: submit_prm.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "q.h"


extern void	err_abrt();


/* -- local routines -- */
int		rd_prm_info();
int		rd_q_info();




/* ---------------------  Begin  Routines  -------------------------------- */




int rd_prm_info (prm)  /* -- read in a prm management structure -- */
struct prm_vars         *prm;
{
	int             retval;

	PP_TRACE (("submit/rd_prm_info (prm)"));

	if (rp_isbad (retval = rd_prm (prm, stdin))) {
		switch (retval) {
		case RP_EOF:
			break;
		case RP_PARM:
			err_abrt (RP_PARM, "Aborting message");
		}
	}
	return (retval);
}




int rd_q_info (qp)  /* -- read in a queue structure -- */
register Q_struct       *qp;
{
	int             retval;

	PP_TRACE (("submit/rd_q_info (qp)"));

	if (rp_isbad (retval = rd_q (qp, stdin))) {
		switch (retval) {
		    case RP_EOF:
			err_abrt (RP_EOF, "Can't read qstruct");
			break;
		    case RP_PARM:
			err_abrt (RP_PARM, "Aborting message");
			break;
		}
	}
	return (retval);
}
