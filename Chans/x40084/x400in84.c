/* x400in84.c: receives the incomming x400 messages */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40084/RCS/x400in84.c,v 6.0 1991/12/18 20:13:50 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/x400in84.c,v 6.0 1991/12/18 20:13:50 jpo Rel $
 *
 * $Log: x400in84.c,v $
 * Revision 6.0  1991/12/18  20:13:50  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "dr.h"
#include "q.h"
#include "Rts84-types.h"
#include <isode/internet.h>
#include <sys/stat.h>
#include <isode/rtsap.h>
#include <isode/psap.h>
#include <isode/isoservent.h>
#include <isode/cmd_srch.h>



/* -- defines -- */
				/* --- hex dump of msg	--- */
#define qb2postie(qb)		(send2postie (NULLPE, qb, 0)) 
				/* --- splat of msg --- */
#define pe2postie1(pe)		(send2postie (pe, (struct qbuf *)0, 1))
				/* --- pretty print of msg --- */
#define pe2postie(pe)		(send2postie (pe, (struct qbuf *)0, 2))



#define RTS_MAX_NO_OF_TURNS	5
#define RTS_SENT_TURN		'S'
#define SUBMIT_NOT_CALLED	1


/* -- externals -- */
extern int		errno,
			body_len;
extern char		*isodesetailor(),
			*hdr_p2_bp,
			*cont_p2,	
			*loc_dom_site,
			*quedfldir,
			*body_string,
			*rcmd_srch(),
			*remote_site;
extern int		hdrproc(),
			bodyproc(),
			msgfinished();
extern void		isodetailor();
extern ADDR		*adr_new();
extern Q_struct		*PPQuePtr;
extern DRmpdu		*DRptr;
extern CMD_TABLE	qtbl_mt_type[];
extern IFP		asn_procfnx;


/* -- statics  -- */
static struct timeval	data_timer;
static int		data_bytes=0,
			initial_mode,
			initial_turn,
			nturns=0,
			rts_uptrans();
int			submit_running = 0;

static char		turn_status=' ';
CHAN			*mychan;


/* -- globals -- */
char			*pp_myname = "x400in84";

#ifdef PP_DEBUG
static int do_debug_transfer();
#endif


/* -- local routines -- */
void			set_msg_adrs();
static int		qblen();
static int		rts_uptrans();
static int		rts_do_turn();
static void		rts_get_request();
static void		rts_abort();
static void		rts_close();
static void		rts_get_request();
static void		rts_indication();



/* ---------------------  Main	Routine	 ---------------------------------- */


/*ARGSUSED)*/

