/* auth.c: authorisation procedures used by submit */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/auth.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/auth.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: auth.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "dr.h"
#include "tb_auth.h"
#include <isode/cmd_srch.h>



#define ORED			FALSE
#define freason(i)		(rcmd_srch (i, authtbl_reasons))
#define rrights(i)		(rcmd_srch (i, authtbl_rights))
#define gval(x)			((x -> ad_type) == AD_X400_TYPE ? \
				 (x -> ad_r400adr) : (x -> ad_r822adr))	


extern CMD_TABLE		authtbl_rights[];
extern CMD_TABLE		authtbl_loglevel[];
extern LIST_AUTH_CHAN		*authchan_new();
extern LIST_AUTH_MTA		*authmta_new ();
extern AUTH_USER		*authusr_new();
extern AUTH			*auth_new();
extern char			*re_comp();
extern CMD_TABLE		authtbl_reasons[];
extern char             	*authchannel;
extern char             	*authloglevel;
extern char			*loc_dom_site;
extern void			do_reason(), err_abrt();
extern void			tb_parse_authchan();
extern void			authchan_add(), authmta_add();
/* -- static  variables -- */
static AUTH                     *sender_auth	= NULL_AUTH;
static LIST_AUTH_CHAN		*def_authchan	= NULLIST_AUTHCHAN;
static LIST_AUTH_CHAN		*authchan_start = NULLIST_AUTHCHAN;
static LIST_AUTH_MTA		*authmta_start	= NULLIST_AUTHMTA;


/* -- globals -- */
char				auth2submit_msg[BUFSIZ];
int				auth_loglev;


/* -- local routines -- */
int			 	auth_finish();
int				fillin_orig_outchan();
void				auth_start();

static Bindings			*bindings_new();
static Bindings			*do_ch_binds();
static char			*ltoa();
static int			test_pair();
static int			bindings_free();
static int			chk_mtarights();
static int			chk_usrights();
static int			do_stage_1();
static int			do_stage_2();
static int			test_flags();
static int			test_recip_finish();
static int			test_recip_start();
static int			test_regex();
static void			auth_chan();
static void			auth_err2adr();
static void			auth_init();
static void			auth_init_recip();
static void			auth_mta();
static void			auth_remdup();
static void			auth_usr();
static void			auth_usr_chks();
static void			bindings_add();
static void			pr_rights();
static void			rem_excess_outchans();
static void			rem_pair();
static void			set_fmt_and_content();




/* ------------------------  Begin  Routines  ------------------------------- */




void auth_start(que, sender, recip)
Q_struct *que;
ADDR	*sender, *recip;
{
        ADDR   *ad;

        PP_TRACE (("auth_start()"));

        auth_init(que, recip);
        auth_usr(sender, recip);    

        for (ad = recip; ad; ad = ad -> ad_next)
                if (ad -> ad_status == AD_STAT_PEND)
                        test_recip_start (que, sender, ad);
}





int  auth_finish(qp, sender, recip)
Q_struct *qp;
ADDR	*sender, *recip;
{
        ADDR	*ad;
        int     retval = RP_OK;

        for (ad = recip; ad; ad = ad -> ad_next) {

		PP_TRACE (("auth_finish ('%s', '%d')",
			ad -> ad_value, ad -> ad_status));

		switch (ad -> ad_status) { 
		case AD_STAT_PEND:
                        if (test_recip_finish (qp, sender, ad) == NOTOK)
				retval = RP_NAUTH;
			continue;
		default:
			continue;
		}
	}	

        PP_TRACE (("auth_finish returns ('%s')", rp_valstr (retval)));
        return retval;
}




int fillin_orig_outchan (qp, ad) 
Q_struct *qp;
ADDR	*ad;
{
	Bindings	*binds, *ix;
	int		unknownBP;
	if ((binds = do_ch_binds (qp, ad, &unknownBP)) == (Bindings *) NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unable to reformat return of contents"));
		/* take first channel */
		rem_excess_outchans(ad, ad->ad_outchan);
		ad->ad_type = ad->ad_outchan->li_chan->ch_out_ad_type;
		return OK;
	}

	/* try for one without reformatting ? */
	/* should also check unknownBP */
	for (ix = binds;ix && ix -> fmtchans;ix = ix->next)
		continue;
	if (!ix) {
		/* revert to first choice */
		ix = binds;
#ifdef	NOTDEF
		if (unknownBP == TRUE) {
			/* oh oh unknown bodypart and reformatting */
			/* just have to forget about return of contents */
			list_rchan_free(ix -> fmtchans);
			ix -> fmtchans = NULLIST_RCHAN;
		}
#endif
	}

	set_fmt_and_content(qp, ad, ix);

	rem_excess_outchans (ad, ix -> outchan);
	ad -> ad_type = ix -> outchan -> li_chan -> ch_out_ad_type;
	bindings_free (binds);
	return OK;
}	




