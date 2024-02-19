/* display.c : display routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/display.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/display.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: display.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"
extern Pixmap			backpixmap;
extern Mode			mode;
extern int			max_horiz_mtas;
extern State			connectState;
extern char			*time_t2RFC(),
				*vol2str(),
				*itoa(),
				*msginfo_args[3],
				*Qinformation,
				*Qversion;
extern time_t			time();
void				ChanToggle(),
				MtaToggle(),
				MsgToggle();
extern unsigned long		chancolourOf(),
				mtacolourOf(),
				msgcolourOf();
extern XFontStruct		*chanFont(),
				*mtaFont(),
				*msgFont(),
				*normalFont,
				*disabledFont;
extern Heuristic heuristic;
/*  */
/* channels */
extern void			Channel();
extern struct chan_struct	*currentchan,
				**globallist;
extern Widget			channel_all,
				channels,
				channel_info,
				channel_viewport,
				channel_label,
				monitor_form,
				monitor_viewport,
				switchform,
				qversion;
extern int 			max_vert_lines,
				read_currentchan,
				monitor_form_managed;
Widget				*channel_array = NULL;
extern int		actual_nchans_present = 0,
				num_channels,
				chan_info_shown = FALSE;
extern struct monitor_item	**display_list;
extern int			compat;

static void redisplay_channel();
static void monitor_display_channels();
static void undisplay_monitor();
static char *create_channel_display_string();
static char *create_channel_monitor_string();
static char *time_t2str();
static void resize_info_list();

extern char		*reverse_mta(), *reverse_adr();
extern int	total_volume, delta_volume,
	total_number_messages, delta_messages, 
	total_number_reports, delta_reports;
extern Widget	total_number_label, total_volume_label;
extern unsigned long	volcolourOf(), numcolourOf();

display_totals()
{
	char	buf[BUFSIZ], num[15];

	if (total_volume == 0
	    || (total_number_messages == 0
	    && total_number_reports == 0)) {
		MapVolume(False);
		return;
	}
	num2unit(total_volume, num);
	(void) sprintf(buf, "Volume = %s", num);
	if (!compat && delta_volume) {
		num2unit(delta_volume, num);
		(void) sprintf(buf, "%s (%s%s)",
			       buf,
			       (delta_volume > 0) ? "+" : "",
			       num);
	}

	XtVaSetValues(total_volume_label,
		  XtNlabel,	buf,
		  XtNborderColor,	volcolourOf(total_volume),
		  NULL);

	(void) sprintf(buf, "%d msg%s",
		       total_number_messages,
		       (total_number_messages == 1) ? "" : "s");
	if (!compat && delta_messages)
		(void) sprintf(buf, "%s (%s%d)",
			       buf, 
			       (delta_messages > 0) ? "+" : "",
			       delta_messages);
	(void) sprintf(buf, "%s + %d report%s", 
		       buf, total_number_reports,
		       (total_number_reports == 1) ? "" : "s");
	if (!compat && delta_reports)
		(void) sprintf(buf, "%s (%s%d)",
			       buf,
			       (delta_reports > 0) ? "+" : "",
			       delta_reports);

	XtVaSetValues(total_number_label,	
		  XtNlabel,	buf,
		  XtNborderColor,	numcolourOf(total_number_messages+total_number_reports),
		  NULL);
	MapVolume(True);
}
	
char *create_channel_header()
{
	char	*str;
	char	buf[BUFSIZ];
	if (currentchan == NULL)
		str = strdup("No current channel");
	else {
		sprintf(buf, "Current channel : %s : %s", 
			currentchan->channelname, currentchan->channeldescrip);
		str = strdup(buf);
	}
	return str;
}

#define ssformat	"%-*s %s"
#define sdformat	"%-*s %s"
#define	plus_ssformat	"%s\n%-*s %s"
#define plus_sdformat	"%s\n%-*s %d"

#define tab 30

extern Widget	channel_all_list;

char	*channel_info_list[100];

set_info_list(list, ix, str)
char	*list[];
int	ix;
char	*str;
{
	if (list[ix]) free(list[ix]);
	list[ix] = str;
}

chan_display_info(chan)
struct chan_struct	*chan;
{
	char		*str, buf[BUFSIZ];
	int		ix = 0;
	XFontStruct	*font;

	set_info_list(channel_info_list, ix++, strdup(chan->channelname));
	set_info_list(channel_info_list, ix++, strdup(chan->channeldescrip));
	if (chan->status->cachedUntil != 0)  {
		set_info_list(channel_info_list, ix++, strdup("Delayed until"));
		str = time_t2RFC(chan->status->cachedUntil);
		set_info_list(channel_info_list, ix++, str);
	}
	if (chan->oldestMessage != 0 && 
	    (chan->numberMessages != 0 || chan->numberReports != 0)) {
		set_info_list(channel_info_list, ix++, strdup("Oldest message"));
		str = time_t2str(time((time_t *)0) - chan->oldestMessage);
		set_info_list(channel_info_list, ix++, str);
	}
		
	if (chan->numberMessages != 0) {
		set_info_list(channel_info_list, ix++, strdup("number of messages"));
		str = itoa(chan->numberMessages);
		set_info_list(channel_info_list, ix++, str);
	}

	if (chan->numberReports != 0) {
		set_info_list(channel_info_list, ix++, strdup("number of reports"));
		str = itoa(chan->numberReports);
		set_info_list(channel_info_list, ix++, str);
	}

	if (chan->volumeMessages != 0) {
		char	num[15];
		set_info_list(channel_info_list, ix++, strdup("Volume"));
		num2unit(chan->volumeMessages, num);
		str = strdup(num);
		set_info_list(channel_info_list, ix++, str);
	}
		
	if (chan->numberActiveProcesses != 0) {
		set_info_list(channel_info_list, ix++, strdup("number of active processes"));
		str = itoa(chan->numberActiveProcesses);
		set_info_list(channel_info_list, ix++, str);
	}

	set_info_list(channel_info_list, ix++, strdup("status"));

	set_info_list(channel_info_list, ix++, 
		      strdup((chan->status->enabled == TRUE) ? "enabled" : "disabled"));

	if (chan->status->lastAttempt != 0) {
		set_info_list(channel_info_list, ix++, strdup("last attempt"));
		str = time_t2RFC(chan->status->lastAttempt);
		set_info_list(channel_info_list, ix++, str);
	}

	if (chan->status->lastSuccess != 0) {
		set_info_list(channel_info_list, ix++, strdup("last success"));
		str = time_t2RFC(chan->status->lastSuccess);
		set_info_list(channel_info_list, ix++, str);
	}

	set_info_list(channel_info_list, ix++, strdup("priority"));
	switch (chan->priority) {
	    case int_Qmgr_Priority_low:
		str = strdup("low");
		break;
	    case int_Qmgr_Priority_high:
		str = strdup("high");
		break;
	    case int_Qmgr_Priority_normal:
	    default:
		str = strdup("normal");
		break;
	}
	set_info_list(channel_info_list, ix++, str);

	set_info_list(channel_info_list, ix++, strdup("maximum number of processes"));
	if (chan -> maxprocs == 0)
		sprintf(buf, "unlimited");
	else
		sprintf(buf, "%d", chan->maxprocs);
	set_info_list(channel_info_list, ix++, strdup(buf));

	XawListChange(channel_all_list,
		      channel_info_list,
		      ix, 0, True);

	if ((font = chanFont(chan)) != NULL)
		XtVaSetValues(channel_all_list,
			      XtNfont,	font,
			      NULL);

	XtVaSetValues(channel_info,
		      XtNlabel, "all",
		      NULL);

	XtSetMappedWhenManaged(channel_all, True);
	XtSetMappedWhenManaged(channel_viewport, False);
	chan_info_shown = TRUE;
}


