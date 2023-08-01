/* lchan_acheck.c: apply local channel address checking */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/lchan_acheck.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/lchan_acheck.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: lchan_acheck.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "adr.h"
#include "qmgr.h"

int lchan_acheck (ap, mychan, first, recip)
ADDR    *ap;
CHAN	*mychan;
int     first;
char	**recip;
{
	LIST_RCHAN      *chan;
	char	*to;
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

	if (ap -> ad_resp == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("responsibility bit not set for addr %d",
			 ap -> ad_no));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (recip == NULLVP)	/* mta check not required */
		return OK;

	switch (ap -> ad_type) {
	    case AD_822_TYPE:
		to = ap -> ad_r822adr;
		break;

	    case AD_X400_TYPE:
		to = ap -> ad_r400adr;
		break;

	    default:
	    case AD_ANY_TYPE:
		PP_LOG (LLOG_EXCEPTIONS, ("Bad address type %d",
					  ap -> ad_type));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (to == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS,
			("No recipient for address %d", ap -> ad_no));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (*recip == NULLCP)
		*recip = strdup (to);
	else if (strcmp (*recip, to) != 0) {
		if (first) {	/* ok to change here */
			free (*recip);
			*recip = strdup (to);
		}
		else {
			PP_LOG (LLOG_EXCEPTIONS,
				("Mta changed from %s to %s", *recip,
				 to));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_messageFailure);
			return NOTOK;
		}
	}
	return OK;
}
