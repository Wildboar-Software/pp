/* routines to start and stop associations */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/assoc.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/assoc.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: assoc.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <varargs.h>
#include "console.h"
#include "Qmgr-ops.h"
#include "qmgr.h"

static char *myservice = 	"pp qmgr";
extern char *myname;
int				remoteStop();
static void			initiate_responder();
static IFP			startfnx;
static IFP			stopfnx;
int				sd = -1;
static int			tryForAuth;
static struct client_dispatch   *channelread_op,
				*channelcontrol_op,
				*mtaread_op,
				*mtacontrol_op,
				*readchannelmtamessage_op,
				*msgcontrol_op,
				*quecontrol_op,
				*qmgrStatus_op,
				*quit_op;
extern caddr_t retryMask;
extern Authentication  authentication;
extern char	*Qinformation, *Qversion;
extern Widget	qversion;
extern int	compat;

/* CLIENT OPERATIONS */
extern int do_channelread();
extern int do_channelcontrol();
extern int do_mtaread();
extern int do_mtacontrol();
extern int do_readchannelmtamessage();
extern int do_msgcontrol();
extern int do_quecontrol();
int do_quit();

/* CLIENT RESULTS */
extern int channelread_result();
extern int channelcontrol_result();
extern int mtaread_result();
extern int mtacontrol_result();
extern int readchannelmtamessage_result();
extern int msgcontrol_result();
extern int quecontrol_result();
extern int qmgrStatus_result();

/* CLIENT ERRORS */
static int general_error();

#define channelread_error	general_error
#define channelcontrol_error	general_error
#define mtaread_error		general_error
#define mtacontrol_error	general_error
#define readchannelmtamessage_error	general_error
#define msgcontrol_error	general_error
#define quecontrol_error	general_error
#define qmgrStatus_error	general_error

/* CLIENT STRUCTURE */

struct client_dispatch client_dispatches[] = {
{
	"channelread", operation_Qmgr_channelread,
#ifdef PEPSY_VERSION
	do_channelread, &_ZUNIV_mod, _ZUTCTimeUNIV,
#else
	do_channelread, free_UNIV_UTCTime,
#endif
	channelread_result, channelread_error,
	"read information on channels"
},
{
	"channelcontrol", operation_Qmgr_channelcontrol,
#ifdef PEPSY_VERSION
	do_channelcontrol, &_ZQmgr_mod, _ZChannelControlQmgr,
#else
	do_channelcontrol, free_Qmgr_ChannelControl,
#endif
	channelcontrol_result, channelcontrol_error,
	"control channel"
},
{
	"mtaread", operation_Qmgr_mtaread,
#ifdef PEPSY_VERSION
	do_mtaread, &_ZQmgr_mod, _ZMtaReadQmgr,
#else
	do_mtaread, free_Qmgr_MtaRead,
#endif
	mtaread_result, mtaread_error,
	"read information on mtas"
},
{
	"mtacontrol", operation_Qmgr_mtacontrol,
#ifdef PEPSY_VERSION
	do_mtacontrol, &_ZQmgr_mod, _ZMtaControlQmgr,
#else
	do_mtacontrol, free_Qmgr_MtaControl,
#endif
	mtacontrol_result, mtacontrol_error,
	"control mta"
},
{
	"readchannelmtamessage", operation_Qmgr_readChannelMtaMessage,
#ifdef PEPSY_VERSION
	do_readchannelmtamessage, &_ZQmgr_mod, _ZMsgReadQmgr,
#else
	do_readchannelmtamessage, free_Qmgr_MsgRead,
#endif
	readchannelmtamessage_result, readchannelmtamessage_error,
	"read a set of messages"
},
{
	"msgcontrol", operation_Qmgr_msgcontrol,
#ifdef PEPSY_VERSION
	do_msgcontrol, &_ZQmgr_mod, _ZMsgControlQmgr,
#else
	do_msgcontrol, free_Qmgr_MsgControl,
#endif
	msgcontrol_result, msgcontrol_error,
	"control msg"
},
{
	"quecontrol", operation_Qmgr_qmgrControl,
#ifdef PEPSY_VERSION
	do_quecontrol, &_ZQmgr_mod, _ZQMGRControlQmgr,
#else
	do_quecontrol, free_Qmgr_QMGRControl,
#endif
	quecontrol_result, quecontrol_error,
	"control qmgr"
},
{
	"qmgrStatus", operation_Qmgr_qmgrStatus, NULLIFP,
#ifdef PEPSY_VERSION
	&_ZQmgr_mod, _ZQmgrStatusQmgr,
#else
	free_Qmgr_QmgrStatus,
#endif
	qmgrStatus_result, qmgrStatus_error,
	"control qmgr"
},
{
    "quit",     0,      do_quit,
#ifdef PEPSY_VERSION
    NULL, 0, NULLIFP, NULLIFP,
#else
    NULLIFP, NULLIFP, NULLIFP,
#endif
    "terminate the association and exit"
},
{
    NULL
}
};