main (argc, argv, envp)
int					argc;
char					**argv,
					**envp;
{
	int				sd;
	struct RtSAPstart		rtss;
	register struct RtSAPstart	*rts = &rtss;
	struct RtSAPindication		rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort	*rta = &rti -> rti_abort;
	struct SSAPaddr			*sai = &rts -> rts_initiator.rta_addr;
	struct TSAPaddr			*tai = &sai -> sa_addr;
	struct NSAPaddr			*nai = tai -> ta_addrs;
	PE				accept_pe = NULLPE;
	struct type_Rts84_Request	*request = 0;
	int				retval;
	RP_Buf				rps, *rp = &rps;
	int				opt;
	extern char			*optarg;
	extern int optind;



	/* -- Reading of the tailor file info -- */

	if (pp_myname = rindex (argv[0], '/'))
		pp_myname++;

	if (pp_myname == NULL || *pp_myname == NULL)
		pp_myname = argv[0];

	while ((opt = getopt (argc, argv, "c:")) != EOF) {
		switch (opt) {
			case 'c':
				pp_myname = optarg;
				break;
			default:
				adios (NULLCP, "Unknown argument -%c", opt);
		}
	}

	chan_init (pp_myname);

	if ((mychan = ch_nm2struct (pp_myname)) == NULLCHAN)
		adios (NULLCP, "No such channel %s", pp_myname);

	pp_setuserid();

	(void) chdir (quedfldir);

	or_myinit();

	PP_NOTICE (("Starting %s (%s)", mychan->ch_name, mychan->ch_show));


       /* --- *** ---
	Since isode/isoservices calls x400in84, the RTS connection needs
	to be terminated, if an error occurs, otherwise the connection
	hangs ... the remote initiator waits for a response and none
	is forthcoming ...
	--- *** --- */

#if PP_DEBUG
	if (argv[optind] && strcmp (argv[optind], "debug") == 0) {
		do_debug_transfer();
		exit (0);
	}
#endif
	if (RtBInit (argc, argv, rts, rti) == NOTOK) {
		rts_exceptions (rta, "RtBInit");
		exit (NOTOK);
	}


	/* --- And lots & lots of tracing! --- */

	PP_TRACE (("RT-BEGIN.INDICATION: <%d, %s, %s, %d, 0x%x>",
		rts->rts_sd,
		rts->rts_mode == RTS_TWA ? "twa" : "mon",
		rts->rts_turn == RTS_RESPONDER ? "responder" : "initiator",
		ntohs (rts->rts_port),
		rts->rts_data));

	PP_TRACE (("Calling addresses: <<%s, %s>, %s>",
		    na2str (nai),
		    sel2str (tai->ta_selector, tai->ta_selectlen, 1),
		    sel2str (sai->sa_selector, sai->sa_selectlen, 1)));

	PP_TRACE (("more ....  <<%s, '%s' '%d'>, '%s' '%d'>",
		na2str (nai),
		&tai->ta_selector[0], tai->ta_selectlen,
		&sai->sa_selector[0], sai->sa_selectlen));


	/* -- Important in determining the logic of the RTS turns -- */

	initial_mode = rts->rts_mode;
	initial_turn = rts->rts_turn;


	sd = rts -> rts_sd;


	/* -- x400in84 never transmits information -- */

	if (initial_turn == RTS_RESPONDER && initial_mode == RTS_MONOLOGUE) {
		do_reason ("main/turn=responder && mode=monologue");
		RtBeginResponse (sd, RTS_REJECT, NULLPE, rti);
		exit (NOTOK);
	}


	if (rts->rts_data == NULLPE) {
		do_reason ("main/Rejected -- no user data parameter");
		RtBeginResponse (sd, RTS_REJECT, NULLPE, rti);
		exit (NOTOK);
	}


	PP_PDU (print_Rts84_Request, rts -> rts_data,
		"RTS84.Request", PDU_READ);

	if ((retval = rts_decode_request (rts, &accept_pe,
					  &request, mychan)) != OK) {
		RtBeginResponse (sd, retval, NULLPE, rti);
		exit (NOTOK);
	}

	PP_PDU (print_Rts84_Request, accept_pe, "RTS84.Response", PDU_WRITE);


	if (RtBeginResponse (sd, RTS_ACCEPT, accept_pe, rti) == NOTOK) {
	    rts_advise (rta, "RT-BEGIN.RESPONSE Accept");
	    RtBeginResponse (sd, RTS_REMOTE, NULLPE, rti);
	    exit (NOTOK);
	}


	if (accept_pe)
		pe_free (accept_pe);
	if (request)
		free_Rts84_Request (request);



	/* --- *** ---
	Initialise submit as late as possible so if error occurs earlier on
	x400in and submit are not both running.
	--- *** --- */

	if (rp_isbad (io_init(rp)))
		adios (NULLCP, "io_init error: %s", rp -> rp_line);
	submit_running = 1;


	/* -- Returns the turn. x400in84 never transmits info -- */

	if (initial_turn == RTS_RESPONDER) {
		if (RtGTurnRequest (sd, rti) == NOTOK)
			rts_adios (rta, "RT-TURN-GIVE.REQUEST");
		turn_status = RTS_SENT_TURN;
		nturns++;
	}


	RTSFREE (rts);


	/* -- Sets up the RTS to receive incomming data -- */

	if (RtSetUpTrans (sd, rts_uptrans, rti) == NOTOK)
		rts_adios (rta, "RtSetUpTrans upcall");


	rts_get_request (sd);

	do_reason ("main/Impossible to arrive here!");

	exit (NOTOK);
}




void set_msg_adrs()
{
	ADDR		*ap;

	for (ap = PPQuePtr->Raddress; ap; ap = ap->ad_next)
		if (ap->ad_resp == NO)
			ap->ad_status = AD_STAT_DONE;
}




/* ---------------------  Static  Routines  ------------------------------- */