chan_display_all()
{
	XtVaSetValues(channel_info,
		  XtNlabel, "info",
		  NULL);
	XtSetMappedWhenManaged(channel_viewport, True);
	XtSetMappedWhenManaged(channel_all, False);
	chan_info_shown = FALSE;
}


display_channels()
{
	clear_mta_refresh_list();
	control_display_channels();
	if (mode == monitor)
		monitor_display_channels();
	if (compat)
		display_totals();
}

control_display_channels()
{
	int			i = 0;
	char			*str = NULL;
	XFontStruct		*font;
	for (i = 0; i < num_channels; i++)
		XtSetMappedWhenManaged(channel_array[i], False);
	i = 0;
	while (i < num_channels) {
		str = create_channel_display_string(globallist[i]);
		XtVaSetValues(channel_array[i],
			  XtNlabel, str,
			  XtNborderColor, chancolourOf(globallist[i]),
			  XtNborderWidth, chanborderOf(globallist[i]),
			  XtNbackgroundPixmap, 
			  (globallist[i]->status->enabled == FALSE) ? backpixmap : ParentRelative,
			  NULL);
		if ((font = chanFont(globallist[i])) != NULL)
			XtVaSetValues(channel_array[i],
				  XtNfont, font,
				  NULL);

		free(str);
		i++;
	}
	for (i = 0; i < num_channels; i++)
		XtSetMappedWhenManaged(channel_array[i], True);
	if (chan_info_shown == TRUE
	    && currentchan != NULL)
		chan_display_info(currentchan);
	if (currentchan != NULL && mode == control && read_currentchan != 0) {
		clear_mta_refresh_list();
		if (heuristic != chanonly) {
			add_mta_refresh_list(currentchan->channelname);
			construct_event(mtaread);
		}
	}
}

/* called when channel number of msgs gets out of step with total */
/* mtas number of msgs i.e. msgs go out or come in between chanread */
/* and subsequent mtareads */

redisplay_control_channel(num)
int	num;
{
	char	*str = NULL;

	XtSetMappedWhenManaged(channel_array[num], False);
	str = create_channel_display_string(globallist[num]);
	XtVaSetValues(channel_array[num],
		  XtNlabel, str,
		  XtNborderColor, chancolourOf(globallist[num]),
		  XtNborderWidth, chanborderOf(globallist[num]),
		  XtNbackgroundPixmap,(globallist[num]->status->enabled == FALSE) ? backpixmap : ParentRelative,
		  NULL);
	free(str);
	XtSetMappedWhenManaged(channel_array[num], True);
}

static void monitor_display_channels()
{
	int num_displayed = 0,
		i;

	order_display_channels();
	XtUnmanageChild(monitor_form);
	while ((num_displayed < num_channels) 
	       && (chanBadness(*(display_list[num_displayed]->channel)) != 0)) {
		monitor_channel(num_displayed);
		num_displayed++;
	}
	
	i = num_displayed;
	
	while (i < num_channels
	       && display_list[i]->chan != NULL) {
		undisplay_monitor(display_list[i]);
		i++;
	}
	XtManageChild(monitor_form);
	construct_event(mtaread);
}

static void undisplay_monitor(item)
struct monitor_item	*item;
{
	int	i = 0;
	if (item->form != NULL) {
		XtDestroyWidget(item->form);
		item -> chan = NULL;
		if (item->mtas != NULL) {
			while (i < item -> num_allocd
			       && item->mtas[i] != NULL) {
				item->mtas[i] = NULL;
				i++;
			}
		}
		item->box = NULL;
		item->form = NULL;
	}
}	

monitor_channel(num)
int	num;
{
	
	struct chan_struct	*actualchan;
	int			managed = TRUE;
	char			*str;
	XFontStruct		*font;
	/* display display_list[num] */
	actualchan = *(display_list[num]->channel);
	actualchan->display_num = num;

	XawFormDoLayout (monitor_form, False);

	if (connectState == connected) {
		if (display_list[num]->form == NULL) {
			/* need to create form */	
			display_list[num]->form = XtVaCreateWidget("MonitorChannelForm",
							       formWidgetClass,
							       monitor_form,
							       XtNborderColor, chancolourOf(actualchan),
							       XtNfromVert, (num == 0) ? NULL : display_list[num-1]->form,
							       XtNfromHoriz, NULL,
							       XtNresizable, TRUE,
							       NULL);
			managed = FALSE;
		}
		if (display_list[num]->chan != NULL) {
			XtUnmanageChild(display_list[num]->chan);
			str = create_channel_monitor_string(actualchan);
			XtVaSetValues(display_list[num]->chan,
				  XtNborderColor, chancolourOf(actualchan),
				  XtNborderWidth, chanborderOf(actualchan),
				  XtNlabel, 	str,
				  XtNbackgroundPixmap, (actualchan->status->enabled == FALSE) 
				  ? backpixmap : ParentRelative,
				  NULL);
			if ((font = chanFont(actualchan)) != NULL)
				XtVaSetValues(display_list[num]->chan,
					  XtNfont,	font,
					  NULL);

			XtManageChild(display_list[num]->chan);
			free(str);
		} else {
			str = create_channel_monitor_string(actualchan);
			display_list[num]->chan =
				XtVaCreateManagedWidget("MonitorChannel",
							labelWidgetClass,
							display_list[num]->form,
							XtNresizable, TRUE,
							XtNborderColor, chancolourOf(actualchan),
							XtNborderWidth, chanborderOf(actualchan),
							XtNlabel, str,
							XtNbackgroundPixmap, (actualchan->status->enabled == FALSE) 
							? backpixmap : ParentRelative,
							XtNfromVert, NULL,
							XtNfromHoriz, NULL,
							XtNleft,	XtChainLeft,
							XtNright,	XtChainLeft,
							XtNtop,	XtChainTop,
							XtNbottom,	XtChainTop,
							XtNjustify, XtJustifyRight,
							NULL);
			if ((font = chanFont(actualchan)) != NULL)
				XtVaSetValues(display_list[num]->chan,
					  XtNfont,	font,
					  NULL);
			free(str);

		}
		if (managed == FALSE)
			XtManageChild(display_list[num]->form);
		XawFormDoLayout(display_list[num]->form, False);
		XtVaSetValues(display_list[num]->form,
			  XtNborderColor, chancolourOf(actualchan),
			  NULL);
		if (heuristic != chanonly
		    && (actualchan->numberMessages > 0 
		    || actualchan->numberReports > 0
		    || actualchan->volumeMessages > 0))
			add_mta_refresh_list(actualchan->channelname);
	}
	if (display_list[num]->box != NULL)
		XtUnmanageChild(display_list[num]->box);
	XawFormDoLayout(display_list[num]->form, True);
	XawFormDoLayout(monitor_form, True);
}

static char *create_channel_display_string(chan)
struct chan_struct	*chan;
{
	char	str[BUFSIZ];

	sprintf(str,"%s",chan->channelname);
	if (chan -> numberActiveProcesses > 1)
		sprintf(str, "%s (%d)",
			str, chan->numberActiveProcesses);
	sprintf(str,"%s: %d",
		str,chan->numberMessages);
	if (chan->numberReports != 0)
	  sprintf(str,"%s + %d", str, chan->numberReports);
	if (chan->given_num_mtas != 0)
		sprintf(str,"%s/%d",str, chan->given_num_mtas);
	return strdup(str);
}

static char *create_channel_monitor_string(chan)
struct chan_struct	*chan;
{
	char	str[BUFSIZ];

	sprintf(str,"%s : %d %s",
		chan->channelname,
		chan->numberMessages,
		(chan->numberMessages == 1) ? "msg" : "msgs");

	if (chan->numberReports != 0)
		sprintf(str, "%s and %d %s",
		  str,
		  chan->numberReports,
		  (chan->numberReports == 1) ? "DR" : "DRs");