/* SERVER OPERATIONS */
extern int op_channelread();
extern int op_mtaread();
extern int op_readchannelmtamessage();

/* SERVER STRUCTURE */
static struct server_dispatch server_dispatches[] = {
	"channelread",      operation_Qmgr_channelread,     op_channelread,
	"mtaread",          operation_Qmgr_mtaread,         op_mtaread,
	"readchannelmtamessage", operation_Qmgr_readChannelMtaMessage,	    op_readchannelmtamessage,
	NULL
	};

static void 	acs_advise(),
		ros_advise();
void		advise();
int	remoteStop();
PE	passwdpeps[1], *passwdpep = passwdpeps;
int     fatal;

initiate_assoc(argc, argv)
int	argc;
char	**argv;
{
	struct client_dispatch 		*ix;

	initiate_responder(argc,argv, PLocalHostName(), myservice, 
			   server_dispatches,
			   table_Qmgr_Operations, NULLIFP, remoteStop);
	
	for (ix = client_dispatches; ix->ds_name; ix++){
		if (strcmp(ix -> ds_name, "channelread") == 0)
			channelread_op = ix;
		else if (strcmp(ix->ds_name, "channelcontrol") == 0)
			channelcontrol_op = ix;
		else if (strcmp(ix->ds_name, "quit") == 0)
			quit_op = ix;
		else if (strcmp(ix->ds_name, "mtaread") == 0)
			mtaread_op = ix;
		else if (strcmp(ix->ds_name, "mtacontrol") == 0)
			mtacontrol_op = ix;
		else if (strcmp(ix->ds_name, "readchannelmtamessage") == 0)
			readchannelmtamessage_op = ix;
		else if (strcmp(ix->ds_name, "msgcontrol") == 0)
			msgcontrol_op = ix;
		else if (strcmp(ix->ds_name, "quecontrol") == 0)
			quecontrol_op = ix;
		else if (strcmp(ix->ds_name, "qmgrStatus") == 0)
			qmgrStatus_op = ix;
	}
}

/* ARGSUSED */
static void initiate_responder(argc, argv, host, service, dispatches, ops, start, stop)
int			argc;
char			**argv,
			*host,
			*service;
struct server_dispatch 	*dispatches;
struct RyOperation 	*ops;
IFP			start,
			stop;
{
	register struct server_dispatch	*ds;
	struct RoSAPindication  	rois;
	register struct RoSAPindication	*roi = &rois;
	register struct RoSAPpreject   	*rop = &roi -> roi_preject;
	
	for (ds = dispatches; ds -> ds_name; ds++)
		if (RyDispatch (NOTOK, ops, ds -> ds_operation, ds -> ds_vector, roi)
		    == NOTOK) {
			ros_advise (rop, ds -> ds_name);
			exit(1);
		}

	startfnx = start;
	stopfnx = stop;
}

/*  */
/* connection stuff */

/* ARGSUSED */
int	assoc_start (argc, argv, service)
int			argc;
char			**argv,
        		*service;
/*struct RyOperation 	ops[];
struct client_dispatch	*dispatches;*/
{
	struct SSAPref 			sfs;
	register struct SSAPref 	*sf;
	register struct PSAPaddr 	*pa;
	struct AcSAPconnect 		accs;
	register struct AcSAPconnect   	*acc = &accs;
	struct AcSAPindication  	acis;
	register struct AcSAPindication	*aci = &acis;
	register struct AcSAPabort 	*aca = &aci -> aci_abort;
	AEI	    			aei;
	OID	    			ctx,
					tmppci;
	struct PSAPctxlist	 	pcs;
	register struct PSAPctxlist 	*pc = &pcs;
	struct RoSAPindication 		rois;
	register struct RoSAPindication	*roi = &rois;
	register struct RoSAPpreject 	*rop = &roi -> roi_preject;
	int				result;

