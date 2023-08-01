/* bops.c: various buttons routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/bops.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/bops.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: bops.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"
extern int	autoReconnect,
		userConnected;
extern State	connectState;
extern struct chan_struct	*find_channel();
extern struct mta_struct	*find_mta();
extern struct msg_struct	*find_msg();
extern Widget	top;
extern char	*reverse_mta();
extern int	uk_order;
extern int	compat;
extern int	displayInactIns;
extern Heuristic heuristic,
		newheur;

void		SetAuthDisplay();

/* ARGSUSED */ 
void Refresh(w, client_data, call_data)
Widget	w;
caddr_t	client_data;	/* unused */
caddr_t call_data;	/* unused */
{
/*	StartWait();*/
	if (connectState == connected) {
		if (!compat)
			my_invoke(qmgrStatus, (char **) NULL);
		my_invoke(chanread, (char **) NULL);
	} else if (autoReconnect == TRUE && userConnected == TRUE) {
		TermConnectTimeOut();
		ConnectTimeOut((caddr_t) NULL,(XtIntervalId *) NULL);
	}
	ResetInactiveTimeout();
/*	EndWait();*/
}

extern time_t	parsetime();
Operations			currentop;
extern struct chan_struct	*currentchan;
extern struct mta_struct	*currentmta;
extern struct msg_struct	*currentmsg;
extern int			confirm, read_currentchan;
extern char			*hostname;
extern Popup_menu		*yesno,
				*one,
				*two,
				*three;
int				forceDown = FALSE;

