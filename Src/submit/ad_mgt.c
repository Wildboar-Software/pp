/* ad_mgt.c: address management routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/ad_mgt.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/ad_mgt.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: ad_mgt.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <varargs.h>
#include "retcode.h"
#include <isode/cmd_srch.h>
#include "q.h"
#include "dr.h"
#include "ap.h"

extern int		accept_all;

extern void		x400_add(), rfc822_add(), err_abrt(), pro_reply();
extern void		redirect_free ();
extern char		*getpostmaster();

/* -- globals -- */
static int		ad_extend;


/* -- local routines -- */
int		rplose ();
int		validate_sender ();
void		ad_init ();
int		validate_recip ();
void		ad_log_print ();

static int		bad_address();
static int		recip_parse();
static int		do_sender_dr_outchan();
static void		set_return_chan();

#define gval(x)			((x -> ad_type) == AD_X400_TYPE ? \
				 (x -> ad_r400adr) : (x -> ad_r822adr))	


/* ---------------------  Begin	 Routines  -------------------------------- */


ADDR *read_sender (qp)
Q_struct *qp;
{
	RP_Buf	rpbuf,
		*rp = &rpbuf;
	ADDR	*ad = NULLADDR;
	int 	retval;
	int type = qp -> inbound -> li_chan -> ch_in_ad_type;

	PP_TRACE (("submit/read_sender (%d)", type));
	rp -> rp_val = RP_OK;

	if ((retval = rd_adr (stdin, TRUE, &ad)) == RP_DONE)
		err_abrt (retval, "Error reading sender");


	if (rp_isbad (retval) || ad -> ad_value == NULLCP) {
		if (ad == NULLADDR)
			err_abrt (RP_EOF, "No sender given");

		err_abrt (retval, "Invalid sender '%s'",
			  isstr (ad -> ad_value) ? ad -> ad_value : "(none)");
	}


	ad -> ad_resp = FALSE;
	if (qp -> msgtype == MT_DMPDU)
		ad -> ad_status = AD_STAT_PEND;
	else
		ad -> ad_status = AD_STAT_DONE;
	ad -> ad_extension = 0;
	ad -> ad_no = 0;
	ad -> ad_type = type;



	if (qp -> msgtype == MT_DMPDU)
		ad -> ad_resp = TRUE;

	return ad;
}

static void copy_drstuff(ad, newad, isPostie)
ADDR	*ad, *newad;
int	isPostie;
{
	LIST_RCHAN	*tmpchan;
	char		*tmpchar;

	tmpchan = ad->ad_fmtchan;
	ad->ad_fmtchan = newad->ad_fmtchan;
	newad->ad_fmtchan = tmpchan;

	tmpchan = ad->ad_outchan;
	ad->ad_outchan = newad->ad_outchan;
	newad->ad_outchan = tmpchan;
			
	if (isPostie == TRUE) {
		tmpchar = ad->ad_r400DR;
		ad->ad_r400DR = newad->ad_r400adr;
		newad->ad_r400adr = tmpchar;

		tmpchar = ad->ad_r822DR;
		ad->ad_r822DR = newad->ad_r822adr;
		newad->ad_r822adr = tmpchar;
	}
}

static int x400routeback (ad, inbound, mta)
ADDR	*ad;
CHAN	*inbound;
char	*mta;
{
	LIST_RCHAN	*outchans = NULLIST_RCHAN, *ix;
	int		found;
	char		buf[BUFSIZ];
	/* attempt to route back x400 message to inbound mta */
	
	if (tb_getchan (mta, &outchans) != OK)
		/* can't find inbound mta in routing tables */
		return NOTOK;
	ix = outchans;
	found = FALSE;
	while (ix != NULLIST_RCHAN && found == FALSE) {
		if (ix -> li_chan != NULLCHAN
		    && ix -> li_chan  == inbound) {
			/* found routing via inbound */
			/* now check that can route to inbound mta that way */
			if ((ix -> li_mta == NULLCP
			    || lexequ(ix -> li_mta, mta) == 0)
			    && (ix -> li_chan -> ch_table == NULLTBL
				|| tb_k2val(ix -> li_chan -> ch_table,
					    mta, buf, 1) == OK))
				found = TRUE;
		}
		if (found == FALSE)
			ix = ix -> li_next;
	}

	if (found == TRUE) {
		/* can routeback via x400 */
		if (ad->ad_outchan != NULLIST_RCHAN)
			list_rchan_free(ad->ad_outchan);
		ad->ad_outchan = ix;
		/* remove all but ad->ad_outchan */
		if (ad -> ad_outchan -> li_next != NULLIST_RCHAN) {
			list_rchan_free (ad->ad_outchan->li_next);
			ad->ad_outchan->li_next = NULLIST_RCHAN;
		}
		if (outchans != ad->ad_outchan) {
			for (ix = outchans;
			     ix != NULLIST_RCHAN &&
			     ix -> li_next != ad->ad_outchan;
			     ix = ix -> li_next)
			  continue;
			if (ix != NULLIST_RCHAN)
				ix -> li_next = NULLIST_RCHAN;
			list_rchan_free(outchans);
		}
		PP_NOTICE (("Will route DRs for unrouteable sender '%s' via inbound mta '%s'",
			    ad->ad_value,
			    mta));
		return OK;
	}
	PP_LOG (LLOG_EXCEPTIONS,
		("unable to route DRs to inbound mta '%s' via channel '%s' for unrouteable sender '%s'",
		 mta, inbound->ch_name, ad->ad_value));
	return NOTOK;
}