/* ------------------------  Static Routines  ------------------------------- */




static int bindings_free (ix)
Bindings	*ix;
{
	if (ix == NULL) return;
	if (ix -> next != (Bindings *) NULL) bindings_free (ix -> next);
	list_rchan_free (ix -> fmtchans);
	list_bpt_free (ix -> bpts);
	free ((char *) ix);
}



static Bindings	*bindings_new (out, fmts, bpts, cost)
LIST_RCHAN	*out,
		*fmts;
LIST_BPT	*bpts;
int		cost;
{
	Bindings	*ret = (Bindings *) malloc(sizeof(Bindings));
	ret -> outchan = out;
	ret -> fmtchans = fmts;
	ret -> bpts = bpts;
	ret -> cost = cost;
	ret -> next = (Bindings *) NULL;
	return ret;
}



static void bindings_add (plist, new)
Bindings	**plist,
		*new;
{
	Bindings	*ix;

	PP_TRACE (("bindings_add ('%s')",
		new -> outchan -> li_chan -> ch_name));

	if (*plist == (Bindings *) NULL
	    || new -> cost < (*plist) -> cost) {
		/* easy */
		new -> next = *plist;
		*plist = new;
		return;
	}
	
	ix = *plist;
	
	while (ix -> next != (Bindings *) NULL
	       && ix -> next -> cost <= new -> cost)
		ix = ix -> next;
	new -> next = ix -> next;
	ix -> next = new;
}
	


static Bindings	*do_ch_binds (qp, ad, punknown)
Q_struct *qp;
ADDR	*ad;
int	*punknown;
{
	Bindings	*ret = (Bindings *) NULL;
	LIST_RCHAN	*ix = ad -> ad_outchan, *fmts;
	LIST_BPT	*bpts;
	int		cost, unknown;

	*punknown = FALSE;
	
	while (ix != NULLIST_RCHAN) {
		PP_TRACE (("do_ch_binds ('%s')", ix -> li_chan -> ch_name));
		if (ch_bind (qp, ad, ix, &fmts, &bpts, &cost, &unknown) == OK) {
			bindings_add(&ret, bindings_new(ix, fmts, bpts, cost));
			if (unknown == TRUE)
				*punknown = TRUE;
		}
		ix = ix -> li_next;
	}
	return ret;
}




static int test_recip_finish (qp, orig, ad)
Q_struct *qp;
ADDR	*orig, *ad;
{
	Bindings	*binds, *ix;
	int		retval = OK, unknownBP;
	int		test = ad -> ad_outchan -> li_auth -> chan -> test; 

	PP_TRACE (("test_recip_finish : %s", gval(ad)));
	
	if ((binds = do_ch_binds (qp, ad, &unknownBP)) == (Bindings *) NULL) 
		return NOTOK;
	
	for (ix = binds; ix != (Bindings *) NULL; ix = ix -> next) {
		
		if (unknownBP == TRUE
		    && ix -> fmtchans)
			continue;

		switch (ix -> outchan -> li_auth -> status) {
		case AUTH_OK:
		case AUTH_DR_OK:
			ad -> ad_type = ix -> outchan -> li_chan -> ch_out_ad_type;
			retval = test_pair (qp, ad, ix -> outchan, orig);

			if (retval == NOTOK) {
				rem_pair (qp, ad, ix -> outchan);
				if (test != AUTH_CHAN_TEST) 
					continue;
			}

			/* --- otherwise fall through --- */

		case AUTH_FAILED_TEST:	/* --- failed at do_stage_1 --- */
			set_fmt_and_content (qp, ad, ix);
			rem_excess_outchans (ad, ix -> outchan);
			bindings_free (binds);
			return OK;

		default:
			continue;
		}
	}

	if (unknownBP == TRUE && ad -> ad_status != AD_STAT_DRREQUIRED) {
		/* failed because of unknown bodypart */
		ad -> ad_status = AD_STAT_DRREQUIRED;
		ad -> ad_reason = DRR_CONVERSION_NOT_PERFORMED;
		ad -> ad_diagnostic = DRD_CONVERSION_IMPRACTICAL;
		ad -> ad_add_info = strdup("Unrecognised bodypart in message");
	}
	bindings_free (binds);
	return NOTOK;
}




