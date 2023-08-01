/* gen_probedr.c: generate DRs or NDRs for probes */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_probedr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_probedr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: gen_probedr.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "dr.h"

extern char             msg_fullpath[];

extern void 		gen_ndr (), err_abrt();
/* -- local routines -- */
void			gen_probedr();




/*----------------------  Begin  Routines  -------------------------------- */




/* -- Generate NDRs or DRs for submitted probe -- */


void gen_probedr(qp)
Q_struct *qp;
{
	register ADDR           *ad,
				*sender;
	int			found = FALSE;

	PP_TRACE (("submit/gen_probedr()"));

	/* -- clear reformatting for sender -- */

	for (ad = qp->Oaddress; ad != NULLADDR; ad = ad->ad_next) {
		if (ad -> ad_fmtchan) { 
			list_rchan_free (ad -> ad_fmtchan);
			ad -> ad_fmtchan = NULLIST_RCHAN;
		}
	}

	/* -- First generate Non Delivery Notifications -- */

	for (ad = qp->Raddress; ad != NULLADDR; ad = ad->ad_next) {
		if (ad->ad_status != AD_STAT_PEND &&
		    ad->ad_status != AD_STAT_DRREQUIRED)
			continue;

		if (rp_isbad (ad->ad_parse_stat) ||
		    ad -> ad_status == AD_STAT_DRREQUIRED)
			found = TRUE;

		if (ad -> ad_fmtchan) { 
			list_rchan_free (ad -> ad_fmtchan);
			ad -> ad_fmtchan = NULLIST_RCHAN;
		}
	}


	if (found )
		gen_ndr(qp);


	/* -- Secondly Generate Delivery Notifications -- */

	sender = qp->Oaddress;
	if (qp -> queuetime == NULLUTC)
		qp -> queuetime = utcnow();

	for (ad=qp->Raddress, found=FALSE; ad != NULLADDR; ad = ad->ad_next) {
		if (ad->ad_status != AD_STAT_PEND) 
			continue;
		if (ad->ad_outchan->li_chan->ch_probe)
			continue;

		found = TRUE;
		ad->ad_type = sender->ad_type;
		ad->ad_status = AD_STAT_DRREQUIRED;
		ad -> ad_reason = DRR_NO_REASON;
	}
		
	if (found)
		if (wr_q2drfile (qp, msg_fullpath, 0) != RP_OK)
			err_abrt (RP_AGN, "Error writing DR");

	return;
}
