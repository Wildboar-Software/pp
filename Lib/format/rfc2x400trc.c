/* rfc2x400trace.c: converts an RFC string to an x400 trace via rfc 1138 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2x400trc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2x400trc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2x400trc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "mta.h"

extern char	*compress();
static int 	do_md_mta(),
		do_deferred(),
		do_converted(),
		do_attempt_md_mta(),
		do_arrival(),
		do_actions();

#define MATCH(a,b)	(lexnequ((a),(b),strlen(b)) == 0)


int rfc2x400trace (trace, str)
Trace	*trace;
char	*str;
{
	/* see sect 5.3.7 in 1148 */

	char	*start, *end, *tmp = strdup(str);
	int	result = OK;
	PP_DBG (("Lib/rfc2x400trace (%s)", str));
	
	start = tmp;
	
	while (result == OK && start !=  NULLCP && *start != '\0') {
		if ((end = index (start, ';')) != NULLCP) 
			*end++ = '\0';
		(void) compress (start, start);
		if (MATCH(start, "by"))
			/* md and mta bit */
			result = do_md_mta(trace, start);
		else if (MATCH(start, "deferred until"))
			/* deffered delivery time */
			result = do_deferred (trace, start);
		else if (MATCH(start, "converted"))
			/* converted eits */
			result = do_converted(trace, start);
		else if (MATCH(start, "attempted"))
			/* attempted md and mta */
			result = do_attempt_md_mta(trace, start);
		else if (end == NULLCP) 
			/* arrival-time */
			result = do_arrival(trace, start);
		else 
			/* action-list */
			result = do_actions (trace, start);
		start = end;
	}
	free(tmp);
	if (trace->trace_DomSinfo.dsi_time == NULL)
		trace->trace_DomSinfo.dsi_time = utcnow ();
	return result;
}

static int do_md_mta (trace, start)
Trace	*trace;
char	*start;
{
	char	*end;
	PP_DBG (("Lib/rfc2x400trace/do_md_mta (%s)", start));
	start += strlen("by");
	
	while (isspace(*start)) start++;
	
	if (MATCH (start, "mta ")) {
		start += strlen("mta ");
		while (isspace(*start)) start++;
		end = start;
		while (*end != ' ') end++;
		*end = '\0';
		trace -> trace_mta = strdup(start);
		start = end+1;
		while (isspace(*start)) start++;
		if (!MATCH (start, "in "))
			return NOTOK;
		start += strlen("in ");
		while (isspace(*start)) start++;
	}
	
	return rfc2x400globalid (&trace -> trace_DomId, start);
}
	     
static int do_deferred (trace, start)
Trace	*trace;
char	*start;
{
	PP_DBG (("Lib/rfc2x400trace/do_deferred (%s)", start));
	start += strlen("deferred until");
	while (isspace(*start)) start++;
	
	return rfc2UTC (start, &trace->trace_DomSinfo.dsi_deferred);
}

static int do_converted (trace, start)
Trace	*trace;
char	*start;
{
	char	*end;
	PP_DBG (("Lib/rfc2x400trace/do_converted (%s)", start));
	if ((start = index(start, '(')) == NULLCP)
		return NOTOK;

	start++;
	if ((end = index(start, ')')) == NULLCP)
		return NOTOK;
	*end = '\0';

	return rfc2encinfo (&trace -> trace_DomSinfo.dsi_converted, start);
}

static int do_attempt_md_mta(trace, start)
Trace	*trace;
char	*start;
{
	char	*end;

	PP_DBG (("Lib/rfc2x400trace/do_attempt_md_mta (%s)", start));

	start += strlen("attempted");
	
	while (isspace(*start)) start++;
	
	if (MATCH (start, "mta ")) {
		start += strlen("mta ");
		while (isspace(*start)) start++;
		end = start;
		while (*end != ' ') end++;
		*end = '\0';
		trace -> trace_DomSinfo.dsi_attempted_mta = strdup(start);
		start = end+1;
		while (isspace(*start)) start++;
		if (!MATCH(start, "in "))
			return NOTOK;
		start += strlen("in ");
		while (isspace(*start)) start++;
	}
	
	return rfc2x400globalid (&trace -> trace_DomSinfo.dsi_attempted_md,
				 start);
}
	
static int do_actions (trace, start)
Trace	*trace;
char	*start;
{

	PP_DBG (("Lib/rfc2x400trace/do_actions (%s)", start));

	while (start != NULLCP && *start != '\0') {
		while (isspace(*start)) start++;
		if (MATCH(start, "relayed")) {
			trace -> trace_DomSinfo.dsi_action = ACTION_RELAYED;
			start += strlen("relayed");
		} else if (MATCH(start, "rerouted")) {
			trace -> trace_DomSinfo.dsi_action = ACTION_ROUTED;
			start += strlen("rerouted");
		} else if (MATCH(start, "redirected")) {
			trace -> trace_DomSinfo.dsi_other_actions 
				= trace -> trace_DomSinfo.dsi_other_actions 
					| ACTION_REDIRECTED;
			start += strlen("redirected");
		} else if (MATCH (start, "expanded")) {
			trace -> trace_DomSinfo.dsi_other_actions 
				= trace -> trace_DomSinfo.dsi_other_actions
					| ACTION_EXPANDED;
			start += strlen("expanded");
		} else {
			PP_TRACE(("Unknown action in x400 trace '%s'", start));
			return NOTOK;
		}
		while (isspace(*start)) start++;
	}
	return OK;
}
			
static int do_arrival (trace, start)
Trace	*trace;
char	*start;
{
	PP_DBG (("Lib/rfc2x400trace/do_arrival (%s)", start));

	return rfc2UTC (start, &trace->trace_DomSinfo.dsi_time);
}
