/* data.c: routines to deal with data structures */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/data.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/data.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: data.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"
#include	"Qmgr-types.h"

extern time_t		convert_time();
struct chan_struct	*find_channel();
struct mta_struct	*find_mta();
struct procStatus	*create_status();
void			update_status();
/*  */
/* channel */
static void create_channel_list();
static void update_channel_list();
static struct chan_struct *create_channel();
static void update_channel();

struct monitor_item	**display_list = NULL;
struct chan_struct	**globallist = NULL,
			**ordered_list = NULL,
			*currentchan = NULL;
int			num_channels,
			read_currentchan = 0;
int			firstChanRead;
int			total_number_messages,
			total_number_reports,
			total_volume,
			delta_volume,
			delta_messages,
			delta_reports;
extern int		forceDown;
extern int		uk_order;
extern int		compat;

channel_list(new)
struct type_Qmgr_PrioritisedChannelList *new;
{
	if (compat) {
		total_number_messages = 0;
		total_number_reports = 0;
		total_volume = 0;
	}
	if (globallist == NULL) {
		create_channel_list(new);
		firstChanRead = TRUE;
	} else {
		update_channel_list(new);
		firstChanRead = FALSE;
	}
}

static int chantype_compare(one, two)
struct chan_struct	**one,
			**two;
{
	register int	onetype, twotype;
	
	onetype = (*one)->chantype;
	twotype = (*two)->chantype;

	if (onetype != twotype) {
		if (onetype == int_Qmgr_chantype_mta)
			return -1;
		if (twotype == int_Qmgr_chantype_mta)
			return 1;
		if (onetype == int_Qmgr_chantype_mts)
			return -1;
		if (twotype == int_Qmgr_chantype_mts)
			return 1;
		if (onetype == int_Qmgr_chantype_internal)
			return -1;
		if (twotype == int_Qmgr_chantype_internal)
			return 1;
	} 

	/* order on inbound and outbound */
	if ((*one)->inbound == (*two)->inbound
	    && (*one)->outbound == (*two) -> outbound)
		return 0;
	if ((*one)->outbound > (*two) -> outbound)
		return -1;
	if ((*one)->outbound < (*two)->outbound)
		return 1;
	if ((*one)->inbound > (*two) -> inbound)
		return -1;
	if ((*one)->inbound < (*two)->inbound)
		return 1;
	return 0;
}

static void	create_channel_list(list)
struct type_Qmgr_PrioritisedChannelList	*list;
/* calloc and fillin in globallist */
{
	struct type_Qmgr_PrioritisedChannelList	*ix = list;
	int	i,
		num = 0;

	while (ix != NULL) {
		num++;

		ix = ix->next;
	}

	num_channels = num;
	globallist = (struct chan_struct **) calloc((unsigned) num, 
						     sizeof(struct chan_struct *));
	ordered_list = (struct chan_struct **) calloc((unsigned) num, 
						      sizeof(struct chan_struct *));
	display_list = (struct monitor_item **) calloc((unsigned) num, 
						       sizeof(struct monitor_item *));
	resize_chan_array(num);
	
	ix = list;
	i = 0;
	while ((ix != NULL) 
	       && (i < num)) {
		globallist[i] = create_channel(ix);
		i++;
		ix = ix->next;
	}
	
	qsort((char *) &(globallist[0]), num,
	      sizeof(globallist[0]), (IFP)chantype_compare);

	i = 0;
	while (i < num) {
		ordered_list[i] = globallist[i];
		display_list[i] = (struct monitor_item *) calloc(1, sizeof(struct monitor_item));
		display_list[i]->channel = &(ordered_list[i]);
		i++;
	}
}

extern time_t	boottime;

static struct chan_struct	*create_channel(chan)
struct type_Qmgr_PrioritisedChannelList *chan;
{
	struct chan_struct		*temp;
	struct type_Qmgr_ChannelInfo	*info;

	temp = (struct chan_struct *) calloc(1, sizeof(*temp));
	info = chan->PrioritisedChannel->channel;
	
	temp->channelname = qb2str(info->channel);
	temp->channeldescrip = qb2str(info->channelDescription);

