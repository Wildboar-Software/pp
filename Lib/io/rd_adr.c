/* rd_adr.c: read a text address struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_adr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_adr.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: rd_adr.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"adr.h"
#include	"sys.file.h"
#include	<sys/time.h>


extern int	ad_count;
CHAN		*ch_nm2struct();




/* ---------------------  Begin	 Routines  -------------------------------- */



int rd_adr (fp, justone, base)  /* get an addr seq from a file */
FILE	*fp;
int	justone;
ADDR	**base;
{
	long	fp_startln;
	char	*argv[100],
		buf[10 * BUFSIZ];
	int	argc,
		gotone=FALSE,
		retval;

	PP_DBG (("Lib/io/rd_adr (fp, %d, base)", justone));

	for (;;) {

		fp_startln = ftell(fp);

		if (rp_isbad (retval = txt2buf (buf, sizeof buf, fp))) {
			PP_DBG (("Lib/io/rd_adr txt2buf retval (%d - %s)",
					retval, rp_valstr (retval)));
			if (retval == RP_EOF && !justone && gotone)
				return (RP_DONE);
			return (retval);
		}

		if (retval == RP_DONE)
			return (RP_DONE);

		if ((argc = str2arg (buf, 100, argv)) == 0)
			return (RP_PARM);

		retval = txt2adr (base, fp_startln, argv, argc);

		PP_DBG (("Lib/io/rd_adr retval (%d)", retval));

		if (retval == NOTOK)
			return (RP_PARM);

		if (gotone == FALSE)
			gotone = TRUE;

		ad_count ++;

		if (justone)
			return (RP_OK);
	}
}
