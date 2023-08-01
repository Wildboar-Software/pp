/* ppp.c: ppp interface routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/grey/RCS/ppp.c,v 6.0 1991/12/18 20:10:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/grey/RCS/ppp.c,v 6.0 1991/12/18 20:10:19 jpo Rel $
 *
 * $Log: ppp.c,v $
 * Revision 6.0  1991/12/18  20:10:19  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "ppp.h"
#include "qmgr.h"
#include "prm.h"
#include "q.h"
#include "adr.h"
#include "chan.h"
#include <varargs.h>
#include "retcode.h"
#include "dr.h"
#include "ap.h"

static int ppp_sd = NOTOK;
static CHAN *mychan;
static struct prm_vars prm;
static Q_struct Qs;
static ADDR *ad_sender, *ad_recip, *ad_list;
static enum { jnthdr, initialise, go } state;
static char *this_msg;
static int naddrs = 0;
static char *cur_host;
static char *cur_net;
static char *jnt_host;
static int rox_id;
static int ppp_error_status;
static int data_bytes;
static struct timeval data_timer;
#if PP_DEBUG > PP_DEBUG_NONE
static int ppp_debug = 0;
static int debug_init (), debug_nextmsg ();
static void pdeliverystatus (), pindividualdeliverystatus ();
#endif

static int ppp_initfnx (), ppp_procfnx ();
static void ppp_lose ();
static int ros_indication (), ros_work (), ros_result (),
	error (), ureject ();
static void ros_advise (), acs_advise ();
static void cleanup_everything ();
static char *bigendian ();
static char *mk_jnt ();
static int assemble_jntheader ();
static int fetch_data ();
static int ppp_procfnx_aux ();

static int lastop = NOTOK;
#define INITOP	1
#define PROCOP	2
extern struct RyOperation table_Qmgr_Operations[];
extern char *quedfldir;

int ppp_init (argc, argv)
int	argc;
char	**argv;
{
	struct AcSAPstart   acss;
	register struct AcSAPstart *acs = &acss;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort   *aca = &aci -> aci_abort;
        struct RoSAPindication  rois;
        register struct RoSAPindication *roi = &rois;
        register struct RoSAPpreject   *rop = &roi -> roi_preject;
	register struct PSAPstart *ps = &acs -> acs_start;
	int	result;

	chan_init (argv[0]);
	(void) chdir (quedfldir);
#if PP_DEBUG > PP_DEBUG_NONE
	for (result = 0; result < argc; result ++)
		if (strcmp (argv[result], "debug") == 0)
			return debug_init ();
#endif
	if (RyDispatch (NOTOK, table_Qmgr_Operations,
			operation_Qmgr_channelInitialise, ppp_initfnx,
			roi) == NOTOK) {
		ros_advise (rop, "RyDispatch: channelInitialise");
		return NOTOK;
	}
	if (RyDispatch (NOTOK, table_Qmgr_Operations,
			operation_Qmgr_processmessage, ppp_procfnx,
			roi) == NOTOK) {
		ros_advise (rop, "RyDispatch: processmessage");
		return NOTOK;
	}

	if (AcInit (argc, argv, acs, aci) == NOTOK) {
		acs_advise (aca, "AcInit");
		return NOTOK;
	}

	PP_DBG (("A-ASSOCIATE.INDICATION: <%d, %s, %s, %s, %d>",
		 acs -> acs_sd, sprintoid (acs -> acs_context),
		 sprintaei (&acs -> acs_callingtitle),
		 sprintaei (&acs -> acs_calledtitle), acs -> acs_ninfo));
	ppp_sd = acs -> acs_sd;

	result = AcAssocResponse (ppp_sd, ACS_ACCEPT, ACS_USER_NULL,
				  NULLOID, NULLAEI, NULLPA, NULLPC,
				  ps -> ps_defctxresult,
				  ps -> ps_prequirements,
				  ps -> ps_srequirements, SERIAL_NONE,
				  ps -> ps_settings, &ps -> ps_connect,
				  NULLPEP, 0, aci);
	ACSFREE (acs);
	if (result == NOTOK) {
		acs_advise (aca, "A-ASSOCIATE.RESPONSE");
		return ppp_sd = NOTOK;
	}
	if (RoSetService (ppp_sd, RoPService, roi) == NOTOK) {
		ros_advise (rop, "set RO/PS fails");
		ppp_lose ();
		return NOTOK;
	}

	/* try and get the init channel op */
	if (ros_work (ppp_sd, INITOP) != OK) {
		ppp_lose ();
		return NOTOK;
	}
	return OK;
}