/* ARGSUSED */
void Command(w, op, call_data)
Widget	w;
Operations	op;	
caddr_t	call_data;	/* unused */
{
	char		*title;
	int		x, y;
	Popup_menu	*popup = NULL;
	Window		r_child;

	ResetInactiveTimeout();
	XtVaGetValues(w,
		  XtNlabel, &title,
		  NULL);

	if (strcmp(title, "all") == 0) {
		if (op == chaninfo)
			chan_display_all();
		else if (op == mtainfo)
			mta_display_all();
		else if (op == msginfo) 
			msg_display_all();
		return;
	}

	currentop = op;
/*	StartWait();	*/
	/* should pop up template hereabouts */
	switch (op) {
	    case quit:
		if (confirm == TRUE) {
			popup = yesno;
		} else {
			Quit();
		}
		break;
	    case connect:
	    case disconnect:
		if (strcmp(title, "connect") == 0) {
			currentop = connect;
			popup = one;
			textdisplay(&(one->tuple[0]), 
			    (hostname == NULL) ? "" : hostname);
			XtVaSetValues(one->tuple[0].label,
				  XtNlabel, "Connect to ",
				  NULL);
		} else {
			currentop = disconnect;
			if (confirm == TRUE) {
				popup = yesno;
			} else {
				Disconnect();
			}
		}
		break;
	    case chanstart:
		if (currentchan == NULL) {
			textdisplay(&(one->tuple[0]), "");
			XtVaSetValues(one->tuple[0].label, 
				  XtNlabel, "Enable channel",
				  NULL);
			popup = one;
		} else 
			ChanControl(currentop, currentchan->channelname, 
				    (char *) NULL);
		break;
	    case chanstop:
		if (currentchan == NULL) {
			textdisplay(&(one->tuple[0]), "");
			XtVaSetValues(one->tuple[0].label, 
				  XtNlabel, "Disable channel",
				  NULL);
			popup = one;
		} else 
			ChanControl(currentop, currentchan->channelname, 
				    (char *) NULL);
		break;
	    case chanclear:
		if (currentchan == NULL) {
			textdisplay(&(one->tuple[0]), "");
			XtVaSetValues(one->tuple[0].label, 
				  XtNlabel, "Remove delay on channel",
				  NULL);
			popup = one;
		} else 
			ChanControl(currentop, currentchan->channelname, 
				    (char *) NULL);
		break;
	    case chancacheadd:
		popup = two;
		XtVaSetValues(two->tuple[0].label,
			  XtNlabel, "Add delay (in s m h d and/or w)",
			  NULL);
		XtVaSetValues(two->tuple[1].label,
			  XtNlabel, "On channel",
			  NULL);
		textdisplay(&(two->tuple[1]),
			    (currentchan == NULL) ? "" : currentchan->channelname);
		textdisplay(&(two->tuple[0]), "");
		break;
			
	    case chaninfo:
		if (currentchan == NULL) {
			textdisplay(&(one->tuple[0]), "");
			XtVaSetValues(one->tuple[0].label, 
				  XtNlabel, "Info on channel",
				  NULL);
			popup = one;
		} else
			ChanInfo(currentchan->channelname);
		break;

	    case channext:
		ChanNext();
		break;

	    case chanprev:
		ChanPrev();
		break;
			
	    case chandownforce:
		if (currentchan == NULL) {
			textdisplay (&(one->tuple[0]), "");
			XtVaSetValues(one->tuple[0].label,
				  XtNlabel, "Downward force attempt on channel",
				  NULL);
			popup = one;
		} else {
			forceDown = TRUE;
			ChanDownForce (currentchan);
			forceDown = FALSE;
		}
		break;

	    case mtastart:
		if (currentmta == NULL
		    || currentchan == NULL) {
			textdisplay(&(two->tuple[0]), 
				    (currentmta == NULL) ? "" : currentmta->mta);
			XtVaSetValues(two->tuple[0].label,
				  XtNlabel, "Enable mta  ",
				  NULL);  
			textdisplay(&(two->tuple[1]),
				    (currentchan == NULL) ? "" : currentchan->channelname);
			XtVaSetValues(two->tuple[1].label,
				  XtNlabel, "On channel",
				  (char *) NULL);
			popup = two;
		} else 
			MtaControl(currentop, 
				   currentmta->mta,
				   currentchan->channelname,
				   (char *) NULL);
		break;

	    case mtastop:
		if (currentmta == NULL
		    || currentchan == NULL) {
			textdisplay(&(two->tuple[0]), 
				    (currentmta == NULL) ? "" : currentmta->mta);
			XtVaSetValues(two->tuple[0].label,
				  XtNlabel, "Disable mta  ",
				  NULL);
			textdisplay(&(two->tuple[1]),
				    (currentchan == NULL) ? "" : currentchan->channelname);
			XtVaSetValues(two->tuple[1].label,
				  XtNlabel, "On channel",
				  NULL);
			popup = two;
		} else 
			MtaControl(currentop, 
				   currentmta->mta,
				   currentchan->channelname,
				   (char *) NULL);
		break;
	    case mtaclear:
		if (currentmta == NULL
		    || currentchan == NULL) {
			textdisplay(&(two->tuple[0]), 
				    (currentmta == NULL) ? "" : currentmta->mta);
			XtVaSetValues(two->tuple[0].label,
				  XtNlabel, "Remove delay on mta",
				  NULL);
			textdisplay(&(two->tuple[1]),
				    (currentchan == NULL) ? "" : currentchan->channelname);
			XtVaSetValues(two->tuple[1].label,
				  XtNlabel, "On channel",
				  NULL);
			popup = two;
		} else 
			MtaControl(currentop, 
				   currentmta->mta,
				   currentchan->channelname,
				   (char *) NULL);

		break;
	    case mtaforce:
		if (currentmta == NULL
		    || currentchan == NULL) {
			textdisplay(&(two->tuple[0]), 
				    (currentmta == NULL) ? "" : currentmta->mta);
			XtVaSetValues(two->tuple[0].label,
				  XtNlabel, "Force attempt of mta",
				  NULL);
			textdisplay(&(two->tuple[1]),
				    (currentchan == NULL) ? "" : currentchan->channelname);
			XtVaSetValues(two->tuple[1].label,
				  XtNlabel, "On channel",
				  NULL);
			popup = two;
		} else 
			MtaForce(currentmta,
				 currentchan);

		break;

	    case mtadownforce:
		if (currentmta == NULL
		    || currentchan == NULL) {
			textdisplay(&(two->tuple[0]), 
				    (currentmta == NULL) ? "" : currentmta->mta);
			XtVaSetValues(two->tuple[0].label,
				  XtNlabel, "Downward force attempt of mta",
				  NULL);
			textdisplay(&(two->tuple[1]),
				    (currentchan == NULL) ? "" : currentchan->channelname);
			XtVaSetValues(two->tuple[1].label,
				  XtNlabel, "On channel",
				  NULL);
			popup = two;
		} else {
			forceDown = TRUE;
			MtaDownForce(currentchan,
				 currentmta);
			forceDown = FALSE;
		}
		break;
	    case mtacacheadd:
		popup = three;
		XtVaSetValues(three->tuple[0].label,
			  XtNlabel, "Add delay (in s m h d and/or w)",
			  NULL);
		textdisplay(&(three->tuple[0]), "");
		XtVaSetValues(three->tuple[1].label,
			  XtNlabel, "For mta",
			  NULL);
		textdisplay(&(three->tuple[1]),
			    (currentmta == NULL) ? "" : currentmta->mta);
		XtVaSetValues(three->tuple[2].label,
			  XtNlabel,"On channel",
			  NULL);
		textdisplay(&(three->tuple[2]),
			    (currentchan == NULL) ? "" : currentchan->channelname);
		break;
	    case mtainfo:
		if (currentmta == NULL
		    || currentchan == NULL) {
			textdisplay(&(two->tuple[0]), 
				    (currentmta == NULL) ? "" : currentmta->mta);
			XtVaSetValues(two->tuple[0].label,
				  XtNlabel, "Info on mta",
				  NULL);
			textdisplay(&(two->tuple[1]),
				    (currentchan == NULL) ? "" : currentchan->channelname);
			XtVaSetValues(two->tuple[1].label,
				  XtNlabel, "On channel",
				  NULL);
			popup = two;
		} else 
			MtaInfo(currentmta->mta,
				currentchan->channelname);
		break;

	    case mtanext:
		MtaNext();
		break;
		
	    case mtaprev:
		MtaPrev();
		break;

	    case msgstart:
		popup = two;
		XtVaSetValues(two->tuple[0].label,
			  XtNlabel, "Thaw message",
			  NULL);
		textdisplay(&(two->tuple[0]),
			    (currentmsg == NULL) ? "" : currentmsg->msginfo->queueid);
		textdisplay(&(two->tuple[1]), "");
		XtVaSetValues(two->tuple[1].label,
			  XtNlabel, "User list (,) *=all",
			  NULL);
		break;
	    case msgstop:
		popup = two;
		XtVaSetValues(two->tuple[0].label,
			  XtNlabel, "Freeze message  ",
			  NULL);
		textdisplay(&(two->tuple[0]),
			    (currentmsg == NULL) ? "" : currentmsg->msginfo->queueid);
		textdisplay(&(two->tuple[1]), "");
		XtVaSetValues(two->tuple[1].label,
			  XtNlabel, "User list (,) *=all",
			  NULL);
		break;
	    case msgclear:
		popup = two;
		XtVaSetValues(two->tuple[0].label,
			  XtNlabel, "Remove delay on message",
			  NULL);
		textdisplay(&(two->tuple[0]),
			    (currentmsg == NULL) ? "" : currentmsg->msginfo->queueid);
		textdisplay(&(two->tuple[1]), "");
		XtVaSetValues(two->tuple[1].label,
			  XtNlabel, "User list (,) *=all",
			  NULL);
		break;
	    case msgforce:
		if (currentmta == NULL) {
			popup = three;
			XtVaSetValues(three->tuple[0].label,
				  XtNlabel, "Force Attempt on Message",
				  NULL);
			textdisplay(&(three->tuple[0]),
				    (currentmsg == NULL) ? "" : currentmsg->msginfo->queueid);
			XtVaSetValues(three->tuple[1].label,
				  XtNlabel, "User list (,) *=all",
				  NULL);
			textdisplay(&(three->tuple[1]),"");
			XtVaSetValues(three->tuple[2].label,
				  XtNlabel, "On Mta",
				  NULL);
			textdisplay(&(three->tuple[2]),"");
		} else {
			/* know what mta is */
			popup = two;
			XtVaSetValues(two->tuple[0].label,
				  XtNlabel, "Force Attempt on Message",
				  NULL);
			textdisplay(&(two->tuple[0]),
				    (currentmsg == NULL) ? "" : currentmsg->msginfo->queueid);
			XtVaSetValues(two->tuple[1].label,
				  XtNlabel, "User list (,) *=all",
				  NULL);
			textdisplay(&(two->tuple[1]),"");
		}
		break;
	    case msginfo:
		if (currentmsg == NULL) {
			popup = one;
			XtVaSetValues(one->tuple[0].label,
				  XtNlabel, "Info on message",
				  NULL);
			textdisplay(&(one->tuple[0]), "");
		} else 
			MsgInfo(currentmsg->msginfo->queueid);
			
		break;

	    case msgnext:
		MsgNext();
		break;
		
	    case msgprev:
		MsgPrev();
		break;

	    case msgcacheadd:
		popup = three;
		XtVaSetValues(three->tuple[0].label,
			  XtNlabel, "Add delay (in s m h d and/or w)",
			  NULL);
		textdisplay(&(three->tuple[0]), "");
		XtVaSetValues(three->tuple[1].label,
			  XtNlabel, "To message",
			  NULL);
		textdisplay(&(three->tuple[1]),
			    (currentmsg == NULL) ? "" : currentmsg->msginfo->queueid);
		XtVaSetValues(three->tuple[2].label,
			  XtNlabel, "User list (,) *=all",
			  NULL);
		textdisplay(&(three->tuple[2]), "");
		break;
	    default:
		popup = one;
		XtVaSetValues(one->tuple[0].label,
			  XtNlabel, "WHAT ????????",
			  NULL);
		break;
	}
/*	EndWait();*/
	if (popup != NULL) {
		XTranslateCoordinates(XtDisplay(w),XtWindow(w),
				      XDefaultRootWindow(XtDisplay(w)),0,0,
				      &x,&y,
				      &r_child);
		Popup(popup,currentop,x,y);
	}
	
}

