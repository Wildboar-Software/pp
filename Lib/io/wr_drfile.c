/* wr_q2drsubmit.c: write out a Delivery Notification struct 
	from the info held in the queue and address structures of submit
*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_drfile.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_drfile.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: wr_drfile.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include        "head.h"
#include        "q.h"
#include        "dr.h"
#include 	"sys.file.h"
#include        "list_bpt.h"
#include        <sys/time.h>

extern char     *dr_file;


/* ---------------------  Begin  Routines  -------------------------------- */


int wr_q2drsubmit (qp, path)
register Q_struct       *qp;
char			*path;
{
	FILE            *fp;
	ADDR            *ap;
	char            *fname = NULL,
			buf[MAXPATHLENGTH],
			odir[MAXPATHLENGTH];
	int             retval=NOTOK,
			dr_file_no = -1,
			dr_required = FALSE;

	(void) strcpy (odir, path);

	PP_DBG (("Lib/io/wr_q2drsubmit (dir=%s)", odir));


	/* open a DR file (dr.999) name is determined by recip */

	for (ap=qp->Raddress, retval=OK;
		ap != NULLADDR && retval == OK; ap=ap->ad_next)

		if (ap->ad_status == AD_STAT_DRREQUIRED) {

			dr_required = TRUE;

			dr_file_no = ap->ad_no;
			PP_DBG(("ap->ad_no = %d",ap->ad_no));
			(void) sprintf (&buf[0], "%s%d", dr_file, dr_file_no);

			fname = multcat (odir, "/", &buf[0], NULLCP);

			if (access (fname, F_OK) == NOTOK)
				break;


			retval = NOTOK;
			PP_LOG (LLOG_EXCEPTIONS,
			      ("Lib/io/wr_q2drsubmit (file %s exists)",
				fname));
		}

	if (ap == NULLADDR && dr_required == FALSE) return (RP_OK);

	if (retval == NOTOK) {
		PP_LOG (LLOG_FATAL,
		("Lib/io/wr_q2drsubmit (dr required but first file in use)"));
		return (RP_BAD);
	}

	PP_DBG (("Lib/io/wr_q2drsubmit (opening %s)", fname));

	if ((fp = fopen (fname, "w")) == NULLFILE) {
		PP_LOG (LLOG_FATAL,
			("Lib/io/wr_q2drsubmit (unable to open %s)", fname));
		return (RP_FIO);
	}


	retval = wr_dr_info (fp, qp, dr_file_no, TRUE);

	(void) fclose(fp);

	if (chmod(fname, 0666) != 0)
		PP_SLOG(LLOG_EXCEPTIONS, fname,
			("chmod failed"));

	if (retval == NOTOK)    return (RP_FIO);

	return (RP_OK);
}
