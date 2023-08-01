/* x400out88: x400 out bound channel for 1988 stuff */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400out88.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400out88.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: x400out88.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */


#include "Trans-types.h"
#include "head.h"
#include "prm.h"
#include "q.h"
#include "dr.h"
#include "qmgr.h"
#include "rtsparams.h"
#include <sys/stat.h>
#include "sys.file.h"
#include <isode/rtsap.h>
#include <isode/isoservent.h>


static enum { st_init, st_hdr, st_bodyinit, st_body, st_end } trans_state;


/* -- defines -- */
#define MAXTRIES	10
#define STR2QB(s)	str2qb (s, strlen (s), 1)


/* -- externals -- */
extern int
	build_p1(),
	build_dr(),
	build_probe ();
extern char
	*loc_dom_site,
	ub_error_string[],
	*quedfldir;
extern int
	ub_error_set;


/* -- globals -- */
static ADDR		*ad_list;
static CHAN		*mychan;
static FILE		*body_fp;	
static char		*chan_name = NULLCP;
static char		*connected_to_site = NULLCP;
static char		*current_mta = NULLCP;
static char		*dumpp1 = NULLCP;
static char		*fix_orig;
static char		*myname;
static char		*p1_ptr;
static char		*p1_string;
static char		*this_msgid = NULLCP;
static int		conn_type = NOTOK;
static int		data_bytes;
static int		mctx_id;
static int		p1_length = 0;
static int		trace_type;
static struct timeval	data_timer;
int			rts_sd = NOTOK;
static int		RTS_PING_88 = FALSE;


/* -- queue variables -- */
static Q_struct		Qstruct;
Q_struct		*PPQuePtr = &Qstruct;
static DRmpdu		DRstruct;
DRmpdu			*DRptr = &DRstruct;
static struct prm_vars	PRMstruct;


/* -- local routines -- */
static char		*pe_flatten();
static int		attempt_connect();
static int		check_params();
static void		close_body();
static int		construct_dr();
static int		construct_msg();
static int		construct_probe();
static int		deliver();
static void		dirinit();
static int		dump2file();
static int		endproc();
static int		get_more_message();
static int		initproc();
static void		open_body();
static int		read_body();
static int		rts_downtrans_all();
static int		rts_downtrans_inc();
static int		rts_end();
static int		rts_start();
static int		rts_start84();
static int		rts_start88();
static int		rts_transfer_request();
static int		rtsping88();
static struct type_MTA_Content		*get_dr_content();
static struct type_MTA_MTABindArgument	*convert_prms();
static struct type_Qmgr_DeliveryStatus	*process();




/* ---------------------  Begin	 Routines  -------------------------------- */



main (argc, argv)
int	argc;
char	**argv;
{
	if (myname = rindex (argv[0], '/'))
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = argv[0];


	/* -- initialise -- */
	chan_init (myname);


	/* -- rts88 ping required ? -- */
	if (argc > 1 && strcmp (argv[1], "ping") == 0)
		rtsping88 (argc - 1, argv + 1);


	/* -- get ready to transmit x400out88 messages -- */
	or_myinit();
	dirinit();


#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0) {
		if (argc > 2)
			dumpp1 = argv[2];
		debug_channel_control (argc, argv, initproc, process, endproc);
	} else
#endif
		channel_control (argc, argv, initproc, process, endproc);

	exit (0);
	/* NOTREACHED */
}




/* ---------------------  Static  Routines  ------------------------------- */




static void dirinit()	/* -- Change into pp queue space -- */
{
	PP_TRACE (("dirinit()"));
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, "Unable to change to dir '%s'", quedfldir);
}




static int initproc (arg)
struct type_Qmgr_Channel	*arg;
{

	if (!RTS_PING_88)  chan_name = qb2str (arg);

	PP_TRACE (("initproc (%s)", chan_name));

	if ((mychan = ch_nm2struct (chan_name)) == NULLCHAN)
		adios (NULLCP, "Channel '%s' not known", chan_name);

	rename_log (chan_name);
	free (chan_name);
	chan_name = NULLCP;

	PP_NOTICE (("Starting %s (%s)", mychan->ch_name, mychan->ch_show));

	if (RTS_PING_88)  return OK;

	prm_init (&PRMstruct);
	q_init (PPQuePtr);
	dr_init (&DRstruct);
	if (current_mta)
		free (current_mta);
	current_mta = NULLCP;
	return OK;
}




static int endproc()
{
	switch (rts_end()) {
	    case OK:
		PP_NOTICE (("Connection successfully terminated"));
		break;
	    case NOTOK:
		PP_NOTICE (("Connection badly terminated"));
		break;
	    case DONE:
		PP_NOTICE (("Connection not made"));
		break;
	}
	return OK;
}




