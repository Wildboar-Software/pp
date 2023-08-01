/* qmgr-int.h: interface to qmgr for consoles */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/qmgr-int.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: qmgr-int.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */

typedef struct bindResult {
	int	access;
#define	FULL_ACCESS	1
#define LIMITED_ACCESS	2

	char	*info;
	char	*version;
} BindResult;
void free_BindResult ();

typedef struct qmgrStatus {
	time_t	boottime;
	int	messagesIn;
	int	messagesOut;
	int	addrIn;
	int	addrOut;
	int	opsPerSec;
	int	runnableChans;
	int	msgsInPerSec;
	int	msgsOutPerSec;
	int	maxChans;
	int	currChans;
	int	totalMsgs;
	int	totalVolume;
	int	totalDRs;
} QmgrStatus;

#define free_QmgrStatus(x)	free((char *)x)

typedef	struct procStatus {
	char	enabled;	/* boolean TRUE or FALSE */
	time_t	lastAttempt,
		cachedUntil,
		lastSuccess;
} ProcStatus;

extern ProcStatus *convert_ProcStatus();
#define free_ProcStatus(x)	free((char *)x)

typedef struct channelinfo_struct {
	struct channelinfo_struct	*next;

	int			priority;

	char			*channelname,
				*channeldescrip;
	time_t			oldestMessage;
	int			numberMessages;
	int			volumeMessages;
	int			numberActiveProcesses;
	ProcStatus		*status;
	int			numberReports;

	int			inbound;
	int			outbound;

	int			chantype;

	int			maxprocs;
	int			numberMtas;
} ChannelInfo;

typedef struct mtainfo_struct {
	struct mtainfo_struct	*next;
	
	int			priority;

	char			*channel;
	char			*mta;
	time_t			oldestMessage;
	int			numberMessages;
	int			volumeMessages;
	ProcStatus		*status;
	int			numberReports;
	int			active;
	char			*info;
} MtaInfo;

typedef struct strlist_struct {
	struct strlist_struct 	*next;

	char			*str;
} Strlist;
	

typedef struct recipinfo_struct {
	struct recipinfo_struct	*next;

	char			*user;
	int			id;
	char			*mta;
	Strlist			*channels;
	int			channelsDone;
	ProcStatus		*status;
	char			*info;
} RecipInfo;

typedef struct permsginfo_struct {
	char	*qid;
	
	/* mpduid */
	char	*country;
	char	*admd;
	char	*prmd;
	char	*local;

	char	*originator;
	char	*contenttype;
	Strlist	*eits;
	int	priority;
	int	size;
	time_t	age;
	int	warnInterval;
	int	numberWarningsSent;
	time_t 	expiryTime;
	time_t	deferredTime;
	char	*uaContentId;
	int	errorCount;
	char	*inChannel;
} PerMessageInfo;

typedef struct msginfo_struct {
	struct msginfo_struct	*next;

	PerMessageInfo		*permsginfo;
	
	RecipInfo		*recips;
} MsgInfo;

typedef struct msglist_struct {
	MsgInfo		*msgs;
	Strlist		*deleted;
} MsgList;


typedef struct measureInfo {
	struct measureInfo *next;

	char	*key;
	double	ub_number, 
		ub_volume,
		ub_age,
		ub_last;
} MeasureInfo;

#define	CTRL_ENABLE	"enable"
#define CTRL_DISABLE	"disable"
#define CTRL_CACHECLEAR	"cacheclear"
#define CTRL_CACHEADD	"cacheadd"
	
#define QCTRL_ABORT	"abort"
#define QCTRL_GRACEFUL	"graceful"
#define QCTRL_RESTART	"restart"
#define QCTRL_REREAD	"reread"
#define QCTRL_DISSUB	"disablesubmission"
#define QCTRL_ENASUB	"enablesubmission"
#define QCTRL_DISALL	"disableall"
#define QCTRL_ENAALL	"anableall"
#define QCTRL_INCMAX	"increasemax"
#define QCTRL_DECMAX	"decreasemax"

#define	QFILTER_CONTENT		"contenttype"
#define	QFILTER_EIT		"eits"
#define	QFILTER_PRIO		"priority"
#define	QFILTER_MORERECENT	"morerecentthan"
#define	QFILTER_EARLIER		"earlierthan"
#define	QFILTER_MAXSIZE		"maxsize"
#define	QFILTER_ORIG		"originator"
#define	QFILTER_RECIP		"recipient"
#define	QFILTER_CHANNEL		"channel"
#define	QFILTER_MTA		"mta"
#define	QFILTER_QUEUEID		"queueid"
#define	QFILTER_MPDU		"mpduidentifier"
#define	QFILTER_UA		"uacontentid"

extern time_t convert_time ();
