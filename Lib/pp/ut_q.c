/* ut_q.c: Queue utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_q.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_q.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_q.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "q.h"




/* ---------------------  Begin  Routines  -------------------------------- */




void q_init (qp)
Q_struct        *qp;
{
	PP_DBG (("Lib/pp/q_init()"));
	bzero ((char *)qp, sizeof(*qp));
	qp->msgtype = MT_UMPDU;
	qp -> nwarns = NOTOK;
	qp -> disclose_recips = FALSE;
	qp -> forwarding_request = NOTOK;
}

void q_free (qp)
Q_struct        *qp;
{
	PP_DBG (("Lib/pp/q_free()"));

	if (qp->defertime)
		free ((char *)qp -> defertime);
	if (qp -> latest_time)
		free ((char *)qp -> latest_time);
	if (qp -> cont_type)
		free (qp -> cont_type);
	encodedinfo_free (&qp->encodedinfo);
	encodedinfo_free (&qp -> orig_encodedinfo);
	if (qp->ua_id != NULLCP)
		free (qp->ua_id);
	if (qp -> pp_content_correlator)
		free (qp -> pp_content_correlator);
	if (qp -> general_content_correlator)
		qb_free (qp -> general_content_correlator);
	if (qp -> originator_return_address)
		fn_free (qp -> originator_return_address);
	if (qp -> originator_certificate)
		qb_free (qp -> originator_certificate);
	if (qp -> algorithm_identifier)
		qb_free (qp -> algorithm_identifier);
	if (qp -> message_origin_auth_check)
		qb_free (qp -> message_origin_auth_check);
	if (qp -> security_label)
		qb_free (qp -> security_label);
	if (qp -> per_message_extensions)
		extensions_free (qp -> per_message_extensions);
	if (qp->Oaddress)
		adr_tfree (qp->Oaddress);
	if (qp->Raddress)
		adr_tfree (qp->Raddress);
	if (qp->inbound != NULL)
		list_rchan_free(qp -> inbound);
	MPDUid_free (&qp->msgid);
	if (qp->trace)
		trace_free (qp->trace);
	if (qp -> dl_expansion_history)
		dlh_free (qp -> dl_expansion_history);
	if (qp->queuetime)
		free ((char *)qp -> queuetime);
	if (qp->departime)
		free ((char *)qp -> departime);
	q_init (qp);
}

void eit_dup(to, from)
EncodedIT	*to, *from;
{
	to->eit_g3parms = from->eit_g3parms;
	to->eit_tTXparms = from->eit_g3parms;
	to->eit_presentation = from->eit_presentation;
	to->eit_types = list_bpt_dup(from->eit_types);
}
	
void q_almost_dup (to, from)
Q_struct	*to, *from;
{
	if (from->latest_time != NULLUTC)
		to -> latest_time = utcdup(from -> latest_time);
	to->latest_time_crit = from->latest_time_crit;

	if (from->cont_type != NULLCP)
		to->cont_type = strdup(from->cont_type);
	eit_dup(&(to->orig_encodedinfo), &(from->orig_encodedinfo));
	to->priority = from->priority;
	
	to->disclose_recips = from->disclose_recips;
	to->implicit_conversion_prohibited = from->implicit_conversion_prohibited;
	to->alternate_recip_allowed = from->alternate_recip_allowed;
	to->content_return_request = from->content_return_request;
	
	to->recip_reassign_prohibited = from->recip_reassign_prohibited;
	to->recip_reassign_crit = from->recip_reassign_crit;
	
	to->dl_expansion_prohibited = from->dl_expansion_prohibited;
	to->dl_expansion_crit = from->dl_expansion_crit;
	
	to->conversion_with_loss_prohibited = from->conversion_with_loss_prohibited;
	to->conversion_with_loss_crit = from->conversion_with_loss_crit;
	
	if (from->ua_id)
		to->ua_id = strdup(from->ua_id);
	if (from->pp_content_correlator)
		to->pp_content_correlator = strdup(from->pp_content_correlator);
/* need to duplicate qbufs */
/*	general_content_correlator */
	to->content_correlator_crit = from->content_correlator_crit;
	
/*	originator_return_address */
	to->originator_return_address_crit = 
		from->originator_return_address_crit;
	
	to->forwarding_request = from->forwarding_request;
	to->forwarding_request_crit = from->forwarding_request_crit;
	
/*	originator_certificate */
	to->originator_certificate_crit = from->originator_certificate_crit;
	
/*	algorithm_identifier */
	to->algorithm_identifier_crit = from->algorithm_identifier_crit;

/*	message_origin_auth_check */
	to->message_origin_auth_check_crit = 
		from->message_origin_auth_check_crit;

/*	security_label */
	to->security_label_crit = from->security_label_crit;

	to->proof_of_submission_request = from->proof_of_submission_request;
	to->proof_of_submission_crit = from->proof_of_submission_crit;

/*	per_message_extensions */

	to->dl_expansion_history = dlh_dup (from->dl_expansion_history);

}
