/* x400out84.c: transmit x400 messages out across RTS */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40084/RCS/x400out84.c,v 6.0 1991/12/18 20:13:50 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/x400out84.c,v 6.0 1991/12/18 20:13:50 jpo Rel $
 *
 * $Log: x400out84.c,v $
 * Revision 6.0  1991/12/18  20:13:50  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include <sys/param.h>
#include <sys/stat.h>
#include "sys.file.h"
#include "prm.h"
#include "q.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "dr.h"
#include "Rts84-types.h"
#include "rtsparams.h"
#include <isode/internet.h>
#include <isode/rtsap.h>
#include <isode/psap.h>
#include <isode/cmd_srch.h>


#define MAXTRIES			10


/* -- externals -- */
extern char				*loc_dom_site;
extern char				*quedfldir;
extern char				*rcmd_srch();
extern char				reason[];
extern CMD_TABLE			qtbl_mt_type[];


/* -- queue variables -- */
static Q_struct				Qstruct;
static DRmpdu				DRstruct;
static struct prm_vars			PRMstruct;
Q_struct				*PPQuePtr = &Qstruct;
DRmpdu					*DRptr = &DRstruct;


/* -- globals -- */
char					*body_string = NULLCP;
char					*pp_myname;
char					*fix_orig;
int					body_len = 0;
int					trace_type = RTSP_TRACE_ALL; 
ADDR					*ad_list;


/* -- statics -- */
static int				RTS_PING_84 = FALSE;
static CHAN				*mychan;
static char				*this_msgid = NULLCP,
					*dumpp1 = NULLCP,
					*chan_name = NULLCP,
					*current_mta = NULLCP,
					*connected_to_site = NULLCP,
					*p1_string = NULLCP,
					*p1_ptr;
static struct timeval			data_timer;
static int				data_bytes=0,
					p1_length = 0,
					rts_sd = NOTOK;
static enum { st_init, st_hdr, st_bodyinit, st_body, st_end } trans_state;


/* -- local routines -- */
static char				*pe_flatten();
static int				attempt_connect();
static void				close_body();
static int				construct_dr();
static int				construct_msg();
static int				construct_probe();
static int				deliver();
static void				dirinit();
static void				dump2file();
static int				endproc();
static int				get_more_message();
static int				initproc();
static void				open_body();
static int				read_body();
static int				rts_connect();
static int				rts_downtrans_all();
static int				rts_downtrans_inc();
static int				rts_end();
static int				rts_start();
static int				rts_transfer_request();
static int				rtsping84();
static void				wr_sender2log();
static struct type_Qmgr_DeliveryStatus	*process();




/* ---------------------  Begin	 Routines  -------------------------------- */





main (argc, argv)
int	argc;
char	**argv;
{
	if (pp_myname = rindex (argv[0], '/'))
		pp_myname++;
	if (pp_myname == NULL || *pp_myname == NULL)
		pp_myname = argv[0];


	/* -- read pp tailor file -- */
	chan_init (pp_myname);


	/* -- rts84 ping required ? -- */
	if (argc > 1 && strcmp(argv[1], "ping") == 0)
		rtsping84 (argc - 1, argv + 1);


	/* -- get ready to transmit x400out84 messages -- */
	pp_setuserid();
	or_myinit();
	dirinit();


#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0) {
		if (argc > 2)
			dumpp1 = argv[2];
		debug_channel_control (argc, argv, initproc, process, endproc);
	} else