	sprintf(str, "%s on %d %s",
		str,
		chan->given_num_mtas,
		(chan->given_num_mtas == 1) ? "mta" : "mtas");

	if (chan -> numberActiveProcesses > 0) {
		sprintf (str, "%s, %d process%s active", str,
			 chan -> numberActiveProcesses,
			 chan -> numberActiveProcesses > 1 ? "es" : "");
	}

	if (chan->deltaMessages || chan->deltaReports || 
	    chan->deltaMtas || chan->deltaVolume) {
		sprintf(str, "%s, net change:", str);
		if (chan->deltaMessages)
			sprintf(str, "%s %s%d msg%s", str,
				(chan->deltaMessages > 0) ? "+" : "",
				chan->deltaMessages, (chan->deltaMessages == 1 || chan->deltaMessages == -1) ? "" : "s");
		if (chan->deltaReports)
			sprintf(str, "%s %s%d report%s", str,
				(chan->deltaReports > 0) ? "+" : "",
				chan->deltaReports, (chan->deltaReports == 1 || chan->deltaReports == -1) ? "" : "s");
		if (chan->deltaMtas)
			sprintf(str, "%s %s%d mta%s", str,
				(chan->deltaMtas > 0) ? "+" : "",
				chan->deltaMtas, (chan->deltaMtas == 1 || chan->deltaMtas == -1) ? "" : "s");

		if (chan->deltaVolume) {
			char	num[15];
			num2unit(chan->deltaVolume, num);
			sprintf(str, "%s (%s%s)", str,
				(chan->deltaVolume > 0) ? "+" : "",
				num);
		}
	}
	return strdup(str);
}

resize_chan_array(num)
int	num;	/* number of channels that will be displayed */
{
	int	i = num;
	char	*str;

	XtSetMappedWhenManaged(channels, False); 
	while (i < actual_nchans_present) {
		XtUnmanageChild(channel_array[i]);
		i++;
	}
	
	XSync(XtDisplay(channels), False);

	i = num;
	while (i < actual_nchans_present) {
		XtDestroyWidget(channel_array[i]);
		i++;
	}

	if (num == 0) {
		if (channel_array != NULL) {
			free ((char *) channel_array);
			channel_array = NULL;
		}
	} else if (actual_nchans_present == 0)
		/* haven't got any so malloc first lot */
		channel_array = (Widget *) calloc((unsigned) num, 
						  sizeof(Widget));
	else if (actual_nchans_present != num)
		/* need some more */
		channel_array = (Widget *) realloc((char *) channel_array, 
						   (unsigned) (num * sizeof(Widget)));
	while (actual_nchans_present < num) {
		str = itoa(actual_nchans_present);
		channel_array[actual_nchans_present] = 
			XtVaCreateManagedWidget(NULL,
				     labelWidgetClass,
				     channels,
				     XtNborderWidth,	2,
				     XtNlabel, str,
				     NULL);
		actual_nchans_present++;
		free(str);
	}
	actual_nchans_present = num;
	XtSetMappedWhenManaged(channels, True); 
}

/*  */
/* mtas */
extern struct mta_struct	*currentmta;
extern int			read_currentmta;
extern int			max_mta_border;
extern void			Mta();
extern Widget			mtas,
				mta_all,
				mta_all_list,
				mta_info,
				mta_viewport,
				mta_label;

struct mta_disp_struct		*mta_array = NULL;
int				actual_nmtas_present = 0,
				num_mtas_displayed = 0,
				mta_info_shown = FALSE;
static char			*create_mta_display_string();
extern int			mta_info_strlen;
extern int			uk_order;

char	*mta_info_list[100];

mta_display_info(chan,mta)
struct chan_struct	*chan;
struct mta_struct	*mta;
{
	char		*str;
	char		buf[BUFSIZ];
	int		ix = 0;
	XFontStruct	*font;

	str = (uk_order) ? reverse_mta(mta->mta) : strdup(mta->mta);
	set_info_list(mta_info_list, ix++, str);
	
	sprintf(buf, "on channel '%s'", chan->channelname);
	set_info_list(mta_info_list, ix++, strdup(buf));

	if (mta->status->cachedUntil != 0) {
		set_info_list(mta_info_list, ix++, strdup("Delayed until"));
		str = time_t2RFC(mta->status->cachedUntil);
		set_info_list(mta_info_list, ix++, str);
	}

	if (mta->info != NULLCP) {
		set_info_list(mta_info_list, ix++, strdup("Error Information"));
		set_info_list(mta_info_list, ix++, strdup(mta->info));
	}
		      
	set_info_list(mta_info_list, ix++, strdup("Oldest message"));
	str = time_t2str(time((time_t *) 0) - mta->oldestMessage);
	set_info_list(mta_info_list, ix++, str);

	if (mta->numberMessages != 0) {
		set_info_list(mta_info_list, ix++, strdup("Number of messages"));
		str = itoa(mta->numberMessages);
		set_info_list(mta_info_list, ix++, str);
	}
	if (mta->numberReports != 0) {
		set_info_list(mta_info_list, ix++, strdup("Number of reports"));
		str = itoa(mta->numberReports);
		set_info_list(mta_info_list, ix++, str);
	}

	set_info_list(mta_info_list, ix++, strdup("Volume"));
	str = vol2str(mta->volumeMessages);
	set_info_list(mta_info_list, ix++, str);

	if (mta -> active) {
		set_info_list(mta_info_list, ix++, strdup("active"));
		set_info_list(mta_info_list, ix++, strdup("processes running"));
	}
	
	set_info_list(mta_info_list, ix++, strdup("Status"));
	set_info_list(mta_info_list, ix++,
		      strdup((mta->status->enabled == TRUE) ? "enabled" : "disabled"));

	if (mta->status->lastAttempt != 0) {
		set_info_list(mta_info_list, ix++, strdup("Last attempt"));
		str = time_t2RFC(mta->status->lastAttempt);
		set_info_list(mta_info_list, ix++, str);
	}

	if (mta->status->lastSuccess != 0) {
		set_info_list(mta_info_list, ix++, strdup("Last success"));
		str = time_t2RFC(mta->status->lastSuccess);
		set_info_list(mta_info_list, ix++, str);
	}

	set_info_list(mta_info_list, ix++, strdup("Priority"));
	switch (mta->priority) {
	    case int_Qmgr_Priority_low:
		str = strdup("low");
		break;
	    default:
	    case int_Qmgr_Priority_normal:
		str = strdup("normal");
		break;
	    case int_Qmgr_Priority_high:
		str = strdup("high");
		break;
	}
	set_info_list(mta_info_list, ix++, str);

	XawListChange(mta_all_list,
		      mta_info_list,
		      ix, 0, True);

	if ((font = mtaFont(mta)) != NULL)
		XtVaSetValues(mta_all_list,
			      XtNfont, font,
			      NULL);

	XtVaSetValues(mta_info,
		  XtNlabel, "all",
		  NULL);
	XtSetMappedWhenManaged(mta_all, True);
	XtSetMappedWhenManaged(mta_viewport, False);
	mta_info_shown = TRUE;
}

mta_display_all()
{
	XtVaSetValues(mta_info,
		  XtNlabel, "info",
		  NULL);
	XtSetMappedWhenManaged(mta_viewport, True);
	XtSetMappedWhenManaged(mta_all, False);
	mta_info_shown = FALSE;
}

display_empty_mta_list(chan)
struct chan_struct	*chan;
{
	/* reset counters */
	if (chan != NULL && chan->mtalist != NULL)
		free_mta_list(&(chan->mtalist), &(chan->num_mtas));
	XtUnmanageChild(mtas);
	XtSetMappedWhenManaged(mtas, False);
	resize_mta_array(0);
	XtSetMappedWhenManaged(mtas, True);
	XtManageChild(mtas);
	reset_label(mta_label);
	MtaToggle();
	MsgToggle();
}
	