static void removeNon822chans (plist)
LIST_RCHAN	**plist;
{
	LIST_RCHAN	*ix, *tmp;
	/* remove all non 822 channels */

	/* remove ones at front of queue */
	while (*plist != NULLIST_RCHAN
	       && (*plist) -> li_chan -> ch_out_ad_type != AD_822_TYPE) {
		tmp = *plist;
		*plist = tmp -> li_next;
		tmp -> li_next = NULLIST_RCHAN;
		list_rchan_free(tmp);
	}

	if (*plist != NULLIST_RCHAN) {
		ix = *plist;
		while (ix -> li_next != NULLIST_RCHAN) {
			if (ix -> li_next -> li_chan -> ch_out_ad_type != AD_822_TYPE) {
				tmp = ix -> li_next;
				ix -> li_next = tmp -> li_next;
				tmp -> li_next = NULLIST_RCHAN;
				list_rchan_free (tmp);
			} else
				ix  = ix -> li_next;
		}
	}
}

static int r822routeback (ad, inbound, mta)
ADDR	*ad;
CHAN	*inbound;
char	*mta;
{
	AP_ptr	tree, group, name, local, domain, route;
	RP_Buf	rpbuf;
	ADDR	*newad;
	char	buf[BUFSIZ];

	/* attempt to source route DRs back via inbound mta */

	if (ap_s2p(ad->ad_value, &tree, &group, &name, &local,
		   &domain, &route) != (char *) NOTOK) {
		char	*str;
		AP_ptr	newap = ap_new (AP_DOMAIN,
					mta);
		if (domain == NULLAP)
			str = ap_p2s (group, name, local,
				      newap, NULLAP);
		else if (route == NULLAP)
			str = ap_p2s (group, name, local,
				      domain, newap);
		else {
			newap->ap_next = route;
			newap->ap_ptrtype = AP_PTR_MORE;
			str = ap_p2s (group, name, local,
				      domain, newap);
			newap->ap_next = NULLAP;
		}
		ap_sqdelete(tree, NULLAP);
		ap_free(tree);
		ap_free(newap);
		strcpy(buf, str);
		free(str);
	} else {
		PP_LOG (LLOG_EXCEPTIONS,
			("unable to parse sender '%s' so unwilling to route DRs via inbound mta",
			 ad->ad_value));
		return NOTOK;
	}
			
	newad = adr_new (buf,
			 inbound->ch_in_ad_type,
			 0);
	newad->ad_resp = FALSE;

	if (rp_isbad (ad_parse (newad, &rpbuf,
				inbound->ch_ad_order))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("unable to route DRs to inbound mta '%s' for bad sender '%s' [%s]",
			 mta,
			 ad -> ad_value,
			 rpbuf.rp_line));
		adr_free (newad);
		return NOTOK;
	}

	removeNon822chans(&(newad->ad_outchan));
	if (newad->ad_outchan == NULLIST_RCHAN) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Unable to route DRs to inbound mta '%s' via an rfc 822 channel for unrouteable sender '%s'",
			 mta, ad->ad_value));
		adr_free(newad);
		return NOTOK;
	}

	/* copy various things across */
	copy_drstuff(ad, newad, FALSE);
	adr_free (newad);
	PP_NOTICE(("Will source route DRs for unrouteable sender '%s' via inbound mta '%s'",
		   ad->ad_value, mta));
	return OK;
}