#endif
		(void) channel_control (argc, argv, initproc, process, endproc);

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

	if (!RTS_PING_84)  chan_name = qb2str (arg);

	PP_TRACE (("initproc (%s)", chan_name));

	if ((mychan = ch_nm2struct (chan_name)) == NULLCHAN)
		err_abrt (RP_PARM, "Channel '%s' not known", chan_name);

	rename_log (chan_name);
	free (chan_name);
	chan_name = NULLCP;

	PP_NOTICE (("Starting %s (%s)", mychan -> ch_name, mychan -> ch_show));

	if (RTS_PING_84)  return OK;

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
	case RP_OK:
		PP_NOTICE (("Connection successfully terminated"));
		break;
	case RP_BAD:
		PP_NOTICE (("Connection badly terminated"));
		break;
	default:
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
					*ad_recip  = NULLADDR,
					*rp,
					*sp,
					*lp = NULL;
	int				naddrs = 0,
					ad_count,
					retval;

	if (this_msgid)	 free (this_msgid);
	this_msgid = qb2str (arg -> qid);

	PP_NOTICE (("Reading message '%s'", this_msgid));

	q_free (PPQuePtr);
	dr_free (&DRstruct);
	prm_free (&PRMstruct);

	delivery_init (arg -> users);

	retval = rd_msg (this_msgid, &PRMstruct, PPQuePtr,
			&ad_sender, &ad_recip, &ad_count);

	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("Read message failed for '%s'", this_msgid));
		reason[0] = 0;
		goto process_error;
	}

	if (ad_sender -> ad_r400adr == NULLCP) {
		do_reason ("x400 address not set in sender field of '%s'",
			 this_msgid);
		goto process_error;
	}


	for (sp = ad_sender, rp = ad_recip; rp; rp = rp -> ad_next) {

		for (up = arg -> users; up; up = up -> next) {

			PP_TRACE (("'%s' '%s' ad_no=%d up_no=%d",
				this_msgid, rp -> ad_value, rp -> ad_no,
				up -> RecipientId -> parm));

			if (up -> RecipientId -> parm != rp -> ad_no)
				continue;

			if (dchan_acheck (rp, sp, mychan, naddrs == 0,
					&current_mta))
				continue;
			naddrs ++;
			break;

		} /* -- end of for -- */


		if (up == NULL)
			continue;

		if (ad_list == NULLADDR)
			ad_list = lp = (ADDR *)calloc(1, sizeof *lp);
		else {
			lp -> ad_next = (ADDR *)calloc(1, sizeof *lp);
			lp = lp -> ad_next;
		}

		*lp = *rp;     /* struct copy */
		lp -> ad_next = NULLADDR;

	} /* -- end of for -- */


	if (naddrs == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("No recips to be processed in '%s'", this_msgid));
		rd_end ();
		return deliverystate; /* delivery state is set correct */
	}


	deliver (ad_list);		/* deliverystate set in deliver */

	for (lp = ad_list; lp; lp = rp) {
		rp = lp -> ad_next;
		free ((char *)lp);
	}

	ad_list = NULLADDR;
	rd_end();
	return deliverystate;


process_error: ;
	rd_end();
	return delivery_setallstate (int_Qmgr_status_messageFailure, reason);
}




static void wr_sender2log (sender)
ADDR	*sender;
{
	switch (PPQuePtr -> msgtype) { 
	case MT_PMPDU: 
		PP_NOTICE (("Sender (of probe) '%s'", sender -> ad_r400adr));
		break;

	default:
		if (ad_list -> ad_status == AD_STAT_PEND)
			PP_NOTICE (("Sender '%s'", sender -> ad_r400adr));
		else 
			PP_NOTICE (("Sender (of delivery report '%s'",
					sender -> ad_r400adr));
		break;
	}

	return;
}