	if ((pa = str2paddr (argv[1])) == NULLPA) {
		if ((aei = _str2aei (argv[1], service, QMGR_CTX_OID, 
				     0, dap_user, dap_passwd)) == NULLAEI) {
			fatal = TRUE;
			advise (NULLCP, "%s: unknown entity",
				argv[1]);
			return NOTOK;
		}
		if ((pa = aei2addr (aei)) == NULLPA) {
			fatal = TRUE;
			advise (NULLCP, "%s", "address translation failed");
			return NOTOK;
		}
	}
	if ((ctx = oid_cpy (QMGR_AC)) == NULLOID) {
		fatal = TRUE;
		advise (NULLCP, "%s", "out of memory");
		return NOTOK;
	}

	if ((tmppci = oid_cpy (QMGR_PCI)) == NULLOID) {
		fatal = TRUE;
		advise (NULLCP, "%s", "out of memory"); 
		return NOTOK;
	}
	pc -> pc_nctx = 1;
	pc -> pc_ctx[0].pc_id = 1;
	pc -> pc_ctx[0].pc_asn = tmppci;
	pc -> pc_ctx[0].pc_atn = NULLOID;

	if ((sf = addr2ref (PLocalHostName ())) == NULL) {
		sf = &sfs;
		(void) bzero ((char *) sf, sizeof *sf);
	}

	switch (result = AcAsynAssocRequest (ctx, NULLAEI, NULLAEI, NULLPA,
					     pa, pc, NULLOID, 0, ROS_MYREQUIRE,
					     SERIAL_NONE, 0, sf, passwdpep, 1,
					     NULLQOS, acc, aci, 1)) {
	    case NOTOK:
		acs_advise (aca, "A-ASSOCIATE.REQUEST");
		return NOTOK;
#ifdef CONNECTING_1
	    case CONNECTING_1:
	    case CONNECTING_2:
		sd = acc -> acc_sd;
		ACCFREE (acc);
		PP_TRACE (("Association initiated"));
		return result;
#else
	    case OK:
		sd = acc -> acc_sd;
		ACCFREE (acc);
		PP_TRACE (("Association initiated"));
		return OK;
#endif
	    case DONE:
		if (acc -> acc_result != ACS_ACCEPT) {
			set_failmode (acc);
			ACCFREE (acc);
			return NOTOK;
		}
		sd = acc -> acc_sd;
		
		set_authmode(acc);

		ACCFREE (acc);

		if (RoSetService (sd, RoPService, roi) == NOTOK) {
			ros_advise (rop, "set RO/PS fails");
			return NOTOK;
		}
		PP_TRACE (("Service set"));
		return DONE;
	    default:
		advise (NULLCP, "%s", "Bad response from AcAsynAssocRequest");
		return NOTOK;
	}
}

set_failmode (acc)
struct AcSAPconnect   	*acc;
{
	struct type_Qmgr_BindError	*be;
	char				*info = NULLCP;
	be = NULL;
	
	/* failed by qmgr so no point in retrying connect */
	if (acc -> acc_result == ACS_PERMANENT)
		fatal = TRUE;
	
	if (acc->acc_ninfo >= 1) {
		if (decode_Qmgr_BindError (acc->acc_info[0], 1,
					    NULLVP, NULLIP, &be) == NOTOK) 
			PP_LOG(LLOG_EXCEPTIONS, ("failed to parse connect data [%s]",
						 PY_pepy));
		else {
			if (be -> information != NULL) 
				info = qb2str (be -> information);

			advise (NULLCP,
				"Association rejected [%s] (%s)",
				(be -> reason == int_Qmgr_reason_badCredentials) ? "Bad Credentials" : "Congested",
				(info != NULLCP) ? info : "");
			if (info != NULLCP) free(info);
		}
	} else 
		advise (NULLCP,
			"Association rejected: [%s]",
			AcErrString (acc -> acc_result));
		
}

set_authmode (acc)
struct AcSAPconnect   	*acc;
{
	struct type_Qmgr_BindResult	*br;
	
	br = NULL;
	authentication = limited;
	if (Qversion != NULLCP) {
		free(Qversion);
		Qversion = NULLCP;
	}
	if (Qinformation != NULLCP) {
		free (Qinformation);
		Qinformation = NULLCP;
	}