static int postmasterrouteback (ad, inbound, mta)
ADDR	*ad;
CHAN	*inbound;
char	*mta;
{
	RP_Buf	rpbuf;
	ADDR	*newad;

	/* attempt to route DRs to 822 postmaster */
	PP_NOTICE(("Failed to route DRs for unrouteable sender '%s' back to inbound mta '%s', now attepmting sloppy mode routing to postmaster",
		   ad->ad_value, mta));

	newad = adr_new(getpostmaster(inbound->ch_in_ad_type),
			inbound->ch_in_ad_type,
			0);
	newad->ad_resp = FALSE;
	if (rp_isbad (ad_parse (newad, &rpbuf, 
				inbound->ch_ad_order))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("parsing of postmaster address '%s' fails [%s]",
			 newad->ad_value,
			 rpbuf.rp_line));
		adr_free (newad);
		return NOTOK;
	}

	removeNon822chans(&(newad->ad_outchan));
	if (newad->ad_outchan == NULLIST_RCHAN) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Unable to route DRs to postmaster '%s' via an rfc 822 channel for unrouteable sender '%s'",
			 newad->ad_value, ad->ad_value));
		adr_free(newad);
		return NOTOK;
	}
	
	/* copy various things across */
	PP_NOTICE(("Will route DRs for bad sender '%s' to postmaster '%s'",
		   ad->ad_value, newad->ad_value));
	copy_drstuff(ad, newad, TRUE);
	adr_free (newad);
	return OK;
}

int validate_sender(qp, ad, user, rp) /* -- read in sender and validate -- */
ADDR	*ad;
Q_struct *qp;
char *user;
RP_Buf *rp;
{
  CHAN	*ch_in = qp -> inbound -> li_chan;
  int	oldresp;
  /* --- *** --- 
     Reject bad senders unless we can figure out some hack to
     return them.
     --- *** ---  */

  ad_extend = 1;		/* -- reset -- */
	
  oldresp = ad -> ad_resp;
  ad->ad_resp = FALSE;
  if (rp_isbad (ad_parse (ad, rp, ch_in -> ch_ad_order))) {
    if (ch_in->ch_badSenderPolicy == CH_BADSENDER_SLOPPY
	&& ((ch_in->ch_in_ad_type == AD_X400_TYPE 
	     && x400routeback(ad, ch_in, qp -> inbound -> li_mta) == OK)
	    /* managed to route back via x400 to inbound mta */
	    || (ch_in->ch_in_ad_type == AD_822_TYPE
		&& r822routeback (ad, ch_in, qp->inbound->li_mta) == OK)
	    /* managed to route back via 822 to inbound mta */
	    || postmasterrouteback (ad, ch_in, qp->inbound->li_mta) == OK
	    /* managed to route back to 822 local postmaster */))
      /* managed to route DRs somehow */
      rp -> rp_val = RP_OK;
    else {
      /* bad sender bounce */
      PP_LOG(LLOG_EXCEPTIONS,
	     ("Bounced message due to unrouteable sender"));
      return rp -> rp_val;
    }

  }
  ad->ad_resp = oldresp;

  PP_NOTICE (("Sender '%s'", ad -> ad_value));

  if (ch_in -> ch_access == CH_MTS && user != NULLCP &&
      tb_checkauth (gval(ad), ch_in,
		    user, NULLCP) == NOTOK)
    return rplose (rp, RP_USER,
		   "User %s not allowed to submit as %s",
		   gval(ad), user);

  ad -> ad_resp = FALSE;
  return do_sender_dr_outchan (ad, qp, rp);
}

ADDR *read_recipient (qp)
Q_struct *qp;
{
	int		retval;
	RP_Buf rps, *rp = &rps;
	ADDR		*ad;
	int type = qp -> inbound -> li_chan -> ch_in_ad_type;

	PP_TRACE (("submit/read_recip()"));

	ad = NULLADDR;

	retval = rd_adr (stdin, TRUE, &ad);

	switch (retval) {
	    case RP_OK:
		if (ad -> ad_value == NULLCP)
			break;
		ad -> ad_type = type;
		if (ad -> ad_resp == NO)
			ad -> ad_status = AD_STAT_DONE;
		return ad;

	    case RP_DONE:
		return NULL;

	}
	PP_TRACE (("submit/read_recipient(def=%s)",
		   rp_valstr (retval)));
	rp -> rp_val = retval;
	(void) strcpy(rp -> rp_line, "Empty address ?");
	if (ad == NULLADDR)
		ad = adr_new(" ", NULL, NULL);
	else
		ad -> ad_value = strdup(" ");
	ad->ad_r822adr = strdup(" ");
	x400_add(ad);
	ad->ad_type = type;
	ad -> ad_no = ad_extend++;
	if (ad -> ad_extension == 0)
		ad -> ad_extension = ad -> ad_no;
	if (ad -> ad_resp)
		ad->ad_status = AD_STAT_PEND;
	else
		ad->ad_status = AD_STAT_DONE;
	return ad;
}