static void rts_get_request (sd)
int				sd;
{
	struct RtSAPindication	rti;
	int			loop_forever = TRUE,
				result;


	while (loop_forever) {
		switch (result = RtWaitRequest (sd, NOTOK, &rti)) {
		case NOTOK:
		case OK:
		case DONE:
			PP_TRACE (("rts_get_request (%d)", result));
			rts_indication (sd, &rti);
			break;
		default:
			do_reason ("rts_get_request/result='%d'", result);
		}
	}
}




static void rts_indication (sd, rti)
int			sd;
struct RtSAPindication	*rti;
{
	switch (rti->rti_type) {
	case RTI_TURN:
		PP_TRACE (("RTI_TURN"));
		if (rts_do_turn (sd, &rti->rti_turn) == NOTOK)
			rts_abort (sd, &rti->rti_abort);
		break;
	case RTI_TRANSFER:
		PP_TRACE (("RTI_TRANSFER"));
		/* -- rts_uptrans() does all the work -- */
		timer_end (&data_timer, data_bytes, "Transfer Completed");
		break;
	case RTI_ABORT:
		PP_TRACE (("RTI_ABORT"));
		rts_abort (sd, &rti->rti_abort);
		break;
	case RTI_CLOSE:
		PP_TRACE (("RTI_CLOSE"));
		rts_close (sd, &rti->rti_close);
		break;
	default:
		do_reason ("rts_indication/'%d' unknown", rti->rti_type);
		rts_abort (sd, &rti->rti_abort);
		break;
	}
}




static int rts_do_turn (sd, rtu)
int			sd;
struct RtSAPturn	*rtu;
{
	struct RtSAPindication rti;


	PP_TRACE (("rts_do_turn %s %s %d %s",
		initial_mode == RTS_TWA ? "twa" : "mon",
		initial_turn == RTS_RESPONDER ? "responder" : "initiator",
		nturns,
		turn_status == RTS_SENT_TURN ? "send" : "received"));


	/* -- Remote site is requesting the turn which x400in84	 -- */
	/* -- already possesses -- */


	if (rtu->rtu_please)
		if (turn_status == RTS_SENT_TURN) {
			do_reason ("rts_do_turn/Already sent turn!");
			return NOTOK;
		}
		else {
			/* -- should never have turn! but left for now -- */
			do_reason ("rts_do_turn/Should not have turn!");
			return NOTOK;
		}


	/* -- Remote site is transferring the turn to x400in84 -- */

	if (initial_turn == RTS_INITIATOR && initial_mode == RTS_MONOLOGUE) {
		do_reason ("rts_do_turn/Should not give turn in Monologue!");
		return NOTOK;
	}

	if (nturns > RTS_MAX_NO_OF_TURNS) {
		do_reason ("rts_do_turn/Too many turns occurred!");
		return NOTOK;
	}


	/* -- Returns turn but keeps count to prevent turn looping -- */

	if (RtGTurnRequest (sd, &rti) == NOTOK) {
		do_reason ("rts_do_turn/RT-TURN-GIVE.REQUEST failed");
		return NOTOK;
	}

	turn_status = RTS_SENT_TURN;
	nturns++;
	return OK;
}




/*ARGSUSED*/
static void rts_abort (sd, rta)
int				sd;
register struct RtSAPabort	*rta;
{

	if (submit_running)
		io_end (NOTOK);

	if (rta->rta_peer)
		rts_adios (rta, "RT-U-ABORT.INDICATION");

	if (RTS_FATAL (rta->rta_reason))
		rts_adios (rta, "RT-P-ABORT.INDICATION");

	rts_adios (rta, "RT-P-ABORT.INDICATION (error)");
}




/* ARGSUSED */
static void rts_close (sd, rtc)
int			sd;
struct RtSAPclose	*rtc;
{
	struct RtSAPindication		rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort	*rta = &rti->rti_abort;

	PP_TRACE (("RT-END.INDICATION %d", sd));

	if (RtEndResponse (sd, rti) == NOTOK)
		rts_adios (rta, "RT-END-RESPONSE.REQUEST error");

	if (submit_running)
		io_end (OK);

	PP_NOTICE (("Connection successfully terminated"));

	exit (0);

}




static struct qbuf  *fullqb = NULL;


