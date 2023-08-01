/* workops.c: routines that do work */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/workops.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/workops.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: workops.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"

extern CMD_TABLE	chantbl_commands[],
			mtatbl_commands[],
			msgtbl_commands[];
extern char		*strdup(), *itoa(),
			*stripstr(),
			*rcmd_srch();
extern void		ChanToggle(), MtaToggle(), MsgToggle(), ChangeMode();
extern struct recip	*find_recip();

/* ARGUSED */
ChanForce(chan)
struct chan_struct	*chan;
{
	char	*args[3];

	args[0] = chan->channelname;
	args[2] = NULLCP;
	if (chan->status->enabled != TRUE) {
		args[1] = rcmd_srch((int) chanstart, chantbl_commands);
		my_invoke(chanstart, args);
	}
	if (chan->status->cachedUntil != 0) {
		args[1] = rcmd_srch((int) chanclear, chantbl_commands);
		my_invoke(chanclear, args);
	}
}

extern struct chan_struct	*currentchan, **globallist;
extern int			num_channels;
extern struct mta_struct	*currentmta;
extern struct msg_struct	*currentmsg;

ChanNext()
{
	if (currentchan == NULL)
		currentchan = globallist[0];
	else {
		int i = 0;
		while (i < num_channels
		       && currentchan != globallist[i]) i++;
		if (i < num_channels-1)
			currentchan = globallist[i+1];
		else
			currentchan = globallist[0];
	}
	currentmta = NULL;
	currentmsg = NULL;
	refresh_control_channel();
}

ChanPrev()
{
	if (currentchan == NULL)
		currentchan = globallist[num_channels - 1];
	else {
		int i = 0;
		while (i < num_channels
		       && currentchan != globallist[i]) i++;
		if (i == 0)
			currentchan = globallist[num_channels - 1];
		else
			currentchan = globallist[i - 1];
	}
	currentmta = NULL;
	currentmsg = NULL;
	refresh_control_channel();
}

extern Widget	channel_label, mta_label, msg_label;
extern int	read_currentchan, chan_info_shown, mta_info_shown, msg_info_shown;

refresh_control_channel()
{	
	reset_label(channel_label);
	reset_label(mta_label);
	reset_label(msg_label);
	if (read_currentchan) {
		add_mta_refresh_list(currentchan->channelname);
		construct_event(mtaread);
	}
	if (chan_info_shown == TRUE)
		chan_display_info(currentchan);
}

ChanDownForce (chan)
struct chan_struct	*chan;
{
	add_mta_refresh_list(chan->channelname);
	my_invoke(mtaread, &(chan->channelname));
	ChanForce (chan);
}

/* ARGSUSED */
ChanControl(op, channel, mytime)
Operations op;
char	*channel,
	*mytime;
{
	char	*str,
		*chan,
		**args;
	str = strdup(channel);
	chan = stripstr(str);
	args = (char **) calloc(3, sizeof(char *));
	args[0] = chan;
	args[1] = rcmd_srch((int) op, chantbl_commands);
	args[2] = mytime;
	my_invoke(op, args);
	free((char *) args);
	free(str);
}

/* ARGSUSED */
MtaForce(mta, chan)
struct mta_struct	*mta;
struct chan_struct	*chan;
{
	char	*args[4];
	args[0] = chan->channelname;
	args[1] = mta->mta;
	args[3] = NULLCP;
	
	if (mta->status->enabled != TRUE) {
		args[2] = rcmd_srch((int) mtastart,mtatbl_commands);
		my_invoke(mtastart, args);
	}
	if (mta->status->cachedUntil != 0) {
		args[2] = rcmd_srch((int) mtaclear,mtatbl_commands);
		my_invoke(mtaclear, args);
	}
	ChanForce(chan);
}

extern char	*msginfo_args[];

