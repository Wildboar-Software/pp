/* auth_logs.c: Prints out the authorisation logging info */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/auth_logs.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/auth_logs.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: auth_logs.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "q.h"
#include "dr.h"
#include <isode/cmd_srch.h>


#define gval(x)                 ((x -> ad_type) == AD_X400_TYPE ? \
                                        (x -> ad_r400adr) : (x -> ad_r822adr))

/* -- externals -- */
extern CMD_TABLE	authtbl_statmsgs[];
extern int		auth_loglev;
extern UTC		utclocalise ();

/* -- local routines -- */
void			auth_log();
static int		auth_isdr();
static int		auth_isgood();
static void		gen_auth_logs();
static void		gen_auth_notok();
static void		gen_auth_ok();
static void		gen_auth_stats();
static void		wr2statfile();
static void		wrdr2statfile();
static void		wrmsg2statfile();
static void		logfailure ();



/* -------------------  Begin  Routines  ------------------------------------ */




void auth_log(qp, sender, recip, msgname)
Q_struct *qp;
ADDR	*sender;
ADDR	*recip;
char *msgname;
{
        ADDR    *ad;

        PP_TRACE (("auth_log()"));

        for (ad = recip; ad; ad = ad->ad_next)
                gen_auth_logs (qp, sender, ad, msgname);
}




/* ----------------------------- Logging ------------------------------------ */





static void gen_auth_logs (qp, sender, ad, msgname)
Q_struct *qp;
ADDR   *sender, *ad;
char *msgname;
{
	LIST_RCHAN	*lp;
	int		retval = NOTOK;

	PP_TRACE (("gen_auth_logs(%s)", gval(ad)));

	for (lp = ad -> ad_outchan; lp != NULLIST_RCHAN; lp = lp -> li_next)
		if ((retval = auth_isgood (lp)) == OK)
			break; 

	gen_auth_stats (qp, sender, ad, retval, msgname);
}




static void gen_auth_stats (qp, sender, ad, result, msgname)
Q_struct *qp;
ADDR	*sender, *ad;
int     result;
char *msgname;
{

	PP_TRACE (("gen_auth_stats '%d' '%s'", result, gval (ad)));

	switch (result) {
	case OK: 
		gen_auth_ok (qp, sender, ad, msgname);
		break;
	default:
		gen_auth_notok (sender, ad);
		break;
	}
}




static void gen_auth_ok (qp, sender, ad, msgname)
Q_struct *qp;
ADDR	*sender;
ADDR	*ad;
char *msgname;
{
	LIST_RCHAN	*lp = ad -> ad_outchan;

	PP_TRACE (("gen_auth_ok '%s'", gval (ad)));

	wr2statfile (qp, sender, ad, lp, msgname);
}

static void gen_auth_notok (sender, ad)
ADDR	*sender;
ADDR	*ad;
{
	LIST_RCHAN	*lp = ad -> ad_outchan;

	PP_TRACE (("gen_auth_notok '%s'", gval (ad)));

	switch (auth_loglev) {
	default:
	case AUTH_LOGLEVEL_LOW:
		if (lp != NULLIST_RCHAN)
			logfailure (sender, ad, lp);
		return;

	case AUTH_LOGLEVEL_MEDIUM:
	case AUTH_LOGLEVEL_HIGH:
		for (; lp != NULLIST_RCHAN; lp = lp -> li_next)
			logfailure (sender, ad, lp);
		return;
	}
}

static int auth_isgood (lp)
LIST_RCHAN	*lp;
{
	if (lp == NULLIST_RCHAN)
		return FALSE; 

	switch (lp -> li_auth -> status) {
	case AUTH_OK:
	case AUTH_DR_OK:
	case AUTH_FAILED_TEST: 
		return OK;
	default:
		return NOTOK;
	}
}

static int auth_isdr (lp)
LIST_RCHAN	*lp;
{
	if (lp == NULLIST_RCHAN)
		return FALSE; 

	switch (lp -> li_auth -> status) {
	case AUTH_DR_OK:
	case AUTH_DR_FAILED:
		return OK;
	}
	return NOTOK;
}