	temp->oldestMessage = convert_time(info->oldestMessage); 
	temp->numberMessages = info->numberMessages;
	temp->numberReports = info->numberReports;
	temp->volumeMessages = info->volumeMessages;
	temp->given_num_mtas = info->numberMtas;
	if (compat) {
		total_volume += temp->volumeMessages;
		total_number_reports += temp->numberReports;
		total_number_messages += temp->numberMessages;
	}
	temp->numberActiveProcesses = info->numberActiveProcesses;
	temp->status = create_status(info->status);
	if (temp->status->lastSuccess == 0)
		temp->status->lastSuccess = boottime;
	temp->priority = chan->PrioritisedChannel->priority->parm;
	temp->inbound = bit_test(info->direction, bit_Qmgr_direction_inbound);
	temp->outbound = bit_test(info->direction, bit_Qmgr_direction_outbound);
	temp->chantype = info->chantype;
	temp->maxprocs = info->maxprocs;

	add_tailor_to_chan(temp);
	return temp;
}

static void	update_channel_list(new)
struct type_Qmgr_PrioritisedChannelList	*new;
{
	struct type_Qmgr_PrioritisedChannelList *ix = new;
	char					*name = NULL;
	struct chan_struct			*chan;

	while (ix != NULL) {
		if (name != NULL) free(name);
		name = qb2str(ix->PrioritisedChannel->channel->channel);

		chan = find_channel(name);
		if (chan == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("console: can not find channel %s",name));
			abort();
		}
		update_channel(chan,ix);
		ix = ix->next;
	}
}

static void	update_channel(old, new)
struct chan_struct			*old;
struct type_Qmgr_PrioritisedChannelList	*new;
{
	struct type_Qmgr_ChannelInfo	*info;

	info = new->PrioritisedChannel->channel;
	/* name and description don't change */
	old->oldestMessage = convert_time(info->oldestMessage);
	old->deltaMessages = info->numberMessages - old->numberMessages;
	old->numberMessages = info->numberMessages;
	old->deltaReports = info->numberReports - old->numberReports;
	old->numberReports = info->numberReports;
	old->deltaVolume = info->volumeMessages-old->volumeMessages;
	old->volumeMessages = info->volumeMessages;
	old->deltaMtas = info->numberMtas - old->given_num_mtas;
	old->given_num_mtas = info->numberMtas;
	if (compat) {
		total_volume += old->volumeMessages;
		total_number_reports += old->numberReports;
		total_number_messages += old->numberMessages;
	}
	old->numberActiveProcesses = info->numberActiveProcesses;
	update_status(old->status,info->status);
	old->priority = new->PrioritisedChannel->priority->parm;
	old->maxprocs = info->maxprocs;
}

free_channel_list()
{
	int	i =0;
	if (globallist == NULL)
		return;
	while (i < num_channels) {
		free(globallist[i]->channelname);
		free(globallist[i]->channeldescrip);
		free ((char *)globallist[i]->status);
		if (globallist[i]->mtalist != NULL)
			free_mta_list(&(globallist[i]->mtalist),
				      &(globallist[i]->num_mtas));
		i++;
	}
	free((char *) globallist);
	free((char *) ordered_list);
	ordered_list = NULL;
	globallist = NULL;
}

struct chan_struct *find_channel(name)
char			*name;
{
	int	i = 0;
	while ((i < num_channels) && 
	       (lexequ(globallist[i]->channelname, name) != 0)) 
		i++;

	if (i >= num_channels) 
		return NULL;
	else
		return globallist[i];
}

/*  */
/* mtas */
static void update_mta_list();
static struct mta_struct **create_mta_list();
static struct mta_struct *create_mta();

struct mta_struct	*currentmta = NULL;
int			read_currentmta = 0;

struct chan_struct *mta_list(new, channame)
struct type_Qmgr_PrioritisedMtaList	*new;
char					*channame;
{
	struct chan_struct			*chan;

	if (channame == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console: mta_list no channel name given"));
		abort();
	}

	chan = find_channel(channame);
	if (chan == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console: can not find channel %s", channame));
		abort();
	}

	free_mta_list(&(chan->mtalist), &(chan->num_mtas));
	chan->mtalist = NULL;
	chan->num_mtas = 0;
	if (new != NULL)
		update_mta_list(new, chan);
	else
		update_channel_from_mtas(chan, 0, 0, 0, 0);
	return chan;
}