display_mtas(chan)
struct chan_struct	*chan;
{
	if (mode == monitor)
		monitor_display_mtas(chan);
	else
		control_display_mtas(chan);
}

control_display_mtas(chan)
struct	chan_struct	*chan;
{
	int	i;
	char	*str;
	XFontStruct	*font;

	XtSetMappedWhenManaged(mtas, False);
	XtUnmanageChild(mtas);

	resize_mta_array(chan->num_mtas);

	i = 0;
	while (i < num_mtas_displayed) {
		mta_array[i].mta = chan->mtalist[i];
		str = create_mta_display_string(mta_array[i].mta);
		XtSetMappedWhenManaged(mta_array[i].widget, True);
		XtUnmanageChild(mta_array[i].widget);
		XtVaSetValues(mta_array[i].widget,
			  XtNlabel, str,
			  XtNborderColor, mtacolourOf(mta_array[i].mta),
			  XtNborderWidth, mtaborderOf(mta_array[i].mta),
			  XtNfromVert, (i == 0) ? NULL : mta_array[i-1].widget,
			  XtNbackgroundPixmap,
			  (mta_array[i].mta->status->enabled == FALSE) ? backpixmap : ParentRelative,
			  NULL);
		if ((font = mtaFont(mta_array[i].mta)) != NULL)
			XtVaSetValues(mta_array[i].widget,
				  XtNfont, font,
				  NULL);

		XtManageChild(mta_array[i].widget);
		free(str);
		i++;
	}
	if (mta_info_shown == TRUE
	    && currentchan != NULL
	    && currentmta != NULL)
		mta_display_info(currentchan, currentmta);
	if (currentchan != NULL
	    && read_currentmta != 0
	    && currentmta != NULL) {
		msginfo_args[0] = currentchan->channelname;
		msginfo_args[1] = currentmta->mta;
		if (is_loc_chan(currentchan) == TRUE)
			/* local */
			msginfo_args[2] = (char *) 1;
		else
			msginfo_args[2] = (char *) 0;
		construct_event(readchannelmtamessage);
	} else 
		display_empty_msg_list();
	XtManageChild(mtas);
	XtSetMappedWhenManaged(mtas, True);
}

int	oldnum;
extern int	percent,
		lower_bound_mtas;

static void resize_and_zero (pbuf, pnum, num, size)
char	**pbuf;
int	*pnum;
int	num;
unsigned size;
{
	if (*pbuf == NULLCP)
		*pbuf = calloc((unsigned)num, size);
	else {
		*pbuf = realloc(*pbuf,
				(unsigned) (num * size));
		if (num > *pnum)
			bzero (((*pbuf) + (*pnum)*size),
			       (unsigned) (num-(*pnum))*size);
	}
	*pnum = num;
}
			

		
monitor_display_mtas(chan)
struct chan_struct	*chan;
{
	int 	num_displayed = 0,
		line_num = 0,
		bigestBorderWidth = 0, horiz;
	int	max_allowed, num_alloced;
	int num_vert_lines = 0, max_num;

	Dimension	vp_len, chan_len, dist, longest_len,
			line_len = 0;
	Widget		abovewidget = NULL;
	int		temp;

	XawFormDoLayout(monitor_form, False);
	XawFormDoLayout(display_list[chan->display_num]->form, False);
	XtVaSetValues(monitor_form,
		  XtNleft,	XtRubber,
		  XtNright,	XtRubber,
		  XtNtop,	XtRubber,
		  XtNbottom,	XtRubber,
		  NULL);
	XtVaSetValues(display_list[chan->display_num]->form,
		  XtNleft,	XtRubber,
		  XtNright,	XtRubber,
		  XtNtop,	XtRubber,
		  XtNbottom,	XtRubber,
		  NULL);

	if (display_list[chan->display_num]->box == NULL) {
		display_list[chan->display_num]->box =
			XtVaCreateWidget("MonitorMTAForm",
					 formWidgetClass,
					 display_list[chan->display_num]->form,
					 XtNresizable,	TRUE,
					 XtNborderWidth,	0,
					 XtNfromHoriz,	NULL,
					 XtNfromVert,	display_list[chan->display_num]->chan,
					 XtNbackgroundPixmap, ParentRelative,
					 XtNvertDistance,	0,
					 NULL);
	}
	XtVaSetValues(display_list[chan->display_num]->box,
		  XtNleft,	XtRubber,
		  XtNright,	XtRubber,
		  XtNtop,	XtRubber,
		  XtNbottom,	XtRubber,
		  NULL);

	XawFormDoLayout(display_list[chan->display_num]->box, False);

	XtVaGetValues(switchform,
		      XtNwidth,	&vp_len,
		      NULL);
	XtVaGetValues(display_list[chan->display_num]->chan,
		      XtNwidth,	&chan_len,
		      NULL);
	XtVaGetValues(display_list[chan->display_num]->form,
		      XtNdefaultDistance, &temp,
		      NULL);
	XtVaGetValues(display_list[chan->display_num]->box,
		      XtNhorizDistance,	&horiz,
		      NULL);
	dist = (Dimension) temp;
	longest_len = vp_len-3*dist-horiz-20;
	/* -20 is fudge for scrollbar */

	switch (heuristic) {
	    case chanonly:
		break;
	    case percentage:
		max_allowed = (chan->num_mtas * chanBadness(chan) * percent) / (max_bad_channel * 100);
		num_alloced = (max_allowed < lower_bound_mtas) ? lower_bound_mtas : max_allowed;

		if (display_list[chan->display_num]->num_allocd < num_alloced)
			resize_and_zero (&(display_list[chan->display_num]->mtas),
					 &display_list[chan->display_num]->num_allocd,
					 num_alloced,
					 sizeof(Mta_disp_struct *));
		       
		while (num_displayed < chan->num_mtas
		       && mtaBadness(chan->mtalist[num_displayed]) != 0
		       && (num_displayed < lower_bound_mtas
			   || num_displayed < max_allowed)) {
			if (monitor_mta(chan->display_num, 
					chan->mtalist[num_displayed], 
					num_displayed,
					&line_num,
					&line_len,
					&abovewidget,
					longest_len,
					dist,
					max_allowed,
					&bigestBorderWidth) == OK)
				num_displayed++;
		}
		break;
	    case line:
		num_vert_lines = NumVertLines(chan);
		if (num_vert_lines <= 0) num_vert_lines = 1;
		max_num = num_vert_lines * max_horiz_mtas;
		if (max_num == 0) max_num++;

		if (display_list[chan->display_num]->num_allocd < max_num) 
			resize_and_zero(&(display_list[chan->display_num]->mtas),
					&(display_list[chan->display_num]->num_allocd),
					max_num,
					sizeof(Mta_disp_struct *));

		while (num_displayed < max_num 
		       && line_num < num_vert_lines
		       && (num_displayed < chan->num_mtas)
		       && (mtaBadness(chan->mtalist[num_displayed]) != 0)) {
			if (monitor_mta(chan->display_num, 
					chan->mtalist[num_displayed], 
					num_displayed,
					&line_num,
					&line_len,

					&abovewidget,
					longest_len,
					dist,
					num_vert_lines, 
					&bigestBorderWidth) == OK)
				num_displayed++;
		}
		break;
	    case all:
		if (display_list[chan->display_num]->num_allocd <
		    chan->num_mtas) 
			resize_and_zero(&(display_list[chan->display_num]->mtas),
					&(display_list[chan->display_num]->num_allocd),
					chan->num_mtas,
					sizeof(Mta_disp_struct *));

		while (num_displayed < chan->num_mtas
		       && mtaBadness(chan->mtalist[num_displayed]) != 0) {
			if (monitor_mta(chan->display_num, 
					chan->mtalist[num_displayed], 
					num_displayed,
					&line_num,
					&line_len,

					&abovewidget,
					longest_len,
					dist,
					num_vert_lines, 
					&bigestBorderWidth) == OK)
				num_displayed++;
		}
		break;
	}
			
							 
	oldnum = display_list[chan->display_num]->num_mtas;

