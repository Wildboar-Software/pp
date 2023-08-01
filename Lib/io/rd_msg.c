/* rd_msg.c: read in a msg from the queue + write addr offset routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_msg.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_msg.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: rd_msg.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



/*
-- These routines are 'high level' taking care of most of the housekeeping. --
*/



#include        "head.h"
#include        "q.h"
#include        <fcntl.h>
#include        <isode/cmd_srch.h>
#include        "prm.h"


extern	void getfpath ();
extern	long lseek ();

extern  int     errno;
extern  char    *aquefile,
		*no2txt3();
static  FILE    *msg_fp;
static  char    msgname[MAXPATHLENGTH];
int             ad_count;


void	rd_end ();

/* ---------------------  Read Message Routines  -------------------------- */



int rd_msg_file (mdir, prm, que, sender, recip, rcount, mode)
char            *mdir;
struct prm_vars *prm;
Q_struct        *que;
ADDR            **sender;
ADDR            **recip;
int             *rcount;
int		mode;
{
	int     retval;
	extern char *quedfldir;

	(void) sprintf (msgname, "%s/%s", mdir, aquefile);
	PP_DBG (("Lib/io/rd_msg(%s)", msgname));

	switch (mode) {
	    default:
		return RP_MECH;

	    case RDMSG_RDONLY:
		if ((msg_fp = fopen (msgname, "r")) == NULLFILE) {
			PP_SLOG (LLOG_EXCEPTIONS, msgname,
				 ("Lib/io/rd_msg flckopen error"));
			return RP_FOPN;
		}
		break;
	    case RDMSG_RDWR:
	    case RDMSG_RDONLYLCK:
		if ((msg_fp = flckopen (msgname,
					mode == RDMSG_RDWR ? "r+" : "r"))
		    == NULLFILE) {
			PP_SLOG (LLOG_EXCEPTIONS, msgname,
				 ("Lib/io/rd_msg flckopen error"));
			return (RP_FOPN);
		}
		break;
	}

	if (rp_isbad (retval = rd_prm (prm, msg_fp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/io/rd_msg/rd_prm err: '%s'", msgname));
		rd_end();
		return (retval);
	}

	if (rp_isbad (retval = rd_q (que, msg_fp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/io/rd_msg/rd_q err: '%s'", msgname));
		rd_end();
		return (retval);
	}

	if (rp_isbad (retval = rd_adr (msg_fp, TRUE, sender))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/io/rd_msg/rd_adr sender err: %s %s",
			rp_valstr (retval), msgname));
		rd_end();
		return (retval);
	}

	ad_count = 0;

	if (rp_isbad (retval = rd_adr (msg_fp, FALSE, recip))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/io/rd_msg/rd_adr recip err: %s %s",
			rp_valstr(retval), msgname));
		rd_end();
		return (retval);
	}

	que -> Oaddress = *sender;
	que -> Raddress = *recip;
	*rcount = ad_count;

	return (RP_OK);
}




void rd_end ()
{
	PP_DBG (("Lib/io/rd_end()"));
	if (msg_fp)  (void) flckclose (msg_fp);
	msg_fp = NULLFILE;
}




/* ---------------------  Write Address Offset Routines  ------------------ */




int wr_ad_status (ap, status)
ADDR    *ap;
int     status;
{
	char                    *str;
	extern  CMD_TABLE       atbl_status[];

	PP_DBG (("Lib/io/wr_ad_status()"));

	if (msg_fp == NULLFILE) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/io/wr_ad_status (NULLFILE)"));
		return (RP_MECH);
	}

	switch (status) {
	    case AD_STAT_PEND:
	    case AD_STAT_DONE:
	    case AD_STAT_DRREQUIRED:
	    case AD_STAT_DRWRITTEN:
		break;
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/io/wr_ad_status unknown"));
		status = AD_STAT_UNKNOWN;
		break;
	}
	str = rcmd_srch (status, atbl_status);

	if (lseek (fileno (msg_fp), ap -> ad_stat_offset, 0) == -1) {
		PP_LOG (LLOG_FATAL,
			("Lib/io/wr_ad_status lseek err %s", msgname));
		return (RP_FIO);
	}


	if (write (fileno (msg_fp), str, 4) != 4) {
		PP_LOG (LLOG_FATAL,
			("Lib/io/wr_ad_status write err %s", msgname));
		return (RP_FIO);
	}
	return (RP_OK);
}

int wr_ad_rcntno (ap, rcnt)
ADDR    *ap;
int     rcnt;
{
	char    buf[5];

	PP_DBG (("Lib/io/wr_ad_rcntno(%d)", rcnt));

	if (msg_fp == NULLFILE) {
		PP_LOG (LLOG_EXCEPTIONS, ("Lib/io/wr_ad_rcntno (NULLFILE)"));
		return (RP_MECH);
	}

	if (lseek (fileno (msg_fp), ap -> ad_rcnt_offset, 0) == -1) {
		PP_LOG (LLOG_FATAL,
			("Lib/io/wr_ad_rcntno lseek err %s", msgname));
		return (RP_FIO);
	}

	if (write (fileno (msg_fp), no2txt3 (rcnt, &buf[0]), 3) != 3) {
		PP_LOG (LLOG_FATAL,
			("Lib/io/wr_ad_rcntno write err %s", msgname));
		return (RP_FIO);
	}
	return (RP_OK);
}

int wr_q_nwarns (q)
Q_struct    *q;
{
	char    buf[5];

	PP_DBG (("Lib/io/wr_q_nwarns(%d)", q->nwarns));

	if (msg_fp == NULLFILE) {
		PP_LOG (LLOG_EXCEPTIONS, ("Lib/io/wr_q_nwarns (NULLFILE)"));
		return (RP_MECH);
	}

	if (lseek (fileno (msg_fp), q -> nwarns_offset, 0) == -1) {
		PP_LOG (LLOG_FATAL,
			("Lib/io/wr_q_nwarns lseek err %s", msgname));
		return (RP_FIO);
	}

	if (write (fileno (msg_fp), no2txt3 (q->nwarns, &buf[0]), 3) != 3) {
		PP_LOG (LLOG_FATAL,
			("Lib/io/wr_q_nwarns write err %s", msgname));
		return (RP_FIO);
	}
	return (RP_OK);
}

