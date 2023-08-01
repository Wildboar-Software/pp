/* ut_p1_st.c: Deals with new p1 structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_p1_st.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_p1_st.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_p1_st.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"mta.h"
#include	<sys/time.h>


extern GlobalDomId	Myglobalname;
extern char	*loc_dom_mta;
extern char	*loc_dom_site;



/* ---------------------  Begin	 Routines  -------------------------------- */



new_GlobalDomId (gp)   /* generate a new global id */
GlobalDomId	*gp;
{
	PP_DBG (("Lib/pp/new_GlobalDomId()"));

	gp->global_Country = strdup(Myglobalname.global_Country);
	gp->global_Admin = strdup(Myglobalname.global_Admin);
	if(Myglobalname.global_Private)
		gp->global_Private = strdup(Myglobalname.global_Private);
}

GlobalDomId_dup(new, old)
GlobalDomId	*new, *old;
{
	if (old->global_Country)
		new->global_Country = strdup(old->global_Country);
	if (old->global_Admin)
		new->global_Admin = strdup(old->global_Admin);
	if (old->global_Private)
		new->global_Private = strdup(old->global_Private);
}

new_MPDUid(mp)
MPDUid	*mp;
{
	time_t		t, time();
	struct tm	*tp;
	char		tbuf[LINESIZE];

	PP_DBG (("Lib/pp/new_MPDUid()"));

	t = time((time_t *)0);
	tp = gmtime(&t);
	(void) sprintf(tbuf, "%s.%d:%02d.%02d.%02d.%02d.%02d.%02d",
			loc_dom_mta, getpid(),
			tp->tm_mday, tp->tm_mon, tp->tm_year+70,
			tp->tm_hour, tp->tm_min, tp->tm_sec);
	mp->mpduid_string = strdup(tbuf);
	new_GlobalDomId(&mp->mpduid_DomId);
}



new_Trace_add (tpp)
register Trace		**tpp;
{
	Trace		*new,
			*tp = *tpp;


	PP_DBG (("new_trace_add()"));

	new = (Trace *) smalloc (sizeof (*new));
	bzero ((char *) new, sizeof (*new));
	new->trace_mta = strdup (loc_dom_mta);
	new_GlobalDomId (&new->trace_DomId);
	new->trace_DomSinfo.dsi_time = utcnow ();

	if (tp != (Trace *)0)
		new->trace_next = tp;

	*tpp = new;
	return (OK);
}
