/* ut_p1_st.c: Deals with new p1 structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_gldm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_gldm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_gldm.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"mta.h"
#include 	"or.h"
#include	<sys/time.h>


extern GlobalDomId	Myglobalname;



/* ---------------------  Begin	 Routines  -------------------------------- */


void GlobalDomId_new (gp)   /* generate a new global id */
GlobalDomId	*gp;
{
	PP_DBG (("Lib/pp/new_GlobalDomId()"));

	or_myinit ();
	gp->global_Country = strdup(Myglobalname.global_Country);
	gp->global_Admin = strdup(Myglobalname.global_Admin);
	if(Myglobalname.global_Private)
		gp->global_Private = strdup(Myglobalname.global_Private);
}

void GlobalDomId_free (gp)
GlobalDomId *gp;
{
	if (gp -> global_Country)
		free (gp -> global_Country);
	if (gp -> global_Private)
		free (gp -> global_Private);
	if (gp -> global_Admin)
		free (gp -> global_Admin);
	bzero ((char *)gp, sizeof *gp);
}
