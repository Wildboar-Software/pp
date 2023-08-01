/* chans.c: channel specific stuff for the qmgr */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/qmgr/RCS/chans.c,v 6.0 1991/12/18 20:27:38 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/chans.c,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: chans.c,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 */



#include <isode/logger.h>
#include "types.h"
#include "Qmgr-ops.h"
#include "util.h"

extern struct Mtalist *findmtalist ();
extern MsgStruct *newmsgstruct ();
extern MsgStruct *find_msg ();
extern Chanlist *findchanlist ();
extern Mlist	*findmtamsg ();

extern int warn_number;

static int	cb_once_only = 0;
static Connblk cbqueue;
static Connblk *CHead = &cbqueue;
static char	*cb2name ();
static char	*cb_print ();
static int	cb_count = 0;
static int	nspecials = 0;
static int	demand_chan = 0;

Chanlist _runq, *runq;
void	addtorunq (), delfromrunq ();

struct eventqueue {
	struct eventqueue *ev_forw;
	struct eventqueue *ev_back;

	time_t	ev_time;
	Connblk *ev_conn;
	enum cb_type ev_type;
	Chanlist *ev_clp;
};

struct stats stats;	

static struct eventqueue evqueue;
static struct eventqueue *EHead = &evqueue;
static int ev_count = 0;
static struct eventqueue *newevblk ();
static void freevblk (), init_events ();
static void do_event ();
static void channel_event ();
static void special_event ();
static void restart ();
#define NULLEV	((struct eventqueue *)0)

static int runnable ();
static int invoke ();
static int timeout_proc ();
static int channel_ttl ();
static int expiremsg (), warnmsg ();
/* static void setfds (); */
static void getresponse ();
static void start_timer ();
extern void kill_msg ();

extern Mlist *nextmsg ();

extern fd_set perf_rfds, perf_wfds;
extern int perf_nfds;
extern int delaytime;
extern int nodisablemsgclean;

void	review_channels ();
void	investigate_chan ();
void	investigate_mta ();
void	delay_channel ();
void	cleanup_conn ();
void	cache_clear (), clear_msgcache ();
void	cache_inc (), msgcache_inc ();
void	freems ();
void	insertinchan ();
void	insertindelchan ();
void	insertindrchan ();
void	sort_chans();
void	zapmsg ();

time_t 	msgmincache ();
int nchansrunning = 0;
static int timer_running = 0;
static int high_wtmk, low_wtmk;

static int	do_processmessage (), processmessage_result (),
	processmessage_error (), do_quit ();
static int	do_initchannel (), channelinit_result ();
static int	do_readqueue (), readqueue_result ();
static int	general_error ();
static time_t	nextchantime;
static int	rqueue;
static int 	noperations;

struct client_dispatch {
	char		*ds_name;
	int		ds_operation;

	IFP		ds_argument;
	modtyp		*ds_fr_mod;
	int		ds_fr_index;
	IFP		ds_result;
	IFP		ds_error;

	char		*ds_help;
};

static struct client_dispatch dis[] = {
#define	DIS_PROC	0
{
	"processmessage", operation_Qmgr_processmessage,
	do_processmessage, &_ZQmgr_mod, _ZProcMsgQmgr,
	processmessage_result, processmessage_error,
	"Process some messages"
},
#define DIS_INIT	1
{
	"channelInitialise", operation_Qmgr_channelInitialise,
	do_initchannel, &_ZQmgr_mod, _ZChannelQmgr,
	channelinit_result, general_error,
	"Initialise a channel"
},
#define DIS_QUIT	2
{
	"quit", 0, do_quit, NULL, 0, NULLIFP, NULLIFP,
	"terminate the association"
},
#define DIS_READQ	3
{
	"readqueue", operation_Qmgr_readqueue,
	do_readqueue, &_ZQmgr_mod, _ZPseudo_readqueueQmgr,
	readqueue_result, general_error,
	"Load up the queue"
},
	NULL
};

static void create_chan (chan)
CHAN	*chan;
{
	Chanlist *clp;
	extern int chan_state;

	PP_TRACE (("create_chan (%s)", chan -> ch_name));

	if (nchanlist == 0)
		chan_list = (Chanlist **) malloc (sizeof (Chanlist *));
	else
		chan_list = (Chanlist **) realloc ((char *)chan_list,
						   (unsigned)(nchanlist + 1)
						   * sizeof (Chanlist *));
	chan_list[nchanlist] = clp = (Chanlist *) calloc (1, sizeof (Chanlist));
	clp -> channame = chan -> ch_name;
	clp -> chan = chan;
	clp -> mtas = (struct Mtalist *) calloc (1, sizeof (Mtalist));
	clp -> mtas -> mta_back = clp -> mtas -> mta_forw = clp -> mtas;
	clp -> chan_enabled = chan_state;
	clp -> mta_hash = NULL;
	switch (chan -> ch_chan_type) {
	    case CH_DELETE:
		delete_chan = clp;
		break;

	    case CH_QMGR_LOAD:
		clp -> chan_special = 1;
		loader_chan = clp;
		break;

	    case CH_DEBRIS:
		clp -> chan_special = 1;
		trash_chan = clp;
		break;

	    case CH_TIMEOUT:
		timeout_chan = clp;
		break;
	    case CH_WARNING:
		warn_chan = clp;
		break;

	    case CH_OUT:
	    case CH_BOTH:
		if (chan -> ch_sort[0] == 0)
			chan -> ch_sort[0] = CH_SORT_MTA;
		break;
	}
	if (chan -> ch_sort[0] == 0)
		chan -> ch_sort[0] = CH_SORT_NONE;
	if (chan -> ch_sort[0] != CH_SORT_NONE)
		clp -> mta_hash = (Mtalist **)
			calloc (MTA_HASHSIZE, sizeof (*clp->mta_hash));
	if (clp -> chan_special)
		addtorunq (clp);
	nchanlist ++;
}

void init_chans ()
{
	int	i;

	PP_TRACE (("init_chans ()"));

	runq = &_runq;
	runq -> cl_forw = runq -> cl_back = runq;

	for (i = 0; ch_all[i]; i++)
		create_chan (ch_all[i]);
	init_events ();
	stats.boottime = current_time;
}

/* ARGSUSED */
int chan_lose (fd, acf)
int	fd;
struct AcSAPfinish *acf;
{
	Connblk *cb;

	PP_TRACE (("chan_lose (%d)", fd));
	if ((cb = findcblk (fd)) == NULLCB)
		return ACS_ACCEPT;
	/* restore the state */
	delay_channel (cb);
	if (fd != NOTOK) {
		FD_CLR (fd, &perf_rfds);
		FD_CLR (fd, &perf_wfds);
	}
	cb -> cb_fd = NOTOK;
	cleanup_conn (cb);

	return ACS_ACCEPT;
}

void start_specials ()
{
	Connblk *cb;

	PP_TRACE (("start_specials ()"));

	if (loader_chan == NULLCHANLIST)
		PP_LOG (LLOG_EXCEPTIONS,
			("No loading channel specified"));
	else {
		loader_chan -> chan_update = 1;
	}

	if (trash_chan == NULLCHANLIST)
		PP_LOG (LLOG_EXCEPTIONS,
			("No debris channel specified"));
	else {
		cache_inc (&trash_chan -> cache,
			   random() % debris_time);
		trash_chan -> chan_update = 1;
	}
	if (timeout_chan == NULLCHANLIST)
		PP_LOG (LLOG_EXCEPTIONS,
			("No timeout channel specified"));
	else {
		if (warn_chan == NULLCHANLIST)
			PP_LOG (LLOG_EXCEPTIONS,
				("No warning channel specified"));
		cb = newcblk (cb_timer);
		cb -> cb_proc = timeout_proc;
		cb -> cb_reload = timeout_time;
		(void) newevblk (NULLCHANLIST, cb, cb_timer,
				 random () % timeout_time);
	}
}	

#define LOAD_UPDATE_INTERVAL	15
#define L1_DECAY	(0.9)
#define L2_DECAY	(0.92)

