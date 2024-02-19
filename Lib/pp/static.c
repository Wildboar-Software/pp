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


extern CHAN            **ch_all = NULL;
extern Table           **tb_all = NULL;
extern LIST_BPT        *bodies_all = NULLIST_BPT;
extern LIST_BPT        *headers_all = NULLIST_BPT;



extern char		*aquefile = "addr";
extern char            *bquedir = "base";
extern char            *chndfldir = "chans";
extern char            *formdfldir = "format";
extern char            *loc_dom_mta = "bogus";
extern char            *loc_dom_site = "bogus";
extern char            *loc_or = "";
extern char            *postmaster = "***INVALID***";
extern char		*adminstration_req_alt = NULLCP;
extern char            *ppdbm = "ppdbm";
extern char            *pplogin = "pp";   /* -- login user id of pp -- */
/* pptsapd_addr used to contain "localhost," but this caused problems in
previous versions of ISODE. Modern Linux systems often use two aliases of
"localhost": one for IPv4 and one for IPv6. Even when explicitly requesting only
IPv4 addresses, the IPv6 address seems to get converted. This means two
duplicate IP addresses will be returned for this alias. This is not a problem
when a presentation address is used for connecting to a remote host, but it is
a nasty problem when listening, because you will get "already listening on
that port" errors. Despite this being fixed in my latest changes to ISODE, I
have changed this to 0.0.0.0 to prevent this problem. */
extern char            *pptsapd_addr = "Internet=0.0.0.0+20001";
extern char            *delim1 = "\1\1\1\1\n";
extern char            *delim2 = "\1\1\1\1\n";
extern char            *mboxname = "ppmailbox";
extern char            *qmgr_hostname = "localhost";
extern char            *authchannel = "block";
extern char            *authloglevel = "low";
extern char            *wrndfldir = "warnings"; /* relative to tbldfldir */
extern char		*wrnfile = "warning"; 	 /* basename of files containing template warnings */

extern char		*x400_mta = NULLCP;

extern int		return_interval_norm = 24 * 3;	/* return after 3 days */
extern int		return_interval_high = 24 * 1; /* return time 1 day */
extern int		return_interval_low = 24 * 7; /* return time 7 days */
extern int		warn_interval = 24 * 1;		/* warn after 1 */
extern int		warn_number = 2;		/* 2 warnings */
extern int		max_hops = 25;	/* max number of trace fields */
extern int		max_loops = 5; /* max times through our MTA */
extern int		queue_fanout = 0; /* how many sub directories at each level */
extern int		queue_depth = 0; /* how many depths of sub directory */
extern int		use_fsync = 1; /* use fsync if available */
extern int		disk_percent = NOTOK;	/* free block percentage */
extern int		disk_blocks = NOTOK;	/* free block count */

/* hardwired in names */
extern char		*submit_prog = "submit";
extern char            *dr_file = "report.";
extern char            *uucpin_chan = "uucp-in";
extern char            *local_822_chan = "822-local";
extern char            *alias_tbl = "aliases";
extern char            *channel_tbl = "channel";
extern char            *list_tbl = "list";
extern char            *user_tbl = "users";
extern char            *or_tbl = "or";
extern char            *or2rfc_tbl = "or2rfc";
extern char            *rfc2or_tbl = "rfc2or";
extern char		*rfc1148gateway_tbl = "rfc1148gate";
extern char            *chan_auth_tbl = "auth.channel";
extern char            *mta_auth_tbl = "auth.mta";
extern char            *user_auth_tbl = "auth.user";
extern char		*hdr_prefix = "hdr.";
extern char            *hdr_822_bp = "hdr.822";
extern char		*hdr_p2_bp = "hdr.p2";
extern char		*hdr_p22_bp = "hdr.p22";
extern char		*hdr_ipn_bp = "hdr.ipn";
extern char            *ia5_bp = "ia5";
extern char		*qmgr_auth_tbl = "auth.qmgr";
extern char		*cont_822 = "822";
extern char		*cont_p2 = "p2";
extern char		*cont_p22 = "p22";
extern char		*mailfilter = ".mailfilter";
extern char		*sysmailfilter = "/usr/local/lib/mailfilter";
extern char		*submit_addr = NULLCP;
extern char		*dap_user = NULLCP;
extern char		*dap_passwd = NULLCP;

/* used in distribution list stuff */
extern char		*loc_dist_prefix = "dist-";
extern char		*list_tbl_comment = "Comment:";

/* used in locking */
extern char	*lockdir = "/tmp";
extern int	lockstyle = LOCK_FLOCK;
extern int	lock_break_time = 30 * 60; /* 30 mins grace time */

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

extern LLog    *pp_log_norm = &norm_log;
extern LLog    *pp_log_oper = &oper_log;
extern LLog    *pp_log_stat = &stat_log;
