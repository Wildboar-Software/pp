/* wr_adr: address writing routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_adr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_adr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: wr_adr.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



/* --- *** --- 
This routine is used so that there is only one entry point to adr2txt()
which is located in Lib/pp/adr2txt.c
--- *** --- */


#include        "head.h"
#include        "adr.h"
#include 	"sys.file.h"
#include        <sys/time.h>




/* ---------------------  Begin  Routines  -------------------------------- */



int wr_adr (ap, fp, type)
ADDR    *ap;
FILE    *fp;
int     type;
{
	int     retval;

	PP_DBG (("wr_adr (%d)", type));

	retval = adr2txt (fp, ap, type);
	if (retval == NOTOK)
		return (RP_FIO);
	return (RP_OK);
}
