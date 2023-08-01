/* op_msg.c msg ROS operation routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_msg.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_msg.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: op_msg.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "qmgr-int.h"
#include "consblk.h"
#include "Qmgr-types.h"
#include <isode/cmd_srch.h>
#include <isode/rosap.h>

#define 	MSG_READ_INTERVAL	60	/* in secs */

/* ARGSUSED */
int	arg_readchannelmtamessage (ad, args, arg)
int	ad;
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

static MsgList *convert_MsgList();

/* ARGSUSED */
int res_readchannelmtamessage (ad, id, dummy, result, roi)
int						ad,
						id,
						dummy;
register struct type_Qmgr_MsgList		*result;
struct RoSAPindication				*roi;
{
	struct cons_opblk *op;

	if ((op = find_copblk(ad, id)) == NULL)
		return NOTOK;
	
	if (op -> fnx)
		(*op -> fnx)(convert_MsgList(result), id);
	free_copblk (op);
	return OK;
}

/*  */

static struct type_Qmgr_UserList *create_userlist();

extern char *mystrtotime();

static CMD_TABLE control_tbl[] = {
	CTRL_ENABLE,	type_Qmgr_Control_start,
	CTRL_DISABLE,	type_Qmgr_Control_stop,
	CTRL_CACHECLEAR,type_Qmgr_Control_cacheClear,
	CTRL_CACHEADD,	type_Qmgr_Control_cacheAdd,
	NULLCP,		NOTOK
	};

