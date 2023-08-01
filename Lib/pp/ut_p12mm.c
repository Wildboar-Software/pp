 /* ut_p12mm.c: Deals with conversions between p1 structures -> memory */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_p12mm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_p12mm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_p12mm.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"




/* ---------------------  Field -> Memory  -------------------------------- */




void prf2mem (prf, resp, mta, usr)	/* PerRecipientFlag -> Mem */
int	prf;
int	*resp;
int	*mta;
int	*usr;
{
	int	i;

	PP_DBG (("Lib/pp/prf2mem(%d)", prf));

	*resp = *mta = *usr = 0;

	for (i = 0; i < 5; i++)
		if (prf & (1 << i))
			switch (i) {
			case 0:
				*resp = 1;
				break;
			case 1:
				*mta |= 2;
				break;
			case 2:
				*mta |= 1;
				break;
			case 3:
				*usr |= 2;
				break;
			case 4:
				*usr |= 1;
				break;
			}


	PP_DBG (("Lib/pp/prf2mem returns %d  (%d %d %d)",
		prf, *resp, *mta, *usr));
}
