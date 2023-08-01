/* submit_mgt.c: management stuff */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_mgt.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_mgt.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: submit_mgt.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "q.h"
#include "dr.h"
#include "ap.h"
#include <sys/time.h>

/* -- externals -- */
extern int max_hops, max_loops;
extern  time_t          time();
extern  int             privileged;
extern	int		return_interval_norm,
			return_interval_low,
			return_interval_high,
			warn_interval;
extern  int             accept_all;
extern	int		notrace;
extern  char            *loc_dom_site, *loc_dom_mta,
			*ia5_bp, *hdr_822_bp,
			io_fpart[];

/* -- globals -- */
CHAN                    *ch_inbound;
char                    *mgt_inhost;
int                     mgt_adtype = AD_822_TYPE;
extern void		message_failure();
extern void		rd_rfchdr ();
extern void		pro_reply ();
extern void		err_abrt ();


/* -- local routines -- */
int			cntrl_mgt();
int			q_mgt();
void			message_failure();
void			msg_mgt();
void			time_mgt();
void			txt_mgt();

static void		tracecheck_mgt();




/* ---------------------  Begin  Routines  -------------------------------- */




int cntrl_mgt (prm, qp)
register struct prm_vars        *prm;
register Q_struct               *qp;
{
	CHAN	*ch_in;

	PP_TRACE (("submit/cntrl_mgt (type = %d)", mgt_adtype));

	accept_all = (prm -> prm_opts & PRM_ACCEPTALL) == PRM_ACCEPTALL;
	notrace = (prm -> prm_opts & PRM_NOTRACE) == PRM_NOTRACE;

	/* -- what incomming channel is calling submit -- */

	if (qp -> inbound == NULL ||
	    (ch_inbound = ch_in = qp -> inbound -> li_chan) == NULLCHAN)
		err_abrt (RP_MECH, "No channel given");

	if (ch_in -> ch_access == CH_MTA) {
		if (!privileged)
			err_abrt (RP_USER, "Privileged channel");
	}
	if (ch_in -> ch_chan_type != CH_BOTH &&
	    ch_in -> ch_chan_type != CH_IN &&
	    ch_in -> ch_chan_type != CH_WARNING) 
		err_abrt (RP_MECH, "Channel %s not inbound",
			  ch_in -> ch_name);

	if (ch_in -> ch_in_ad_subtype == AD_JNT
	    || ch_in -> ch_in_ad_subtype == AD_REAL733)
		ap_use_percent();

	if (ch_in -> ch_domain_norm == CH_DOMAIN_NORM_ALL)
		ap_norm_all_domains();
	else
		ap_norm_first_domain();

	if ((mgt_inhost = qp -> inbound -> li_mta) == NULLCP) {
		mgt_inhost = loc_dom_site;
	} else {
		char    official[LINESIZE], *subdom;

		/* normalise inbound mta */
		if (tb_getdomain (mgt_inhost, NULLCP, official, 
				  ch_in -> ch_ad_order,
				  &subdom) == OK 
		    && official[0]) {
			free(qp->inbound->li_mta);
			mgt_inhost = qp->inbound->li_mta = strdup(official);
		}
		if (subdom != NULLCP) free (subdom);
	}
			
	mgt_adtype = ch_in -> ch_in_ad_type;

	if (mgt_adtype == AD_822_TYPE) {
                qp -> content_return_request = TRUE;
		/* to cope with possible 1148 conversion */
		qp -> alternate_recip_allowed = TRUE;
	}

	PP_TRACE (("submit/cntrl_mgt (type='%d', order='%d')",
		   mgt_adtype, ch_in -> ch_ad_order));

	return (RP_OK);
}



static void hdr_eit_swap (eits, hdrs)
LIST_BPT	*eits;
LIST_BPT	*hdrs;
{
	LIST_BPT	*eits_ix, *hdrs_ix;
	int		cont;

	for (eits_ix = eits; 
	     eits_ix != NULLIST_BPT;
	     eits_ix = eits_ix -> li_next) {
		cont = TRUE;
		for (hdrs_ix = hdrs;
		     hdrs_ix != NULLIST_BPT && cont == TRUE;
		     hdrs_ix = hdrs_ix -> li_next) {
			if (lexnequ(eits_ix->li_name,
				    hdrs_ix->li_name,
				    strlen(eits_ix->li_name)) == 0
			    && (int) strlen(eits_ix->li_name) < (int) strlen(hdrs_ix->li_name)) {
				/* prefix and swap */
				free (eits_ix->li_name);
				eits_ix->li_name = strdup(hdrs_ix->li_name);
				cont = FALSE;
			}
		}
	}
}
	