static void do_load_avg ()
{
	static time_t lastload;
	static int last_msg_out, last_msg_in;
	int mo, mi;
	static int count = 0;

	if (lastload == 0)
		lastload = current_time;
	if (current_time - lastload < LOAD_UPDATE_INTERVAL) {
		stats.runnable_chans = L1_DECAY * stats.runnable_chans +
			(1.0 - L1_DECAY) * rqueue;
		return;
	}
	mo = stats.n_messages_out - last_msg_out;
	mi = stats.n_messages_in - last_msg_in;
	if (mi < 0 || mo < 0)
		PP_LOG (LLOG_EXCEPTIONS, ("Negative stats %d & %d",
					  mi, mo));

	while (lastload < current_time) {
		stats.runnable_chans = L1_DECAY * stats.runnable_chans +
			(1.0 - L1_DECAY) * rqueue;
		stats.ops_sec = L2_DECAY * stats.ops_sec +
			(1.0 - L2_DECAY) *
				((double)noperations /
					 (double)LOAD_UPDATE_INTERVAL);
		lastload += LOAD_UPDATE_INTERVAL;
		stats.msg_sec_in = L2_DECAY * stats.msg_sec_in +
			(1.0 - L2_DECAY) * ((double)mi/
					    (double)LOAD_UPDATE_INTERVAL);
		stats.msg_sec_out = L2_DECAY * stats.msg_sec_out +
			(1.0 - L2_DECAY) * ((double)mo/
					    (double)LOAD_UPDATE_INTERVAL);
	}
	last_msg_in = stats.n_messages_in;
	last_msg_out = stats.n_messages_out;

	noperations = 0;
	PP_TRACE (("Load averages rq %g, ops %g mi %g mo %g",
		   stats.runnable_chans, stats.ops_sec,
		   stats.msg_sec_in, stats.msg_sec_out));
	lastload = current_time;
	if (count++ > 100) {
		count = 0;
		PP_NOTICE (("STATS: m-in=%d m-out=%d, a-in=%d a-out=%d os=%g rc=%g mis=%g mos=%g %.22s",
			    stats.n_messages_in, stats.n_messages_out,
			    stats.n_addr_in, stats.n_addr_out,
			    stats.ops_sec, stats.runnable_chans,
			    stats.msg_sec_in, stats.msg_sec_out,
			    ctime (&stats.boottime)));
	}
	ll_close (pp_log_norm);
}

void schedule ()
{
	struct eventqueue *ev;
	static time_t lasttime;
	time_t	delta;

	PP_TRACE (("schedule ()"));

	if (opmode && nchansrunning == 0) {
		if (opmode == OP_SHUTDOWN)
			exit (0);
		if (opmode == OP_RESTART)
			restart ();
	}
#ifdef notdef
	if (countdown -- < 0) {
		countdown = 50;
		sort_chans ();
	}
#endif

	if (lasttime == 0)
		lasttime = current_time;
	delta = current_time - lasttime;

	review_channels (current_time);
	do_load_avg ();

	if (EHead -> ev_forw != EHead)
		EHead -> ev_forw -> ev_time -= delta;


	for (ev = EHead -> ev_forw; ev != EHead;) {
		struct eventqueue *ev2 = ev -> ev_forw;

		if (ev -> ev_time <= 0) {
			do_event (ev);
			freevblk (ev);
		}
		else
			break;
		ev = ev2;
	}
	review_channels (current_time);

	if (EHead -> ev_forw == EHead )
		delaytime = nextchantime;
	else {
		PP_DBG (("event head time = %d delta = %d",
			 EHead -> ev_forw -> ev_time, delta));
		delaytime = max(0, EHead -> ev_forw -> ev_time);
	}
	if (nextchantime < delaytime)
		delaytime = nextchantime;

	PP_TRACE (("delaytime == %d", delaytime));
	lasttime = current_time;
	if (opmode && nchansrunning == 0) {
		if (opmode == OP_SHUTDOWN)
			exit (0);
		if (opmode == OP_RESTART)
			restart ();
	}
}

void	addtorunq (clp)
Chanlist *clp;
{
	Chanlist *clp2;

	PP_TRACE (("adding channel %s to runq", clp -> channame));
	for (clp2 = runq -> cl_forw; clp2 != runq; clp2 = clp2 -> cl_forw)
		if (clp2 == clp) {
			PP_LOG (LLOG_EXCEPTIONS,
				("channel %s already on runq",
				 clp -> channame));
			return;
		}
	insque (clp, runq -> cl_forw);
}

static void prunq ()
{
	Chanlist *clp;

	for (clp = runq -> cl_forw; clp != runq; clp = clp -> cl_forw)
		PP_TRACE (("runq: %s", clp -> channame));
}


void	delfromrunq (clp)
Chanlist *clp;
{
	PP_TRACE (("removeing channel %s from runq", clp -> channame));

	if (clp -> cl_forw == NULLCHANLIST ||
	    clp -> cl_back == NULLCHANLIST) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("chan %s not on runq", clp -> channame));
		return;
	}
	if (clp -> nmtas != 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("nmtas not zero for %s",
					  clp -> channame));
		return;
	}
	if (clp -> mtas -> mta_back != clp -> mtas) {
		PP_LOG (LLOG_EXCEPTIONS, ("Still mtas on channel"));
		clp -> chan_update = 1;
		return;
	}
	if (clp -> chan_special) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Trying to remove special chan %s from runq",
			 clp -> channame));
		return;
	}
	remque (clp);
	clp -> cl_forw = clp -> cl_back = NULLCHANLIST;
}
			
			
static void restart ()
{
	int 	n, i;
	extern char *cmddfldir, *dupfpath ();
	char	*path;
	char *argv[20];
	int	argc;
	char	nbuf[20];
	extern int chan_state;

	PP_TRACE (("restart ()"));

	while (CHead -> cb_forw != CHead)
		freecblk (CHead -> cb_forw);	/* shut down connecitons */

	n = getdtablesize ();

	i = (isatty(2) ? 3 : 0);
	for (; i <= n; i++)
		(void) close (i);
#ifdef notdef
	for (i = 0; i < NSIG; i++)
		(void) signal (i, SIG_DFL);
#endif
	argc = 0;
	argv[argc++] = myname;
	if (maxchansrunning != MAXCHANSRUNNING) {
		argv[argc++] = "-m";
		(void) sprintf (nbuf, "%d", maxchansrunning);
		argv[argc++] = strdup (nbuf);
	}
	if (load_time != LOAD_RETRY_INTERVAL) {
		argv[argc++] = "-l";
		(void) sprintf (nbuf, "%d", load_time/60/60);
		argv[argc++] = strdup (nbuf);
	}
	if (debris_time != TRASH_RETRY_INTERVAL) {
		argv[argc++] = "-d";
		(void) sprintf (nbuf, "%d", debris_time/60/60);
		argv[argc++] = strdup (nbuf);
	}
	if (cache_time != CACHE_TIME) {
		argv[argc++] = "-c";
		(void) sprintf (nbuf, "%d", cache_time);
		argv[argc++] = strdup (nbuf);
	}
	if (timeout_time != TIMEOUT_RETRY_INTERVAL) {
		argv[argc++] = "-t";
		(void) sprintf (nbuf, "%d", timeout_time / 60 / 60);
		argv[argc++] = strdup (nbuf);
	}
	if (chan_state == 0)
		argv[argc++] = "-D";
	argv[argc] = NULLCP;
	path = dupfpath (cmddfldir, myname);
	(void) execv (path, argv);
	(void) execv (myname, argv);
	_exit(1);
}

void review_channels (now)
time_t	now;
{
	Chanlist *clp;
	int ndiffchans = 0;
	time_t	tdiff;

	PP_TRACE (("review_channels"));
	nextchantime = MAX_SLEEP;
	rqueue = nchansrunning;
	demand_chan = 0;

	/* first find how many different channels are running */
	for (clp = runq -> cl_forw; clp != runq; clp = clp -> cl_forw)
		if (clp -> nactive > 0)
			ndiffchans ++;

	/* high watermark is the share of the channels allowed each */
	high_wtmk = max(1, (maxchansrunning-1)/(ndiffchans ? ndiffchans : 1));
	/* low water mark - each channel allowed at least this */
	low_wtmk = max(1, high_wtmk/2);

	for (clp = runq -> cl_forw; clp != runq; clp = clp -> cl_forw) {
		switch (runnable (clp, now)) {
		    case OK:
			(void) newevblk (clp, NULLCB,
					 clp-> chan_special ?
					 cb_special : cb_channel,
					 (time_t)0);
			rqueue ++;
			break;
		    case DONE:
			/*
			 * If it is allowed more - but can't get one
			 * it can demand its fair share.
			 * In this case we have to reasses the high
			 * water mark.
			 */
			if (clp -> nactive < low_wtmk) {
				if (clp -> nactive == 0)
					ndiffchans ++;
				demand_chan = 1;
				PP_TRACE(("%s demands a channel",
					   clp -> channame));
			}
			rqueue ++;
			break;
		    case NOTOK:
			tdiff = clp -> nextevent - current_time;
			if (tdiff > 0 && tdiff < nextchantime)
				nextchantime = tdiff;
			break;
		}
	}

	high_wtmk = max(1, maxchansrunning/(ndiffchans ? ndiffchans : 1));
	PP_TRACE (("review channels lw=%d hw=%d nd=%d",
		   low_wtmk, high_wtmk, ndiffchans));
}


