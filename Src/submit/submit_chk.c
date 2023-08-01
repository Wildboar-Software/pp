/* submit_chk.c: checks done by submit */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_chk.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_chk.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: submit_chk.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "dr.h"

/* -- externals -- */
extern char		*cont_p22;
extern void		message_failure();


/* -- local routines -- */
void			check_conversions();
void			check_crits();
void			check_dr_crits();
int			check_crit();
static void		check_extn_crits();
static void		check_q_crits();
static void		check_rrinfo_crits();




/* ---------------------  Begin  Routines  -------------------------------- */




int check_crit (this, mask, qp, adr, str)
char		this,
		mask;
Q_struct	*qp;
ADDR		*adr;	/* if NULLADDR and fail, fail total message */
char		*str;
{
	char	buf[BUFSIZ];
	if (this != 0 && (this & (~mask))) {
		/* fail on critical */
		(void) sprintf (buf, "Unsupported criticality of '%s'",
				str);
		if (adr == NULLADDR)
			message_failure (DRR_UNABLE_TO_TRANSFER,
					 DRD_UNSUPPORTED_CRITICAL_FUNCTION,
					 buf);
		else {
			if (adr -> ad_status == AD_STAT_PEND) {
				adr -> ad_status = AD_STAT_DRREQUIRED;
				adr -> ad_reason = DRR_UNABLE_TO_TRANSFER;
				adr -> ad_diagnostic = DRD_UNSUPPORTED_CRITICAL_FUNCTION;
				adr -> ad_add_info = strdup(buf);
			}
		}
		return NOTOK;
	}
	return OK;
}




void check_crits (qp)
register Q_struct 	*qp;
{
	ADDR	*ix = qp -> Raddress;
	
	check_extn_crits (qp, qp -> per_message_extensions, 
			  "per_message_extensions");
	while (ix != NULLADDR) {
		if (ix -> ad_status == AD_STAT_PEND)
			check_q_crits (qp, ix);
		ix = ix -> ad_next;
	}
}




void check_dr_crits (qp, dr)
Q_struct *qp;
DRmpdu	*dr;
{	
	Rrinfo	*ix = dr -> dr_recip_list;

	check_extn_crits (qp, dr->dr_per_envelope_extensions,
			  "per_envelope_extensions");
	check_extn_crits (qp, dr -> dr_per_report_extensions,
			  "per_report_extensions");
	
	while (ix != (Rrinfo *) NULL) {
		check_rrinfo_crits (qp, dr, ix);
		ix = ix -> rr_next;
	}
}
	



void check_conversions (qp)
register Q_struct	*qp;
{
	ADDR	*ix;
	LIST_RCHAN	*chan;
	int		cont;
	if (qp -> implicit_conversion_prohibited == TRUE) {
		ix = qp -> Raddress;
		
		while (ix != NULLADDR) {
			if (ix -> ad_status == AD_STAT_PEND
			    && ix -> ad_fmtchan != NULLIST_RCHAN) {
				chan = ix -> ad_fmtchan;
				cont = TRUE;
				while (cont == TRUE
				       && chan != NULLIST_RCHAN) {
					switch (chan -> li_chan -> ch_conversion) {
					    case CH_CONV_CONVERT:
					    case CH_CONV_WITHLOSS:
						/* fail this recipient */
						ix -> ad_status = AD_STAT_DRREQUIRED;
						ix -> ad_reason = DRR_UNABLE_TO_TRANSFER;
						ix -> ad_diagnostic = DRD_CONVERSION_PROHIBITED;
						ix -> ad_add_info = strdup("Conversion of this message was prohibited");
						cont = FALSE;
					    default:
						break;
					}
					chan = chan -> li_next;
				}
			}
			ix = ix -> ad_next;
		}
	}
	if (qp -> conversion_with_loss_prohibited == TRUE) {
		ix = qp -> Raddress;
		while (ix != NULLADDR) {
			if (ix -> ad_status == AD_STAT_PEND
			    && ix -> ad_fmtchan != NULLIST_RCHAN) {
				chan = ix -> ad_fmtchan;
				cont = TRUE;
				while (cont == TRUE
				       && chan != NULLIST_RCHAN) {
					if (chan -> li_chan -> ch_conversion == CH_CONV_WITHLOSS) {
						/* fail this recipient */
						ix -> ad_status = AD_STAT_DRREQUIRED;
						ix -> ad_reason = DRR_UNABLE_TO_TRANSFER;
						ix -> ad_diagnostic = DRD_CONVERSION_PROHIBITED;
						ix -> ad_add_info = strdup("Conversion of this message would incur loss of information");
						cont = FALSE;
					}
					chan = chan -> li_next;
				}
			}
			ix = ix -> ad_next;
		}
	}
}




/* ---------------------  Static Routines  -------------------------------- */