int ppp_getnextmessage (host, net)
char	**host;
char	**net;
{
	int	result;

	state = jnthdr;
#if PP_DEBUG > PP_DEBUG_NONE
	if (ppp_debug)
		result = debug_nextmsg ();
	else
#endif
	result = ros_work (ppp_sd, PROCOP);
	switch (result) {
	    default:
	    case NOTOK:
		ros_result (ppp_sd, deliverystate, rox_id);
		ppp_lose ();
		return NOTOK;

	    case OK:
		if (ppp_error_status == NOTOK) {
			ros_result (ppp_sd, deliverystate, rox_id);
			return ppp_getnextmessage (host, net);
		}
		if (jnt_host)
			free (jnt_host);
		*host = jnt_host = mk_jnt(cur_host);
		if (jnt_host == NULLCP)
			return NOTOK;
		if (cur_net)
			free (cur_net);
		if (mychan -> ch_out_info == NULLCP) {
			PP_OPER (NULLCP, ("Channel %s has no info string",
					  mychan -> ch_name));
			return NOTOK;
		}
		*net = cur_net = strdup (mychan -> ch_out_info);
		return OK;

	    case DONE:
		return DONE;
	}
}

static char	*hdrbuf;

int ppp_getdata (buf, len)
char	**buf;
int	*len;
{
	if (hdrbuf) {
		free (hdrbuf);
		hdrbuf = NULLCP;
	}

	switch (state) {
	    case jnthdr:
		data_bytes = 0;
		timer_start (&data_timer);
		state = initialise;
		return assemble_jntheader (buf, len);

	    case initialise:
	    case go:
		return fetch_data (buf, len);

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Bad state in getdata"));
		return NOTOK;
	}
}

static int assemble_jntheader (buf, lengthp)
char **buf;
int *lengthp;
{
	ADDR	*ap;
	char	*str;
	int	len = 0;

	PP_NOTICE (("Originator %s", ad_sender -> ad_r822adr));
	for (ap = ad_list; ap; ap = ap -> ad_next) {
		if ((str = bigendian (ap -> ad_r822adr)) == NULLCP)
			return NOTOK;
		if (hdrbuf == NULL) {
			hdrbuf = smalloc (len = strlen (str) + 2 + 1);
			(void) strcpy (hdrbuf, str);
			(void) strcat (hdrbuf, ap -> ad_next ? ",\n" : "\n\n");
		}
		else {
			hdrbuf = realloc (hdrbuf, len += strlen (str) + 2);
			(void) strcat (hdrbuf, str);
			(void) strcat (hdrbuf, ap -> ad_next ? ",\n" : "\n\n");
		}
	}
	*buf = hdrbuf;
	*lengthp = len - 1;
	data_bytes += len - 1;
	return OK;
}

static char transbuf[BUFSIZ];
static char	*formatdir;

static int fetch_data (buf, lengthp)
char	**buf;
int	*lengthp;
{
	char filename[BUFSIZ];
	static FILE *fp;
	static int first;
	int	n;

	*buf = "";
	*lengthp = 0;

	if (state == initialise) {
		state = go;
		first = 1;

		if (qid2dir (this_msg, ad_list, TRUE, &formatdir) == NOTOK)
			return NOTOK;
		if (msg_rinit (formatdir) != RP_OK)
			return NOTOK;
		if (msg_rfile (filename) != RP_OK) {
			msg_rend ();
			return NOTOK;
		}
		if ((fp = fopen (filename, "r")) == NULL) {
			msg_rend ();
			return NOTOK;
		}
	}

	if (!feof(fp) && (n = fread (transbuf, 1, sizeof transbuf, fp)) > 0) {
		*buf = transbuf;
		*lengthp = n;
		data_bytes += n;
		return OK;
	}
	if (first) {
		first = 0;
		(void) strcpy (transbuf, "\n");
		*buf = transbuf;
		*lengthp = 1;
		data_bytes += 1;
		return OK;
	}
	
	if (ferror (fp)) {
		msg_rend ();
		(void) fclose (fp);
		return NOTOK;
	}
	(void) fclose (fp);
	
		
	if ((n = msg_rfile (filename)) == RP_DONE){
		msg_rend ();
		*lengthp = 0;
		return DONE;
	}

	if (rp_isbad (n)) {
		msg_rend ();
		return NOTOK;
	}

	if ((fp = fopen (filename, "r")) == NULL) {
		msg_rend ();
		return NOTOK;
	}

	return fetch_data (buf, lengthp);
}

