/* p2explode: channel to explode a P2 message into body parts */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/p2explode/RCS/explode_chan.c,v 6.0 1991/12/18 20:20:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/p2explode/RCS/explode_chan.c,v 6.0 1991/12/18 20:20:02 jpo Rel $
 *
 * $Log: explode_chan.c,v $
 * Revision 6.0  1991/12/18  20:20:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "q.h"
#include "prm.h"
#include "retcode.h"
#include "tb_bpt88.h"
#include "dr.h"
#include <sys/stat.h>
#include "sys.file.h"
#include <isode/cmd_srch.h>

extern CMD_TABLE bptbl_body_parts88[/* x400 84 body_parts */];
extern char	*quedfldir;
extern CHAN	*ch_nm2struct();
extern char	*cont_p2;
extern void	rd_end(), sys_init(), err_abrt();
extern struct type_Qmgr_DeliveryStatus	*delivery_resetDRs();
extern int	err_fatal;
static CHAN	*mychan;
static char	*this_msg, *this_chan;

static int	processMsg();
static int	doSplit();
static struct type_Qmgr_DeliveryStatus	*process();
static int				initialise();
static int				security_check(), x40084;
static void				dirinit();
static ADDR				*getnthrecip ();
int first_failureDR;
/*  */
/* main routine */

main(argc, argv)
int 	argc;
char	**argv;
{
	sys_init(argv[0]);
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control (argc, argv, initialise, process, NULLIFP);
	else
#endif
	channel_control(argc, argv, initialise, process, NULLIFP);
}

/*  */
/* move to correct place in file system */

static void dirinit ()
{
	if (chdir (quedfldir) < 0)
		err_abrt( RP_LIO, " Unable to change directory to '%s'",
			quedfldir);
}

/*  */
/* channel initialise routine */

static int initialise (arg)
struct type_Qmgr_Channel	*arg;
{
	char	*name;

	name = qb2str(arg);

	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP, 
			("Chans/p2explode : Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}

	return OK;
}

/*  */

static int security_check (msg)
struct type_Qmgr_ProcMsg	*msg;
{
	char	*msg_file = NULL,
		*msg_chan = NULL;
	int	result;

	result = TRUE;
	msg_file = qb2str(msg->qid);
	msg_chan = qb2str(msg->channel);

	if ((mychan == NULLCHAN) || (strcmp(msg_chan, mychan->ch_name) != 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/explode_chan channel err: '%s'",msg_chan));
		result = FALSE;
	}

	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	return result;
}

/*  */
/* routine to do explode */

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg	*arg;
{
	struct prm_vars			prm;
	Q_struct			que;
	ADDR				*sender = NULL;
	ADDR				*recips = NULL;
	int				rcount, retval;
	struct type_Qmgr_UserList	*ix;
	ADDR				*adr;
	char				*error;

	bzero((char *)&que,sizeof(que));
	bzero((char *)&prm,sizeof(prm));
	first_failureDR = TRUE;
	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_messageFailure);
		
	if (security_check(arg) != TRUE)
		return deliverystate;

	if (this_msg != NULL) free(this_msg);
	if (this_chan != NULL) free(this_chan);

	this_msg = qb2str(arg->qid);
	this_chan = qb2str(arg->channel);

	PP_LOG(LLOG_TRACE,
		("P2 explode on msg '%s' through '%s'",this_msg,this_chan));
		
