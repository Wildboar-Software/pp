/* ryresponder.c - responder stuff - mackled out of ryresponder.c */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/qmgr/RCS/ryresponder.c,v 6.0 1991/12/18 20:27:38 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/ryresponder.c,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: ryresponder.c,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "qmgr.h"
#include "ryresponder.h"
#include <isode/tsap.h>         /* for listening */
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

/*    DATA */

char     *myname = "qmgr";
static jmp_buf  toplevel;
static IFP      startfnx;
static IFP      stopfnx;
static int      ros_init (),
		ros_work (),
		ros_lose ();
static void	ros_indication ();
extern void	chan_manage (),
		isodeexport (),
		init_chans ();
extern int      errno;
extern int	nobackground;
extern time_t	time ();
fd_set          perf_rfds,
		perf_wfds;
int             perf_nfds;
int		delaytime = NOTOK;
extern time_t current_time;

void		start_specials (), schedule ();

static int  TMagic (vecp, vec, td)
int     *vecp;
char   **vec;
struct TSAPdisconnect *td;
{
	int     sd;
	struct TSAPstart tss;
	register struct TSAPstart  *ts = &tss;

	if (TInit (*vecp, vec, ts, td) == NOTOK)
		return NOTOK;
	sd = ts -> ts_sd;

	if (TConnResponse (sd, &ts -> ts_called, ts -> ts_expedited, NULLCP, 0,
			   NULLQOS, td) == NOTOK)
		return NOTOK;

	if (TSaveState (sd, vec + 1, td) == NOTOK)
		return NOTOK;
	vec[*vecp = 2] = NULL;

	return OK;
}

background ()
{
	int i;

	for (i = 0; i < 5; i++) {
		switch (fork ()) {
		    case NOTOK: 
			sleep (5);
			continue;

		    case OK: 
			break;

		    default: 
			_exit (0);
		}
		break;
	}

	if ((i = open ("/dev/null", O_RDWR)) != NOTOK) {
		if (i != 0)
			(void) dup2 (i, 0), (void) close (i);
		(void) dup2 (0, 1);
		(void) dup2 (0, 2);
	}

#ifdef	SETSID
	(void) setsid ();
#endif
#ifdef	TIOCNOTTY
	if ((i = open ("/dev/tty", O_RDWR)) != NOTOK) {
		(void) ioctl (i, TIOCNOTTY, NULLCP);
		(void) close (i);
	}
#else
#ifdef	SYS5
	(void) setpgrp ();
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
#endif
#endif
	isodexport (NULLCP);	/* re-initialize logfiles */
}

/*    RESPONDER */

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
	AEI     aei;
	struct TSAPdisconnect   tds;
	struct TSAPdisconnect  *td = &tds;
	struct RoSAPindication  rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject   *rop = &roi -> roi_preject;
	fd_set      rfds;
	fd_set      wfds;
	int	n;
	int fd;

	PP_TRACE (("ryresponder (%d, argv, %s, %s...)",
		   argc, host, myservice));
	FD_ZERO (&perf_rfds);
	FD_ZERO (&rfds);
	FD_ZERO (&perf_wfds);
	FD_ZERO (&wfds);

	if ((myname = rindex (argv[0], '/')) != NULLCP)
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = argv[0];

	isodetailor (myname, 0);
	ll_hdinit (pp_log_norm, myname);

	if (isatty (fileno (stderr)))
		pp_log_norm -> ll_stat |= LLOGTTY;
	else {
		if (nobackground == 0)
			background ();
			
	}

	PP_TRACE (("starting"));
	init_chans ();

	for (ds = dispatches; ds -> ds_name; ds++)
		if (RyDispatch (NOTOK, ops, ds -> ds_operation,
				ds -> ds_vector, roi) == NOTOK)
			ros_adios (rop, ds -> ds_name);

	startfnx = start;
	stopfnx = stop;

	for (n = 1;; n++) {
		if ((aei = _str2aei (host,myservice, QMGR_CTX_OID, 0,
				     dap_user, dap_passwd)) == NULLAEI)
			adios (NULLCP, "%s %s (%s): unknown service",
			       host, QMGR_CTX_OID, myservice);

		if (iserver_init_aux (0, argv, aei, ros_init, TMagic, 1, td) == NOTOK) {
			if (n > 15) {
				if (td -> td_cc > 0)
					adios (NULLCP, 
					       "iserver_init: [%s] %*.*s",
					       TErrString (td -> td_reason),
					       td -> td_cc, td -> td_cc,
					       td -> td_data);
				else
					adios (NULLCP, "iserver_init: [%s]",
					       TErrString (td -> td_reason));
			}
			if (td -> td_cc > 0)
				PP_LOG (LLOG_EXCEPTIONS, 
					("iserver_init: [%s] %*.*s",
					TErrString (td -> td_reason),
					td -> td_cc, td -> td_cc,
					td -> td_data));
			else
				PP_LOG (LLOG_EXCEPTIONS,
					("iserver_init: [%s]",
					 TErrString (td -> td_reason)));
			sleep ((unsigned) 5 * n);
			continue;
		}
		break;
	}