static void set_return_chan (ad, chan, host)
ADDR	*ad;
CHAN	*chan;
char	*host;
{
	LIST_RCHAN	*ch;

	if (chan == NULLCHAN || ad == NULLADDR || host == NULLCP)
		err_abrt (RP_BAD, "set_return_chan called with NULL fields");

	PP_TRACE (("set_return_chan (chan='%s', host='%s', drchan='%s')", 
		chan -> ch_name ? chan -> ch_name : "", 
		host ? host : "", 
		chan -> ch_drchan ? chan -> ch_drchan : ""));

	if (isstr (chan -> ch_drchan)) 
		ch = list_rchan_new (host, chan -> ch_drchan);
	else
		ch = list_rchan_new (host, chan -> ch_name);

	if (ad -> ad_outchan)
		list_rchan_free (ad -> ad_outchan);
	ad -> ad_outchan = ch;
}


static int do_sender_dr_outchan (ad, qp, rp)
ADDR	*ad;
Q_struct *qp;
RP_Buf *rp;
{
	PP_TRACE (("do_sender_dr_outchan (%s)", ad -> ad_value));

	if (ad -> ad_outchan == NULLIST_RCHAN ||
	    ad -> ad_outchan -> li_chan == NULLCHAN)
		return rplose (rp, RP_BAD, "DR has no channel");

	if (fillin_orig_outchan (qp, ad) == NOTOK)
		return rplose (rp, RP_BAD, "Can't find channel for DR");

	set_return_chan (ad, ad -> ad_outchan -> li_chan,
			 ad -> ad_outchan -> li_mta);
	return RP_OK;
}


/* -- read recip, & validate before adding to list. -- */
int validate_recip(ad, qp, rp)
ADDR	*ad;
Q_struct *qp;
RP_Buf *rp;
{
	int	retval;

	PP_TRACE (("submit/validate_recip()"));

	ad -> ad_no = ad_extend++;
	if (ad -> ad_extension == 0)
		ad -> ad_extension = ad -> ad_no;

	switch (ad -> ad_status) {
	case AD_STAT_DRWRITTEN:
		if (ad -> ad_resp == FALSE) {
			(void) rplose (rp, RP_AOK, "DR '%s'", ad -> ad_value);
			break;
		}
		else if (ad -> ad_no != DR_FILENO_DEFAULT)
			ad -> ad_status = AD_STAT_DONE;
		/* fall */

	case AD_STAT_PEND:
		retval = recip_parse (ad, qp, rp);
		if (rp_isbad (retval)) {
			if (rp_isbad (bad_address (ad, qp, rp)))
				return rp -> rp_val;
			break;
		}

		(void) rplose (rp, RP_AOK, "Nice address %s", ad -> ad_value);
		break;

	default:
		(void) rplose (rp, RP_AOK, "Skip '%s'", ad -> ad_value);
		break;

	}
	return rp -> rp_val;
}

static int bad_address (ad, qp, rp)
ADDR	*ad;
Q_struct *qp;
RP_Buf	*rp;
{
	int	retval = rp -> rp_val;

	if (rp_gbval (retval) == RP_BTNO &&
	    retval != RP_BHST &&
	    retval != RP_DHST)
		return retval;	/* temp failure */

	if (accept_all && qp -> msgtype != MT_DMPDU) {
		ad -> ad_parse_stat = retval;
		ad -> ad_status = AD_STAT_DRREQUIRED; 
		ad -> ad_reason =  DRR_UNABLE_TO_TRANSFER; 
		ad -> ad_diagnostic = DRD_UNRECOGNISED_OR; 
		ad -> ad_add_info = strdup (rp -> rp_line);
		(void) rplose (rp, RP_AOK,
			       "Recipient %s failed to parse: DR to be generated",
				ad -> ad_value);
	}
	else if (accept_all && qp -> msgtype == MT_DMPDU)
		return rp -> rp_val;
	else {
		ad -> ad_resp = FALSE;
		ad -> ad_status = AD_STAT_DONE;
	}

	return rp -> rp_val;
}

