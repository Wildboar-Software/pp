/* ryresponder.c - generic idempotent responder */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/ryresponder.c,v 6.0 1991/12/18 20:11:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/ryresponder.c,v 6.0 1991/12/18 20:11:26 jpo Rel $
 *
 * $Log: ryresponder.c,v $
 * Revision 6.0  1991/12/18  20:11:26  jpo
 * Release 6.0
 *
 */



/*
 *                                NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


#include "Qmgr-ops.h"                   /* operation definitions */
#include "util.h"
#include <stdarg.h>
#include "ryresponder.h"
#include <isode/tsap.h>         /* for listening */

/*    DATA */

static char     *myname = "ryresponder";
static jmp_buf  toplevel;
static IFP      startfnx;
static IFP      stopfnx;
static int	ros_init (),
		ros_work (),
		ros_indication (),
		ros_lose ();
extern int      errno;



/*    RESPONDER */

/* ARGSUSED */
int     ryresponder (argc, argv, host, myservice, dispatches, ops, start, stop)
int     argc;
char  **argv,
       *host,
       *myservice;
struct server_dispatch *dispatches;
struct RyOperation *ops;
IFP     start,
	stop;
{
    register struct server_dispatch   *ds;
    AEI     aei = NULLAEI;
    struct TSAPdisconnect   tds;
    struct TSAPdisconnect  *td = &tds;
    struct RoSAPindication  rois;
    register struct RoSAPindication *roi = &rois;
    register struct RoSAPpreject   *rop = &roi -> roi_preject;

    if ((myname = rindex (argv[0], '/')) != NULLCP)
	myname++;
    if (myname == NULL || *myname == NULL)
	myname = argv[0];

    PP_TRACE(("starting"));

    for (ds = dispatches; ds -> ds_name; ds++)
	if (RyDispatch (NOTOK, ops, ds -> ds_operation, ds -> ds_vector, roi)
		== NOTOK)
	    ros_adios (rop, ds -> ds_name);

    startfnx = start;
    stopfnx = stop;

    if (isodeserver (argc, argv, aei, ros_init, ros_work, ros_lose, td)
	    == NOTOK) {
	if (td -> td_cc > 0)
	    adios (NULLCP, "isodeserver: [%s] %*.*s",
		    TErrString (td -> td_reason),
		    td -> td_cc, td -> td_cc, td -> td_data);
	else
	    adios (NULLCP, "isodeserver: [%s]",
		    TErrString (td -> td_reason));
    }

    return 0;
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
    PP_LOG( LLOG_NOTICE,
	   ("A-ASSOCIATE.INDICATION: <%d, %s, %s, %s, %d>",
		acs -> acs_sd, sprintoid (acs -> acs_context),
		sprintaei (&acs -> acs_callingtitle),
		sprintaei (&acs -> acs_calledtitle), acs -> acs_ninfo));

    sd = acs -> acs_sd;

    for (vec++; *vec; vec++)
	PP_LOG( LLOG_EXCEPTIONS, ("unknown argument \"%s\"", *vec));

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
		(*stopfnx) (fd, (struct AcSAPfinish *) 0);
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

static int ros_indication (sd, roi)
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
		    PP_LOG( LLOG_NOTICE, 
			   ("RO-REJECT-U.INDICATION/%d: %s",
			    sd, RoErrString (rou -> rou_reason)));
		else
		    PP_LOG( LLOG_NOTICE,
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

		PP_LOG( LLOG_NOTICE, 
		       ("A-RELEASE.INDICATION/%d: %d",
			sd, acf -> acf_reason));

		reply = stopfnx ? (*stopfnx) (sd, acf) : ACS_ACCEPT;

		result = AcRelResponse (sd, reply, ACR_NORMAL, NULLPEP, 0,
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
	    adios (NULLCP, "unknown indication type=%d", roi -> roi_type);
    }
}

/*  */

static int  ros_lose (td)
struct TSAPdisconnect *td;
{
    if (td -> td_cc > 0)
	PP_LOG( LLOG_NOTICE,
	       ("TNetAccept: [%s] %*.*s",
		TErrString (td -> td_reason), td -> td_cc, td -> td_cc,
		td -> td_data));
    else
	    PP_LOG( LLOG_NOTICE,
		   ("TNetAccept: [%s]",TErrString (td -> td_reason)));
}

/*    ERRORS */

void    ros_adios (rop, event)
register struct RoSAPpreject *rop;
char   *event;
{
    ros_advise (rop, event);

    longjmp (toplevel, NOTOK);
}


void    ros_advise (rop, event)
register struct RoSAPpreject *rop;
char   *event;
{
    char    buffer[BUFSIZ];

    if (rop -> rop_cc > 0)
	(void) sprintf (buffer, "[%s] %*.*s", RoErrString (rop -> rop_reason),
		rop -> rop_cc, rop -> rop_cc, rop -> rop_data);
    else
	(void) sprintf (buffer, "[%s]", RoErrString (rop -> rop_reason));

    PP_LOG( LLOG_NOTICE, ("%s: %s", event, buffer));
}

/*  */

void    acs_advise (aca, event)
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

    PP_LOG( LLOG_NOTICE, 
	   ("%s: %s (source %d)", event, buffer,aca -> aca_source));
}

void adios (char *what, char* fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _ll_log (pp_log_norm, LLOG_FATAL, ap);
    va_end (ap);
    _exit (1);
}

void advise (char *what, char *fmt, ...)
{
    int code;
    va_list ap;
    va_start (ap, fmt);
    code = va_arg (ap, int);
    _ll_log (pp_log_norm, code, ap);
    va_end (ap);
}

void ryr_advise (char *what, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _ll_log (pp_log_norm, LLOG_NOTICE, ap);
    va_end (ap);
}