static void set_fmt_and_content (qp, ad, ix)
Q_struct *qp;
ADDR		*ad;
Bindings	*ix;
{
	char	*content_out = ix -> outchan -> li_chan -> ch_content_out;

	PP_TRACE (("submit/Give message to '%s'",
		ix -> outchan -> li_chan -> ch_name));

	ad -> ad_fmtchan = ix -> fmtchans;

	/* -- stop them being freed -- */
	ix -> fmtchans = NULLIST_RCHAN;
	ad -> ad_eit = ix -> bpts;

	/* -- stop them being freed -- */
	ix -> bpts = NULLIST_BPT;
	if (ad -> ad_content != NULLCP) { 
		free (ad -> ad_content);
		ad -> ad_content = NULLCP;
	}

	if (isstr (qp -> cont_type) && 
		isstr (content_out) &&
		strcmp (qp -> cont_type, content_out) != 0)
			ad -> ad_content = strdup (content_out);
	else if (isstr (content_out))
			ad -> ad_content = strdup (content_out);

	return;
}




static int test_recip_start (qp, orig, ad)
Q_struct *qp;
ADDR   *orig, *ad;
{
	LIST_RCHAN	*lp = ad -> ad_outchan;
	int		retval = FALSE;

	PP_TRACE (("test_recip_start : %s", gval(ad)));

	for (; lp != NULLIST_RCHAN; lp = lp -> li_next) {

		ad -> ad_type = lp -> li_chan -> ch_out_ad_type;

		switch (lp -> li_auth -> status) {
		case AUTH_OK:
		case AUTH_DR_OK:
			retval = test_pair (qp, ad, lp, orig);
			if (retval == OK)  return OK;
			rem_pair (qp, ad, lp);
			continue;
		default:
			continue;
		}
	}

	return retval;
}



static int  test_pair (qp, ad, lp, orig)  /* Test chan/mta/usr per recip */
Q_struct *qp;
ADDR   		*ad;
LIST_RCHAN 	*lp;
ADDR	*orig;
{
	char		*val = gval(ad);
	AUTH		*au = lp -> li_auth;
	int		retval;	

	PP_TRACE (("test_pair : '%s' '%s' '%s'", val,
		au -> chan -> li_chan -> ch_name, au -> mta -> li_mta));



	switch (au -> stage) {

	case AUTH_STAGE_1:
		if (au -> chan -> li_found == FALSE)
			auth_chan (au -> chan);
		retval = do_stage_1 (qp, ad, au, orig);
		return retval;


	case AUTH_STAGE_2: 
		retval = do_stage_2 (qp, val, au);
		return retval;
	}


	PP_LOG (LLOG_EXCEPTIONS, 
		("auth/Internal error unknown test stage %d", au -> stage));
	return NOTOK;
}


static void auth_user (au, ad)
AUTH	*au;
ADDR	*ad;
{
	if (au -> user -> found == TRUE)
		return;

	if (tb_getauthusr (au -> user, ad) == NOTOK)
		err_abrt (RP_NAUTH, "Cannot look up authorisation for '%s'",
			  gval(ad));
	if (au -> user -> found == FALSE)
		PP_TRACE (("auth_usr_chks/recipient '%s' missing",
			   gval (ad)));
}

