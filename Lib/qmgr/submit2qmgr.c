/* submit_interface: interface to qmgr for submit's use */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/submit2qmgr.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/submit2qmgr.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: submit2qmgr.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include <stdarg.h>

#include "qmgr.h"

/* Sumbit types */
#include "q.h"
#include "prm.h"

/* Outside routines */
struct type_Qmgr_MsgStruct      *qstruct2qmgr();

/* DATA */
static char *myservice =        "pp qmgr";

/* OPERATIONS */
static int     newmessage_result();
static int     general_error();

#define newmessage_error        general_error

static void    acs_log ();
static void    ros_log ();
static int getresult ();
static int conn_status;
static char qmgr_addr[] = "qmgr-pa.cache";

/*  */
static struct PSAPaddr *get_addr (hostname)
char *hostname;
{
    register struct PSAPaddr *pa;
    AEI     aei;
    char buf[BUFSIZ];
    FILE *fp = NULL;

    if ((fp = fopen (qmgr_addr, "r")) != NULL &&
	fgets (buf, sizeof buf, fp) != NULL &&
	(pa = str2paddr (buf)) != NULLPA) {
	    (void) fclose (fp);
	    return pa;
    }
    if (fp)
	    (void) fclose (fp);

    if ((aei = _str2aei(hostname, myservice, QMGR_CTX_OID, 0, dap_user,
			  dap_passwd)) == NULLAEI) {
	    PP_LOG (LLOG_EXCEPTIONS,
		    ("qmgr_start %s-%s: unknown application-entity",
		     hostname,myservice));
	    return NULLPA;
    }
    if ((pa = aei2addr(aei)) == NULLPA) {
	    PP_LOG (LLOG_EXCEPTIONS,
		    ("qmgr_address translation failed"));
	    return NULLPA;
    }
    if ((fp = fopen (qmgr_addr, "w")) != NULL) {
	    (void) fprintf (fp, "%s\n", _paddr2str (pa, NULLNA, -1));
	    (void) fclose (fp);
    }
    else
	    PP_SLOG (LLOG_EXCEPTIONS, qmgr_addr,
		     ("Can't write qmgr addr cache"));
    return pa;
}

/* main interface routines */

int qmgr_start(hostname, status, async)
char    *hostname;
int *status;
int async;
{
    int sd;
    struct SSAPref sfs;
    register struct SSAPref *sf;
    register struct PSAPaddr *pa;
    struct AcSAPconnect accs;
    register struct AcSAPconnect   *acc = &accs;
    struct AcSAPindication  acis;
    register struct AcSAPindication *aci = &acis;
    register struct AcSAPabort *aca = &aci -> aci_abort;
    OID     ctx,
	    pci;
    struct PSAPctxlist pcs;
    register struct PSAPctxlist *pc = &pcs;
    struct RoSAPindication rois;
    register struct RoSAPindication *roi = &rois;
    register struct RoSAPpreject *rop = &roi -> roi_preject;

    if ((pa = get_addr (hostname)) == NULLPA) {
	    PP_LOG (LLOG_EXCEPTIONS, ("Can't get paddr of qmgr"));
	    return NOTOK;
    }

    if ((ctx = oid_cpy(QMGR_AC)) == NULLOID) {
	    PP_LOG (LLOG_EXCEPTIONS,
		    ("qmgr_start: out of memory"));
	    return NOTOK;
    }
    if ((pci = oid_cpy(QMGR_PCI)) == NULLOID) {
	    PP_LOG (LLOG_EXCEPTIONS,
		    ("qmgr_start: out of memory"));
	    return NOTOK;
    }
    pc->pc_nctx = 1;
    pc->pc_ctx[0].pc_id = 1;
    pc->pc_ctx[0].pc_asn = pci;
    pc->pc_ctx[0].pc_atn = NULLOID;

    if ((sf = addr2ref ("submit")) == NULL) {
	    sf = &sfs;
	    (void) bzero ((char *) sf, sizeof(*sf));
    }

    switch (AcAsynAssocRequest (ctx, NULLAEI, NULLAEI, NULLPA, pa, pc, NULLOID,
			    0, ROS_MYREQUIRE, SERIAL_NONE, 0, sf, NULLPEP,
			    0, NULLQOS, acc, aci, async)) {
	case NOTOK:
	    *status = conn_status = NOTOK;
	    acs_log(aca,"A-ASSOCIATE.REQUEST");
	    return NOTOK;

	case CONNECTING_2:
	    *status = conn_status = CONNECTING_2;
	    sd = acc -> acc_sd;
	    ACCFREE (acc);
	    return sd;

	case CONNECTING_1: /* if !async - this is OK */
	    if (async == 1) {
		    *status = conn_status = CONNECTING_1;
		    sd = acc -> acc_sd;
		    ACCFREE (acc);
		    return sd;
	    }
	    /* otherwise - we are really DONE */

	case DONE:
	    if (acc->acc_result != ACS_ACCEPT) {
		    PP_LOG (LLOG_EXCEPTIONS,
			    ("qmgr_start: Association rejected: [%s]",
			     AcErrString(acc->acc_result)));
		    return NOTOK;
	    }
	    sd = acc->acc_sd;
	    ACCFREE(acc);
	    if (RoSetService(sd, RoPService, roi) == NOTOK) {
		    ros_log(rop,"set RO/PS fails");
		    return NOTOK;
	    }
	    *status = conn_status = DONE;
	    return sd;
    }
    return NOTOK;
}