	if (acc->acc_ninfo >= 1) {
		if (decode_Qmgr_BindResult (acc->acc_info[0], 1,
					    NULLVP, NULLIP, &br) == NOTOK) 
			PP_LOG(LLOG_EXCEPTIONS, ("failed to parse connect data [%s]",
						 PY_pepy));
		else {
			switch (br->result) {
			    case int_Qmgr_result_acceptedFullAccess:
				authentication = full;
				break;
			    case int_Qmgr_result_acceptedLimitedAccess:
			    default:
				authentication = limited;
				break;
			}
			if (br -> information != NULL) 
				Qinformation = qb2str (br -> information);

			if (br -> version != NULL) {
				Qversion = qb2str (br -> version);
				XtVaSetValues (qversion,
					   XtNlabel, Qversion,
					   NULL);
				free(Qversion);
				Qversion = NULLCP;
			}

			free_Qmgr_BindResult(br);
		}
	}
}

int acsap_retry (fd)
int	fd;
{
	struct AcSAPconnect accs;
	register struct AcSAPconnect   *acc = &accs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;
	struct RoSAPindication rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject *rop = &roi -> roi_preject;
	int	result;

	PP_TRACE (("acsap_retry(%d)", fd));
	switch (result = AcAsynRetryRequest (fd, acc, aci)) {
#ifdef CONNECTING_1
	    case CONNECTING_1:
	    case CONNECTING_2:
		ACCFREE (acc);
		return result;
#else
	    case OK:
		ACCFREE (acc);
		return OK;
#endif
	    case NOTOK:
		acs_advise (aca, "A-ASSOCIATE.REQUEST");
		return NOTOK;
	    case DONE:
		if (acc -> acc_result != ACS_ACCEPT) {
			set_failmode(acc);
			ACCFREE (acc);
			return NOTOK;
		}
		sd = acc -> acc_sd;

		set_authmode(acc);

		ACCFREE (acc);
		if (RoSetService (sd, RoPService, roi) == NOTOK) {
			ros_advise (rop, "set RO/PS fails");
			return NOTOK;
		}
		return DONE;
	    default:
		advise (NULLCP, "%s", "Bad response from AcAsynRetryRequest");
		return NOTOK;
	}
}

int  assoc_release (ad)
int     ad;
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

#ifdef	CONNECTING_1
	if (AcRelRequest (ad, ACF_NORMAL, NULLPEP, 0, NOTOK, acr, aci) == NOTOK) {
#else
	if (AcRelRequest (ad, ACF_NORMAL, NULLPEP, 0, acr, aci) == NOTOK) {
#endif
		acs_advise (aca, "A-RELEASE.REQUEST");
		return DONE;
	}

	if (!acr -> acr_affirmative) {
		(void) AcUAbortRequest (ad, NULLPEP, 0, aci);
		advise (NULLCP, "Release rejected by peer: %d",
			acr -> acr_reason);
	}

	ACRFREE (acr);
	PP_TRACE (("Association released"));

	return DONE;
}

/*  */
/* work routines */
extern State	connectState;		
static jmp_buf	toplevel;

static	int invoke (ad, ops, ds, args)
int				ad;
struct RyOperation 		ops[];
register struct client_dispatch	*ds;
char  				**args;
{
    int	    				result;
    caddr_t 				in;
    struct AcSAPindication  acis;
    struct RoSAPindication  rois;
    register struct RoSAPindication	*roi = &rois;
    register struct RoSAPpreject   	*rop = &roi -> roi_preject;

    in = NULL;

    if (ds -> ds_argument && (*ds -> ds_argument) (ad, ds, args, &in) == NOTOK)
	    return 0;

    switch (setjmp (toplevel)) {
	case OK: 
	    break;

	default: 
	    if (stopfnx)
		return (*stopfnx) (ad, (struct AcSAPfinish *) 0);
	    break;
	case DONE:
	    (void) AcUAbortRequest (ad, NULLPEP, 0, &acis);
	    (void) RyLose (ad, roi);
	    return NOTOK;
    }

    if (connectState != connected)
	    return NOTOK;

    switch (result = RyStub (ad, ops, ds -> ds_operation, RyGenID (ad),
			     NULLIP, in, ds -> ds_result, 
			     ds -> ds_error, ROS_SYNC, roi)) {
	case NOTOK:		/* failure */
	    if (ROS_FATAL(rop->rop_reason)) {
		    ros_advise (rop, "STUB");
		    emergency_disconnect(ad);
	    }
	    break;
		
	case OK:		/* got a result/error response */
	    break;
	    
	case DONE:		/* got RO-END? */
	    advise (NULLCP, "%s", "got RO-END.INDICATION");
	    connectState = notconnected;
	    /* NOTREACHED */

	default:
	    advise (NULLCP, "unknown return from RyStub=%d", result);
	    /* NOTREACHED */
    }

#ifdef PEPSY_VERSION
    if (ds -> ds_fr_mod && in)
	    fre_obj (in,  ds -> ds_fr_mod -> md_dtab[ds -> ds_fr_index],
		     ds -> ds_fr_mod);
#else
    if (ds -> ds_free && in)
	    (*ds -> ds_free)(in);
#endif
    return OK;
}