static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg		*arg;
{
	struct type_Qmgr_UserList	*up;
	ADDR				*ad_sender = NULLADDR,
					*asp,
					*ad_recip  = NULLADDR,
					*alp	   = NULLADDR,
					*ap	   = NULLADDR;
	int	naddrs	   = 0,
		ad_count,
		retval;

	if (this_msgid)
		free (this_msgid);
	this_msgid = qb2str (arg->qid);

	PP_NOTICE (("Reading message '%s'", this_msgid));

	delivery_init (arg->users);

	/* -- queue initialisation - frees memory if called many times -- */
	q_free (PPQuePtr);
	dr_free (&DRstruct);
	prm_free (&PRMstruct);


	retval = rd_msg (this_msgid, &PRMstruct, PPQuePtr,
			 &ad_sender, &ad_recip, &ad_count);


	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("rd_msg %s", this_msgid));
		(void) rd_end ();
		return (delivery_setallstate (int_Qmgr_status_messageFailure,
					      "Can't read message"));
	}


	for (asp = ad_sender, ap = ad_recip; ap; ap = ap -> ad_next) {

		for (up = arg->users; up; up = up->next) {

			PP_TRACE (("'%s' '%s' ad_no=%d up_no=%d",
				this_msgid, ap->ad_value, ap->ad_no,
				up->RecipientId->parm));

			if (up -> RecipientId -> parm != ap -> ad_no)
				continue;

			if (dchan_acheck (ap, asp, mychan, naddrs == 0,
					  &current_mta) == NOTOK) {
				continue;
			}
			naddrs ++;
			break;

		}

		if (up == NULL)
			continue;

		if (ad_list == NULLADDR)
			ad_list = alp = (ADDR *)calloc(1, sizeof *alp);
		else {
			alp -> ad_next = (ADDR *)calloc(1, sizeof *alp);
			alp = alp -> ad_next;
		}

		*alp = *ap;	/* struct copy */
		alp -> ad_next = NULLADDR;

	}

	if (naddrs == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("No recips to be processed in specified user list"));
		rd_end ();
		return deliverystate;
	}


	deliver (PPQuePtr, ad_list); /* deliverystate set in deliver */

	for (alp = ad_list; alp; alp = ap) {
		ap = alp -> ad_next;
		free ((char *)alp);
	}
	ad_list = NULLADDR;

	rd_end();

	return deliverystate;
}




static int deliver (qp, recip)
Q_struct *qp;
ADDR	*recip;
{
	int	retval;
	int	value;
	ADDR	*ap;
	int msgtype;

	switch (recip->ad_status) {
	    case AD_STAT_PEND:
	    case AD_STAT_DRWRITTEN:
		break;
	    case AD_STAT_DONE:
		delivery_setall (int_Qmgr_status_success);
		return NOTOK;
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("bad state"));
		delivery_setallstate (int_Qmgr_status_messageFailure,
				      "bad state");
		return NOTOK;
	}
	if (attempt_connect () != OK)
		return NOTOK;

	msgtype = Qstruct.msgtype;
	if (fix_orig)
		x400_fixorig (Qstruct.Oaddress, fix_orig);

	if (recip -> ad_status == AD_STAT_DRWRITTEN ||
	    Qstruct.msgtype == MT_DMPDU) {
		retval = construct_dr( &Qstruct, recip);
		msgtype = MT_DMPDU;
	}
	else if (Qstruct.msgtype == MT_PMPDU)
		retval = construct_probe (&Qstruct, recip);
	else
		retval = construct_msg (&Qstruct, recip);

	if (retval != OK) {
		delivery_setallstate (int_Qmgr_status_messageFailure,
				 "Can't build message");
		return NOTOK;
	}
		
	PP_NOTICE (("Sender '%s'", Qstruct.Oaddress->ad_r400adr));

	for (ap = recip; ap; ap = ap -> ad_next)
		PP_NOTICE (("Recipient Address '%s'",  ap->ad_r400adr));

	switch (rts_transfer_request(msgtype)) {
	    case OK:
		value = int_Qmgr_status_success;
		break;
	    case NOTOK:
		if (rts_sd == NOTOK)
			value = int_Qmgr_status_mtaAndMessageFailure;
		else
			value = int_Qmgr_status_messageFailure;
		break;
	    case DONE:
		value = int_Qmgr_status_negativeDR;
		break;
	}

	free (p1_string);

	for (ap = recip; ap; ap = ap -> ad_next) {
		if (ap -> ad_resp) {
			delivery_set (ap -> ad_no, value);
			switch (value) {
			    case int_Qmgr_status_success:
				wr_ad_status (ap, AD_STAT_DONE);
				wr_stat (ap, qp, this_msgid, data_bytes);
				break;
			    case int_Qmgr_status_negativeDR:
				value = int_Qmgr_status_failureSharedDR;
			    case int_Qmgr_status_failureSharedDR:
				set_1dr (qp, ap -> ad_no, this_msgid,
					 DRR_UNABLE_TO_TRANSFER,
					 DRD_MTA_CONGESTION,
					 "Remote site aborted the transfer");
				PP_LOG (LLOG_EXCEPTIONS,
					("Remote site aborted the connection"));
				break;
			}
		}
	}
	if (rp_isbad(wr_q2dr (qp, this_msgid)))
		delivery_resetDRs (int_Qmgr_status_messageFailure);
	return OK;

}




static int attempt_connect ()
{
	if (connected_to_site == NULLCP || rts_sd == NOTOK) {
		if (rts_start() != OK) {
			delivery_setallstate (int_Qmgr_status_mtaFailure,
					 "Connection failed");
			return NOTOK;
		}
		if (connected_to_site) free (connected_to_site);
		connected_to_site = strdup (current_mta);
	}
	else if (lexequ (connected_to_site, current_mta) != 0) {
		free (connected_to_site);
		connected_to_site = NULLCP;
		(void) rts_end();
		if (rts_start() != OK) {
			delivery_setallstate (int_Qmgr_status_mtaFailure,
					 "Connection failed");
			return NOTOK;
		}
		connected_to_site = strdup (current_mta);
	}
	return OK;
}




