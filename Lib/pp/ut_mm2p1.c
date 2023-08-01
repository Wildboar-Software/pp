/* ut_mm2p1.c: Deals with conversions between memory -> p1 structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_mm2p1.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_mm2p1.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_mm2p1.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"



/* ---------------------  Memory -> Field  -------------------------------- */



int mem2prf (resp, mta, usr)   /* creates a PerRecipientFlag from Memory */
int	resp;
int	mta;
int	usr;
{
	int	field = 0;

	PP_DBG (("Lib/mem2prf (%d %d %d)", resp, mta, usr));

	/* -- set the responsibility field -- */
	field |= resp;


	/* -- set the report request field -- */
	switch (mta) {
	case 0:
		break;
	case 1:
		field |= (1 << 2);
		break;
	case 2:
		field |= (1 << 1);
		break;
	case 3:
		field |= (1 << 1);
		field |= (1 << 2);
		break;
	}


	/* -- set the user report request field -- */
	switch (usr) {
	case 0:
		break;
	case 1:
		field |= (1 << 4);
		break;
	case 2:
		field |= (1 << 3);
		break;
	case 3:
		field |= (1 << 3);
		field |= (1 << 4);
		break;
	}


	PP_DBG (("Lib/mem2prf returns (%d)", field));

	return (field);
}