extern Operations	currentop;

my_invoke(op, args)
Operations	op;
char		**args;
{
	currentop = op;
	StartWait();
	switch (op) {
	    case chanread:
		invoke(sd, table_Qmgr_Operations, 
		       channelread_op,(char **) NULL);
	  	update_time_label();
		break;
	    case chanstop:
	    case chanstart:
	    case chanclear:
	    case chancacheadd:
		invoke(sd, table_Qmgr_Operations,
		       channelcontrol_op, args);
		break;
	    case mtaread:
		invoke(sd, table_Qmgr_Operations,
		       mtaread_op, args);
		break;
	    case mtastop:
	    case mtastart:
	    case mtaclear:
	    case mtacacheadd:
		invoke(sd, table_Qmgr_Operations,
		       mtacontrol_op, args);
		break;
	    case readchannelmtamessage:
		invoke(sd, table_Qmgr_Operations,
		       readchannelmtamessage_op, args);
		break;
	    case msgstop:
	    case msgstart:
	    case msgclear:
	    case msgcacheadd:
		invoke(sd, table_Qmgr_Operations,
		       msgcontrol_op, args);
		break;
	    case quit:
		invoke(sd, table_Qmgr_Operations,
		       quit_op, (char **) NULL);
		break;
	    case connect:
		EndWait();
		return do_connect(args);
	    case disconnect:
		do_disconnect();
		break;
	    case quecontrol:
		invoke(sd, table_Qmgr_Operations,
		       quecontrol_op, args);
		break;
	    case qmgrStatus:
		invoke(sd, table_Qmgr_Operations,
		       qmgrStatus_op, args);
		EndWait();
		return 0;
	    default:
		break;
	}
	InitRefreshTimeOut((unsigned long) 0);
	EndWait();
	return 0;
}

/*  */
/* advising routines */

static void    acs_advise (aca, event)
register struct AcSAPabort *aca;
char   *event;
{
	char    buffer[BUFSIZ];

	if (aca -> aca_cc > 0)
		(void) sprintf (buffer, "[%s] %*.*s",
				AcErrString (aca -> aca_reason),
				aca -> aca_cc, aca -> aca_cc, aca -> aca_data);
	else
		(void) sprintf (buffer, "[%s]",
				AcErrString (aca -> aca_reason));

	advise (NULLCP, "%s: %s (source %d)", event, buffer,
		aca -> aca_source);
}

int  ros_work (fd)
int     fd;
{
    int     result;
    caddr_t out;
    struct AcSAPindication  acis;
    struct RoSAPindication  rois;
    register struct RoSAPindication *roi = &rois;
    register struct RoSAPpreject   *rop = &roi -> roi_preject;

    switch (setjmp (toplevel)) {
	case OK: 
	    break;

	default: 
	    if (stopfnx)
		return (*stopfnx) (fd, (struct AcSAPfinish *) 0);
	    break;
	case DONE:
	    (void) AcUAbortRequest (fd, NULLPEP, 0, &acis);
	    (void) RyLose (fd, roi);
	    return NOTOK;
    }

    switch (result = RyWait (fd, NULLIP, &out, OK, roi)) {
	case NOTOK: 
	    if (rop -> rop_reason == ROS_TIMER)
		break;
	case OK: 
	case DONE: 
	    ros_indication (fd, roi);
	    break;

	default: 
	    advise (NULLCP, "unknown return from RoWaitRequest=%d", result);
    }
    return OK;
}

