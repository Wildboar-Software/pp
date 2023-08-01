/* ut_trace.c: Deals trace structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_trace.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_trace.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_trace.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"mta.h"
#include	<sys/time.h>

extern char *loc_dom_mta;

Trace	*trace_new ()
{
	Trace	*tp;

	tp = (Trace *) smalloc (sizeof *tp);

	bzero ((char *)tp, sizeof *tp);

	tp -> trace_mta = strdup (loc_dom_mta);
	GlobalDomId_new (&tp -> trace_DomId);
	tp -> trace_DomSinfo.dsi_time = utcnow ();
	return tp;
}

Trace	*trace_dup(old)
Trace	*old;
{
	Trace *ret;
	
	if (old == NULL)
		return NULL;

	ret = (Trace *) smalloc (sizeof *ret);
	bzero ((char *)ret, sizeof *ret);

	if (old->trace_mta)
		ret->trace_mta = strdup(old->trace_mta);
	GlobalDomId_dup(&ret->trace_DomId, &old -> trace_DomId);
	ret->trace_DomSinfo.dsi_time = utcdup(old->trace_DomSinfo.dsi_time);
	trace_add(&ret, trace_dup(old->trace_next));
	return ret;
}

void	trace_add (base, new)
Trace	**base, *new;
{
	Trace	**tp;

	for (tp = base; *tp; tp = &(*tp) -> trace_next)
		continue;

	*tp = new;
}

void	trace_free (tp)
Trace	*tp;
{
	if (tp == NULL)
		return;

	if (tp -> trace_next)
		trace_free (tp -> trace_next);
	if (tp -> trace_mta)
		free (tp -> trace_mta);

	GlobalDomId_free (&tp -> trace_DomId);
	DomSupInfo_free (&tp -> trace_DomSinfo);
	free ((char *)tp);
}

int trace_equ(one, two)
Trace	*one, *two;
{
        if (one == NULL 
	    || two == NULL)
	        return 1;

	if (one->trace_mta == NULLCP
	    || two->trace_mta == NULLCP)
		return 1;

	if (one->trace_mta && two->trace_mta
	    && lexequ (one->trace_mta, two->trace_mta) != 0)
		return 1;

	if (lexequ (one -> trace_DomId.global_Country, 
		    two -> trace_DomId.global_Country) != 0)
		return 1;
	if (lexequ (one -> trace_DomId.global_Admin, 
		    two -> trace_DomId.global_Admin) != 0)
		return 1;
	if (one -> trace_DomId.global_Private == NULLCP
	    && two -> trace_DomId.global_Private == NULLCP)
		return 0;
	if (one -> trace_DomId.global_Private == NULLCP
	    || two -> trace_DomId.global_Private == NULLCP)
		return 1;
	if (lexequ (one -> trace_DomId.global_Private, 
		    two -> trace_DomId.global_Private) != 0)
		return 1;

	return 0;
}
	
	
