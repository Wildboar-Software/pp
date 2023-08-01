/* wr_stat.c: write out some stats about a message being delivered */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/wr_stat.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/wr_stat.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: wr_stat.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "q.h"

#define gval(x)                 ((x -> ad_type) == AD_X400_TYPE ? \
                                        (x -> ad_r400adr) : (x -> ad_r822adr))
UTC utclocalise ();

wr_stat (ap, qp, msg, size)
ADDR	*ap;
Q_struct *qp;
char	*msg;
int	size;
{
	char	*argv[100];
	int	argc = 0;
	char	buffer[BUFSIZ];
	char midbuf[BUFSIZ];
	char tbuf[BUFSIZ];
	char tbuf2[BUFSIZ];
	static char *eq = "=";
	char	sbuf[40];

	argv[argc++] = "Deliv";
	argv[argc++] = msg;

	argv[argc++] = qp -> inbound -> li_chan -> ch_name;
	argv[argc++] = "->";
	if (ap -> ad_outchan && ap -> ad_outchan -> li_chan)
		argv[argc++] = ap -> ad_outchan -> li_chan -> ch_name;
	else
		argv[argc++] = "none";

	argv[argc++] = eq;
	argv[argc++] = "p1msgid";
	(void) msgid2rfc_aux (&qp -> msgid, midbuf, FALSE);
	argv[argc++] = midbuf;

	argv[argc++] = eq;
	argv[argc++] = "size";
	(void) sprintf (sbuf, "%d", size);
	argv[argc++] = sbuf;

	argv[argc++] = eq;
	argv[argc++] = "sender";
	argv[argc++] = (qp -> inbound -> li_chan -> ch_in_ad_type == AD_X400_TYPE) ? qp -> Oaddress -> ad_r400adr : qp -> Oaddress -> ad_r822adr;

	argv[argc++] = qp -> inbound -> li_mta;

	argv[argc++] = eq;
	argv[argc++] = "recip";
	argv[argc++] = gval(ap);

	if (ap -> ad_outchan && ap -> ad_outchan -> li_mta)
		argv[argc++] = ap -> ad_outchan -> li_mta;
	else
		argv[argc++] = "none";

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

	argv[argc] = NULLCP;

	if (arg2vstr (0, sizeof buffer, buffer, argv) == NOTOK)
		return NOTOK;
	PP_STAT (("%s", buffer));
	return OK;
}