/* ARGSUSED */
int arg_msgcontrol (ad, args, arg)
int	ad;
char				**args;
struct type_Qmgr_MsgControl	**arg;
/* args[0] = qid */
/* args[1] = time */
/* args[2] = stop, start, clear cacheadd*/
/* args[3..] = NULL terminated list of users */
{
	int n;

	*arg = (struct type_Qmgr_MsgControl *) malloc(sizeof(**arg));
	
	(*arg)->qid = str2qb(args[0], strlen(args[0]), 1);

	(*arg)->users = create_userlist(&(args[3]));

	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	if ((n = cmd_srch (args[2], control_tbl)) == NOTOK)
		return NOTOK;

	if (((*arg)->control->offset = n) == type_Qmgr_Control_cacheAdd) {
		char	*timestr;
		timestr = mystrtotime(args[1]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
	}
	return OK;
}

/* ARGSUSED */
int res_msgcontrol (ad, id, dummy, result, roi)
int	ad, id, dummy;
struct type_Qmgr_Pseudo__newmessage	*result;
struct RoSAPindication	*roi;
{
	struct cons_opblk *op;

	if ((op = find_copblk(ad, id)) == NULL)
		return NOTOK;
	
	if (op -> fnx)
		(*op -> fnx) (id);
	free_copblk (op);
	return OK;
}

/*  */

static Strlist	*convert_EITs (pepsy)
register struct type_Qmgr_EncodedInformationTypes	*pepsy;
{
	Strlist	*head, *tail, *temp;

	head = tail = (Strlist *) NULL;
	
	while (pepsy != (struct type_Qmgr_EncodedInformationTypes *) NULL) {
		if (pepsy->PrintableString) {
			temp = (Strlist *) calloc (1, sizeof(*temp));
			temp -> str = qb2str (pepsy->PrintableString);
			if (head == (Strlist *) NULL)
				head = tail = temp;
			else {
				tail -> next = temp;
				tail = temp;
			}
		}
		pepsy = pepsy -> next;
	}
	return head;
}

static PerMessageInfo	*convert_PerMessageInfo(pepsy)
register struct type_Qmgr_PerMessageInfo	*pepsy;
{
	PerMessageInfo	*ret;

	if (pepsy == (struct type_Qmgr_PerMessageInfo *) NULL)
		return (PerMessageInfo *) NULL;

	ret = (PerMessageInfo *) calloc(1, sizeof(*ret));

	ret -> qid = qb2str(pepsy->queueid);
	
	if (pepsy->mpduiden) {
		if (pepsy->mpduiden->global) {
			if (pepsy->mpduiden->global->country)
				ret->country = 
					qb2str(pepsy->mpduiden->
					       global->country -> un.printable);
			if (pepsy->mpduiden->global->admd)
				ret->admd = 
					qb2str(pepsy->mpduiden->global->
					       admd -> un.printable);
			if (pepsy->mpduiden->global->prmd)
				ret->prmd = 
					qb2str(pepsy->mpduiden->global->
					       prmd -> un.printable);
		}
		if (pepsy->mpduiden->local)
			ret->local =
				qb2str(pepsy->mpduiden->local);
	}

	ret->originator = qb2str(pepsy->originator);
	if (pepsy->contenttype)
		ret->contenttype = qb2str(pepsy->contenttype);
	if (pepsy->eit)
		ret -> eits = convert_EITs(pepsy->eit);
	ret -> priority = pepsy->priority->parm;
	ret -> size = pepsy->size;
	ret->age = convert_time(pepsy->age);
	ret -> warnInterval = pepsy -> warnInterval;
	ret -> numberWarningsSent = pepsy ->numberWarningsSent;
	if (pepsy -> expiryTime)
		ret -> expiryTime = convert_time(pepsy -> expiryTime);
	if (pepsy -> deferredTime)
		ret -> deferredTime = convert_time(pepsy -> deferredTime);
	if (pepsy -> uaContentId)
		ret -> uaContentId = qb2str (pepsy -> uaContentId);
	if (pepsy -> optionals & opt_Qmgr_PerMessageInfo_errorCount)
		ret->errorCount = pepsy->errorCount;
	if (pepsy -> inChannel)
		ret -> inChannel = qb2str(pepsy -> inChannel);
	return ret;
}

static Strlist	*convert_ChannelList (pepsy)
register struct type_Qmgr_ChannelList	*pepsy;
{
	Strlist	*head, *tail, *temp;

	head = tail = (Strlist *) NULL;
	
	while (pepsy != (struct type_Qmgr_ChannelList *) NULL) {
		if (pepsy->Channel) {
			temp = (Strlist *) calloc (1, sizeof(*temp));
			temp -> str = qb2str (pepsy->Channel);
			if (head == (Strlist *) NULL)
				head = tail = temp;
			else {
				tail -> next = temp;
				tail = temp;
			}
		}
		pepsy = pepsy -> next;
	}
	return head;
}

static RecipInfo *convert_RecipientInfo (pepsy)
register struct type_Qmgr_RecipientInfo	*pepsy;
{
	RecipInfo	*ret;

	if (pepsy == (struct type_Qmgr_RecipientInfo *) NULL)
		return (RecipInfo *) NULL;

	ret = (RecipInfo *) calloc (1, sizeof(*ret));
	
	ret -> user = qb2str(pepsy->user);
	
	if (pepsy->id)
		ret->id = pepsy->id->parm;

	if (pepsy->mta)
		ret->mta = qb2str(pepsy->mta);
	
	if (pepsy->channelList)
		ret->channels = convert_ChannelList(pepsy->channelList);

	ret->channelsDone = pepsy->channelsDone;
	ret->status = convert_ProcStatus(pepsy->procStatus);
	if (pepsy->info)
		ret->info = qb2str(pepsy->info);
	return ret;
}
	
static RecipInfo *convert_RecipientList (pepsy)
register struct type_Qmgr_RecipientList	*pepsy;
{
	RecipInfo	*head, *tail, *temp;

	head = tail = (RecipInfo *) NULL;
	
	while (pepsy != (struct type_Qmgr_RecipientList *) NULL) {
		if (pepsy -> RecipientInfo
		    && ((temp = 
			convert_RecipientInfo (pepsy -> RecipientInfo)) 
			!= (RecipInfo *) NULL)) {
			if (head == (RecipInfo *) NULL)
				head = tail = temp;
			else {
				tail -> next = temp;
				tail = temp;
			}
		}
		pepsy = pepsy -> next;
	}
	return head;
}
	
static MsgInfo	*convert_MsgStruct(pepsy)
register struct type_Qmgr_MsgStruct	*pepsy;
{
	MsgInfo	*ret;

	if (pepsy == (struct type_Qmgr_MsgStruct *) NULL)
		return (MsgInfo *) NULL;
	
	ret = (MsgInfo *) calloc (1, sizeof(*ret));
	
	ret -> permsginfo = convert_PerMessageInfo (pepsy -> messageinfo);
	
	ret -> recips = convert_RecipientList (pepsy -> recipientlist);
	
	return ret;
}

static MsgInfo	*convert_MsgStructList(pepsy)
register struct type_Qmgr_MsgStructList	*pepsy;
{
	MsgInfo	*head, *tail, *temp;

	head = tail = (MsgInfo *) NULL;
	
	while (pepsy != (struct type_Qmgr_MsgStructList *) NULL) {
		if (pepsy->MsgStruct
		    && (temp = convert_MsgStruct(pepsy->MsgStruct)) !=
		    (MsgInfo *) NULL) {
			if (head == (MsgInfo *) NULL)
				head = tail = temp;
			else {
				tail -> next = temp;
				tail = temp;
			}
		}
		pepsy = pepsy -> next;
	}

	return head;
}

static Strlist	*convert_QidList (pepsy)
register struct type_Qmgr_QidList	*pepsy;
{
	Strlist	*head, *tail, *temp;

	head = tail = (Strlist *) NULL;
	
	while (pepsy != (struct type_Qmgr_QidList *) NULL) {
		if (pepsy->QID) {
			temp = (Strlist *) calloc (1, sizeof(*temp));
			temp -> str = qb2str (pepsy->QID);
			if (head == (Strlist *) NULL)
				head = tail = temp;
			else {
				tail -> next = temp;
				tail = temp;
			}
		}
		pepsy = pepsy -> next;
	}
	return head;
}

static MsgList	*convert_MsgList(pepsy)
register struct type_Qmgr_MsgList	*pepsy;
{
	MsgList	*ret;
	
	if (pepsy == (struct type_Qmgr_MsgList *) NULL)
		return (MsgList *) NULL;
	
	ret = (MsgList *) calloc (1, sizeof(*ret));

	if (pepsy->msgs)
		ret->msgs = convert_MsgStructList (pepsy->msgs);
	if (pepsy->deleted)
		ret->deleted = convert_QidList (pepsy->deleted);
	return ret;
}


/*  */

free_MsgList(list)
MsgList	*list;
{
	if (list -> msgs)
		free_MsgInfo (list -> msgs);
	if (list -> deleted)
		free_Strlist (list -> deleted);
	free((char *) list);
}

free_Strlist(list)
Strlist	*list;
{
	Strlist *temp;

	while (list != (Strlist *) NULL) {
		if (list->str) free(list->str);
		temp = list;
		list = list -> next;
		free((char *) temp);
	}
}

free_MsgInfo (info)
MsgInfo	*info;
{
	MsgInfo	*temp;

	while ((MsgInfo *) NULL != info) {
		if (info->permsginfo)
			free_PerMessageInfo(info->permsginfo);
		if (info->recips)
			free_RecipInfo(info->recips);
		temp = info;
		info = info->next;
		free((char *) temp);
	}
}

free_PerMessageInfo (permsg)
PerMessageInfo	*permsg;
{
	if (permsg->qid) free(permsg->qid);
	if (permsg->country) free(permsg->country);
	if (permsg->admd) free(permsg->admd);
	if (permsg->prmd) free(permsg->prmd);
	if (permsg->local) free(permsg->local);
	if (permsg->originator) free(permsg->originator);
	if (permsg->contenttype) free(permsg->contenttype);
	if (permsg->eits) free_Strlist(permsg->eits);
	if (permsg->uaContentId) free(permsg->uaContentId);
	if (permsg->inChannel) free(permsg->inChannel);
	free((char *) permsg);
}

free_RecipInfo(recips)
RecipInfo	*recips;
{
	RecipInfo	*temp;
	while (recips) {
		if (recips->user) free(recips->user);
		if (recips->mta) free(recips->mta);
		if (recips->channels) free_Strlist(recips->channels);
		if (recips->status) free_ProcStatus (recips->status);
		if (recips->info) free(recips->info);
		temp = recips;
		recips = recips->next;
		free((char *) temp);
	}
}

/*  */

static struct type_Qmgr_UserList	*create_userlist(argv)
char	**argv;
{
	struct type_Qmgr_UserList	*temp,
	*head = NULL,
	*tail = NULL;
	int				i = 0;
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

	return head;
}