MtaDownForce (chan, mta)
struct chan_struct	*chan;
struct mta_struct	*mta;
{
	msginfo_args[0] = chan->channelname;
	msginfo_args[1] = mta->mta;
	if (is_loc_chan(chan) == TRUE)
		msginfo_args[2] = (char *) 1;
	else
		msginfo_args[2] = (char *) 0;
	my_invoke (readchannelmtamessage, msginfo_args);
	/* reset after message down force */
	msginfo_args[0] = chan->channelname;
	msginfo_args[1] = mta->mta;
	if (is_loc_chan(chan) == TRUE)
		msginfo_args[2] = (char *) 1;
	else
		msginfo_args[2] = (char *) 0;
	if (mta->status->enabled != TRUE) {
		msginfo_args[2] = rcmd_srch((int) mtastart,mtatbl_commands);
		my_invoke(mtastart, msginfo_args);
	}
	if (mta->status->cachedUntil != 0) {
		msginfo_args[2] = rcmd_srch((int) mtaclear,mtatbl_commands);
		my_invoke(mtaclear, msginfo_args);
	}
}	

/* ARGSUSED */
MtaControl(op, givenmta, givenchan, time)
Operations	op;
char		*givenmta,
		*givenchan,
		*time;
{
	char	*str1,
		*str2,
		*mta,
		*chan,
		**args;

	/* get selection and all that */
	str1 = strdup(givenmta);
	mta = stripstr(str1);
	
	str2 = strdup(givenchan);
	chan = stripstr(str2);
	
	args = (char **) calloc(4, sizeof(char *));
	args[0] = chan;
	args[1] = mta;
	args[2] = rcmd_srch((int) op, mtatbl_commands);
	args[3] = time;
	my_invoke(op,args);
	free((char *) args);
	free(str1);
	free(str2);
}

/* ARGSUSED */
MsgForce(msg, usrs, mta, chan)
struct msg_struct	*msg;
char			*usrs;
struct mta_struct	*mta;
struct chan_struct	*chan;
{
	char	*usrlist[20],
		**args;
	int	numberUsrs = 0, force;
	int	i = 0, ix, all;
	struct recip	*temp;
	if (msg == NULL
	    || msg->msginfo == NULL) return;

	if (strcmp(usrs, "*") == 0) {
		all = TRUE;
		for (temp = msg->reciplist, numberUsrs = 0; 
		     temp != NULL; temp = temp -> next, numberUsrs++)
			continue;
	} else {
		all = FALSE;
		numberUsrs = sstr2arg(usrs, 20, usrlist, ",");
	}
	
	args = (char **) calloc((unsigned)(4+numberUsrs), sizeof(char *));
	args[0] = msg->msginfo->queueid;
	args[1] = NULL;

	ix = 3;
	force = FALSE;

	if (all == TRUE) {
		for (temp = msg->reciplist; temp != NULL; temp = temp -> next)
			if (temp->status
			    && temp->status->enabled != TRUE) {
				args[ix++] = itoa(temp->id);
				force = TRUE;
			}
	} else {
		while (i < numberUsrs && ix < (3+numberUsrs)) {
			if ((temp = find_recip(msg, atoi(usrlist[i]))) != NULL
			    && temp->status
			    && temp->status->enabled != TRUE) {
				args[ix++] = usrlist[i];
				force = TRUE;
			}
			i++;
		}
	}
	if (force == TRUE) {
		args[ix] = NULLCP;
		args[2] = rcmd_srch((int) msgstart, msgtbl_commands);
		my_invoke(msgstart, args);
	}

	ix = 3;
	i = 0;
	force = FALSE;

	if (all == TRUE) {
		for (temp = msg->reciplist; temp != NULL; temp = temp -> next)
			if (temp->status
			    && temp->status->cachedUntil != 0) {
				args[ix++] = itoa(temp->id);
				force = TRUE;
			}
	} else {
		while (i < numberUsrs && ix < (3+numberUsrs)) {
			if ((temp = find_recip(msg, atoi(usrlist[i]))) != NULL
			    && temp->status
			    && temp->status->cachedUntil != 0) {
				args[ix++] = usrlist[i];
				force = TRUE;
			}
			i++;
		}
	}
	if (force == TRUE) {
		args[ix] = NULLCP;
		args[2] = rcmd_srch((int) msgclear, msgtbl_commands);
		my_invoke(msgstart, args);
	}
	free((char *) args);
	MtaForce(mta, chan);
}

