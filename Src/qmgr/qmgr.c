/* qmgr.c: Queue manager main routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/qmgr/RCS/qmgr.c,v 6.0 1991/12/18 20:27:38 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/qmgr.c,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: qmgr.c,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <sys/signal.h>
#include "ryresponder.h"                /* for generic idempotent responders */
#include "qmgr.h"                   /* operation definitions */
#include "types.h"

static char *myservice = "pp qmgr";
extern char *quedfldir;
static SFD debug_on (), debug_off (), shutdown (), spurious ();
int ureject ();

/* OPERATIONS */
static int
	op_newmessage (), op_readmsginfo (),
	op_channelbegin (), op_channelread (),
	op_channelcontrol (), op_mtaread (), op_mtacontrol (),
	op_messagecontrol (), op_qmgrcontrol (),
	op_readchannelmtamessage (), op_qmgrstatus ();


static struct server_dispatch dispatches[] = {
	"newmessage",       operation_Qmgr_newmessage,      op_newmessage,
	"readmsginfo",      operation_Qmgr_readmsginfo,     op_readmsginfo,
	"channelbegin",     operation_Qmgr_channelbegin,    op_channelbegin,
	"channelread",      operation_Qmgr_channelread,     op_channelread,
	"channelcontrol",   operation_Qmgr_channelcontrol,  op_channelcontrol,
	"mtaread",          operation_Qmgr_mtaread,         op_mtaread,
	"mtacontrol",       operation_Qmgr_mtacontrol,      op_mtacontrol,
	"msgcontrol",	    operation_Qmgr_msgcontrol,	    op_messagecontrol,
	"qmgrcontrol",	    operation_Qmgr_qmgrControl,	    op_qmgrcontrol,
	"readchannelmtamessage",
			    operation_Qmgr_readChannelMtaMessage,
						op_readchannelmtamessage,
	"qmgrstatus",	    operation_Qmgr_qmgrStatus,	    op_qmgrstatus,
	NULL
	};

extern int	chan_lose ();
extern int	start_routine ();
extern char *qmgr_hostname;

time_t	current_time;
time_t	debris_time = TRASH_RETRY_INTERVAL, load_time = LOAD_RETRY_INTERVAL;
time_t	cache_time = CACHE_TIME, timeout_time = TIMEOUT_RETRY_INTERVAL;
int	chan_state = 1;
int	maxchansrunning = MAXCHANSRUNNING;
int	submission_disabled = 0;
int	nobackground = 0;
int	opmode = 0;
int 	nocdir = 0;
int	nodisablemsgclean = 0;

/*    MAIN */

main (argc, argv)
int     argc;
char  **argv;
{
	int	opt;
	extern char *optarg;
	extern char *pptailor;

	myname = argv[0];
	(void) time (&current_time);
	while ((opt = getopt (argc, argv, "bDd:l:c:Ct:m:MsT:X")) != EOF) {
		switch (opt) {
		    case 'b':
			nobackground = 1;
			break;
		    case 'C':
			nocdir = 1;
			break;
		    case 'D':
			chan_state = 0;
			break;

		    case 's':
			submission_disabled = 1;
			break;

		    case 'l':
			if ((load_time = atoi(optarg)) < 0)
				load_time = LOAD_RETRY_INTERVAL;
			else	load_time *= 60 * 60;
			break;

		    case 'd':
			if ((debris_time = atoi(optarg)) < 0)
				debris_time = TRASH_RETRY_INTERVAL;
			else	debris_time *= 60 * 60;
			break;

		    case 'c':
			if ((cache_time = atoi (optarg)) < 0)
				cache_time = CACHE_TIME;
			break;

		    case 't':
			if ((timeout_time = atoi (optarg)) < 0)
				timeout_time = TIMEOUT_RETRY_INTERVAL;
			else	timeout_time *= 60 * 60;
			break;

		    case 'm':
			if ((maxchansrunning = atoi (optarg)) < 1)
				maxchansrunning = MAXCHANSRUNNING;
			break;

		    case 'M':
			nodisablemsgclean = 1;
			break;

		    case 'T':
			pptailor = optarg;
			break;

		    case 'X':
			debug_on ();
			break;
		    default:
			adios (NULLCP,
"Usage: %s [-bCDNMsX] [-d hours] [-l hours] [-t hours]\n\t\
[-c secs] [-m nchans] [-T tailorfile]",
			       myname);
			break;
		}
	}
	sys_init (myname);
	srandom(getpid());

#if defined(SIGUSR1) && defined(SIGUSR2)
	(void) signal (SIGUSR1, debug_on);
	(void) signal (SIGUSR2, debug_off);
#endif
	(void) signal (SIGTERM, shutdown);
	(void) signal (SIGPIPE, spurious);
	(void) signal (SIGALRM, spurious);

	if (nocdir == 0 && chdir (quedfldir) == NOTOK)
		PP_LOG (LLOG_EXCEPTIONS, ("can't cd to %s", quedfldir));
		
	(void) ryresponder (argc, argv,  qmgr_hostname, myservice, dispatches,
			    table_Qmgr_Operations, start_routine,  chan_lose);

	exit (0);                       /* NOTREACHED */
}