int qmgr_retry (fd, status)
int	fd;
int	*status;
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

	PP_DBG (("acsap_retry(%d)", fd));
	if (conn_status == DONE)
		return fd;
	switch (result = AcAsynRetryRequest (fd, acc, aci)) {
	    case CONNECTING_1:
	    case CONNECTING_2:
		ACCFREE (acc);
		*status = conn_status = result;
		return fd;
	    case NOTOK:
		acs_log (aca, "A-ASSOCIATE.REQUEST");
		conn_status = NOTOK;
		return NOTOK;
	    case DONE:
		if (acc -> acc_result != ACS_ACCEPT) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Association Rejected: [%s]",
				 AcErrString (acc -> acc_result)));
			return NOTOK;
		}
		ACCFREE (acc);
		if (RoSetService (fd, RoPService, roi) == NOTOK) {
			ros_log (rop, "set RO/PS fails");
			return NOTOK;
		}
		*status = conn_status = DONE;
		return fd;
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Bad response from AcAsynRetryRequest"));
		return NOTOK;
	}
}


static int outstanding = NOTOK;

int qmgr_end(sd)
int     sd;
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

	if (outstanding != NOTOK)
		if (getresult (sd, outstanding, 60) == NOTOK)
			return NOTOK;

	outstanding = NOTOK;
#ifdef CONNECTING_1
	if (AcRelRequest(sd,ACF_NORMAL,NULLPEP,0, NOTOK, acr,aci) == NOTOK) {
#else
	if (AcRelRequest(sd,ACF_NORMAL,NULLPEP,0,acr,aci) == NOTOK) {
#endif
		acs_log(aca,"A-RELEASE.REQUEST");
		return NOTOK;
	}

	if (!acr->acr_affirmative) {
		(void) AcUAbortRequest (sd, NULLPEP, 0, aci);
		PP_LOG (LLOG_EXCEPTIONS,
			("qmgr_end: Release rejected by peer: %d",
			 acr->acr_reason));
		return NOTOK;
	}

	ACRFREE(acr);
	return OK;
}


int message_send (sd, file, prm, que, sender, recips, rcount)
int             sd;
char            *file;
struct prm_vars *prm;
Q_struct        *que;
ADDR            *sender,
		*recips;