static int runnable (clp, now)
Chanlist *clp;
time_t	now;
{
	PP_TRACE (("runnable %s?", clp -> channame));
	if (opmode)
		return NOTOK;
	if (clp -> chan_update && clp -> nactive == 0) {
		investigate_chan (clp, now);
		clp -> chan_update = 0;
	}
	if (clp -> chan_special == 0 && clp -> num_msgs <= 0
	    && clp -> num_drs <= 0) /* special channel with things to do */
		return NOTOK;
	if (clp -> chan_enabled == 0) /* channel switched off */
		return NOTOK;
	if (clp -> cache.cachetime > now ||
	    clp -> nextevent > now ) /* not time yet */
		return NOTOK;
	if (clp -> chan_special && nspecials > 0)
		return NOTOK;
	if (clp -> chan -> ch_maxproc > 0 &&
	    clp -> nactive >= clp -> chan -> ch_maxproc)
		return NOTOK;
	if (clp -> nactive <= 0) {
		if (nchansrunning >= maxchansrunning) /* too many running already */
			return DONE;

		return OK; /* None running - start one */
	}
/*
 * So there is a channel already running:
 * OK - should we start more - to do this we need the following
 * o More mtas ready than chans currently running
 * o We started the last channel more than a 15 secs ago
 *
 * o We aren't consuming all the channels allowed
 * o More messages than channels * 5 OR last time we started was ages ago
 */
	if (clp -> nmtas <= clp -> nactive
	    || clp -> laststart + 15 > current_time)
		return NOTOK;

	
	/* ok - we ought to start a new channel - but should we? */

	if (clp -> nactive >= max(1,maxchansrunning-1))
		return DONE;	/* Would like to - but out of resources */
	
	if ((clp -> num_msgs + clp -> num_drs) > clp -> nactive * 5
	    || clp -> laststart + 60*5 < current_time) {
		Mtalist *mlp = NULLMTALIST;
		
		if (nextmsg (clp, &mlp, 0, 0) == NULL)
			return NOTOK; /* nope all locked */
		PP_TRACE (("Can start a new chan on %s", mlp -> mtaname));
		if (nchansrunning >= maxchansrunning) /* too many running already */
			return DONE;
		return OK;
	}
	/* No - this channel is just not ready to start another one */
	return NOTOK;
}

static void do_event (ev)
struct eventqueue *ev;
{
	Connblk *cb;

	PP_TRACE (("do_event ()"));

	if ((cb = ev -> ev_conn) == NULLCB) {
		ev -> ev_conn = cb = newcblk (ev -> ev_type);
		cb -> cb_clp = ev -> ev_clp;
	}

	switch (cb -> cb_type) {
	    case cb_channel:
		channel_event (ev);
		break;

	    case cb_special:
		special_event (ev);
		break;

	    case cb_timer:
		PP_TRACE (("Timeout event"));
		if (cb -> cb_proc)
			(*cb -> cb_proc)();
		if (cb -> cb_reload)
			(void) newevblk (NULLCHANLIST, cb, cb_timer,
					 cb -> cb_reload);
		else	freecblk (cb);
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown type %d", cb -> cb_type));
		break;
	}
}

static int chan_still_ready (cb)
Connblk *cb;
{
	/* should we continue to process? */

	/* don't process if:
	 * - channel disabled
	 * - we are in some global shutdown
	 * - we are being greedy & others are suffering
	 */
	if (cb -> cb_clp -> chan_enabled == 0 || opmode ||
	    (demand_chan && cb -> cb_nops > MIN_OPS &&
	     cb -> cb_clp -> nactive > high_wtmk)) {
		if (demand_chan) {
			PP_TRACE (("Closing down %s due to demand h=%d l=%d",
				   cb_print (cb), high_wtmk, low_wtmk));
			if (cb -> cb_clp -> cache.cachetime <
			    current_time)
				cb -> cb_clp -> cache.cachetime =
					current_time + 15;
				/* we are going to satisfy request */
			demand_chan = 0;
		}
		return NOTOK;
	}
	/*
	 * hmm - this channel is not going so well - perhaps we should
	 * not try so hard. E.g. close down this channel if there is more
	 * than one running anyway.
	 */
	if (cb -> cb_clp -> error_count > 5) {
		cb -> cb_clp -> error_count = 0;
		if (cb -> cb_clp -> cache.cachetime < current_time)
			cache_inc (&cb -> cb_clp -> cache, cache_time);
		if (cb -> cb_clp -> nactive > 1) {
			PP_NOTICE (("Closing channel %s as things aren't going very well",
				    cb_print(cb)));
			return NOTOK;
		}
	}
	/*
	 * OK - grab the next message and GO.
	 */
	if (cb -> cb_ml == NULL &&
	    (cb -> cb_ml = nextmsg (cb -> cb_clp,
				    &cb -> cb_mlp, 1, 0)) == NULL) {
			
		if (cb -> cb_clp -> cache.cachetime < current_time)
			/* stop it thrashing... */
			cb -> cb_clp -> cache.cachetime =
				current_time + 15;
		return NOTOK;
	}
	return OK;
}

static void channel_event (ev)
struct eventqueue *ev;
{
	Connblk *cb = ev -> ev_conn;
	Mlist	*ml;
	Chanlist *clp;

	PP_TRACE (("channel_event ()"));

	if ((clp = ev -> ev_clp) == NULLCHANLIST)
		PP_TRACE (("No chanlist of event queue"));

	switch (cb -> cb_state) {
	    case cb_idle:
		if (runnable (clp, current_time) != OK) {
			freecblk (cb);
			return;
		}
		if ((ml = nextmsg (cb -> cb_clp, &cb -> cb_mlp, 1, 0)) == NULL) {
			clp -> chan_update = 1;
			freecblk (cb);
			return;
		}
		cb -> cb_ml = ml;
		cb -> cb_clp -> lastattempt = current_time;
		cb -> cb_clp -> laststart = current_time;
		switch (start_async (cb, clp -> channame)) {
		    case NOTOK:
			cache_inc (&clp -> cache, cache_time);
			clp -> nextevent = clp -> cache.cachetime;
			cleanup_conn (cb);
			break;

		    case DONE:
			(void) newevblk (cb -> cb_clp, cb,
					 cb -> cb_type, (time_t)0);
			/* and fall */
		    case OK:
			clp -> nactive ++;
			nchansrunning ++;
			break;
		}
		break;

	    case cb_conn_established:
		(void) chan_invoke (cb, DIS_INIT,
				    (caddr_t *) &clp -> channame);
		break;

	    case cb_conn_request1:
	    case cb_conn_request2:
	    case cb_init_sent:
	    case cb_proc_sent:
	    case cb_close_sent:
		PP_TRACE (("Shouldn't be in this state - %d",
			   cb -> cb_state));
		break;
		
	    case cb_active:
		if (chan_still_ready(cb) == NOTOK) {
			(void) chan_invoke (cb, DIS_QUIT,
					    (caddr_t *)0);
		}
		else {
			(void) chan_invoke (cb, DIS_PROC, (caddr_t *)0);
		}
		break;
	}
}

static void special_event (ev)
struct eventqueue *ev;
{
	Connblk *cb = ev -> ev_conn;
	Chanlist	*clp = ev -> ev_clp;

	PP_TRACE (("special_event (%s)", cb_print (cb)));
	switch (cb -> cb_state) {
	    case cb_idle:
		if (runnable (clp, current_time) != OK) {
			freecblk (cb);
			return;
		}
		clp -> lastattempt = current_time;
		clp -> laststart = current_time;

		switch (start_async (cb, clp -> channame)) {
		    case NOTOK:
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't start special channel %s",
				 clp -> channame));
			cache_inc (&clp -> cache, (time_t)60);
			(void) newevblk (NULLCHANLIST, NULLCB, cb_timer,
					 (time_t) 60);
			clp -> chan_update = 1;
			freecblk (cb);
			break;

		    case DONE:
			(void) newevblk (clp, cb, cb -> cb_type,
					 (time_t)0);
		    case OK:
			nspecials ++;
			clp -> nactive ++;
			nchansrunning ++;
			if (clp -> chan -> ch_chan_type == CH_QMGR_LOAD &&
			    nodisablemsgclean == 0 &&
			    delete_chan -> chan_enabled) {
				delete_chan -> chan_enabled = 0;
				delete_chan -> chan_syssusp = 1;
			}
			break;
		}
		break;

	    case cb_active:
		switch (clp -> chan -> ch_chan_type) {
		    case CH_QMGR_LOAD:
			if (cb -> cb_clp -> chan_enabled == 0 || opmode) {
				(void) chan_invoke (cb, DIS_QUIT, (caddr_t*)0);
				clp -> cache.cachetime =
					current_time + 60;
			}
			else
				(void) chan_invoke (cb, DIS_READQ,
						    (caddr_t *)0);
			break;
		    case CH_DEBRIS:
			(void) chan_invoke (cb, DIS_QUIT, (caddr_t *)0);
			clp -> lastsuccess = current_time;
			clp -> cache.cachetime = current_time + debris_time;
			break;
		    case CH_TIMEOUT:
			(void) chan_invoke (cb, DIS_QUIT, (caddr_t *)0);
			break;
		}
		clp -> chan_update = 1;
		break;

	    case cb_conn_established:
		switch (clp -> chan -> ch_chan_type) {
		    case CH_QMGR_LOAD:
			PP_TRACE (("start loading"));
			(void) chan_invoke (cb, DIS_READQ, (caddr_t *)0);
			break;
		    case CH_DEBRIS:
			PP_TRACE (("start debris collection"));
			(void) chan_invoke (cb, DIS_INIT,
					    (caddr_t *)&clp -> channame);
			break;
		    case CH_TIMEOUT:
			break;
		}
		break;
	    case cb_finished:
		switch (clp -> chan -> ch_chan_type) {
		    case CH_QMGR_LOAD:
			(void) chan_invoke (cb, DIS_QUIT, (caddr_t *)0);
			clp -> cache.cachetime = current_time + load_time;
			if (delete_chan -> chan_enabled == 0 &&
			    delete_chan -> chan_syssusp == 1) {
				delete_chan -> chan_enabled = 1;
				delete_chan -> chan_syssusp = 0;
				delete_chan -> chan_update = 1;
			}
			clp -> chan_update = 1;
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS, ("Bad state in special %d",
						  cb -> cb_state));
			break;
		}
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Bad state in special %d",
					  cb -> cb_state));
		break;
	}
}