ros_indication (ad, roi)
int     ad;
register struct RoSAPindication *roi;
{
	int     reply,
	result;

	switch (roi -> roi_type) {
	    case ROI_INVOKE: 
	    case ROI_RESULT: 
	    case ROI_ERROR: 
		advise (NULLCP, "unexpected indication type=%d",
		       roi -> roi_type);
		break;

	    case ROI_UREJECT: 
		{
			register struct RoSAPureject   *rou =
				&roi -> roi_ureject;

			if (rou -> rou_noid)
				advise (NULLCP,
					"RO-REJECT-U.INDICATION/%d: %s",
					ad, RoErrString (rou -> rou_reason));
			else
				advise (NULLCP,
					"RO-REJECT-U.INDICATION/%d: %s (id=%d)",
					ad, RoErrString (rou -> rou_reason),
					rou -> rou_id);
		}
		break;

	    case ROI_PREJECT: 
		{
			register struct RoSAPpreject   *rop = &roi -> roi_preject;
			connectState = notconnected;
			if (ROS_FATAL (rop -> rop_reason)) {
				ros_adios (rop, "RO-REJECT-P.INDICATION");
				emergency_disconnect(ad);
			}
			ros_advise (rop, "RO-REJECT-P.INDICATION");
		}
		break;

	    case ROI_FINISH: 
		{
			register struct AcSAPfinish *acf = &roi -> roi_finish;
			struct AcSAPindication  acis;
			register struct AcSAPabort *aca = &acis.aci_abort;

			advise (NULLCP, "A-RELEASE.INDICATION/%d: %d",
				ad, acf -> acf_reason);

			reply = stopfnx ? (*stopfnx) (ad, acf) : ACS_ACCEPT;

			result = AcRelResponse (ad, reply, ACR_NORMAL, NULLPEP, 0,
						&acis);

			ACFFREE (acf);

			if (result == NOTOK)
				acs_advise (aca, "A-RELEASE.RESPONSE");
			else
				if (reply != ACS_ACCEPT)
					break;
			longjmp (toplevel, DONE);
		}
		/* NOTREACHED */

	    default: 
		advise (NULLCP, "unknown indication type=%d", roi -> roi_type);
	}
}

ros_adios (rop, event)
register struct RoSAPpreject *rop;
char   *event;
{
	ros_advise (rop, event);

	longjmp (toplevel, NOTOK);
}

static void    ros_advise (rop, event)
register struct RoSAPpreject *rop;
char   *event;
{
	char    buffer[BUFSIZ];

	if (rop -> rop_cc > 0)
		(void) sprintf (buffer, "[%s] %*.*s",
				RoErrString (rop -> rop_reason),
				rop -> rop_cc, rop -> rop_cc, rop -> rop_data);
	else
		(void) sprintf (buffer, "[%s]",
				RoErrString (rop -> rop_reason));

	advise (NULLCP, "%s: %s", event, buffer);
}

extern Widget	header,
		error,
		refresh_command;
extern int	autoReconnect,
		userConnected,
		errorUp;
extern char	*hostname;

#ifndef lint
void advise (va_alist)
va_dcl
{
	va_list ap;
	
	char	buffer[BUFSIZ],
		headerbuf[BUFSIZ];

	va_start (ap);
	asprintf (buffer, ap);
	if (header != NULL) {
		XtSetMappedWhenManaged(error, True);
		errorUp = 5;
	}
	if (header == NULL)
		printf("%s\n",buffer);
	else if (connectState == notconnected && 
		 autoReconnect == TRUE
		 && fatal == FALSE) {
		sprintf(headerbuf,"Attempting to reconnect to %s",
			hostname);
		XtVaSetValues(header,
			  XtNlabel, headerbuf,
			  NULL);
		XtVaSetValues(error,
			  XtNlabel, buffer,
			  NULL);
	} else {
		XtVaSetValues(error,
			  XtNlabel, buffer,
			  NULL);
	}
	va_end (ap);
}
#else
/* VARARGS2 */

void advise(what, fmt)
char	*what,
	*fmt;
{
	advise(what,fmt);
}
#endif

/*  */
/* error routine */

/* ARGSUSED */
static general_error (ad, id, err, parameter, roi)
int     ad,
	id,
	err;
caddr_t parameter;
struct RoSAPindication *roi;
{
	register struct RyError *rye;

	if (err == RY_REJECT) {
		advise (NULLCP, "%s", RoErrString ((int) parameter));
		return OK;
	}

	if ((rye = finderrbyerr (table_Qmgr_Errors, err)) != NULL)
		advise (NULLCP, "Error: %s", rye -> rye_name);
	else
		advise (NULLCP, "Error: %d", err);

	return OK;
}

