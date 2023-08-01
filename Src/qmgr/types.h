/* types.h: queue manager general purpose type definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/types.h,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: types.h,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_QMGR_TYPES
#define _H_QMGR_TYPES


#include <stdio.h>
#include "Qmgr-types.h"
#include "table.h"
#include "chan.h"
#include "list_bpt.h"
#include "list_rchan.h"
#include "mta.h"


#define MTA_HASHSIZE	503
#define QUE_HASHSIZE	1009
#define DISP_INDENT	2


/* how often to reload from disk */
#define LOAD_RETRY_INTERVAL	((time_t)(6 * 60 * 60))	/* every hour or so */
/* how often to rerun trash channel */
#define TRASH_RETRY_INTERVAL	((time_t)(3 * 60 * 60)) /* every 3 hours */
/* first timeout sweep of queues */
#define TIMEOUT_RETRY_INTERVAL	((time_t)(12 * 60 * 60)) /* every 12 hours */
/* A constant - no channel allowed to sleep longer than this */
#define MAX_SLEEP		((time_t)(1 * 24 * 60 * 60)) /* 1 day */
/* kill a channel after this time if it doesn't appear to be working */
#define CHAN_TIMEOUT		((time_t)(60 * 15))	/* 15 mins */
/* default number of simultaneous channels */
#define MAXCHANSRUNNING		3



/* --- Cache structure - general purpose. --- */
typedef struct cache {
	time_t		cachetime;
	int		cacheplus;
#define	CACHE_TIME		(5*60)	/* 5 minutes retry time */
#define CACHE_MAX		(10)	/* 50 mins max */
} Cache;



/* --- Recipient list - internal form --- */
typedef struct Reciplist {
	struct Reciplist *rp_next;
	
	int		id;
	char		*user;
	char		*mta;
	LIST_RCHAN	*chans;
	LIST_RCHAN	*cchan;

	/* --- non protocol stuff --- */
	unsigned int	status:4,
#define ST_NORMAL	0
#define ST_DR		1
#define ST_DELETE	2
#define ST_TIMEOUT	3
#define ST_WARNING	4
    			msg_enabled:1,
			msg_locked:1;
	Cache		cache;
	struct Mlist	*ml;	/* back pointer */
} Reciplist;
#define NULLRL		((Reciplist *)0)
	


/* --- Msg information - internal form --- */
typedef struct MsgStruct {
	struct MsgStruct	*ms_forw;

	char			*queid;
	MPDUid			*mpduid;
	char			*originator;
	char			*contenttype;
	LIST_BPT		*eit;
	int			size;
	time_t			age;
	int			warninterval;
	int			numberwarns;
	time_t			expirytime;
	time_t			defferedtime;
	char			*uacontent;
	unsigned int		m_locked:1,
				m_timeout:1,
				priority:4,
				count:10;
    	unsigned short		nerrors;
	CHAN			*inchan;
	Reciplist		*recips;
} MsgStruct;
#define NULLMS ((MsgStruct *)0)




/* --- Msg list - as associated with an MTA --- */
typedef struct Mlist {
	struct Mlist		*ml_forw;
	struct Mlist		*ml_back;

	MsgStruct		*ms;
	char			*info;
	Reciplist 		**recips;
	int			rcount;
	struct Mtalist		*mlp;
} Mlist;
#define NULLMLIST	((Mlist *)0)



/* --- List of MTA's associated with channel --- */
typedef struct Mtalist {
	struct Mtalist		*mta_forw;
	struct Mtalist		*mta_back;

	struct Mtalist		*hash_next;
	char			*mtaname;
	Cache			cache;
	time_t			oldest;
	time_t			lastattempt;
	time_t			lastsuccess;
	time_t			nextevent;
	int			num_msgs;
	int			num_drs;
	long			volume;
	char			*info;
	int			error_count;
	unsigned int		mta_enabled:1,
				mta_changed:1,
    				mta_waiting:1,
				nactive:12;
	struct chanlist		*clp;
	Mlist			*lastone;
	Mlist			*msgs;
} Mtalist;
#define NULLMTALIST	((Mtalist *)0)