int start_async (cb, title)
Connblk *cb;
char	*title;
{
	int	sd;
	int	result;

	PP_NOTICE (("Starting channel %s %s", title, cb_print(cb)));
	switch(result = assoc_start (title, &sd)) {
	    case NOTOK:
		PP_LOG (LLOG_EXCEPTIONS, ("Can't start channel %s",
					  title));
		return NOTOK;

	    case CONNECTING_1:
		cb -> cb_state = cb_conn_request1;
		cb -> cb_fd = sd;
		FD_CLR (sd, &perf_rfds);
		FD_SET (sd, &perf_wfds);
		if (sd >= perf_nfds)
			perf_nfds = sd + 1;
		break;

	    case CONNECTING_2:
		cb -> cb_state = cb_conn_request2;
		cb -> cb_fd = sd;
		FD_CLR (sd, &perf_wfds);
		FD_SET (sd, &perf_rfds);
		if (sd >= perf_nfds)
			perf_nfds = sd + 1;
		break;

	    case DONE:
		cb -> cb_state = cb_conn_established;
		cb -> cb_fd = sd;
		FD_CLR (sd, &perf_wfds);
		FD_SET (sd, &perf_rfds);
		if (sd >= perf_nfds)
			perf_nfds = sd + 1;
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Unknown return code from assoc_start"));
		return NOTOK;
	}
	start_timer ();
	cb -> cb_ttl = current_time + 60;
	remque (cb -> cb_clp);
	insque (cb -> cb_clp, runq -> cl_back);
	PP_DBG (("Association descriptor=%d", sd));
	return result;
}

int chan_invoke (cb, type, arg)
Connblk *cb;
int	type;
caddr_t	*arg;
{
	struct client_dispatch *ds;
	int result;

	noperations ++;
	cb -> cb_nops ++;
	ds = &dis[type];
	switch (type) {
	    default:
		PP_TRACE (("%s %s", ds -> ds_name, cb_print (cb)));
		break;
	    case DIS_PROC:
		PP_NOTICE (("%s %s", ds -> ds_name, cb_print(cb)));
		break;
	}
	switch (result = invoke (cb, table_Qmgr_Operations, 
				 ds, arg)) {
	    case OK:
		break;

	    case NOTOK:
		cb -> cb_state = cb_active;
		break;

	    case DONE:
		cleanup_conn (cb);
		break;
	}
	return result;
}

/*  */

static int invoke (cb, ops, ds, args)
Connblk *cb;
struct RyOperation ops[];
register struct client_dispatch *ds;
char  **args;
{
    int	    result;
    caddr_t in;
    struct RoSAPindication  rois;
    register struct RoSAPindication *roi = &rois;
    register struct RoSAPpreject   *rop = &roi -> roi_preject;
    int	sd;

    PP_TRACE (("invoke (%s)", cb_print (cb)));

    sd = cb -> cb_fd;
    in = NULL;
    if (ds -> ds_argument &&
	(result = (*ds -> ds_argument) (sd, ds, args, &in)) != OK)
	return result;

     switch (result = RyStub (sd, ops, ds -> ds_operation,
			      cb -> cb_id = RyGenID (sd),
			      NULLIP, in, ds -> ds_result, ds -> ds_error,
			      ROS_ASYNC, roi)) {
	case NOTOK:		/* failure */
	    ros_advise (rop, "STUB");
	    if (ROS_FATAL (rop -> rop_reason))
		    return DONE;
	    return NOTOK;

	case OK:		/* got a result/error response */
	    break;

	case DONE:		/* got RO-END? */
	    adios (NULLCP, "got RO-END.INDICATION");
	    return NOTOK;

	default:
	    adios (NULLCP, "unknown return from RyStub=%d", result);
	    /* NOTREACHED */
    }

    if (ds -> ds_fr_mod && in)
	fre_obj (in, ds -> ds_fr_mod -> md_dtab[ds -> ds_fr_index],
		 ds -> ds_fr_mod, 1);
    return OK;
}

void chan_manage (fd)
int	fd;
{
	register Connblk *cb;
	int	sd;

	if ((cb = findcblk (fd)) == NULLCB) {
		PP_LOG (LLOG_EXCEPTIONS, ("fd %d is not registered!", fd));
		FD_CLR (fd, &perf_rfds);
		FD_CLR (fd, &perf_wfds);
		return;
	}

	PP_TRACE (("chan_manage (%s)", cb_print (cb) ));

	switch (cb -> cb_state) {
	    case cb_conn_request1:
	    case cb_conn_request2:
		PP_TRACE (("Awaiting async connection (state %d)",
			   cb -> cb_state == cb_conn_request1 ? 1 : 2));
		switch (acsap_retry (sd = cb -> cb_fd)) {
		    case NOTOK:
			PP_TRACE (("Connection error on %s", cb_print (cb)));
			delay_channel (cb);
			cleanup_conn (cb);
			return;

		    case CONNECTING_1:
			cb -> cb_state = cb_conn_request1;
			PP_TRACE (("State 1 on %s", cb_print (cb)));
			FD_CLR (sd, &perf_rfds);
			FD_SET (sd, &perf_wfds);
			if (sd >= perf_nfds)
				perf_nfds = sd + 1;
			break;

		    case CONNECTING_2:
			cb -> cb_state = cb_conn_request2;
			PP_TRACE (("State 2 on %s", cb_print (cb)));
			FD_CLR (sd, &perf_wfds);
			FD_SET (sd, &perf_rfds);
			if (sd >= perf_nfds)
				perf_nfds = sd + 1;
			break;
		    case DONE:
			cb -> cb_state = cb_conn_established;
			PP_TRACE (("connection now established on %s",
				   cb_print (cb)));
			FD_CLR (sd, &perf_wfds);
			FD_SET (sd, &perf_rfds);
			if (sd >= perf_nfds)
				perf_nfds = sd + 1;
			(void) newevblk (cb -> cb_clp, cb,
					 cb -> cb_type, (time_t)0);
			break;
		}
		break;
	    case cb_idle:
		freecblk (cb);
		break;
	    case cb_proc_sent:
		PP_TRACE (("Awaiting proc response ..."));
		getresponse (cb, OK);
		break;
	    case cb_init_sent:
		PP_TRACE (("Awaiting init response"));
		getresponse (cb, OK);
		break;
	    default:
		PP_TRACE (("Funny state!"));
		break;
	}
}

static void getresponse (cb, type)
Connblk *cb;
int	type;
{
	caddr_t	out;
	struct RoSAPindication	rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject   *rop = &roi -> roi_preject;
	
	PP_TRACE (("getresponse (%s)", cb_print (cb)));

	switch (RyWait (cb -> cb_fd, &cb -> cb_id, &out, type, roi)) {
	    case NOTOK:
		if (rop -> rop_reason == ROS_TIMER)
			break;
		PP_TRACE (("RyWait returned NOTOK"));
		ros_advise (rop, "STUB");
		delay_channel (cb);
		cleanup_conn (cb);
		break;

	    case OK:
		PP_TRACE (("RyWait returned OK"));
		switch (cb -> cb_state) {
		    default:
			cb -> cb_state = cb_active;
			/* and fall */
		    case cb_finished:
			(void) newevblk (cb -> cb_clp, cb,
					 cb -> cb_type, (time_t)0);
			break;
		    case cb_proc_sent:
			break;
		    case cb_error:
			cleanup_conn (cb);
			break;
		}
		break;

	    case DONE:
		cb -> cb_ml = NULL;
		cleanup_conn (cb);
		break;
	}
}

#ifdef notdef
static void setfds ()
{
	register Connblk *cb;
	struct PSAPindication pi;

	PP_TRACE (("setfds ()"));

	FD_ZERO (&perf_wfds);
	FD_ZERO (&perf_rfds);
	perf_nfds = 0;

	for (cb = CHead -> cb_forw; cb != CHead; cb = cb -> cb_forw)
		switch (cb -> cb_state) {
		    case cb_conn_request1:
			if (PSelectMask (cb -> cb_fd, &perf_wfds,
					 &perf_nfds, &pi) == NOTOK)
				PP_LOG (LLOG_EXCEPTIONS,
					 ("PSelectMask failed"));
			break;
		    case cb_conn_request2:
		    case cb_proc_sent:
		    case cb_init_sent:
		    case cb_conn_established:
			if (PSelectMask (cb -> cb_fd, &perf_rfds,
					 &perf_nfds, &pi) == NOTOK)
				PP_LOG (LLOG_EXCEPTIONS,
					("PSelectMask failed"));
			break;
		    default:
			break;
		}
	PP_TRACE (("nrfds=%d rfds=0x%x wfds=0x%x", perf_nfds,
		 perf_rfds.fds_bits[0], perf_wfds.fds_bits[0]));
}
#endif