	display_list[chan->display_num]->num_mtas = num_displayed;
	
	while (num_displayed < display_list[chan->display_num] -> num_allocd
	       && display_list[chan->display_num]->mtas[num_displayed] != NULL
	       && display_list[chan->display_num]->mtas[num_displayed]->widget != NULL) {
		undisplay_mta_monitor(&(display_list[chan->display_num]->mtas[num_displayed]->widget));
		num_displayed++;
	}

	if (display_list[chan->display_num]->num_mtas == 0) {
		undisplay_mta_monitor(&(display_list[chan->display_num]->box));
		XawFormDoLayout(display_list[chan->display_num]->form, True);
		XtVaSetValues(display_list[chan->display_num]->form,
			  XtNleft,	XtChainLeft,
			  XtNright,	XtChainLeft,
			  XtNtop,	XtChainTop,
			  XtNbottom,	XtChainTop,
			  NULL);
		XawFormDoLayout(monitor_form, True);
		XtVaSetValues(monitor_form,
			  XtNleft,	XtChainLeft,
			  XtNright,	XtChainLeft,
			  XtNtop,	XtChainTop,
			  XtNbottom,	XtChainTop,
			  NULL);
		return;
	}
	XawFormDoLayout(display_list[chan->display_num]->box, True);
	
	XtManageChild(display_list[chan->display_num]->box);
	XtVaSetValues(display_list[chan->display_num]->box,
		  XtNleft,	XtChainLeft,
		  XtNright,	XtChainLeft,
		  XtNtop,	XtChainTop,
		  XtNbottom,	XtChainTop,
		  NULL);
	XawFormDoLayout(display_list[chan->display_num]->form, True);
	XtVaSetValues(display_list[chan->display_num]->form,
		  XtNleft,	XtChainLeft,
		  XtNright,	XtChainLeft,
		  XtNtop,	XtChainTop,
		  XtNbottom,	XtChainTop,
		  NULL);
	XawFormDoLayout(monitor_form, True);
	XtVaSetValues(monitor_form,
		  XtNleft,	XtChainLeft,
		  XtNright,	XtChainLeft,
		  XtNtop,	XtChainTop,
		  XtNbottom,	XtChainTop,
		  NULL);
}

undisplay_mta_monitor(pwidget)
Widget	*pwidget;
{
	if (*pwidget != NULL) {
/*		XtDestroyWidget(*pwidget);
		*pwidget = NULL;
*/
		if (XtIsManaged (*pwidget))
			XtUnmanageChild (*pwidget);
	}
}

monitor_mta(chan_display_num,
	    mta, 
	    num, 
	    pline_num,
	    pline_len,
	    pvert,
	    longest,
	    dist,
	    linesAllowed,
	    borderWidth_ub)
int			chan_display_num;
struct mta_struct	*mta;
int			num;
int			*pline_num;
Dimension		*pline_len;
Widget			*pvert;
Dimension		longest,
			dist;
int			linesAllowed;
int			*borderWidth_ub;
{
	char	*str;
	Widget	horiz = NULL;
	Widget *w;
	Dimension	widget_width;
	Dimension	borderWidth = mtaborderOf(mta);
	XFontStruct	*font;

	str = create_mta_display_string(mta);
	
	if (display_list[chan_display_num]->mtas[num] == NULL)
		/* need to create widget */
		display_list[chan_display_num]->mtas[num] = 
			(Mta_disp_struct *) calloc(1, sizeof(Mta_disp_struct));

	display_list[chan_display_num]->mtas[num]->mta = mta;

	w = &display_list[chan_display_num]->mtas[num]->widget;
	if (*w == NULL) {
		*w = XtVaCreateManagedWidget("MonitorMTA",
					     labelWidgetClass,
					     display_list[chan_display_num] ->box,
					     XtNresizable,	TRUE,
					     XtNborderWidth,	borderWidth,
					     XtNlabel, str,
					     XtNborderColor, mtacolourOf(mta),
					     XtNbackgroundPixmap,
					     (mta->status->enabled == FALSE) ? backpixmap : ParentRelative,
					     XtNleft,	XtChainLeft,
					     XtNright,	XtChainLeft,
					     XtNtop,	XtChainTop,
					     XtNbottom,	XtChainTop,
					     NULL);
	} else {
		XtVaSetValues (*w,
			       XtNborderWidth,	borderWidth,
			       XtNlabel, str,
			       XtNborderColor, mtacolourOf(mta),
			       XtNbackgroundPixmap,
			       (mta->status->enabled == FALSE ?
				backpixmap : ParentRelative),
			       NULL);
	}
	free(str);
	
	if ((font = mtaFont(mta)) != NULL)
		XtVaSetValues(*w,
			      XtNfont,  font,
			      NULL);

	XtVaGetValues(*w,
		      XtNwidth,	&widget_width,
		      NULL);

	if (*pline_len+dist+widget_width > longest) {
		/* new line */
		(*pline_num)++;
		if (heuristic == line && (*pline_num) >= linesAllowed) {
			undisplay_mta_monitor(w);
			return NOTOK;
		}
		if (num == 0) /* problems ? */
			*pvert = NULL;
		else
			*pvert = display_list[chan_display_num]->
				mtas[num-1]->widget;
		horiz = NULL;
		(*pline_len) = dist+widget_width;
	} else {
		if (num == 0)
			horiz = NULL;
		else
			horiz = display_list[chan_display_num]->
				mtas[num-1]->widget;
		(*pline_len) += dist+widget_width;
	}

	XtVaSetValues(*w,
		      XtNfromVert,	*pvert,
		      XtNfromHoriz,	horiz,
		      NULL);
	if (!XtIsManaged (*w))
		XtManageChild(*w);
	if ((int) borderWidth > *borderWidth_ub)
		*borderWidth_ub = borderWidth;
	return OK;
}

char	*create_mta_header()
{
	/* takes from current channel and current mta */
	char	*str;
	char	*ch;
	char	buf[BUFSIZ];
	if (currentmta == NULL
	    || currentchan == NULL)
		str = strdup("No current mta");
	else {
		ch = (uk_order) ? 
			reverse_mta(currentmta -> mta) : currentmta -> mta;
		sprintf(buf, "Current mta : %s on %s", 
			ch, currentchan->channelname);
		if (uk_order && ch != currentmta -> mta) free(ch);
		str = strdup(buf);
	}

	return str;
}

static char *create_mta_display_string(mta)
struct mta_struct	*mta;
{
	char	str[BUFSIZ];
	char	*ch;

	ch = (uk_order)? reverse_mta(mta->mta) : mta->mta;
	sprintf(str, "%s : %d",ch, mta->numberMessages);
	if (uk_order && ch != mta->mta) free(ch);
	if (mta->numberReports != 0)
	  sprintf(str, "%s + %d", str, mta->numberReports);
	return strdup(str);
}