static int deliver (recip)
ADDR	*recip;
{
	int	value	= int_Qmgr_status_messageFailure;
	ADDR	*ap;
	int msgtype;
	int	retval;


	switch (recip -> ad_status) {
	    case AD_STAT_PEND:
	    case AD_STAT_DRWRITTEN:
		break;
	    case AD_STAT_DONE:
		delivery_setallstate (int_Qmgr_status_success, "All delivered");
		return NOTOK;
	    default:
		do_reason ("bad state");
		delivery_setallstate (int_Qmgr_status_messageFailure, reason);
		return NOTOK;
	}


	if (attempt_connect() != OK)
		return NOTOK;

	msgtype = Qstruct.msgtype;
	if (fix_orig)
		x400_fixorig (Qstruct.Oaddress, fix_orig);
	if (recip -> ad_status == AD_STAT_DRWRITTEN ||
	    Qstruct.msgtype == MT_DMPDU) {
		retval = construct_dr ( &Qstruct, recip);
		msgtype = MT_DMPDU;
	}
	else if (Qstruct.msgtype == MT_PMPDU)
		retval = construct_probe (&Qstruct, recip);
	else
		retval = construct_msg (&Qstruct, recip);


	if (retval == NOTOK)
		return NOTOK;


	wr_sender2log (Qstruct.Oaddress);

	for (ap = recip; ap; ap = ap -> ad_next)
		PP_NOTICE (("Recipient Address '%s'",  ap -> ad_r400adr));

	switch (rts_transfer_request(msgtype)) {
	    case OK:
		value = int_Qmgr_status_success;
		break;
	    default:
		if (rts_sd == NOTOK)
			value = int_Qmgr_status_mtaAndMessageFailure;
		else
			value = int_Qmgr_status_messageFailure;
		break;
	}


	for (ap = recip; ap; ap = ap -> ad_next)
		if (ap -> ad_resp) {
			switch (value) {
			case int_Qmgr_status_success:
				delivery_set (ap -> ad_no, value);
				wr_ad_status (ap, AD_STAT_DONE);
				wr_stat (ap, &Qstruct, this_msgid, data_bytes);
				break;
			default:
				delivery_setstate (ap -> ad_no, value, reason);
				break;
			}
		}


	if (p1_string)		free (p1_string);
	p1_string		= NULLCP;
	data_bytes		= 0;
	p1_length		= 0;

	return OK;
}


static void get_dr_content (qp, recip)
Q_struct *qp;
ADDR *recip;
{
	char *dir = NULLCP;
	char filename[MAXPATHLENGTH];
	char buf[MAXPATHLENGTH];
	int fd;
	struct stat st;

	if (qid2dir (this_msgid, qp->Oaddress, TRUE, &dir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("qid2dir can't find DR content"));
		return;
	}
	if (msg_rinit (dir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("msg_rinit failed"));
		return;
	}
	if (msg_rfile (filename) != RP_OK) {
		PP_LOG (LLOG_EXCEPTIONS, ("msg_file failed"));
		return;
	}
	if (msg_rfile(buf) != RP_DONE) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Extra message component - expecting only one"));
		return;
	}
	msg_rend ();
	if ((fd = open (filename, O_RDONLY, 0)) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, filename, ("Cant open"));
		return;
	}
	if (fstat (fd, &st) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, filename, ("Can't fstat"));
		(void) close (fd);
		return;
	}
	body_string = smalloc (body_len = st.st_size);
	if (read (fd, body_string, st.st_size) != st.st_size) {
		PP_SLOG (LLOG_EXCEPTIONS, "read", ("failed"));
		(void) close (fd);
		return;
	}
	(void) close(fd);
}

static int construct_dr (qp, recip)
Q_struct *qp;
ADDR	*recip;
{
	PE	pe;

	if (rp_isbad (get_dr (recip -> ad_no, this_msgid, &DRstruct))) {
		do_reason ("Can't read delivery report '%s' recip '%d'",
			 this_msgid, recip -> ad_no);
		delivery_setallstate (int_Qmgr_status_messageFailure, reason);
		return NOTOK;
	}

	qp -> msgtype = MT_DMPDU;
	if (qp -> content_return_request == TRUE)
		get_dr_content(qp, recip);

	if (build_P1_MPDU (&pe, 1, NULLINT, NULLCP, NULL) == NOTOK) {
		do_reason ("Can't build DR ASN.1: %s", PY_pepy);
		delivery_setallstate (int_Qmgr_status_messageFailure, reason);
		return NOTOK;
	}

	PP_PDU (print_P1_MPDU, pe, "P1.MPDU", PDU_WRITE);

	p1_length = 0;
	p1_string = p1_ptr = pe_flatten (pe, NULLCP, 1, &p1_length);
	pe_free (pe);
	dr_free (&DRstruct);
	if (body_string) {
		free (body_string);
		body_string = NULLCP;
		body_len = 0;
	}
	return OK;
}