/* ARGSUSED */
static int  do_quit (sd, ds, args, arg)
int	sd;
struct client_dispatch *ds;
char  **args;
caddr_t	*arg;
{
	Connblk *cb;

	PP_TRACE (("do_quit(%d)", sd));

	(void) assoc_release (sd);
	if (sd != NOTOK) {
		FD_CLR (sd, &perf_wfds);
		FD_CLR (sd, &perf_rfds);
	}
	if ((cb = findcblk (sd)) != NULLCB)
		cb -> cb_fd = NOTOK;
	else
		return DONE;

	PP_TRACE (("do_quit (%s)", cb_print (cb) ));
	if (cb -> cb_type == cb_channel)
		cb -> cb_clp -> chan_update = 1;

	PP_NOTICE (("Closing channel %s", cb_print (cb)));

	return DONE;
}

/* connection block functions */

Connblk *newcblk (type)
enum	cb_type type;
{
	Connblk *cb;

	PP_TRACE (("newcblk () cnt=%d", cb_count));

	cb = (Connblk *) calloc (1, sizeof *cb);
	if (cb == NULLCB)
		return cb;

	cb -> cb_fd = NOTOK;
	cb -> cb_type = type;

	if (cb_once_only == 0) {
		CHead -> cb_forw = CHead -> cb_back = CHead;
		cb_once_only ++;
	}
	insque (cb, CHead -> cb_back);
	cb_count ++;
	return cb;
}

void freecblk (cb)
Connblk *cb;
{
	struct AcSAPindication	acis;
	struct RoSAPindication	rois;
	register struct RoSAPindication *roi = &rois;
	
	if (cb == NULLCB)
		return;

	PP_DBG (("freecblk (%s) cnt=%d",
		cb_print(cb), cb_count));

	if (cb -> cb_fd != NOTOK) {
		PP_NOTICE (("Killing channel %s",
			    cb_print (cb)));
		(void) AcUAbortRequest (cb -> cb_fd, NULLPEP, 0, &acis);
		(void) RyLose (cb -> cb_fd, roi);
		FD_CLR (cb -> cb_fd, &perf_wfds);
		FD_CLR (cb -> cb_fd, &perf_rfds);
	}
	remque (cb);

	cb_count --;
	if (cb_count < 0)
		PP_LOG (LLOG_EXCEPTIONS, ("Internal error cb_count = %d",
					  cb_count));
	free ((char *) cb);
}

Connblk *findcblk (fd)
int	fd;
{
	Connblk *cb;

	PP_DBG (("findcblk (%d) cnt=%d", fd, cb_count));

	if (cb_once_only == 0)
		return NULLCB;

	for (cb = CHead -> cb_forw; cb != CHead; cb = cb -> cb_forw)
		if (cb -> cb_fd == fd)
			return cb;

	PP_TRACE (("Can't locate block with %d (%d alloc'd)",
		fd, cb_count));
	return NULLCB;
}

static char	*cb2name (cb)
Connblk *cb;
{
	switch (cb -> cb_type) {
	    case cb_channel:
	    case cb_special:
		if (cb -> cb_clp && cb -> cb_clp -> channame)
			return cb -> cb_clp -> channame;
		else	return "unknown channel";

	    case cb_timer:
		return "timer";

	    case cb_responder:
		return "responder";

	    default:
		break;
	}
	return "something";
}

static char	*cb_print (cb)
Connblk *cb;
{
	static char buf[1024];

	switch (cb -> cb_type) {
	    case cb_channel:
	    case cb_special:
		(void) sprintf (buf, "<fd=%d chan=%s mta=%s msg=%s>",
				cb -> cb_fd,
				cb2name(cb),
				cb -> cb_mlp ? cb -> cb_mlp -> mtaname: "none",
				(cb -> cb_ml && cb -> cb_ml -> ms ) ?
				cb -> cb_ml -> ms -> queid : "none");
		break;
	    case cb_timer:
		(void) sprintf (buf, "<timer proc=0x%x reload=%d>",
				cb -> cb_proc, cb -> cb_reload);
		break;

	    case cb_responder:
		(void) sprintf (buf, "<fd=%d responder auth=%d>",
				cb -> cb_fd, cb -> cb_authenticated);
		break;
	}

	return buf;
}

static void pcblock_error (cb)
Connblk *cb;
{
	PP_OPER (NULLCP,
		 ("Channel %s in state %d taking too long - aborting rfd=%s wfd=%s nfds=%d",
		  cb_print (cb), cb -> cb_state,
		  FD_ISSET (cb->cb_fd, &perf_rfds) ? "set" : "unset",
		  FD_ISSET (cb->cb_fd, &perf_wfds) ? "set" : "unset",
		  perf_nfds
		 ));
}
	
static struct eventqueue *newevblk (clp, cb, type, when)
Chanlist *clp;
Connblk *cb;
time_t	when;
enum cb_type type;
{
	struct eventqueue *ev, *pev;

	PP_TRACE (("newevblk (clp, cb, %d, %d) cnt=%d",
		   type, when, ev_count));

	ev = (struct eventqueue *) calloc (1, sizeof *ev);
	if (ev == NULLEV) 
		return ev;

	ev_count ++;
	ev -> ev_clp = clp;
	ev -> ev_conn = cb;
	ev -> ev_type = type;

	for (pev = EHead -> ev_forw; pev != EHead; pev = pev -> ev_forw) {
		if (pev -> ev_time > when) {
			insque (ev, pev -> ev_back);
			break;
		}
		else
			when -= pev -> ev_time;
	}
	if (pev == EHead)
		insque (ev, EHead -> ev_back);
	for (; pev != EHead; pev = pev -> ev_forw)
		pev -> ev_time -= when;

	ev -> ev_time = when;

	return ev;
}

static void freevblk (ev)
struct eventqueue *ev;
{
	PP_DBG (("freevblk () cnt=%d", ev_count));

	if (ev == NULLEV)
		return;

	if (ev -> ev_forw != EHead)
		ev -> ev_forw -> ev_time += ev -> ev_time;
			
	remque (ev);

	free ((char *) ev);
	ev_count --;
	if (ev_count < 0)
		PP_LOG (LLOG_EXCEPTIONS, ("Internal error - ev_count = %d",
					  ev_count));
}

#ifdef notdef
static time_t evtimebyclp (clp)
Chanlist *clp;
{
	struct eventqueue *ev;
	time_t	now = current_time;
	
	for (ev = EHead -> ev_forw; ev != EHead; ev = ev -> ev_forw)
		if (ev -> ev_clp == clp)
			return now + ev -> ev_time;
		else now += ev -> ev_time;
	return now + MAX_SLEEP + 2;
}

static struct eventqueue *findevbyclp (clp)
Chanlist *clp;
{
	struct eventqueue *ev;

	for (ev = EHead -> ev_forw; ev != EHead; ev = ev -> ev_forw)
		if (ev -> ev_clp == clp)
			return ev;
	return NULLEV;
}
#endif

static void init_events ()
{
	PP_TRACE (("init_events ()"));

	EHead -> ev_forw = EHead -> ev_back = EHead;
}

sched_delete (rlp, ms, zapped)
Reciplist *rlp;
MsgStruct *ms;
int zapped;
{
	rlp -> status = ST_DELETE;
	if (ms -> count == 0)
		insertindelchan (ms);
	stats.n_addr_out ++;
	if (zapped)
		stats.n_messages_out ++;
}