/*  */

/* ARGSUSED */
int remoteStop(fd, dummy)
int	fd;
struct AcSAPfinish	*dummy;
{
	struct AcSAPindication  acis;
	struct RoSAPindication  rois;
	register struct RoSAPindication *roi = &rois;

	/* x release */
	connectState = notconnected;
	
	SensitizeButtons(False);
	TermRefreshTimeOut();
	TermConnectRetry();
	TermListen();

	if (autoReconnect == FALSE)
		userConnected = FALSE;
	/* isode release */
	(void) AcUAbortRequest (fd, NULLPEP, 0, &acis);
	(void) RyLose (fd, roi);
	
	if (autoReconnect == TRUE && userConnected == TRUE) {
		clear_displays();
		XtVaSetValues(refresh_command,
			  XtNlabel, "Reconnect",
			  NULL);
		InitConnectTimeOut();
		MapVolume(False);
	} else {
		XtVaSetValues(header,
			  XtNlabel, NO_CONNECTION,
			  NULL);
		ResetForDisconnect(); 
	}
	return NOTOK;
}

/* ARGSUSED */
emergency_disconnect(ad)
int	ad;
{
	char	headerbuf[BUFSIZ];

	connectState = notconnected;
	SensitizeButtons(False);
	TermRefreshTimeOut();
	TermConnectRetry();
	TermListen();

	if (autoReconnect == FALSE)
		userConnected = FALSE;
	if (autoReconnect == TRUE && userConnected == TRUE) {
		clear_displays();
		XtVaSetValues(refresh_command,
			  XtNlabel, "Reconnect",
			  NULL);
		sprintf(headerbuf,"Attempting to reconnect to %s",
			hostname);
		XtVaSetValues(header,
			  XtNlabel, headerbuf,
			  NULL);
		InitConnectTimeOut();
		MapVolume(False);
	} else {
		XtVaSetValues(header,
			  XtNlabel, NO_CONNECTION,
			  NULL);
		ResetForDisconnect(); 
	}

}

/*  */


/* disconnect from current host with part of a quit */
do_disconnect()
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

	if (connectState == connected) {
		/* Disconnect(sd) */
		TermListen();
		connectState = notconnected;

#ifdef	CONNECTING_1
		if (AcRelRequest (sd, ACF_NORMAL, NULLPEP, 0, NOTOK, acr, aci) == NOTOK) {
#else
		if (AcRelRequest (sd, ACF_NORMAL, NULLPEP, 0, acr, aci) == NOTOK) {
#endif
			acs_advise (aca, "A-RELEASE.REQUEST");
			return;
		}

		if (!acr -> acr_affirmative) {
			(void) AcUAbortRequest (sd, NULLPEP, 0, aci);
			advise (NULLCP, "Release rejected by peer: %d", acr -> acr_reason);
		}

		ACRFREE (acr);
		sd = -1;
	} else if (autoReconnect == TRUE) {
		TermConnectTimeOut();
		XtVaSetValues(header,
			  XtNlabel, NO_CONNECTION,
			  NULL);
		autoReconnect = FALSE;
	}
}

/* ARGSUSED */
int  do_quit (ad, ds, args, arg)
int     ad;
struct client_dispatch *ds;
char  	**args,
	**arg;
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

	if (ad >= 0) {
#ifdef	CONNECTING_1
		if (AcRelRequest (ad, ACF_NORMAL, NULLPEP, 0, NOTOK, acr, aci) == NOTOK)
#else
		if (AcRelRequest (ad, ACF_NORMAL, NULLPEP, 0, acr, aci) == NOTOK)
#endif
			acs_advise (aca, "A-RELEASE.REQUEST");

		if (!acr -> acr_affirmative) {
			(void) AcUAbortRequest (ad, NULLPEP, 0, aci);
			advise (NULLCP, "Release rejected by peer: %d", acr -> acr_reason);
		}
		ad = -1;
	}
	terminate_display();
	exit (0);

}

/*  */

do_connect(phost)
char	**phost;
{
	char	*targv[2];
	int	result;

	targv[0] = myname;
	targv[1] = *phost;
	fatal = FALSE;
	switch(result = assoc_start(2, targv, myservice)) {
#ifdef	CONNECTING_1
	    case CONNECTING_1:
	    case CONNECTING_2:
		Connecting(result);
		break;
#else
	    case OK:
		retryMask = (caddr_t) XtInputWriteMask;
		Connecting(result);
		break;
#endif
	    case DONE:
		Connected();
		break;
	    case NOTOK:
	    default:
		NotConnected();
		break;
	}
	return result;
}

