/* comm.c: ROS communication routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/comms.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/comms.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: comms.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "qmgr-int.h"
#include "Qmgr-types.h"
#include "Qmgr-ops.h"
#include "qmgr.h"
#include <varargs.h>
#include "consblk.h"

#define PP_SERVICE	"pp qmgr"

static void	advise(), ros_advise(), acs_advise();

static int	general_error();

typedef struct client_dispatch {
    char			*ds_name;
    int				ds_operation;
    IFP				ds_argument;
    modtyp			*ds_fr_mod;
    int				ds_fr_index;
    IFP				ds_result;
    IFP				ds_error;
    char			*ds_help;
} Client_dispatch;

/* CLIENT STRUCTURE */

extern int	arg_channelread(), res_channelread();
#define channelread_error	general_error

extern int	arg_channelcontrol(), res_channelcontrol();
#define channelcontrol_error	general_error

extern int	arg_mtaread(), res_mtaread();
#define mtaread_error		general_error

extern int	arg_mtacontrol(), res_mtacontrol();
#define mtacontrol_error	general_error

extern int	arg_readchannelmtamessage(), res_readchannelmtamessage();
#define readchannelmtamessage_error	general_error

extern int	arg_msgcontrol(), res_msgcontrol();
#define msgcontrol_error		general_error

extern int	arg_quecontrol(), res_quecontrol();
#define quecontrol_error	general_error

extern int	res_qmgrStatus();
#define qmgrStatus_error	general_error

extern int	arg_filter ();
#define filter_error		general_error

static int	arg_quit();

struct client_dispatch client_dispatches[] = {
{
	"channelread", operation_Qmgr_channelread,
	arg_channelread, &_ZUNIV_mod, _ZUTCTimeUNIV,
	res_channelread, channelread_error,
	"read information on channels"
},
{
	"channelcontrol", operation_Qmgr_channelcontrol,
	arg_channelcontrol, &_ZQmgr_mod, _ZChannelControlQmgr,
	res_channelcontrol, channelcontrol_error,
	"control channel"
},
{
	"mtaread", operation_Qmgr_mtaread,
	arg_mtaread, &_ZQmgr_mod, _ZMtaReadQmgr,
	res_mtaread, mtaread_error,
	"read information on mtas"
},
{
	"mtacontrol", operation_Qmgr_mtacontrol,
	arg_mtacontrol, &_ZQmgr_mod, _ZMtaControlQmgr,
	res_mtacontrol, mtacontrol_error,
	"control mta"
},
{
	"readchannelmtamessage", operation_Qmgr_readChannelMtaMessage,
	arg_readchannelmtamessage, &_ZQmgr_mod, _ZMsgReadQmgr,
	res_readchannelmtamessage, readchannelmtamessage_error,
	"read a set of messages"
},
{
	"msgcontrol", operation_Qmgr_msgcontrol,
	arg_msgcontrol, &_ZQmgr_mod, _ZMsgControlQmgr,
	res_msgcontrol, msgcontrol_error,
	"control msg"
},
{
	"quecontrol", operation_Qmgr_qmgrControl,
	arg_quecontrol, &_ZQmgr_mod, _ZQMGRControlQmgr,
	res_quecontrol, quecontrol_error,
	"control qmgr"
},
{
	"qmgrStatus", operation_Qmgr_qmgrStatus, NULLIFP,
	&_ZQmgr_mod, _ZQmgrStatusQmgr,
	res_qmgrStatus, qmgrStatus_error,
	"control qmgr"
},
{
	"readmsginfo", operation_Qmgr_readmsginfo, arg_filter,
	&_ZQmgr_mod, _ZReadMessageArgumentQmgr,
	res_readchannelmtamessage, filter_error,
	"read messages"
},
{
    "quit",     0,      arg_quit,
    NULL, 0, NULLIFP, NULLIFP,
    "terminate the association and exit"
},
{
    NULL
}
};

static struct client_dispatch *get_dispatch(id)
{
	struct client_dispatch *ix;

	for (ix = client_dispatches;
	     NULL != ix && NULLCP != ix->ds_name;
	     ix++)
		if (id == ix -> ds_operation)
			return ix;
	return (struct client_dispatch *) NULL;
}


/*  */