static int rts_start ()
{
	RtsParams	*rp;
	int	retcode, i;

	for (i = 0; i < MAXTRIES; i++) {
		if ((rp = tb_rtsparams (mychan -> ch_table, current_mta))
		    == NULL) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't find connect information for %s",
				 current_mta));
			return NOTOK;
		}

		conn_type = rp -> type;
		trace_type = rp -> trace_type;
		if (conn_type == RTSP_1984 || conn_type == RTSP_1988_X410MODE)
			retcode = rts_start84 (rp);
		else
			retcode = rts_start88 (rp);
		switch (retcode) {
		    case NOTOK:
			if (rp -> try_next) {
				free (current_mta);
				current_mta = strdup (rp -> try_next);
				continue;
			}
			break;
		    case OK:
			if (fix_orig)
				free (fix_orig);
			if (rp -> fix_orig)
				fix_orig = strdup (rp -> fix_orig);
			else fix_orig = NULLCP;
			RPfree (rp);
			break;
		}
		break;
	}
	return retcode;
}




static int rts_start84 (rp)
RtsParams	*rp;
{
	struct type_MTA_MTABindArgument *mtabind;
	struct type_MTA_MTABindResult *mtaresult;
	PE	pe;
	struct RtSAPaddr	rtsapto, rtsapfrom;
	struct RtSAPaddr	*rtto = &rtsapto, *rtfrom = &rtsapfrom;
	struct RtSAPconnect	rtcs, *rtc = &rtcs;
	struct RtSAPindication	rtis, *rti = &rtis;
	struct RtSAPabort	*rta = & rti -> rti_abort;
	struct SSAPaddr		*sa;


	if ((sa = str2saddr (rp -> their_address)) == NULLSA) {
		PP_LOG (LLOG_EXCEPTIONS, ("BAD address in table, %s for %s",
					  rp -> their_address, current_mta));
		return NOTOK;
	}
	rtto -> rta_addr = *sa; /* struct copy */
	rtto -> rta_port = rp -> type == RTSP_1984 ? 1 : 12;

	if (rp -> our_address) {
		if ((sa = str2saddr (rp -> our_address)) == NULLSA) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Bad address in table, %s for us!",
				 rp -> our_address));
			return NOTOK;
		}
		rtfrom -> rta_addr = *sa; /* struct copy */
		rtfrom -> rta_port = rtto -> rta_port;
	}
	else	rtfrom = NULL;

	if ((mtabind = convert_prms (rp)) == NULL)
		return NOTOK;

	if (encode_MTA_MTABindArgument (&pe, 1, 0, NULLCP, mtabind) == NOTOK) {
		free_MTA_MTABindArgument (mtabind);
		return NOTOK;
	}

	PP_PDUP (MTA_MTABindArgument, pe, "MTA.BindArgument", PDU_WRITE);

	PP_NOTICE (("Connecting (RTS 84 mode) to site %s", current_mta));

	if (RtBeginRequest2 (rtto, rtfrom, rp->rts_mode, RTS_INITIATOR,
			     pe, rtc, rti) == NOTOK) {
		rts_advise (rta, "RT-BEGIN.REQUEST");
		RTAFREE (rta);
		pe_free (pe);
		free_MTA_MTABindArgument (mtabind);
		return NOTOK;
	}

	pe_free (pe);
	free_MTA_MTABindArgument (mtabind);

	if (rtc -> rtc_result != RTS_ACCEPT) {
		PP_LOG (LLOG_EXCEPTIONS, ("Association rejected: [%s]",
					  RtErrString (rtc -> rtc_result)));
		free_MTA_MTABindResult (mtabind);
		RTCFREE (rtc);
		return NOTOK;
	}


	PP_PDUP (MTA_MTABindResult, rtc -> rtc_data,
		"MTA.BindResult", PDU_READ);

	if (decode_MTA_MTABindResult (rtc -> rtc_data, 1, NULLIP,
				      NULLVP, &mtaresult)
	    == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't decode result [%s]", PY_pepy));
		RTCFREE (rtc);
		return NOTOK;
	}


	rts_sd = rtc -> rtc_sd;

	RTCFREE (rtc);

	if (check_params (rp, mtaresult) != OK) {
		rts_end ();
		rts_sd = NOTOK;
		return NOTOK;
	}

	PP_NOTICE (("Connected sucessfully"));
	return OK;
}




static int rts_start88 (rp)
RtsParams *rp;
{
	struct type_Trans_Bind1988Argument *mtabind;
	struct type_Trans_Bind1988Result *bindresult;
	PE	pe;
	struct PSAPaddr	pa_tos, pa_froms;
	struct PSAPaddr *pa_to, *pa_from;
	struct PSAPctxlist pcs;
	struct PSAPctxlist *pc = &pcs;
	OID	a_ctx, r_ctx, m_ctx, t_ctx;
	struct RtSAPconnect rtcs, *rtc = &rtcs;
	struct RtSAPindication rtis, *rti = &rtis;
	struct RtSAPabort *rta = &rti -> rti_abort;
	int	n;

	if ((mtabind = convert_prms (rp)) == NULL)
		return NOTOK;

	if (encode_Trans_Bind1988Argument (&pe, 1, 0,
					   NULLCP, mtabind) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("encode failed [%s]", PY_pepy));
		free_Trans_Bind1988Argument (mtabind);
		return NOTOK;
	}

	PP_PDUP (Trans_Bind1988Argument, pe,
		"MTA.BindArgument(1988)", PDU_WRITE);

	PP_NOTICE (("Connecting (RTS 88 mode) to site %s", current_mta));

	if ((pa_to = str2paddr (rp -> their_address)) == NULLPA) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't translate %s",
					  rp -> their_address));
		free_Trans_Bind1988Argument (mtabind);
		pe_free (pe);
		return NOTOK;
	}
	pa_tos = *pa_to;	/* struct copy */
	pa_to = &pa_tos;

	if (rp -> our_address) {
		if ((pa_from = str2paddr (rp -> our_address)) == NULLPA) {
			PP_LOG (LLOG_EXCEPTIONS, ("Can't translate %s",
						  rp -> our_address));
			free_Trans_Bind1988Argument (mtabind);
			pe_free (pe);
			return NOTOK;
		}
		pa_froms = *pa_from;
		pa_from = &pa_froms;
	}
	else
		pa_from = NULLPA;
		