static void update_mta_list(new, chan)
struct type_Qmgr_PrioritisedMtaList	*new;
struct chan_struct			*chan;
{
	int					numberMsgs = 0,
						numberDrs = 0,
						numberMtas = 0,
						volume = 0;
	chan->mtalist = create_mta_list(new,&numberMtas,
					&numberDrs,&numberMsgs,&volume,
					chan);
	update_channel_from_mtas(chan, numberMsgs, numberDrs, numberMtas, volume);
}

static struct mta_struct	**create_mta_list(list, pnum, pdr, pmsgnum, pvolume, chan)
struct type_Qmgr_PrioritisedMtaList	*list;
int					*pnum,
  					*pdr,
					*pmsgnum,
					*pvolume;
struct chan_struct			*chan;
{
	struct type_Qmgr_PrioritisedMtaList 	*ix = list;
	struct mta_struct			**mtalist;
	int					i;
	while (ix != NULL) {
		(*pnum)++;

		ix = ix->next;
	}
	
	mtalist = (struct mta_struct	**) calloc( (unsigned)(*pnum), 
						   sizeof(struct mta_struct *));

	ix = list;
	i = 0;
	*pmsgnum = 0;
	*pdr = 0;

	while ((ix != NULL)
	       && ( i < (*pnum))) {
		mtalist[i] = create_mta(ix->PrioritisedMta, chan);
		*pmsgnum += mtalist[i]->numberMessages;
		*pdr += mtalist[i]->numberReports;
		*pvolume += mtalist[i]->volumeMessages;
		i++;
		ix = ix->next;
	}
	return mtalist;
}

static struct mta_struct	*create_mta(mta, chan)
struct type_Qmgr_PrioritisedMta	*mta;
struct chan_struct		*chan;
{
	struct mta_struct		*temp;
	struct type_Qmgr_MtaInfo	*info = mta->mta;

	temp = (struct mta_struct *) malloc(sizeof(*temp));

	temp->mta = qb2str(info->mta);
	temp->oldestMessage = convert_time(info->oldestMessage);
	temp->numberMessages = info->numberMessage;
	temp->numberReports = info->numberDRs;
	temp->volumeMessages = info->volumeMessages;
	temp->status = create_status(info->status);
	temp->priority = mta->priority->parm;
	temp->active = info->active;
	if (info -> info != NULL)
		temp -> info = qb2str(info -> info);
	else
		temp -> info = NULLCP;
	add_tailor_to_mta(chan, temp);
	if (forceDown == TRUE)
		MtaDownForce(chan, temp);
	return temp;
}

update_mta(old, info)
struct mta_struct			*old;
struct type_Qmgr_MtaInfo		*info;
{
	/* name doesn't change */
	old->oldestMessage = convert_time(info->oldestMessage);
	old->numberMessages = info->numberMessage;
	old->numberReports = info->numberDRs;
	old->volumeMessages = info->volumeMessages;
	old->active = info->active;
	if (old -> info) 
		free (old -> info);
	if (info -> info != NULL)
		old -> info = qb2str(info -> info);
	else
		old -> info = NULLCP;
	update_status(old->status,info->status);
}

/* ordering routines */	
struct mta_struct	*find_mta(chan, name)
struct chan_struct	*chan;
char			*name;
{
	int	i = 0;
	while ((i < chan->num_mtas) &&
	       (strcmp(chan->mtalist[i]->mta, name) != 0))
		i++;
	if (i >= chan->num_mtas)
		return NULL;
	else
		return chan->mtalist[i];
}	

/* garbage collection routine */

free_mta_list(plist, pnum)
struct mta_struct	***plist;
int			*pnum;
{
	int	i = 0;

	while(i < (*pnum)){
		if ((*plist) [i] != NULL) {
			free((*plist)[i]->mta);
			if ((*plist)[i]->info != NULLCP)
				free((*plist)[i] -> info);
			free((char *) (*plist)[i]->status);
		}
		i++;
	}
	if (*plist != NULL)
		free((char *) (*plist));
	*plist = NULL;
	*pnum = 0;
}