int	initiate_op (ad, op_id, args, pid, fnx, errbuf)
int	ad;
int	op_id;
char	**args;
int	*pid;
IFP	fnx;
char	errbuf[];
{
	int	    				result, id;
	caddr_t 				in = NULL;
	struct RoSAPindication  rois;
	struct client_dispatch	*ds;
	register struct RoSAPindication	*roi = &rois;
	register struct RoSAPpreject   	*rop = &roi -> roi_preject;

	in = NULL;

	if ((ds = get_dispatch(op_id)) == NULL) {
		advise(errbuf, "Unknown dispatch number [%d]",
		       op_id);
		return NOTOK;
	}

	if (ds -> ds_argument
	    && (*ds -> ds_argument) (ad, args, &in) == NOTOK)
		return NOTOK;

	id = RyGenID (ad);
	if (pid) *pid = id;
	switch (result = RyStub (ad, 
				 table_Qmgr_Operations, 
				 ds -> ds_operation, 
				 id,
				 NULLIP, 
				 in, 
				 ds -> ds_result, 
				 ds -> ds_error, 
				 ROS_ASYNC, 
				 roi)) {
	    case NOTOK:		/* failure */
		if (ROS_FATAL(rop->rop_reason)) {
			ros_advise (errbuf, rop, "STUB");
			result = DONE;
		}
		break;
		
	    case OK:		/* should get a result/error response */
		(void) newcopblk (ad, id, fnx);
		break;
	    
	    case DONE:		/* got RO-END? */
		advise (errbuf, "%s", "got RO-END.INDICATION");
		break;
	    default:
		advise (errbuf, "unknown return from RyStub=%d", result);
		/* NOTREACHED */
		result = NOTOK;
		break;
	}

	if (ds -> ds_fr_mod && in)
		(void) fre_obj (in,
				ds -> ds_fr_mod -> md_dtab[ds -> ds_fr_index],
				ds -> ds_fr_mod, 1);
	return result;
}

/*  */

int result_op (ad, pid, errbuf)
int	ad;
int	*pid;
char	errbuf[];
{
	caddr_t	out;
	struct RoSAPindication	rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject   *rop = &roi -> roi_preject;

	errbuf[0] = '\0';

	switch (RyWait (ad, pid, &out, NOTOK, roi)) {
	    case NOTOK:
		if (rop -> rop_reason == ROS_TIMER)
			break;
		PP_TRACE (("RyWait returned NOTOK"));
		ros_advise (errbuf, rop, "STUB");
		if (ROS_FATAL (rop -> rop_reason))
			return DONE;
		break;

	    case OK:
		PP_TRACE (("RyWait returned OK"));
		break;

	    case DONE:
		return DONE;
	}

	return OK;
}
	
/*  */

static PE	fillin_passwd(user, passwd, auth, errbuf)
char	*user;
char	*passwd;
int	auth;
char	errbuf[];
{
	PE	ret;
	struct type_Qmgr_BindArgument	*ba;

	ba = (struct type_Qmgr_BindArgument *)
		smalloc(sizeof *ba);
	switch ((ba->offset = auth)) {
	    case type_Qmgr_BindArgument_noAuthentication:
	    default:
		break;
	    case type_Qmgr_BindArgument_weakAuthentication:
		ba->un.weakAuthentication =
			(struct type_Qmgr_WeakAuthentication *)
				smalloc(sizeof(struct type_Qmgr_WeakAuthentication));
		ba->un.weakAuthentication->username = str2qb(user,
							     strlen(user),
							     1);
		if (NULLCP != passwd  && '\0' != *passwd)
			ba->un.weakAuthentication->passwd = str2qb(passwd,
								   strlen(passwd),
								   1);
		else
			ba->un.weakAuthentication->passwd = NULL;
		break;

	}

	if (encode_Qmgr_BindArgument (&ret, 1, NULLCP, 0, ba) == NOTOK) {
		advise(errbuf,
		       "failed to encode BindArgument [%s]", PY_pepy);
		free_Qmgr_BindArgument(ba);
		return NULLPE;
	}
	free_Qmgr_BindArgument(ba);
	ret->pe_context = 3;
	return ret;
}