MsgDownForce (msg)
struct msg_struct	*msg;
{
	struct recip	*ix;
	int		recip_count = 0, i, j, force;
	char		**args;

	if (msg == NULL
	    || msg->msginfo == NULL) return;
	
	for (ix = msg -> reciplist; ix != NULL; 
	     recip_count++, ix = ix -> next)
		continue;
	
	args = (char **) calloc((unsigned)(4+recip_count),
				sizeof(char *));
	args[0] = msg->msginfo->queueid;
	args[1] = NULL;
	i = 3;
	ix = msg -> reciplist;
	force = FALSE;
	while (ix != NULL) {
		if (ix -> status
		    && ix -> status -> enabled != TRUE) {
			args[i++] = itoa(ix->id);

			force = TRUE;
		}
		ix = ix -> next;
	}
	if (force == TRUE) {
		args[i] = NULLCP;
		args[2] = rcmd_srch((int) msgstart, msgtbl_commands);
		my_invoke(msgstart, args);
	}
	for (j = 3; j < i; j++)
		if (args[j] != NULLCP) {
			free(args[j]);
			args[j] = NULLCP;
		}
	i = 3;
	ix = msg -> reciplist;
	force = FALSE;
	while (ix != NULL) {
		if (ix -> status
		    && ix -> status -> cachedUntil != 0) {
			args[i++] = itoa(ix->id);

			force = TRUE;
		}
		ix = ix -> next;
	}
	if (force == TRUE) {
		args[i] = NULLCP;
		args[2] = rcmd_srch((int) msgclear, msgtbl_commands);
		my_invoke(msgstart, args);
	}
	for (j = 3; j < i; j++)
		if (args[j] != NULLCP) {
			free(args[j]);
			args[j] = NULLCP;
		}
	free ((char *) args);
}

MsgControl(op, msg, usrs, time)
Operations	op;
char		*msg;
char		*usrs;
char		*time;
{
	char	*str0,
		*str1,
		*qid,
		*usrlist[20],
		**args;
	int	numberUsrs,
		i = 0,
		ix;

	/* arg1 = QID arg2 = time arg3 = control arg4.. = userlist */
	
	/* get selection and all that */
	str0 = strdup(msg);
	qid = stripstr(str0);

	str1 = strdup(usrs);
	numberUsrs = sstr2arg(str1, 20, usrlist, ",");
	
	args = (char **) calloc((unsigned)(4+numberUsrs), sizeof(char *));
	args[0] = qid;
	args[1] = time;
	args[2] = rcmd_srch((int) op, msgtbl_commands);
	ix = 3;
	while(i < numberUsrs && ix < (3+numberUsrs)) 
		args[ix++] = usrlist[i++];
	my_invoke(op, args);
	free((char *) args);
	free(str0);
	free(str1);
}

/*  */

extern struct chan_struct	*find_channel();

/* ARGSUSED */
ChanInfo(channel)
char	*channel;
{
	struct chan_struct	*chan;
	char	*channame,
		*str;

	str = strdup(channel);
	channame = stripstr(str);

	if ((chan = find_channel(channame)) != NULL) {
		chan_display_info(chan);
	} else {
		/* put out error message */
		PP_LOG(LLOG_EXCEPTIONS,
		       ("MTAcosnole confused: Unable to find '%s'",
			channame));
	}
	free(str);
}

extern struct mta_struct	*find_mta();

/* ARGSUSED */
MtaInfo(mtain, channel)
char	*mtain,
	*channel;
{
	struct chan_struct	*chan;
	struct mta_struct	*mta;
	char	*mtaname,
		*channame,
		*str1,
		*str2;

	str1 = strdup(channel);
	channame = stripstr(str1);
	str2 = strdup(mtain);
	mtaname = stripstr(str2);

	if (((chan = find_channel(channame)) != NULL)
	    && ((mta = find_mta(chan, mtaname)) != NULL)) {
		mta_display_info(chan, mta);
	} else {
		/* put out error message */
		PP_LOG(LLOG_EXCEPTIONS,
		       ("MTAconsole confused: unable to find channel/mta pair '%s/%s'",
			channame, mtaname));
	}
	free(str1);
	free(str2);

}