int ppp_status (status, reason)
int	status;
char	*reason;
{
	ADDR	*ap;
	char	buf[BUFSIZ];
	int	confrep = int_Qmgr_status_positiveDR;
	int	drrep = int_Qmgr_status_negativeDR;

	if (status == PPP_STATUS_DONE)
		timer_end (&data_timer, data_bytes, "Data Transfered");

	for (ap = ad_list; ap; ap = ap -> ad_next) {
		if (ap -> ad_resp == 0)
			continue;
		switch (status) {
		    case PPP_STATUS_DONE:
			if (ap ->ad_usrreq == AD_USR_CONFIRM ||
			    ap -> ad_mtarreq == AD_MTA_CONFIRM ||
			    ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
				(void) sprintf (buf, "Successfully sent to %s for recipient %s",
						cur_host, ap -> ad_r822adr);
				set_1dr (&Qs, ap -> ad_no, this_msg,
					 DRR_NO_REASON,
					 -1, buf);
				delivery_set (ap -> ad_no, confrep);
				confrep = int_Qmgr_status_successSharedDR;
			}
			else {
				(void) wr_ad_status (ap, AD_STAT_DONE);
				(void) wr_stat (ap, &Qs, this_msg, data_bytes);
				delivery_set (ap -> ad_no,
					      int_Qmgr_status_success);
			}
			PP_NOTICE ((">>> Message %s transfered to %s on %s", 
					this_msg, ap -> ad_r822adr, cur_host));
			break;

		    case PPP_STATUS_CONNECT_FAILED:
			delivery_setstate (ap -> ad_no,
					   int_Qmgr_status_mtaFailure,
					   reason);
			PP_NOTICE (("Connection to %s failed [%s]",
					cur_host, reason ? reason : ""));
			break;

		    case PPP_STATUS_PERMANENT_FAILURE:
			(void) sprintf (buf, 
	"Blue book transfer to %s failed (permanent error code): \"%s\"",
					cur_host, reason);
			delivery_set (ap -> ad_no,
				      drrep);
			drrep = int_Qmgr_status_failureSharedDR;
			set_1dr (&Qs, ap -> ad_no, this_msg,
				 DRR_TRANSFER_FAILURE,
				 DRD_UNRECOGNISED_OR, buf);
			PP_NOTICE (("Transfer to %s for %s perm failed [%s]",
				    cur_host, this_msg, buf));
			break;

		    case PPP_STATUS_TRANSIENT_FAILURE:
			delivery_setstate (ap -> ad_no,
					   int_Qmgr_status_messageFailure,
					   reason);
			PP_NOTICE (("Temporary failure for %s to %s [%s]",
				    this_msg, cur_host, reason ? reason : ""));
			break;
		}
	}
	if (rp_isbad(wr_q2dr (&Qs, this_msg))) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure",mychan->ch_name));
		(void) delivery_resetDRs (int_Qmgr_status_messageFailure);
	}

	cleanup_everything ();
	rd_end ();
#if PP_DEBUG > PP_DEBUG_NONE
	if (ppp_debug){
		 pdeliverystatus (deliverystate);
		 return OK;
	 }
	else
#endif
		return ros_result (ppp_sd, deliverystate, rox_id);
}

void ppp_terminate ()
{
	if (ppp_sd != NOTOK)
		ppp_lose ();
}