#define aCSE	"2.2.1.0.1"
#define rTSE	"2.6.0.2.12"
#define mTSE	"2.6.0.2.7"
#define tRANSFER "2.6.0.1.6"
	n = 1;
	if ((a_ctx = str2oid (aCSE)) == NULLOID)
		adios (NULLCP, "No %s object defined", aCSE);
	a_ctx = oid_cpy (a_ctx);
	pc -> pc_ctx[n-1].pc_id = 2 * n - 1;
	pc -> pc_ctx[n-1].pc_asn = oid_cpy (a_ctx);
	pc -> pc_ctx[n-1].pc_atn = NULLOID;
	n ++;

	if ((r_ctx = str2oid (rTSE)) == NULLOID)
		adios (NULLCP, "No %s object defined", rTSE);
	r_ctx = oid_cpy (r_ctx);
	pc -> pc_ctx[n-1].pc_id = 2 * n - 1;
	pc -> pc_ctx[n-1].pc_asn = r_ctx;
	pc -> pc_ctx[n-1].pc_atn = NULLOID;
	n ++;

	if ((m_ctx = str2oid (mTSE)) == NULLOID)
		adios (NULLCP, "No %s object defined", mTSE);
	m_ctx = oid_cpy (m_ctx);
	pc -> pc_ctx[n-1].pc_id = mctx_id = 2 * n - 1;
	pc -> pc_ctx[n-1].pc_asn = m_ctx;
	pc -> pc_ctx[n-1].pc_atn = NULLOID;

	pc -> pc_nctx = n;
	
	if ((t_ctx = str2oid (tRANSFER)) == NULLOID)
		adios (NULLCP, "No %s object defined", tRANSFER);
	t_ctx = oid_cpy (t_ctx);

	if (RtOpenRequest2 (rp -> rts_mode, RTS_INITIATOR, t_ctx,
			    NULLAEI, NULLAEI,
			    pa_from, pa_to, pc, NULLOID, pe, NULLQOS,
			    oid_cpy (r_ctx), rtc, rti) == NOTOK) {
		rts_advise (rta, "RT-OPEN.REQUEST");
		RTAFREE(rta);
		return NOTOK;
	}

	if (rtc -> rtc_result != RTS_ACCEPT) {
		PP_LOG (LLOG_EXCEPTIONS, ("Association rejected: [%s]",
					  RtErrString (rtc -> rtc_result)));
		RTCFREE (rtc);
		return NOTOK;
	}
	rts_sd = rtc -> rtc_sd;

	PP_PDUP (Trans_Bind1988Result, rtc -> rtc_data,
		"MTA.BindResult", PDU_READ);
	if (decode_Trans_Bind1988Result (rtc -> rtc_data, 1, NULLIP,
					 NULLVP, &bindresult) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't decode result [%s]", PY_pepy));
		RTCFREE (rtc);
		return NOTOK;
	}

	if (check_params (rp, bindresult) != OK) {
		RTCFREE (rtc);
		free_Trans_Bind1988Result (bindresult);
		rts_end ();
		return NOTOK;
	}
	RTCFREE (rtc);
	free_Trans_Bind1988Result (bindresult);
	PP_NOTICE (("Connected Sucessfully"));
	return OK;
}




static struct type_MTA_MTABindArgument *convert_prms (rp)
RtsParams *rp;
{
	struct type_MTA_MTABindArgument *bindarg;
	struct member_MTA_11 *mp;
	struct type_MTA_InitiatorCredentials *ic;

	bindarg = (struct type_MTA_MTABindArgument *)
		smalloc (sizeof *bindarg);

	bindarg -> offset = type_MTA_MTABindArgument_auth;

	bindarg -> un.auth = mp = (struct member_MTA_11 *)
		smalloc (sizeof *mp);

	mp -> initiator__name = STR2QB (rp -> our_name);
	mp -> initiator__credentials = ic =
		(struct type_MTA_InitiatorCredentials *)
			smalloc (sizeof *ic);

	ic -> offset = type_MTA_InitiatorCredentials_octetstring;
	ic -> un.octetstring = STR2QB (rp -> our_passwd);

	mp -> security__context = NULL;

	return bindarg;
}




static int check_params (rp, bindresult)
RtsParams *rp;
struct type_MTA_MTABindResult *bindresult;
{
	char	*str;
	struct member_MTA_12 *mp;

	switch (bindresult -> offset) {
	    case type_MTA_MTABindResult_auth:
		break;
	    case type_MTA_MTABindResult_no__auth:
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Bad result type"));
		return NOTOK;
	}
	
	mp = bindresult -> un.auth;
	str = qb2str (mp -> responder__name);
	if (strcmp (str, rp -> their_name) != 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("mta name mismatch %s %s",
					  str, rp -> their_name));
		free (str);
		return NOTOK;
	}

	free (str);
	switch (mp -> responder__credentials -> offset) {
	    case type_MTA_ResponderCredentials_ia5string:
		str = qb2str (mp -> responder__credentials->un.ia5string);
		break;
	    case type_MTA_ResponderCredentials_octetstring:
		str = qb2str (mp -> responder__credentials->un.octetstring);
		break;
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Not an octet string"));
		return NOTOK;
	}
		
	if (isstr (rp -> their_passwd)) {
		if (strcmp (str, rp -> their_passwd) != 0) {
			PP_LOG (LLOG_EXCEPTIONS, ("passwd mismatch %s %s",
					  	str, rp -> their_passwd));
			free (str);
			return NOTOK;
		}
	}

	free (str);
	return OK;
}