static BindResult	*convert_BindResult(acc, errbuf)
struct AcSAPconnect	*acc;
char	*errbuf;
{
	struct type_Qmgr_BindResult	*br;

	if (acc->acc_ninfo >= 1) {
		if (decode_Qmgr_BindResult (acc->acc_info[0],
					    1, NULLIP, NULLVP, &br) == NOTOK) {
			advise (errbuf,
				"failed to parse BindResult [%s]",
				PY_pepy);
			return (BindResult *) NOTOK;
		} else {
			BindResult	*ret = (BindResult *) 
				calloc(1, sizeof(*ret));
			switch (br->result) {
			    case int_Qmgr_result_acceptedFullAccess:
				ret->access = FULL_ACCESS;
				break;
			    case int_Qmgr_result_acceptedLimitedAccess:
			    default:
				ret->access = LIMITED_ACCESS;
				break;
			}
			
			if (NULL != br->information)
				ret->info = qb2str(br->information);
			if (NULL != br->version)
				ret->version = qb2str(br->version);

			free_Qmgr_BindResult(br);
			return ret;
		}
	}
	return NULL;
}

void free_BindResult (br)
BindResult	*br;
{
	if (br -> info)
		free(br -> info);
	if (br -> version)
		free(br->version);
	free((char *) br);
}

int init_connect (host, user, passwd, auth, pid, errbuf, bindRes)
char	*host, *user, *passwd;
int	auth;
int	*pid;
char	errbuf[];
BindResult	**bindRes;
{
	struct SSAPref 			sfs;
	register struct SSAPref 	*sf;
	register struct PSAPaddr 	*pa;
	AEI	    			aei;
	OID	    			ctx,
					tmppci;
	struct PSAPctxlist	 	pcs;
	register struct PSAPctxlist 	*pc = &pcs;
	struct RoSAPindication 		rois;
	register struct RoSAPindication	*roi = &rois;
	register struct RoSAPpreject 	*rop = &roi -> roi_preject;
	struct AcSAPconnect 		accs;
	register struct AcSAPconnect   	*acc = &accs;
	struct AcSAPindication  	acis;
	register struct AcSAPindication	*aci = &acis;
	register struct AcSAPabort 	*aca = &aci -> aci_abort;
	int				result;
	PE				passwdpep;

	if ((pa = str2paddr (host)) == NULLPA) {
		if ((aei = _str2aei (host, PP_SERVICE, QMGR_CTX_OID, 
				     0, dap_user, dap_passwd)) == NULLAEI) {
			advise (errbuf, "%s: unknown entity",
				host);
			return NOTOK;
		}
		if ((pa = aei2addr (aei)) == NULLPA) {
			advise (errbuf, "%s", "address translation failed");
			return NOTOK;
		}
	}
	if ((ctx = oid_cpy (QMGR_AC)) == NULLOID) {
		advise (errbuf, "%s", "out of memory");
		return NOTOK;
	}

	if ((tmppci = oid_cpy (QMGR_PCI)) == NULLOID) {
		advise (errbuf, "%s", "out of memory"); 
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

	if ((passwdpep = fillin_passwd(user, passwd, auth, errbuf))
	    == NULLPE)
		return NOTOK;

	result = AcAsynAssocRequest (ctx, NULLAEI, NULLAEI, NULLPA,
					     pa, pc, NULLOID, 0,
					     ROS_MYREQUIRE,
					     SERIAL_NONE, 0, sf, 
					     &passwdpep, 1,
					     NULLQOS, acc, aci, 1);
	pe_free(passwdpep);

	switch(result) {
	    case NOTOK:
		acs_advise (errbuf, aca, "A-ASSOCIATE.REQUEST");
		return NOTOK;
	    case CONNECTING_1:
	    case CONNECTING_2:
		*pid = acc->acc_sd;
		ACCFREE(acc);
		PP_TRACE (("Association initiated"));
		return result;
	    case DONE:
		if (acc -> acc_result != ACS_ACCEPT) {
			advise(errbuf,
			       "Association rejected: [%s]",
			       AcErrString (acc -> acc_result));
			ACCFREE(acc)
			return NOTOK;
		}
		PP_TRACE (("Association established"));
		if (RoSetService (acc->acc_sd, RoPService, roi) == NOTOK) {
			ros_advise (errbuf, rop, "set RO/PS fails");
			ACCFREE(acc);
			return NOTOK;
		}
		PP_TRACE (("Service set"));
		*pid = acc->acc_sd;
		*bindRes = convert_BindResult(acc, errbuf);
		ACCFREE(acc);
		return DONE;
	    default:
		advise(errbuf,
		       "Bad response from AcAsynAssocRequest [%d]",
		       result);
		return NOTOK;
	}
}

/*  */

int retry_connect (id, errbuf, bindRes)
int	id;
char	errbuf[];
BindResult	**bindRes;
{
	struct AcSAPconnect 		accs;
	register struct AcSAPconnect   	*acc = &accs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;
	struct RoSAPindication rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject *rop = &roi -> roi_preject;
	int	result;

	PP_TRACE (("acsap_retry(%d)", id));
	switch (result = AcAsynRetryRequest (id, acc, aci)) {
	    case CONNECTING_1:
	    case CONNECTING_2:
		ACCFREE(acc);
		return result;
	    case NOTOK:
		acs_advise (errbuf, aca, "A-ASSOCIATE.REQUEST");
		ACCFREE(acc);
		return NOTOK;
	    case DONE:
		if (acc -> acc_result != ACS_ACCEPT) {
			advise(errbuf,
			       "Association Rejected: [%s]",
			       AcErrString (acc -> acc_result));
			ACCFREE(acc);
			return NOTOK;
		}
		if (RoSetService (acc->acc_sd, RoPService, roi) == NOTOK) {
			ros_advise (errbuf, rop, "set RO/PS fails");
			ACCFREE(acc);
			return NOTOK;
		}
		*bindRes = convert_BindResult(acc, errbuf);
		ACCFREE(acc);
		return DONE;
	    default:
		advise (errbuf, "%s", "Bad response from AcAsynRetryRequest");
		return NOTOK;
	}
}

/*  */

char	errBuf[BUFSIZ];

/* ARGSUSED */
static int  arg_quit (ad, args, arg)
int	ad;
char	**args, **arg;
{
	return release_connect(ad, errBuf);
}

int  release_connect (sd, errbuf)
int     sd;
char	errbuf[];
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

	if (AcRelRequest (sd, ACF_NORMAL, NULLPEP, 0, NOTOK, acr, aci) == NOTOK) {
		acs_advise (errbuf, aca, "A-RELEASE.REQUEST");
		return DONE;
	}

	if (!acr -> acr_affirmative) {
		(void) AcUAbortRequest (sd, NULLPEP, 0, aci);
		advise (errbuf,	"Release rejected by peer: %d",
			acr -> acr_reason);
	}

	ACRFREE (acr);
	PP_TRACE (("Association released"));

	return DONE;
}