resize_mta_array(num)
int	num;	/* new number of mtas */
{

	int	i = num;

	while (i < actual_nmtas_present) {
		XtSetMappedWhenManaged(mta_array[i].widget, False);
		XtUnmanageChild(mta_array[i].widget);
		XtDestroyWidget(mta_array[i].widget);
		i++;
	}
	if (num == 0) {
		if (mta_array != NULL) {
			free((char *) mta_array);
			mta_array = NULL;
		}
	} else if (actual_nmtas_present == 0)
		mta_array = (struct mta_disp_struct *)
			calloc((unsigned int) num,
			       sizeof(struct mta_disp_struct));
	else if (num != actual_nmtas_present)
		mta_array = (struct mta_disp_struct *)
			realloc((char *) mta_array,
				(unsigned int) (num * sizeof(struct mta_disp_struct)));
	while (actual_nmtas_present < num) {
		mta_array[actual_nmtas_present].mta = NULL;
		mta_array[actual_nmtas_present].widget =
			XtVaCreateManagedWidget(NULL,
				     labelWidgetClass,
				     mtas,
				     XtNresizable,	TRUE,
				     XtNleft,	XtChainLeft,
				     XtNright,	XtChainLeft,
				     XtNtop,	XtChainTop,
				     XtNbottom,	XtChainTop,
				     XtNborderWidth,	2,
				     NULL);
		actual_nmtas_present++;
	}
	actual_nmtas_present = num;
	num_mtas_displayed = num;
}

/*  */
/* msgs */
extern struct msg_struct	*currentmsg,
				**global_msg_list;
extern int			max_msg_border;
extern void			Msg();
extern Widget			msg_all,
				msg_all_list,
				msgs,
				msg_info,
				msg_viewport,
				msg_label;
extern int		       	number_msgs,
				msg_info_strlen;

struct msg_disp_struct		*msg_array = NULL;
int				actual_nmsgs_present = 0,
				num_msgs_displayed = 0,
				msg_info_shown = FALSE;
static char			*create_msg_display_string();

char	**msg_info_list;
int	msg_info_list_size = 0;

msg_display_info(msg)
struct msg_struct	*msg;
{
	struct recip	*ix;
	char		*str, buf[BUFSIZ];
	XFontStruct	*font;
	int		i = 0;

	if (msg_info_list_size == 0) 
		resize_info_list(&msg_info_list, &msg_info_list_size);

	set_info_list(msg_info_list, i++, strdup(msg->msginfo->queueid));
	set_info_list(msg_info_list, i++, strdup(" "));

	set_info_list(msg_info_list, i++, strdup("Originator"));
	str = (uk_order) ? reverse_adr(msg->msginfo->originator) 
		: strdup(msg->msginfo->originator);
	set_info_list(msg_info_list, i++, str);

	if (msg->msginfo->size != 0) {
		set_info_list(msg_info_list, i++, strdup("Size"));
		num2unit(msg->msginfo->size, buf);
		set_info_list(msg_info_list, i++, strdup(buf));
	}
	if (msg->msginfo->uaContentId != NULLCP) {
		set_info_list(msg_info_list, i++, strdup("UA content ID"));
		set_info_list(msg_info_list, i++, strdup(msg->msginfo->uaContentId));
	}

	if (msg->msginfo->inChannel != NULLCP) {
		set_info_list(msg_info_list, i++, strdup("Inbound channel"));
		set_info_list(msg_info_list, i++, strdup(msg->msginfo->inChannel));
	}
	
	ix = msg->reciplist;
	if (ix != NULL) ix = ix->next;
	while (ix != NULL) {
		/* make sure have enough space for next recip */
		if (i + 20 >= msg_info_list_size)
			resize_info_list(&msg_info_list, &msg_info_list_size);

		set_info_list(msg_info_list, i++, strdup(" "));
		set_info_list(msg_info_list, i++, strdup(" "));

		set_info_list(msg_info_list, i++, strdup("To"));
		str = (uk_order) ? reverse_adr(ix -> recipient) 
			: strdup(ix -> recipient);
		sprintf(buf, "%s (id %d)", str, ix->id);
		free(str);
		set_info_list(msg_info_list, i++, strdup(buf));

		if (ix->info != NULLCP) {
			set_info_list(msg_info_list, i++, strdup("Error Information"));
			set_info_list(msg_info_list, i++, strdup(ix->info));
		}

		if (ix->chansOutstanding == NULL) {
			set_info_list(msg_info_list, i++, strdup(" "));
			set_info_list(msg_info_list, i++, strdup("awaiting DRs"));
		} else {
			set_info_list(msg_info_list, i++, strdup("Remaining channels"));
			set_info_list(msg_info_list, i++, strdup(ix->chansOutstanding));
		}

		if (ix->status->cachedUntil != 0) {
			set_info_list(msg_info_list, i++, strdup("Delayed until"));
			str = time_t2RFC(ix->status->cachedUntil);
			set_info_list(msg_info_list, i++, str);
		}

		set_info_list(msg_info_list, i++, strdup("Status"));
		set_info_list(msg_info_list, i++,
			      strdup((ix->status->enabled == TRUE) ? "enabled" : "disabled"));
		
		if (ix->status->lastAttempt != 0) {
			set_info_list(msg_info_list, i++, strdup("Last attempt"));
			str = time_t2RFC(ix->status->lastAttempt);
			set_info_list(msg_info_list, i++, str);
		}

		if (ix->status->lastSuccess != 0) {
			set_info_list(msg_info_list, i++, strdup("Last success"));
			str = time_t2RFC(ix->status->lastSuccess);
			set_info_list(msg_info_list, i++, str);
		}
		ix = ix->next;
	}
	/* make sure have enough space for rest of info */
	if (i + 10 >= msg_info_list_size)
		resize_info_list(&msg_info_list, &msg_info_list_size);

	set_info_list(msg_info_list, i++, strdup(" "));
	set_info_list(msg_info_list, i++, strdup(" "));

	if (msg->msginfo->contenttype != NULLCP) {
		set_info_list(msg_info_list, i++, strdup("Content type"));
		set_info_list(msg_info_list, i++, strdup(msg->msginfo->contenttype));
	}

	if (msg->msginfo->eit != NULL) {
		set_info_list(msg_info_list, i++, strdup("Eits"));
		set_info_list(msg_info_list, i++, strdup(msg->msginfo->eit));
	}

	set_info_list(msg_info_list, i++, strdup("Age"));
	str = time_t2str(time((time_t *) 0) - msg->msginfo->age);
	set_info_list(msg_info_list, i++, str);

	if (msg->msginfo->expiryTime != 0) {
		set_info_list(msg_info_list, i++, strdup("Expiry time"));
		str = time_t2RFC(msg->msginfo->expiryTime);
		set_info_list(msg_info_list, i++, str);
	}

	if (msg->msginfo->deferredTime != 0) {
		set_info_list(msg_info_list, i++, strdup("Deferred until"));
		str = time_t2RFC(msg->msginfo->deferredTime);
		set_info_list(msg_info_list, i++, str);
	}
	
	set_info_list(msg_info_list, i++, strdup("Priority"));
	switch (msg->msginfo->priority) {
	    case int_Qmgr_Priority_low:
		str = strdup("low");
		break;
	    case int_Qmgr_Priority_normal:
	    default:
		str = strdup("normal");
		break;
	    case int_Qmgr_Priority_high:
		str = strdup("high");
		break;
	}
	set_info_list(msg_info_list, i++, str);

	if (msg->msginfo->errorCount != 0) {
		set_info_list(msg_info_list, i++, strdup("Number of errors"));
		str = itoa(msg->msginfo->errorCount);
		set_info_list(msg_info_list, i++, str);
	}


	XawListChange(msg_all_list,
		      msg_info_list,
		      i, 0, True);

	XtVaSetValues(msg_info,
		  XtNlabel, "all",
		  NULL);

	XtSetMappedWhenManaged(msg_all, True);
	XtSetMappedWhenManaged(msg_viewport, False);
	if ((font = msgFont(msg)) != NULL)
		XtVaSetValues(msg_info,
			  XtNfont, font,
			  NULL);
	msg_info_shown = TRUE;
}

msg_display_all()
{
	XtVaSetValues(msg_info,
		  XtNlabel, "info",
		  NULL);
	XtSetMappedWhenManaged(msg_viewport, True);
	XtSetMappedWhenManaged(msg_all, False);
	msg_info_shown = FALSE;
}