static int do_stage_1 (qp, ad, au, orig)
Q_struct *qp;
ADDR	*ad;
AUTH	*au;
ADDR	*orig;
{

	int		t[4];
	int		i = 0;
	int		retval = NOTOK;
	char    	buf[LINESIZE];
	LIST_REGEX	*lr;


	PP_TRACE (("do_stage_1: %s", gval(ad)));

	switch (au -> chan -> policy) {

	default:
		PP_LOG (LLOG_EXCEPTIONS,
			("auth/do_stage_1 unknown channel policy %d",
			 au -> chan -> policy));
		break;

	case AUTH_CHAN_FREE:
		do_reason (au, freason(AUR_CH_FREE));
		retval = OK;
		break;


	case AUTH_CHAN_NONE:
		do_reason (au, freason(AUR_CH_NONE));
		break;


	case AUTH_CHAN_NEGATIVE:
		auth_mta (au);
		auth_mta (sender_auth);
		auth_user (sender_auth, orig);
		auth_user (au, ad);

		t [i++] = chk_mtarights (qp, sender_auth,
					 au -> chan -> li_chan); 
		t [i++] = chk_usrights (qp, sender_auth,
					au -> chan -> li_chan);
		t [i++] = chk_mtarights (qp, au, au -> chan -> li_chan);
		t [i++] = chk_usrights (qp, au, au -> chan -> li_chan);

		/* with negative, OK means one of the values is true,
		 *  which in turn means there is an explicit block on this
		 *   which means its failed authorisation */
		if (test_flags (t, NOTOK, ORED) != OK)
			retval = OK;
		pr_rights (au, AUTH_CHAN_NEGATIVE, retval);
		break;


	case AUTH_CHAN_BLOCK:
		auth_mta (au);
		auth_mta (sender_auth);
		auth_user (sender_auth, orig);
		auth_user (au, ad);
		t [i++] = chk_mtarights (qp, sender_auth,
					 au -> chan -> li_chan);
		t [i++] = chk_usrights (qp, sender_auth,
					au -> chan -> li_chan);
		t [i++] = chk_mtarights (qp, au, au -> chan -> li_chan);
		t [i++] = chk_usrights (qp, au, au -> chan -> li_chan);
		retval = test_flags (t, OK, ORED);
		pr_rights (au, AUTH_CHAN_BLOCK, retval);
		break;
	}



	if (retval == NOTOK)
		return NOTOK;


	/* -- regex testing -- */

	bzero (buf, LINESIZE); 

	lr = sender_auth -> mta -> requires;
	if (lr && (!test_regex (gval (orig), lr, buf))) {
		do_reason (au, freason(AUR_IMTA_MISSING_SNDR), buf);
		return NOTOK;	
	}

	lr = sender_auth -> mta -> excludes;
	if (lr && (test_regex (gval (orig), lr, buf))) {
		do_reason (au, freason(AUR_IMTA_EXCLUDES_SNDR), buf);
		return NOTOK;
	}

	lr = au -> mta -> requires; 
	if (lr && (!test_regex (gval(ad), lr, buf))) {
		do_reason (au, freason(AUR_OMTA_MISSING_RECIP), buf);
		return NOTOK;
	}

	lr = au -> mta -> excludes;
	if (lr && (test_regex (gval(ad), lr, buf))) {
		do_reason (au, freason(AUR_OMTA_EXCLUDES_RECIP), buf);
		return NOTOK;
	}

	au -> stage = AUTH_STAGE_2;
	return OK;
}




static int do_stage_2 (qp, adval, au)
Q_struct *qp;
char	*adval;
AUTH	*au;
{
	LIST_BPT	*le, *lb;
	long		size;


	PP_TRACE (("do_stage_2: %s", adval));

	/* -- content tests -- */

	size = au -> chan -> sizelimit;
	if (size && (qp -> msgsize > size)) {
		do_reason (au, freason(AUR_MSGSIZE_FOR_CHAN),
			ltoa(qp -> msgsize), ltoa(size));
		return NOTOK;
	}

	size = au -> mta -> sizelimit;
	if (size && (qp -> msgsize > size)) {
		do_reason (au, freason(AUR_MSGSIZE_FOR_MTA),
			ltoa(qp -> msgsize), ltoa(size));
		return NOTOK;
	}

	size = au -> user -> sizelimit;
	if (size && (qp -> msgsize > size)) {
		do_reason (au, freason(AUR_MSGSIZE_FOR_USR),
			ltoa(qp -> msgsize), ltoa(size));
		return NOTOK;
	}


	/* -- body part tests -- */


	for (le = au -> mta -> content_excludes; le; le = le -> li_next) {
		for (lb = qp->encodedinfo.eit_types; lb; lb = lb -> li_next) {

			PP_TRACE (("do_stage_2 exclude mta body parts %s : %s",
				le -> li_name, lb -> li_name));

			if (strcmp (le -> li_name, lb -> li_name) ==  0) {
				do_reason (au, freason(AUR_MTA_BPT),
						le -> li_name);
				return NOTOK;
			}
		}
	}


	for (le = au -> user -> content_excludes; le; le = le -> li_next) {
		for (lb = qp->encodedinfo.eit_types; lb; lb=lb -> li_next) {

			PP_TRACE (("do_stage_2 exclude user body parts %s : %s",
				le -> li_name, lb -> li_name));

			if (strcmp (le -> li_name, lb -> li_name) ==  0) {
				do_reason (au, freason(AUR_USR_BPT),
						le -> li_name);
				return NOTOK;
			}
		}
	}


	au -> stage ++;
	PP_TRACE (("do_stage_2:  successful & returns '%s'", au -> reason));
	return OK;
}