static void check_rrinfo_crits (qp, dr, rr)
Q_struct *qp;
register DRmpdu	*dr;
register Rrinfo	*rr;
{
	char 	mask = CRITICAL_SUBMISSION;
	ADDR	*adr = qp -> Raddress;
	CHAN	*outchan;
	X400_Extension	*ix;

	while (adr != NULLADDR && adr -> ad_no != rr -> rr_recip)
		adr = adr -> ad_next;
	if (adr == NULLADDR)
		return;
	
	/* --- *** ---  
	ad_outchan may be missing if msg is a x400 DR & its Recip is invalid
	--- *** --- */

	if (adr -> ad_outchan == NULLIST_RCHAN ||
		adr -> ad_outchan -> li_chan == NULLCHAN)
			return;

	outchan = adr -> ad_outchan -> li_chan;
	if (lexequ (outchan -> ch_content_out, cont_p22) == 0)
		mask = mask | CRITICAL_TRANSFER | CRITICAL_DELIVERY;
	else if (outchan -> ch_access == CH_MTS)
		mask = mask | CRITICAL_DELIVERY;

	if (check_crit (dr -> dr_dl_history_crit, mask, qp,
			adr, "dr_dl_history_crit") == NOTOK)
		return;
	if (check_crit (dr -> dr_reporting_dl_name_crit, mask, qp,
			adr, "dr_reporting_dl_name_crit") == NOTOK)
		return;
	if (check_crit (dr -> dr_security_label_crit, mask, qp,
			adr, "dr_security_label_crit") == NOTOK)
		return;
	if (check_crit (dr -> dr_reporting_mta_certificate_crit, mask, 
			qp,
			adr, "dr_reporting_mta_certificate_crit") == NOTOK)
		return;
	if (check_crit (dr -> dr_report_origin_auth_check_crit, mask, qp,
			adr, "dr_report_origin_auth_check_crit") == NOTOK)
		return;
	if (check_crit (rr -> rr_redirect_history_crit, mask, qp,
			adr, "rr_redirect_history_crit") == NOTOK)
		return;
	if (check_crit (rr -> rr_physical_fwd_addr_crit, mask, qp,
			adr, "rr_physical_fwd_addr_crit") == NOTOK)
		return;
	if (check_crit (rr -> rr_recip_certificate_crit, mask, qp,
			adr, "rr_recip_certificate_crit") == NOTOK)
		return;
	if (check_crit (rr -> rr_report_origin_authentication_check_crit, 
			mask, qp,
			adr, "rr_report_origin_authentication_check_crit") == NOTOK)
		return;

	ix = rr -> rr_per_recip_extensions;
	while (ix != NULL) {
		if (check_crit (ix -> ext_criticality, CRITICAL_SUBMISSION, 
				qp,
				adr, "rr_per_recip_extensions") == NOTOK)
			return;
		ix = ix -> ext_next;
	}
}




static void check_q_crits (qp, adr)
register Q_struct	*qp;
register ADDR		*adr;
{
	char	mask = CRITICAL_SUBMISSION;
	CHAN	*outchan = adr -> ad_outchan -> li_chan;
	X400_Extension	*ix;

	if (lexequ (outchan -> ch_content_out, cont_p22) == 0)
		mask = mask | CRITICAL_TRANSFER | CRITICAL_DELIVERY;
	else if (outchan -> ch_access == CH_MTS)
		mask = mask | CRITICAL_DELIVERY;

	if (check_crit (qp -> latest_time_crit, mask, qp,
			adr, "latest_time") == NOTOK)
		return;

	if (check_crit(qp -> recip_reassign_crit, mask, qp, adr,
		       "recipient reassignment") == NOTOK)
		return;

	if (check_crit (qp -> dl_expansion_crit, mask, qp, adr,
			"dl expansion") == NOTOK)
		return;
	if (check_crit (qp -> conversion_with_loss_crit, mask, qp, 
			adr, "conversion_with_loss") == NOTOK)
		return;

	if (check_crit (qp -> content_correlator_crit, mask, qp, adr,
			"content_correlator") == NOTOK)
		return;

	if (check_crit(qp -> originator_return_address_crit, mask, qp, 
		       adr, "originator_return_address") == NOTOK)
		return;
	if (check_crit(qp -> forwarding_request_crit, mask, qp, adr,
		       "forwarding_request") == NOTOK)
		return;

	if (check_crit(qp -> originator_certificate_crit, mask, qp, 
		       adr, "originator_certificate") == NOTOK)
		return;

	if (check_crit(qp -> algorithm_identifier_crit, mask, qp, 
		       adr, "algorithm_identifier") == NOTOK)
		return;

	if (check_crit(qp -> message_origin_auth_check_crit, mask, qp, 
		       adr, "message_origin_auth_check") == NOTOK)
		return;
	if (check_crit(qp -> security_label_crit, mask, qp, adr,
		       "security_label") == NOTOK)
		return;

	if (check_crit(qp -> proof_of_submission_crit, mask, qp, adr,
		       "proof_of_submission") == NOTOK)
		return;

	if (check_crit (qp -> dl_expansion_history_crit, mask, qp, 
			adr, "dl_expansion_history") == NOTOK)
		return;

	/* now do addr crits */
	if (check_crit( adr -> ad_orig_req_alt_crit, mask, qp, adr,
		       "ad_orig_req_alt") == NOTOK)
		return;

	if (check_crit (adr -> ad_req_del_crit, mask, qp, adr, 
			"ad_req_del_crit") == NOTOK)
		return;

	if (check_crit (adr -> ad_phys_forward_crit, mask, qp, adr,
			"ad_phys_forward") == NOTOK)
		return;

	if (check_crit (adr -> ad_phys_fw_ad_crit, mask, qp, adr,
			"ad_phys_fw_ad") == NOTOK)
		return;

	if (check_crit (adr -> ad_phys_modes_crit, mask, qp, adr,
			"ad_phys_modes") == NOTOK)
		return;

	if (check_crit (adr -> ad_reg_mail_type_crit, mask, qp, adr,
			"ad_reg_mail_type") == NOTOK)
		return;

	if (check_crit (adr -> ad_recip_number_for_advice_crit, mask, qp, 
			adr, "ad_recip_number_for_advice") == NOTOK)
		return;

	if (check_crit (adr -> ad_phys_rendition_attribs_crit, mask, qp, 
			adr, "ad_phys_rendition_attribs") == NOTOK)
		return;

	if (check_crit (adr -> ad_pd_report_request_crit, mask, qp, 
			adr, "ad_pd_report_request") == NOTOK)
		return;

	if (check_crit (adr -> ad_redirection_history_crit, mask, qp, 
			adr, "ad_redirection_history") == NOTOK)
		return;

	if (check_crit (adr -> ad_message_token_crit, mask, qp, adr,
			"ad_message_token") == NOTOK)
		return;

	if (check_crit (adr -> ad_content_integrity_crit, mask, qp, 
			adr, "ad_content_integrity") == NOTOK)
		return;

	if (check_crit (adr -> ad_proof_delivery_crit, mask, qp, adr,
			"ad_proof_delivery") == NOTOK)
		return;

	ix = adr -> ad_per_recip_ext_list;
	while (ix != NULL) {
		if (check_crit (ix -> ext_criticality, CRITICAL_SUBMISSION, qp,
				adr, "ad_per_recip_ext") == NOTOK)
			return;
		ix = ix -> ext_next;
	}

}




