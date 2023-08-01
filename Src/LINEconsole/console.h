/* console.h: include files and typedefs for lineconsole */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/console.h,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: console.h,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 *
 */
#ifndef _H_MTACONSOLE_CONSOLE
#define _H_MTACONSOLE_CONSOLE

#include	"head.h"
#include	"sys.file.h"
#include	"qmgr.h"
#include	<varargs.h>
#include	<isode/cmd_srch.h>

typedef enum {
	Unknown, Ambiguous,
	Quit, Connect, Disconnect, Heuristics, Quecontrol,
	Up, Down, Status, Set, Current, Refresh,
	List, Enable, Disable, Delay, Clear, Info,
	BelowClear, AboveClear, Next, Previous,
} Command;

typedef enum { 
		unknown, 
		quit, connect, disconnect, quecontrol, qmgrStatus,
		chanread, chanstop, chanstart, 
		chancacheadd, chandownforce, chanclear, chaninfo,
		channext, chanprev,
		mtaread, mtastop, mtastart, 
		mtacacheadd, mtaforce, mtadownforce, mtaclear, mtainfo,
		mtanext, mtaprev,
		readchannelmtamessage, msgstop, msgstart, 
		msgcacheadd, msgforce, msgclear, msginfo,
		msgnext, msgprev,
		mtacontrol
} Operations;

typedef enum {
	top, 
	channel,
	mta,
	msg
} Level;


typedef struct client_dispatch {
    char			*ds_name;
    int				ds_operation;
    IFP				ds_argument;
#ifdef PEPSY_VERSION
    modtyp			*ds_fr_mod;
    int				ds_fr_index;
#else
    IFP				ds_free;
#endif
    IFP				ds_result;
    IFP				ds_error;
    char			*ds_help;
} Client_dispatch;

typedef	struct procStatus {
	char	enabled;	/* boolean TRUE or FALSE */
	time_t	lastAttempt,
		cachedUntil,
		lastSuccess;
} ProcStatus;

typedef struct tailor {
	char	*key;
	double	ub_number,
		ub_volume,
		ub_age,
		ub_last;
	struct tailor *next;
} TailorInfo;

/* 4 factors (num, vol, age, last) == max 400 % */
#define max_bad_channel		400

/* 4 factors (num, vol, age, last) == max 400 % */
#define max_bad_mta		400

/* 2 factors (vol, age) == max 200 % */
#define max_bad_msg		200

typedef struct permsginfo {
	char			*queueid;
	char			*originator;
	char			*contenttype;
	char			*eit;
	int			priority;
	int			size;
	time_t			expiryTime;
	time_t			deferredTime;
	time_t			age;
	int			errorCount;
	char			*inChannel;
	char			*uaContentId;
} Permsginfo;



typedef	 struct recip {
	int			id;
	char			*recipient;
	char			*mta;
	char			*actChan;
	char			*chansOutstanding;
	struct procStatus	*status;
	char			*info;
	struct recip 		*next;
} Recip;



typedef struct msg_struct {
	struct permsginfo 	*msginfo;
	struct recip		*reciplist;
	struct tailor		*tai;
} Msg_struct;



typedef struct mta_struct {
	char			*mta;
	time_t			oldestMessage;
	int			numberMessages,
				numberReports,
				volumeMessages;
	struct procStatus	*status;
	int			priority;
	char			active;
	char			*info;
	struct tailor		*tai;
} Mta_struct;



typedef struct chan_struct {
	char			*channelname,
				*channeldescrip;
	time_t			oldestMessage;
	int			numberMessages,
				numberReports,
	 			volumeMessages,
				numberActiveProcesses;
	struct procStatus	*status;
	int			priority;
	struct mta_struct	**mtalist;
	int			num_mtas;
	int			given_num_mtas;
	int			display_num;
	int			inbound;
	int			outbound;
	int			chantype;
	int			maxprocs;
	struct tailor		*tai;
} Chan_struct;

extern void openpager(), closepager(), update_status();
extern char *command2str(), *lowerfy();
extern Command get_command();

#define ssformat	"%-*s %s"
#define sdformat	"%-*s %s"
#define	plus_ssformat	"%s\n%-*s %s"
#define plus_sdformat	"%s\n%-*s %d"

#define	tab		30

#endif