extern Popup_menu	*connectpopup;
extern int		newauth,
			auth;
extern char		password[],
			username[],
			tailorfile[];

/* ARGSUSED */
void Connectbutton(w, op, call_data)
Widget	w;
Operations	op;	
caddr_t	call_data;	/* unused */
{
	char		*title;
	int		x, y;
	Popup_menu	*popup = NULL;
	Window		r_child;
	
	XtVaGetValues(w,
		  XtNlabel, &title,
		  NULL);

	currentop = op;
	if (strcmp(title, "connect") == 0) {
		currentop = connect;
		newauth = auth;
		SetAuthDisplay(newauth);
		popup = connectpopup;
		textdisplay(&(connectpopup->tuple[0]), 
			    (hostname == NULL) ? "" : hostname);
		textdisplay(&(connectpopup->tuple[1]),
			    (tailorfile[0] == '\0') ? "" : tailorfile);
	} else {
		currentop = disconnect;
		if (confirm == TRUE) {
			popup = yesno;
		} else {
			Disconnect();
		}
	}
	if (popup != NULL) {
		XTranslateCoordinates(XtDisplay(w),XtWindow(w),
				      XDefaultRootWindow(XtDisplay(w)),0,0,
				      &x,&y,
				      &r_child);
		Popup(popup,currentop,x,y);
	}
}