display_msgs()
{
	if (mode == control)
		control_display_msgs();
}

int display_empty_msg_list()
{
	int	i;

	for (i = 0; i < actual_nmsgs_present; i++) {
		msg_array[i].msg = NULL;
		XtSetMappedWhenManaged(msg_array[i].widget, False);
	}
	currentmsg = NULL;
	reset_label(msg_label);
	MtaToggle();
	MsgToggle();
}

int	control_msgs_ub = 10, 
	msgs_ub = True;
extern	Widget	msgs_showall;

control_display_msgs()
{
	int	i;
	char	*str;
	XFontStruct	*font;
	XtUnmanageChild(msgs);
	XawFormDoLayout(msgs, False);

	for (i = 0; i < actual_nmsgs_present; i++)
		XtSetMappedWhenManaged(msg_array[i].widget, False);
	resize_msg_array((msgs_ub == True && number_msgs > control_msgs_ub) ? control_msgs_ub : number_msgs);

	i = 0;
	while (i < num_msgs_displayed) {
		msg_array[i].msg = global_msg_list[i];
		str = create_msg_display_string(global_msg_list[i]);
		XtSetMappedWhenManaged(msg_array[i].widget, True);
		XtUnmanageChild(msg_array[i].widget);
		font = msgFont(msg_array[i].msg);
		XtVaSetValues(msg_array[i].widget,
			  XtNlabel, str,
			  XtNbackgroundPixmap,
			  (font != NULL && font == disabledFont) ? backpixmap : ParentRelative,
			  XtNborderColor, msgcolourOf(msg_array[i].msg),
			  XtNborderWidth, msgborderOf(msg_array[i].msg),
			  NULL);
		if (font != NULL)
			XtVaSetValues(msg_array[i].widget,
				  XtNfont, font,
				  NULL);

		XtManageChild(msg_array[i].widget);
		free(str);
		i++;
	}
	if (msgs_ub == True && num_msgs_displayed < number_msgs)
		XtSetMappedWhenManaged(msgs_showall, True);
	else
		XtSetMappedWhenManaged(msgs_showall, False);

	if (msg_info_shown == TRUE
	    && currentmsg != NULL)
		msg_display_info(currentmsg);
	XawFormDoLayout(msgs, True);
	XtManageChild(msgs);
}

monitor_display_msgs()
{
}

char	*create_msg_header()
{
	/* takes from current msg */
	char	*str;
	char	buf[BUFSIZ];
	if (currentmsg == NULL)
		str = strdup("No current message");
	else {
		sprintf(buf, "Current msg : %s", 
			currentmsg->msginfo->queueid);
		str = strdup(buf);
	}

	return str;
}

resize_msg_array(num)
int	num;	/* new number of msgs */
{
	int	i = num;
/*	if (control_msgs_ub < actual_nmsgs_present)*/
		
	while (i < actual_nmsgs_present) {
		XtSetMappedWhenManaged(msg_array[i].widget, False);
		XtDestroyWidget(msg_array[i].widget);
		i++;
	}

	if (num == 0) {
		if (msg_array != NULL) {
			free ((char *) msg_array);
			msg_array = NULL;
		}
	} else if (actual_nmsgs_present == 0)
		msg_array = (struct msg_disp_struct *)
			calloc((unsigned int) num,
			       sizeof(struct msg_disp_struct));
	else if (actual_nmsgs_present != num)
		msg_array = (struct msg_disp_struct *)
			realloc((char *) msg_array,
				 (unsigned int) (num * sizeof(struct msg_disp_struct)));

	while (actual_nmsgs_present < num) {
		msg_array[actual_nmsgs_present].msg = NULL;
		msg_array[actual_nmsgs_present].widget =
			XtVaCreateManagedWidget(NULL,
				     labelWidgetClass,
				     msgs,
				     XtNfromVert, (actual_nmsgs_present == 0) ? NULL : msg_array[actual_nmsgs_present-1].widget,
				     XtNfromHoriz,	NULL,
				     XtNresizable,	TRUE,
				     XtNborderWidth,	2,
				     XtNleft,	XtChainLeft,
				     XtNright,	XtChainLeft,
				     XtNtop,	XtChainTop,
				     XtNbottom,	XtChainTop,
				     NULL);
		actual_nmsgs_present++;
	}
	actual_nmsgs_present = num;
	num_msgs_displayed = num;
}

static char	*create_msg_display_string(msg)
struct msg_struct	*msg;
{
	char	str[BUFSIZ];
	char	*ch1, *ch2;

	if (msg->reciplist == NULL
	    || msg->reciplist->next == NULL)
		/* delivery report */
		sprintf(str,"dr (%s) to %s",
			msg->msginfo->queueid,
			msg->msginfo->originator);
	else {
		struct recip	*ix = msg->reciplist;
		int		found = 0;

		while (ix != NULL && found == 0) {
			if (ix->actChan != NULL 
			    && currentchan != NULL 
			    && strcmp(ix->actChan, 
				      currentchan->channelname) == 0) {
				/* channel matches */
				if (is_loc_chan(currentchan) == TRUE) { 
					if (ix->recipient && currentmta
					    && strcmp(ix->recipient, currentmta->mta) == 0)
						found = 1;
				} else if (ix->mta != NULL 
					   && currentmta != NULL
					   && strcmp(ix->mta, 
						     currentmta->mta) == 0)
					/* mta matches */
					found = 1;
			}
			if (found == 0)
				ix = ix->next;
		}
		if (ix != NULL && ix->id == 0) {
			/* delivery report ? */
			ch1 = (uk_order) ? reverse_adr(ix -> recipient)
				: ix -> recipient;
			sprintf(str, "dr (%s) to %s",
				msg->msginfo->queueid,
				ch1);
			if (uk_order && ch1 != ix -> recipient) free(ch1);
		} else {
			if (ix == NULL) ix = msg->reciplist->next;

			ch1 = (uk_order) ? reverse_adr(ix -> recipient) : ix -> recipient;
			ch2 = (uk_order) ? reverse_adr(msg->msginfo->originator) : msg->msginfo->originator;

			sprintf(str, "%s to %s from %s",
				msg->msginfo->queueid,
				ch1, ch2);
			if (uk_order) {
				if (ch1 != ix -> recipient) free(ch1);
				if (ch2 != msg->msginfo->originator) free(ch2);
			}
		}
	}

	return strdup(str);
}

reset_label(hdr)
Widget	hdr;
{
	char *str = NULL;
	XFontStruct	*font = normalFont;
	if (hdr == channel_label) {
		str = create_channel_header();
		font = chanFont(currentchan);
	} else if (hdr == mta_label) {
		str = create_mta_header();
		font = mtaFont(currentmta);
	} else if (hdr == msg_label) {
		str = create_msg_header();
		font = msgFont(currentmsg);
	}
	XtVaSetValues(hdr,
		  XtNlabel, str,
		  NULL);
	if (font != NULL)
		XtVaSetValues(hdr,
			  XtNfont, font,
			  NULL);

	if (str) free(str);
}	

/*  */
extern Widget	connect_command,
		refresh_command;
extern struct tailor *tailors;
extern void	tailor_free();

ResetForDisconnect()
{
	/* change disconnect to connect */
	XtVaSetValues(connect_command,
		  XtNlabel, "connect",
		  NULL);
	XtVaSetValues(refresh_command,
		  XtNlabel, "refresh",
		  NULL);
	if (tailors) {
		tailor_free(tailors);
		tailors = NULL;
	}
	MapButtons(False);
	MapVolume(False);
	undisplay_time_label();

	if (connectState == connected)
		clear_displays();
	if (Qinformation != NULLCP) {
		free(Qinformation);
		Qinformation = NULLCP;
	}
	if (Qversion != NULLCP) {
		free(Qversion);
		Qversion = NULLCP;
		XtVaSetValues(qversion,
			  XtNlabel,	"???",
			  NULL);
	}
}

