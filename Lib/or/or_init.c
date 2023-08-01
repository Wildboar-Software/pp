/* or_init.c: or-name initialisation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_init.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_init.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_init.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "mta.h"
#include "or.h"


GlobalDomId             Myglobalname;
extern  OR_ptr          loc_ortree;


extern	void err_abrt ();

/* ---------------------  Start  Routines  -------------------------------- */




/*
Starts up the or package etc.
*/

void or_myinit()
{
	static  char    or_inited = 0;
	OR_ptr          or = NULLOR;

	if (or_inited)
		return;
	or_inited++;
	if (or_init() == NOTOK)
		err_abrt (RP_MECH, "or_init failed");
	or = or_find (loc_ortree, OR_C, NULLCP);
	if (or == NULLOR)
		err_abrt (RP_MECH, "or_init - no country in loc_or");
	Myglobalname.global_Country = or->or_value;
	or = or_find (loc_ortree, OR_ADMD, NULLCP);
	if (or == NULLOR)
		err_abrt (RP_MECH, "or_init - no ADMD in loc_or");
	Myglobalname.global_Admin = or->or_value;
	or = or_find (loc_ortree, OR_PRMD, NULLCP);
	if (or != NULLOR)
		Myglobalname.global_Private = or->or_value;
}
