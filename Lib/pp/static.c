/* static.c: static configuration */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/static.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/static.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: static.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "chan.h"
#include "list_bpt.h"


CHAN            **ch_all;
Table           **tb_all;
LIST_BPT        *bodies_all = NULLIST_BPT;
LIST_BPT        *headers_all = NULLIST_BPT;



char		*aquefile = "addr";
char            *bquedir = "base";
char            *chndfldir = "chans";
char            *formdfldir = "format";
char            *loc_dom_mta = "bogus";
char            *loc_dom_site = "bogus";
char            *loc_or = "";
char            *postmaster = "***INVALID***";
char		*adminstration_req_alt = NULLCP;
char            *ppdbm = "ppdbm";
char            *pplogin = "pp";   /* -- login user id of pp -- */
char            *pptsapd_addr = "Internet=localhost+20001";
char            *delim1 = "\1\1\1\1\n";
char            *delim2 = "\1\1\1\1\n";
char            *mboxname = "ppmailbox";
char            *qmgr_hostname = "localhost";
char            *authchannel = "block";
char            *authloglevel = "low";
char            *wrndfldir = "warnings"; /* relative to tbldfldir */
char		*wrnfile = "warning"; 	 /* basename of files containing template warnings */

char		*x400_mta = NULLCP;

int		return_interval_norm = 24 * 3;	/* return after 3 days */
int		return_interval_high = 24 * 1; /* return time 1 day */
int		return_interval_low = 24 * 7; /* return time 7 days */
int		warn_interval = 24 * 1;		/* warn after 1 */
int		warn_number = 2;		/* 2 warnings */
int		max_hops = 25;	/* max number of trace fields */
int		max_loops = 5; /* max times through our MTA */
int		queue_fanout = 0; /* how many sub directories at each level */
int		queue_depth = 0; /* how many depths of sub directory */
int		use_fsync = 1; /* use fsync if available */
int		disk_percent = NOTOK;	/* free block percentage */
int		disk_blocks = NOTOK;	/* free block count */

/* hardwired in names */
char		*submit_prog = "submit";
char            *dr_file = "report.";
char            *uucpin_chan = "uucp-in";
char            *local_822_chan = "822-local";
char            *alias_tbl = "aliases";
char            *channel_tbl = "channel";
char            *list_tbl = "list";
char            *user_tbl = "users";
char            *or_tbl = "or";
char            *or2rfc_tbl = "or2rfc";
char            *rfc2or_tbl = "rfc2or";
char		*rfc1148gateway_tbl = "rfc1148gate";
char            *chan_auth_tbl = "auth.channel";
char            *mta_auth_tbl = "auth.mta";
char            *user_auth_tbl = "auth.user";
char		*hdr_prefix = "hdr.";
char            *hdr_822_bp = "hdr.822";
char		*hdr_p2_bp = "hdr.p2";
char		*hdr_p22_bp = "hdr.p22";
char		*hdr_ipn_bp = "hdr.ipn";
char            *ia5_bp = "ia5";
char		*qmgr_auth_tbl = "auth.qmgr";
char		*cont_822 = "822";
char		*cont_p2 = "p2";
char		*cont_p22 = "p22";
char		*mailfilter = ".mailfilter";
char		*sysmailfilter = "/usr/local/lib/mailfilter";
char		*submit_addr = NULLCP;
char		*dap_user = NULLCP;
char		*dap_passwd = NULLCP;

/* used in distribution list stuff */
char		*loc_dist_prefix = "dist-";
char		*list_tbl_comment = "Comment:";

/* used in locking */
char	*lockdir = "/tmp";
int	lockstyle = LOCK_FLOCK;
int	lock_break_time = 30 * 60; /* 30 mins grace time */

/* -- Logging info -- */

static LLog oper_log = {
	"oper", NULLCP, NULLCP,
	LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE, LLOG_FATAL, -1,
	LLOGCLS | LLOGCRT, NOTOK
};

static LLog stat_log = {
	"stat", NULLCP, NULLCP,
	LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE, LLOG_FATAL, -1,
	LLOGCLS | LLOGCRT, NOTOK
};

static LLog norm_log = {
	"norm", NULLCP, NULLCP,
	LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE, LLOG_FATAL, -1,
	LLOGCRT, NOTOK
};

LLog    *pp_log_norm = &norm_log;
LLog    *pp_log_oper = &oper_log;
LLog    *pp_log_stat = &stat_log;
