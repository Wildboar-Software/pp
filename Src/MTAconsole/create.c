/* create.c: creation for all X widgets */
# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/create.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/create.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: create.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"

#define	TOTAL_WIDTH	600
#define VP_HEIGHT	120

static void create_commandwidgets();
static void create_channelwidgets();
static void create_mtawidgets();
static void create_msgwidgets();
static void create_controlwidgets();
static void create_monitorwidgets();
static void create_popupwidgets();
extern char	*myname;
Widget		top,
		mainform,
		topform,
		nextform,
		mainvpane,
		channel_form,
		mta_form,
		msg_form,
		switchform,
		header = NULL,
		error = NULL,
		statistics = NULL,
		monitor_viewport,
		control_form;
int		errorUp = 0;

extern char	*ppversion;
extern void client_msg_handler();
extern XFontStruct	*normalFont;
int		strlen_msgall;

Arg	argList[10];

/* ARGSUSED */
create_widgets(display, context)
Display		*display;
XtAppContext	context;
{
	Dimension	ht;

	XtSetArg(argList[0], XtNfont, normalFont);

	XtAddEventHandler(top, NoEventMask, TRUE, client_msg_handler, NULL);
	
	mainform = XtVaCreateManagedWidget("mainform",
				formWidgetClass,
				top,
				NULL);

	mainvpane = XtVaCreateManagedWidget("mainvpane",
				 panedWidgetClass,
				 mainform,
				 NULL);
				 
	create_commandwidgets();
	create_channelwidgets();

	switchform = XtVaCreateManagedWidget("switchForm",
				  formWidgetClass,
				  mainvpane,
				  NULL);
	create_controlwidgets();
	create_popupwidgets(context);
	XtRealizeWidget(top);
	XtVaGetValues(topform,
		  XtNheight,	&ht,
		  NULL);
	XtVaSetValues(topform,
		  XtNmin,	ht,
		  XtNmax,	ht,
		  NULL);
	XtVaGetValues(nextform,
		  XtNheight,	&ht,
		  NULL);
	XtVaSetValues(nextform,
		  XtNmin,	ht,
		  XtNmax,	ht,
		  NULL);
	XtVaGetValues(channel_form,
		  XtNheight,	&ht,
		  NULL);
	XtVaSetValues(channel_form,
		  XtNmin,	ht,
		  XtNmax,	ht,
		  NULL);
	XtVaGetValues(mta_form,
		  XtNheight,	&ht,
		  NULL);
	XtVaSetValues(mta_form,
		  XtNmin,	ht,
		  XtNmax,	ht,
		  NULL);
	XtVaGetValues(msg_form,
		  XtNheight,	&ht,
		  NULL);
	XtVaSetValues(msg_form,
		  XtNmin,	ht,
		  XtNmax,	ht,
		  NULL);
	create_monitorwidgets();
	XtInstallAllAccelerators(mainform, mainform);
	MapButtons(False); /* can't see before connect */
	MapVolume(False);
}

Widget		quit_command,
		connect_command,
		refresh_command,
		mode_command,
		config_command,
		qcontrol_command,
		total_volume_label,
		total_number_label,
  		time_label,
		runChans_label, qOps_label, msgInPs_label, msgOutPs_label,
		runChans_sc, qOps_sc, msgInPs_sc, msgOutPs_sc,
		compat_stat_pane;

extern void Refresh(),
	    Command(),
	    Connectbutton(),
	    Configure(),
	    QControlPopup(),
	    ChangeMode(),
	    setLoad();
extern double	runnableChans, opsPerSec, msgsInPerSec, msgsOutPerSec;

extern XFontStruct	*normalFont;
extern unsigned long	timeoutFor;
extern int		compat;

static void create_commandwidgets()
{
	Widget	pane, form, mainpane;

	topform = XtVaCreateManagedWidget("generalCommands",
			       formWidgetClass,
			       mainvpane,
			       NULL);

	refresh_command = XtVaCreateManagedWidget("refreshCommand",
				       commandWidgetClass,
				       topform,
				       XtNlabel,	"refresh",
				       NULL);
	if (normalFont != NULL)
		XtSetValues(refresh_command, argList, 1);

	XtAddCallback(refresh_command, XtNcallback, Refresh, NULL);

	quit_command = XtVaCreateManagedWidget("quitCommand",
				    commandWidgetClass,
				    topform,
				    XtNlabel,		"quit",
				    NULL);
	XtAddCallback(quit_command, XtNcallback, Command, quit);
	if (normalFont != NULL)
		XtSetValues(quit_command, argList, 1);

	connect_command = XtVaCreateManagedWidget("connectCommand",
				       commandWidgetClass,
				       topform,
				       XtNlabel, "connect",
				       NULL);
	XtAddCallback(connect_command, XtNcallback, Connectbutton, connect); 
	if (normalFont != NULL)
		XtSetValues(connect_command,argList, 1);

	mode_command = XtVaCreateManagedWidget("modeCommand",
				    commandWidgetClass,
				    topform,
				    XtNlabel, "control",
				    NULL);
	XtAddCallback(mode_command, XtNcallback, ChangeMode, NULL);
	if (normalFont != NULL)
		XtSetValues(mode_command,argList, 1);

	config_command = XtVaCreateManagedWidget("configCommand",
				      commandWidgetClass,
				      topform,
				      XtNlabel, "configuration",
				      NULL);
	XtAddCallback(config_command, XtNcallback, Configure, NULL);
	if (normalFont != NULL)
		XtSetValues(config_command,argList, 1);

	qcontrol_command = XtVaCreateManagedWidget("qcontrolCommand",
					commandWidgetClass,
					topform,
					XtNlabel, "Qmgr control",
					NULL);
	XtAddCallback(qcontrol_command, XtNcallback, QControlPopup, NULL);
	if (normalFont != NULL)
		XtSetValues(qcontrol_command,argList, 1);

	nextform = XtVaCreateManagedWidget("statusForm",
			       formWidgetClass,
			       mainvpane,
			       NULL);

	mainpane = XtVaCreateManagedWidget("statusMainPane",
			    panedWidgetClass,
			    nextform,
			    NULL);
	pane = XtVaCreateManagedWidget("statusPane",
			    panedWidgetClass,
			    mainpane,
			    NULL);
	form = XtVaCreateManagedWidget("statusForm1",
			    formWidgetClass,
			    pane,
			    NULL);
	runChans_label = XtVaCreateManagedWidget("runChans",
				   labelWidgetClass,
				   form,
				   NULL);
	if (normalFont != NULL)
		XtSetValues(runChans_label,argList, 1);

	runChans_sc = XtVaCreateManagedWidget("runChansStripChart",
				stripChartWidgetClass,
				form,
				XtNupdate,	(int) (timeoutFor / 2000),
				NULL);
	XtAddCallback(runChans_sc, XtNgetValue, setLoad, &runnableChans);

	form = XtVaCreateManagedWidget("statusForm2",
			    formWidgetClass,
			    pane,
			    NULL);

	qOps_label = XtVaCreateManagedWidget("qOps",
				   labelWidgetClass,
				   form,
				   NULL);
	if (normalFont != NULL)
		XtSetValues(qOps_label,argList, 1);

	qOps_sc = XtVaCreateManagedWidget("qOpsStripChart",
				stripChartWidgetClass,
				form,
				XtNupdate,	(int) (timeoutFor / 2000),
				NULL);
	XtAddCallback(qOps_sc, XtNgetValue, setLoad, &opsPerSec);
	
	if (!compat) {
		compat_stat_pane = pane = XtVaCreateManagedWidget("statusPane2",
								  panedWidgetClass,
								  mainpane,
								  NULL);
		form = XtVaCreateManagedWidget("statusForm3",
					       formWidgetClass,
					       pane,
					       NULL);
		msgInPs_label = XtVaCreateManagedWidget("msgInPs",
							labelWidgetClass,
							form,
							NULL);
		if (normalFont != NULL)
			XtSetValues(msgInPs_label,argList, 1);

		msgInPs_sc = XtVaCreateManagedWidget("msgInPsStripChart",
						     stripChartWidgetClass,
						     form,
						     XtNupdate,	(int) (timeoutFor / 2000),
						     NULL);
		XtAddCallback(msgInPs_sc, XtNgetValue, setLoad, &msgsInPerSec);

		form = XtVaCreateManagedWidget("statusForm4",
					       formWidgetClass,
					       pane,
					       NULL);

		msgOutPs_label = XtVaCreateManagedWidget("msgOutPs",
							 labelWidgetClass,
							 form,
							 NULL);
		if (normalFont != NULL)
			XtSetValues(msgOutPs_label,argList, 1);

		msgOutPs_sc = XtVaCreateManagedWidget("msgOutPsStripChart",
						      stripChartWidgetClass,
						      form,
						      XtNupdate,	(int) (timeoutFor / 2000),
						      NULL);
		XtAddCallback(msgOutPs_sc, XtNgetValue, setLoad, &msgsOutPerSec);
	}
	form = XtVaCreateManagedWidget("statusFormEnd",
			    formWidgetClass,
			    nextform,
			    NULL);
	total_volume_label = XtVaCreateManagedWidget("totalVolume",
					  labelWidgetClass,
					  form,
					  XtNlabel,	"Total volume",
					  NULL);
	if (normalFont != NULL)
		XtSetValues(total_volume_label,argList, 1);

	total_number_label = XtVaCreateManagedWidget("totalNumber",
					  labelWidgetClass,
					  form,
					  XtNlabel,	"Total number of messages",
					  NULL);
	if (normalFont != NULL)
		XtSetValues(total_number_label,argList, 1);

	time_label = XtVaCreateManagedWidget("timeLabel",
				  labelWidgetClass,
				  form,
				  XtNlabel, "qmgr status",
				  NULL);
	if (normalFont != NULL)
		XtSetValues(time_label,argList, 1);

	header = XtVaCreateManagedWidget("headerLabel",
			      labelWidgetClass,
			      form,
			      XtNlabel,	NO_CONNECTION,
			      NULL);
	if (normalFont != NULL)
		XtSetValues(header,argList, 1);


	error = XtVaCreateManagedWidget("errorLabel",
					labelWidgetClass,
					form,
					XtNlabel,	NO_CONNECTION,
					NULL);
	if (normalFont != NULL)
		XtSetValues(error,argList, 1);

	if (!compat) {
		statistics = XtVaCreateManagedWidget("statLabel",
						     labelWidgetClass,
						     form,
						     XtNlabel,	"Qmgr\nstatistics\n\n",
						     NULL);
		if (normalFont != NULL)
			XtSetValues(statistics,argList, 1);
		errorUp = 0;
	}
}