static void ppp_lose ()
{
	struct AcSAPindication  acis;
	struct RoSAPindication  rois;
	register struct RoSAPindication *roi = &rois;

	(void) AcUAbortRequest (ppp_sd, NULLPEP, 0, &acis);
	(void) RyLose (ppp_sd, roi);
	ppp_sd = NOTOK;
}

static int ppp_initfnx (sd, ryo, rox, in, roi)
int	sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	char	*p;

        if (rox -> rox_nolinked == 0) {
                PP_LOG (LLOG_EXCEPTIONS,
                        ("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
                        sd, ryo -> ryo_name, rox -> rox_linkid));
                return ureject (sd, ROS_IP_LINKED, rox, roi);
        }
        PP_DBG (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	p = qb2str ((struct type_Qmgr_Channel *) in);

	if ((mychan = ch_nm2struct (p)) == NULLCHAN) {
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown channel %s", p));
		free (p);
		error (sd, error_Qmgr_protocol, (caddr_t) NULL, rox -> rox_id);
		return NOTOK;
	}
	PP_NOTICE (("Starting %s (%s)", mychan -> ch_name, mychan -> ch_show));
	rename_log (mychan -> ch_name);

	free (p);

	if (RyDsResult (sd, rox -> rox_id, (caddr_t) NULL,
			ROS_NOPRIO, roi) == NOTOK) {
		ros_advise (&roi -> roi_preject, "RESULT");
		return NOTOK;
	}
	
	lastop = INITOP;
	return OK;
}

static int ppp_procfnx (sd, ryo, rox, in, roi)
int	sd;
struct RyOperation *ryo;
struct RoSAPinvoke *rox;
caddr_t in;
struct RoSAPindication *roi;
{
	struct type_Qmgr_ProcMsg *arg = (struct type_Qmgr_ProcMsg *)in;

        if (rox -> rox_nolinked == 0) {
                PP_LOG (LLOG_EXCEPTIONS,
                        ("RO-INVOKE.INDICATION/%d: %s, unknown linkage %d",
                        sd, ryo -> ryo_name, rox -> rox_linkid));
                return ureject (sd, ROS_IP_LINKED, rox, roi);
        }
        PP_DBG (("RO-INVOKE.INDICATION/%d: %s", sd, ryo -> ryo_name));

	rox_id = rox -> rox_id;
	lastop = PROCOP;
	return ppp_procfnx_aux (arg);
}

static int ppp_procfnx_aux (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct type_Qmgr_UserList *up;
	int	retval;
	int	ad_count = 0;
	ADDR	*ap, *alp = NULL;

	delivery_init (arg -> users);

	if (this_msg)
		free (this_msg);
	this_msg = qb2str (arg -> qid);

	ad_sender = NULLADDR;

	if (ad_recip) {
		q_free (&Qs);
		prm_free (&prm);
	}
	ad_recip = NULLADDR;

	retval = rd_msg (this_msg, &prm, &Qs, &ad_sender, &ad_recip, &ad_count);
	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("local/rd_msg err: %s", this_msg));
		return NOTOK;
	}

	naddrs = 0;
	ad_list = NULLADDR;
	ppp_error_status = OK;
	for (ap = ad_recip; ap; ap = ap -> ad_next) {
                for (up = arg ->users; up; up = up -> next) {
                        if (up -> RecipientId -> parm != ap -> ad_no)
                                continue;

                        switch (chan_acheck (ap, mychan, ad_list == NULLADDR, 
						&cur_host)) {
				case OK:
					break;

				default:
					continue;
			}
			break;
                }
                if (up == NULL)
                        continue;

                if (ad_list == NULLADDR)
                        ad_list = alp = (ADDR *) calloc (1, sizeof *alp);
                else {
                        alp -> ad_next = (ADDR *) calloc (1, sizeof *alp);
                        alp = alp -> ad_next;
                }
                *alp = *ap;
                alp -> ad_next = NULLADDR;
		naddrs ++;
        }

	if (ad_list == NULLADDR) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipients in user list"));
		rd_end ();
		ppp_error_status = NOTOK;
		return OK;
	}
	PP_NOTICE (("Processing msg %s to %s", this_msg, cur_host));
	return OK;
}


