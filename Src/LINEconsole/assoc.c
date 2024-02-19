/* assoc.c: routines to manage association with qmgr */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/assoc.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/assoc.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: assoc.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */

#include	"Qmgr-ops.h"
#include	"Qmgr-types.h"
#include	"console.h"

extern char	*host, *actual_host, *cmd_argv[];
extern int	assocD, cmd_argc;
extern Command comm;

int	connected = FALSE;
extern int	authorised;


static IFP			stopfnx = NULLIFP;
static jmp_buf			toplevel;

static char	*service = "pp qmgr";
static struct client_dispatch   *channelread_op,
				*channelcontrol_op,
				*mtaread_op,
				*mtacontrol_op,
				*readchannelmtamessage_op,
				*msgcontrol_op,
				*quecontrol_op,
				*qmgrStatus_op,
				*quit_op;
static void	acs_advise(), ros_advise();
void		advise(char *, char *, ...);
int	remoteStop();
PE	passwdpeps[1], *passwdpep = passwdpeps;
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

init_assoc()
{
	struct client_dispatch 		*ix;
	
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
	stopfnx = remoteStop;
}

ConnectProc ()
{
	char	*args;
	args = strdup(host);
	do_connect(&args);
	free(args);
}

do_connect(args)
char	**args;
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

	if (host) free(host);
	host = strdup(*args);
	if (actual_host) free(actual_host);
	actual_host = strdup(host);
	if ((aei = _str2aei (host, service, QMGR_CTX_OID, 
			     0, dap_user, dap_passwd)) == NULLAEI) {
		advise (NULLCP, "%s: unknown entity",
		       host);
		return NOTOK;
	}
	if ((pa = aei2addr (aei)) == NULLPA) {
		advise (NULLCP, "%s", "address translation failed");
		return NOTOK;
	}

	if ((ctx = oid_cpy (QMGR_AC)) == NULLOID) {
		advise (NULLCP, "%s", "out of memory");
		return NOTOK;
	}

	if ((tmppci = oid_cpy (QMGR_PCI)) == NULLOID) {
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

	fprintf(stdout, "Please wait while attempt to connect to %s...", host);
	fflush(stdout);

	if (AcAssocRequest (ctx, NULLAEI, aei, NULLPA,
			    pa, pc, NULLOID, 0, ROS_MYREQUIRE,
			    SERIAL_NONE, 0, sf, passwdpep, *passwdpep ? 1 : 0,
			    NULLQOS, acc, aci) == NOTOK) {
		acs_advise (aca, "A-ASSOCIATE.REQUEST");
		return NOTOK;
	}
	if (acc -> acc_result != ACS_ACCEPT) {
		advise (NULLCP, "Association rejected: [%s]",
			AcErrString (acc->acc_result));
		return NOTOK;
	}

	assocD = acc->acc_sd;
	
	set_authmode(acc);

	ACCFREE (acc);

	if (RoSetService (assocD, RoPService, roi) == NOTOK) {
		ros_advise (rop, "set RO/PS fails");
		return NOTOK;
	}
	fprintf(stdout, "connected\n");
	fflush(stdout);
	connected = TRUE;
	TaiInit();
	return OK;
}

DisconnectProc()
{
	my_invoke(disconnect, &host);
}

QuitProc()
{
	if (connected == TRUE)
		DisconnectProc();
}

static void resetForDisconnect()
{
	connected = FALSE;
	clear_msg_level();
	clear_mta_level();
	clear_channel_level();
}

extern time_t	currentTime;

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
    time(&currentTime);

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

    if (connected != TRUE)
	    return NOTOK;

    switch (result = RyStub (ad, ops, ds -> ds_operation, RyGenID (ad),
			     NULLIP, in, ds -> ds_result, 
			     ds -> ds_error, ROS_SYNC, roi)) {
	case NOTOK:		/* failure */
	    if (ROS_FATAL(rop->rop_reason)) {
		    ros_advise (rop, "STUB");
		    fprintf(stderr,
			    "Disconnected from '%s'\n", host);
		    comm = Disconnect;
		    resetForDisconnect();
	    }
	    break;
		
	case OK:		/* got a result/error response */
	    break;
	    
	case DONE:		/* got RO-END? */
	    advise (NULLCP, "%s", "got RO-END.INDICATION");
	    connected = FALSE;
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

Operations	currentop;

my_invoke(op, args)
Operations	op;
char		**args;
{
	caddr_t	in;
	currentop = op;
	switch (op) {
	    case chanread:
		invoke(assocD, table_Qmgr_Operations, 
		       channelread_op,(char **) NULL);
		break;
	    case chanstop:
	    case chanstart:
	    case chanclear:
	    case chancacheadd:
		invoke(assocD, table_Qmgr_Operations,
		       channelcontrol_op, args);
		break;
	    case mtaread:
		invoke(assocD, table_Qmgr_Operations,
		       mtaread_op, args);
		break;
	    case mtastop:
	    case mtastart:
	    case mtaclear:
	    case mtacacheadd:
		invoke(assocD, table_Qmgr_Operations,
		       mtacontrol_op, args);
		break;
	    case readchannelmtamessage:
		invoke(assocD, table_Qmgr_Operations,
		       readchannelmtamessage_op, args);
		break;
	    case msgstop:
	    case msgstart:
	    case msgclear:
	    case msgcacheadd:
		invoke(assocD, table_Qmgr_Operations,
		       msgcontrol_op, args);
		break;
	    case quit:
		invoke(assocD, table_Qmgr_Operations,
		       quit_op, (char **) NULL);
		break;
	    case connect:
		return do_connect(args);
	    case disconnect:
		if (connected == TRUE)
			do_quit(assocD, quit_op, args, &in);
		break;
	    case quecontrol:
		invoke(assocD, table_Qmgr_Operations,
		       quecontrol_op, args);
		break;
	    case qmgrStatus:
		invoke(assocD, table_Qmgr_Operations,
		       qmgrStatus_op, args);
		return 0;
	    default:
		break;
	}
	return 0;
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
	
	resetForDisconnect();
}

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

	if ((rye = finderrbyerr (table_Qmgr_Errors, err)) != 
	    (struct RyError *) NULL)
		advise (NULLCP, "Error: %s", rye -> rye_name);
	else
		advise (NULLCP, "Error: %d", err);

	return OK;
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

#ifndef lint
void advise (char *what, char *fmt, ...)
{
	va_list ap;
	
	char	buffer[BUFSIZ];

	va_start (ap, fmt);
	_asprintf (buffer, what, fmt, ap);
	fprintf(stdout, "%s\n",buffer);
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

/* ARGSUSED */
int remoteStop(fd, dummy)
int	fd;
struct AcSAPfinish	*dummy;
{
	struct AcSAPindication  acis;
	struct RoSAPindication  rois;
	register struct RoSAPindication *roi = &rois;

	connected = FALSE;

	/* isode release */
	(void) AcUAbortRequest (fd, NULLPEP, 0, &acis);
	(void) RyLose (fd, roi);
	
	comm = Disconnect;
	return NOTOK;
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