static int construct_probe (qp, recip)
Q_struct *qp;
ADDR	*recip;
{
	PE pe;

	if (build_P1_MPDU (&pe, 1, NULLINT, NULLCP, NULL) == NOTOK) {
		do_reason ("Can't build Probe ASN.1: %s", PY_pepy);
		return NOTOK;
	}

	PP_PDU (print_P1_MPDU, pe, "P1.MPDU", PDU_WRITE);

	p1_length = 0;
	p1_string = p1_ptr = pe_flatten (pe, NULLCP, 1, &p1_length);
	pe_free (pe);
	return OK;
}



static int construct_msg (qp, recip)
Q_struct *qp;
ADDR *recip;
{
	PE	pe;

	if (build_P1_UMPDUEnvelope (&pe, 1, NULLINT, NULLCP, qp) == NOTOK) {
		do_reason ("Can't build MSG ASN.1: %s", PY_pepy);
		return NOTOK;
	}
	PP_PDU (print_P1_UMPDUEnvelope, pe, "P1.UMPDUEnvelope", PDU_WRITE);

	p1_length = 0;
	p1_string = p1_ptr = pe_flatten (pe, NULLCP, 1, &p1_length);
	pe_free (pe);
	return OK;
}




static int attempt_connect()
{
	if (connected_to_site == NULLCP || rts_sd == NOTOK)
		goto attempt_rts_start;

	if (lexequ (connected_to_site, current_mta) == 0)
		return OK;

	(void) rts_end();
	goto attempt_rts_start;

attempt_rts_start: ;
	if (connected_to_site) free (connected_to_site);
	connected_to_site = NULLCP;


	if (rts_start() != OK) {
		delivery_setallstate (int_Qmgr_status_mtaFailure, reason);
		return NOTOK;
	}

	connected_to_site = strdup (current_mta);
	return OK;
}




static int rts_start()
{
	RtsParams	*rp;
	int		count, retcode = NOTOK;

	PP_TRACE (("rts_start site %s", current_mta));

	for (count = 0; count < MAXTRIES; count++) {

		if ((rp = tb_rtsparams (mychan -> ch_table,
					current_mta)) == NULL) {
			do_reason ("Unable to locate '%s'", current_mta);
			return NOTOK;
		}

		if (rp -> type != RTSP_1984) {
			do_reason ("Not 1984 connection type %s", current_mta);
			break;
		}

		retcode = rts_connect (rp);

		switch (retcode) {
		case OK:
			break;
		default:
			if (rp -> try_next) {
				free (current_mta);
				current_mta = strdup (rp -> try_next);
				RPfree (rp);
				continue;
			}	
			break;
		}

		break;
	}


	trace_type = rp -> trace_type;
	if (fix_orig)
		free (fix_orig);
	if (rp -> fix_orig)
		fix_orig = strdup (rp -> fix_orig);
	else fix_orig = NULLCP;
	RPfree (rp);
	return retcode;
}