extern struct msg_struct	*find_msg();
/* ARGSUSED */
MsgInfo(msgin)
char	*msgin;
{
	struct msg_struct	*msg;
	char	*qid,
		*str;
	str = strdup(msgin);
	qid = stripstr(str);

	if ((msg = find_msg(qid)) != NULL)
		msg_display_info(msg);
	free(str);

}

/*  */
extern State	connectState;
extern int	userConnected,
		autoReconnect;
extern Widget	header, error;
extern char	*hostname;

/* ARGSUSED */
Disconnect()
{
	/* put out required message */
	XtVaSetValues(header,
		  XtNlabel, NO_CONNECTION,
		  NULL);
	XtSetMappedWhenManaged(error, False);
	userConnected = FALSE;
	TermRefreshTimeOut();
	TermConnectRetry();
	TermConnectTimeOut();
	ResetForDisconnect();
	my_invoke(disconnect, &hostname);
	SensitizeButtons(False);
}

/* ARGSUSED */
Connect(host)
char	*host;
{
	char	*str,
		*convertedstr;

	str = strdup(host);

	convertedstr = stripstr(host);

	if (strcmp(hostname, convertedstr) != 0) {
		free(hostname);
		hostname = strdup(convertedstr);
	}
	free(str);

	if (autoReconnect == TRUE) userConnected = TRUE;

	my_invoke(connect, &hostname);
}
		
/* ARGSUSED */
Quit()
{
	my_invoke(quit,(char **) NULL);
}

/*  */

extern struct chan_struct	*currentchan,
				**globallist;
extern struct mta_struct	*currentmta;
extern struct monitor_item	**display_list;
extern int			read_currentchan;
extern int			read_currentmta;
extern Widget			*channel_array;
extern int			num_channels;
extern Widget			channel_label;
extern Widget			mta_label;
extern Mode			mode;
extern struct msg_struct	*currentmsg;
extern Widget			msg_label;

/* ARGSUSED */
void curChan (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;
	ResetInactiveTimeout();
	while (i < num_channels
	       && channel_array[i] != w) i++;

	if (i < num_channels) {
		if (mode == monitor)
			ChangeMode((Widget)NULL, (caddr_t) NULL, 
				   (caddr_t) NULL);

		if (currentchan != globallist[i]) {
			read_currentmta = 0;
			currentmta = NULL;
			currentmsg = NULL;
			currentchan = globallist[i];
		}
	}
	reset_label(channel_label);
	reset_label(mta_label);
	reset_label(msg_label);
}

/* ARGSUSED */
void excl_curChan (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;
	ResetInactiveTimeout();
	while (i < num_channels
	       && channel_array[i] != w) i++;

	read_currentchan = 0;
	if (i < num_channels) {
		if (mode == monitor)
			ChangeMode((Widget)NULL, 
				   (caddr_t) NULL, (caddr_t) NULL);

		if (currentchan != NULL) 
			display_empty_mta_list(currentchan);
		display_empty_msg_list();

		if (currentchan == globallist[i]) 
			/* deselect it */
			currentchan = NULL;
		else 
			currentchan = globallist[i];

		currentmta = NULL;
		currentmsg = NULL;
		read_currentmta = 0;
		MtaToggle();
		MsgToggle();
	}
	reset_label(channel_label);
	reset_label(mta_label);
	reset_label(msg_label);
}

/* ARGSUSED */
void chanRefresh (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;
	struct chan_struct	*chan;

	while (i < num_channels && display_list[i]->chan != w)
		i++;
	
	if (i < num_channels) {
		chan = *(display_list[i]->channel);
		add_mta_refresh_list (chan->channelname);
		construct_event(mtaread);
	}
}
		
/* ARGSUSED */
void chanModeRead (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;

	while (i < num_channels && display_list[i]->chan != w)
		i++;
	
	if (i < num_channels) {
		read_currentchan = 1;
		currentchan = *(display_list[i]->channel);
		reset_label(channel_label);
		ChangeMode(w, (caddr_t) NULL, (caddr_t) NULL);
	}
}