static int rts_transfer_request (type)
int	type;
{
	struct RtSAPindication		rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort	*rta = &rti -> rti_abort;
	int retval;

	PP_TRACE (("rts_transfer_request()"));

	trans_state = st_init;

	switch (type) {
	    case MT_PMPDU:
	    case MT_DMPDU:
		if (RtSetDownTrans (rts_sd, rts_downtrans_all, rti) == NOTOK) {
			rts_advise (rta, "set DownTrans upcall");
			RTAFREE(rta);
			return NOTOK;
		}
		break;
		
	    default:
	    case MT_UMPDU:
		if (RtSetDownTrans (rts_sd, rts_downtrans_inc, rti) == NOTOK) {
			rts_advise (rta, "set DownTrans upcall");
			RTAFREE(rta);
			return NOTOK;
		}
		break;
	}
	
	data_bytes = 0;
	timer_start (&data_timer);
	if (RtTransferRequest (rts_sd, NULLPE, NOTOK, rti) == NOTOK) {
		rts_advise (rta, "RT-TRANSFER.REQUEST");
		if (RTS_FATAL(rta->rta_reason))
			rts_sd = NOTOK;
		return retval;
	}

	timer_end (&data_timer, data_bytes, "Transfer Completed");

	PP_NOTICE((">>> Message %s transfered to %s",
		   this_msgid, current_mta));

	return OK;
}




/* ARGSUSED */
static int  rts_downtrans_inc (sd, base, len, size, ssn, ack, rti)
int			sd;
char			**base;
int			*len,
			size;
long			ssn,
			ack;
struct RtSAPindication	*rti;
{
	int	cc, count;
	int	n;
	char	*ptr, *p;
	static char	*trans_buf;
	static int	bsize;
	static int resetbuf = 0;
	int	dynamic = 0;

	PP_TRACE (("rts_downtrans_inc (%d, base, len, %d, %ld, %ld, rti)",
		   sd, size, ssn, ack));

	if (base == NULLVP) {
		PP_DBG (("RT-PLEASE.INDICATION: %d", size));
		return OK;
	}

	if (resetbuf) {
		resetbuf = 0;
		free (trans_buf);
		trans_buf = NULLCP;
		bsize = 0;
	}

	if (trans_buf == NULLCP) {

		if (size == 0) { /* no checkpointing... */
			n  =  BUFSIZ * 10;
			dynamic = 1;
		}
		else
			n = size;
			
		if ((trans_buf = malloc ((unsigned) n)) == NULL)
			return (rtsaplose (rti, RTS_CONGEST,
				NULLCP, "out of memory"));

		PP_TRACE (("Selecting block size of %d", n));

		bsize = n;
	}

	if (size > 0 && size < bsize) {
		PP_LOG (LLOG_EXCEPTIONS, ("downtrans size decreased..."));
		bsize = size;
	}

	if (dynamic) {
		for (n = bsize, p = trans_buf, count = 0; ;) {
			if ((cc = get_more_message (&ptr, n)) == 0)
				break;
			bcopy (ptr, p, cc);
			p += cc;
			n -= cc;
			count += cc;

			if (n < 1000) {
				bsize += BUFSIZ * 10;
				n += BUFSIZ * 10;
				trans_buf = realloc (trans_buf,
						     (unsigned)bsize);
				p = trans_buf + count;
			}
		}

	} else {
		for (n = bsize, p = trans_buf, count = 0; n > 0;) {
			if ((cc = get_more_message (&ptr, n)) == 0)
				break;
			bcopy (ptr,  p, cc);
			p += cc;
			n -= cc;
			count += cc;
		}
	}
	if (dumpp1)
		dump2file (trans_buf, count, dumpp1);
	if (count == 0) {
		*base = NULLCP;
		*len = 0;
		free (trans_buf);
		trans_buf = NULLCP;
		dynamic = 0;
		return OK;
	}
	*base = trans_buf;
	*len = count;
	data_bytes += count;
	if (dynamic) resetbuf = 1;
	PP_TRACE (("rts_downtrans_inc read %d bytes (%d checkpoint)",
		   count, bsize));
	
	return OK;
}




