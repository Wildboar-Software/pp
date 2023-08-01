/* channel_control.c: control the channel process */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/chan_control.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/chan_control.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: chan_control.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */



#include <stdio.h>
#include <syslog.h>
#include <setjmp.h>
#include <varargs.h>
#include "qmgr.h"
#include "retcode.h"
#include <isode/tsap.h>
#include "ll_log.h"

static jmp_buf toplevel;


static IFP      startfnx;
static IFP      stopfnx;
static IFP      initchfnx;
static struct type_Qmgr_DeliveryStatus *(*workfnx)();

static int     ros_init (), ros_work (), ros_lose ();
static int     ros_worker (), error (), ureject ();
static void    adios ();
static void    acs_advise ();
static void    ros_adios (), ros_advise ();
static void    ros_indication ();

#ifdef notdef
static  char    *myservice = "pp channel";
#endif
extern struct RyOperation table_Qmgr_Operations[];

int channel_control (argc, argv, init, work, finish)
int     argc;
char    **argv;
IFP     init;
struct type_Qmgr_DeliveryStatus *(*work)();
IFP     finish;
{
	AEI         aei = NULLAEI;
	struct TSAPdisconnect   tds;
	struct TSAPdisconnect  *td = &tds;
	struct RoSAPindication  rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject   *rop = &roi -> roi_preject;

	PP_DBG (("starting"));

	workfnx = work;
	initchfnx = init;
	stopfnx = finish;

	if (RyDispatch (NOTOK, table_Qmgr_Operations,
			operation_Qmgr_processmessage,
			ros_worker, roi) == NOTOK)
		ros_adios (rop, "processmessage");
	if (RyDispatch (NOTOK, table_Qmgr_Operations,
			operation_Qmgr_channelInitialise,
			ros_worker, roi) == NOTOK)
		ros_adios (rop, "channelInitialise");

	if (isodeserver (argc, argv, aei, ros_init, ros_work, ros_lose, td)
	    == NOTOK) {
		if (td -> td_cc > 0)
			adios (NULLCP, "isodeserver: [%s] %*.*s",
			       TErrString (td -> td_reason),
			       td -> td_cc, td -> td_cc,
			       td -> td_data);
		else
			adios (NULLCP, "isodeserver: [%s]",
			       TErrString (td -> td_reason));
	}

	return 0;
}

static int ros_result (sd, val, rox, roi)
int     sd;
struct type_Qmgr_DeliveryStatus *val;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	if (val == NULL)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);

	if (RyDsResult (sd, rox -> rox_id, (caddr_t) val, ROS_NOPRIO, roi)
			== NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	return OK;
}

static int ros_worker (sd, ryo, rox, in, roi)
int     sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_DeliveryStatus *status;

	if (rox -> rox_nolinked == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
			sd, ryo -> ryo_name, rox -> rox_linkid));
		return ureject (sd, ROS_IP_LINKED, rox, roi);
	}
	PP_DBG (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	switch (ryo -> ryo_op) {
	    case operation_Qmgr_channelInitialise:
		switch ((*initchfnx) (in)) {
		    case OK:
			if (RyDsResult (sd, rox -> rox_id, (caddr_t) NULL,
					ROS_NOPRIO, roi) == NOTOK)
				ros_adios (&roi -> roi_preject, "RESULT");
			return OK;

		    case NOTOK:
		    default:
			return error (sd, error_Qmgr_protocol, (caddr_t) NULL,
				      rox, roi);
		}

	    case operation_Qmgr_processmessage:
		status = (*workfnx) (in);
		return ros_result (sd, status, rox, roi);

	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("RO-INVOKE.INDICATION/%d: operation %s not expected",
			sd, ryo -> ryo_name));
		return error (sd, error_Qmgr_protocol, (caddr_t) NULL,
			      rox, roi);
	}
}

/*  */

static int  ros_init (vecp, vec)
int     vecp;
char  **vec;
{
    int     reply,
	    result,
	    sd;
    struct AcSAPstart   acss;
    register struct AcSAPstart *acs = &acss;
    struct AcSAPindication  acis;
    register struct AcSAPindication *aci = &acis;
    register struct AcSAPabort   *aca = &aci -> aci_abort;
    register struct PSAPstart *ps = &acs -> acs_start;
    struct RoSAPindication  rois;
    register struct RoSAPindication *roi = &rois;
    register struct RoSAPpreject   *rop = &roi -> roi_preject;

    if (AcInit (vecp, vec, acs, aci) == NOTOK) {
	acs_advise (aca, "initialization fails");
	return NOTOK;
    }
    PP_DBG (("A-ASSOCIATE.INDICATION: <%d, %s, %s, %s, %d>",
		acs -> acs_sd, sprintoid (acs -> acs_context),
		sprintaei (&acs -> acs_callingtitle),
		sprintaei (&acs -> acs_calledtitle), acs -> acs_ninfo));

    sd = acs -> acs_sd;

    for (vec++; *vec; vec++)
	PP_LOG (LLOG_EXCEPTIONS, ("unknown argument \"%s\"", *vec));

    reply = startfnx ? (*startfnx) (sd, acs) : ACS_ACCEPT;

    result = AcAssocResponse (sd, reply, 
		reply != ACS_ACCEPT ? ACS_USER_NOREASON : ACS_USER_NULL,
		NULLOID, NULLAEI, NULLPA, NULLPC, ps -> ps_defctxresult,
		ps -> ps_prequirements, ps -> ps_srequirements, SERIAL_NONE,
		ps -> ps_settings, &ps -> ps_connect, NULLPEP, 0, aci);

    ACSFREE (acs);

    if (result == NOTOK) {
	acs_advise (aca, "A-ASSOCIATE.RESPONSE");
	return NOTOK;
}
    if (reply != ACS_ACCEPT)
	return NOTOK;

    if (RoSetService (sd, RoPService, roi) == NOTOK)
	ros_adios (rop, "set RO/PS fails");

    return sd;
}