#ifdef MALLCHK
	_mal_trace_restart ();
#endif
	start_specials ();

	PP_TRACE(("Listening"));

	for (;;) {
		schedule ();
		rfds = perf_rfds;
		wfds = perf_wfds;
		PP_TRACE (("Main loop..."));
		switch (iserver_wait (ros_init, ros_work, ros_lose, perf_nfds,
				      &rfds, &wfds, NULLFD, delaytime, td)) {
		    case DONE:
			return 0;

		    case NOTOK:
			if (td -> td_cc > 0)
				adios (NULLCP, "iserver_wait: [%s] %*.*s",
				       TErrString (td -> td_reason),
				       td -> td_cc, td -> td_cc,
				       td -> td_data);
			else
				adios (NULLCP, "iserver_wait: [%s]",
				       TErrString (td -> td_reason));
			break;
		    case OK:
			(void) time (&current_time);

			for (fd = 0; fd < perf_nfds; fd++)
				if (FD_ISSET (fd, &rfds) ||
				    FD_ISSET (fd, &wfds))
					chan_manage (fd);
			break;
		    default:
			adios (NULLCP, "Illegal return from iserver_wait");
		}
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
	PE	pep[1];
	int	npe = 0;
	struct AcSAPstart   acss;
	register struct AcSAPstart *acs = &acss;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort   *aca = &aci -> aci_abort;
	register struct PSAPstart *ps = &acs -> acs_start;
	struct RoSAPindication  rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject   *rop = &roi -> roi_preject;
	struct TSAPdisconnect   tds;
	struct TSAPdisconnect  *td = &tds;

	PP_TRACE (("ros_init (%d, vec)", vecp));
	
	if (AcInit (vecp, vec, acs, aci) == NOTOK) {
		acs_advise (aca, "initialization fails");
		return NOTOK;
	}
	PP_NOTICE (("A-ASSOCIATE.INDICATION/%d", acs -> acs_sd));
	sd = acs -> acs_sd;
	
	for (vec++; *vec; vec++)
		if (**vec)
			PP_LOG (LLOG_EXCEPTIONS,
				("unknown argument \"%s\"", *vec));
	
	pep[0] = NULLPE;
	reply = startfnx ? (*startfnx) (sd, acs, pep, &npe) : ACS_ACCEPT;
	
	result = AcAssocResponse (sd, reply, 
				  reply != ACS_ACCEPT ?
				  ACS_USER_NOREASON : ACS_USER_NULL, 
				  NULLOID, NULLAEI, NULLPA, NULLPC,
				  ps -> ps_defctxresult,
				  ps -> ps_prequirements,
				  ps -> ps_srequirements, SERIAL_NONE,
				  ps -> ps_settings, &ps -> ps_connect,
				  pep, npe, aci);
	
	if (pep[0])
		pe_free (pep[0]);
	ACSFREE (acs);
	
	if (result == NOTOK) {
		acs_advise (aca, "A-ASSOCIATE.RESPONSE");
		return NOTOK;
	}
	if (reply != ACS_ACCEPT)
		return NOTOK;
	
	if (RoSetService (sd, RoPService, roi) == NOTOK)
		ros_adios (rop, "set RO/PS fails");
	
	if (TSetQueuesOK (sd, 1, td) == NOTOK) {
		if (td -> td_cc > 0)
			PP_LOG (LLOG_EXCEPTIONS,
				("iserver_init: [%s] %*.*s",
				 TErrString (td -> td_reason),
				 td -> td_cc, td -> td_cc,
				 td -> td_data));
		else
			PP_LOG (LLOG_EXCEPTIONS,
				("iserver_init: [%s]",
				 TErrString (td -> td_reason)));
		
	}
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
	extern time_t time ();
	
	PP_TRACE (("ros_work (%d)", fd));
	
	(void) time (&current_time);
	switch (setjmp (toplevel)) {
	    case OK: 
		break;
		
	    default: 
		if (stopfnx)
			(void) (*stopfnx) (fd, (struct AcSAPfinish *) 0);
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
	
	PP_TRACE (("ros_indication (%d)", sd));
	
	switch (roi -> roi_type) {
	    case ROI_INVOKE: 
	    case ROI_RESULT: 
	    case ROI_ERROR: 
		adios (NULLCP, "unexpected indication type=%d",
		       roi -> roi_type);
		break;
		
	    case ROI_UREJECT: 
	{
		register struct RoSAPureject   *rou =
			&roi -> roi_ureject;
		
		if (rou -> rou_noid)
			PP_LOG (LLOG_EXCEPTIONS,
				("RO-REJECT-U.INDICATION/%d: %s",
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
		char tbuf[BUFSIZ];
		register struct RoSAPpreject   *rop = &roi -> roi_preject;
		
		(void) sprintf (tbuf, "RO-REJECT-P.INDICATION/%d",
				sd);
		if (ROS_FATAL (rop -> rop_reason))
			ros_adios (rop, tbuf);
		ros_advise (rop, tbuf);
	}
		break;
		
	    case ROI_FINISH: 
	{
		register struct AcSAPfinish *acf = &roi -> roi_finish;
		struct AcSAPindication  acis;
		register struct AcSAPabort *aca = &acis.aci_abort;
		
		PP_NOTICE (("A-RELEASE.INDICATION/%d: %d",
			  sd, acf -> acf_reason));
		
		reply = stopfnx ? (*stopfnx) (sd, acf) : ACS_ACCEPT;
		
		result = AcRelResponse (sd, reply, ACR_NORMAL,
					NULLPEP, 0, NOTOK, &acis);
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
	PP_TRACE (("ros_lose ()"));
	
	if (td -> td_cc > 0)
		PP_LOG (LLOG_EXCEPTIONS,
			("TNetAccept: [%s] %*.*s",
			 TErrString(td -> td_reason), td -> td_cc, td -> td_cc,
			 td -> td_data));
	else
		PP_LOG (LLOG_EXCEPTIONS,
			("TNetAccept: [%s]",
			 TErrString (td -> td_reason)));
	return OK;
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
		(void) sprintf (buffer, "[%s] %*.*s",
				RoErrString (rop -> rop_reason),
				rop -> rop_cc, rop -> rop_cc, rop -> rop_data);
	else
		(void) sprintf (buffer, "[%s]",
				RoErrString (rop -> rop_reason));
	
	PP_LOG (LLOG_EXCEPTIONS, ("%s: %s", event, buffer));
}

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
		(void) sprintf (buffer, "[%s]",
				AcErrString (aca -> aca_reason));
	
	PP_LOG (LLOG_EXCEPTIONS, ("%s: %s (source %d)", event, buffer,
				  aca -> aca_source));
}

void adios (char *what, char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	_ll_log (pp_log_norm, LLOG_FATAL, ap);
	va_end (ap);
	_exit (1);
}

void advise (int code, char *what, char *fmt, ...)
{
	int code;
	va_list ap;
	va_start (ap, fmt);
	code = va_arg (ap, int);
	_ll_log (pp_log_norm, code, ap);
	va_end (ap);
}

void ryr_advise (char *what, char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	_ll_log (pp_log_norm, LLOG_NOTICE, ap);
	va_end (ap);
}