clear_displays()
{
	/* unmap all displays */
	/* deal with channel display */
	clear_monitor();
	clear_channel_display();
	clear_control();
	
}

clear_monitor()
{
	int 	i = 0,
		j = 0;
	XawFormDoLayout(monitor_form, False);
	if (display_list != NULL) {
		while (i < num_channels) {
			if (display_list[i] != NULL) {
				if (display_list[i]->form)
					XawFormDoLayout(display_list[i]->form, False);		
				if (display_list[i]->box)
					XawFormDoLayout(display_list[i]->box, False);
				if (display_list[i]->chan != NULL) {
					XtDestroyWidget(display_list[i]->chan);
					j = 0;
					while (j < display_list[i] -> num_allocd
					       && display_list[i]->mtas[j] != NULL) {
						if (display_list[i]->mtas[j]->widget != NULL)
							XtDestroyWidget(display_list[i]->mtas[j]->widget);
						free((char *) display_list[i]->mtas[j]);
						j++;
					}
					if (display_list[i]->mtas)
						free((char *) display_list[i]->mtas);
				}
				if (display_list[i]->box != NULL)
					XtDestroyWidget(display_list[i]->box);
				if (display_list[i]->form != NULL)
					XtDestroyWidget(display_list[i]->form);
			}
		free((char *) display_list[i]);
		i++;
		}
		free((char *) display_list);
		display_list = NULL;
	}
	XawFormDoLayout(monitor_form, True);
}

clear_control()
{
	/* destructive clear removing channel, mta, msg displays */
	/* and freeing globallist */
	clear_mta_display();
	clear_msg_display();
}

clear_channel_display()
{
	int	i;
	free_channel_list();
	currentchan = NULL;
	resize_chan_array(0);
	reset_label(channel_label);
	ChanToggle();
	for (i = 0; i < actual_nchans_present; i++) {
		if (channel_array[i] != NULL) 
			XtSetMappedWhenManaged(channel_array[i], False);
	}
}

clear_mta_display()
{
	int	i;

	XtSetMappedWhenManaged(mta_viewport, False);
	XtSetMappedWhenManaged(mtas, False);
	XtUnmanageChild(mtas);
	resize_mta_array(0);
	XtManageChild(mtas);
	XtSetMappedWhenManaged(mtas, True);
	XtSetMappedWhenManaged(mta_viewport, True);
	currentmta = NULL;
	read_currentmta = 0;
	reset_label(mta_label);
	MtaToggle();
	for (i = 0; i < actual_nmtas_present; i++) {
		if (mta_array[i].widget != NULL) 
			XtSetMappedWhenManaged(mta_array[i].widget, False);
	}
}

clear_msg_display()
{
	int	i;

	resize_msg_array(0);
	currentmsg = NULL;
	reset_label(msg_label);
	MsgToggle();
	if (msg_info_shown == TRUE)
		msg_display_all();
	for (i = 0; i < actual_nmsgs_present; i++) {
		if (msg_array[i].widget != NULL)
			XtSetMappedWhenManaged(msg_array[i].widget, False);
	}
}

void ChanToggle()
{
	if (chan_info_shown == TRUE)
		chan_display_all();
}

void MtaToggle()
{
	if (mta_info_shown == TRUE)
		mta_display_all();
}

void MsgToggle()
{
	if (msg_info_shown == TRUE)
		msg_display_all();
}

toggle_info_displays()
{
	ChanToggle();
	MtaToggle();
	MsgToggle();
}

/*  */
textdisplay(tuple, str)
Popup_tuple	*tuple;
char	*str;
{
	XtVaSetValues (tuple -> text, XtNstring, str == NULLCP ? "" : str, 0);
}

extern Widget	top;
terminate_display()
{
	XtDestroyWidget(top);
/*	XtDestroyWidget(popup);*/
}

update_channel_from_mtas(chan, nmsgs, ndrs, nmtas, volume)
struct chan_struct	*chan;
int			nmsgs,
  			ndrs,
			nmtas,
			volume;
{
	int	changed = FALSE;
	
	if (chan -> numberMessages != nmsgs) {
		if (compat)
			total_number_messages 
				+= nmsgs - chan -> numberMessages;
		chan -> numberMessages = nmsgs;
		changed = TRUE;
	}
	
	if (chan -> numberReports != ndrs) {
		if (compat)
			total_number_reports += ndrs - chan -> numberReports;
		chan -> numberReports = ndrs;
		changed = TRUE;
	}

	if (chan -> num_mtas != nmtas) {
		chan -> num_mtas = nmtas;
		changed = TRUE;
	}
	if (chan -> volumeMessages != volume) {
		if (compat)
			total_volume += volume - chan -> volumeMessages;
		chan -> volumeMessages = volume;
		changed = TRUE;
	}

	if (chan->given_num_mtas != chan -> num_mtas) {
		chan -> given_num_mtas = chan->num_mtas;
		changed = TRUE;
	}


	if (changed == TRUE) {
		redisplay_channel(chan);
		if (compat)
			display_totals();
	}
}

static void redisplay_channel(chan)
struct chan_struct	*chan;
{
	int	i = 0;
	char	*str;

	/* do control display */
	while (i < num_channels && globallist[i] != chan)
		i++;

	if (i < num_channels) {
		str = create_channel_display_string(globallist[i]);
		XtVaSetValues(channel_array[i],
			  XtNlabel, str,
			  XtNborderColor, chancolourOf(globallist[i]),
			  XtNborderWidth, chanborderOf(globallist[i]),
			  XtNbackgroundPixmap,
			  (globallist[i]->status->enabled == FALSE) ? backpixmap : ParentRelative,
			  NULL);
		free(str);
	}

	if (mode == monitor) {
		/* do monitor display */
		str = create_channel_monitor_string(chan);
		XtVaSetValues(display_list[chan->display_num] ->  chan,
			  XtNlabel, str,
			  XtNborderColor, chancolourOf(chan),
			  XtNborderWidth, chanborderOf(chan),
			  XtNbackgroundPixmap, (chan->status->enabled == FALSE) ? backpixmap : ParentRelative,
			  NULL);
		free(str);
	}
}

static char	*time_t2str(in)
time_t  in;
{
	char	buf[BUFSIZ];
	time_t	result;
	buf[0] = '\0';
	
	if (in < 0)
		return strdup("still in the womb");

	if ((result = in / (60 * 60 * 24)) != 0) {
		sprintf(buf, "%d day%s",
			result,
			(result == 1) ? "" : "s");
		in = in % (60 * 60 * 24);
	}

	if ((result = in / (60 * 60)) != 0) {
		sprintf(buf, 
			(buf[0] == '\0') ? "%s%d hr%s" : "%s %d hr%s",
			buf, 
			result,
			(result == 1) ? "" : "s");
		in = in % (60 * 60);
	}
	if ((result = in / 60) != 0) {
		sprintf(buf, 
			(buf[0] == '\0') ? "%s%d min%s" : "%s %d min%s",
			buf, 
			result,
			(result == 1) ? "" : "s");
		in = in % 60;
	}

	if (buf[0] == '\0' && in != 0)
		sprintf(buf, "%d sec%s", 
			in,
			(in == 1) ? "" : "s");
	if (buf[0] == '\0')
		sprintf(buf, "just born");
	return strdup(buf);
}

static void	resize_info_list(plist, psize)
char		***plist;
int		*psize;
{
	if (*psize == 0) {
		*psize += BUFSIZ;
		*plist = (char **) calloc(*psize, sizeof(char *));
	} else {
		*psize += BUFSIZ;
		*plist = (char **) realloc(*plist, 
					   (unsigned) (*psize) * sizeof(char *));
	}
}
