/* get_dr.c: get a delivery report given a reference */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/get_dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/get_dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: get_dr.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "adr.h"
#include "q.h"
#include "dr.h"
#include "retcode.h"




/* ---------------------  Begin  Routines  -------------------------------- */


extern char	*dr_file;

int get_dr (dno, msg_name, drmpdu)
int     dno;
char    *msg_name;
DRmpdu  *drmpdu;
{
	char    file[MAXPATHLENGTH];
	FILE    *fp;
	int     retval;

	(void) sprintf (file, "%s/%s%d", msg_name, dr_file, dno);

	if ((fp = fopen (file, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, file,
		      ("Can't read DR"));
		return RP_BAD;
	}

	retval = rd_dr (drmpdu, fp);
	(void) fclose (fp);
	return (retval);
}
