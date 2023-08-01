/* qmgr-load.c: load the queue manager via readqueue */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/qmgr-load.c,v 6.0 1991/12/18 20:11:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/qmgr-load.c,v 6.0 1991/12/18 20:11:26 jpo Rel $
 *
 * $Log: qmgr-load.c,v $
 * Revision 6.0  1991/12/18  20:11:26  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <pwd.h>
#include "retcode.h"
#include <isode/usr.dirent.h>
#include <sys/stat.h>
#include "Qmgr-ops.h"                   /* operation definitions */
#include "Qmgr-types.h"                 /* type definitions */
#include "ryresponder.h"                /* for generic idempotent responders */

/* Sumbit types */
#include "q.h"
#include "prm.h"

#define	DEFAULT_MSG_CHUNKS	50


/* Outside routines */
struct type_Qmgr_MsgStruct      *qstruct2qmgr();
void                            qinit();
CHAN 		*mychan = NULLCHAN;
int		msg_chunks;
extern char     *quedfldir;
extern void	rd_end(), sys_init(), err_abrt(), getfpath();
static char *myservice = "qmgr-load";

/* OPERATIONS */
static int     op_readqueue();

static struct server_dispatch dispatches[] = {
	"readqueue", operation_Qmgr_readqueue, op_readqueue,
	NULL
	};

static int get_msg_list();
static struct type_Qmgr_MsgStructList *get_n_messages();
static void dirinit ();
static struct type_Qmgr_MsgStruct *readInMsg();

/*  MAIN */

/* ARGSUSED */
main (argc, argv, envp)
int     argc;
char    **argv,
	**envp;
{
	register CHAN	**chp;
	sys_init (argv[0]);
	dirinit ();

	for (chp = ch_all; 
	     *chp != NULLCHAN && (*chp) -> ch_chan_type != CH_QMGR_LOAD;
	     chp++)
		continue;

	if (*chp != NULLCHAN)
		mychan = *chp;

	if (mychan == NULLCHAN
	    || mychan->ch_out_info == NULLCP
	    || (msg_chunks = atoi(mychan->ch_out_info)) <= 0)
		msg_chunks = DEFAULT_MSG_CHUNKS;

#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0)) {
		do_debug ();
		exit (0);
	} 
#endif
	ryresponder (argc, argv, PLocalHostName(), myservice, dispatches,
		     table_Qmgr_Operations, NULLIFP, NULLIFP); 
	exit (0);       /* NOT REACHED */
}

/* routine to move to correct place in file system */
static void dirinit ()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, " Unable to change directory to '%s'",
			  quedfldir);
}

#ifdef PP_DEBUG
do_debug ()
{
	struct type_Qmgr_MsgStructList *ml, *mp;
	struct type_Qmgr_MsgList *mlp;

	mlp = (struct type_Qmgr_MsgList *) smalloc (sizeof *mlp);
	mlp -> deleted = NULL;

	printf ("start load\n");
	while ((ml = get_n_messages (msg_chunks)) != NULL) {
		mlp -> msgs = ml;

		printf ("\tstart batch\n");
		for (mp = ml; mp; mp = mp -> next) {
			char *p = qb2str(mp -> MsgStruct ->
					 messageinfo -> queueid);
			printf ("\t\tload %s\n", p);
			free (p);
		}
		printf ("\tend batch\n");
		free_Qmgr_MsgList (mlp);
		mlp = (struct type_Qmgr_MsgList *) smalloc (sizeof *mlp);
		mlp -> deleted = NULL;
	}
	printf ("end load\n");
}
#endif


/*  OPERATIONS */

/* ARGSUSED */
static int op_readqueue (ad, ryo, rox, in, roi)
int                     ad;
struct RyOperation      *ryo;
struct RoSAPinvoke      *rox;
caddr_t                 in;
struct RoSAPindication  *roi;
{
	if (rox->rox_nolinked == 0) {
		PP_LOG (LLOG_NOTICE,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			ad, ryo->ryo_name, rox->rox_linkid));
		return ureject (ad, ROS_IP_LINKED, rox, roi);
	}
	PP_LOG (LLOG_NOTICE,
		("RO-INVOKE.INDICATION/%d: %s",ad, ryo -> ryo_name));
	return get_msg_list(ad, rox, roi);
}