static int test_regex (val, list, ptr)
char		*val;
LIST_REGEX	*list;
char		*ptr;
{
	LIST_REGEX	*lr;
	char		*cp;
	int		test;

	PP_TRACE (("test_regex: %s", val));

	for (lr = list; lr != NULLIST_REGEX; lr = lr -> li_next) {

		PP_TRACE (("auth/test_regex: '%s'", lr -> li_regex));

		cp = re_comp (lr -> li_regex);

		if (cp) {
			PP_LOG (LLOG_EXCEPTIONS, 
			("auth/test_regex: invalid expression <%s> %s",
			lr -> li_regex, cp));
			continue;
		}

		test = re_exec (val);

		if (test == 1) {
			(void) sprintf (ptr, "%s", lr -> li_regex);
			PP_TRACE (("auth/test_regex : %s matches %s",
				val, lr -> li_regex));
			return TRUE;
		}


		if (test == -1) {
			PP_LOG (LLOG_EXCEPTIONS, 
			("auth/test_regex : internal error <%s>",
			lr -> li_regex));
			continue;
		}


		if (strlen (ptr)) {
			(void) strcat (ptr, " | ");
			(void) strcat (ptr, lr -> li_regex);
		}
		else 
			(void) sprintf (ptr, "%s", lr -> li_regex);

		PP_TRACE (("auth/test_regex: no match: %s : %s", 
			lr -> li_regex, ptr));

		continue;
	}

	return FALSE;
}




static int chk_mtarights (qp, au, ch)  
Q_struct *qp;
AUTH	*au;
CHAN	*ch;
{
	int			infound = FALSE,
				outfound = FALSE;
	LIST_CHAN_RIGHTS	*lr;


	PP_TRACE (("chk_mtarights()"));

	if (au -> mta -> li_found == FALSE)  return MAYBE;

	if (au -> mta -> rights != AUTH_RIGHTS_UNSET) {
		au -> mta_inrights  = au -> mta -> rights; 
		au -> mta_outrights = au -> mta -> rights;
	}


	for (lr = au -> mta -> li_cr; lr; lr = lr -> li_next) {
		if (lr -> li_chan == qp -> inbound -> li_chan) {
			infound = TRUE;
			au -> mta_inrights = lr -> li_rights;
		}

		if (lr -> li_chan == ch) {
			outfound = TRUE;
			au -> mta_outrights = lr -> li_rights;
		}

		if (infound && outfound)
			break;
	}


	/* -- returns OK if (inbound = in|both && outbound = out|both -- */

	if (au -> mta_inrights == AUTH_RIGHTS_NONE || 
	    au -> mta_inrights == AUTH_RIGHTS_OUT || 
	    au -> mta_inrights == AUTH_RIGHTS_UNSET) 
		return NOTOK;	

	if (au -> mta_outrights == AUTH_RIGHTS_NONE ||
	    au -> mta_outrights == AUTH_RIGHTS_IN ||
	    au -> mta_outrights == AUTH_RIGHTS_UNSET) 
		return NOTOK;

	PP_TRACE (("chk_mtarights : successful: in = %s  out = %s", 
		   rrights (au -> mta_inrights), 
		   rrights (au -> mta_outrights)));

	return OK;
}

static int chk_usrights (qp, au, ch)
Q_struct *qp;
AUTH	*au;
CHAN	*ch;
{
	int			infound = FALSE, 
				outfound = FALSE; 
	LIST_CHAN_RIGHTS	*lr;


	PP_TRACE (("chk_usrights()"));

	if (au -> user -> found == FALSE)
		return MAYBE;

	if (au -> user -> rights != AUTH_RIGHTS_UNSET) {
		au -> user_inrights = au -> user -> rights;
		au -> user_outrights = au -> user -> rights;
	}

	for (lr = au -> user -> li_cr; lr; lr = lr -> li_next) {
		if (lr -> li_chan == qp -> inbound -> li_chan) {
			infound = TRUE;
			au -> user_inrights = lr -> li_rights;
		}

		if (lr -> li_chan == ch) {
			outfound = TRUE;
			au -> user_outrights = lr -> li_rights;
		}

		if (infound && outfound)
			break;
	}


	/* -- returns OK if (inbound = in|both && outbound = out|both -- */

	if (au -> user_inrights == AUTH_RIGHTS_NONE || 
		au -> user_inrights == AUTH_RIGHTS_OUT || 
		au -> user_inrights == AUTH_RIGHTS_UNSET) 
			return NOTOK;

	if (au -> user_outrights == AUTH_RIGHTS_NONE || 
		au -> user_outrights == AUTH_RIGHTS_IN || 
		au -> user_outrights == AUTH_RIGHTS_UNSET)
			return NOTOK;

	PP_TRACE (("chk_usrights : successful : in = %s  out = %s", 
		rrights (au -> user_inrights), 
		rrights (au -> user_outrights)));

	return OK;
}

