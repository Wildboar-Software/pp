/* set_1dr.c: Do the necessary stuff to mark one recip as needing a DR */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/set_1dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/set_1dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: set_1dr.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "adr.h"
#include "q.h"
#include "dr.h"
#include "retcode.h"
#include <isode/cmd_srch.h>

extern UTC		utclocalise ();
extern char		*ppversion;
extern CMD_TABLE	rr_tcode[],
			rr_dcode[],
			rr_rcode[];

#define gval(x)                 ((x -> ad_type) == AD_X400_TYPE ? \
                                        (x -> ad_r400adr) : (x -> ad_r822adr))

int set_all_dr (qp, ap, msg, reason, diag, str)
Q_struct *qp;
ADDR	*ap;
char	*msg;
int	reason, diag;
char	*str;
{
	for (; ap; ap = ap -> ad_next)
		if (set_1dr(qp, ap -> ad_no, msg, reason, diag, str) == NOTOK)
			return NOTOK;
	return OK;
}

int set_1dr (qp, no, msg, reason, diag, str)
Q_struct *qp;
int	no;
char	*msg;
int	reason, diag;
char	*str;
{
	ADDR	*ap;

	for (ap = qp -> Raddress; ap; ap = ap -> ad_next) {
		if (ap -> ad_no == no)
			break;
	}

	if (ap == NULLADDR) {
		PP_LOG (LLOG_EXCEPTIONS, ("set_1dr: address %d not found",
					  no));
		return NOTOK;
	}

	ap -> ad_status = AD_STAT_DRREQUIRED;

	ap -> ad_reason = reason;
	ap -> ad_diagnostic = diag;
	ap -> ad_add_info = (str == NULLCP) ? NULLCP : strdup(str);

	return wr_dr_stat_log (qp, qp -> Oaddress, ap, msg,
			       reason == DRR_NO_REASON ?
			       "positive" : "negative");
}


/* ARGSUSED */
wr_dr_stat_log (qp, sender, ap, msg, drtype)
Q_struct *qp;
ADDR *sender;
ADDR *ap;
char *msg;
char *drtype;
{
	char 	*argv[100];
	int	argc;
	char buffer[BUFSIZ];
	char midbuf[BUFSIZ];
	char tbuf1[BUFSIZ], tbuf2[BUFSIZ];
	char sbuf[40];
	LIST_RCHAN	*lastchan;
	int	arg2vstr ();
	static char *eq = "=";
	static char *none = "none";

	argc = 0;
	argv[argc ++] = "DR";
	argv[argc++] = drtype;

	if (msg)
		argv[argc++] = msg;
	else
		argv[argc++] = none;

	argv[argc++] = eq;
	argv[argc++] = "p1msgid";
	(void) msgid2rfc_aux (&qp -> msgid, midbuf, FALSE);
	argv[argc++] = midbuf;

	argv[argc++] = eq;
	argv[argc++] = "size";
	(void) sprintf (sbuf, "%d", qp -> msgsize);
	argv[argc++] = sbuf;

	argv[argc++] = eq;
	argv[argc++] = "inchan";
	argv[argc++] = qp->inbound->li_chan->ch_name;

	argv[argc++] = eq;
	argv[argc++] = "dr_dest";
	argv[argc++] = gval(qp->Oaddress);

	argv[argc++] = qp->inbound->li_mta;

	argv[argc++] = eq;
	argv[argc++] = "outchan";
	if (ap -> ad_outchan && ap -> ad_outchan -> li_chan)
		argv[argc++] = ap->ad_outchan->li_chan->ch_name;
	else
		argv[argc++] = none;

	argv[argc++] = eq;
	argv[argc++] = "dr-src";
	argv[argc++] =  gval(ap);

	if (ap -> ad_outchan && ap -> ad_outchan -> li_mta)
		argv[argc++] = ap->ad_outchan->li_mta;
	else
		argv[argc++] = none;

	lastchan = ap -> ad_fmtchan;
	if (ap->ad_rcnt != 0) {
		int i = 0;
		while (i < ap->ad_rcnt && lastchan != NULLIST_RCHAN) {
			i++;
			lastchan = lastchan->li_next;
		}
	}			
	if (lastchan == NULLIST_RCHAN)
		lastchan = ap->ad_outchan;
		
	argv[argc++] = eq;
	argv[argc++] = "cur-channel";
	if (lastchan && lastchan -> li_chan)
		argv[argc++] = lastchan->li_chan->ch_name;
	else	argv[argc++] = "submit";

	if (ap -> ad_reason != DRR_NO_REASON) {
		argv[argc ++] = eq;
		argv[argc++] = "reason";
		argv[argc++] = rcmd_srch(ap -> ad_reason, rr_rcode);

		argv[argc++] = eq;
		argv[argc++] = "diag";
		argv[argc++] = rcmd_srch (ap -> ad_diagnostic, rr_dcode);

		if (ap -> ad_add_info)
			argv[argc++] = ap -> ad_add_info;
	}
	else {
		argv[argc++] = eq;
		argv[argc++] = "submit-time";
		(void) strcpy (tbuf1, "unknown");
		if (qp -> trace != NULL &&
		    qp -> trace -> trace_DomSinfo.dsi_time) {
			UTC ut = utclocalise (qp -> trace -> trace_DomSinfo.dsi_time);
			if (ut) {
				(void) sprintf (tbuf1, "%d:%d:%d:%02d:%02d",
						ut -> ut_mon, ut -> ut_mday,
						ut -> ut_hour, ut -> ut_min,
						ut -> ut_sec);
				free ((char *) ut);
			}
		
		}
		argv[argc++] = tbuf1;

		argv[argc++] = eq;
		argv[argc++] = "queued-time";
		(void) strcpy (tbuf2, "unknown");
		if (qp -> queuetime) {
			UTC ut = utclocalise (qp -> queuetime);
			if (ut) {
				(void) sprintf (tbuf2, "%d:%d:%d:%02d:%02d",
						ut -> ut_mon, ut -> ut_mday,
						ut -> ut_hour, ut -> ut_min,
						ut -> ut_sec);
				free ((char *) ut);
			}
		
		}
		argv[argc++] = tbuf2;

	}
	argv[argc] = NULL;

	if (arg2vstr (0, sizeof buffer, buffer, argv) == NOTOK)
		return NOTOK;
	PP_STAT (("%s", buffer));
	return OK;
}
