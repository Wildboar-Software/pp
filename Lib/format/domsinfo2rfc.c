/* domsinfo2rfc.c - Converts a DomSupInfo struct into a RFC string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/domsinfo2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/domsinfo2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: domsinfo2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include        "mta.h"
#include        "tb_p1.h"


extern CMD_TABLE        p1tbl_domsinfo[];

extern int 	x400eits2rfc ();
extern int	globalid2rfc ();
extern UTC	utclocalise ();

static int action2rfc ();
/* -------------------- Begin  Routines  ---------------------------------- */




int domsinfo2rfc (dominfo, buffer)  /* DomainSuppliedInfo -> RFC */
DomSupInfo      *dominfo;
char            *buffer;
{
	char    tbuf [LINESIZE];
	UTC	lut;
	char *cp, *p;

	cp = buffer;

	/* -- arrival -- */
	if (dominfo -> dsi_time != NULLUTC && 
	    (lut = utclocalise(dominfo -> dsi_time)) != NULLUTC &&
	    UTC2rfc (lut, tbuf) == OK &&
	    (p = rcmd_srch (DSI_TIME, p1tbl_domsinfo)) != NULLCP) {
		(void) sprintf (cp, "%s %s", p, tbuf);
		free ((char *) lut);

		cp += strlen (cp);
	}
	else
		return NOTOK;


	/* -- deferred -- */
	if (dominfo -> dsi_deferred != NULLUTC) {
		if ((lut = utclocalise(dominfo -> dsi_deferred)) != NULLUTC &&
		    UTC2rfc (lut, tbuf) == OK &&
		    (p = rcmd_srch (DSI_DEFERRED, p1tbl_domsinfo)) != NULLCP) {
			(void) sprintf (cp, " %s %s", p, tbuf);
			free ((char *) lut);

			cp += strlen(cp);
		}
		else return NOTOK;
	}


	/* -- action -- */
	if (action2rfc (dominfo -> dsi_action, tbuf) == OK &&
	    (p = rcmd_srch (DSI_ACTION, p1tbl_domsinfo)) != NULLCP)
		(void) sprintf (cp, " %s %s", p, tbuf);
	else return NOTOK;

	/* -- converted -- */
	if (dominfo -> dsi_converted.eit_types) {
		if (x400eits2rfc (&dominfo -> dsi_converted, tbuf) == NOTOK)
			return NOTOK;
		(void) sprintf (cp, " %s (%s)",
				rcmd_srch (DSI_CONVERTED, p1tbl_domsinfo),
				tbuf);
		cp += strlen (cp);
	}


	/* -- attempted/previous -- */
	if (dominfo -> dsi_attempted_md.global_Country != NULLCP ||
	    dominfo -> dsi_attempted_md.global_Admin   != NULLCP ||
	    dominfo -> dsi_attempted_md.global_Private != NULLCP) {
		if (globalid2rfc (&dominfo -> dsi_attempted_md, tbuf) == NOTOK)
			return NOTOK;
		(void) sprintf (cp, " %s %s",
				rcmd_srch (DSI_ATTEMPTED, p1tbl_domsinfo),
				tbuf);
	}
	PP_DBG (("Lib/domsinfo2rfc returns (%s)", buffer));

	return OK;
}




/* ---------------------  Static  Routines  ------------------------------- */




static int action2rfc (action, buffer)  /* DomainSuppliedInfo(action) -> RFC */
int     action;
char    *buffer;
{
	PP_DBG (("Lib/action2rfc(%d)", action));

	switch (action) {
	    case ACTION_RELAYED:
		(void) strcpy (buffer, "Relayed");
		break;
	    case ACTION_ROUTED:
		(void) strcpy (buffer, "Routed");
		break;
	    default:
		*buffer = '\0';
		return NOTOK;
	}
	return OK;
}
