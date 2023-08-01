/* dchan_acheck.c: address checking for a channel allowing DR's */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/dchan_acheck.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/dchan_acheck.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: dchan_acheck.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "adr.h"
#include "qmgr.h"

int dchan_acheck (ap, asp, thechan, first, mta)
ADDR    *ap, *asp;
CHAN	*thechan;
int     first;
char	**mta;
{
	ADDR	*ac;
	LIST_RCHAN      *chan;
	int     	i;
	int		donealready = 0;
	static int	drwr = 0;
	static int	pend = 0;

	if (first)
		drwr = pend = 0;

	switch (ap -> ad_status) {
	    case AD_STAT_PEND:
		if (drwr) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Recips have diff status"));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_messageFailure);
			return NOTOK;
		}
		ac = ap;
		break;

	    case AD_STAT_DONE:
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_success);
		return NOTOK;
		
	    case AD_STAT_DRWRITTEN:
		if (pend) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Recips have diff status"));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_messageFailure);
			return NOTOK;
		}
		if (first)
			drwr = ap->ad_no;
		else if (drwr != ap->ad_no) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Mismatch on DR's %d & %d",
				 ap->ad_no, drwr));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_messageFailure);
			return NOTOK;
		}
		ac = asp;
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("adr_checks - wrong status"));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	for (chan = ac -> ad_fmtchan, i = 0; i < ac -> ad_rcnt && chan;
	     chan = chan -> li_next, i++) {
		if (strcmp (chan->li_chan->ch_name, thechan->ch_name) == 0)
			donealready = 1;
	}

	if (chan == NULL)
		chan = ac -> ad_outchan;


	if (chan -> li_chan == NULLCHAN || 
		chan->li_chan->ch_name == NULLCP || 
		thechan->ch_name == NULLCP) 
	{
		PP_NOTICE (("Unable to compare channels - nullchan found"));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}


	if (strcmp (chan -> li_chan -> ch_name, thechan -> ch_name) != 0) {
		if (donealready) {
			PP_NOTICE (("Done channel %s already",
				    thechan -> ch_name));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_success);
			return NOTOK;
		}
		PP_LOG (LLOG_EXCEPTIONS,
			("adr %d not ready for channel %s yet",
			 ap -> ad_no, thechan -> ch_name));
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

	if (mta == NULLVP)	/* mta check not required */
		return OK;

	if (thechan -> ch_mta) {
		*mta = strdup (thechan -> ch_mta);
		return OK;
	}

	if (ac -> ad_outchan -> li_mta == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS,
			("No mta for address %d", ap -> ad_no));
		(void) delivery_set (ap -> ad_no,
				     int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (*mta == NULLCP)
		*mta = strdup (ac -> ad_outchan -> li_mta);
	else if (strcmp (*mta, ac -> ad_outchan -> li_mta) != 0) {
		if (first) {	/* ok to change here */
			free (*mta);
			*mta = strdup (ac -> ad_outchan -> li_mta);
		}
		else {
			PP_LOG (LLOG_EXCEPTIONS,
				("Mta changed from %s to %s", *mta,
				 ac -> ad_outchan -> li_mta));
			(void) delivery_set (ap -> ad_no,
					     int_Qmgr_status_messageFailure);
			return NOTOK;
		}
	}
	return OK;
}

