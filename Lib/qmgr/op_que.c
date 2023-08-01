/* op_que.c: que ROS operation routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_que.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_que.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: op_que.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "consblk.h"
#include "qmgr-int.h"
#include "Qmgr-types.h"
#include <isode/cmd_srch.h>
#include <isode/rosap.h>

static CMD_TABLE ops_tbl[] = {
	QCTRL_ABORT,	int_Qmgr_QMGROp_abort,
	QCTRL_GRACEFUL,	int_Qmgr_QMGROp_gracefulTerminate,
	QCTRL_RESTART,	int_Qmgr_QMGROp_restart,
	QCTRL_REREAD,	int_Qmgr_QMGROp_rereadQueue,
	QCTRL_DISSUB,	int_Qmgr_QMGROp_disableSubmission,
	QCTRL_ENASUB,	int_Qmgr_QMGROp_enableSubmission,
	QCTRL_DISALL,	int_Qmgr_QMGROp_disableAll,
	QCTRL_ENAALL,	int_Qmgr_QMGROp_enableAll,
	QCTRL_INCMAX,	int_Qmgr_QMGROp_increasemaxchans,
	QCTRL_DECMAX,	int_Qmgr_QMGROp_decreasemaxchans,
	NULLCP,		NOTOK
};

/* ARGSUSED */
int arg_quecontrol (ad, args, arg)
int	ad;
char	**args;
struct type_Qmgr_QMGROp **arg;
{
	*arg = (struct type_Qmgr_QMGROp *) malloc(sizeof(**arg));
	(*arg)->parm = cmd_srch(args[0], ops_tbl);
	if ((*arg) -> parm == NOTOK)
		return NOTOK;
	return OK;
}

/* ARGSUSED */
int res_quecontrol (ad, id, dummy, result, roi)
int	ad,
	id,
	dummy;
struct type_Qmgr_Pseudo__qmgrControl	*result;
struct RoSAPindication			*roi;
{
	return OK;
}

/*  */

static QmgrStatus *convert_QmgrStatus();

/* ARGSUSED */
int res_qmgrStatus (ad, id, dummy, result, roi)
int	ad,
	id,
	dummy;
struct type_Qmgr_QmgrStatus	*result;
struct RoSAPindication	*roi;
{
	struct cons_opblk *op;

	if ((op = find_copblk(ad, id)) == NULL)
		return NOTOK;
	
	if (op -> fnx)
		(*op -> fnx) (convert_QmgrStatus(result), id);
	free_copblk (op);
	return OK;
}

/*  */

static QmgrStatus *convert_QmgrStatus(pepsy)
struct type_Qmgr_QmgrStatus	*pepsy;
{
	QmgrStatus	*ret;

	if ((struct type_Qmgr_QmgrStatus *) NULL == pepsy)
		return (QmgrStatus *) NULL;

	ret = (QmgrStatus *) malloc(sizeof(*ret));

	if (pepsy->boottime)
		ret->boottime = convert_time(pepsy->boottime);
	else
		ret->boottime = 0;

	ret->messagesIn = pepsy->messagesIn;
	ret->messagesOut = pepsy->messagesOut;
	ret->addrIn = pepsy->addrIn;
	ret->addrOut = pepsy->addrOut;
	ret->opsPerSec = pepsy->opsPerSec;
	ret->runnableChans = pepsy->runnableChans;
	ret->msgsInPerSec = pepsy->msgsInPerSec;
	ret->msgsOutPerSec = pepsy->msgsOutPerSec;
	ret->maxChans = pepsy->maxChans;
	ret->currChans = pepsy->currChans;
	ret->totalMsgs = pepsy->totalMsgs;
	ret->totalVolume = pepsy->totalVolume;
	ret->totalDRs = pepsy->totalDrs;

	return ret;
}
