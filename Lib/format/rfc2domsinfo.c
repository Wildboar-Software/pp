/* rfc2domsinfo.c - Converts a RFC string to a DomSupInfo struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2domsinfo.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2domsinfo.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2domsinfo.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/cmd_srch.h>
#include "mta.h"
#include "tb_p1.h"


extern CMD_TABLE        p1tbl_domsinfo[],
			p1tbl_action[];




/* ---------------------  Begin  Routines  -------------------------------- */




int rfc2domsinfo (dp, str)
DomSupInfo      *dp;
char            *str;
{
	char    *cp = str,
		*bp = str,
		*ptr;

	PP_DBG (("Lib/rfc2domsinfo (%s)", str));

	if ((cp = index (str, ' ')) == NULLCP)
		return NOTOK;
	else
		*cp++ = '\0';


	/* -- arrival -- */
	if (lexequ (bp, rcmd_srch (DSI_TIME, p1tbl_domsinfo)) == 0) {
		bp = cp;
		if ((cp = rindex (cp, ':')) == NULLCP)
			return NOTOK;
		if ((ptr = index (cp, '+')) == NULLCP)
			ptr = index (cp, '-');
		if (ptr)
			cp = ptr;
		if ((cp = index (cp, ' ')) != NULLCP)
			*cp++ = '\0';
		if (rfc2UTC (bp, &dp->dsi_time) == NOTOK)
			return NOTOK;
	}
	else
		return NOTOK;


	/* -- optional stuff now follows  -- */
	if ((bp = cp) == NULLCP)
		return OK;
	if ((cp = index (bp, ' ')) == NULLCP)
		return NOTOK;
	else
		*cp++ = '\0';



	/* -- deferred -- */
	if (lexequ (bp, rcmd_srch (DSI_DEFERRED, p1tbl_domsinfo)) == 0) {
		bp = cp;
		if ((cp = rindex (cp, ':')) == NULLCP)
			return NOTOK;
		if ((ptr = index (cp, '+')) == NULLCP)
			ptr = index (cp, '-');
		if (ptr)
			cp = ptr;
		if ((cp = index (bp, ' ')) != NULLCP)
			*cp++ = '\0';
		if (rfc2UTC (bp, &dp->dsi_deferred) == NOTOK)
			return NOTOK;
		if ((bp = cp) == NULLCP)
			return OK;
		if ((cp = index (bp, ' ')) == NULLCP)
			return NOTOK;
		else
			*cp++ = '\0';
	}



	/* -- action -- */
	if (lexequ (bp, rcmd_srch (DSI_ACTION, p1tbl_domsinfo)) == 0) {
		bp = cp;

		if ((cp = index (bp, ' ')) != NULLCP)
			*cp++ = '\0';

		switch (cmd_srch (bp, p1tbl_action)) {
		case ACTION_RELAYED:
			dp->dsi_action = ACTION_RELAYED;
			break;
		default:
			dp->dsi_action = ACTION_ROUTED;
			break;
		}

		if ((bp = cp) == NULLCP)
			return OK;
		if ((cp = index (bp, ' ')) == NULLCP)
			return NOTOK;
		else
			*cp++ = '\0';
	}



	/* -- converted -- */
	if (lexequ (bp, rcmd_srch (DSI_CONVERTED, p1tbl_domsinfo)) == 0) {
		if ((cp = index (cp, '(')) == NULLCP)
			return NOTOK;
		else
			bp = ++cp;

		if ((cp = index (bp, ')')) == NULLCP)
			return NOTOK;
		else
			*cp++ = '\0';

		if (rfc2encinfo (&dp->dsi_converted, bp) == NOTOK)
			return NOTOK;

		bp = cp;
		if ((cp = index (bp, ' ')) == NULLCP)
			return OK;
		else
			*cp++ = '\0';
	}



	/* -- attempted -- */
	if (lexequ (bp, rcmd_srch (DSI_ATTEMPTED, p1tbl_domsinfo)) == 0) {
		bp = cp;
		if (rfc2globalid (&dp->dsi_attempted_md, bp) == NOTOK)
			return NOTOK;
	}


	return OK;

}