static void wr2statfile (qp, sender, ad, lp, msgname)
Q_struct *qp;
ADDR	*sender;
ADDR	*ad;
LIST_RCHAN	*lp;
char *msgname;
{
	int	oldType;

	oldType = sender->ad_type;
	if (qp && qp -> inbound && qp -> inbound -> li_chan)
		sender->ad_type = qp -> inbound -> li_chan -> ch_in_ad_type;

	if (auth_isdr (lp) == OK)
		wrdr2statfile (qp, sender, ad, lp, msgname);
	else 
		wrmsg2statfile (qp, sender, ad, lp, msgname);
	sender->ad_type=oldType;
}

static void wrmsg2statfile (qp, sender, ad, lp, msgname)
Q_struct *qp;
ADDR		*sender;
ADDR		*ad;
LIST_RCHAN	*lp;
char *msgname;
{
	AUTH	*au = lp -> li_auth;
	char	*cp;
	char 	*argv[100];
	int	argc;
	char	midbuf[BUFSIZ];
	char	buffer[BUFSIZ];
	char	tbuf[BUFSIZ];
	char	sizebuf[30];
	static char eq[] = "=";
	PP_TRACE (("submit/wrmsg2statfile ('%s')", au -> reason));


	cp = rcmd_srch (au -> status, authtbl_statmsgs);

	if (cp == NULLCP)
		cp = rcmd_srch (AUTH_FAILED, authtbl_statmsgs);

	argc = 0;
	argv[argc++] = cp;
	argv[argc++] = msgname;

	argv[argc++] = qp -> inbound -> li_chan -> ch_name;
	argv[argc++] = "->";
	argv[argc++] = au -> chan -> li_chan -> ch_name;

	argv[argc++] = eq;
	argv[argc++] = "p1msgid";
	(void) msgid2rfc_aux (&qp->msgid, midbuf, FALSE);
	argv[argc++] = midbuf;

	argv[argc++] = eq;
	argv[argc++] = "size";
	(void) sprintf (sizebuf, "%d", qp -> msgsize);
	argv[argc++] = sizebuf;

	argv[argc++] = eq;
	argv[argc++] = "sender";
	argv[argc++] = gval (sender);
	argv[argc++] = qp -> inbound -> li_mta;

	argv[argc++] = eq;
	argv[argc++] = "recip";
	argv[argc++] = gval (ad);
	argv[argc++] = au -> mta -> li_mta;
	argv[argc++] = au -> reason;

	argv[argc++] = eq;
	argv[argc++] = "submit-time";
	(void) strcpy (tbuf, "unknown");
	if (qp -> trace != NULL &&
	    qp -> trace -> trace_DomSinfo.dsi_time) {
		UTC ut = utclocalise (qp -> trace -> trace_DomSinfo.dsi_time);
		if (ut) {
			(void) sprintf (tbuf, "%d:%d:%d:%02d:%02d",
					ut -> ut_mon, ut -> ut_mday,
					ut -> ut_hour, ut -> ut_min,
					ut -> ut_sec);
			free ((char *) ut);
		}
		
	}
	argv[argc++] = tbuf;

	argv[argc] = NULLCP;

	arg2vstr (0, sizeof buffer, buffer, argv);
	PP_STAT (("%s", buffer));
}

static void wrdr2statfile (qp, sender, ad, lp, msgname)
Q_struct *qp;
ADDR		*sender;
ADDR		*ad;
LIST_RCHAN	*lp;
char *msgname;
{
	int sreason;

	PP_TRACE (("submit/wrdr2statfile ('%s')", lp -> li_auth -> reason));

	sreason = ad -> ad_reason;
	ad -> ad_reason = DRR_NO_REASON;
	(void) wr_dr_stat_log (qp, sender, ad, msgname, "transit");
	ad -> ad_reason = sreason;
}

static void logfailure (sender, ad, lp)
ADDR		*sender;
ADDR		*ad;
LIST_RCHAN	*lp;
{
	PP_LOG (LLOG_EXCEPTIONS,
		("Message failed to %s (%s) from %s to %s: %s", lp -> li_mta,
		 lp -> li_chan -> ch_name, gval(sender), gval(ad),
		 lp -> li_auth -> reason));
}