/* ARGSUSED */
static int	general_error(ad, id, err, parameter, roi)
int	ad, id, err;
caddr_t	parameter;
struct RoSAPindication	*roi;
{
	struct RyError	*rye;

	if (RY_REJECT == err) {
		advise(errBuf, "%s", RoErrString ((int) parameter));
		return OK;
	}

	if (NULL != (rye = finderrbyerr (table_Qmgr_Errors, err)))
		advise(errBuf, "Error: %s", rye -> rye_name);
	else
		advise(errBuf, "Error: %d", err);

	return OK;
}

/*  */

static void ros_advise(buf, rop, event)
char	*buf;
struct RoSAPpreject	*rop;
char	*event;
{
	char	buffer[BUFSIZ];
	if (rop->rop_cc > 0)
		(void) sprintf (buffer,
				 "[%s] %*.*s",
				 RoErrString (rop->rop_reason),
				 rop->rop_cc, rop->rop_cc,
				 rop->rop_data);
	else
		(void) sprintf (buffer,
				"[%s]",
				RoErrString(rop->rop_reason));
	advise(buf, "%s: %s", event, buffer);
}

static void    acs_advise (buf, aca, event)
char	*buf;
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

	advise (buf, "%s: %s (source %d)", event, buffer,
		aca -> aca_source);
}

#ifndef lint
static void advise (va_alist)
va_dcl
{
	char	*buf;
	va_list	ap;

	va_start (ap);

	buf = va_arg(ap, char *);
	_asprintf(buf, NULLCP, ap);
	va_end(ap);
}
#else
/* VARARGS2 */

static void advise(what, fmt)
char	*what, *fmt;
{
	advise(what, fmt);
}
#endif