int q_mgt (qp)
register Q_struct       *qp;
{
	PP_TRACE (("submit/q_mgt (qp)"));

	/* -- generate a submit-event-id -- */
	if (mgt_adtype == AD_822_TYPE ||
	    qp -> msgid.mpduid_string == NULLCP)
		MPDUid_new (&qp -> msgid);

	/* -- add trace info -- */
	trace_add (&qp -> trace, trace_new());

	if (qp -> retinterval == 0) {
		switch (qp -> priority) {
		    case PRIO_URGENT:
			qp -> retinterval = return_interval_high;
			break;
		    case PRIO_NONURGENT:
			qp -> retinterval = return_interval_low;
			break;
		    default:
			qp -> retinterval = return_interval_norm;
			break;
		}
	}
	if (qp -> warninterval == 0)
		qp -> warninterval = warn_interval;

	/* -- override hdr eits that lexnequ to those specified 
	   in inbound channel */
	hdr_eit_swap(qp->encodedinfo.eit_types, 
		     qp->inbound->li_chan->ch_hdr_in);
	return (RP_OK);
}




void time_mgt(qp)
Q_struct *qp;
{
	time_t	then, now;
	char	buf[BUFSIZ];

	if (qp -> latest_time != NULLUTC) {
		then = utc2time_t(qp -> latest_time);
		(void) time(&now);
		if (then < now) {
			(void) sprintf (buf,
					"This message's latest time has been exceeded");
			message_failure (DRR_UNABLE_TO_TRANSFER,
					 DRD_MAX_TIME_EXPIRED,
					 buf);
		}
	}
}




void msg_mgt()
{
	PP_TRACE (("submit/msg_mgt()"));
	ch_inbound = NULLCHAN;
	mgt_inhost = NULLCP;
}




void txt_mgt(qp)   /* -- management tasks on the input text -- */
Q_struct *qp;
{
	PP_TRACE (("submit/txt_mgt()"));


	/* -- if its an 822 body, check it's parts -- */
	if (qp -> msgtype == MT_UMPDU &&
	    list_bpt_nfind (qp -> encodedinfo.eit_types,
			    hdr_822_bp,
			    strlen(hdr_822_bp)) != NULLIST_BPT) {
		if (isnull (*io_fpart))
			err_abrt (RP_MECH, "Help - lost io_part");
		rd_rfchdr (io_fpart);
	}
	if (qp -> encodedinfo.eit_types == NULLIST_BPT) {
		qp -> encodedinfo.eit_types =
			list_bpt_dup (qp -> orig_encodedinfo.eit_types);
		hdr_eit_swap(qp->encodedinfo.eit_types,
			     qp->inbound->li_chan->ch_hdr_in);
	}
	/* -- check for trace looping -- */
	tracecheck_mgt(qp);
	/* -- currently ok -- */
	pro_reply (RP_OK, "text input successful");
}




void message_failure (reason, diag, str)
int	reason, diag;
char	*str;
{
	ADDR	*ap, *apf = NULLADDR;
	extern ADDR *ad_recip;

	for (ap = ad_recip; ap; ap = ap -> ad_next) {
		if (ap -> ad_resp == FALSE)
			continue;
		if (ap -> ad_status != AD_STAT_PEND)
			continue;
		if (apf == NULLADDR)
			apf = ap;
		ap -> ad_status = AD_STAT_DRREQUIRED;
	}
	if (apf) {
		apf -> ad_reason = reason;
		apf -> ad_diagnostic = diag;
		apf -> ad_add_info = strdup (str);
	}
}





/* ---------------------  Static  Routines  ------------------------------- */




static void tracecheck_mgt (qp)
Q_struct	*qp;
{
	register Trace	*tp;
	int		nhops = 0, first = 0;
	char		buf[BUFSIZ];
	PP_TRACE (("submit/tracecheck_mgt (qp)"));

	for (tp = qp -> trace; tp != NULL; tp = tp -> trace_next) {
		/* -- too many hops -- */
		
		if (++nhops > max_hops) {
			(void) sprintf (buf,
				"%s%s %d",
				"This Message has travelled a long time, ",
				"there are too many Trace hops", nhops);
			message_failure (DRR_UNABLE_TO_TRANSFER,
					 DRD_LOOP_DETECTED, buf);
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", buf));
			return;
		}

		if (tp -> trace_mta != NULLCP) {
			/* can only check against those with mta specified */
			if (lexequ(tp -> trace_mta, loc_dom_mta) == 0) {
				if (first >= max_loops) {
					(void) sprintf (buf, "%s%s",
							"A loop has been detected in the Trace at ",
						"field for this Message");
					PP_LOG(LLOG_EXCEPTIONS,
					       ("tracecheck_mgt(%s)", buf));
					message_failure (DRR_UNABLE_TO_TRANSFER,
							 DRD_LOOP_DETECTED, buf);
					return;
				} else
					first++;
			}
		}
	}
}