static int get_more_message (dptr, dlen)
char	**dptr;
int	dlen;
{
	static char	eoc[3];
	static char *ptr;
	static int len = 0, inited = 0;
	static char buffer[BUFSIZ * 8];
	char	*cp;
	int	n;
	PE	pe;

	PP_TRACE (("get_more_message (%d)", dlen));
	if (!inited) {
		pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_UNIV_EOC);
		inited = 3;
		(void) pe_flatten (pe, eoc, 1, &inited);
		pe_free (pe);
	}
	
	if (len <= 0) {
		switch (trans_state) {
		    case st_init:
			cp = buffer; len = 0;
			if (conn_type == RTSP_1988_NORMAL) {
				pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS,
					       PE_CONS_EXTN);
				pe -> pe_len = PE_LEN_INDF;
				n = sizeof buffer;
				(void) pe_flatten (pe, cp, -1, &n);
				pe_free (pe);
				cp = buffer + n;
				len += n;

				pe = int2prim  (mctx_id);
				n = (sizeof buffer) - len;
				(void) pe_flatten (pe, cp, -1, &n);
				cp = cp + n;
				len += n;
				pe_free (pe);


				pe = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 0);
				pe -> pe_len = PE_LEN_INDF;
				n = (sizeof buffer) - len;
				(void) pe_flatten (pe, cp, -1, &n);
				cp = cp + n;
				len += n;
				pe_free (pe);
			}
			pe = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 0);
			pe -> pe_len = PE_LEN_INDF;
			n = (sizeof buffer) - len;
			(void) pe_flatten (pe, cp, -1, &n);
			len += n;
			pe_free (pe);
			trans_state = st_hdr;
			ptr = buffer;
			break;

		    case st_hdr:
			len = p1_length;
			ptr = p1_string;
			trans_state = st_bodyinit;
			break;

		    case st_bodyinit:
			pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_PRIM_OCTS);
			pe -> pe_len = PE_LEN_INDF;
			len = sizeof buffer;
			(void) pe_flatten (pe, buffer, -1, &len);
			pe_free (pe);
			ptr = buffer;
			open_body ();
			trans_state = st_body;
			break;
		
		    case st_body:
			if ((len = read_body (buffer, sizeof buffer)) != 0) {
				ptr = buffer;
				break;
			}
			close_body ();
			cp = buffer;
			len = 0;
			if (conn_type == RTSP_1988_NORMAL) {
				bcopy (eoc, cp, 2);
				cp += 2; len += 2;
				bcopy (eoc, cp, 2);
				cp += 2; len += 2;
			}
			bcopy (eoc, cp, 2); /* end of octet string (contents) */
			cp += 2; len += 2;
			bcopy (eoc, cp, 2); /* end of hdr sequence */
			cp += 2; len += 2;

			ptr = buffer;
			trans_state = st_end;
			break;
		
		    case st_end:
			return 0;
		}
	}
	*dptr = ptr;
	n = MIN (dlen, len);
	len -= n;
	ptr += n;
	PP_TRACE (("get_more_message returneing %d bytes", n));
	return n;

}




static int  rts_downtrans_all (sd, base, len, size, ssn, ack, rti)
int			sd;
char			**base;
int			*len,
			size;
long			ssn,
			ack;
struct RtSAPindication	*rti;
{
	int	n;
	static char	*trans_buf;
	static int	bsize;

	PP_TRACE (("rts_downtrans_all (%d, base, len, %d, %ld, %ld, rti)",
		   sd, size, ssn, ack));

	if (base == NULLVP) {
		PP_DBG (("RT-PLEASE.INDICATION: %d", size));
		return OK;
	}

	if (trans_buf == NULLCP) {

		if (size == 0)	/* no checkpointing... */
			n  =  p1_length;
		else  
			n = size;

		if ((trans_buf = malloc ((unsigned) n)) == NULL)
			return (rtsaplose (rti, RTS_CONGEST,
				NULLCP, "out of memory"));

		PP_TRACE (("Selecting block size of %d", n));
		bsize = n;
	}
	if (size > 0 && size < bsize) {
		PP_LOG (LLOG_EXCEPTIONS, ("downtrans size decreased..."));
		bsize = size;
	}

	n = MIN(bsize, p1_length);
	if (n == 0) {
		free(trans_buf);
		trans_buf = NULLCP;
		*base = NULLCP;
		*len = 0;
		return OK;
	}
	*base = p1_ptr;
	*len = n;
	data_bytes += n;

	p1_ptr += n;
	p1_length -= n;
	
	if (dumpp1)
		dump2file (*base, *len, dumpp1);
		
	return OK;
}




static char *pe_flatten (pe, buffer, flag, ccp)
PE	pe;
char	*buffer;
int	*ccp;
int	flag;
{
	PS	ps;
	char	*cp;

	if ((ps = ps_alloc (str_open)) == NULLPS)
		adios (NULLCP, "ps_alloc failed");
	if (str_setup (ps, buffer, *ccp, buffer == NULLCP ? 0 : 1) == NOTOK)
		adios (NULLCP, "str_setup failed", ps_error (ps -> ps_errno));

	if (pe2ps_aux (ps, pe, flag) == NOTOK)
		adios (NULLCP, "pe2ps failed [%s]", ps_error (ps -> ps_errno));
	*ccp = ps -> ps_ptr - ps -> ps_base;
	cp = ps -> ps_base;
	ps -> ps_base = NULLCP;
	ps_free (ps);
	return cp;
}




static int rts_end ()
{
	struct RtSAPindication rtis, *rti = &rtis;
	struct RtSAPabort *rta = &rti -> rti_abort;
	struct AcSAPrelease acrs;
	register struct AcSAPrelease *acr = &acrs;

	if (rts_sd == NOTOK)
		return DONE;
	if (conn_type == RTSP_1988_NORMAL) {
		if (RtCloseRequest (rts_sd, ACF_NORMAL, NULLPE,
				    acr, rti) == NOTOK) {
			rts_advise (rta, "RT-CLOSE.REQUEST");
			RTAFREE(rta);
			(void) RtUAbortRequest (rts_sd, NULLPE, rti);
			return NOTOK;
		}
		if (!acr -> acr_affirmative) {
			(void) RtUAbortRequest (rts_sd, NULLPE, rti);
			PP_LOG (LLOG_EXCEPTIONS,
				("release rejected by peer: %d, %d elements",
				 acr -> acr_reason, acr -> acr_ninfo));
		}
		ACRFREE (acr);
	}
	else {
		if (RtEndRequest (rts_sd, rti) == NOTOK) {
			rts_advise (rta, "RT-END.REQUEST");
			RTAFREE(rta);
			rts_sd = NOTOK;
			return NOTOK;
		}
	}
	rts_sd = NOTOK;
	return OK;
}