Mode	mode = monitor;
extern Widget	mode_command, channel_label, qcontrol_command;
extern Authentication authentication;
extern void ChanToggle();

/* ARGSUSED */
void	ChangeMode(w, client_data, call_data)
Widget	w;
caddr_t	client_data;
caddr_t call_data;
{
	extern Widget	channel_form,
			monitor_viewport,
			control_form;
/*	StartWait();*/
	if (mode == monitor) {
		XtVaSetValues(mode_command,
			XtNlabel, "monitor",
			NULL);
		mode = control;
		toggle_info_displays();
		reset_label(channel_label);
		XtSetMappedWhenManaged(monitor_viewport, False);
		XtSetMappedWhenManaged(control_form, True);
		XtSetMappedWhenManaged(channel_form, True);
		if (authentication == full && connectState == connected)
			XtSetMappedWhenManaged(qcontrol_command, True);
		if (currentchan != NULL && read_currentchan != 0) {
			clear_mta_refresh_list();
			add_mta_refresh_list(currentchan->channelname);
			construct_event(mtaread);
		}
	} else {
		XtVaSetValues(mode_command, 
			  XtNlabel, "control", 
			  NULL);
		mode = monitor;
		ChanToggle();
		XtSetMappedWhenManaged(monitor_viewport, True);
		XtSetMappedWhenManaged(control_form, False);
		XtSetMappedWhenManaged(channel_form, False);
		if (authentication == full)
			XtSetMappedWhenManaged(qcontrol_command, False);
		if (connectState == connected) {
			if (!compat)
				my_invoke(qmgrStatus, (char **) NULL);
			my_invoke(chanread,(char **) NULL);
		}
	}
/*	EndWait();*/
	ResetInactiveTimeout();
}

extern unsigned long 	timeoutFor,
			inactiveFor,
			connectInit,
			connectInc,
			connectMax;
extern char	*unparsetime();
extern Popup_menu	*config;
extern Widget		refresh_toggle,
			reconnect_toggle,
			backoff_toggle,
			confirm_toggle,
			compat_toggle,
			inbounds_toggle,
			heur_toggle;
extern int	autoRefresh,
		autoReconnect,
		backoff,
		percent,
		lower_bound_mtas,
		max_vert_lines;
extern int	newautoRefresh,
		newautoReconnect,
		newbackoff,
		newconfirm,
		newcompat,
		newdisplayInactIns;
int		newint;
unsigned long	newTimes;
extern XColor	white, black;
extern Pixel	bg, fg;

/* ARGSUSED */
void Configure(w, client_data, call_data)
Widget	w;
caddr_t	client_data;	/* unused */
caddr_t call_data;	/* unused */
{
	int	x,y,i;
	Window	r_child;
	char	number[BUFSIZ], *str;

	newautoRefresh = autoRefresh;
	newautoReconnect = autoReconnect;
	newbackoff = backoff;
	newconfirm = confirm;
	newcompat = compat;
	newdisplayInactIns = displayInactIns;
	newheur = heuristic;
	XawFormDoLayout (config->form, False);
	textdisplay(&(config->tuple[REFRESH]), 
		    unparsetime(timeoutFor / 1000 ));
	textdisplay(&(config->tuple[INACTIVE]),
		    unparsetime(inactiveFor / 1000));
	textdisplay(&(config->tuple[START]), 
		    unparsetime(connectInit / 1000 ));
	textdisplay(&(config->tuple[BACKOFF]), 	
		    unparsetime(connectInc / 1000 ));
	textdisplay(&(config->tuple[CONNECTMAX]), 
		    unparsetime(connectMax / 1000 ));
	sprintf(number, "%d", percent);
	textdisplay(&(config->tuple[PERCENT]),number);
	sprintf(number, "%d", lower_bound_mtas);
	textdisplay(&(config->tuple[MINBADMTA]), number);
	sprintf(number, "%d", max_vert_lines);
	textdisplay(&(config->tuple[LINEMAX]), number);
	
	XtSetKeyboardFocus(config->form, config->tuple[REFRESH].text);
	XtVaSetValues(config->tuple[REFRESH].text,
		  XtNborderColor, fg,
		  NULL);
	XtVaSetValues (refresh_toggle,
		   XtNlabel, (autoRefresh == TRUE) ? "enabled" : "disabled",
		   NULL);
	XtVaSetValues (reconnect_toggle,
		   XtNlabel, (autoReconnect == TRUE) ? "enabled" : "disabled",
		   NULL);
	XtVaSetValues (backoff_toggle,
		   XtNlabel, (backoff == TRUE) ? "enabled" : "disabled",
		   NULL);
	XtVaSetValues (confirm_toggle,
		   XtNlabel, (confirm == TRUE) ? "enabled" : "disabled",
		   NULL);
	XtVaSetValues (compat_toggle,
		   XtNlabel, (compat == TRUE) ? "enabled" : "disabled",
		   NULL);
	XtVaSetValues (inbounds_toggle,
		   XtNlabel, (displayInactIns == TRUE) ? "enabled" : "disabled",
		   NULL);

	switch (heuristic) {
	    case percentage:
		str = PERCENT_BASED;
		break;
	    case all:
		str = ALL_BASED;
		break;
	    case chanonly:
		str = CHANONLY_BASED;
		break;
	    default:
		heuristic = line;
	    case line:
		str = LINE_BASED;
		break;
	}
	XtVaSetValues (heur_toggle,
		   XtNlabel,	str,
		   NULL);

	for (i = 1; i < config->numberOftuples; i++) 
		XtVaSetValues(config->tuple[i].text,
			  XtNborderColor, bg,
			  NULL);
	XawFormDoLayout (config->form, True);
	XTranslateCoordinates(XtDisplay(w),XtWindow(w),
			      XDefaultRootWindow(XtDisplay(w)),0,0,&x,&y,
			      &r_child);
	Popup(config, unknown, x, y);
}