static void rem_excess_outchans (ad, outchan)
ADDR	*ad;
LIST_RCHAN	*outchan;
{
	LIST_RCHAN	*ix, *tmp;

	/* remove excess ad_outchans */
	
	ix = ad -> ad_outchan;

	if (ix != outchan) {
		while (ix -> li_next != NULLIST_RCHAN
		       && ix -> li_next != outchan)
			ix = ix -> li_next;
		if (ix -> li_next != NULLIST_RCHAN) {
			tmp = ad -> ad_outchan;
			ad -> ad_outchan = ix -> li_next;
			ix -> li_next = NULLIST_RCHAN;
			list_rchan_free (tmp);
		}	
	}
	
	if (ad -> ad_outchan -> li_next != NULLIST_RCHAN) {
		list_rchan_free (ad -> ad_outchan -> li_next);
		ad -> ad_outchan -> li_next = NULLIST_RCHAN;
	}
}




static void rem_pair (qp, ad, remch)
Q_struct	*qp;
ADDR		*ad;
LIST_RCHAN	*remch;
{
	LIST_RCHAN	*lp;
	AUTH		*au = remch -> li_auth;
	int		allgone = TRUE;


	if (remch == NULLIST_RCHAN)  return;


	PP_TRACE (("rem_pair : '%s' '%s' '%s' '%d'", 
		gval (ad), remch -> li_chan -> ch_name,
		remch -> li_mta, au -> chan -> test));


	switch (au -> chan -> test) { 
	case AUTH_CHAN_TEST:  
		au -> status = AUTH_FAILED_TEST;
		return;
	default:
		au -> status = AUTH_FAILED;
		break;
	}


	for (lp = ad -> ad_outchan; lp; lp = lp -> li_next) {
		switch (lp -> li_auth -> status) {
		    case AUTH_OK:
		    case AUTH_DR_OK:
		    case AUTH_FAILED_TEST:
			allgone = FALSE;
			break;

		    case AUTH_DR_FAILED:
		    case AUTH_FAILED:
			continue;
		}
		break;
	}

	if (allgone)
		auth_err2adr (qp, ad);
}




static void auth_err2adr (qp, ad)
Q_struct	*qp;
ADDR   		*ad;
{
	char    buf [BUFSIZ * 2];
	int	oldType;
	if (ad == NULLADDR)				return;

	ad -> ad_status	    = AD_STAT_DRREQUIRED;
	ad -> ad_reason	    = DRR_UNABLE_TO_TRANSFER;
	ad -> ad_diagnostic = DRD_NO_BILATERAL_AGREEMENT;

	oldType = ad->ad_type;
	if (qp && qp -> inbound && qp -> inbound -> li_chan)
		ad->ad_type = qp -> inbound -> li_chan -> ch_in_ad_type;
	(void) sprintf (buf, 
	      "Authorisation failure at site '%s' for recip '%s' Reason: '%s'",
	      loc_dom_site, gval(ad), auth2submit_msg);
	ad->ad_type = oldType;

	ad -> ad_add_info   = strdup (buf);

	PP_TRACE (("auth_err2adr: '%s'", buf));
}




static void auth_chan (ch)
LIST_AUTH_CHAN	*ch;
{
	if (tb_getauthchan (ch) == NOTOK)
		err_abrt (RP_NAUTH,
			  "Cannot look up channel authorisation table '%s'",
			  ch -> li_chan -> ch_name);

	if (ch -> li_found == FALSE)
		PP_TRACE (("auth_chan/Chan '%s' missing",
			   ch -> li_chan -> ch_name));
}




static void auth_mta (au)
AUTH	*au;
{
	if (au -> mta -> li_found == TRUE)
		return;
	if (tb_getauthmta (au -> mta) == NOTOK)
		err_abrt (RP_NAUTH,
			  "Cannot look up mta authorisation table '%s'",
			  au -> mta -> li_mta);

	if (au -> mta -> li_found == FALSE)
		PP_TRACE (("auth_mta/Mta '%s' missing",
			   au -> mta -> li_mta));
}