Widget	channels,
	channel_header,
	channel_commands,
	channel_stop,
	channel_start,
	channel_clear,
	channel_cacheadd,
	channel_downforce,
	channel_label,
	channel_info,
	channel_next,
	channel_prev,
	channel_all,
	channel_viewport,
	channel_all_form,
	channel_all_list;

extern struct chan_struct	*currentchan;
extern int			max_chan_border,
				read_currentchan;
char				*curlabel,
				*channel_info_str,
				*create_channel_header();
int				channel_info_strlen;

static void create_channelwidgets()
{
	Widget	temp_form;
	char	*str;

	channel_form = XtVaCreateManagedWidget("channelForm",
				    formWidgetClass,
				    mainvpane,
				    NULL);

	channel_header = XtVaCreateManagedWidget("channelHeader",
				      labelWidgetClass,
				      channel_form,
				      NULL);
	if (normalFont != NULL)
		XtSetValues(channel_header,argList, 1);

	XtVaGetValues(channel_header,
		  XtNlabel,	&str,
		  NULL);
	
	/* check for ap defaults */
	if (lexequ(str, "channelHeader") == 0) {
		printf("The application defaults aren't installed!\n");
		exit(1);
	}
	channel_commands = XtVaCreateManagedWidget("channelCommands",
					boxWidgetClass,
					channel_form,
					NULL);

	channel_info = XtVaCreateManagedWidget("channelInfo",
				    commandWidgetClass,
				    channel_commands,
				    XtNlabel,	"info",
				    NULL);
	if (normalFont != NULL)
		XtSetValues(channel_info,argList, 1);
	XtAddCallback(channel_info, XtNcallback, Command, chaninfo);

	channel_next = XtVaCreateManagedWidget("channelNext",
				    commandWidgetClass,
				    channel_commands,
				    NULL);
	if (normalFont != NULL)
		XtSetValues(channel_next,argList, 1);
	XtAddCallback(channel_next, XtNcallback, Command, channext);

	channel_prev = XtVaCreateManagedWidget("channelPrev",
				    commandWidgetClass,
				    channel_commands,
				    NULL);
	if (normalFont != NULL)
		XtSetValues(channel_prev,argList, 1);
	XtAddCallback(channel_prev, XtNcallback, Command, chanprev);


	channel_stop = XtVaCreateManagedWidget("channelStop",
				    commandWidgetClass,
				    channel_commands,
				    XtNlabel,	"disable",
				    NULL);
	if (normalFont != NULL)
		XtSetValues(channel_stop,argList, 1);

	XtAddCallback(channel_stop, XtNcallback, Command, chanstop);

	channel_start = XtVaCreateManagedWidget("channelStart",
				     commandWidgetClass,
				     channel_commands,
				     XtNlabel,	"enable",
				     NULL);
	if (normalFont != NULL)
		XtSetValues(channel_start,argList, 1);

	XtAddCallback(channel_start, XtNcallback, Command, chanstart);

	channel_clear = XtVaCreateManagedWidget("channelClear",
				     commandWidgetClass,
				     channel_commands,
				     NULL);
	if (normalFont != NULL)
		XtSetValues(channel_clear,argList, 1);

	XtAddCallback(channel_clear, XtNcallback, Command, chanclear);

	channel_cacheadd = XtVaCreateManagedWidget("channelCacheadd",
					commandWidgetClass,
					channel_commands,
					NULL);
	if (normalFont != NULL)
		XtSetValues(channel_cacheadd,argList, 1);

	XtAddCallback(channel_cacheadd, XtNcallback, Command, chancacheadd);

	channel_downforce = XtVaCreateManagedWidget("channelDownforce",
				     commandWidgetClass,
				     channel_commands,
				     NULL);
	if (normalFont != NULL)
		XtSetValues(channel_downforce,argList, 1);

	XtAddCallback (channel_downforce, XtNcallback, Command, chandownforce);


	currentchan = NULL;
	read_currentchan = 0;
	curlabel = create_channel_header();
	channel_label = XtVaCreateManagedWidget("channelLabel",
				     labelWidgetClass,
				     channel_form,
				     XtNlabel,	curlabel,
				     NULL);
	free(curlabel);
	if (normalFont != NULL)
		XtSetValues(channel_label,argList, 1);

	temp_form = XtVaCreateManagedWidget("channelForm2",
				 formWidgetClass,
				 mainvpane,
				 NULL);

	channel_viewport = XtVaCreateManagedWidget("channelViewport",
					viewportWidgetClass,
					temp_form,
					NULL);
	
	channels = XtVaCreateManagedWidget("channels",
				boxWidgetClass,
				channel_viewport,
				XtNvSpace,	(2*max_chan_border +1),
				NULL);

	channel_all = XtVaCreateManagedWidget("channelAllViewport",
					      viewportWidgetClass,
					      temp_form,
					      NULL);
	channel_all_form = XtVaCreateManagedWidget("channelAll",
						   formWidgetClass,
						   channel_all,
						   NULL);
	/* channel info widgets */
	channel_all_list = XtVaCreateManagedWidget("channelAllList",
						   listWidgetClass,
						   channel_all_form,
						   NULL);
	if (normalFont != NULL)
		XtSetValues(channel_all_list, argList, 1);

	XtSetMappedWhenManaged(channel_form, False);
	XtSetSensitive(channel_commands,FALSE);
}