char	buf[BUFSIZ];
extern Widget	header,
		error,
		qcontrol_command,
		connect_command,
		refresh_command,
		statistics;
extern Mode	mode;

Connected()
{
	sprintf(buf, "Connected to %s (%s)",
		(Qinformation != NULLCP && *Qinformation != '\0') ? Qinformation : hostname,
		(authentication == full) ? "full authorisation" : "limited authorisation");
	XtVaSetValues(header,
		  XtNlabel, buf,
		  NULL);
	display_time_label();
/*	if (authentication != full && tryForAuth == TRUE)
		advise(NULLCP, "%s",
		       "Failed to gain authorised connection");
	else
		XtSetMappedWhenManaged(error, False); */
	MapButtons(authentication == full);
	if (full == authentication 
	    && monitor == mode)
		XtSetMappedWhenManaged (qcontrol_command, False);
	SensitizeButtons(True);
	XtVaSetValues(connect_command,
		  XtNlabel, "disconnect",
		  NULL);
	XtVaSetValues(refresh_command,
		  XtNlabel, "refresh",
		  NULL);
	XtSetMappedWhenManaged(error, False);
	errorUp = 0;
	connectState = connected;
	TaiInit();
	InitListen(sd);
	if (!compat)
		my_invoke(qmgrStatus, (char **) NULL);
	InitRefreshTimeOut((unsigned long) 500);
	TermConnectTimeOut();
	userConnected = TRUE;
}

extern Widget	time_label;

NotConnected()
{
	char	headerbuf[BUFSIZ];
	connectState = notconnected;	
	undisplay_time_label();
	TermRefreshTimeOut();
	MapButtons(False);
	MapVolume(False);
	if (autoReconnect == TRUE 
	    && fatal == FALSE) {
		XtVaSetValues(refresh_command,
			  XtNlabel, "reconnect",
			  NULL);
		XtVaSetValues(connect_command,
			  XtNlabel, "disconnect",
			  NULL);
		sprintf(headerbuf, "Attempting to reconnect to %s",hostname);
		XtVaSetValues(header,
			  XtNlabel, headerbuf,
			  NULL);
		InitConnectTimeOut();
	} else {
		XtVaSetValues(header,
			  XtNlabel, NO_CONNECTION,
			  NULL);
		userConnected = FALSE;
		ResetForDisconnect();
	}
	
}

Connecting(res)
int	res;
{
	connectState = connecting;
	sprintf(buf,"Connecting to %s",hostname);
	XtVaSetValues(header,
		  XtNlabel, buf,
		  NULL);
	XtVaSetValues(connect_command,
		  XtNlabel, "disconnect",
		  NULL);
	InitConnectRetry(sd, res);
}

struct type_Qmgr_BindArgument	*ba;

fillin_passwdpep(user, passwd, auth)
char	*user,
	*passwd;
int	auth;
{
	
	if (*passwdpep != NULL) {
		pe_free(*passwdpep);
		free_Qmgr_BindArgument(ba);
		*passwdpep = NULL;
	}

	ba = (struct type_Qmgr_BindArgument *) smalloc (sizeof *ba);
	
	if (auth != TRUE) {
		/* no authentication */
		ba -> offset = type_Qmgr_BindArgument_noAuthentication;
		tryForAuth = FALSE;
	} else {
		ba -> offset = type_Qmgr_BindArgument_weakAuthentication;
		ba -> un.weakAuthentication = 
			(struct type_Qmgr_WeakAuthentication *) 
				smalloc(sizeof(struct type_Qmgr_WeakAuthentication));
		ba -> un.weakAuthentication->username = str2qb(user, strlen(user), 1);
		if (passwd != NULLCP && *passwd != '\0')
			ba -> un.weakAuthentication->passwd = str2qb(passwd, strlen(passwd), 1);
		else 
			ba -> un.weakAuthentication->passwd = NULL;
		tryForAuth = TRUE;
	}
		
	if (encode_Qmgr_BindArgument(passwdpep, 1, NULLCP, 0, ba) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("failed to encode BindArgument [%s]", PY_pepy));
		*passwdpep = NULLPE;
		free_Qmgr_BindArgument(ba);
		exit(1);
	}
	(*passwdpep)->pe_context = 3;
}