/*  */

static int  ros_work (fd)
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
		(*stopfnx) ();
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
	    adios (NULLCP, "unknown return from RoWaitRequest=%d", result);
    }

    return OK;
}

/*  */

static void ros_indication (sd, roi)
int     sd;
register struct RoSAPindication *roi;
{
    int     reply,
	    result;

    switch (roi -> roi_type) {
	case ROI_INVOKE: 
	case ROI_RESULT: 
	case ROI_ERROR: 
	    adios (NULLCP, "unexpected indication type=%d", roi -> roi_type);
	    break;

	case ROI_UREJECT: 
	    {
		register struct RoSAPureject   *rou = &roi -> roi_ureject;

		if (rou -> rou_noid)
		    PP_LOG (LLOG_EXCEPTIONS, ("RO-REJECT-U.INDICATION/%d: %s",
			    sd, RoErrString (rou -> rou_reason)));
		else
		    PP_LOG (LLOG_EXCEPTIONS, 
			    ("RO-REJECT-U.INDICATION/%d: %s (id=%d)",
			    sd, RoErrString (rou -> rou_reason),
			    rou -> rou_id));
	    }
	    break;

	case ROI_PREJECT: 
	    {
		register struct RoSAPpreject   *rop = &roi -> roi_preject;

		if (ROS_FATAL (rop -> rop_reason))
		    ros_adios (rop, "RO-REJECT-P.INDICATION");
		ros_advise (rop, "RO-REJECT-P.INDICATION");
	    }
	    break;

	case ROI_FINISH: 
	    {
		register struct AcSAPfinish *acf = &roi -> roi_finish;
		struct AcSAPindication  acis;
		register struct AcSAPabort *aca = &acis.aci_abort;

		PP_TRACE (("A-RELEASE.INDICATION/%d: %d",
			sd, acf -> acf_reason));

		result = AcRelResponse (sd, reply = ACS_ACCEPT,
					ACR_NORMAL, NULLPEP, 0, &acis);

		ACFFREE (acf);

		if (result == NOTOK)
		    acs_advise (aca, "A-RELEASE.RESPONSE");
		else
		    if (reply != ACS_ACCEPT)
			break;

		if (stopfnx)
			(*stopfnx)();

		longjmp (toplevel, DONE);
	    }
	/* NOTREACHED */

	default: 
	    adios (NULLCP, "unknown indication type=%d", roi -> roi_type);
    }
}

/*  */

static int  ros_lose (td)
struct TSAPdisconnect *td;
{
    if (td -> td_cc > 0)
	PP_LOG (LLOG_EXCEPTIONS, ("TNetAccept: [%s] %*.*s",
		TErrString (td -> td_reason), td -> td_cc, td -> td_cc,
		td -> td_data));
    else
	PP_LOG (LLOG_EXCEPTIONS, ("TNetAccept: [%s]",
		TErrString (td -> td_reason)));
}

/*    ERRORS */

static void    ros_adios (rop, event)
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
	(void) sprintf (buffer, "[%s] %*.*s", RoErrString (rop -> rop_reason),
		rop -> rop_cc, rop -> rop_cc, rop -> rop_data);
    else
	(void) sprintf (buffer, "[%s]", RoErrString (rop -> rop_reason));

    PP_LOG (LLOG_EXCEPTIONS, ("%s: %s", event, buffer));
}

/*  */

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
	(void) sprintf (buffer, "[%s]", AcErrString (aca -> aca_reason));

    PP_LOG (LLOG_EXCEPTIONS, ("%s: %s (source %d)", event, buffer,
		aca -> aca_source));
}

/*  */

#ifndef lint

static void    adios (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _ll_log (pp_log_oper, LLOG_FATAL, ap);

    va_end (ap);

    _exit (1);
}
#else
/* VARARGS */

static void    adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif

/*    ERROR */

static int  error (sd, err, param, rox, roi)
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

static int  ureject (sd, reason, rox, roi)
int     sd,
	reason;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
    if (RyDsUReject (sd, rox -> rox_id, reason, ROS_NOPRIO, roi) == NOTOK)
	ros_adios (&roi -> roi_preject, "U-REJECT");

    return OK;
}
