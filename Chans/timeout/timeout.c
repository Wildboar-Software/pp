/* timeout.c: message timeout channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/timeout/RCS/timeout.c,v 6.0 1991/12/18 20:12:51 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/timeout/RCS/timeout.c,v 6.0 1991/12/18 20:12:51 jpo Rel $
 *
 * $Log: timeout.c,v $
 * Revision 6.0  1991/12/18  20:12:51  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "dbase.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "q.h"
#include "dr.h"
#include "prm.h"


extern char     *quedfldir;
extern CHAN     *ch_nm2struct();
extern void	sys_init(), rd_end(), err_abrt();
CHAN            *mychan;

static struct type_Qmgr_DeliveryStatus *process ();
static int initialise ();
static void dirinit ();
static int security_check ();
static ADDR *get_address ();

/*  */
/* main routine */

main (argc, argv)
int     argc;
char    **argv;
{
	sys_init(argv[0]);
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control(argc,argv,initialise,process,NULLIFP);
	else
#endif
		channel_control (argc, argv, initialise, process,NULLIFP);
}

/*  */
/* routine to move to correct place in file system */

static void dirinit ()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, " Unable to change directory to '%s'",
			  quedfldir);
}

/*  */
/* channel initialise routine */

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char *name;

	name = qb2str(arg);

	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP,
			("Chans/timeout : Channel '%s' not known",name));
		if (name) free(name);
		return NOTOK;
	}

	/* check if a timeout channel */
	if (mychan->ch_chan_type != CH_TIMEOUT) {
		PP_OPER(NULLCP,
			("timeout: Channel '%s' not specified as type timeout",
			 name));
		if (name) free(name);
		return NOTOK;
	}
	if (name) free(name);
	return OK;
}

/*  */
/* routine called to clean do timeout out */

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars prm;
	Q_struct        que;
	ADDR            *sender;
	ADDR            *recips;
	int             rcount;
	char            *this_msg;
	ADDR            *adr;

	prm_init (&prm);
	q_init (&que);

	sender = NULLADDR;
	recips = NULLADDR;

	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_messageFailure);

	if (security_check(arg) != TRUE)
		return deliverystate;

	/* ok-timeout message */
	this_msg = qb2str (arg->qid);

	PP_LOG(LLOG_NOTICE,
	       ("timing out msg %s", this_msg));

	if (rp_isbad(rd_msg(this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/timeout rd_msg err: '%s'",this_msg));
		rd_end();
		/* free all storage used */
		if (this_msg != NULL) free(this_msg);
		q_free (&que);

		return deliverystate;
	}

	for (adr = que.Raddress; adr; adr = adr -> ad_next)
		if (adr -> ad_status == AD_STAT_PEND) 
			set_1dr(&que, adr -> ad_no, this_msg,
				DRR_UNABLE_TO_TRANSFER,
				DRD_MAX_TIME_EXPIRED,
				NULLCP);
	if (arg->users != NULL
	    && (adr = get_address(arg->users->RecipientId->parm, &que)) != 0
	    && !rp_isbad(wr_q2dr(&que, this_msg)))
		 delivery_set(adr -> ad_no, int_Qmgr_status_negativeDR);
	else
		delivery_set(adr -> ad_no, int_Qmgr_status_messageFailure);

	/* unlock file */
	rd_end();


	/* free all storage used */
	if (this_msg != NULL) free(this_msg);
	q_free (&que);
	prm_free(&prm);

	return deliverystate;
}

/*  */
/* routine to check if allo<wed to timeout this message */
static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
	char *msg_file = NULL, *msg_chan = NULL;
	int result;

	result = TRUE;
	msg_file = qb2str (msg->qid);
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp(msg_chan,mychan->ch_name) !=0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/timeout channel err: '%s'",msg_chan));
		result = FALSE;
	}
	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	return result;
}

/* routine to extract the required address from the Q_struct */
static ADDR *get_address (usr, que)
int             usr;
Q_struct        *que;
{
	ADDR *ix;

	if (usr == 0)
		/* originator */
		return que->Oaddress;

	ix = que->Raddress;
	while ((ix != 0) && (--usr > 0))
		ix = ix->ad_next;

	return ix;
}