	if (rp_isbad(rd_msg(this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/explode_chan rd_msg err: '%s'",this_msg));
		rd_end();
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Can't read message");
	}

	for (ix = arg->users; ix; ix=ix->next) {
		error = NULLCP;
		err_fatal = NOTOK;
		if ((adr = getnthrecip(&que, ix->RecipientId->parm)) == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
				("Chans/explode_chan : failed to find recipient %d",ix->RecipientId->parm));
			delivery_setstate(ix->RecipientId->parm,
					  int_Qmgr_status_messageFailure,
					  "Unable to find specified recipient");
			continue;
		}
		switch(chan_acheck (adr, mychan, 1, (char **) NULL)) {
		    case OK:
			if (processMsg(this_msg,adr,&que,&error) == NOTOK) {
				if (err_fatal == OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Chans/explode_chan : failed to process message '%s' for recipient %d: %s",this_msg,adr->ad_no, error));
					set_1dr(&que, adr->ad_no, this_msg,
						DRR_CONVERSION_NOT_PERFORMED,
						DRD_CONTENT_SYNTAX_ERROR,
						(error == NULLCP) ? "Unable to parse the p2" : error);
					delivery_set(adr->ad_no, 
						     (first_failureDR == TRUE) ? int_Qmgr_status_negativeDR : int_Qmgr_status_failureSharedDR);
					first_failureDR = FALSE;
				} else
					delivery_setstate(adr -> ad_no,
							  int_Qmgr_status_messageFailure,
							  (error == NULLCP) ? "" : error);
			} else {
				adr->ad_rcnt++;
				wr_ad_rcntno(adr, adr->ad_rcnt);
				delivery_set(adr->ad_no,int_Qmgr_status_success);
			}
			if (error != NULLCP) free(error);
			break;
		    default:
			break;
		}
	}
	if (rp_isbad(retval = wr_q2dr(&que, this_msg))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure '%d'",mychan->ch_name,retval));
		delivery_resetDRs(int_Qmgr_status_messageFailure);
	}
	rd_end();
	q_free (&que);
	prm_free(&prm);
	return deliverystate;
}

/*  */

extern LIST_BPT	*outbound_bpts, *outbound_hdrs;

/* returns ok if processed msg on mychan for recip */
static int processMsg (msg,recip, qp, perr)
char	*msg;
ADDR	*recip;
Q_struct	*qp;
char		**perr;
{
	char		*origdir = NULL,
			*newdir = NULL,
			*encr_str = NULLCP;
	struct stat	statbuf;
	
	if (qid2dir(msg, recip, TRUE, &origdir) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/explode_chan original directory not found for recipient %d of message '%s'",recip->ad_no, msg));
		*perr = strdup("Unable to find source directory");
		return NOTOK;
	}

	/* temporary change to get new directory name */
	recip->ad_rcnt++;
	if (qid2dir(msg, recip, FALSE, &newdir) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/explode_chan couldn't construct new directory name for recipient %d of message '%s'", recip->ad_no, msg));
		*perr = strdup ("Unable to construct new directory");
		return NOTOK;
	}
	recip->ad_rcnt--;

	if (stat(newdir, &statbuf) == OK
		&& (statbuf.st_mode & S_IFMT) == S_IFDIR) {
			/* new directory already exists, os processing already done */
			if (origdir != NULL) free(origdir);
			if (newdir != NULL) free(newdir);
			return OK;
	}
	x40084 = (lexequ(qp->cont_type, cont_p2) == 0) ? TRUE : FALSE;
       
	/* HACK for undeclared bodyparts that are supported by outchan */
	if (recip -> ad_outchan && recip -> ad_outchan -> li_chan
	    && recip -> ad_outchan -> li_chan -> ch_bpt_out)
		outbound_bpts = recip -> ad_outchan -> li_chan -> ch_bpt_out;
	else
		outbound_bpts = NULLIST_BPT;

	if (recip -> ad_outchan && recip -> ad_outchan -> li_chan
	    && recip -> ad_outchan -> li_chan -> ch_hdr_out)
		outbound_hdrs = recip -> ad_outchan -> li_chan -> ch_hdr_out;
	else
		outbound_hdrs = NULLIST_BPT;
	

	if (doSplit(origdir,newdir,msg,qp,perr) != OK)
		return NOTOK;

	if (origdir != NULL) free(origdir);
	if (newdir != NULL) free(newdir);
	return OK;
}

/*  */
/* explodes p2 in orig into files in new */
static int doSplit (old, new,msg,qp,perr)
char	*old,
	*new,
	*msg;
Q_struct	*qp;
char		**perr;
{
	struct stat	statbuf;
	char		tmpdir[MAXPATHLENGTH], buf[BUFSIZ];
	int		result = OK;
	
	(void) sprintf(tmpdir, "%s/tmp.%s",
		       msg,mychan->ch_name);

	if (stat(tmpdir, &statbuf) == OK) {
		/* exists so remove it */
		char *cmdline = malloc((unsigned) (strlen("rm -rf ") + strlen(tmpdir) + 1));
		(void) sprintf(cmdline, "rm -rf %s",tmpdir);
		system(cmdline);
		if (cmdline != NULL) free(cmdline);
	}

	if (mkdir(tmpdir, 0777) != OK) {
		PP_SLOG(LLOG_EXCEPTIONS, tmpdir,
			("Can't make directory"));
		(void) sprintf(buf,
			       "Failed to make temporary directory '%s'",
			       tmpdir);
		*perr = strdup(buf);
		return NOTOK;
	}
	result = unflatten(old,tmpdir, x40084, qp, perr);
	if ((result == OK) && (rename(tmpdir,new) == -1)) {
		PP_SLOG(LLOG_EXCEPTIONS, "rename",
			("Can't rename directory '%s' to '%s'",
			 tmpdir, new));
		result = NOTOK;
	}
	return result;
}

static ADDR *getnthrecip(que, num)
Q_struct	*que;
int		num;
{
	ADDR *ix = que->Raddress;

	if (num == 0)
		return que->Oaddress;
	while ((ix != NULL) && (ix->ad_no != num))
		ix = ix->ad_next;
	return ix;
}

