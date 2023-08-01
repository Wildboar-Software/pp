/* op_chan.c: channel ROS operation routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_chan.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_chan.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: op_chan.c,v $
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

extern char	*mystrtotime();

static ChannelInfo *convert_ChanReadResult();
static ChannelInfo *convert_PrioChannelInfos();

#define CHAN_READ_INTERVAL	60

/* ARGSUSED */
int arg_channelread (ad, args, arg)
int				ad;
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

static ChannelInfo *convert_ChanReadResult();

/* ARGSUSED */
int res_channelread (ad, id, dummy, result, roi)
int	ad, id, dummy;
register struct type_Qmgr_ChannelReadResult	*result;
struct RoSAPindication	*roi;
{
	struct cons_opblk *op;

	if ((op = find_copblk(ad, id)) == NULL)
		return NOTOK;
	
	if (op -> fnx)
		(*op -> fnx)(convert_ChanReadResult(result), id);
	free_copblk(op);
	return OK;
}

/*  */

static CMD_TABLE control_tbl[] = {
	CTRL_ENABLE,	type_Qmgr_Control_start,
	CTRL_DISABLE,	type_Qmgr_Control_stop,
	CTRL_CACHECLEAR,type_Qmgr_Control_cacheClear,
	CTRL_CACHEADD,	type_Qmgr_Control_cacheAdd,
	NULLCP,		NOTOK
	};

/* ARGSUSED */ 
int arg_channelcontrol (ad, args, arg)
int				ad;
char				**args;
struct type_Qmgr_ChannelControl	**arg;
/* args[0] = channel */
/* args[1] = stop,start,clear,cacheadd */
/* args[2] = time */
{
	int n;

	*arg = (struct type_Qmgr_ChannelControl *) malloc(sizeof(**arg));
	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	(*arg)->channel = str2qb(args[0], strlen(args[0]), 1);

	if ((n = cmd_srch (args[1], control_tbl)) == NOTOK)
		return NOTOK;
	if (((*arg)->control->offset = n) == type_Qmgr_Control_cacheAdd) {
		char	*timestr;

		timestr = mystrtotime(args[2]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,
						      strlen(timestr),1);
		free(timestr);
	}
	return OK;
}

/* ARGSUSED */
int res_channelcontrol (ad, id, dummy, result, roi)
int	ad, id, dummy;
register struct type_Qmgr_PrioritisedChannelList	*result;
struct RoSAPindication	*roi;
{
	struct cons_opblk *op;

	if ((op = find_copblk(ad, id)) == NULL)
		return NOTOK;
	
	if (op -> fnx)
		(*op -> fnx)(convert_PrioChannelInfos(result), id);
	free_copblk(op);
	return OK;
}

/*  */

static ChannelInfo	*convert_ChannelInfo(pepsy)
register struct type_Qmgr_ChannelInfo	*pepsy;
{
	ChannelInfo	*ret;

	if (pepsy == (struct type_Qmgr_ChannelInfo *) NULL)
		return (ChannelInfo *) NULL;

	ret = (ChannelInfo *) calloc (1, sizeof(*ret));

	ret->channelname = qb2str(pepsy->channel);
	ret->channeldescrip = qb2str(pepsy->channelDescription);

	ret->oldestMessage = convert_time(pepsy->oldestMessage); 
	ret->numberMessages = pepsy->numberMessages;
	ret->numberReports = pepsy->numberReports;
	ret->volumeMessages = pepsy->volumeMessages;
	ret->numberMtas = pepsy->numberMtas;
	ret->numberActiveProcesses = pepsy->numberActiveProcesses;
	ret->status = convert_ProcStatus(pepsy->status);
	ret->inbound = bit_test(pepsy->direction, bit_Qmgr_direction_inbound);
	ret->outbound = bit_test(pepsy->direction, bit_Qmgr_direction_outbound);
	ret->chantype = pepsy->chantype;
	ret->maxprocs = pepsy->maxprocs;
	ret->numberMtas = pepsy->numberMtas;

	return ret;
}	

static ChannelInfo	*convert_PrioChannelInfos(pepsy)
register struct type_Qmgr_PrioritisedChannelList	*pepsy;
{
	ChannelInfo	*head, *tail, *temp;
	
	head = tail = (ChannelInfo *) NULL;
	
	while (pepsy != (struct type_Qmgr_PrioritisedChannelList *) NULL) {
		if ((temp = 
		     convert_ChannelInfo(pepsy->PrioritisedChannel->channel))
		    == (ChannelInfo *) NULL) {
			free_ChannelInfo(head);
			return (ChannelInfo *) NULL;
		}
		temp -> priority =
			pepsy -> PrioritisedChannel -> priority -> parm;
		if (head == NULL)
			head = tail = temp;
		else {
			tail -> next = temp;
			tail = temp;
		}
		pepsy = pepsy -> next;
	}
	return head;
}

static ChannelInfo	*convert_ChanReadResult(pepsy)
register struct type_Qmgr_ChannelReadResult	*pepsy;
{
	return convert_PrioChannelInfos(pepsy -> channels);
}

/*  */

free_ChannelInfo(list)
ChannelInfo	*list;
{
	ChannelInfo	*temp;

	while (list != (ChannelInfo *) NULL) {
		if (list -> channelname) free (list -> channelname);
		if (list -> channeldescrip) free (list -> channeldescrip);
		if (list -> status) free_ProcStatus (list -> status);
		temp = list;
		list = list -> next;
		free ((char *) temp);
	}
}

	