Widget	control_vpane;

static void create_controlwidgets()
{
	control_form = XtVaCreateManagedWidget("controlForm",
				    formWidgetClass,
				    switchform,
				    NULL);

	control_vpane = XtVaCreateManagedWidget("controlVpane",
				     panedWidgetClass,
				     control_form,
				     NULL);
	create_mtawidgets();
	create_msgwidgets();
}

Widget		mta_divider,
		mta_commands,
		mta_stop,
		mta_start,
		mta_clear,
		mta_cacheadd,
		mta_force,
		mta_downforce,
		mta_label,
		mta_info,
		mta_next,
		mta_prev,
		mta_all,
		mta_all_form,
		mta_all_list,
		mta_viewport,
		mtas;
extern struct mta_struct	*currentmta;
extern int			read_currentmta;
extern int			max_mta_border;
extern char			*create_mta_header();
char				*mta_info_str;
int				mta_info_strlen;

static void create_mtawidgets()
{
	Widget	temp_form;

	mta_form = XtVaCreateManagedWidget("mtaForm",
				formWidgetClass,
				control_vpane,
				NULL);

	mta_divider = XtVaCreateManagedWidget("mtaDivider",
				   labelWidgetClass,
				   mta_form,
				   NULL);
	if (normalFont != NULL)
		XtSetValues(mta_divider,argList, 1);

	mta_commands = XtVaCreateManagedWidget("mtaCommands",
				    boxWidgetClass,
				    mta_form,
				    NULL);
	XtSetSensitive(mta_commands, FALSE);

	mta_info = XtVaCreateManagedWidget("mtaInfo",
				commandWidgetClass,
				mta_commands,
				XtNlabel,	"info",
				NULL);
	if (normalFont != NULL)
		XtSetValues(mta_info,argList, 1);

	XtAddCallback(mta_info, XtNcallback, Command, mtainfo);

	mta_next = XtVaCreateManagedWidget("mtaNext",
				commandWidgetClass,
				mta_commands,
				NULL);
	if (normalFont != NULL)
		XtSetValues(mta_next,argList, 1);

	XtAddCallback(mta_next, XtNcallback, Command, mtanext);

	mta_prev = XtVaCreateManagedWidget("mtaPrev",
				commandWidgetClass,
				mta_commands,
				NULL);
	if (normalFont != NULL)
		XtSetValues(mta_prev,argList, 1);

	XtAddCallback(mta_prev, XtNcallback, Command, mtaprev);

	mta_stop = XtVaCreateManagedWidget("mtaStop",
				commandWidgetClass,
				mta_commands,
				XtNlabel,	"disable",
				NULL);
	if (normalFont != NULL)
		XtSetValues(mta_stop,argList, 1);

	XtAddCallback(mta_stop, XtNcallback, Command, mtastop);

	mta_start = XtVaCreateManagedWidget("mtaStart",
				 commandWidgetClass,
				 mta_commands,
				 XtNlabel,	"enable",
				 NULL);
	if (normalFont != NULL)
		XtSetValues(mta_start,argList, 1);

	XtAddCallback(mta_start, XtNcallback, Command, mtastart);

	mta_clear = XtVaCreateManagedWidget("mtaClear",
				 commandWidgetClass,
				 mta_commands,
				 NULL);
	if (normalFont != NULL)
		XtSetValues(mta_clear,argList, 1);

	XtAddCallback(mta_clear, XtNcallback, Command, mtaclear);

	mta_cacheadd = XtVaCreateManagedWidget("mtaCacheadd",
				    commandWidgetClass,
				    mta_commands,
				    NULL);
	if (normalFont != NULL)
		XtSetValues(mta_cacheadd,argList, 1);

	XtAddCallback(mta_cacheadd, XtNcallback, Command, mtacacheadd);

	mta_force = XtVaCreateManagedWidget("mtaForce",
				 commandWidgetClass,
				 mta_commands,
				 NULL);
	if (normalFont != NULL)
		XtSetValues(mta_force,argList, 1);

	XtAddCallback(mta_force, XtNcallback, Command, mtaforce);

	mta_downforce = XtVaCreateManagedWidget("mtaDownforce",
				     commandWidgetClass,
				     mta_commands,
				     NULL);
	if (normalFont != NULL)
		XtSetValues(mta_downforce,argList, 1);

	XtAddCallback (mta_downforce, XtNcallback, Command, mtadownforce);

	currentmta = NULL;
	read_currentmta = 0;
	curlabel = create_mta_header();
	mta_label = XtVaCreateManagedWidget("mtaLabel",
				 labelWidgetClass,
				 mta_form,
				 XtNlabel, curlabel,
				 NULL);
	free(curlabel);
	if (normalFont != NULL)
		XtSetValues(mta_label,argList, 1);

	temp_form = XtVaCreateManagedWidget("mtaForm2",
				 formWidgetClass,
				 control_vpane,
				 NULL);

	mta_viewport = XtVaCreateManagedWidget("mtaViewport",
				    viewportWidgetClass,
				    temp_form,
				    NULL);
	mtas = XtVaCreateManagedWidget("mtas",
			    boxWidgetClass,
			    mta_viewport,
			    XtNdefaultDistance, (2*max_mta_border+1),
			    NULL);
	mta_all = XtVaCreateManagedWidget("mtaAllViewport",
			       viewportWidgetClass,
			       temp_form,
			       NULL);
	mta_all_form = XtVaCreateManagedWidget("mtaAll",
					       formWidgetClass,
					       mta_all,
					       NULL);
	mta_all_list = XtVaCreateManagedWidget("mtaAllList",
					       listWidgetClass,
					       mta_all_form,
					       NULL);
	if (normalFont != NULL)
		XtSetValues(mta_all_list, argList, 1);
}

Widget		msg_divider,
		msg_commands,
		msg_stop,
		msg_start,
		msg_clear,
		msg_cacheadd,
		msg_force,
		msg_label,
		msg_info,
		msg_next,
		msg_prev,
		msg_all,
		msg_all_form,
		msg_all_list,
		msgs_showall,
		msg_viewport,
		msgs;

extern struct msg_struct	*currentmsg;
extern char			*create_msg_header();
extern void			MsgsShowAll();
char				*msg_info_str;
int				msg_info_strlen;