static int  ros_work (fd, what)
int     fd;
int	what;
{
	int     result;
	caddr_t out;
	struct RoSAPindication  rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject   *rop = &roi -> roi_preject;
	fd_set rfds;

	for (;;) {
		PP_TRACE (("ros_work loop (%d, %d)", fd, what));
		FD_ZERO (&rfds);
		FD_SET (fd, &rfds);
		xselect (fd + 1, &rfds, NULLFD, NULLFD, NOTOK);
		switch (result = RyWait (fd, NULLIP, &out, OK, roi)) {
		    case NOTOK: 
			if (rop -> rop_reason == ROS_TIMER)
				break;
		    case OK: 
		    case DONE: 
			if ((result = ros_indication (fd, roi)) != OK)
				return result;
			break;

		    default: 
			PP_LOG (LLOG_EXCEPTIONS,
				("unknown return from RoWaitRequest=%d",
				 result));
			return NOTOK;
		}
		break;
	}
	if (lastop == what)
		return OK;
	return NOTOK;
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
		PP_LOG (LLOG_EXCEPTIONS,
			("unexpected indication type=%d", roi -> roi_type));
		return NOTOK;

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
			return NOTOK;
		}

	    case ROI_PREJECT: 
		{
			register struct RoSAPpreject   *rop =
				&roi -> roi_preject;

			ros_advise (rop, "RO-REJECT-P.INDICATION");
			return NOTOK;
		}

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

			return DONE;
		}
		/* NOTREACHED */

	    default: 
		PP_LOG (LLOG_EXCEPTIONS,
			("unknown indication type=%d", roi -> roi_type));
		return NOTOK;
	}
	return OK;
}

static int ros_result (sd, val, id)
int     sd;
struct type_Qmgr_DeliveryStatus *val;
int	id;
{
	struct RoSAPindication rois;
	struct RoSAPindication *roi = &rois;

	if (val == NULL)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      id);

	if (RyDsResult (sd, id, (caddr_t) val, ROS_NOPRIO, roi)
			== NOTOK) {
		ros_advise (&roi -> roi_preject, "RESULT");
		return NOTOK;
	}
	return OK;
}

/*    ERRORS */

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
    if (ROS_FATAL (rop -> rop_reason))
	    exit (1);
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

/*    ERROR */

static int  error (sd, err, param, id)
int     sd,
	err;
caddr_t param;
int	id;
{
	struct RoSAPindication rois;
	struct RoSAPindication *roi = &rois;

	if (RyDsError (sd, id, err, param, ROS_NOPRIO, roi) == NOTOK) {
		ros_advise (&roi -> roi_preject, "ERROR");
		return NOTOK;
	}

	return OK;
}

static int  ureject (sd, reason, rox, roi)
int     sd,
	reason;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	if (RyDsUReject (sd, rox -> rox_id, reason, ROS_NOPRIO, roi) == NOTOK) {
		ros_advise (&roi -> roi_preject, "U-REJECT");
		return NOTOK;
	}

	return OK;
}

static char *bigendian (addr)
char    *addr;
{
	extern int ap_outtype;
	static char     *newaddr = NULLCP;
	AP_ptr          tree = NULLAP,
			group = NULLAP,
			name = NULLAP,
			local = NULLAP,
			domain = NULLAP,
			route = NULLAP;

	if (newaddr) {
		free (newaddr);
		newaddr = NULLCP;
	}


	ap_outtype = AP_PARSE_733 | AP_PARSE_BIG;


	if (ap_s2p  (addr, &tree, &group, &name, &local, &domain, &route)
	    == (char *)NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Unable to parse (s2p) Recipient addr %s", addr));
		return NULLCP;
	}


	if ((newaddr = ap_p2s (group, name, local, domain, route))
	    == (char *)NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Unable to parse (p2s) Recipient addr %s", addr));
		newaddr = NULLCP;
	}

	ap_free (tree);

	PP_NOTICE (("Recipient Address '%s'",  newaddr));
	return newaddr;
}

