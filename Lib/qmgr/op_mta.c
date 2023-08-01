/* op_mta.c: mta ROS operation routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_mta.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_mta.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: op_mta.c,v $
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

#define 	MTA_READ_INTERVAL	60	/* in secs */

/* ARGSUSED */
int arg_mtaread (ad, args, arg)
int				ad;
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

static MtaInfo *convert_PrioritisedMtaList();

/* ARGSUSED */
int res_mtaread (ad, id, dummy, result, roi)
int	ad, id, dummy;
register struct type_Qmgr_PrioritisedMtaList	*result;
struct RoSAPindication	*roi;
{
	struct cons_opblk *op;

	if ((op = find_copblk(ad, id)) == NULL)
		return NOTOK;
	
	if (op -> fnx)
		(*op -> fnx) (convert_PrioritisedMtaList(result), id);
	free_copblk (op);
	return OK;
}

/*  */

extern char *mystrtotime();
static CMD_TABLE control_tbl[] = {
	CTRL_ENABLE,	type_Qmgr_Control_start,
	CTRL_DISABLE,	type_Qmgr_Control_stop,
	CTRL_CACHECLEAR,type_Qmgr_Control_cacheClear,
	CTRL_CACHEADD,	type_Qmgr_Control_cacheAdd,
	NULLCP,		NOTOK
	};

/* ARGSUSED */
int arg_mtacontrol (ad, args, arg)
int				ad;
char				**args;
struct type_Qmgr_MtaControl	**arg;
/* args[0] = channel */
/* args[1] = mta */
/* args[2] = stop,start,clear,cacheadd */
/* args[3] = time */
{
	int n;

	*arg = (struct type_Qmgr_MtaControl *) malloc(sizeof(**arg));
	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	(*arg)->channel = str2qb(args[0], strlen(args[0]), 1);
	(*arg)->mta = str2qb(args[1], strlen(args[1]), 1);

	if ((n = cmd_srch (args[2], control_tbl)) == NOTOK)
		return NOTOK;

	if (((*arg)->control->offset = n) == type_Qmgr_Control_cacheAdd) {
		char	*timestr;

		timestr = mystrtotime(args[3]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
	}
	return OK;
}

static MtaInfo *convert_MtaInfo();

/* ARGSUSED */
int res_mtacontrol (ad, id, dummy, result, roi)
int	ad, id, dummy;
register struct type_Qmgr_MtaInfo	*result;
struct RoSAPindication	*roi;
{
	struct cons_opblk *op;

	if ((op = find_copblk(ad, id)) == NULL)
		return NOTOK;
	
	if (op -> fnx)
		(*op -> fnx) (convert_MtaInfo(result), id);
	free_copblk(op);
	return OK;
}

/*  */

static MtaInfo	*convert_MtaInfo(pepsy)
register struct type_Qmgr_MtaInfo	*pepsy;
{
	MtaInfo	*ret;

	if (pepsy == (struct type_Qmgr_MtaInfo *) NULL)
		return (MtaInfo *) NULL;

	ret = (MtaInfo *) calloc (1, sizeof(*ret));

	ret->channel = qb2str(pepsy->channel);
	ret->mta = qb2str(pepsy->mta);
	ret->oldestMessage = convert_time(pepsy->oldestMessage);
	ret->numberMessages = pepsy->numberMessage;
	ret->volumeMessages = pepsy->volumeMessages;
	ret->status = convert_ProcStatus(pepsy->status);
	ret->numberReports = pepsy->numberDRs;
	ret->active = pepsy->active;
	if (pepsy->info)
		ret->info = qb2str(pepsy->info);

	return ret;
}

static MtaInfo *convert_PrioritisedMtaList(pepsy)
register struct type_Qmgr_PrioritisedMtaList	*pepsy;
{
	MtaInfo	*head, *tail, *temp;

	head = tail = (MtaInfo *) NULL;
	
	while (pepsy != (struct type_Qmgr_PrioritisedMtaList *) NULL) {
		if ((temp =
		     convert_MtaInfo(pepsy->PrioritisedMta->mta))
		    == (MtaInfo *) NULL) {
			free_MtaInfo(head);
			return (MtaInfo *) NULL;
		}
		temp -> priority =
			pepsy->PrioritisedMta->priority->parm;
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

/*  */

free_MtaInfo(list)
MtaInfo	*list;
{
	MtaInfo	*temp;

	while (list != (MtaInfo *) NULL) {
		if (list -> channel) free (list -> channel);
		if (list -> mta) free (list -> mta);
		if (list -> status) free_ProcStatus (list -> status);
		if (list -> info) free (list -> info);
		temp = list;
		list = list -> next;
		free ((char *) temp);
	}
}