/* ARGSUSED */
void popup_NOTOK(w, op, call_data)
Widget	w;
Operations	*op;
caddr_t call_data;
{
	Popup_menu	*popup;
	
	switch(*op) {
	    case quit:
	    case disconnect:
		popup = yesno;
		break;
	    case connect:
		popup = connectpopup;
		break;
	    case chanread:
	    case chanstop:
	    case chanstart:
	    case chanclear:
	    case chaninfo:
	    case chandownforce:
	    case msginfo:
		popup = one;
		break;
	    case chancacheadd:
	    case mtaread:
	    case mtastop:
	    case mtastart:
	    case mtaclear:
	    case msgstop:
	    case msgstart:
	    case msgclear:
	    case mtainfo:
	    case mtaforce:
	    case mtadownforce:
		popup = two;
		break;
	    case mtacacheadd:
	    case msgcacheadd:
		popup = three;
		break;
	    case msgforce:
		if (currentmta == NULL)
			popup = three;
		else 
			popup = two;
		break;
	    default:
		popup = config;
		break;
	}
	ResetInactiveTimeout();
	if (popup != NULL)
		Popdown(popup);
}

/* ARGSUSED */
void popup_OK(w, op, call_data)
Widget	w;
Operations	*op;
caddr_t 	call_data;
{
	Popup_menu	*popup = NULL;
	struct chan_struct	*chan;
	char	*str0, *str1, *str2;
	char	*ch;

	XSync(XtDisplay(w), 0);
	switch(*op) {
	    case quit:
		Quit();
		popup = yesno;
		break;
	    case disconnect:
		Disconnect();
		popup = yesno;
		break;
	    case chanstop:
	    case chanstart:
	    case chanclear:
		XtVaGetValues(one->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		if (str0[0] != '\0') {
			ChanControl(*op, str0, (char *) NULL);
			popup = one;
		}
		break;
	    case chancacheadd:
		XtVaGetValues(two->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(two->tuple[1].text,
			      XtNstring, &str1,
			      NULL);
		if (str0[0] != '\0' 
		    && str1[0] != '\0') {
			ChanControl(*op, str1, str0);
			popup = two;
		}
		break;
	    case chaninfo:
		XtVaGetValues(one->tuple[0].text,
			      XtNstring, &str0,
			      NULL);

		if (str0[0] != '\0') {
			ChanInfo(str0);
			popup = one;
		}
		break;
	    case chandownforce:
		XtVaGetValues(one->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		if (str0[0] != '\0') {
			if ((chan = find_channel(str0)) != NULL){
				forceDown = TRUE;
				ChanDownForce(chan);
				forceDown = FALSE;
				popup = one;
			}
		}
		break;
	    case msgstop:
	    case msgstart:
	    case msgclear:
		XtVaGetValues(two->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(two->tuple[1].text,
			      XtNstring, &str1,
			      NULL);
		
		if (str0[0] != '\0' && str1[0] != '\0') {
			MsgControl(*op, str0, 
				   str1, (char *) NULL);
			popup = two;
		}
		break;
	    case msgforce:
		if (currentchan == NULL)
			break;
		if (currentmta == NULL) {
			XtVaGetValues(three->tuple[0].text,
				      XtNstring, &str0,
				      NULL);
			XtVaGetValues(three->tuple[1].text,
				      XtNstring, &str1,
				      NULL);
			XtVaGetValues(three->tuple[2].text,
				      XtNstring, &str2,
				      NULL);

			if (str0[0] != '\0'
			    && str1[0] != '\0'
			    && str2[0] != '\0') {
				struct mta_struct	*mta = NULL;
				struct msg_struct	*msg = NULL;
				char			*ch = NULL;
				
				ch = (uk_order) ? reverse_mta(str2) : str2;
				if ((mta = find_mta(currentchan,
						    ch)) != NULL
				    && (msg = find_msg(str0)) != NULL) {
					MsgForce(msg, str1,
						 mta, currentchan);
					popup = three;
				}
				if (uk_order && ch != str2) free(ch);
			} 
		} else {
			XtVaGetValues(two->tuple[0].text,
				      XtNstring, &str0,
				      NULL);
			XtVaGetValues(two->tuple[1].text,
				      XtNstring, &str1,
				      NULL);

			if (str0[0] != '\0'
			    && str1[0] != '\0') {
				struct msg_struct	*msg;
				if ((msg = find_msg(str0)) != NULL) {
					MsgForce(msg, str1,
						 currentmta, currentchan);
					popup = two;
				}
			}
		}
		break;
	    case msgcacheadd:
		XtVaGetValues(three->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(three->tuple[1].text,
			      XtNstring, &str1,
			      NULL);
		XtVaGetValues(three->tuple[2].text,
			      XtNstring, &str2,
			      NULL);

		if (str0[0] != '\0'
		    && str1[0] != '\0'
		    && str2[0] != '\0') {
			MsgControl(*op, str1,
				   str2, str0);
			popup = three;
		}
		break;
	      
	    case msginfo:
		XtVaGetValues(one->tuple[0].text,
			      XtNstring, &str0,
			      NULL);

		if (str0[0] != '\0') {
			MsgInfo(str0);
			popup = one;
		}
		break;
	    case mtaclear:
	    case mtastop:
	    case mtastart:
		XtVaGetValues(two->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(two->tuple[1].text,
			      XtNstring, &str1,
			      NULL);
		
		if (str0[0] != '\0'
		    && str1[0] != '\0') {
			ch = (uk_order) ? reverse_mta(str0) : str0;
			MtaControl(*op, ch, 
				   str1, (char *) NULL);
			if (uk_order && ch != str0) free (ch);
			popup = two;
		}
		break;
	    case mtaforce:
		XtVaGetValues(two->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(two->tuple[1].text,
			      XtNstring, &str1,
			      NULL);

		if (str0[0] != '\0'
		    && str1[0] != '\0') {
			struct mta_struct	*mta;
			struct chan_struct	*chan;
			ch = (uk_order) ? reverse_mta(str0) : str0;
			if ((chan = find_channel(str1)) != NULL
			    && (mta = find_mta(chan,
					       ch)) != NULL) {
				MtaForce(mta, chan);
				popup = two;
			}
			if (uk_order && ch != str0) free(ch);
		}
		break;

	    case mtadownforce:
		XtVaGetValues(two->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(two->tuple[1].text,
			      XtNstring, &str1,
			      NULL);

		if (str0[0] != '\0'
		    && str1[0] != '\0') {
			struct mta_struct	*mta;
			struct chan_struct	*chan;
			if ((chan = find_channel(str1)) != NULL) {
				if (chan->num_mtas == 0)
					my_invoke(mtaread, 
						  &(chan->channelname)); 
				ch = (uk_order) ? reverse_mta(str0) : str0;
				if((mta = find_mta(chan,
						   ch)) != NULL) {
					forceDown = TRUE;
					MtaDownForce(chan, mta);
					forceDown = FALSE;
					popup = two;
				}
				if (uk_order && ch != str0) free(ch);
			}
		}
		break;

	    case mtainfo:
		XtVaGetValues(two->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(two->tuple[1].text,
			      XtNstring, &str1,
			      NULL);

		if (str0[0] != '\0'
		    && str1[0] != '\0') {
			ch = (uk_order) ? reverse_mta(str0) : str0;
			MtaInfo(ch, str1);
			if (uk_order && ch != str0) free(ch);
			popup = two;
		}
		break;
	    case mtacacheadd:
		XtVaGetValues(three->tuple[0].text,
			      XtNstring, &str0,
			      NULL);
		XtVaGetValues(three->tuple[1].text,
			      XtNstring, &str1,
			      NULL);
		XtVaGetValues(three->tuple[2].text,
			      XtNstring, &str2,
			      NULL);

		if (str0[0] != '\0'
		    && str1[0] != '\0'
		    && str2[0] != '\0') {
			ch = (uk_order) ? reverse_mta(str1) : str1;
			MtaControl(*op, ch, 
				   str2, str0);
			if (uk_order && ch != str1) free(ch);
			popup = three;
		}
		
	    default:
		break;
	}
	ResetInactiveTimeout();
	if (popup != NULL)
		Popdown(popup);
	else 
		XBell(XtDisplay(w), 50);
}


/* configure buttons */
#define MINTIMEOUT	30000	/* 1/2 min */

extern Widget	runChans_sc, qOps_sc, msgOutPs_sc, msgInPs_sc;

settimeoutFor(str)
char	*str;
{
	newTimes = (unsigned long) (parsetime(str) * 1000);
	if (newTimes > MINTIMEOUT) 
		timeoutFor = newTimes;
}

extern Widget	statistics;
		
/* ARGSUSED */
void config_OK(w, client_data, call_data)
Widget	w;
caddr_t client_data;
caddr_t call_data;
{
	char	*str;
	int	ok = TRUE,
		do_reset_refresh = FALSE,
		do_reset_reconnect = FALSE;
	
	if (newautoRefresh != autoRefresh) {
		autoRefresh = newautoRefresh;
		do_reset_refresh = TRUE;
	}

	XtVaGetValues (config->tuple[REFRESH].text,
		       XtNstring, &str,
		       NULL);
	newTimes = (unsigned long) (parsetime(str) * 1000);
	
	if (newTimes != timeoutFor) {
		if (newTimes < MINTIMEOUT) {
			/* BEEP */
			timeoutFor = MINTIMEOUT;
			textdisplay(&(config->tuple[REFRESH]), 	
				    unparsetime(timeoutFor / 1000));
			ok = FALSE;
		} else {
			timeoutFor = newTimes;
			do_reset_refresh = TRUE;
		}
		XtVaSetValues(runChans_sc,
			      XtNupdate, (int) (timeoutFor / 2000),
			      NULL);
		XtVaSetValues(qOps_sc,
			      XtNupdate, (int) (timeoutFor / 2000),
			      NULL);
		if (statistics != NULL) {
			XtVaSetValues(msgInPs_sc,
				      XtNupdate, (int) (timeoutFor / 2000),
				      NULL);
			XtVaSetValues(msgOutPs_sc,
				      XtNupdate, (int) (timeoutFor / 2000),
				      NULL);
		}

	}
			
	XtVaGetValues (config->tuple[INACTIVE].text,
		       XtNstring, &str,
		       NULL);
	newTimes = (unsigned long) (parsetime(str) * 1000);
	
	if (newTimes != inactiveFor) 
		inactiveFor = newTimes;
			
	XtVaGetValues (config->tuple[START].text,
		       XtNstring, &str,
		       NULL);
	newTimes = (unsigned long) (parsetime(str) * 1000);
	
	if (newTimes != connectInit) {
		connectInit = newTimes;
		do_reset_reconnect = TRUE;
	}

	XtVaGetValues (config->tuple[BACKOFF].text,
		       XtNstring, &str,
		       NULL);
	newTimes = (unsigned long) (parsetime(str) * 1000);
	
	if (newTimes != connectInc) {
		connectInc = newTimes;
		do_reset_reconnect = TRUE;
	}

	XtVaGetValues (config->tuple[CONNECTMAX].text,
		       XtNstring, &str,
		       NULL);
	newTimes = (unsigned long) (parsetime(str) * 1000);
	
	if (newTimes != connectMax) {
		connectMax = newTimes;
		do_reset_reconnect = TRUE;
	}


	if (newautoReconnect != autoReconnect) {
		autoReconnect = newautoReconnect;
		do_reset_reconnect = TRUE;
	}

	if (newbackoff != backoff) {
		backoff = newbackoff;
		do_reset_reconnect = TRUE;
	}

	if (newconfirm != confirm)
		confirm = newconfirm;
	if (newcompat != compat) {
		if (statistics != NULL) {
			compat = newcompat;
			redo_statistics_compat();
		} else
			ok = FALSE;

	}
	if (newdisplayInactIns != displayInactIns)
		displayInactIns = newdisplayInactIns;

	if (newheur != heuristic)
		heuristic = newheur;
	switch (heuristic) {
	    case percentage:
		XtVaGetValues (config->tuple[PERCENT].text,
			       XtNstring, &str,
			       NULL);
		if ((newint = atoi(str)) != 0)
			percent = newint;
		XtVaGetValues (config->tuple[MINBADMTA].text,
			       XtNstring, &str,
			       NULL);
		if ((newint = atoi(str)) != 0)
			lower_bound_mtas = newint;
		break;
	    case line:
		XtVaGetValues (config->tuple[LINEMAX].text,
			       XtNstring, &str,
			       NULL);
		if ((newint = atoi(str)) != 0)
			max_vert_lines = newint;
		break;
	    default:
		break;
	}
	if (do_reset_refresh == TRUE
	    && connectState == connected) 
		InitRefreshTimeOut((unsigned long) 500);
	if (do_reset_reconnect == TRUE
	    && connectState != connected
	    && userConnected == TRUE) InitConnectTimeOut();
	ResetInactiveTimeout();
	if (ok == TRUE) 
		Popdown(config);
	else
		XBell(XtDisplay(w), 50);

}

extern Popup_menu	*currentpopup,
			*qcontrol;

extern Widget	line_form, percent_form;

/* ARGSUSED */
void heurToggle (w, pheur, call_data)
Widget	w;
Heuristic	*pheur;
caddr_t	call_data;
{
	if (currentpopup != config)
		return;
	switch (*pheur) {
	    case percentage:
		*pheur = line;
		XtVaSetValues(w,
			  XtNlabel, LINE_BASED,
			  NULL);
		XtSetMappedWhenManaged (line_form, True);
		XtSetMappedWhenManaged (percent_form, False);
		break;

	    case line:
		*pheur = all;
		XtVaSetValues(w,
			  XtNlabel, ALL_BASED,
			  NULL);
		XtSetMappedWhenManaged (line_form, False);
		XtSetMappedWhenManaged (percent_form, False);
		break;

	    case all:
		*pheur = chanonly;
		XtVaSetValues(w,
			  XtNlabel, CHANONLY_BASED,
			  NULL);
		XtSetMappedWhenManaged (line_form, False);
		XtSetMappedWhenManaged (percent_form, False);
		break;
	    case chanonly:
		*pheur = percentage;
		XtVaSetValues (w,
			       XtNlabel, PERCENT_BASED,
			       NULL);
		XtSetMappedWhenManaged (line_form, False);
		XtSetMappedWhenManaged (percent_form, True);
		break;
	}
}

extern Widget	auth_toggle;

/* ARGSUSED */
void configToggle(w, pint, call_data)
Widget	w;
int	*pint;
caddr_t	call_data;
{
	if (currentpopup == NULL)
		return;
	if (*pint == TRUE) {
		/* enabled -> disabled */
		*pint = FALSE;	
		XtVaSetValues(w,
			  XtNlabel, "disabled",
			  NULL);
		if (w == auth_toggle
		    && currentpopup->selected != 0) {
			XtSetKeyboardFocus(top,
					   currentpopup->tuple[0].text);
			XtSetKeyboardFocus(currentpopup->form, 
					   currentpopup->tuple[0].text);
			XtVaSetValues(currentpopup->tuple[0].text,
				  XtNborderColor, fg,
				  NULL);
			XtVaSetValues(currentpopup->tuple[currentpopup->selected].text,
				  XtNborderColor, bg,
				  NULL);
			currentpopup->selected = 0;
		}

	} else {
		/* disabled -> enabled */
		*pint = TRUE;
		XtVaSetValues(w,
			  XtNlabel, "enabled",
			  NULL);
	}
	if (w == auth_toggle)
		SetAuthDisplay(*pint);
	ResetInactiveTimeout();
}

void SetAuthDisplay(bool)
int	bool;
{
	XtSetMappedWhenManaged(connectpopup->tuple[2].label, bool);
	XtSetMappedWhenManaged(connectpopup->tuple[3].label, bool);
	XtSetMappedWhenManaged(connectpopup->tuple[2].text, bool);
	XtSetMappedWhenManaged(connectpopup->tuple[3].text, bool);
	if (bool == TRUE) {
		textdisplay(&(connectpopup->tuple[2]),
			    (username == NULL) ? "" : username);
		password[0] = '\0';
		textdisplay(&(connectpopup->tuple[3]),"");
	}		
}

/* ARGSUSED */
void connectpopup_OK(w, client_data, call_data)
Widget	w;
caddr_t client_data;
caddr_t call_data;
{
	char	*str0, *str1, *str2;
	if (newauth != auth) 
		auth = newauth;

	XtVaGetValues (connectpopup -> tuple[0].text,
		       XtNstring, &str0,
		       NULL);
	XtVaGetValues (connectpopup -> tuple[2].text,
		       XtNstring, &str2,
		       NULL);
	if (str0[0] == '\0'
	    || (auth == TRUE && str2[0] == '\0')) {
		XBell(XtDisplay(w), 50);
		return;
	}
	
	XtVaGetValues (connectpopup -> tuple[1].text,
		       XtNstring, &str1,
		       NULL);
	if (str1[0] == '\0')
		tailorfile[0] = '\0';
	else
		strcpy(tailorfile, str1);

	if (str2[0] != '\0')
		strcpy(username, str2);
	fillin_passwdpep(username, password, auth);
	StartWait();
	Connect(str0);
	EndWait();
	ResetInactiveTimeout();
	Popdown(connectpopup);
}

/* ARGSUSED */
void QControlPopup(w, op, call_data)
Widget	w;
int	op;
caddr_t	call_data;
{
	int	x,y;
	Window	r_child;
	
	XTranslateCoordinates(XtDisplay(w),XtWindow(w),
			      XDefaultRootWindow(XtDisplay(w)),0,0,&x,&y,
			      &r_child);
	ResetInactiveTimeout();
	Popup(qcontrol, unknown, x, y);
	
}
extern int	msgs_ub;
/* ARGSUSED */
void MsgsShowAll (w, client_data, call_data)
Widget	w;
caddr_t	client_data,
	call_data;
{
	msgs_ub = False;
	control_display_msgs();
	msgs_ub = True;
}

/* ARGSUSED */
void QControl(w, op, call_data)
Widget	w;
int	op;
caddr_t	call_data;
{
	my_invoke(quecontrol, (char **) op);
	ResetInactiveTimeout();
	Popdown(qcontrol);
}

/*  */
extern Widget	channel_commands,
		mta_commands,
		msg_commands,
		qcontrol_command;

SensitizeButtons(bool)
int	bool;
{
	XtSetSensitive(qcontrol_command,bool);
	XtSetSensitive(channel_commands,bool);
	XtSetSensitive(mta_commands,bool);
	XtSetSensitive(msg_commands,bool);
}



