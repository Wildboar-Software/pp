/* ops.c: encoding and decoding routines for qmgr interaction */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/ops.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/ops.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: ops.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"
#include	"Qmgr-types.h"
#include	<isode/rosy.h>

extern Widget			channel_label,
				mta_label,
				msg_label;
extern struct chan_struct	**globallist,
				*currentchan,
				*find_channel();
extern char			*mystrtotime();
extern int			forceDown;

time_t	boottime = 0;
int	messagesIn = 0, messagesOut = 0, addrIn = 0, addrOut = 0, 
	maxChans = 0, currChans = 0;
double  opsPerSec = 0.0, runnableChans = 0.0, 
	msgsInPerSec = 0.0, msgsOutPerSec = 0.0;

/*  */
/* channel operations */

#define 	CHAN_READ_INTERVAL	60	/* in secs */

CMD_TABLE	chantbl_commands [] = {
	"start",	(int) chanstart,
	"stop",		(int) chanstop,
	"clear",	(int) chanclear,
	"info",		(int) chaninfo,
	"cache add",	(int) chancacheadd,
	0,		0
	};

/* ARGSUSED */
int do_channelread (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_UNIV_UTCTime	**arg;
{
	char	*str;
	UTC	utc;
	      
	utc = (UTC) malloc (sizeof(struct UTCtime));
	utc->ut_flags = UT_SEC;

	utc->ut_sec = CHAN_READ_INTERVAL;

	str = utct2str(utc);
	*arg = str2qb (str, strlen(str), 1);
	return OK;
}

/* ARGSUSED */
int do_channelcontrol (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_Qmgr_ChannelControl	**arg;
/* args[0] = channel */
/* args[1] = stop,start,clear,cacheadd */
/* args[2] = time */
{
	char	*timestr;
	*arg = (struct type_Qmgr_ChannelControl *) malloc(sizeof(**arg));
	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	(*arg)->channel = str2qb(args[0],
				 strlen(args[0]),
				 1);
	switch (cmd_srch(args[1], chantbl_commands)) {
	    case chanstart:
		(*arg)->control->offset = type_Qmgr_Control_start;
		break;
	    case chanstop:
		(*arg)->control->offset = type_Qmgr_Control_stop;
		break;
	    case chanclear:
		(*arg)->control->offset = type_Qmgr_Control_cacheClear;
		break;
	    case chancacheadd:
		(*arg)->control->offset = type_Qmgr_Control_cacheAdd;
		timestr = mystrtotime(args[2]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
		break;
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console : '%s' unknown control param for channels",
			args[1]));
		return NOTOK;
	}
	return OK;
}

extern time_t	time();

extern int		firstChanRead;
extern Widget		top, channel_viewport;

/* ARGSUSED */
int channelcontrol_result (ad, id, dummy, result, roi)
int							ad,
							id,
							dummy;
register struct type_Qmgr_PrioritisedChannelList	*result;
struct RoSAPindication					*roi;
{
	Dimension	width,
			height;
	extern time_t	currentTime;
	time(&currentTime);
	channel_list(result);

	if (globallist != NULL) {
		display_channels();
		if (firstChanRead == TRUE) {
			XtVaGetValues(channel_viewport,
				  XtNwidth,	&width,
				  XtNheight, 	&height,
				  NULL);
			XtVaSetValues(channel_viewport,
				  XtNheight,	height-1,
				  XtNwidth,	width-1,
				  NULL);
		}
		reset_label(channel_label);
		return OK;
	} else {
		currentchan = NULL;
	}
	reset_label(channel_label);
	return NOTOK;
}

extern int compat;

/* ARGSUSED */
int channelread_result (ad, id, dummy, result, roi)
int							ad,
							id,
							dummy;
register struct type_Qmgr_ChannelReadResult		*result;
struct RoSAPindication					*roi;
{
	Dimension	width,
			height;
	extern time_t	currentTime;
	time(&currentTime);

	if (compat) {
		runnableChans = (double) result->load1;
		opsPerSec = (double) result->load2;
		currChans = (int) result->currchans;
		maxChans = (int) result->maxchans;
	}
	channel_list(result->channels);

	if (globallist != NULL) {
		display_channels();
		if (firstChanRead == TRUE) {
			XtVaGetValues(channel_viewport,
				  XtNwidth,	&width,
				  XtNheight, 	&height,
				  NULL);
			XtVaSetValues(channel_viewport,
				  XtNheight,	height-1,
				  XtNwidth,	width-1,
				  NULL);
		}
		reset_label(channel_label);
		return OK;
	} else {
		currentchan = NULL;
	}
	reset_label(channel_label);
	return NOTOK;
}

/* ARGSUSED */
int op_channelread (ad, ryo, rox, in, roi)
int			ad;
struct RyOperation	*ryo;
struct RoSAPinvoke	*rox;
caddr_t			in;
struct RoSAPindication	*roi;
{
	register struct type_Qmgr_ChannelReadResult	*arg;

	arg = (struct type_Qmgr_ChannelReadResult *) in;

	channel_list(arg->channels);

	if (compat) {
		runnableChans = (double) arg->load1;
		opsPerSec = (double) arg -> load2;
		currChans = arg->currchans;
		maxChans = arg->maxchans;
	}
	if (globallist != NULL) {
		display_channels();
		return OK;
	} else {
		currentchan = NULL;
	}
	reset_label(channel_label);
	return NOTOK;
}

/*  */
/* mtas */
extern struct mta_struct	*find_mta(),
				*currentmta;
extern struct chan_struct	*mta_list();
extern Mode			mode;

#define 	MTA_READ_INTERVAL	60	/* in secs */

CMD_TABLE	mtatbl_commands [] = {
	"start",	(int) mtastart,
	"stop",		(int) mtastop,
	"clear",	(int) mtaclear,
	"info",		(int) mtainfo,
	"cache add",	(int) mtacacheadd,
	0,		0
	};

/* ARGSUSED */
int do_mtaread (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args; /* args[0] is channel name */
struct type_Qmgr_MtaRead	**arg;
{
	char	*str;
	UTC	utc;

	*arg = (struct type_Qmgr_MtaRead *) malloc(sizeof(**arg));
	utc = (UTC) malloc (sizeof(struct UTCtime));

	utc->ut_flags = UT_SEC;
	utc->ut_sec = MTA_READ_INTERVAL;
	str = utct2str(utc);
	(*arg)->time = str2qb(str, strlen(str), 1);

	(*arg)->channel = str2qb(args[0],
				 strlen(args[0]),
				 1);
	return OK;
}

/* ARGSUSED */
int do_mtacontrol (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_Qmgr_MtaControl	**arg;
/* args[0] = channel */
/* args[1] = mta */
/* args[2] = stop,start,clear,cacheadd */
/* args[3] = time */
{
	char	*timestr;
	*arg = (struct type_Qmgr_MtaControl *) malloc(sizeof(**arg));
	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	(*arg)->channel = str2qb(args[0],
				 strlen(args[0]),
				 1);
	(*arg)->mta = str2qb(args[1],
			     strlen(args[1]),
			     1);

	switch (cmd_srch(args[2], mtatbl_commands)) {
	    case mtastart:
		(*arg)->control->offset = type_Qmgr_Control_start;
		break;
	    case mtastop:
		(*arg)->control->offset = type_Qmgr_Control_stop;
		break;
	    case mtaclear:
		(*arg)->control->offset = type_Qmgr_Control_cacheClear;
		break;
	    case mtacacheadd:
		(*arg)->control->offset = type_Qmgr_Control_cacheAdd;
		timestr = mystrtotime(args[3]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
		break;
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console : '%s' unknown control param for mtas",
			args[2]));
		return NOTOK;
	}
	return OK;
}

/* ARGSUSED */
int mtacontrol_result (ad, id, dummy, result, roi)
int						ad,
						id,
						dummy;
register struct type_Qmgr_MtaInfo		*result;
struct RoSAPindication				*roi;
{
	char			*chan_name,
				*name;
	struct chan_struct	*chan;
	struct mta_struct	*mta;

	chan_name = qb2str(result->channel);
	chan = find_channel(chan_name);

	if (chan == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console: mtacontrol_result can't find channel %s",chan_name));
		return NOTOK;
	}

	name = qb2str(result->mta);
	mta = find_mta(chan, name);

	if (mta == NULL) {
		if (forceDown != TRUE) 
			PP_NOTICE(("can not find mta %s on channel %s",
				   name, chan_name));
		free(chan_name);
		free(name);
		return OK;
	} else {
		update_mta(mta,result);
	}
		       
	currentchan = chan;

	order_mtas(&(chan->mtalist),chan->num_mtas);
	display_mtas(chan);
	reset_label(mta_label);
	free(chan_name);
	free(name);
	return OK;
}

extern char	*pop_from_mta_refresh_list();

/* ARGSUSED */
int mtaread_result (ad, id, dummy, result, roi)
int						ad,
						id,
						dummy;
register struct type_Qmgr_PrioritisedMtaList	*result;
struct RoSAPindication				*roi;
{
	struct chan_struct	*chan;
	char	*mtaname = NULL;
	char	*channame;

	channame  = pop_from_mta_refresh_list();

	if (currentmta != NULL)
		mtaname = strdup(currentmta->mta);
	chan = mta_list(result, channame);
	free(channame);

	if (forceDown == TRUE
	    && chan != currentchan) {
		if (chan != NULL) {
			free_mta_list(&(chan->mtalist), &(chan->num_mtas));
			chan->mtalist = NULL;
			chan->num_mtas = 0;
		}
		return OK;
	}
	if ((mode == monitor && chan != NULL)
	    || (mode == control && chan != NULL && chan == currentchan)) {
		order_mtas(&(chan->mtalist),chan->num_mtas);
		if (mtaname != NULL) {
			/* reset currentmta */
			currentmta = find_mta(chan, mtaname);
			free(mtaname);
		}
		reset_label(mta_label);
		display_mtas(chan);
		return OK;
	} else if (mode == control) {
		display_empty_mta_list(currentchan);
		currentmta = NULL;
		display_empty_msg_list();
		reset_label(mta_label);
		return OK;
	} else {
		/* fill in here */
		PP_LOG(LLOG_EXCEPTIONS,
		       ("MTAconsole in confused state"));
	}
	return NOTOK;
}

/* ARGSUSED */
int op_mtaread (ad, ryo, rox, in, roi)
int			ad;
struct RyOperation	*ryo;
struct RoSAPinvoke	*rox;
caddr_t			in;
struct RoSAPindication	*roi;
{
	register struct type_Qmgr_PrioritisedMtaList	*arg;
	struct chan_struct				*chan;
	char						*mtaname=NULL, 
							*channame;
	arg = (struct type_Qmgr_PrioritisedMtaList *) in;
	channame = pop_from_mta_refresh_list();
	if (currentmta != NULL)
		mtaname = strdup(currentmta->mta);
	currentmta = NULL;
	chan = mta_list(arg, channame);
	free(channame);
	if (chan != NULL) {
		order_mtas(&(chan->mtalist),chan->num_mtas);
		if (mtaname != NULL) {
			/* reset currentmta */
			currentmta = find_mta(chan, mtaname);
			free(mtaname);
		}
		reset_label(mta_label);
		display_mtas(chan);
		return OK;
	}
	reset_label(mta_label);
	return NOTOK;
}

/*  */
/* msgs */
struct msg_struct		*currentmsg,
				**global_msg_list,
				*find_msg();
struct type_Qmgr_UserList	*create_userlist();
struct type_Qmgr_FilterList	*fillinfilters();
#define 	MSG_READ_INTERVAL	60	/* in secs */

CMD_TABLE	msgtbl_commands [] = {
	"start",	(int) msgstart,
	"stop",		(int) msgstop,
	"clear",	(int) msgclear,
	"info",		(int) msginfo,
	"cache add",	(int) msgcacheadd,
	0,		0
	};

/* ARGSUSED */
int do_msgcontrol (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_Qmgr_MsgControl	**arg;
/* args[0] = qid */
/* args[1] = time */
/* args[2] = stop, start, clear cacheadd*/
/* args[3..] = NULL terminated list of users */
{
	char	*timestr;
	*arg = (struct type_Qmgr_MsgControl *) malloc(sizeof(**arg));
	
	(*arg)->qid = str2qb(args[0],
			     strlen(args[0]),
			     1);

	(*arg)->users = create_userlist(args[0], &(args[3]));

	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	switch (cmd_srch(args[2], msgtbl_commands)) {
	    case msgstart:
		(*arg)->control->offset = type_Qmgr_Control_start;
		break;
	    case msgstop:
		(*arg)->control->offset = type_Qmgr_Control_stop;
		break;
	    case msgclear:
		(*arg)->control->offset = type_Qmgr_Control_cacheClear;
		break;
	    case msgcacheadd:
		(*arg)->control->offset = type_Qmgr_Control_cacheAdd;
		timestr = mystrtotime(args[1]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
		break;
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console : '%s' unknown control param for msgs",
			args[1]));
		return NOTOK;
	}
	return OK;
}

extern char	*msginfo_args[3];

/* ARGSUSED */
msgcontrol_result (sd, id, dummy, result, roi)
int					sd,
	                                id,
	                                dummy;
struct type_Qmgr_Pseudo__newmessage 	*result;
struct RoSAPindication			*roi;
{
	if (currentchan != NULL
	    && currentmta != NULL) {
		msginfo_args[0] = currentchan->channelname;
		msginfo_args[1] = currentmta->mta;
		if (is_loc_chan(currentchan) == TRUE)
			/* local */
			msginfo_args[2] = (char *) 1;
		else
			msginfo_args[2] = (char *) 0;
		construct_event(readchannelmtamessage);
	}
	return OK;
}

/* ARGSUSED */
int do_readchannelmtamessage (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args; 
/* args[0] = chan args[1] = mta or name in case of local chan */
struct type_Qmgr_MsgRead	**arg;
{
	char	*str;
	UTC	utc;

	*arg = (struct type_Qmgr_MsgRead *) malloc(sizeof(**arg));
	bzero ((char *) *arg, sizeof(**arg));
	/* fillin time */
	utc = (UTC) malloc (sizeof(struct UTCtime));

	utc->ut_flags = UT_SEC;
	utc->ut_sec = MSG_READ_INTERVAL;
	str = utct2str(utc);
	(*arg)->time = str2qb(str, strlen(str), 1);
	if (args[0] != NULLCP)
		(*arg)->channel = str2qb(args[0], strlen(args[0]), 1);
	if (args[1] != NULLCP)
		(*arg)->mta = str2qb(args[1], strlen(args[1]), 1);
	return OK;
}

/* ARGSUSED */
int readchannelmtamessage_result (ad, id, dummy, result, roi)
int						ad,
						id,
						dummy;
register struct type_Qmgr_MsgList		*result;
struct RoSAPindication				*roi;
{
	char	*msgname = NULL;
	if (currentmsg != NULL)
		msgname = strdup(currentmsg->msginfo->queueid);
	msg_list(result);

	if (forceDown == TRUE) {
		free_msg_list();
		global_msg_list = NULL;
	}
		
	if (global_msg_list != NULL){
		order_msgs();
		if (msgname != NULL) {
			/* reset currentmsg */
			currentmsg = find_msg(msgname);
			free(msgname);
		}
		reset_label(msg_label);
		display_msgs();
		return OK;
	} else {
		display_empty_msg_list();
	}
	reset_label(msg_label);
	return NOTOK;
}

/* ARGSUSED */
int op_readchannelmtamessage (ad, ryo, rox, in, roi)
int			ad;
struct RyOperation	*ryo;
struct RoSAPinvoke	*rox;
caddr_t			in;
struct RoSAPindication	*roi;
{
	register struct type_Qmgr_MsgList	*arg;
	char	*msgname = NULL;
	arg = (struct type_Qmgr_MsgList *) in;
	if (currentmsg != NULL)
		msgname = strdup(currentmsg->msginfo->queueid);
	currentmsg = NULL;
	msg_list(arg);

	if (global_msg_list != NULL){
		order_msgs();
		if (msgname != NULL) {
			/* reset currentmsg */
			currentmsg = find_msg(msgname);
			free(msgname);
		}
		reset_label(msg_label);
		display_msgs();
		return OK;
	}
	reset_label(msg_label);
	return NOTOK;
	
}


struct type_Qmgr_FilterList	*fillinfilters(args)
char	**args;
/* args[0] = chan args[1] = mta or name in case of local chan */
{
	struct type_Qmgr_FilterList	*retval = 
		(struct type_Qmgr_FilterList *) calloc(1, sizeof(*retval));
	
	retval->Filter = 
		(struct type_Qmgr_Filter *) calloc(1, sizeof(*retval->Filter));
	
	retval->Filter->channel = str2qb(args[0], strlen(args[0]), 1);
	if (args[1] != NULLCP) {
		if (args[2] == (char *) 1)
			retval->Filter->recipient = str2qb(args[1], strlen(args[1]), 1);
		else
			retval->Filter->mta = str2qb(args[1], strlen(args[1]), 1);
	}
	return retval;
}


struct type_Qmgr_UserList	*create_userlist(qid, argv)
char	*qid,
	**argv;
{
	struct type_Qmgr_UserList	*temp,
					*head = NULL,
					*tail = NULL;
	int				i = 0;
	struct msg_struct		*msg;
	struct recip			*ix;

	if (strcmp(argv[i], "*") == 0) {
		if ((msg = find_msg(qid)) == NULL)
			return NULL;
		ix = msg -> reciplist;
		while (ix != NULL) {
			temp = (struct type_Qmgr_UserList *) 
				calloc (1, sizeof(*temp));
			temp->RecipientId = (struct type_Qmgr_RecipientId *) 
				malloc(sizeof(struct type_Qmgr_RecipientId));
			temp->RecipientId->parm = ix -> id;
			ix = ix -> next;
			if (head == NULL)
				head = tail = temp;
			else {
				tail->next = temp;
				tail = tail->next;
			}
		}
	} else {
		while (argv[i] != NULL) {
			temp = (struct type_Qmgr_UserList *) 
				calloc (1, sizeof(*temp));
			temp->RecipientId = (struct type_Qmgr_RecipientId *) 
				malloc(sizeof(struct type_Qmgr_RecipientId));
			temp->RecipientId->parm = atoi(argv[i++]);
		
			if (head == NULL)
				head = tail = temp;
			else {
				tail->next = temp;
				tail = tail->next;
			}
		}
	}
	return head;
}

/* ARGSUSED */
int do_quecontrol (ad, ds, args, arg)
int			ad;
struct client_dispatch	*ds;
int			args;
struct type_Qmgr_QMGROp **arg;
{
	*arg = (struct type_Qmgr_QMGROp *) malloc(sizeof(**arg));
	(*arg)->parm = args;
	return OK;
}

/* ARGSUSED */
int quecontrol_result (ad, id, dummy, result, roi)
int	ad,
	id,
	dummy;
struct type_Qmgr_Pseudo__qmgrControl	*result;
struct RoSAPindication			*roi;
{
	return OK;
}

extern int total_volume, total_number_messages, total_number_reports;
extern int delta_volume, delta_messages, delta_reports;

/* ARGSUSED */
int qmgrStatus_result (ad, id, dummy, result, roi)
int	ad,
	id,
	dummy;
struct type_Qmgr_QmgrStatus	*result;
struct RoSAPindication	*roi;
{
	if (result) {
		if (result->boottime) 
			boottime = convert_time(result->boottime);
		messagesIn = result->messagesIn;
		messagesOut = result->messagesOut;
		addrIn = result->addrIn;
		addrOut = result->addrOut;
		opsPerSec = (double) result->opsPerSec;
		runnableChans = (double) result->runnableChans;
		msgsInPerSec = (double) result->msgsInPerSec;
		msgsOutPerSec = (double) result->msgsOutPerSec;
		maxChans = result->maxChans;
		currChans = result->currChans;
		delta_volume = result->totalVolume - total_volume;
		total_volume = result->totalVolume;
		delta_messages = result->totalMsgs - total_number_messages;
		total_number_messages = result->totalMsgs;
		delta_reports = result->totalDrs - total_number_reports;
		total_number_reports = result->totalDrs;
		/* refresh display */
		update_time_label();
		display_totals();
	}
	return OK;
}
	