/* ARGSUSED */
void chanMode (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;

	while (i < num_channels && display_list[i]->chan != w)
		i++;
	
	if (i < num_channels) {
		read_currentchan = 0;
		currentchan = *(display_list[i]->channel);
		reset_label(channel_label);
		ChangeMode(w, (caddr_t) NULL, (caddr_t) NULL);
	}
}

/* ARGSUSED */
void readChan (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	ResetInactiveTimeout();
	if (currentchan != NULL) {
		ChanToggle();
		MtaToggle();
		MsgToggle();
		read_currentchan = 1;
		if (mode == control) {
			clear_mta_refresh_list();
			add_mta_refresh_list(currentchan->channelname);
			construct_event(mtaread);
		}
	}
}
		
char				*msginfo_args[3];
extern struct mta_disp_struct	*mta_array;
int				actual_nmtas_present;

MtaNext()
{
	if (currentchan == NULL || 
	    actual_nmtas_present == 0)
		return;
	if (currentmta == NULL)
		currentmta = mta_array[0].mta;
	else {
		int i = 0;
		while (i < actual_nmtas_present
		       && currentmta != mta_array[i].mta) i++;
		if (i >= actual_nmtas_present-1)
			currentmta = mta_array[0].mta;
		else
			currentmta = mta_array[i+1].mta;
	}
	currentmsg = NULL;
	refresh_mta_control();
}

MtaPrev()
{
	if (currentchan == NULL || 
	    actual_nmtas_present == 0)
		return;
	if (currentmta == NULL)
		currentmta = mta_array[actual_nmtas_present-1].mta;
	else {
		int i = 0;
		while (i < actual_nmtas_present
		       && currentmta != mta_array[i].mta) i++;
		if (i == 0)
			currentmta = mta_array[actual_nmtas_present-1].mta;
		else
			currentmta = mta_array[i-1].mta;
	}
	currentmsg = NULL;
	refresh_mta_control();
}

refresh_mta_control()
{
	reset_label(mta_label);
	reset_label(msg_label);
	if (currentchan != NULL
	    && currentmta != NULL) {
		if (read_currentmta) {
			msginfo_args[0] = currentchan->channelname;
			msginfo_args[1] = currentmta->mta;
			if (currentchan->chantype == int_Qmgr_chantype_mts
			    && currentchan->outbound > 0)
				msginfo_args[2] = (char *) 1;
			else
				msginfo_args[2] = (char *) 0;
			my_invoke(readchannelmtamessage, msginfo_args);
		}

		if (mta_info_shown == TRUE)
			mta_display_info(currentchan, currentmta);
	}
}

/* ARGSUSED */
void curMta (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;
	ResetInactiveTimeout();
	while (i < actual_nmtas_present
	       && mta_array[i].widget != w) i++;

	if (i < actual_nmtas_present && currentmta != mta_array[i].mta) {
		currentmta = mta_array[i].mta;
		currentmsg = NULL;
	}
	reset_label(mta_label);
	reset_label(msg_label);
}

/* ARGSUSED */
void excl_curMta (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;
	ResetInactiveTimeout();
	while (i < actual_nmtas_present
	       && mta_array[i].widget != w) i++;
	read_currentmta = 0;
	
	if (i < actual_nmtas_present) {

		display_empty_msg_list();

		if (currentmta == mta_array[i].mta)   
			/* deselect it */
			currentmta = NULL;
		else
			currentmta = mta_array[i].mta;

		currentmsg = NULL;
		MsgToggle();
	}
	reset_label(mta_label);
	reset_label(msg_label);
}

/* ARGSUSED */
void mtaRefresh (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0,j = 0, found = 0;
	struct chan_struct	*chan;

	while (i < num_channels 
	       && found == 0) {
		j = 0;
		while (j < display_list[i]->num_mtas
		       && found == 0) {
			if (display_list[i]->mtas[j]->widget == w)
				found = 1;
			else
				j++;
		}
		if (found == 0) 
			i++;
	}
	if (found == 1) {
		chan = *(display_list[i]->channel);
		add_mta_refresh_list (chan->channelname);
		construct_event(mtaread);
	}
}

