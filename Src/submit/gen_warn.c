/* gen_warn.c: generate warning messages */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_warn.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_warn.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: gen_warn.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "q.h"
#include "auth.h"
#include "prm.h"
#include "retcode.h"
#include "expand.h"

extern char *getpostmaster(), *postmaster;
extern char *loc_dom_mta;
extern char *local_822_chan, *loc_dom_site;
extern char *msg_unique;
extern char *ia5_bp, *hdr_822_bp;
extern char *wrndfldir;

extern void getfpath (), ad_log_print(), 
	auth_start(), auth_log(), time_mgt();

static void gen_auth_warn (), setup_macros ();
static int copy_warn_file();

void gen_warnings (qp, sender, recips)
Q_struct *qp;
ADDR *sender;
ADDR *recips;
{
	ADDR	*ad;

	for (ad = recips; ad; ad = ad -> ad_next)
		gen_auth_warn (qp, sender, ad);
}

#define gval(x)			((x -> ad_type) == AD_X400_TYPE ? \
				 (x -> ad_r400adr) : (x -> ad_r822adr))	


static void gen_auth_warn (qp, sender, ad)
Q_struct *qp;
ADDR	*sender, *ad;
{
	LIST_RCHAN 	*lp = ad -> ad_outchan;

	if (lp == NULL || lp -> li_auth == NULL)
		return;

	switch (lp -> li_auth -> status) {
	    case AUTH_OK:
	    case AUTH_DR_OK:
		return;
	    case AUTH_FAILED_TEST:
	    case AUTH_DR_FAILED:
	    case AUTH_FAILED:
		if (lp -> li_auth -> chan -> warnsender) {
			send_warning (qp, sender, ad,
				      lp -> li_auth -> chan -> warnsender,
				      "sender", lp -> li_auth -> reason);
			PP_OPER (NULLCP, ("%s '%s' from file '%s'",
					  "Warning message to sender",
					  gval (sender), 
					  lp -> li_auth -> chan -> warnsender));
		}
		if (lp -> li_auth -> chan -> warnrecipient) {
			send_warning (qp, ad, ad,
				      lp -> li_auth -> chan -> warnrecipient,
				      "recip", lp -> li_auth -> reason);
			PP_OPER (NULLCP, ("%s '%s' from file '%s'",
					  "Warning message to recipient",
					  gval (ad), 
					  lp -> li_auth -> chan -> warnsender));
		}
	}

}