extern Redirection *redirect_new();
extern void redirect_add(), redirect_rewind();
extern char *adminstration_req_alt;

static int recip_parse (ad, qp, rp)
ADDR	*ad;
Q_struct *qp;
RP_Buf	*rp;
{
	int	retval;
	char	*orig;
	Redirection	*rewind;
	PP_TRACE (("submit/recip_parse()"));

	retval = ad_parse (ad, rp, qp -> inbound -> li_chan  -> ch_ad_order);

	if (rp_isbad (retval) 
	    && qp->alternate_recip_allowed == TRUE
	    && qp->recip_reassign_prohibited != TRUE
	    && isstr(ad -> ad_orig_req_alt)) {
		char	buf[BUFSIZ];

		PP_TRACE(("attempting ad_orig_req_alt '%s'", 
			  ad -> ad_orig_req_alt));
		if (ad->ad_type == AD_X400_TYPE) {
			if (isstr(ad->ad_r400adr))
				strcat(buf, ad->ad_r400adr);
			else
				strcat(buf, ad->ad_value);
		} else {
			OR_ptr	or = NULLOR;
			
			if (isstr(ad->ad_r822adr))
				or_rfc2or_aux(ad->ad_r822adr, &or, ad->ad_no);
			else
				or_rfc2or_aux(ad->ad_value, &or, ad->ad_no);
			if (or != NULLOR) {
				or_or2std(or, buf, FALSE);
				or_free(or);
			} else
				buf[0] = '\0';
		}
		rewind = redirect_new(buf,
				      (isstr(ad->ad_dn)) ?
				      ad->ad_dn : "",
				      NULLUTC,
				      RDR_ORIG_ASSIGNED);
		if (redirect_before(ad->ad_redirection_history,
				    rewind) == TRUE) {
			PP_NOTICE(("redirection loop detected for '%s'",
				   buf));
			redirect_free(rewind);
		} else {
			redirect_add(&(ad->ad_redirection_history),
				     rewind);
			orig = ad->ad_value;

			ad -> ad_value = strdup (ad -> ad_orig_req_alt);
			if (ad -> ad_r400adr != NULLCP) {
				free(ad -> ad_r400adr);
				ad -> ad_r400adr = NULLCP;
			}
			if (ad -> ad_r822adr != NULLCP) {
				free(ad -> ad_r822adr);
				ad -> ad_r822adr = NULLCP;
			}

			aparse_rewindx400(ad -> aparse);
			aparse_rewindr822(ad -> aparse);
			if (ad->ad_parse_message) {
				free(ad->ad_parse_message);
				ad->ad_parse_message = NULLCP;
			}
			if (rp_isbad(retval = ad_parse (ad, rp,
							qp -> inbound -> li_chan  -> ch_ad_order))) {
				redirect_rewind(&(ad->ad_redirection_history),
						rewind);
				if (ad->ad_value) free(ad->ad_value);
				ad->ad_value = orig;
				if (ad -> ad_r400adr != NULLCP) {
					free(ad -> ad_r400adr);
					ad -> ad_r400adr = NULLCP;
				}
				if (ad -> ad_r822adr != NULLCP) {
					free(ad -> ad_r822adr);
					ad -> ad_r822adr = NULLCP;
				}
			} else {
				if (orig) free(orig);
			}
		}
	}	

	if (rp_isbad(retval) && isstr(adminstration_req_alt)) {
		char	buf[BUFSIZ];
		char	*saveerror = NULLCP;

		PP_TRACE(("attempting adminstration_req_alt '%s'",
			  adminstration_req_alt));

		if (ad->ad_type == AD_X400_TYPE) {
			if (isstr(ad->ad_r400adr))
				strcat(buf, ad->ad_r400adr);
			else
				strcat(buf, ad->ad_value);
		} else {
			OR_ptr	or = NULLOR;
			
			if (isstr(ad->ad_r822adr))
				or_rfc2or_aux(ad->ad_r822adr, &or, ad->ad_no);
			else
				or_rfc2or_aux(ad->ad_value, &or, ad->ad_no);
			if (or != NULLOR) {
				or_or2std(or, buf, FALSE);
				or_free(or);
			} else
				buf[0] = '\0';
		}
		
		rewind = redirect_new(buf,
				      (isstr(ad->ad_dn)) ?
				      ad->ad_dn : "",
				      NULLUTC,
				      RDR_MD_ASSIGNED);
		if (redirect_before(ad->ad_redirection_history,
				    rewind) == TRUE) {
			PP_NOTICE(("redirection loop detected for '%s'",
				   buf));
			redirect_free(rewind);
		} else {
			redirect_add(&(ad->ad_redirection_history),
				     rewind);
			orig = ad->ad_value;
			ad -> ad_value = strdup (adminstration_req_alt);
			if (ad -> ad_r400adr != NULLCP) {
				free(ad -> ad_r400adr);
				ad -> ad_r400adr = NULLCP;
			}
			if (ad -> ad_r822adr != NULLCP) {
				free(ad -> ad_r822adr);
				ad -> ad_r822adr = NULLCP;
			}

			aparse_rewindx400(ad -> aparse);
			aparse_rewindr822(ad -> aparse);
			saveerror = ad->ad_parse_message;
			ad->ad_parse_message = NULLCP;

			if (rp_isbad(retval = ad_parse (ad, rp,
							qp -> inbound -> li_chan  -> ch_ad_order))) {
				redirect_rewind(&(ad->ad_redirection_history),
						rewind);
				if (ad -> ad_r400adr != NULLCP) {
					free(ad -> ad_r400adr);
					ad -> ad_r400adr = NULLCP;
				}
				if (ad -> ad_r822adr != NULLCP) {
					free(ad -> ad_r822adr);
					ad -> ad_r822adr = NULLCP;
				}
				if (ad->ad_parse_message) 
					free(ad->ad_parse_message);
				ad->ad_parse_message = saveerror;
			} else {
				if (saveerror)
					free(saveerror);
			}
			/* maintain previous ad_value for */
			/* adminstration_req_alt redirects */
			if (ad->ad_value) free(ad->ad_value);
			ad->ad_value = orig;
		}
	}
		
	if (!accept_all && rp_isbad (retval)) {
		PP_TRACE (("recip_parse failed (%s %s)",
			   rp_valstr((int)rp -> rp_val),
			   rp -> rp_line));
		return retval;
	}


	/* -- Patch up the local x400adr -- */

	if (!rp_isbad (retval)) {
		if (ad -> ad_r400adr == NULLCP && ad -> ad_type == AD_822_TYPE)
			x400_add (ad);

		if (ad -> ad_r822adr == NULLCP && ad -> ad_type == AD_X400_TYPE)
			rfc822_add (ad);
	}
	else {
		if (ad -> ad_r400adr == NULLCP)
			ad -> ad_r400adr = strdup (ad -> ad_value);
		if (ad -> ad_r822adr == NULLCP)
			ad -> ad_r822adr = strdup (ad -> ad_value);
	}
	return (retval);
}