/* ARGSUSED */
void mtaModeRead (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0,j = 0, found = 0;

	while (i < num_channels 
	       && found == 0) {
		j = 0;
		while (j < display_list[i]->num_mtas
		       && found == 0) {
			if (display_list[i]->mtas[j]->widget == w)
				found = 1;
			else
				j++;
		}
		if (found == 0) 
			i++;
	}
	if (found == 1) {
		read_currentchan = 1;
		currentchan = *(display_list[i]->channel);
		read_currentmta = 1;
		currentmta = display_list[i]->mtas[j]->mta;
		ChangeMode(w, (caddr_t) NULL, (caddr_t) NULL);
	}
}

/* ARGSUSED */
void mtaMode (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0,j = 0, found = 0;

	while (i < num_channels 
	       && found == 0) {
		j = 0;
		while (j < display_list[i]->num_mtas
		       && found == 0) {
			if (display_list[i]->mtas[j]->widget == w)
				found = 1;
			else
				j++;
		}
		if (found == 0) 
			i++;
	}
	if (found == 1) {
		read_currentchan = 1;
		currentchan = *(display_list[i]->channel);
		read_currentmta = 0;
		currentmta = display_list[i]->mtas[j]->mta;
		ChangeMode(w, (caddr_t) NULL, (caddr_t) NULL);
	}
}

/* ARGSUSED */
void readMta (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	ResetInactiveTimeout();
	MtaToggle();
	MsgToggle();
	if (currentchan != NULL && currentmta != NULL) {
		msginfo_args[0] = currentchan->channelname;
		read_currentmta = 1;
		msginfo_args[1] = currentmta->mta;
		if (currentchan->chantype == int_Qmgr_chantype_mts
		    && currentchan->outbound > 0)
			msginfo_args[2] = (char *) 1;
		else
			msginfo_args[2] = (char *) 0;
		my_invoke(readchannelmtamessage, msginfo_args);
	}
}

extern struct msg_disp_struct	*msg_array;
extern int			actual_nmsgs_present;


MsgNext()
{
	if (actual_nmsgs_present == 0)
		return;
	if (currentmsg == NULL)
		currentmsg = msg_array[0].msg;
	else {
		int i = 0;
		while (i < actual_nmsgs_present
		       && msg_array[i].msg != currentmsg) i++;
		
		if (i >= actual_nmsgs_present-1)
			currentmsg = msg_array[0].msg;
		else
			currentmsg = msg_array[i+1].msg;
	}
	refresh_msg_control();
}
			       
MsgPrev()
{
	if (actual_nmsgs_present == 0)
		return;
	if (currentmsg == NULL)
		currentmsg = msg_array[actual_nmsgs_present-1].msg;
	else {
		int i = 0;
		while (i < actual_nmsgs_present
		       && msg_array[i].msg != currentmsg) i++;
		
		if (i == 0)
			currentmsg = msg_array[actual_nmsgs_present-1].msg;
		else
			currentmsg = msg_array[i-1].msg;
	}
	refresh_msg_control();
}
			       
refresh_msg_control()
{
	reset_label(msg_label);
	if (currentmsg && msg_info_shown)
		msg_display_info(currentmsg);
}

/* ARGSUSED */
void curMsg (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;
	ResetInactiveTimeout();
	while (i < actual_nmsgs_present
	       && msg_array[i].widget != w) i++;

	if (i < actual_nmsgs_present) {
		currentmsg = msg_array[i].msg;
	}
	reset_label(msg_label);
}

/* ARGSUSED */
void excl_curMsg (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	i = 0;
	ResetInactiveTimeout();
	while (i < actual_nmsgs_present
	       && msg_array[i].widget != w) i++;

	if (i < actual_nmsgs_present) {
		if (currentmsg == msg_array[i].msg) {   
			/* deselect it */
			MsgToggle();
			currentmsg = NULL;
		} else 
			currentmsg = msg_array[i].msg;
	}
	reset_label(msg_label);
}

/* ARGSUSED */
void readMsg (w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	if (currentmsg != NULL)
		msg_display_info(currentmsg);
}