static int rts_connect (rp)
RtsParams *rp;
{
	struct RtSAPaddr			rtsapaddr;
	register struct RtSAPaddr		*rtsap = &rtsapaddr;
	struct RtSAPaddr			rts_calling;
	struct RtSAPaddr			*rt_c;
	struct SSAPaddr				*sa;
	struct RtSAPconnect			rtcs;
	register struct RtSAPconnect		*rtc = &rtcs;
	struct RtSAPindication			rtis;
	register struct RtSAPindication		*rti = &rtis;
	register struct RtSAPabort		*rta = &rti -> rti_abort;
	struct type_Rts84_Request		*req = NULL;
	int					retval = NOTOK;
	PE					pe = NULLPE;

	PP_TRACE (("rts_connect(%d)", rp -> rts_mode));

	if (rts_encode_request (&pe, &req, rp -> our_name,
				rp -> our_passwd) == NOTOK)
		goto rts_connect_free;

	PP_PDU (print_Rts84_Request, pe, "RTS84.Request", PDU_WRITE);

	/* -- clears the relavant structures -- */
	bzero ((char *)rtsap, sizeof (*rtsap));
	bzero ((char *)rtc, sizeof (*rtc));
	bzero ((char *)rti, sizeof (*rti));
	bzero ((char *)rta, sizeof (*rta));

	rtsap -> rta_port = htons(1); /* P1 id */

	if (rp -> their_address == NULLCP) {
		do_reason ("Null called address for %s", rp -> their_name);
		goto rts_connect_free;
	}

	if ((sa = str2saddr (rp -> their_address)) == NULLSA) {
		do_reason ("Bad called address for %s", rp -> their_address);
		goto rts_connect_free;
	}

	rtsap -> rta_addr = *sa; /* struct copy */

	if(rp -> our_address == NULLCP ||
	   (sa = str2saddr (rp -> our_address)) == NULLSA)
		rt_c = NULLRtA;
	else {
		rt_c = &rts_calling;
		bzero ((char *)rtc, sizeof *rt_c);
		rt_c -> rta_addr = *sa; /* struct copy */
	}

	PP_NOTICE (("Connecting to site %s", current_mta));

	if (RtBeginRequest2 (rtsap, rt_c, rp -> rts_mode, RTS_INITIATOR,
			     pe, rtc, rti) == NOTOK) {
		rts_advise (rta, "RT-BEGIN.REQUEST");
		goto rts_connect_free;
	}

	rts_sd = rtc -> rtc_sd;

	if (rtc -> rtc_result != RTS_ACCEPT) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Unable to connect to site %s (reason %s)",
			 current_mta, RtErrString (rtc -> rtc_result)));
		goto rts_connect_free;
	}


	PP_PDU (print_Rts84_Request, rtc -> rtc_data, "RTS84.Response",
		PDU_READ);

	if (parameter_checks (rtc -> rtc_data, rp -> their_name,
			      rp -> their_passwd, current_mta, TRUE) == OK)
		retval = OK;

rts_connect_free: ;
	RTCFREE(rtc);
	RTAFREE(rta);
	if (pe)
		pe_free (pe);
	if (req)
		free_Rts84_Request (req);
	return retval;
}