void ad_log_print (ad)
ADDR	       *ad;
{
	char	*recip;
	if (ad == NULLADDR)
		return;


	switch (ad -> ad_type) {
	    case AD_X400_TYPE:
		recip = (ad -> ad_r400adr) ? ad -> ad_r400adr : ad -> ad_value;
		break;
	    case AD_822_TYPE:
		recip = (ad -> ad_r822adr) ? ad -> ad_r822adr : ad -> ad_value;
		break;
	    default:
		recip = ad->ad_value;
		break;
	}
	PP_NOTICE (("Recipient '%s'  mta '%s'",
		    recip,
		    (ad -> ad_outchan && ad -> ad_outchan -> li_mta) ?
		    ad -> ad_outchan -> li_mta : "<unknown>"));

	ad_log_print (ad -> ad_next);
}



#ifdef lint
/* VARARGS3 */
int rplose (rp, val, str)
RP_Buf	*rp;
int	val;
char	*str;
{
	return rplose (rp, val, str);
}
#else
int rplose (va_alist)
va_dcl
{
	va_list ap;
	RP_Buf	*rp;
	int	val;
	char	buf[BUFSIZ];

	va_start (ap);

	rp = va_arg (ap, RP_Buf *);
	val = va_arg (ap, int);

	_asprintf (buf, NULLCP, ap);

	if (rp_isbad (val))
		PP_LOG (LLOG_EXCEPTIONS, ("rplose (%s, '%s')",
					  rp_valstr (val), buf));

	rp -> rp_val = val;
	(void) strncpy (rp -> rp_line, buf, sizeof (rp -> rp_line) - 1);

	va_end (ap);

	return val;
}
#endif