static void check_extn_crits (qp, ix, buf)
Q_struct		*qp;
register X400_Extension	*ix;
char			*buf;
{
	while (ix != NULL) {
		if (check_crit(ix -> ext_criticality, CRITICAL_SUBMISSION, 
			       qp, NULLADDR, buf) == NOTOK)
			return;
		ix = ix -> ext_next;
	}
}

void check_report_level (qp)
Q_struct	*qp;
{
	/* see x.411 12.2.1.1.18 */
	ADDR	*ix;

	/* bad code relies on having same mapping into integers */
	/* see adr.h usrreq and mtareq declarations */

	for (ix = qp->Raddress; ix; ix = ix -> ad_next)
		if (ix -> ad_mtarreq < ix -> ad_usrreq)
			ix -> ad_mtarreq = ix -> ad_usrreq;
}

static CHAN	*getsplitter ()
{
	register CHAN	*spltr, **chp;

	for (chp = ch_all; 
	     (spltr = *chp) != NULLCHAN;
	     chp++)
		if (spltr -> ch_chan_type == CH_SPLITTER)
			return spltr;
	return NULLCHAN;
}

static int	must_solo_proc(ad)
register ADDR	*ad;
{
	register LIST_RCHAN	*ix;
	if (!ad->ad_resp)
		return FALSE;
	
	if (ad->ad_outchan
	    && ad->ad_outchan->li_chan
	    && ad->ad_outchan->li_chan->ch_solo_proc == YES)
		return TRUE;
	
	for (ix = ad->ad_fmtchan; ix != NULLIST_RCHAN; ix = ix -> li_next)
		if (ix -> li_chan
		    && ix -> li_chan -> ch_solo_proc == YES)
			return TRUE;
	return FALSE;
}

static void	replace_with_splitter (ad, spltr)
register ADDR *ad;
CHAN	*spltr;
{
	list_rchan_free (ad->ad_fmtchan);
	ad->ad_fmtchan = NULLIST_RCHAN;
	ad->ad_outchan->li_chan = spltr;
	if (ad->ad_outchan->li_auth
	    && ad->ad_outchan->li_auth->chan)
		ad->ad_outchan->li_auth->chan->li_chan = spltr;
}
	
void check_splitter (qp)
Q_struct	*qp;
{
	CHAN	*spltr;
	register ADDR	*ix;
	int	count;
	if ((spltr = getsplitter()) == NULLCHAN)
		return;

	for (count = 0, ix = qp->Raddress;
	     ix != NULLADDR && count < 2;
	     ix = ix -> ad_next) 
		if (must_solo_proc (ix) == TRUE)
			count++;

	if (count < 2)
		return;

	/* have to replace some with splitter */

	for (ix = qp->Raddress;
	     ix != NULLADDR;
	     ix = ix -> ad_next) {
		if (must_solo_proc (ix) == TRUE)
			replace_with_splitter(ix, spltr);
	}
}
			
	