static int rts_transfer_request(type)
int	type;
{
	struct RtSAPindication		rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort	*rta = &rti -> rti_abort;

	PP_TRACE (("rts_transfer_request()"));

	trans_state = st_init;

	switch (type) {
	    case MT_PMPDU:
	    case MT_DMPDU:
		if (RtSetDownTrans (rts_sd, rts_downtrans_all, rti) == NOTOK) {
			rts_advise (rta, "set DownTrans upcall");
			return NOTOK;
		}
		break;

	    case MT_UMPDU:
	    default:
		if (RtSetDownTrans (rts_sd, rts_downtrans_inc, rti) == NOTOK) {
			rts_advise (rta, "set DownTrans upcall");
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
		return NOTOK;
	}


	if (data_bytes > 0)
		timer_end (&data_timer, data_bytes, "Transfer Completed");

	PP_NOTICE((">>> %s %s transfered to %s",
		rcmd_srch(Qstruct.msgtype, qtbl_mt_type),
		this_msgid, current_mta));

	return OK;
}




/* ARGSUSED */

static int  rts_downtrans_all (sd, base, len, size, ssn, ack, rti)
int			sd;
char			**base;
int			*len,
			size;
long			ssn,
			ack;
struct RtSAPindication	*rti;
{
	register int	n;
	static char   *bp;
	static int	bsize;


	PP_TRACE (("rts_downtrans()"));

	if (base == NULLVP) {
		PP_TRACE (("RT-PLEASE.INDICATION: %d", size));
		return OK;
	}

	if (bp == NULLCP) {

		if (size == 0)	/* no checkpointing... */
			n =  p1_length;
		else
			n = size;

		if ((bp = malloc ((unsigned) n)) == NULL)
			return (rtsaplose (rti, RTS_CONGEST,
				NULLCP, "out of memory"));

		PP_TRACE (("Selecting block size of %d", n));
		bsize = n;
	}
	if (size > 0 && size < bsize) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("rts_downtrans/downtrans size decreased.."));
		bsize = size;
	}

	n = MIN(bsize, p1_length);

	if (n == 0) {
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
		PP_TRACE (("RT-PLEASE.INDICATION: %d", size));
		return OK;
	}

	if (resetbuf) {
		resetbuf = 0;
		free (trans_buf);
		trans_buf = NULLCP;
		bsize = 0;
	}

	if (trans_buf == NULLCP) {

		if (size == 0) {	/* no checkpointing... */
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
				trans_buf = realloc (trans_buf, bsize);
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

static int get_more_message (dptr, n)
char	**dptr;
int	n;
{
	static char	eoc[3];
	static char	*ptr;
	static int	len = 0, inited = 0;
	static char	buffer[BUFSIZ*8];
	PE		pe;

	PP_TRACE (("get_more_message (%d)", n));
	if (!inited) {
		pe = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_UNIV_EOC);
		inited = 3;
		(void) pe_flatten (pe, eoc, 1, &inited);
		pe_free (pe);
	}
	
	if (len <= 0) {
		switch (trans_state) {
		    case st_init:
			pe = pe_alloc (PE_CLASS_CONT, PE_FORM_CONS, 0);
			pe -> pe_len = PE_LEN_INDF;
			len = sizeof buffer;
			(void) pe_flatten (pe, buffer, -1, &len);
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
			len = n;
			if ((len = read_body (buffer, sizeof buffer)) != 0) {
				ptr = buffer;
				break;
			}
			close_body ();
			bcopy (eoc, buffer, 2); /* end of octet string (contents) */
			bcopy (eoc, buffer + 2, 2); /* end of hdr sequence */
			len = 4;
			ptr = buffer;
			trans_state = st_end;
			break;
		
		    case st_end:
			return 0;
		}
	}
	*dptr = ptr;
	n = MIN (n, len);
	len -= n;
	ptr += n;
	PP_TRACE (("get_more_message returning %d bytes", n));
	return n;

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


static int rts_end()
{
	struct RtSAPindication		rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort	*rta = &rti -> rti_abort;

	PP_TRACE (("rts_end (%d)", rts_sd));

	if (rts_sd != NOTOK)
		if (RtEndRequest (rts_sd, rti) == NOTOK) {
			rts_advise (rta, "RT-END.REQUEST");
			rts_sd = NOTOK;
			return RP_BAD;
		}
		else {
			rts_sd = NOTOK;
			return RP_OK;
		}

	return RP_NOOP;
}

#ifdef PP_DEBUG
static void dump2file (str, n, file)
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

static FILE	*body_fp;

static void open_body()
{
	char	filename[MAXPATHLENGTH],
		*msgdir = NULLCP;

	if (body_fp)
		close_body();

	if (qid2dir (this_msgid, ad_list, TRUE, &msgdir) == NOTOK)
		adios (NULLCP, "Can't find message %s", this_msgid);

	if (rp_isbad (msg_rinit (msgdir)))
		adios (NULLCP, "Can't initialise directory %s", msgdir);

	if (rp_isbad (msg_rfile (filename)))
		adios (NULLCP, "Can't read file-name");

	(void) msg_rend();

	if ((body_fp = fopen (filename, "r")) == NULL)
		adios (filename, "Can't open file");

}


static int read_body (buf, size)
char	*buf;
int	size;
{
	int		len, i;
	static PE	pe = NULLPE;
	PE		pe2;
	char		tbuf[BUFSIZ*8];

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
	

	bzero (tbuf, BUFSIZ);
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



static void close_body()
{
	if (body_fp)
		fclose (body_fp);
	body_fp = NULL;
}




/* ---------------------  Rtsping84  Routine  ------------------------------- */




static int rtsping84 (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int		opt;


	PP_NOTICE (("Doing rtsping84"));

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

	RTS_PING_84 = TRUE;
	initproc (NULL);
	if (attempt_connect() != OK)
		PP_NOTICE (("Connection badly terminated"));
	else
		endproc();

	exit(0);
}