/* --- Channel list --- */
typedef struct chanlist {
	struct chanlist *cl_forw;
	struct chanlist *cl_back;

	Mtalist		*mtas;
	Mtalist		**mta_hash;

	CHAN		*chan;
	char		*channame;
	Cache		cache;
	time_t		lastattempt;
	time_t		lastsuccess;
	time_t		laststart;
	time_t		oldest;
	time_t		nextevent;
	float		averaget;
	int		num_msgs;
	int		num_drs;
	int		volume;
	int		nactive;
	int		nmtas;
	int		error_count;
	unsigned int
			chan_special:1,	/* special system channel */
			chan_enabled:1,	/* enabled? */
			chan_syssusp:1,	/* system disabled this channel */
			chan_update:1; /* channel needs looking at */
} Chanlist;
#define NULLCHANLIST ((Chanlist *)0)




/* --- Internal 'compiled' filter type --- */
typedef struct Filter {
	char		*cont;
	LIST_BPT	*eit;
	int		priority;
	time_t		morerecent;
	time_t		earlier;
	int		maxsize;
	char		*orig;
	char		*recip;
	CHAN		*channel;
	char		*mta;
	char		*queid;
	MPDUid		*mpduid;
	char		*uacontent;
	struct	Filter	*next;
} Filter;
#define NULLFL	((Filter *)0)

#define ZAP_MSG		01
#define ZAP_MTA		02
#define ZAP_NONE	00

enum cb_type {
	cb_channel,
	cb_special,
	cb_responder,
	cb_timer
} cb_type;



struct connblk {
	struct connblk	*cb_forw;
	struct connblk	*cb_back;

	int		cb_fd;
	int		cb_id;
	int 		cb_nops;
	enum 		cb_type cb_type;

	union {
		struct {
			Chanlist	*cb_un_clp;
			Mtalist		*cb_un_mlp;
			Mlist		*cb_un_ml;
			time_t		cb_un_ttl;
			enum {
				cb_idle = 0,
				cb_conn_request1,
				cb_conn_request2,
				cb_conn_established,
				cb_init_sent,
				cb_active,
				cb_proc_sent,
				cb_close_sent,
				cb_finished,
				cb_error
				} cb_un_state;
		} cb_chan;
#define cb_state	un.cb_chan.cb_un_state
#define cb_mlp		un.cb_chan.cb_un_mlp
#define cb_ml		un.cb_chan.cb_un_ml
#define cb_clp		un.cb_chan.cb_un_clp
#define cb_ttl		un.cb_chan.cb_un_ttl
		struct {
			IFP cb_un_proc;
			time_t	cb_un_reload;
		} cb_timer;
#define cb_proc		un.cb_timer.cb_un_proc
#define cb_reload	un.cb_timer.cb_un_reload
		struct {
			int cb_un_authenticated;
		} cb_responder;
#define cb_authenticated un.cb_responder.cb_un_authenticated
	} un;
};
#define MIN_OPS 20
typedef struct connblk Connblk;
#define	NULLCB	((Connblk *)0)


void	freecblk();
Connblk *findcblk(), *newcblk();

struct stats {
	time_t boottime;	/* when we started */
	int n_messages_in;	/* total messages in */
	int n_messages_out;	/* total messages out */
	int n_addr_in;		/* total addresses in */
	int n_addr_out;		/* total addresses out */
	double ops_sec;		/* qmgr ops/sec */
	double runnable_chans;	/* runnable qmgr chans */
	double msg_sec_in;	/* msg/sec inbound */
	double msg_sec_out;	/* msg/sec outbound */
};

/* --- misc external things --- */
extern char		*myname;
extern MsgStruct	*msg_hash[];
extern Chanlist		**chan_list;
extern int		nchanlist;
extern Chanlist		*delete_chan, *loader_chan, *trash_chan,
	*timeout_chan, *warn_chan;
extern time_t		current_time;
extern time_t		debris_time, load_time, cache_time, timeout_time;
extern int		maxchansrunning, submission_disabled, opmode;
extern struct stats stats;

#define OP_SHUTDOWN	1
#define OP_RESTART	2

extern void		advise(), adios(), acs_advise(), 
			ros_advise(), ros_adios();
extern time_t		time(), utc2time_t(), utcqb2time_t();
extern char		*chan2mta();

#ifdef SYS5
#define srandom(x) srand((x))
#define random() rand()
#endif

#endif