static int get_msg_list(ad, rox, roi)
int                     ad;
struct RoSAPinvoke      *rox;
struct RoSAPindication  *roi;
{
	struct type_Qmgr_MsgList *base;

	PP_TRACE(("get_msg_list ()"));

	if ((base = (struct type_Qmgr_MsgList *) calloc (1, sizeof(*base)))
	     == NULL)
		return error (ad, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);

	/* now make list */
	if ((base->msgs = get_n_messages(msg_chunks)) == 
	    (struct type_Qmgr_MsgStructList *) NOTOK) {
		base->msgs = NULL;
		return error (ad, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
	}

	if (RyDsResult (ad, rox->rox_id, (caddr_t) base, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios(&roi->roi_preject,"RESULT");
	free_Qmgr_MsgList (base);
	return OK;
}

#define SIZE_INCR 1000
static int	count = 0, noEntries = 0, maxnumb = 0;
static char	**msgs = NULLVP;

static struct type_Qmgr_MsgStructList *get_n_messages(num)
int	num;
{
	struct type_Qmgr_MsgStructList *head, *tail, *temp;
	struct type_Qmgr_MsgStruct *msg;
	extern char *quedfldir;
	int i;

	head = tail = NULL;
	
	if (noEntries > 0 && count == noEntries) {
		return head;
	}

	if (noEntries == 0) {
		if (msgs) {
			free_hier(msgs, noEntries);
			msgs = NULLVP;
		}
		maxnumb = 0;
		hier_scanQ (quedfldir, NULLCP, 
			    &noEntries, &msgs, 
			    &maxnumb, NULLIFP);
		count = 0;
	}

	i = 0;
	while (count < noEntries && i < num) {
		PP_TRACE (("reading message %s", msgs[count]));
		if ((msg = readInMsg(msgs[count++])) != NULL) {
			/* add to list */
			temp = (struct type_Qmgr_MsgStructList *) 
				calloc(1,sizeof(*temp));
			temp->MsgStruct = msg;
			if (head == NULL)
				head = tail = temp;
			else {
				tail->next = temp;
				tail = temp;
			}
			i++;
		}
	}
	return head;
}

static struct type_Qmgr_MsgStruct *readInMsg(name)
char *name;
{
	struct type_Qmgr_MsgStruct *arg = NULL;
	struct prm_vars prm;
	Q_struct        que;
	ADDR            *sender = NULL;
	ADDR            *recips = NULL;
	int             rcount, result;

	result = OK;

	prm_init (&prm);
	q_init (&que);
	qinit (&que);

	if ((result == OK)
	    && (rp_isbad(rd_msg(name,&prm,&que,&sender,&recips,&rcount)))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/qmgr-load rd_msg err: '%s'",name));
		result = NOTOK;
	}
	/* unlock message */
	rd_end();

	if ((result == OK)
	    && ((arg = qstruct2qmgr(name,&prm,&que,sender,recips,rcount)) == NULL)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/qmgr-load qstruct2qmgr err: '%s'",name));
		result = NOTOK;
	}

	/* free all storage */
	q_free (&que);

	return result == NOTOK ? NULL : arg;
}


/*    ERROR */

int  error (sd, err, param, rox, roi)
int     sd,
	err;
caddr_t param;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	if (RyDsError (sd, rox -> rox_id, err, param, ROS_NOPRIO, roi) == NOTOK)
		ros_adios (&roi -> roi_preject, "ERROR");

	return OK;
}

int ureject (sd, reason, rox, roi)
int     sd, reason;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	if ( RyDsUReject (sd, rox -> rox_id, reason, ROS_NOPRIO, roi) == NOTOK)
		ros_adios (&roi -> roi_preject, "U-REJECT");
	return OK;
}