void inc_channel (cb, number)	/* successful delivery */
Connblk *cb;
int	number;
{
	MsgStruct *ms;
	Reciplist *rlp, *rl0;
	LIST_RCHAN *lcp;
	int deleted;

	PP_TRACE (("inc_channel (%s, %d)",
		   cb_print (cb), number));

	cb -> cb_clp -> lastsuccess = current_time;
	cb -> cb_clp -> error_count = 0;
	ms = cb -> cb_ml -> ms;
	clear_msgcache (cb -> cb_ml);

	if (number == 0)	/* set real recip number */
		number = cb -> cb_ml -> recips[0] -> id;
	deleted = delfromchan (cb -> cb_clp, cb -> cb_mlp,
			       cb -> cb_ml, number);

	if (deleted & ZAP_MTA)
		cb -> cb_mlp = NULLMTALIST;

	cache_clear (&cb -> cb_clp -> cache);

	if (cb -> cb_mlp) {
		cache_clear (&cb -> cb_mlp -> cache);
		cb -> cb_mlp -> lastsuccess = current_time;
		cb -> cb_mlp -> error_count = 0;
	}

	if (ms == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't locate message!"));
		return;
	}
	for (rlp = ms  -> recips; rlp; rlp = rlp -> rp_next)
		if (rlp -> id == number)
			break;
	if (rlp == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipient %d in list!",
					  number));
		return;
	}

	switch (rlp -> status) {
	    case ST_WARNING:
		ms -> numberwarns ++;
		lcp = rlp -> cchan;
		rlp -> status = ST_NORMAL;
		if (lcp) {
			insertinchan (lcp -> li_chan, ms, rlp,
				      chan2mta (lcp -> li_chan, rlp));
			return;
		}
		break;
	    case ST_NORMAL:
		lcp = rlp -> cchan = rlp -> cchan -> li_next;
		if (lcp) {
			insertinchan (lcp -> li_chan, ms, rlp,
				      chan2mta (lcp -> li_chan, rlp));
			return;
		}
		sched_delete (rlp, ms, deleted & ZAP_MSG);
		break;

	    case ST_DR:
		for (rl0 = ms -> recips; rl0; rl0 = rl0 -> rp_next)
			if (rl0 -> id == 0)
				break;
		if ((lcp = rl0 -> cchan) == NULLIST_RCHAN) {
			PP_LOG (LLOG_EXCEPTIONS, ("no channels left for DR"));
			break;
		}
		if (lcp -> li_next == NULLIST_RCHAN) /* end of list */
			sched_delete (rlp, ms, deleted & ZAP_MSG);
		else  {
			lcp = rl0 -> cchan = lcp -> li_next;
			insertinchan (lcp -> li_chan, ms, rlp,
				      chan2mta (lcp -> li_chan, rl0));
		}
		break;
	    case ST_DELETE:
		cb -> cb_ml = NULL;
		if (ms -> count == 0)
			zapmsg (ms);
		break;
	}
}

void delay_host (cb, rno, str, first)
Connblk *cb;
int	rno;
char	*str;
int first;
{
	struct Mtalist *mlp;

	PP_TRACE (("delay_host (%s, %d)", cb_print (cb), rno));

	if (cb -> cb_ml)
		msg_unlock (cb -> cb_ml -> ms);

	if ((mlp = cb -> cb_mlp) == NULLMTALIST) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't locate mta in queue"));
		return;
	}
	if (mlp -> info) {
		free (mlp -> info);
		mlp -> info = NULLCP;
	}
	if (str)
		mlp -> info = strdup (str);
	cache_inc (&mlp -> cache, cache_time*3/2);
	mlp -> nextevent = mlp -> cache.cachetime;
	if (cb -> cb_clp) {
		remque (mlp);
		insque (mlp, cb -> cb_clp -> mtas -> mta_back);
		if (first)
			cb -> cb_clp -> error_count ++;
	}
}

void delay_message (cb, str, first)
Connblk *cb;
char	*str;
int first;
{
	Mtalist	*mlp;

	PP_TRACE (("delay_message (%s)", cb_print (cb)));

	if (cb -> cb_ml == NULL)
		return;
	if ((mlp = cb -> cb_mlp) == NULLMTALIST) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't locate mta in queue"));
		return;
	}
	if (cb -> cb_ml -> info) {
		free (cb -> cb_ml -> info);
		cb -> cb_ml -> info = NULLCP;
	}
	if (str)
		cb -> cb_ml -> info = strdup (str);
	if (first)
		cb -> cb_ml -> ms -> nerrors ++;
	msg_unlock (cb -> cb_ml -> ms);
	msgcache_inc (cb -> cb_ml, cache_time*2);
	mlp -> mta_changed = 1;
	if (first)
		mlp -> error_count ++;
	cb -> cb_clp -> chan_update = 1;
}

static void bad_recip (cb, rno)
Connblk *cb;
int	rno;
{
	Reciplist *rlp;
	MsgStruct *ms;
	LIST_RCHAN *lcp;
	int	del;

	PP_TRACE (("bad_recip (%s, %d)", cb_print (cb), rno));

	if (rno == 0)		/* set real recip number */
		rno = cb -> cb_ml -> recips[0] -> id;

	ms = cb -> cb_ml -> ms;
	for (rlp = ms -> recips; rlp; rlp = rlp -> rp_next)
		if (rlp -> id == rno)
			break;
	if (rlp == NULLRL)
		return;

	if (rlp -> status == ST_DR) { /* DR being DR'd */
		Reciplist *rl0;

		for (rl0 = ms -> recips; rl0; rl0 = rl0 -> rp_next)
			if (rl0 -> id == 0)
				break;

		if ((lcp = rl0 -> cchan) && lcp -> li_next == NULLIST_RCHAN) {
			PP_OPER (NULLCP,
				 ("A DR giving status DR on chan %s - Help!",
				  lcp -> li_chan ? lcp -> li_chan -> ch_name :
				  "<unknown>"));
			msgcache_inc (cb -> cb_ml, cache_time*2);
			cb -> cb_mlp -> mta_changed = 1;
			cb -> cb_mlp -> error_count ++;
			cb -> cb_clp -> chan_update = 1;
			return;
		}

		cb -> cb_clp -> lastsuccess = current_time;
		cb -> cb_clp -> error_count = 0;
		if (cb -> cb_mlp) {
			cb -> cb_mlp -> lastsuccess = current_time;
			cb -> cb_mlp -> error_count = 0;
		}
		del = delfromchan (cb -> cb_clp, cb -> cb_mlp, cb -> cb_ml, rno); 
		if (del & ZAP_MTA)
			cb -> cb_mlp = NULLMTALIST;

		PP_NOTICE (("Skipping over DR refromatting channels"));
		/* skip over all the bad reformatters */
		while (lcp -> li_next != NULLIST_RCHAN)
			lcp = lcp -> li_next;
		rl0 -> cchan = lcp;
		insertinchan (lcp -> li_chan, ms,
			      rlp, chan2mta (lcp -> li_chan, rl0));
	}
	else {
		cb -> cb_clp -> lastsuccess = current_time;
		cb -> cb_clp -> error_count = 0;
		if (cb -> cb_mlp) {
			cb -> cb_mlp -> lastsuccess = current_time;
			cb -> cb_mlp -> error_count = 0;
		}
		del = delfromchan (cb -> cb_clp, cb -> cb_mlp, cb -> cb_ml, rno); 
		if (del & ZAP_MTA)
			cb -> cb_mlp = NULLMTALIST;
		insertindrchan (cb -> cb_ml -> ms, rlp);
	}
}

static void sharedDR (cb, rno)
Connblk *cb;
int	rno;
{
	int del;

	PP_TRACE (("sharedDR (%s, %d)", cb_print (cb), rno));

	if (rno == 0)	/* set real recip number */
		rno = cb -> cb_ml -> recips[0] -> id;

	cb -> cb_clp -> lastsuccess = current_time;
	if (cb -> cb_mlp)
		cb -> cb_mlp -> lastsuccess = current_time;
	del = delfromchan (cb -> cb_clp, cb -> cb_mlp, cb -> cb_ml, rno);
	if (del & ZAP_MTA)
		cb -> cb_mlp = NULLMTALIST;
}

/* ARGSUSED */

static int do_processmessage (sd, ds, args, arg)
int	sd;
struct client_dispatch *ds;
char	**args;
struct type_Qmgr_ProcMsg **arg;
{
	Reciplist *rl0;
	Connblk *cb;
	Mlist *ml;
	register struct type_Qmgr_ProcMsg *pm;
	struct type_Qmgr_UserList *lusers ();
	char	*p;

	PP_TRACE (("do_processmessage (%d)", sd));

	if ((cb = findcblk (sd)) == NULLCB)
		return NOTOK;
	if (cb -> cb_state != cb_active)
		return NOTOK;
	ml = cb -> cb_ml;
	if (ml == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("Missing ml structure"));
		return NOTOK;
	}
	*arg = pm = (struct type_Qmgr_ProcMsg *) malloc (sizeof (**arg));
	
	pm -> channel = str2qb (cb -> cb_clp -> channame,
				strlen (cb -> cb_clp -> channame), 1);

	p = ml -> ms -> queid;
	pm -> qid = str2qb (p, strlen (p), 1);
	for (rl0 = ml -> ms -> recips; rl0; rl0 = rl0 -> rp_next)
		if (rl0 -> id == 0)
			break;
	if (ml -> recips[0] -> status == ST_DR &&
	    rl0 && rl0 -> cchan && rl0 -> cchan -> li_next != NULL) {
		pm -> users = (struct type_Qmgr_UserList *)
			smalloc (sizeof *pm -> users);
		pm -> users -> next = NULL;
		pm -> users -> RecipientId = (struct type_Qmgr_RecipientId *)
			smalloc (sizeof *pm -> users -> RecipientId);
		pm -> users -> RecipientId -> parm = 0;
	}
	else
		pm -> users = lusers (ml, 0);
	if (pm -> users == NULL)
		PP_LOG (LLOG_EXCEPTIONS, ("Empty user list!"));
	cb -> cb_ttl = current_time + CHAN_TIMEOUT + (ml -> ms -> size /20);
	cb -> cb_state = cb_proc_sent;
	if (cb -> cb_mlp)
		cb -> cb_mlp -> lastattempt = current_time;
	return OK;
}