static void create_msgwidgets()
{
	Widget	temp_form;

	msg_form = XtVaCreateManagedWidget("msgForm",
				formWidgetClass,
				control_vpane,
				NULL);

	msg_divider = XtVaCreateManagedWidget("msgDivider",
				   labelWidgetClass,
				   msg_form,
				   NULL);
	if (normalFont != NULL)
		XtSetValues(msg_divider,argList, 1);

	msg_commands = XtVaCreateManagedWidget("msgCommands",
				    boxWidgetClass,
				    msg_form,
				    NULL);
	XtSetSensitive(msg_commands, FALSE);

	msg_info = XtVaCreateManagedWidget("msgInfo",
				commandWidgetClass,
				msg_commands,
				XtNlabel,	"info",
				NULL);
	if (normalFont != NULL)
		XtSetValues(msg_info,argList, 1);
	XtAddCallback(msg_info, XtNcallback, Command, msginfo);
		
	msg_next = XtVaCreateManagedWidget("msgNext",
				commandWidgetClass,
				msg_commands,
				NULL);
	if (normalFont != NULL)
		XtSetValues(msg_next,argList, 1);
	XtAddCallback(msg_next, XtNcallback, Command, msgnext);
		
	msg_prev = XtVaCreateManagedWidget("msgPrev",
				commandWidgetClass,
				msg_commands,
				NULL);
	if (normalFont != NULL)
		XtSetValues(msg_prev,argList, 1);
	XtAddCallback(msg_prev, XtNcallback, Command, msgprev);
		
	msgs_showall = XtVaCreateManagedWidget("msgShowall",
				    commandWidgetClass,
				    msg_commands,
				    NULL);
	if (normalFont != NULL)
		XtSetValues(msgs_showall,argList, 1);
	XtAddCallback(msgs_showall, XtNcallback, MsgsShowAll, NULL);

	msg_stop = XtVaCreateManagedWidget("msgStop",
				commandWidgetClass,
				msg_commands,
				XtNlabel, "freeze",
				NULL);
	if (normalFont != NULL)
		XtSetValues(msg_stop,argList, 1);
	XtAddCallback(msg_stop, XtNcallback, Command, msgstop);

	msg_start = XtVaCreateManagedWidget("msgStart",
				 commandWidgetClass,
				 msg_commands,
				 XtNlabel,	"thaw",
				 NULL);
	if (normalFont != NULL)
		XtSetValues(msg_start,argList, 1);
	XtAddCallback(msg_start, XtNcallback, Command, msgstart);

	msg_clear = XtVaCreateManagedWidget("msgClear",
				 commandWidgetClass,
				 msg_commands,
				 NULL);
	if (normalFont != NULL)
		XtSetValues(msg_clear,argList, 1);
	XtAddCallback(msg_clear, XtNcallback, Command, msgclear);

	msg_cacheadd = XtVaCreateManagedWidget("msgCacheadd",
				    commandWidgetClass,
				    msg_commands,
				    NULL);
	if (normalFont != NULL)
		XtSetValues(msg_cacheadd,argList, 1);

	XtAddCallback(msg_cacheadd, XtNcallback, Command, msgcacheadd);

	msg_force = XtVaCreateManagedWidget("msgForce",
				 commandWidgetClass,
				 msg_commands,
				 NULL);
	if (normalFont != NULL)
		XtSetValues(msg_force,argList, 1);

	XtAddCallback(msg_force, XtNcallback, Command, msgforce);

	currentmsg = NULL;
	curlabel = create_msg_header();
	msg_label = XtVaCreateManagedWidget("msgLabel",
				 labelWidgetClass,
				 msg_form,
				 XtNlabel, curlabel,
				 NULL);
	free(curlabel);
	if (normalFont != NULL)
		XtSetValues(msg_label,argList, 1);
	temp_form = XtVaCreateManagedWidget("msgForm2",
				 formWidgetClass,
				 control_vpane,
				 NULL);

	msg_viewport = XtVaCreateManagedWidget("msgViewport",
				    viewportWidgetClass,
				    temp_form,
				    NULL);
	msgs = XtVaCreateManagedWidget("msgs",
			    formWidgetClass,
			    msg_viewport,
			    NULL);

	msg_all = XtVaCreateManagedWidget("msgAllViewport",
					  viewportWidgetClass,
					  temp_form,
					  NULL);
	msg_all_form = XtVaCreateManagedWidget("msgAll",
					       formWidgetClass,
					       msg_all,
					       NULL);
	msg_all_list = XtVaCreateManagedWidget("msgAllList",
					       listWidgetClass,
					       msg_all_form,
					       NULL);
	if (normalFont != NULL)
		XtSetValues(msg_all_list, argList, 1);
}

Widget	monitor_form;

static void create_monitorwidgets()
{
	Dimension	wid,
			ht;
	XtVaGetValues(control_form,
		  XtNwidth, 	&wid,
		  XtNheight,	&ht,
		  NULL);

	monitor_viewport = XtVaCreateWidget("monitorViewport",
					viewportWidgetClass,
					switchform,
					XtNwidth,	wid, /*TOTAL_WIDTH,*/
					XtNheight,	ht,  /* (2*VP_HEIGHT+4*26),*/
					NULL);

	monitor_form = XtVaCreateManagedWidget("monitorForm",
				    formWidgetClass,
				    monitor_viewport,
				    XtNwidth,	wid,
				    XtNheight,	ht,
				    NULL);
	XtManageChild(monitor_viewport);
	XtSetMappedWhenManaged(monitor_viewport, True);
	XtSetMappedWhenManaged(control_form, False);
}

/*  */
/* popups */

Popup_menu 	*yesno,
		*one,
		*two,
		*three,
		*config,
		*connectpopup,
		*qcontrol;
Widget	qversion,
	refresh_toggle,
	reconnect_toggle,
	backoff_toggle,
	confirm_toggle,
	compat_toggle,
	inbounds_toggle,
	heur_toggle,
	percent_form,
	line_form,
	auth_toggle;

extern void 	previousField(),
		nextField(),
		thisField(),
		myinsert_char(),
		mydelete_char(),
		mymenupopdown(),
		curChan(),
		excl_curChan(),
		readChan(),
		curMta(),
		excl_curMta(),
		readMta(),
		curMsg(),
		excl_curMsg(),
		readMsg(),
		chanMode(),
		chanModeRead(),
		chanRefresh(),
		mtaMode(),
		mtaModeRead(),
		mtaRefresh(),
		QControl(),
		popup_OK(),
		popup_NOTOK(),
		config_OK(),
		connectpopup_OK(),
		configToggle(),
		heurToggle();
extern int	autoRefresh,
		autoReconnect,
		backoff,
		confirm,
		compat,
		displayInactIns,
		auth;
int		newautoRefresh,
		newautoReconnect,
		newbackoff,
		newconfirm,
		newcompat,
		newdisplayInactIns,
		newauth;
Heuristic	newheur;
extern Heuristic heuristic;
unsigned long	newTimes;

XtActionsRec	actionTable[] = {
{"previousField",	previousField},
{"nextField",		nextField},
{"thisField",		thisField},
{"myinsert_char",	myinsert_char},
{"mydelete_char",	mydelete_char},
{"mymenupopdown",	mymenupopdown},
{"curChan",		curChan},
{"excl_curChan",	excl_curChan},
{"readChan",		readChan},
{"curMta",		curMta},
{"excl_curMta",		excl_curMta},
{"readMta",		readMta},
{"curMsg",		curMsg},
{"excl_curMsg",		excl_curMsg},
{"readMsg",		readMsg},
{"chanMode",		chanMode},
{"chanModeRead",	chanModeRead},
{"chanRefresh",		chanRefresh},
{"mtaMode",		mtaMode},
{"mtaModeRead",		mtaModeRead},
{"mtaRefresh",		mtaRefresh},
};

XtTranslations	channel_monitorTranslations,
		mta_monitorTranslations,
		passwdTranslations;

/*static char	*channel_translationTable =
	"<Btn1Up>:	curChan() readChan()\n\
	<Btn2Up>:	excl_curChan()\n\
	<Btn3Up>:	readChan()\n";*/
static char	*channel_monitortranslationTable =
	"<Btn1Up>:	chanModeRead()\n\
	<Btn2Up>:	chanMode()\n\
	<Btn3Up>:	chanRefresh()\n";
/*static char	*mta_translationTable =
	"<Btn1Up>:	curMta() readMta()\n\
	<Btn2Up>:	excl_curMta()\n\
	<Btn3Up>:	readMta()\n";*/
static char	*mta_monitortranslationTable =
	"<Btn1Up>:	mtaModeRead()\n\
	<Btn2Up>:	mtaMode()\n\
	<Btn3Up>:	mtaRefresh()\n";
/*static char	*msg_translationTable =
	"<Btn1Up>:	curMsg() readMsg()\n\
	<Btn2Up>:	excl_curMsg()\n\
	<Btn3Up>:	readMsg()\n";*/
/*static char	*text_translationTable =
	"<Key>Up:	previousField()\n\
	<Key>Down:	nextField()\n\
	<Key>Tab:	nextField()\n\
	<Btn1Down>:	thisField() select-start()\n";*/