static void auth_usr(orig, recip)
ADDR *orig, *recip;
{
	ADDR   *ad;
	int ac;

	PP_TRACE (("auth_usr()"));

	if (sender_auth -> user == NULL_AUTHUSR) 
		err_abrt (RP_NAUTH, "No sender user authorisation table");

	if (orig == NULLADDR) 
		err_abrt (RP_NAUTH, "No sender address found");

#ifdef notdef
	if (tb_getauthusr (sender_auth -> user, orig) == NOTOK)
		err_abrt (RP_NAUTH,
			  "Authorisation lookup failed for sender '%s'", 
			  gval (orig));

	if (sender_auth -> user -> found == FALSE) 
		PP_TRACE (("auth_usr/sender user '%s' missing",
			   gval (orig)));
#endif

	for (ad = recip, ac = 0; ad; ac ++,ad = ad -> ad_next) 
		auth_usr_chks (ad);


	if (ac > 1000)
		return;
	/* -- eliminate duplicates -- */

	for (ad = recip; ad; ad = ad -> ad_next) {

		PP_TRACE (("auth_usr/dup loop '%s' stat='%d', resp='%d')",
			ad -> ad_value, ad -> ad_status, ad -> ad_resp));

		if (ad -> ad_resp == FALSE)
			continue;

		switch (ad -> ad_status) { 
		case AD_STAT_DRREQUIRED:
		case AD_STAT_DONE:
			continue;
		}

		auth_remdup (ad, ad -> ad_next);
	}
}




static void auth_usr_chks (ad)
ADDR	*ad;
{
	char	*value = ad -> ad_value;

	PP_TRACE (("auth_usr_chks (%s)", gval (ad)));

	if (ad -> ad_status == AD_STAT_DRREQUIRED || 
		ad -> ad_status == AD_STAT_DRWRITTEN ||
		ad -> ad_resp == FALSE) {
			PP_TRACE (("auth_usr/recip='%s' stat=%d resp=%d",
				value, ad -> ad_status, ad -> ad_resp));
			return;
	}

	if (ad -> ad_outchan == NULL)
		err_abrt (RP_NAUTH, "ad_outchan unset for '%s'",
			value);

	if (ad -> ad_outchan -> li_auth == NULL_AUTH)
		err_abrt (RP_NAUTH, "chan auth unset for '%s'",
			value);
#ifdef notdef
	if (tb_getauthusr (ad -> ad_outchan -> li_auth -> user, ad) == NOTOK)
		err_abrt (RP_NAUTH, "Cannot look up authorisation for '%s'",
			value);

	if (ad -> ad_outchan -> li_auth -> user -> found == FALSE)
		PP_TRACE (("auth_usr_chks/recipient '%s' missing", gval (ad)));
#endif
}




static void auth_remdup (ad, list)  /* -- removes duplicates -- */
ADDR	*ad;
ADDR	*list;
{
	AUTH	*au;

	PP_TRACE (("auth_remdup (%s)", gval (ad)));

	for (list = ad -> ad_next; list; list = list -> ad_next) {

		PP_TRACE (("auth_remdup/'%s' stat='%d', resp='%d')",
			list -> ad_value, list -> ad_status, list -> ad_resp));

		if (list -> ad_resp == FALSE)
			continue;

		switch (list -> ad_status) { 
		case AD_STAT_DRREQUIRED:
		case AD_STAT_DONE:
			continue;
		}


		if (strcmp (gval(ad), gval(list)) == 0) {

			PP_TRACE (("auth_remdup/removed: '%s' (%d)",
				gval (list), list -> ad_no));

			list -> ad_status = AD_STAT_DONE;
			list -> ad_outchan -> li_next = NULLIST_RCHAN;

			au = list -> ad_outchan -> li_auth;
			au -> status = AUTH_OK;
			do_reason (au, freason (AUR_DUPLICATE_REMOVED)); 
		}
	}
}