/* ARGSUSED */
static int do_readqueue (sd, ds, args, arg)
int	sd;
struct client_dispatch *ds;
char	**args;
struct type_Qmgr_Pseudo__readqueue **arg;
{
	Connblk *cb;

	PP_TRACE (("do_readqueue (%d)", sd));
	if ((cb = findcblk (sd)) == NULLCB)
		return NOTOK;
	cb -> cb_state = cb_proc_sent;
	return OK;
}



/* ARGSUSED */
static int do_initchannel (sd, ds, args, arg)
int	sd;
struct client_dispatch *ds;
char **args;
struct type_Qmgr_Channel **arg;
{
	Connblk *cb;

	if ((cb = findcblk (sd)) == NULLCB)
		return NOTOK;

	PP_TRACE (("do_initchannel (%s)", cb_print (cb) ));

	*arg = str2qb (*args, strlen (*args), 1);
	cb -> cb_state = cb_init_sent;
	if (cb -> cb_clp -> chan_special)
		cb -> cb_ttl = current_time + CHAN_TIMEOUT;
	else	cb -> cb_ttl = current_time + 60;
	return OK;
}

/* ARGSUSED */
static int processmessage_result (sd, id, dummy, result, roi)
int	sd,
	id,
	dummy;
register struct type_Qmgr_DeliveryStatus *result;
struct RoSAPindication *roi;
{
	Connblk *cb;
	struct type_Qmgr_DeliveryStatus *ds;
	int	rno;
	char	buf[LINESIZE];
	char	*info;
	int first_mta = 1, first_msg = 1;

	PP_TRACE (("processmessage_result (%d)", sd));

	if ((cb = findcblk (sd)) == NULLCB) {
		PP_LOG (LLOG_EXCEPTIONS, ("No connection block for %d",
					  sd));
		return NOTOK;
	}
	PP_TRACE (("processmessage_result (%s)", cb_print (cb)));
	for (ds = result; ds; ds = ds -> next) {
		rno = ds -> IndividualDeliveryStatus -> recipient -> parm;
		(void) sprintf (buf, "Recipient ID %d %s:", rno,
				cb_print (cb));
		if (ds -> IndividualDeliveryStatus -> info)
			info = qb2str(ds -> IndividualDeliveryStatus -> info);
		else	info = NULLCP;
		switch (ds -> IndividualDeliveryStatus -> status) {
		    case int_Qmgr_status_success:
			PP_NOTICE (("%s success", buf));
			inc_channel (cb, rno);
			break;

		    case int_Qmgr_status_negativeDR:
		    case int_Qmgr_status_positiveDR:
			PP_NOTICE (("%s %s", buf,
				    ds -> IndividualDeliveryStatus -> status
				    == int_Qmgr_status_positiveDR ?
				    "positiveDR" : "negativeDR"));
			bad_recip (cb, rno);
			break;

		    case int_Qmgr_status_successSharedDR:
		    case int_Qmgr_status_failureSharedDR:
			PP_NOTICE (("%s %s", buf,
				    ds -> IndividualDeliveryStatus -> status
				    == int_Qmgr_status_failureSharedDR ?
				    "failureSharedDR" : "sucessSharedDR"));
			sharedDR(cb, rno);
			break;

		    case int_Qmgr_status_messageFailure:
			PP_NOTICE (("%s messageFailure (%s)", buf,
				    info ? info : ""));
			delay_message (cb, info, first_msg);
			first_msg = 0;
			break;

		    case int_Qmgr_status_mtaFailure:
			PP_NOTICE (("%s mtaFailure (%s)", buf,
				    info ? info : ""));
			delay_host (cb, rno, info, first_mta);
			first_mta = 9;
			break;

		    case int_Qmgr_status_mtaAndMessageFailure:
			PP_NOTICE (("%s mtaAndMessageFailure (%s)", buf,
				    info ? info : ""));
			delay_message (cb, info, first_msg);
			delay_host (cb, rno, info, first_mta);
			first_mta = first_msg = 0;
			break;

		    default:
			PP_NOTICE (("%s Unknown response", buf));
			break;
		}
		if (info)
			free (info);
	}
	cb -> cb_state = cb_active;
	/* quick attempt to fire something off now - if its easy
	 * otherwise - do it in the main loop and do more clever scheduling.
	 */
	if (cb -> cb_clp -> chan_enabled == 0 || opmode ||
	    (cb -> cb_ml = nextmsg (cb -> cb_clp, &cb -> cb_mlp, 1, 1))== NULL )
			cb -> cb_ml = NULL;
	else
		(void) chan_invoke (cb, DIS_PROC, (caddr_t *)0);
		
	return OK;
}

/*  RPC procedures... */

/* ARGSUSED */
static int readqueue_result (sd, id, dummy, result, roi)
int	sd,
	id,
	dummy;
register struct type_Qmgr_MsgList *result;
struct RoSAPindication *roi;
{
	struct type_Qmgr_MsgStructList *ml;
	MsgStruct *ms, *oldms;
	char	*p;
	int	count;
	Connblk *cb;
	struct type_Qmgr_QidList *ql;

	PP_TRACE (("readqueue_result (%d, %d)", sd, id));

	if ((cb = findcblk (sd)) == NULLCB)
		return NOTOK;
	cb -> cb_state = cb_active;
	for (count = 0,ml = result -> msgs; ml; ml = ml -> next) {
		ms = newmsgstruct (ml -> MsgStruct);
		if ((oldms = find_msg (ms -> queid)) != NULLMS) {
			(void) updatemessage (oldms, ms);
			freems (ms);
		}
		else
			(void) insertmessage (ms);
		count ++;
	}
	for (ql = result -> deleted; ql; ql = ql -> next) {
		p = qb2str (ql -> QID);
		if ((ms = find_msg (p)) != NULLMS)
			kill_msg (ms);
		free (p);
	}
	if (count == 0)
		cb -> cb_state = cb_finished;
	cb -> cb_ttl = current_time + CHAN_TIMEOUT;
	cb -> cb_clp -> lastsuccess = current_time;
	return OK;
}

/* ARGSUSED */
static int channelinit_result (sd, id, dummy, result, roi)
int	sd,
	id,
	dummy;
struct type_Qmgr_Pseudo__channelInitialise *result;
struct RoSAPindication *roi;
{
	PP_TRACE (("channelinit_result (%d, %d)", sd, id));

	return OK;
}
	
/* ARGSUSED */
static int general_error (sd, id, error, parameter, roi)
int	sd,
	id,
	error;
caddr_t	parameter;
struct RoSAPindication *roi;
{
	register struct RyError *rye;
	Connblk *cb;

	if ((cb = findcblk (sd)) == NULLCB)
		return NOTOK;
	PP_TRACE (("general_error (%s)", cb_print(cb)));
	if (cb -> cb_type != cb_channel) {
		PP_LOG (LLOG_EXCEPTIONS, ("Not a channel!!!"));
		return NOTOK;
	}

	cb -> cb_state = cb_error;
	if (error == RY_REJECT) {
		PP_LOG (LLOG_EXCEPTIONS,
			("%s", RoErrString ((int) parameter)));
		delay_channel (cb);
		return NOTOK;
	}

	if ((rye = finderrbyerr (table_Qmgr_Errors, error)) != NULL)
		PP_LOG (LLOG_EXCEPTIONS, ("%s", rye -> rye_name));
	else
		PP_LOG (LLOG_EXCEPTIONS, ("Error %d", error));

	delay_channel (cb);
	return NOTOK;
}

/* ARGSUSED */
static int processmessage_error (sd, id, error, parameter, roi)
int	sd,
	id,
	error;
caddr_t	parameter;
struct RoSAPindication *roi;
{
	register struct RyError *rye;
	Connblk *cb;
	char	tbuf[128];

	if ((cb = findcblk (sd)) == NULLCB)
		return NOTOK;
	PP_TRACE (("processmessage_error (%s)", cb_print(cb)));

	if (cb -> cb_type != cb_channel) {
		PP_LOG (LLOG_EXCEPTIONS, ("Not a channel!!!"));
		return NOTOK;
	}

	if (error == RY_REJECT) {
		PP_LOG (LLOG_EXCEPTIONS,
			("%s", RoErrString ((int) parameter)));
		delay_message (cb, RoErrString ((int) parameter), 1);
		return OK;
	}

	if ((rye = finderrbyerr (table_Qmgr_Errors, error)) != NULL)
		(void) sprintf (tbuf, "process message error %s",
				rye -> rye_name);
	else
		(void) sprintf (tbuf, "process message error %d", error);
	PP_LOG (LLOG_EXCEPTIONS, ("%s", tbuf));

	delay_message (cb, tbuf, 1);
	return OK;
}

void delay_channel (cb)
Connblk *cb;
{
	PP_TRACE (("delay_channel (%s)", cb_print (cb) ));

	if (cb -> cb_type == cb_responder || cb -> cb_type == cb_timer)
		return;
	if (cb -> cb_clp == NULL)
		return;
	cache_inc (&(cb -> cb_clp -> cache), cache_time);
	cb -> cb_clp -> nextevent = cb -> cb_clp -> cache.cachetime;
}