#ifdef PP_DEBUG
static int dump2file (str, n, file)
char	*str;
int	n;
char	*file;
{
	FILE	*fp;
	static int once_only = 0;

	if ((fp = fopen (file, once_only ? "a": "w")) == NULL) {
		advise (LLOG_NOTICE, NULLCP, file, "Can't open file");
		return;
	}
	once_only ++;

	fwrite (str, 1, n, fp);
	(void) fclose (fp);
}
#endif




static void open_body ()
{
	char	filename[MAXPATHLENGTH],
		*msgdir = NULLCP;

	if (body_fp)
		close_body ();

	if (qid2dir (this_msgid, ad_list, TRUE, &msgdir) == NOTOK)
		adios (NULLCP, "Can't find message %s", this_msgid);

	if (rp_isbad (msg_rinit (msgdir)))
		adios (NULLCP, "Can't initialise directory %s", msgdir);

	if (rp_isbad (msg_rfile (filename)))
		adios (NULLCP, "Can't read file-name");

	(void) msg_rend ();

	if ((body_fp = fopen (filename, "r")) == NULL)
		adios (filename, "Can't open file");

}
	



static int read_body (buf, size)
char	*buf;
int	size;
{
	int	len, i;
	static PE	pe = NULLPE;
	PE		pe2;
	char	tbuf[BUFSIZ*8];

	PP_TRACE (("read_body (%d)", size));
	if (pe == NULLPE)
	    pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_OCTS);

	pe -> pe_len = size;
	pe -> pe_len -= ps_get_abs (pe) - size;
	for (i = 0; i < 10; i++) {
	    if (ps_get_abs(pe) <= size)
		break;
	    pe -> pe_len -= 2;
	}
	if ( i >= 10)
	    adios (NULLCP, "Can't sort out asn1 length after 10 attempts");
	
	if ((len = fread (tbuf, 1, pe -> pe_len, body_fp)) <= 0) {
		if (ferror (body_fp))
			adios ("fread", "error on file");
		else if (feof (body_fp))
			return 0;
	}
	pe2 = oct2prim (tbuf, len);
	(void) pe_flatten (pe2, buf, 1, &size);
	PP_TRACE (("octet string of length %d returned (%d)",
		   pe2 -> pe_len, size));
	pe_free (pe2);
	return size;
}




static void close_body ()
{
	if (body_fp)
		fclose (body_fp);
	body_fp = NULL;
}





static int construct_msg (qp, recip)
Q_struct *qp;
ADDR 	*recip;
{
	struct type_Trans_MtsAPDU *p1;
	ADDR	*ap;
	int value;
	PE	pe;

	switch (build_p1 (qp, recip, &p1, trace_type)) {
	    case NOTOK:
		delivery_setallstate (int_Qmgr_status_messageFailure,
				 "Can't build P1 information");
		return NOTOK;
	    case DONE:
		for (ap = recip; ap; ap = ap -> ad_next) {
			if (!ap -> ad_resp)
				continue;
			set_1dr (qp, ap -> ad_no, this_msgid,
				  DRR_UNABLE_TO_TRANSFER,
				  ub_error_set,
				  ub_error_string);
		}
		delivery_setall (int_Qmgr_status_negativeDR);
		if (rp_isbad(wr_q2dr (qp, this_msgid)))
			delivery_resetDRs (int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (encode_MTA_MessageTransferEnvelope (&pe, 1, 0, NULLCP,
						p1 -> un.message -> envelope)
	    == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Error encoding MessageTransferEnvelope [%s]",
			 PY_pepy));
		pe_free (pe);
		free_Trans_MtsAPDU (p1);
		return NOTOK;
	}
	PP_PDUP (MTA_MessageTransferEnvelope, pe,
		"MTA.MessageTransferEnvelope", PDU_WRITE);

	p1_length = 0;
	p1_string = p1_ptr = pe_flatten (pe, NULLCP, 1, &p1_length);
	free_Trans_MtsAPDU (p1);
	pe_free (pe);

	return OK;
}




static int construct_probe (qp, recip) /* XXX */
Q_struct *qp;
ADDR	*recip;
{
	struct type_Trans_MtsAPDU *p1;
	ADDR	*ap;
	PE	pe;

	switch (build_probe (qp, recip, &p1, trace_type)) {
	    case NOTOK:
		delivery_setallstate (int_Qmgr_status_messageFailure,
				      "Can't build P1 information");
		return NOTOK;
	    case DONE:
		for (ap = recip; ap; ap = ap -> ad_next) {
			if (!ap -> ad_resp)
				continue;
			set_1dr (qp, ap -> ad_no, this_msgid,
				  DRR_UNABLE_TO_TRANSFER,
				  ub_error_set,
				  ub_error_string);
		}
		delivery_setall (int_Qmgr_status_negativeDR);
		if (rp_isbad(wr_q2dr (qp, this_msgid)))
			delivery_resetDRs (int_Qmgr_status_messageFailure);
		return NOTOK;
	}

	if (encode_Trans_MtsAPDU (&pe, 1, 0, NULLCP, p1) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Error encoding MtsAPDU [%s]", PY_pepy));
		if (pe)
			pe_free (pe);
		free_Trans_MtsAPDU (p1);
		return NOTOK;
	}
	PP_PDUP (Trans_MtsAPDU, pe, "MTA.MtsAPDU", PDU_WRITE);

	if (conn_type == RTSP_1988_NORMAL) {
		PE pe_ext, pe2;
		if ((pe_ext = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS,
					PE_CONS_EXTN)) == NULLPE ||
		    (pe2 = int2prim  (mctx_id)) == NULLPE ||
		    seq_add (pe_ext, pe2, NOTOK) == NOTOK ||
		    (pe2 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 0)) == NULLPE)
			return NOTOK;
		pe2 -> pe_cons = pe;
		if (seq_add (pe_ext, pe2, NOTOK) == NOTOK)
			return NOTOK;
		pe = pe_ext;
	}
	p1_length = 0;
	p1_string = p1_ptr = pe_flatten (pe, NULLCP, 1, &p1_length);

	free_Trans_MtsAPDU (p1);
	pe_free (pe);

	return OK;
}