#if PP_DEBUG > PP_DEBUG_NONE
static int debug_init ()
{
	char	buf[BUFSIZ], *p;

	fprintf (stderr, "Running in debug mode\nInput channel name: ");
	ppp_debug = 1;
	(void) fflush (stderr);

	if (fgets (buf, sizeof buf, stdin) == NULL)
		return NOTOK;
	if ((p = index (buf, '\n')) != NULLCP)
		*p = '\0';
	if ((mychan = ch_nm2struct (buf)) == NULLCHAN) {
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown channel %s", p));
		return NOTOK;
	}
	return OK;
}	

static int debug_nextmsg ()
{
	char 	buf[BUFSIZ], *p;
	int	num;
	struct type_Qmgr_ProcMsg *arg;
	struct type_Qmgr_UserList *temp, *tail = NULL;

	fprintf (stderr, "Input message name: ");
	(void) fflush (stderr);

	if (fgets (buf, sizeof buf, stdin) == NULL)
		return NOTOK;
	if ((p = index (buf, '\n')) != NULLCP)
		*p = '\0';
	arg = (struct type_Qmgr_ProcMsg *) calloc (1, sizeof *arg);
	arg -> qid = str2qb (buf, strlen (buf), 1);
	arg -> channel = str2qb (mychan -> ch_name, 
				strlen (mychan -> ch_name), 1);
	fprintf (stderr, "Input recipients terminated by -1: ");
	fflush (stderr);

	do {
		scanf ("%d", &num);
		if (num != -1) {
			temp = (struct type_Qmgr_UserList *)
				calloc (1, sizeof *temp);
			temp -> RecipientId = (struct type_Qmgr_RecipientId *)
				calloc (1, sizeof *temp->RecipientId);
			temp -> RecipientId -> parm = num;
			if (arg -> users) {
				tail -> next = temp;
				tail = temp;
			}
			else
				arg -> users = tail = temp;
		}
	} while (num != -1);
	while ((num = getchar()) != EOF && num != '\n')
		continue;
	return ppp_procfnx_aux (arg);
}

static void pdeliverystatus (status)
struct type_Qmgr_DeliveryStatus *status;
{
        fprintf (stderr, "Delivery status\n");
        if (status == NULL)
                fprintf (stderr, "Complete failure\n");
        else {
                struct type_Qmgr_DeliveryStatus *ix = status;
                while (ix != NULL)
                {
                        pindividualdeliverystatus(ix->IndividualDeliveryStatus);
                        ix = ix->next;
                }
        }
}

static void pindividualdeliverystatus(status)
struct type_Qmgr_IndividualDeliveryStatus *status;
{
        fprintf (stderr, "Recipient %d: ", status->recipient->parm);

        switch (status->status) {
            case int_Qmgr_status_success:
                fprintf (stderr, "success");
                break;
            case int_Qmgr_status_successSharedDR:
                fprintf (stderr, "successSharedDR");
                break;
            case int_Qmgr_status_failureSharedDR:
                fprintf (stderr, "failureSharedDR");
                break;
            case int_Qmgr_status_negativeDR:
                fprintf (stderr, "negativeDR");
                break;
            case int_Qmgr_status_positiveDR:
                fprintf (stderr, "positiveDR");
                break;
            case int_Qmgr_status_messageFailure:
                fprintf (stderr, "message failure");
                break;
            case int_Qmgr_status_mtaFailure:
                fprintf (stderr, "mta failure");
                break;
            case int_Qmgr_status_mtaAndMessageFailure:
                fprintf (stderr, "mta and message failure");
                break;
            default:
                fprintf (stderr, "unknown");
                break;
        }
        putc ('\n', stderr);
}

#endif

static void cleanup_everything ()
{
	if (hdrbuf != NULL) {
		free (hdrbuf);
		hdrbuf = NULL;
	}
	lastop = NOTOK;
}

static char	*mk_jnt (str)
char	*str;
{
	char	buf1[BUFSIZ], buf2[BUFSIZ];
	char	*argv[30];
	int	argc;

	strcpy (buf1, str);
	argc = sstr2arg (buf1, 30, argv, ".");
	
	if (argc < 1)
		return NULLCP;
	strcpy (buf2, argv[--argc]);
	while (--argc >= 0) {
		strcat (buf2, ".");
		strcat (buf2, argv[argc]);
	}
	return strdup (buf2);
}