send_warning (qporig, recip, ad, file, direction, reason)
Q_struct *qporig;
ADDR	*recip;
ADDR	*ad;
char	*file;
char	*direction;
char	*reason;
{
	struct prm_vars prm;
	Q_struct que;
	ADDR	*ap;
	ADDR	*postie;
	RP_Buf rps, *rp = &rps;
	char buf[BUFSIZ];
	char datebuf[LINESIZE];
	UTC	ut;

	prm_init (&prm);
	q_init (&que);
	que.inbound = list_rchan_new (loc_dom_mta, local_822_chan);
	list_bpt_add (&que.encodedinfo.eit_types, list_bpt_new(hdr_822_bp));
	list_bpt_add (&que.encodedinfo.eit_types, list_bpt_new (ia5_bp));
	ap = adr_new (recip -> ad_r822adr, AD_822_TYPE, 1);
	postie = adr_new (getpostmaster(AD_822_TYPE), AD_822_TYPE, 0);
	postie -> ad_resp = FALSE;
	postie -> ad_status = AD_STAT_DONE;
	postie -> ad_no = NULL;

	if (rp_isbad (cntrl_mgt (&prm, &que)) ||
	    rp_isbad (q_mgt (&que))) {
		PP_LOG (LLOG_EXCEPTIONS, ("bad paramters"));
		goto out;
	}
	if (rp_isbad (validate_sender (&que, postie, NULLCP, rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("bad sender %s, %s",
					  getpostmaster(AD_822_TYPE), rp -> rp_line));
		goto out;
	}
	if (rp_isbad (validate_recip (ap, &que, rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("bad recip %s, %s",
					  ap -> ad_value, rp -> rp_line));
		goto out;
	}

	ad_log_print (ap);
	auth_start (&que, postie, ap);
	que.Oaddress = postie;
	que.Raddress = ap;

	time_mgt(&que);

	if (rp_isbad (winit_q (rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("Problem writing into the que: %s",
					  rp -> rp_line));
		goto out;
	}

	if (rp_isbad (txt_init (&que)) ||
	    rp_isbad (txt_psetup (&que, hdr_822_bp, NULLCP))) {
		PP_LOG (LLOG_EXCEPTIONS, ("Problem initialising text"));
		goto out;
	}
	if (UTC2rfc (ut = utcnow(), datebuf) == NOTOK)
		goto out;
	free ((char *)ut);
	(void) sprintf (buf, "To: %s\nFrom: %s\nDate: %s\n\
Subject: Warning message\n",
			recip -> ad_r822adr, postmaster, datebuf);
	if (rp_isbad (txt_write (buf, strlen (buf)))) {
		PP_LOG (LLOG_EXCEPTIONS, ("Problems writing text"));
		goto out;
	}
	if (rp_isbad (txt_pfinish ())) {
		PP_LOG (LLOG_EXCEPTIONS, ("Problems finishing hdr"));
		goto out;
	}

	(void) sprintf (datebuf, "1.%s", ia5_bp);
	if (rp_isbad (txt_psetup (&que, datebuf, NULLCP))) {
		PP_LOG (LLOG_EXCEPTIONS, ("Problems initialising for text"));
		goto out;
	}
	setup_macros (&que, qporig, ad, direction, reason);

	if (copy_warn_file (file) == NOTOK)
		goto out;
	if (rp_isbad (txt_pfinish ()) ||
	    rp_isbad (txt_tend())) {
		PP_LOG (LLOG_EXCEPTIONS, ("Problems finishing text"));
		goto out;
	}

	if (rp_isbad (auth_finish (&que, postie, ap))) {
		PP_LOG (LLOG_EXCEPTIONS, ("Authorisation problems"));
		goto out;
	}

	setup_directories (&que);
	gen_ndr (&que);
	if (rp_isbad (write_q (&que, &prm, rp)) ||
	    rp_isbad (move_q (rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Queue problems: %s", rp -> rp_line));
		goto out;
	}

	auth_log (&que, postie, ap, msg_unique);
	tell_qmgr (msg_unique, &prm, &que, postie, ap, 1);
out:;
	que.Oaddress = que.Raddress = NULL;
	if (ap)
		adr_free (ap);
	if (postie)
		adr_free (postie);
	q_free (&que);
	prm_free (&prm);
	return;
}


static Expand expand_macros[] = {
        "sender",       NULLCP, /* 0 */
        "822sender",    NULLCP, /* 1 */
        "400sender",    NULLCP, /* 2 */
        "ua-id",        NULLCP, /* 3 */
        "p1-id",        NULLCP, /* 4 */
        "recips",       NULLCP, /* 5 */
        "822recips",    NULLCP, /* 6 */
        "400recips",    NULLCP, /* 7 */
        "locmta",       NULLCP, /* 8 */
        "locsite",	NULLCP, /* 9 */
	"reason",	NULLCP, /* 10 */
	"direction",	NULLCP, /* 11 */
        NULLCP, NULLCP};

static void setup_macros (qp, qporig, recip, direction, reason)
Q_struct *qp;
Q_struct *qporig;
ADDR	*recip;
char *direction;
char *reason;
{
	expand_macros[0].expansion = qporig -> Oaddress -> ad_value;
	expand_macros[1].expansion = qporig -> Oaddress -> ad_r822adr;
	expand_macros[2].expansion = qporig -> Oaddress -> ad_r400adr;
	expand_macros[3].expansion = qporig -> ua_id;
	expand_macros[4].expansion = qporig -> msgid.mpduid_string;
	expand_macros[5].expansion = recip -> ad_value;
	expand_macros[6].expansion = recip -> ad_r822adr;
	expand_macros[7].expansion = recip -> ad_r400adr;
	expand_macros[8].expansion = loc_dom_mta;
	expand_macros[9].expansion = loc_dom_site;
	expand_macros[10].expansion = reason;
	expand_macros[11].expansion = direction;
}

static int copy_warn_file (file)
char *file;
{
	char filename[MAXPATHLENGTH];
	char buf[BUFSIZ], *cp;
	FILE	*fp;
	int retval = OK;

	getfpath (wrndfldir, file, filename);

	if ((fp = fopen (filename, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, filename, ("Can't open"));
		return NOTOK;
	}

	while (fgets (buf, sizeof buf, fp) != NULLCP) {
		if (index (buf, '$')) {
			cp = expand_dyn (buf, expand_macros);
			if (rp_isbad(txt_write (cp, strlen(cp)))) {
				PP_LOG (LLOG_EXCEPTIONS, ("write error"));
				retval = NOTOK;
				break;
			}
			free (cp);
		}
		else {
			if (rp_isbad(txt_write (buf, strlen(buf)))) {
				PP_LOG (LLOG_EXCEPTIONS, ("write error"));
				retval = NOTOK;
				break;
			}
		}
	}
	(void) fclose (fp);
	return retval;
}