static struct type_MTA_Content *get_dr_content (qp, recip)
Q_struct *qp;
ADDR *recip;
{
	char *dir;
	char filename[MAXPATHLENGTH];
	char buf[MAXPATHLENGTH];
	struct type_MTA_Content *qb, *qbh;
	int fd;
	struct stat st;

	if (qid2dir (this_msgid, qp->Oaddress, TRUE, &dir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("qid2dir can't find DR content"));
		return NULL;
	}
	if (msg_rinit (dir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("msg_rinit failed"));
		return NULL;
	}
	if (msg_rfile (filename) != RP_OK) {
		PP_LOG (LLOG_EXCEPTIONS, ("msg_file failed"));
		return NULL;
	}
	if (msg_rfile(buf) != RP_DONE) {
		PP_LOG (LLOG_EXCEPTIONS, ("Extra message component - expecting only one"));
		return NULL;
	}
	msg_rend ();

	if ((fd = open (filename, O_RDONLY, 0)) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, filename, ("Cant open"));
		return NULL;
	}
	if (fstat (fd, &st) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, filename, ("Can't fstat"));
		(void) close (fd);
		return NULL;
	}
	qb = (struct type_MTA_Content *) smalloc (st.st_size + sizeof(*qb));
	qb -> qb_data = qb -> qb_base;
	if (read (fd, qb->qb_data, st.st_size) != st.st_size) {
		PP_SLOG (LLOG_EXCEPTIONS, "read", ("failed"));
		(void) close (fd);
		return NULL;
	}
	qb -> qb_len = st.st_size;
	(void) close(fd);
	qbh = str2qb (NULLCP, 0, 0);
	insque(qb, qbh);
	return qbh;
}




static int construct_dr (qp, recip)
Q_struct *qp;
ADDR	*recip;
{
	static DRmpdu drs;
	DRmpdu	*dr = &drs;
	struct type_Trans_MtsAPDU *p1dr;
	PE	pe = NULLPE;
	struct type_MTA_Content *qb = NULL;
	ADDR	*ap;

	dr_free (dr);
	if (rp_isbad (get_dr (recip -> ad_no, this_msgid, dr))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't read delivery report for %s/%d",
			 this_msgid, recip -> ad_no));
		delivery_setallstate (int_Qmgr_status_messageFailure,
				      "Can't read DR");
		return NOTOK;
	}
	if (qp -> content_return_request == TRUE)
		qb = get_dr_content (qp, recip);
	
	switch (build_dr (qp, recip, dr, qb, &p1dr, trace_type)) {
	    case NOTOK:
		delivery_setallstate (int_Qmgr_status_messageFailure,
				      "Can't build P1 information");
		return NOTOK;
	    case DONE:
		for (ap = recip; ap; ap = ap -> ad_next) {
			if (!ap -> ad_resp)
				continue;
			set_1dr (qp, ap -> ad_no, this_msgid,
				  DRR_UNABLE_TO_TRANSFER,
				  ub_error_set,
				  ub_error_string);
		}
		delivery_setall (int_Qmgr_status_negativeDR);
		if (rp_isbad(wr_q2dr (qp, this_msgid)))
			delivery_resetDRs (int_Qmgr_status_messageFailure);
		return NOTOK;
	}
	if (encode_Trans_MtsAPDU (&pe, 1, 0, NULLCP,
				     p1dr)
	    == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Error encoding MtsAPDU [%s]",
			 PY_pepy));
		if (pe)
			pe_free (pe);
		free_Trans_MtsAPDU (p1dr);
		return NOTOK;
	}
	PP_PDUP (Trans_MtsAPDU, pe,
		"MTA.MtsAPDU", PDU_WRITE);
	if (conn_type == RTSP_1988_NORMAL) {
		PE pe_ext, pe2;
		if ((pe_ext = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS,
					PE_CONS_EXTN)) == NULLPE ||
		    (pe2 = int2prim  (mctx_id)) == NULLPE ||
		    seq_add (pe_ext, pe2, NOTOK) == NOTOK ||
		    (pe2 = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 0)) == NULLPE)
			return NOTOK;
		pe2 -> pe_cons = pe;
		if (seq_add (pe_ext, pe2, NOTOK) == NOTOK)
			return NOTOK;
		pe = pe_ext;
		PP_TRACE(("External wrapper in place"));
	}

	p1_length = 0;
	p1_string = p1_ptr = pe_flatten (pe, NULLCP, 1, &p1_length);

	free_Trans_MtsAPDU (p1dr);
	pe_free (pe);
		
	return OK;
}




/* ---------------------  Rtsping88  Routine  ------------------------------- */




static int rtsping88 (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int		opt;

	while ((opt = getopt (argc, argv, "c:m:")) != EOF) {
		switch (opt) {
		case 'c':
			chan_name = strdup (optarg);
			break;
		case 'm':
			current_mta = strdup (optarg);
			break;
		default:
			adios (NULLCP, "Illegal option for rts pinging %s",
				argv[optind-1]);
		}
	}

	RTS_PING_88 = TRUE;

	initproc (NULL);
	if (attempt_connect() != OK)
		PP_NOTICE (("Connection badly terminated"));
	else
		endproc();
	exit (0);
}