/*    OPERATIONS */

static int  op_newmessage (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	register struct type_Qmgr_MsgStruct *arg =
		(struct type_Qmgr_MsgStruct *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_TRACE(("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return add_msg (sd, arg, rox, roi);
}

static int op_readmsginfo (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	register struct type_Qmgr_ReadMessageArgument *arg =
		(struct type_Qmgr_ReadMessageArgument *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}

	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return read_msg (sd, arg, rox, roi);
}

static int op_channelbegin (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_FilterList *arg = (struct type_Qmgr_FilterList *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return chan_begin (sd, arg, rox, roi);
}

/* ARGSUSED */
static int op_channelread (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_UNIV_UTCTime *arg = (struct type_UNIV_UTCTime *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));

		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return channel_list (sd, arg, rox, roi);
}

static int op_mtacontrol (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_MtaControl *arg =
		(struct type_Qmgr_MtaControl *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return mta_control (sd, arg, rox, roi);
}

static int op_channelcontrol (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_ChannelControl *arg =
		(struct type_Qmgr_ChannelControl *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return chan_control (sd, arg, rox, roi);
}

static int op_mtaread (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_MtaRead *arg = (struct type_Qmgr_MtaRead *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return mta_read (sd, arg, rox, roi);
}

static int op_messagecontrol (sd, ryo, rox, in, roi)
int	sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_MsgControl *arg = (struct type_Qmgr_MsgControl *)in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return msg_control (sd, arg, rox, roi);

}

static int op_qmgrcontrol (sd, ryo, rox, in, roi)
int	sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_QMGROp *op = (struct type_Qmgr_QMGROp *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return qmgrcontrol (sd, (int)op -> parm, rox, roi);
}

static int op_readchannelmtamessage (sd, ryo, rox, in, roi)
int	sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_MsgRead *mr = (struct type_Qmgr_MsgRead *) in;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return msgread (sd, mr, rox, roi);
}

/* ARGSUSED */
static int op_qmgrstatus (sd, ryo, rox, in, roi)
int	sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			 sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_NOTICE (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	return qmgrstatus (sd, rox, roi);
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

/* debug control */

static int	debugging = -1;

/* ARGSUSED */
static SFD debug_on (sig)
int sig;
{
	if (debugging == -1)
		debugging = pp_log_norm -> ll_events;
	if ((pp_log_norm -> ll_events & LLOG_NOTICE) == 0)
		pp_log_norm -> ll_events |= LLOG_NOTICE;
	else if ((pp_log_norm -> ll_events & LLOG_TRACE) == 0)
		pp_log_norm -> ll_events |= LLOG_TRACE;
	else
		pp_log_norm -> ll_events |= LLOG_DEBUG;
	ll_close (pp_log_norm);
}

/* ARGSUSED */
static SFD debug_off (sig)
int sig;
{
	if (debugging == -1)
		debugging = pp_log_norm -> ll_events;
	if (debugging != -1) {
		if (pp_log_norm -> ll_events != debugging)
			pp_log_norm -> ll_events = debugging;
		else
			pp_log_norm -> ll_events &= ~LLOG_NOTICE;
		ll_close (pp_log_norm);
	}
}

/* ARGSUSED */
static SFD shutdown (sig)
int sig;
{
	opmode = OP_SHUTDOWN;
}

static SFD spurious (sig)
int sig;
{
	PP_LOG (LLOG_EXCEPTIONS, ("Spurious signal %d", sig));
}