static char	*passwd_translationTable =
	"<Key>Up:	previousField()\n\
	<Key>Down:	nextField()\n\
	<Key>Tab:	nextField()\n\
	<Btn1Down>:	thisField()\n\
	<Key>Delete:	mydelete_char()\n\
	<Key>BackSpace: mydelete_char()\n\
	<Key>:		myinsert_char()\n";
/*static char	*qcontrol_translationTable =
	"<BtnUp>:	mymenupopdown()\n\
	<Leave>:	mymenupopdown()\n";*/

/*static char	*OKAcceleratorTable = 
	"#override\n\
<Key>Return:	set() notify()";*/

static void create_defaultHostspopup()
{
}

static void create_popupwidgets(context)
XtAppContext	context;
{
	Widget	OKwg,
		NOTOKwg,
		label,
		version;
	Arg	arg[2];
	int	i;
	char	*str = NULL;
	extern char password[];
	
	XtAppAddActions(context, actionTable, XtNumber(actionTable));
	passwdTranslations = XtParseTranslationTable(passwd_translationTable);
	channel_monitorTranslations = XtParseTranslationTable(channel_monitortranslationTable);
	mta_monitorTranslations = XtParseTranslationTable(mta_monitortranslationTable);
	create_defaultHostspopup();

	XtSetArg(arg[0], XtNallowShellResize, True);
	XtSetArg(arg[1], XtNinput, True);

	qcontrol = (Popup_menu *) calloc(1, sizeof(*qcontrol));
	qcontrol->popup = XtCreatePopupShell("qcontrol",
					     overrideShellWidgetClass,
					     qcontrol_command,
					     arg,
					     2);
	qcontrol->form = XtVaCreateManagedWidget("qcontrolForm",
				      boxWidgetClass,
				      qcontrol->popup,
				      NULL);

	label = XtVaCreateManagedWidget("increaseMaxChannels",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_increasemaxchans);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	label = XtVaCreateManagedWidget("decreaseMaxChannels",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_decreasemaxchans);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	label = XtVaCreateManagedWidget("enableSubmission",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_enableSubmission);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	label = XtVaCreateManagedWidget("disableSubmission",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_disableSubmission);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	label = XtVaCreateManagedWidget("enableAll",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_enableAll);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	label = XtVaCreateManagedWidget("disableAll",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_disableAll);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
		
	label = XtVaCreateManagedWidget("rereadQ",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_rereadQueue);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	label = XtVaCreateManagedWidget("restartQmgr",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_restart);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	label = XtVaCreateManagedWidget("gracefulStop",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_gracefulTerminate);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	label = XtVaCreateManagedWidget("emergencyStop",
			     commandWidgetClass,
			     qcontrol->form,
			     NULL);
	XtAddCallback(label, XtNcallback, 
		      QControl, int_Qmgr_QMGROp_abort);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	yesno = (Popup_menu *) calloc(1, sizeof(*yesno));

	yesno->popup = XtCreatePopupShell("yesno_popup",
					  transientShellWidgetClass,
					  quit_command,
					  arg,
					  2);

	yesno->form = XtVaCreateManagedWidget("yesnoForm",
				   formWidgetClass,
				   yesno->popup,
				   NULL);

	OKwg = XtVaCreateManagedWidget("yesnoOK",
			    commandWidgetClass,
			    yesno->form,
			    NULL);
	if (normalFont != NULL)
		XtSetValues(OKwg,argList, 1);
	yesno->op = unknown;
	XtAddCallback(OKwg, XtNcallback, popup_OK, &(yesno->op));

	NOTOKwg = XtVaCreateManagedWidget("yesnoNotok",
			       commandWidgetClass,
			       yesno->form,
			       NULL);
	if (normalFont != NULL)
		XtSetValues(NOTOKwg,argList, 1);
	XtAddCallback(NOTOKwg, XtNcallback, popup_NOTOK, &(yesno->op));
	XtInstallAllAccelerators(yesno->form, yesno->form);

	one = (Popup_menu *) calloc(1, sizeof(*one));
	one->tuple = (Popup_tuple *) calloc(1, sizeof(Popup_tuple));
	one->numberOftuples = 1;
	one->popup = XtCreatePopupShell("onetext_popup",
					transientShellWidgetClass,
					quit_command,
					arg,
					2);

	one->form = XtVaCreateManagedWidget("oneForm",
				 formWidgetClass,
				 one->popup,
				 NULL);

	OKwg = XtVaCreateManagedWidget("oneOk",
			    commandWidgetClass,
			    one->form,
			    NULL);
	one->op = unknown;
	if (normalFont != NULL)
		XtSetValues(OKwg,argList, 1);
	XtAddCallback(OKwg, XtNcallback, popup_OK, &(one->op));

	NOTOKwg = XtVaCreateManagedWidget("oneNotok",
			       commandWidgetClass,
			       one->form,
			       NULL);
	if (normalFont != NULL)
		XtSetValues(NOTOKwg,argList, 1);
	XtAddCallback(NOTOKwg, XtNcallback, popup_NOTOK, &(one->op));

	one->tuple[0].label = XtVaCreateManagedWidget("oneLabelOne",
					   labelWidgetClass,
					   one->form,
					   XtNlabel, "clear cache on channel ",
					   NULL);
	if (normalFont != NULL)
		XtSetValues(one->tuple[0].label,argList, 1);

	if (normalFont != NULL)
	one->tuple[0].text = XtVaCreateManagedWidget("oneTextOne",
					  asciiTextWidgetClass,
					  one->form,
						     XtNresize, XawtextResizeWidth,
					  XtNscrollVertical, XawtextScrollWhenNeeded,
					  XtNfont,	normalFont,
					  NULL);
	else
	one->tuple[0].text = XtVaCreateManagedWidget("oneTextOne",
					  asciiTextWidgetClass,
					  one->form,
						     XtNresize, XawtextResizeWidth,
					  XtNscrollVertical, XawtextScrollWhenNeeded,
					  NULL);

	XtInstallAllAccelerators(one->tuple[0].text, one->form);

	two = (Popup_menu *) calloc(1, sizeof(*two));
	two->tuple = (Popup_tuple *) calloc(2, sizeof(Popup_tuple));
	two->numberOftuples = 2;
	two->popup = XtCreatePopupShell("twotext_popup",
					transientShellWidgetClass,
					quit_command,
					arg,
					2);
	two->form  = XtVaCreateManagedWidget("twoForm",
				  formWidgetClass,
				  two->popup,
				  NULL);
	OKwg = XtVaCreateManagedWidget("twoOk",
			    commandWidgetClass,
			    two->form,
			    NULL);
	if (normalFont != NULL)
		XtSetValues(OKwg,argList, 1);
	two->op = unknown;
	XtAddCallback(OKwg, XtNcallback, popup_OK, &(two->op));

	NOTOKwg = XtVaCreateManagedWidget("twoNotok",
			       commandWidgetClass,
			       two->form,
			       NULL);
	if (normalFont != NULL)
		XtSetValues(NOTOKwg,argList, 1);

	XtAddCallback(NOTOKwg, XtNcallback, popup_NOTOK, &(two->op));

	two->tuple[0].label = XtVaCreateManagedWidget("twoLabelOne",
					   labelWidgetClass,
					   two->form,
					   XtNlabel, "clear cache on channel",
					   NULL);
	if (normalFont != NULL)
		XtSetValues(two->tuple[0].label,argList, 1);

	if (normalFont != NULL)
	two->tuple[0].text = XtVaCreateManagedWidget("twoTextOne",
					  asciiTextWidgetClass,
					  two->form,
					  XtNresize, XawtextResizeWidth,
					  XtNscrollVertical, XawtextScrollWhenNeeded,
					  XtNfont,	normalFont,
					  NULL);
	else
	two->tuple[0].text = XtVaCreateManagedWidget("twoTextOne",
					  asciiTextWidgetClass,
					  two->form,
					  XtNresize, XawtextResizeWidth,
					  XtNscrollVertical, XawtextScrollWhenNeeded,
					  NULL);

	two->tuple[1].label = XtVaCreateManagedWidget("twoLabelTwo",
					   labelWidgetClass,
					   two->form,
					   XtNlabel, "clear cache on channel",
					   NULL);
	if (normalFont != NULL)
		XtSetValues(two->tuple[1].label,argList, 1);

	if (normalFont != NULL)
	two->tuple[1].text = XtVaCreateManagedWidget("twoTextTwo",
					  asciiTextWidgetClass,
					  two->form,
					  XtNresize, XawtextResizeWidth,
					  XtNscrollVertical, XawtextScrollWhenNeeded,
					  XtNfont,		normalFont,
					  NULL);
	else
	two->tuple[1].text = XtVaCreateManagedWidget("twoTextTwo",
					  asciiTextWidgetClass,
					  two->form,
					  XtNresize, XawtextResizeWidth,
					  XtNscrollVertical, XawtextScrollWhenNeeded,
					  NULL);

	XtInstallAllAccelerators(two->tuple[0].text, two->form);
	XtInstallAllAccelerators(two->tuple[1].text, two->form);
	
	three = (Popup_menu *) calloc(1, sizeof(*three));
	three->op = unknown;
	three->tuple = (Popup_tuple *) calloc(3, sizeof(Popup_tuple));
	three->numberOftuples = 3;
	three->popup = XtCreatePopupShell("threetext_popup",
					  transientShellWidgetClass,
					  quit_command,
					  arg,
					  2);
	three->form  = XtVaCreateManagedWidget("threeForm",
				    formWidgetClass,
				    three->popup,
				    XtNborderWidth,	0,
				    NULL);
	OKwg = XtVaCreateManagedWidget("threeOk",
			    commandWidgetClass,
			    three->form,
			    NULL);
	if (normalFont != NULL)
		XtSetValues(OKwg,argList, 1);
	XtAddCallback(OKwg, XtNcallback, popup_OK, &(three->op));

	NOTOKwg = XtVaCreateManagedWidget("threeNotok",
			       commandWidgetClass,
			       three->form,
			       NULL);
	if (normalFont != NULL)
		XtSetValues(NOTOKwg,argList, 1);
	XtAddCallback(NOTOKwg, XtNcallback, popup_NOTOK, &(three->op));

	three->tuple[0].label = XtVaCreateManagedWidget("threeLabelOne",
					     labelWidgetClass,
					     three->form,
					     XtNlabel, "clear cache on channel",
					     NULL);

	if (normalFont != NULL)
		XtSetValues(three->tuple[0].label,argList, 1);
	if (normalFont != NULL)
	three->tuple[0].text = XtVaCreateManagedWidget("threeTextOne",
					    asciiTextWidgetClass,
					    three->form,
					    XtNscrollVertical, XawtextScrollWhenNeeded,
						       XtNresize, XawtextResizeWidth,
					    NULL);
	else
	three->tuple[0].text = XtVaCreateManagedWidget("threeTextOne",
					    asciiTextWidgetClass,
					    three->form,
					    XtNscrollVertical, XawtextScrollWhenNeeded,
						       XtNresize, XawtextResizeWidth,
					    XtNfont,		normalFont,
					    NULL);

	three->tuple[1].label = XtVaCreateManagedWidget("threeLabelTwo",
					     labelWidgetClass,
					     three->form,
					     XtNlabel, "clear cache on channel",
					     NULL);
	if (normalFont != NULL)
		XtSetValues(three->tuple[1].label,argList, 1);

	if (normalFont == NULL)
	three->tuple[1].text = XtVaCreateManagedWidget("threeTextTwo",
					    asciiTextWidgetClass,
					    three->form,
					    XtNscrollVertical, XawtextScrollWhenNeeded,
						       XtNresize, XawtextResizeWidth,
					    NULL);
	else
	three->tuple[1].text = XtVaCreateManagedWidget("threeTextTwo",
					    asciiTextWidgetClass,
					    three->form,
					    XtNscrollVertical, XawtextScrollWhenNeeded,
						       XtNresize, XawtextResizeWidth,
					    XtNfont,		normalFont,
					    NULL);

	three->tuple[2].label = XtVaCreateManagedWidget("threeLabelThree",
					     labelWidgetClass,
					     three->form,
					     XtNlabel, "clear cache on channel",
					     NULL);
	if (normalFont != NULL)
		XtSetValues(three->tuple[2].label,argList, 1);
	if (normalFont == NULL)
	three->tuple[2].text = XtVaCreateManagedWidget("threeTextThree",
					    asciiTextWidgetClass,
					    three->form,
					    XtNscrollVertical, XawtextScrollWhenNeeded,
						       XtNresize, XawtextResizeWidth,
					    NULL);
	else
	three->tuple[2].text = XtVaCreateManagedWidget("threeTextThree",
					    asciiTextWidgetClass,
					    three->form,
					    XtNscrollVertical, XawtextScrollWhenNeeded,
						       XtNresize, XawtextResizeWidth,
					    XtNfont,		normalFont,
					    NULL);

	XtInstallAllAccelerators(three->tuple[0].text, three->form);
	XtInstallAllAccelerators(three->tuple[1].text, three->form);
	XtInstallAllAccelerators(three->tuple[2].text, three->form);

	config = (Popup_menu *) calloc(1, sizeof(*config));
	config->op = unknown;
	config->numberOftuples = 8;
	config->tuple = (Popup_tuple *) calloc((unsigned)config->numberOftuples,
					       sizeof(Popup_tuple));

	config->popup = XtCreatePopupShell("config_popup",
					   transientShellWidgetClass,
					   config_command,
					   arg,
					   2);
	config->form = XtVaCreateManagedWidget("configForm",
				    formWidgetClass,
				    config->popup,
				    NULL);

	OKwg = XtVaCreateManagedWidget("configOk",
			    commandWidgetClass,
			    config->form,
			    NULL);
	XtAddCallback(OKwg, XtNcallback, config_OK, NULL);
	if (normalFont != NULL)
		XtSetValues(OKwg,argList, 1);

	NOTOKwg = XtVaCreateManagedWidget("configNotok",
			       commandWidgetClass,
			       config->form,
			       NULL);
	XtAddCallback(NOTOKwg, XtNcallback, popup_NOTOK, &(config->op));
	if (normalFont != NULL)
		XtSetValues(NOTOKwg,argList, 1);

	label = XtVaCreateManagedWidget("versionLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	version = XtVaCreateManagedWidget("configVersion",
			       labelWidgetClass,
			       config->form,
			       XtNlabel, ppversion,
			       NULL);
	if (normalFont != NULL)
		XtSetValues(version,argList, 1);
	
	label = XtVaCreateManagedWidget("qmgrVersionLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	qversion = XtVaCreateManagedWidget("qmgrVersion",
			       labelWidgetClass,
			       config->form,
			       NULL);
	if (normalFont != NULL)
		XtSetValues(qversion,argList, 1);
			       
	label = XtVaCreateManagedWidget("autoRefreshLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	refresh_toggle = XtVaCreateManagedWidget("autoRefreshToggle",
				      commandWidgetClass,
				      config->form,
				      XtNlabel, (autoRefresh == TRUE) ? "enabled" : "disabled",
				      NULL);
	if (normalFont != NULL)
		XtSetValues(refresh_toggle,argList, 1);

	XtAddCallback(refresh_toggle, XtNcallback, configToggle, &newautoRefresh);
				      
	config->tuple[REFRESH].label = XtVaCreateManagedWidget("refreshIntervalLabel",
						    labelWidgetClass,
						    config->form,
						    NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[REFRESH].label,argList, 1);

	if (normalFont == NULL)
	config->tuple[REFRESH].text = XtVaCreateManagedWidget("refreshIntervalText",
						   asciiTextWidgetClass,
						   config->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   NULL);
	else
	config->tuple[REFRESH].text = XtVaCreateManagedWidget("refreshIntervalText",
						   asciiTextWidgetClass,
						   config->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   XtNfont,		normalFont,
						   NULL);

	config->tuple[INACTIVE].label = XtVaCreateManagedWidget("inactiveLabel",
						     labelWidgetClass,
						     config->form,
						     NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[INACTIVE].label,argList, 1);

	if (normalFont == NULL)
	config->tuple[INACTIVE].text = XtVaCreateManagedWidget("inactiveText",
						    asciiTextWidgetClass,
						    config->form,
						    XtNscrollVertical, XawtextScrollWhenNeeded,
							       XtNresize, XawtextResizeWidth,
						    NULL);
	else
	config->tuple[INACTIVE].text = XtVaCreateManagedWidget("inactiveText",
						    asciiTextWidgetClass,
						    config->form,
						    XtNscrollVertical, XawtextScrollWhenNeeded,
							       XtNresize, XawtextResizeWidth,
						    XtNfont,		normalFont,
						    NULL);

	label = XtVaCreateManagedWidget("autoReconnectLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	reconnect_toggle = XtVaCreateManagedWidget("autoReconnectToggle",
					commandWidgetClass,
					config->form,
					XtNlabel, (autoReconnect == TRUE) ? "enabled" : "disabled",
					NULL);
	if (normalFont != NULL)
		XtSetValues(reconnect_toggle,argList, 1);
	XtAddCallback(reconnect_toggle, XtNcallback, configToggle, &newautoReconnect);

	config->tuple[START].label = XtVaCreateManagedWidget("arStartLabel",
						  labelWidgetClass,
						  config->form,
						  NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[START].label,argList, 1);

	if (normalFont == NULL)
	config->tuple[START].text = XtVaCreateManagedWidget("arStartText",
						 asciiTextWidgetClass,
						 config->form,
						 XtNscrollVertical, XawtextScrollWhenNeeded,
							    XtNresize, XawtextResizeWidth,
						 NULL);
	else
	config->tuple[START].text = XtVaCreateManagedWidget("arStartText",
						 asciiTextWidgetClass,
						 config->form,
						 XtNscrollVertical, XawtextScrollWhenNeeded,
							    XtNresize, XawtextResizeWidth,
						 XtNfont,		normalFont,
						 NULL);

	label = XtVaCreateManagedWidget("backOffLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);

	backoff_toggle = XtVaCreateManagedWidget("backOffToggle",
				      commandWidgetClass,
				      config->form,
				      XtNlabel, (backoff == TRUE) ? "enabled" : "disabled",
				      NULL);
	if (normalFont != NULL)
		XtSetValues(backoff_toggle,argList, 1);

	XtAddCallback(backoff_toggle, XtNcallback, configToggle, &newbackoff);

	config->tuple[BACKOFF].label = XtVaCreateManagedWidget("boIncLabel",
						    labelWidgetClass,
						    config->form,
						    NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[BACKOFF].label,argList, 1);


	if (normalFont == NULL)
	config->tuple[BACKOFF].text = XtVaCreateManagedWidget("boIncText",
						   asciiTextWidgetClass,
						   config->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   NULL);
	else
	config->tuple[BACKOFF].text = XtVaCreateManagedWidget("boIncText",
						   asciiTextWidgetClass,
						   config->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   XtNfont,		normalFont,
						   NULL);
				   
	config->tuple[CONNECTMAX].label = XtVaCreateManagedWidget("maxRCIntLabel",
						       labelWidgetClass,
						       config->form,
						       NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[CONNECTMAX].label,argList, 1);

	if (normalFont == NULL)
	config->tuple[CONNECTMAX].text = XtVaCreateManagedWidget("maxRCIntText",
						      asciiTextWidgetClass,
						      config->form,
						      XtNscrollVertical, XawtextScrollWhenNeeded,
								 XtNresize, XawtextResizeWidth,
						      NULL);
	else
	config->tuple[CONNECTMAX].text = XtVaCreateManagedWidget("maxRCIntText",
						      asciiTextWidgetClass,
						      config->form,
						      XtNscrollVertical, XawtextScrollWhenNeeded,
								 XtNresize, XawtextResizeWidth,
						      XtNfont,		normalFont,
						      NULL);

	label = XtVaCreateManagedWidget("confPopupLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	confirm_toggle = XtVaCreateManagedWidget ("confPopupToggle",
				       commandWidgetClass,
				       config->form,
				       XtNlabel, (confirm == TRUE) ? "enabled" : "disabled",
				       NULL);
	if (normalFont != NULL)
		XtSetValues(confirm_toggle,argList, 1);
	XtAddCallback(confirm_toggle, XtNcallback, configToggle, &newconfirm);

	label = XtVaCreateManagedWidget("compatPopupLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	compat_toggle = XtVaCreateManagedWidget ("compatPopupToggle",
				       commandWidgetClass,
				       config->form,
				       XtNlabel, (compat == TRUE) ? "enabled" : "disabled",
				       NULL);
	if (normalFont != NULL)
		XtSetValues(compat_toggle,argList, 1);
	XtAddCallback(compat_toggle, XtNcallback, configToggle, &newcompat);

	label = XtVaCreateManagedWidget("inboundPopupLabel",
			     labelWidgetClass,
			     config->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	inbounds_toggle = XtVaCreateManagedWidget ("inboundPopupToggle",
				       commandWidgetClass,
				       config->form,
				       XtNlabel, (displayInactIns == TRUE) ? "enabled" : "disabled",
				       NULL);
	if (normalFont != NULL)
		XtSetValues(inbounds_toggle,argList, 1);
	XtAddCallback(inbounds_toggle, XtNcallback, configToggle, &newdisplayInactIns);

	label = XtVaCreateManagedWidget ("heurLabel",
			      labelWidgetClass,
			      config->form,
			      NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	
	switch (heuristic) {
	    case percentage:
		str = PERCENT_BASED;
		break;
	    case all:
		str = ALL_BASED;
		break;
	    default:
		heuristic = line;
	    case line:
		str = LINE_BASED;
		break;
	    case chanonly:
		str = CHANONLY_BASED;
		break;
	}
	heur_toggle = XtVaCreateManagedWidget("heurToggle",
				   commandWidgetClass,
				   config->form,
				   XtNlabel, str,
				   NULL);
	newheur = heuristic;
	if (normalFont != NULL)
		XtSetValues(heur_toggle,argList, 1);
	XtAddCallback(heur_toggle, XtNcallback, heurToggle, &newheur);

	percent_form = XtVaCreateManagedWidget("percentForm",
				    formWidgetClass,
				    config->form,
				    NULL);	

	config->tuple[PERCENT].label = XtVaCreateManagedWidget ("percentLabel",
						     labelWidgetClass,
						     percent_form,
						     NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[PERCENT].label,argList, 1);
	if (normalFont == NULL)
	config->tuple[PERCENT].text = XtVaCreateManagedWidget("percentText",
						   asciiTextWidgetClass,
						   percent_form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   NULL);
	else
	config->tuple[PERCENT].text = XtVaCreateManagedWidget("percentText",
						   asciiTextWidgetClass,
						   percent_form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   XtNfont,		normalFont,
						   NULL);

	config->tuple[MINBADMTA].label = XtVaCreateManagedWidget ("numBadMtasLabel",
						     labelWidgetClass,
						     percent_form,
						     NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[MINBADMTA].label,argList, 1);
	if (normalFont == NULL)
	config->tuple[MINBADMTA].text = XtVaCreateManagedWidget("numBadMtasText",
						   asciiTextWidgetClass,
						   percent_form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
								XtNresize, XawtextResizeWidth,
						   NULL);
	else
	config->tuple[MINBADMTA].text = XtVaCreateManagedWidget("numBadMtasText",
						   asciiTextWidgetClass,
						   percent_form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
								XtNresize, XawtextResizeWidth,
						     XtNfont,		normalFont,
						   NULL);

	line_form = XtVaCreateManagedWidget("lineForm",
				 formWidgetClass,
				 config->form,
				 NULL);
	config->tuple[LINEMAX].label = XtVaCreateManagedWidget ("lineLabel",
						     labelWidgetClass,
						     line_form,
						     NULL);
	if (normalFont != NULL)
		XtSetValues(config->tuple[LINEMAX].label,argList, 1);
	if (normalFont == NULL)
		config->tuple[LINEMAX].text = XtVaCreateManagedWidget("lineText",
						   asciiTextWidgetClass,
						   line_form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
								      XtNresize, XawtextResizeWidth,
						   NULL);
	else 		config->tuple[LINEMAX].text = XtVaCreateManagedWidget("lineText",
						   asciiTextWidgetClass,
						   line_form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
									      XtNresize, XawtextResizeWidth,
			  XtNfont,		normalFont,
						   NULL);

	switch (heuristic) {
	    case percentage:
		XtSetMappedWhenManaged (line_form, FALSE);
		break;
	    case line:
		XtSetMappedWhenManaged (percent_form, FALSE);
		break;
	    case all:
	    case chanonly:
		XtSetMappedWhenManaged (line_form, FALSE);
		XtSetMappedWhenManaged (percent_form, FALSE);
		break;
	}

	
	for (i = 0; i < config->numberOftuples; i++) {
		XtInstallAllAccelerators(config->tuple[i].text, config->form);
	}

	connectpopup = (Popup_menu *) calloc(1, sizeof(Popup_menu));
	connectpopup->op = connect;
	connectpopup->tuple = (Popup_tuple *) calloc(4, sizeof(Popup_tuple));
	connectpopup->numberOftuples = 4;

	connectpopup->popup = XtCreatePopupShell("connectpopup",
						 transientShellWidgetClass,
						 connect_command,
						 arg,
						 2);
	connectpopup->form = XtVaCreateManagedWidget("connectForm",
					  formWidgetClass,
					  connectpopup->popup,
					  NULL);
	OKwg = XtVaCreateManagedWidget("connectOk",
			    commandWidgetClass,
			    connectpopup->form,
			    NULL);
	XtAddCallback(OKwg, XtNcallback, connectpopup_OK, NULL);
	if (normalFont != NULL)
		XtSetValues(OKwg,argList, 1);
	NOTOKwg = XtVaCreateManagedWidget("connectNotok",
			       commandWidgetClass,
			       connectpopup->form,
			       NULL);
	if (normalFont != NULL)
		XtSetValues(NOTOKwg,argList, 1);
	XtAddCallback(NOTOKwg, XtNcallback, popup_NOTOK, &(connectpopup->op));

	connectpopup->tuple[0].label = XtVaCreateManagedWidget("connectToLabel",
						    labelWidgetClass,
						    connectpopup->form,
						    NULL);
	if (normalFont != NULL)
		XtSetValues(connectpopup->tuple[0].label,argList, 1);
	if (normalFont != NULL) 
	connectpopup->tuple[0].text = XtVaCreateManagedWidget("connectToText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   XtNfont,		normalFont,
						   NULL);
	else
	connectpopup->tuple[0].text = XtVaCreateManagedWidget("connectToText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   XtNfont,		normalFont,
						   NULL);
	connectpopup->tuple[1].label = XtVaCreateManagedWidget("taiFileLabel",
						    labelWidgetClass,
						    connectpopup->form,
						    NULL);
	if (normalFont != NULL)
		XtSetValues(connectpopup->tuple[1].label,argList, 1);
	if (normalFont == NULL)
		connectpopup->tuple[1].text = XtVaCreateManagedWidget("taiFileText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
								      XtNresize, XawtextResizeWidth,
						   NULL);
	else
		connectpopup->tuple[1].text = XtVaCreateManagedWidget("taiFileText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
								      XtNresize, XawtextResizeWidth,
							   XtNfont,		normalFont,
						   NULL);

	label = XtVaCreateManagedWidget("authLabel",
			     labelWidgetClass,
			     connectpopup->form,
			     NULL);
	if (normalFont != NULL)
		XtSetValues(label,argList, 1);
	auth_toggle = XtVaCreateManagedWidget("authToggle",
				   commandWidgetClass,
				   connectpopup->form,
				   XtNlabel, (auth == TRUE) ? "enabled" : "disabled",
				   NULL);
	if (normalFont != NULL)
		XtSetValues(auth_toggle,argList, 1);
	XtAddCallback(auth_toggle, XtNcallback,	configToggle, &newauth);

	connectpopup->tuple[2].label = XtVaCreateManagedWidget("userLabel",
						    labelWidgetClass,
						    connectpopup->form,
						    NULL);
	if (normalFont != NULL)
		XtSetValues(connectpopup->tuple[2].label,argList, 1);
if (normalFont == NULL)
	connectpopup->tuple[2].text = XtVaCreateManagedWidget("userText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   NULL);
else
	connectpopup->tuple[2].text = XtVaCreateManagedWidget("userText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
			  XtNfont,		normalFont,
						   NULL);

	connectpopup->tuple[3].label = XtVaCreateManagedWidget("passwdLabel",
						    labelWidgetClass,
						    connectpopup->form,
						    NULL);
	if (normalFont != NULL)
		XtSetValues(connectpopup->tuple[3].label,argList, 1);
if (normalFont == NULL)
	connectpopup->tuple[3].text = XtVaCreateManagedWidget("passwdText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
						   NULL);
else
	connectpopup->tuple[3].text = XtVaCreateManagedWidget("passwdText",
						   asciiTextWidgetClass,
						   connectpopup->form,
						   XtNscrollVertical, XawtextScrollWhenNeeded,
							      XtNresize, XawtextResizeWidth,
			  XtNfont,		normalFont,
						   NULL);
	XtOverrideTranslations(connectpopup->tuple[3].text, passwdTranslations);
	XtInstallAllAccelerators(connectpopup->tuple[0].text, 
				 connectpopup->form);
	XtInstallAllAccelerators(connectpopup->tuple[1].text, 
				 connectpopup->form);
	XtInstallAllAccelerators(connectpopup->tuple[2].text, 
				 connectpopup->form);
	XtInstallAllAccelerators(connectpopup->tuple[3].text, 
				 connectpopup->form);
}

MapVolume(bool)
int	bool;
{
	XtSetMappedWhenManaged(total_volume_label, bool);
	XtSetMappedWhenManaged(total_number_label, bool);
}

MapButtons(bool)
int	bool;
{
	/* various buttons that need auth */
	XtSetMappedWhenManaged(qcontrol_command, bool);

	XtSetMappedWhenManaged(channel_stop, bool);
	XtSetMappedWhenManaged(channel_start, bool);
	XtSetMappedWhenManaged(channel_cacheadd, bool);
	XtSetMappedWhenManaged(channel_downforce, bool);
	XtSetMappedWhenManaged(channel_clear, bool);

	XtSetMappedWhenManaged(mta_stop, bool);
	XtSetMappedWhenManaged(mta_start, bool);
	XtSetMappedWhenManaged(mta_cacheadd, bool);
	XtSetMappedWhenManaged(mta_force, bool);
	XtSetMappedWhenManaged(mta_downforce, bool);
	XtSetMappedWhenManaged(mta_clear, bool);

	XtSetMappedWhenManaged(msg_stop, bool);
	XtSetMappedWhenManaged(msg_start, bool);
	XtSetMappedWhenManaged(msg_cacheadd, bool);
	XtSetMappedWhenManaged(msg_force, bool);
	XtSetMappedWhenManaged(msg_clear, bool);
}