/*ARGSUSED)*/
static int  rts_uptrans (sd, type, addr, rti)
int			sd;
int			type;
caddr_t			addr;
struct RtSAPindication	*rti;
{
	register struct SSAPactivity	*sv = (struct SSAPactivity *) addr;
	register struct SSAPsync	*sn = (struct SSAPsync *) addr;
	register struct SSAPreport	*sp = (struct SSAPreport *) addr;
	register struct qbuf		*qbp = (struct qbuf *) addr;
	PE				pe = NULLPE;
	int				len;


	PP_TRACE (("rts_uptrans()"));

	switch (type) {
	case SI_DATA:
		PP_TRACE (("SI_DATA"));

		dump_pdu (qbp, "Incoming X.400 data");
		len = qblen (qbp);
		data_bytes += len;
		PP_TRACE (("Data %d bytes (%d so far)", len, data_bytes));

		switch ((*asn_procfnx)(qbp)) {
		    case NOTOK:
		    default:
			io_end (NOTOK);
			submit_running = 0;
			return rtsaplose (rti, RTS_TRANSFER, NULLCP,
					  "process data failed");
		    case OK:
		    case DONE:
			break;
		}
		break;

	case SI_SYNC:
		PP_TRACE (("S-MINOR-SYNC.INDICATION: %ld", sn->sn_ssn));
		break;

	case SI_ACTIVITY:
		switch (sv->sv_type) {
		case SV_START:
			PP_TRACE (("S-ACTIVITY-START.INDICATION"));
			data_bytes = 0;
			timer_start (&data_timer);
			asn_init (hdrproc, bodyproc, 0);
			break;

		case SV_INTRIND:
		case SV_DISCIND:
			PP_LOG (LLOG_EXCEPTIONS,
				("activity %s: %s",
				 sv->sv_type == SV_INTRIND ?
				 "interrupted" : "discarded",
				 SReportString (sv->sv_reason)));
			break;

		case SV_ENDIND:
			PP_TRACE (("S-ACTIVITY-END.INDICATION"));
			PP_TRACE (("Accumulated '%d'", data_bytes));
			if (msgfinished() == NOTOK)
				return rtsaplose (rti, RTS_TRANSFER,
						  NULLCP,
						  "data termination failed");
			break;

		default:
			return (rtsaplose (rti, RTS_TRANSFER, NULLCP,
					"unexpected activity indication=0x%x",
					sv->sv_type));
		}
		break;


	case SI_REPORT:
		if (!sp->sp_peer)
			return (rtsaplose (rti, RTS_TRANSFER, NULLCP,
				"unexpected provider-initiated SI_REPORT"));
		do_reason ("rts_uptrans/Exception Report: '%s'",
				SReportString (sp->sp_reason));
		if (pe)
			pe_free (pe);
		break;

	default:
		return (rtsaplose (rti, RTS_TRANSFER, NULLCP,
			"unknown rts_uptrans type=0x%x", type));
	}

	return OK;
}




static int qblen (qbstart)
struct qbuf	*qbstart;
{
	int			len; 
	register struct qbuf	*qb;

	for (len = 0, qb = qbstart->qb_forw; qb != qbstart; qb = qb->qb_forw)
		len += qb -> qb_len;

	return len;
}




/* ---------------------  Postmaster Routines  ---------------------------- */




#if PP_DEBUG
static int do_debug_transfer()
{
	char buf[BUFSIZ];
	int	n;
	int	result = NOTOK;
	struct qbuf *qb;
	struct SSAPactivity ssa;
	struct RtSAPindication rti;
	extern char *loc_dom_mta;

	bzero ((char *)&ssa, sizeof ssa);
	bzero ((char *)&rti, sizeof rti);
	ssa.sv_type = SV_START;
	remote_site = loc_dom_mta;

	rts_uptrans (0, SI_ACTIVITY, (caddr_t)&ssa, &rti);
	while ((n = fread (buf, 1, sizeof buf, stdin)) > 0) {
		qb = str2qb (buf, n, 1);
		rts_uptrans (0, SI_DATA, (caddr_t)qb, &rti);
		qb_free (qb);
	}
	ssa.sv_type = SV_ENDIND;
	rts_uptrans (0, SI_ACTIVITY, (caddr_t)&ssa, &rti);
	if (result != DONE)
		advise (NULLCP, "Read message - not parsed");
}
#endif