/*  */
/* msgs */
struct msg_struct	**global_msg_list = NULL,
			*currentmsg = NULL;
int			number_msgs = 0;
static struct msg_struct	**create_msg_list();

msg_list(new)
struct type_Qmgr_MsgList	*new;
{
	free_msg_list();
	global_msg_list = create_msg_list(new->msgs);
}


static char	*create_eits(eit)
struct type_Qmgr_EncodedInformationTypes	*eit;
{
	struct type_Qmgr_EncodedInformationTypes *ix;
	char	*str,
		buf[BUFSIZ];
	int	first;
	if (eit == NULL)
		return NULL;
	ix =eit;
	first = True;

	while (ix != NULL) {
		str = qb2str(ix->PrintableString);
		if (first == True) {
			sprintf(buf,"%s",str);
			first = False;
		} else 
			sprintf(buf,"%s, %s",buf, str);
		free(str);
		ix = ix->next;
	}
	return strdup(buf);
}

static struct permsginfo	*create_msginfo(info)
struct type_Qmgr_PerMessageInfo	*info;
{
	struct permsginfo	*temp;

	temp = (struct permsginfo *) calloc(1,sizeof(*temp));

	temp->queueid = qb2str(info->queueid);
	temp -> originator = qb2str(info->originator);
	if (info->contenttype != NULL)
		temp->contenttype = qb2str(info->contenttype);
	else
		temp->contenttype = NULL;
	temp->eit = create_eits(info->eit);
	temp->age = convert_time(info->age);
	temp->size = info->size;
	temp->priority = info->priority->parm;
	if (info->expiryTime == NULL)
		temp->expiryTime = 0;
	else
		temp->expiryTime = convert_time(info->expiryTime);
	if (info->deferredTime == NULL)
		temp->deferredTime = 0;
	else 
		temp->deferredTime = convert_time(info->deferredTime);
	if (info->optionals & opt_Qmgr_PerMessageInfo_errorCount)
		temp->errorCount = info->errorCount;
	if (info->inChannel != NULL)
		temp->inChannel = qb2str(info->inChannel);
	else
		temp->inChannel = NULLCP;
	if (info->uaContentId != NULL)
		temp -> uaContentId = qb2str(info->uaContentId);
	else
		temp -> uaContentId = NULLCP;
	return temp;
}

static char			*create_chans(list, done, pfirst)
struct type_Qmgr_ChannelList	*list;
int				done;
char				**pfirst;
{
	int	count;
	char	buf[BUFSIZ],
		*str;

	struct type_Qmgr_ChannelList	*ix;
	count = 0;
	ix = list;
	while (count < done && ix != NULL) {
		ix = ix->next;
		count++;
	}
	if (ix == NULL)
		/* all done waiting for delivery notifcation */
		return NULL;

	str = qb2str(ix->Channel);
	sprintf(buf,"%s",str);
	free(str);
	*pfirst = qb2str(ix->Channel);
	ix = ix->next;

	while (ix != NULL) {
		str = qb2str(ix->Channel);
		sprintf(buf, "%s, %s",buf,str);
		free(str);
		ix = ix->next;
	}
	return strdup(buf);
}
	
static struct recip		*create_recip(recip)
struct type_Qmgr_RecipientInfo	*recip;
{
	struct recip	*temp;

	temp = (struct recip *) calloc(1,sizeof(*temp));

	temp->id = recip->id->parm;

	temp -> recipient = qb2str(recip->user);
	temp->mta = qb2str(recip->mta);
	temp->chansOutstanding = create_chans(recip->channelList,
					      recip->channelsDone,
					      &temp->actChan);
	temp->status = create_status(recip->procStatus);
	if (recip -> info != NULL)
		temp -> info = qb2str (recip -> info);
	return temp;
}
	
	
static struct recip 		*create_reciplist(list)
struct type_Qmgr_RecipientList	*list;
{
	struct recip	*head = NULL,
			*tail = NULL,
			*temp;