int             rcount;
{
/* copied from invoke in ryinitiator */
	int                             result;
	struct type_Qmgr_MsgStruct      *in;
	struct RoSAPindication          rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject    *rop = &roi->roi_preject;

	while (conn_status != DONE && conn_status != NOTOK) {
		int status = conn_status;
		sd = qmgr_retry (sd, &status);
		if (conn_status == DONE)
			break;
		sleep (1);
	}
	if (sd == NOTOK || conn_status == NOTOK)
		return NOTOK;

	if (outstanding != NOTOK)
		if (getresult (sd, outstanding, NOTOK) == NOTOK)
			return NOTOK;
	outstanding = NOTOK;

	if ((in = qstruct2qmgr(file,prm,que,sender,recips,rcount)) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS,
			("qstruct2qmgr failure for '%s'",
			 file));
		return NOTOK;
	}
	if (in->recipientlist == NULL) {
		/* no users waiting so no need to pass to qmgr */
		PP_LOG (LLOG_EXCEPTIONS,
			("message_send: No recips for '%s'",
			 file));
		free_Qmgr_PerMessageInfo(in->messageinfo);
		free((char *) in);
		return OK;
	}

	switch (result = RyStub (sd, table_Qmgr_Operations, 
				 operation_Qmgr_newmessage,
				 outstanding = RyGenID(sd), NULLIP,
				 (caddr_t) in, 
				 newmessage_result,
				 newmessage_error, ROS_ASYNC, roi)) {
	    case NOTOK:
		if (ROS_FATAL(rop->rop_reason)) {
			PP_LOG (LLOG_EXCEPTIONS,
				("STUB"));
			return outstanding = NOTOK;
		}
		PP_LOG (LLOG_EXCEPTIONS,
			("message_send:STUB"));
		break;

	    case OK:
		break;

	    case DONE:
		PP_LOG (LLOG_EXCEPTIONS,
			("got RO-END.INDICATION"));
		outstanding = NOTOK;
		return qmgr_end(sd);

	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("submit_interface unknown return from RyStub=%d",result));
		return outstanding = NOTOK;
	}
	if (in)
		free_Qmgr_MsgStruct(in);
	return OK;
}

/*  */
/* subsiduary routines */

/* ARGSUSED */
static int newmessage_result (sd, id, dummy, result, roi)
int                                     sd,
					id,
					dummy;
struct type_Qmgr_Pseudo__newmessage     *result;
struct RoSAPindication                  *roi;
{
	return OK;
}

/* ARGSUSED */
static int general_error (sd, id, error, parameter, roi)
int     sd,
	id,
	error;
caddr_t parameter;
struct RoSAPindication *roi;
{
	register struct RyError *rye;

	if (error == RY_REJECT) {
		PP_LOG (LLOG_EXCEPTIONS, ("%s", RoErrString ((int) parameter)));
		return OK;
	}

	if (rye = finderrbyerr (table_Qmgr_Errors, error))
		PP_LOG (LLOG_EXCEPTIONS, ("%s", rye -> rye_name));
	else
		PP_LOG (LLOG_EXCEPTIONS, ("Error %d", error));

	return OK;
}

/*  */

static void    ros_log (rop, event)
register struct RoSAPpreject *rop;
char   *event;
{
    char    buffer[BUFSIZ];

    if (rop -> rop_cc > 0)
	(void) sprintf (buffer, "[%s] %*.*s", RoErrString (rop -> rop_reason),
		rop -> rop_cc, rop -> rop_cc, rop -> rop_data);
    else
	(void) sprintf (buffer, "[%s]", RoErrString (rop -> rop_reason));

    PP_LOG (LLOG_EXCEPTIONS,
	    ("Lib/qmgr/submit_interface %s: %s", event, buffer));
}

/*  */

static void    acs_log (aca, event)
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

	PP_LOG (LLOG_EXCEPTIONS,
		("Lib/qmgr/submit_interface %s: %s (source %d)", event, buffer,
		 aca -> aca_source));
}

static int getresult (sd, id, delay)
int	sd, id, delay;
{
	caddr_t out;
	struct RoSAPindication          rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject    *rop = &roi->roi_preject;
	
	switch (RyWait (sd, &id, &out, delay, roi)) {
	    case NOTOK:
		if (rop -> rop_reason == ROS_TIMER)
			break;
		ros_log (rop, "RyWait Stub");
		if (ROS_FATAL (rop -> rop_reason)) {
			PP_LOG (LLOG_EXCEPTIONS,
				("qmgr-interface failed fatally"));
			return NOTOK;
		}
		break;
	    case OK:
		break;

	    case DONE:
		PP_LOG (LLOG_EXCEPTIONS,
			("qmgr interface quit"));
		return NOTOK;

	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("qmgr interface - bad response"));
		return NOTOK;
	}
	return OK;
}
