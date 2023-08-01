/* ut_prm.c: MessageManagementParameter utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_prm.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "prm.h"




/* ---------------------  Begin  Routines  -------------------------------- */




void prm_init (pp)
struct prm_vars         *pp;
{
	PP_DBG (("Lib/pp/ut_prm.c/prm_init()"));
	bzero ((char *) pp, sizeof (*pp));
}


void prm_free (pp)
struct prm_vars         *pp;
{
	PP_DBG (("Lib/pp/ut_prm.c/prm_free()"));

	if (pp->prm_logfile != NULLCP)  free (pp->prm_logfile);
	prm_init (pp);
}