void investigate_chan (clp, now)
Chanlist *clp;
time_t	now;
{
	Mtalist *mlp;
	int nmtas = clp -> nmtas;

	PP_TRACE (("investigate_chan (%s)", clp -> channame));

	clp -> nextevent = now + MAX_SLEEP;
	clp -> oldest = now;
	clp -> nmtas = 0;

	for (mlp = clp -> mtas -> mta_forw; mlp != clp -> mtas;
	     mlp = mlp -> mta_forw) {
		if (mlp -> mta_changed || mlp -> nextevent < current_time) {
			investigate_mta (mlp, now);
			mlp -> mta_changed = 0;
		}
		if (mlp -> oldest < clp -> oldest)
			clp -> oldest = mlp -> oldest;
		clp -> nmtas ++;
		if (!mtaready (mlp))
			continue;
		if (mlp -> nextevent < clp -> nextevent)
			clp -> nextevent = mlp -> nextevent;
	}
	if (clp -> chan_special)
		clp -> nextevent = now;
	if (clp -> cache.cachetime > now)
		clp -> nextevent = clp -> cache.cachetime;
	if (clp -> chan_enabled == 0)
		clp -> nextevent = now + MAX_SLEEP;
	if (nmtas != clp -> nmtas)
		PP_LOG(LLOG_EXCEPTIONS, ("mta count mismatch on %s (%d != %d)",
					 clp -> channame,
					 clp -> nmtas, nmtas));
}

void investigate_mta (mlp, now)
Mtalist *mlp;
time_t	now;
{
	Mlist	*ml;
	time_t cachet;

	PP_TRACE (("investigate_mta (%s)", mlp -> mtaname));

	mlp -> nextevent = now + MAX_SLEEP + 1;
	mlp -> oldest = now;

	for (ml = mlp -> msgs -> ml_forw; ml != mlp -> msgs;
	     ml = ml -> ml_forw) {
		/* this calc has to be done  - or we could optimise */
		if (ml -> ms -> age < mlp -> oldest)
			mlp -> oldest = ml -> ms -> age;

		if (!msgready (ml))
			continue;

		if ((cachet = msgmincache (ml)) > now) {
			if (cachet < mlp -> nextevent)
				mlp -> nextevent = cachet;
		}
		else if (ml -> ms -> defferedtime &&
			 ml -> ms -> defferedtime < mlp -> nextevent)
			mlp -> nextevent = ml -> ms -> defferedtime;
		else mlp -> nextevent = now;
	}
	if (mlp -> cache.cachetime > now) 	/* oops - cached */
		mlp -> nextevent = mlp -> cache.cachetime;
	if (mlp -> mta_enabled == 0)
		mlp -> nextevent = now + MAX_SLEEP + 1;
}

void cleanup_conn (cb)
Connblk *cb;
{
	PP_TRACE (("cleanup_conn(%s)", cb_print (cb)));

	switch (cb -> cb_type) {
	    case cb_responder:
	    case cb_timer:
		break;
	    default:
		if (cb -> cb_clp) {
			if (cb -> cb_mlp) {
				cb -> cb_mlp -> nactive --;
				if (cb -> cb_state == cb_idle ||
				    cb -> cb_state == cb_proc_sent)
					delay_message (cb, "Connection broke", 1);
			}
			if (cb -> cb_state != cb_idle) {
				cb -> cb_clp -> nactive --;
				nchansrunning --;
			}
			if (cb -> cb_clp -> chan_special) {
				nspecials --;
				if (cb -> cb_clp -> chan -> ch_chan_type ==
				    CH_QMGR_LOAD)
					if (delete_chan -> chan_enabled == 0 &&
					    delete_chan -> chan_syssusp == 1) {
						delete_chan -> chan_enabled = 1;
						delete_chan -> chan_syssusp = 0;
						delete_chan -> chan_update = 1;
					}

			}
		}
		if (cb -> cb_ml)
			msg_unlock (cb -> cb_ml -> ms);
			
		break;
	}
	freecblk (cb);
}

static int timeout_proc ()
{
	MsgStruct **msp, *ms;
	time_t warn_t;

	PP_TRACE (("timeout_proc"));
	for (msp = msg_hash; msp < &msg_hash[QUE_HASHSIZE]; ) {
		for (ms = *msp; ms; ms = ms -> ms_forw) {
			if (ms -> m_locked)
				continue;
			if (ms -> expirytime < current_time &&
			    expiremsg (ms) != NOTOK)
				continue;

			if (ms -> numberwarns >= warn_number)
				continue;

			warn_t = ms -> age + (ms -> warninterval *
					      (ms -> numberwarns+1));
			if (warn_t < current_time)
				(void) warnmsg (ms);
		}
		if (!ms)
			msp ++;
	}
}

static int expiremsg (ms)
MsgStruct *ms;
{
	int first = 1;
	Reciplist *rlp;
	Chanlist *clp;
	Mtalist *mlp;
	Mlist *ml;

	PP_TRACE (("expiremsg (%s)", ms -> queid));
	if (timeout_chan == NULLCHANLIST  || timeout_chan -> chan_enabled == 0)
		return NOTOK;

	for (rlp = ms -> recips; rlp; rlp = rlp -> rp_next) {
		if (rlp -> id == 0)
			continue;
		switch (rlp -> status) {
		    case ST_DR:
		    case ST_DELETE:
		    case ST_TIMEOUT:
		    case ST_WARNING:
			continue; /* leave these alone! */

		    case ST_NORMAL:
			if ((ml = rlp -> ml) == NULLMLIST) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Recipient not on msg q"));
				continue;
			}
			if ((mlp = ml -> mlp) == NULLMTALIST) {
				PP_LOG (LLOG_EXCEPTIONS,
					("msg q not on mta"));
				continue;
			}
			if ((clp = mlp -> clp) == NULLCHANLIST) {
				PP_LOG (LLOG_EXCEPTIONS,
					("mta not on chan"));
				continue;
			}
			(void) delfromchan (clp, mlp, ml, rlp -> id);
			rlp -> status = ST_TIMEOUT;
			if (first) {
				insertinchan (timeout_chan -> chan, ms, rlp,
					      chan2mta(timeout_chan -> chan,
						       rlp));
				first = 0;
			}
		}
	}
	return first == 1 ? OK : DONE;
}

static int warnmsg (ms)
MsgStruct *ms;
{
	int first = 1;
	Reciplist *rlp;
	Chanlist *clp;
	Mtalist *mlp;
	Mlist *ml;

	PP_TRACE (("warnmsg (%s)", ms -> queid));

	if (warn_chan == NULLCHANLIST || warn_chan -> chan_enabled == 0)
		return NOTOK;
	for (rlp = ms -> recips; rlp; rlp = rlp -> rp_next) {
		if (rlp -> id == 0)
			continue;
		switch (rlp -> status) {
		    case ST_DR:
		    case ST_DELETE:
		    case ST_TIMEOUT:
		    case ST_WARNING:
			continue; /* leave these alone! */

		    case ST_NORMAL:
			if ((ml = rlp -> ml) == NULLMLIST) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Recipient not on msg q"));
				continue;
			}
			if ((mlp = ml -> mlp) == NULLMTALIST) {
				PP_LOG (LLOG_EXCEPTIONS,
					("msg q not on mta"));
				continue;
			}
			if ((clp = mlp -> clp) == NULLCHANLIST) {
				PP_LOG (LLOG_EXCEPTIONS,
					("mta not on chan"));
				continue;
			}
			(void) delfromchan (clp, mlp, ml, rlp -> id);
			rlp -> status = ST_WARNING;
			insertinchan (warn_chan -> chan, ms, rlp,
				      chan2mta(warn_chan -> chan,
					       rlp));
			first = 1;
		}
	}
	return first == 1 ? OK : DONE;
}

static int channel_ttl ()
{
	struct AcSAPindication	acis;
	struct RoSAPindication	rois;
	register struct RoSAPindication *roi = &rois;
	Connblk *cb;

	PP_TRACE (("channel_ttl ()"));

	if (cb_once_only == 0)
		return OK;

	for (cb = CHead -> cb_forw; cb != CHead; cb = cb -> cb_forw) {
		if (cb -> cb_type == cb_channel && cb -> cb_ttl &&
		    cb -> cb_ttl < current_time) {
			pcblock_error (cb);
			(void) AcUAbortRequest (cb -> cb_fd, NULLPEP,
						0, &acis);
			(void) RyLose (cb -> cb_fd, roi);
			(void) chan_lose (cb -> cb_fd,
					  (struct AcSAPfinish *)0);
		}
	}
	timer_running = 0;
	if (nchansrunning > 0)
		start_timer ();
	return OK;			
}

static void start_timer ()
{
	Connblk *cb;

	if (timer_running)
		return;

	cb = newcblk (cb_timer);
	cb -> cb_proc = channel_ttl;
	cb -> cb_reload = 0;
	(void) newevblk (NULLCHANLIST, cb, cb_timer, CHAN_TIMEOUT);
	timer_running = 1;
}
