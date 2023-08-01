/* console.h: include file for MTA console */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/console.h,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: console.h,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_MTACONSOLE_CONSOLE
#define _H_MTACONSOLE_CONSOLE


#include       	"util.h"
#include	<isode/cmd_srch.h>
#include 	"Qmgr-types.h"
#include	<X11/Intrinsic.h>

#include	<X11/StringDefs.h>
#include	<X11/Shell.h>

#include	<X11/Xaw/Form.h>
#include	<X11/Xaw/Command.h>
#include	<X11/Xaw/Viewport.h>
#include	<X11/Xaw/Box.h>
#include	<X11/Xaw/Label.h>
#include	<X11/Xaw/List.h>
#include	<X11/Xaw/Text.h>
#include	<X11/Xaw/AsciiText.h>
#include	<X11/Xaw/Paned.h>
#include	<X11/Xaw/StripChart.h>

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
				deltaMessages,
				numberReports,
				deltaReports,
				deltaVolume,
	 			volumeMessages,
				numberActiveProcesses;
	struct procStatus	*status;
	int			priority;
	struct mta_struct	**mtalist;
	int			num_mtas;
	int			given_num_mtas;
	int			deltaMtas;
	int			display_num;
	int			inbound;
	int			outbound;
	int			chantype;
	int			maxprocs;
	struct tailor		*tai;
} Chan_struct;


typedef struct mta_disp_struct {
	Widget			widget;
	struct mta_struct 	*mta;
} Mta_disp_struct;



typedef struct msg_disp_struct {
	Widget			widget;
	struct msg_struct 	*msg;
} Msg_disp_struct;



typedef struct monitor_item {
	Widget			form,
				chan,
				box;
	Mta_disp_struct		**mtas;
	int			num_allocd; /* num allocd for mtas */
	int			num_mtas;  /* num of mtas actual being displayed */
	struct chan_struct	**channel; /* actual  being displayed */
} Monitor_item;



typedef struct color_item {
	int			badness;
	XColor  		colour;
} Color_item;



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



typedef struct popup_tuple {
	Widget			label,
				text;
} Popup_tuple;



typedef struct popup_menu {
	Widget			popup,
				form;
	int			numberOftuples,
				selected;
	Operations		op;
	Popup_tuple		*tuple;
} Popup_menu;



/* -- defines for config --- */
#define REFRESH			0
#define INACTIVE		1
#define START			2
#define BACKOFF			3
#define CONNECTMAX		4
#define PERCENT			5
#define MINBADMTA		6
#define LINEMAX			7


typedef enum			{ percentage, line, all, chanonly } Heuristic;
typedef enum			{ notconnected, connecting, connected} State;
typedef enum			{ limited, full} Authentication;
typedef enum			{ inactive, green, yellow, red} Colour;
typedef enum			{ control, monitor} Mode;


#define APPLICATION_CLASS	"Mtaconsole"
#define	MAX_EDIT_STRING		100
#define	NO_CONNECTION		"Unconnected"



typedef struct server_dispatch {
	char			*ds_name;
	int			ds_operation;
	IFP			ds_vector;
} Server_dispatch;



typedef struct client_dispatch {
    char			*ds_name;
    int				ds_operation;
    IFP				ds_argument;
    modtyp			*ds_fr_mod;
    int				ds_fr_index;
    IFP				ds_result;
    IFP				ds_error;
    char			*ds_help;
} Client_dispatch;


#define	PERCENT_BASED		"percent based"
#define LINE_BASED		"line based"
#define ALL_BASED		"all"
#define CHANONLY_BASED		"channel only"

#endif