	while (list != NULL) {
		if (list->RecipientInfo->id != 0) {
			temp = create_recip(list->RecipientInfo);
			if (head == NULL)
				tail = head = temp;
			else {
				tail->next = temp;
				tail = temp;
			}
		}
		list = list->next;
	}
	return head;
}

static struct msg_struct	*create_msg(msg)
struct type_Qmgr_MsgStruct	*msg;
{
	struct msg_struct	*temp;
	
	temp = (struct msg_struct *) calloc(1, sizeof(*temp));
	temp->msginfo = create_msginfo(msg->messageinfo);
	temp->reciplist = create_reciplist(msg->recipientlist);
	add_tailor_to_msg(currentchan,temp);
	if (forceDown == TRUE) 
		MsgDownForce(temp);
	return temp;
}

static struct msg_struct	**create_msg_list(list)
struct type_Qmgr_MsgStructList	*list;
{
	struct type_Qmgr_MsgStructList	*ix = list;
	struct msg_struct		**msglist;
	int				i;
	number_msgs = 0;
	while (ix != NULL) {
		number_msgs++;
		ix = ix->next;
	}

	msglist = (struct msg_struct **) calloc((unsigned) number_msgs, 
						sizeof(struct msg_struct *));
	
	ix = list;
	i = 0;
	while ((ix != NULL)
	       & (i < number_msgs)) {
		msglist[i] = create_msg(ix->MsgStruct);
		i++;
		ix = ix->next;
	}
	return msglist;
}

free_msg_list()
{
	int i = 0;
	if (global_msg_list == NULL || number_msgs == 0)
		return;

	while (i < number_msgs) {
		free_permsginfo(global_msg_list[i]->msginfo);
		free_reciplist(global_msg_list[i]->reciplist);
		i++;
	}
	free((char *) global_msg_list);
	global_msg_list = NULL;
	number_msgs = 0;
}

free_permsginfo(info)
struct permsginfo	*info;
{
	free(info->queueid);
	free(info->originator);
	if (info->eit != NULL) free(info->eit);
	if (info->inChannel != NULL) free(info->inChannel);
	if (info->uaContentId != NULLCP) free(info->uaContentId);
	free((char *) info);
}

free_reciplist(list)
struct recip	*list;
{
	struct recip	*ix = list,
			*temp;

	while (ix != NULL) {
		if (ix->recipient) free(ix->recipient);
		if (ix->mta != NULL) free(ix->mta);
		if (ix->actChan != NULL) free(ix->actChan);
		if (ix->chansOutstanding != NULL) free(ix->chansOutstanding);
		if (ix -> info != NULL) free(ix->info);
		free((char *) ix->status);
		temp = ix;
		ix = ix->next;
		free((char *) temp);
	}
}


struct recip	*find_recip(msg, id)
struct msg_struct	*msg;
int			id;
{
	struct recip	*ix;
	if (msg == NULL)
		return NULL;
	ix = msg->reciplist;
	
	while (ix != NULL
	       && ix -> id != id)
		ix = ix->next;
	return ix;
}

struct msg_struct *find_msg(qid)
char	*qid;
{
	int	i = 0;

	while ((i < number_msgs) &&
	       (strcmp(global_msg_list[i]->msginfo->queueid,qid) != 0))
		i++;
	if (i >= number_msgs)
		return NULL;
	else
		return global_msg_list[i];
}

/*  */
/* misc */
struct procStatus	*create_status(status)
struct type_Qmgr_ProcStatus	*status;
{
	struct procStatus	*temp;

	temp = (struct procStatus *) malloc(sizeof(*temp));

	temp->enabled = status->enabled;
	temp->lastAttempt = convert_time(status->lastAttempt);
	temp->cachedUntil = convert_time(status->cachedUntil);
	temp->lastSuccess = convert_time(status->lastSuccess);
	return temp;
}

void	update_status(old, new)
struct procStatus		*old;
struct type_Qmgr_ProcStatus	*new;
{
	time_t	tmp;
	old->enabled = new->enabled;

	old->lastAttempt = convert_time(new->lastAttempt);
	
	old->cachedUntil = convert_time(new->cachedUntil);

	if ((tmp = convert_time(new->lastSuccess)) != 0)
		old->lastSuccess = tmp;
}
