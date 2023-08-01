/* chan_acheck.c: apply channel address checking */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/chan_acheck.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/chan_acheck.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: chan_acheck.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "adr.h"
#include "qmgr.h"

int chan_acheck (ap, mychan, first, mta)
ADDR    *ap;
CHAN	*mychan;
int     first;
char	**mta;
{
	LIST_RCHAN      *chan;
	int     i;
	int	donealready = 0;

	switch (ap -> ad_status) {
	    case AD_STAT_PEND:
		break;

	    case AD_STAT_DONE:
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_success);
		return NOTOK;
		
	    case AD_STAT_DRWRITTEN:
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_positiveDR);
		return NOTOK;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("adr_checks - wrong status"));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	for (chan = ap -> ad_fmtchan, i = 0; i < ap -> ad_rcnt && chan;
	     chan = chan -> li_next, i++) {
		if (strcmp (chan -> li_chan -> ch_name,
			    mychan -> ch_name) == 0)
			donealready = 1;
	}
	if (chan == NULL)
		chan = ap -> ad_outchan;

	if (strcmp (chan -> li_chan -> ch_name, mychan -> ch_name) != 0) {
		if (donealready) {
			PP_NOTICE (("Done channel %s already",
				    mychan -> ch_name));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_success);
			return NOTOK;
		}
		PP_LOG (LLOG_EXCEPTIONS,
			("adr %d not ready for channel %s yet",
			 ap -> ad_no, mychan -> ch_name));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (mychan->ch_chan_type != CH_SHAPER && ap -> ad_resp == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("responsibility bit not set for addr %d",
			 ap -> ad_no));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (mta == NULLVP)	/* mta check not required */
		return OK;

	if (mychan -> ch_mta) {
		*mta = strdup (mychan -> ch_mta);
		return OK;
	}

	if (ap -> ad_outchan -> li_mta == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS,
			("No mta for address %d", ap -> ad_no));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (*mta == NULLCP)
		*mta = strdup (ap -> ad_outchan -> li_mta);
	else if (strcmp (*mta, ap -> ad_outchan -> li_mta) != 0) {
		if (first) {	/* ok to change here */
			free (*mta);
			*mta = strdup (ap -> ad_outchan -> li_mta);
		}
		else {
			PP_LOG (LLOG_EXCEPTIONS,
				("Mta changed from %s to %s", *mta,
				 ap -> ad_outchan -> li_mta));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_messageFailure);
			return NOTOK;
		}
	}
	return OK;
}