static void auth_init(qp, recip)
Q_struct *qp;
ADDR *recip;
{
	char   			*argv[MAX_AUTH_ARGS], 
				*cp;
	int			argc;
	ADDR   			*ad;


	PP_TRACE (("auth_init: authloglevel='%s' authchannel='%s'",
		authloglevel, authchannel));

	bzero (auth2submit_msg, BUFSIZ);


	switch (auth_loglev = cmd_srch (authloglevel, authtbl_loglevel)) {
	case AUTH_LOGLEVEL_LOW:
	case AUTH_LOGLEVEL_MEDIUM:
	case AUTH_LOGLEVEL_HIGH:
		break;
	default:
		auth_loglev = AUTH_LOGLEVEL_LOW;
		PP_LOG (LLOG_EXCEPTIONS,
			("auth_init/authloglevel invalid : '%s' (low assumed)",
			 authloglevel));
		break;
	}


	cp = strdup (authchannel);

	if ((argc = sstr2arg (cp, MAX_AUTH_ARGS, argv, " \t,")) == NOTOK)
		err_abrt (RP_NAUTH, "auth_init/authchannel invalid '%s'",
			authchannel);

	def_authchan = authchan_new (NULLCHAN, NULLIST_AUTHCHAN);
	tb_parse_authchan (def_authchan, argc, argv);

	sender_auth 		= auth_new(qp -> msgtype == MT_DMPDU);

	authmta_start		= authmta_new (qp -> inbound -> li_mta);
	sender_auth -> mta	= authmta_start;

#ifdef notdef
	auth_mta (sender_auth -> mta);
#endif

	sender_auth -> user	= authusr_new();

	for (ad = recip; ad; ad = ad -> ad_next)
		auth_init_recip (qp, ad);

	free (cp);
}




static void auth_init_recip (qp, ad)
Q_struct *qp;
ADDR	*ad;
{
	AUTH_USER 		*up;
	LIST_RCHAN 		*lr;
	LIST_AUTH_CHAN		*lc;
	LIST_AUTH_MTA 		*lm;
	int			found = FALSE;
	char			*val;


	PP_TRACE (("auth_init_recip (%s)", gval (ad)));

	up = authusr_new();

	for (lr = ad -> ad_outchan; lr; lr = lr -> li_next) {

		lr -> li_auth 		= auth_new(qp -> msgtype == MT_DMPDU);
		lr -> li_auth -> user 	= up;
		val 			= lr -> li_chan -> ch_name;
		found			= FALSE;

		for (lc = authchan_start; lc; lc = lc -> li_next)
			if (strcmp (val, lc -> li_chan -> ch_name) == 0) {
				found = TRUE;
				break;
			}

		if (!found && lr -> li_chan) { 
			lc = authchan_new (lr -> li_chan, def_authchan);
			authchan_add (&authchan_start, lc);
		}

		lr -> li_auth -> chan 	= lc;
		found 			= FALSE;

		for (lm = authmta_start; lm; lm = lm -> li_next)
			if (strcmp (lm -> li_mta, lr -> li_mta) == 0) {
				found = TRUE;
				break;
			}

		if (!found && lr -> li_mta) {
			lm = authmta_new (lr -> li_mta);
			authmta_add (&authmta_start, lm);
		}

		lr -> li_auth -> mta	= lm;
	}
}




static void pr_rights (au, ch_policy, val)
AUTH	*au;
int	ch_policy, val;
{
	char	buf[BUFSIZ*2];
	char	*msg, *policy;

	PP_TRACE (("pr_rights ('%d')", ch_policy));

	switch (ch_policy) {
	    case AUTH_CHAN_BLOCK:
		policy = "block";
		break;
	    case AUTH_CHAN_NEGATIVE:
		policy = "negative";
		break;
	    default:
		policy = "Unknown";
		break;
	}
	if (val == OK)
		msg = "Authorisation approved";
	else 
		switch (ch_policy) {
		    case AUTH_CHAN_BLOCK:
			msg = "You are not authorised for this route";
			break;
		    case AUTH_CHAN_NEGATIVE:
			msg = "You are prohibited from using this route";
			break;
		    default:
			msg = "UnknownPolicy";
			break;
		}
	(void) sprintf (buf,
"%s: (policy %s, imta (%s %s) sender (%s %s) omta (%s %s) recip (%s %s))",
			msg, policy,
			rrights (sender_auth -> mta_inrights),
			rrights (sender_auth -> mta_outrights),
			rrights (sender_auth -> user_inrights),
			rrights (sender_auth -> user_outrights),
			rrights (au -> mta_inrights),
			rrights (au -> mta_outrights),
			rrights (au -> user_inrights),
			rrights (au -> user_outrights));

	do_reason (au, "%s", buf); 
	return;
}




static char *ltoa (l)
long    l;
{
        char    buf[LINESIZE];

        (void) sprintf (buf, "%ld", l);
        return (strdup (buf));
}




static int test_flags (f, val, type)
int	f[];
int	val;
int	type;
{
	switch (type) {
	case ORED:
		if (f[0] == val || f[1] == val || f[2] == val || f[3] == val)
			return OK;
		break;
	}

	return NOTOK;
}	
