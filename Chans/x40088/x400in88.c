/* x400in88.c: X400(1988) protocol machine - inbound */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400in88.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400in88.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: x400in88.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */



#include "Trans-types.h"
#include "util.h"
#include "chan.h"
#include "q.h"
#include "adr.h"
#include "or.h"
#include "retcode.h"
#include <isode/rtsap.h>
#include <sys/time.h>
#include "RTS84-types.h"

extern char *quedfldir;
extern char *remote_site;

char	*myname;
CHAN	*mychan;
int	submit_running;
int	canabort = 0;

extern IFP asn_procfnx;
extern int hdrproc (), bodyproc ();

static struct timeval data_timer;
static int data_bytes = 0;
static enum { x400_1988, x400_1984 } stack = x400_1984;
static int rts_sd;
static void rts_adios ();
static void rts_advise ();
static int rts_close ();
static int rts_indication ();
static int rts_turn ();
static int rts_abort ();
static int rts_finish ();
static int rts_uptrans ();
static char *SReportString ();
void	advise (), adios ();
#if PP_DEBUG >= PP_DEBUG_SOME
static int do_debug_transfer ();
static int waittime = 0;
#endif

main (argc, argv)
int	argc;
char	**argv;
{
	int	sd = -1;
	RP_Buf rps, *rp = &rps;
	int	opt;
	extern int optind;
	extern char *optarg;

	if (myname = rindex (argv[0], '/'))
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = argv[0];

	while ((opt = getopt (argc, argv, "c:now:")) != EOF) {
		switch (opt) {
		    case 'n':
			stack = x400_1988;
			break;
		    case 'o':
			stack = x400_1984;
			break;
		    case 'c':
			myname = optarg;
			break;
		    case 'w':
#if PP_DEBUG >= PP_DEBUG_SOME
			waittime = atoi(optarg);
#endif
			break;
		    default:
			adios (NULLCP, "Unknown argument -%c", opt);
		}
	}
#if PP_DEBUG >= PP_DEBUG_SOME
	if (waittime > 0)
		sleep (waittime);
#endif
	if (stack == x400_1988)
		canabort = 1;
	chan_init (myname);

	if ((mychan = ch_nm2struct (myname)) == NULLCHAN)
		adios (NULLCP, "No channel %s", myname);

	pp_setuserid();

	(void) chdir (quedfldir);

	or_myinit();

	if (rp_isbad (io_init(rp)))
		adios (NULLCP, "Initialisation error %s", rp -> rp_line);
	submit_running = 1;

	PP_NOTICE (("Starting %s (%s)", mychan->ch_name, mychan->ch_show));

#if PP_DEBUG >= 0
	if (argv[optind] && strcmp (argv[optind], "debug") == 0) {
		do_debug_transfer ();
		exit(0);
	}
#endif
	switch (stack) {
	    case x400_1984:
		sd = rts_init_1984 (argc, argv);
		break;

	    case x400_1988:
		sd = rts_init_1988 (argc, argv);
		break;

	    default:
		adios (NULLCP, "Illegal stack");
		break;
	}

	rts_receive (sd);

	exit (0);
}

static int rts_uptrans ();

static int check_contexts (ctxlist)
struct PSAPctxlist *ctxlist;
{
	OID	a_ctx, r_ctx, m_ctx;
	int	a,r,m, i;
	struct PSAPcontext *pp;

	a = r = m = 0;
#define aCSE	"2.2.1.0.1"
#define rTSE	"2.6.0.2.12"
#define mTSE	"2.6.0.2.7"
	a_ctx = str2oid (aCSE);
	r_ctx = str2oid (rTSE);
	m_ctx = str2oid (mTSE);
	for (i = 0, pp = ctxlist -> pc_ctx;
	     i < ctxlist -> pc_nctx; pp++, i++) {
		if (oid_cmp (pp -> pc_asn, a_ctx) == 0) {
			a = 1;
			continue;
		}
		if (oid_cmp (pp -> pc_asn, r_ctx) == 0) {
			r = 1;
			continue;
		}

		if (oid_cmp (pp -> pc_asn, m_ctx) == 0) {
			m = 1;
			continue;
		}

		advise (NULLCP, "Unexpected context %s",
			sprintoid (pp -> pc_asn));
	}
	if (a != 1)
		advise (NULLCP, "context %s not present", aCSE);
	if (r != 1)
		advise (NULLCP, "context %s not present", rTSE);
	if (m != 1)
		advise (NULLCP, "context %s not present", mTSE);
	oid_free (a_ctx);
	oid_free (m_ctx);
	oid_free (r_ctx);
	return OK;
}


rts_init_1988 (argc, argv)
int	argc;
char	**argv;
{
	struct RtSAPstart rtss;
	struct RtSAPstart *rts = &rtss;
	struct RtSAPindication rtis;
	struct RtSAPindication *rti = &rtis;
	struct RtSAPabort *rta = &rti -> rti_abort;
	struct AcSAPstart *acs = &rts -> rts_start;
	struct PSAPstart *ps = &acs -> acs_start;
	struct type_Trans_Bind1988Result *res;
	PE	accept_pe;
	OID	r_ctx;

	r_ctx = oid_cpy (str2oid(rTSE));
	PP_TRACE (("rts_init_1988 ()"));

	if (RtInit_Aux (argc, argv, rts, rti, r_ctx) == NOTOK)
		rts_adios (rta, "RtInit");

        advise (LLOG_NOTICE, NULLCP, "RT-OPEN.INDICATION: <%d, %s, %s, 0x%x>",
                rts -> rts_sd,
                rts -> rts_mode == RTS_TWA ? "twa" : "mono",
                rts -> rts_turn == RTS_RESPONDER ? "responder" : "initiator",
                rts -> rts_data);

        advise (LLOG_NOTICE, NULLCP, "ACSE: <%d, %s, %s, %s, %d>",
                acs -> acs_sd, oid2ode (acs -> acs_context),
                sprintaei (&acs -> acs_callingtitle),
                sprintaei (&acs -> acs_calledtitle), acs -> acs_ninfo);

        advise (LLOG_NOTICE, NULLCP,
                "PSAP: <%d, %s, %s, %d, %d, %d>",
                ps -> ps_sd,
                paddr2str (&ps -> ps_calling, NULLNA),
                paddr2str (&ps -> ps_called, NULLNA),
                ps -> ps_isn, ps -> ps_ssdusize);
	rts_sd = rts -> rts_sd;

	if (check_contexts (&ps -> ps_ctxlist) == NOTOK) {
		(void) RtOpenResponse (rts_sd, ACS_PERMANENT, NULLOID, NULLAEI,
					&ps -> ps_called, NULLPC,
					ps -> ps_defctxresult,
					NULLPE,
					rti);
		adios (NULLCP, "Bad presentation context");
	}

	if (rts -> rts_data == NULLPE) {
		(void) RtOpenResponse (rts_sd, ACS_PERMANENT, NULLOID, NULLAEI,
				       &ps -> ps_called, NULLPC,
				       ps -> ps_defctxresult,
				       int2prim (int_MTA_MTABindError_authentication__error),
				       rti);
		adios (NULLCP, "No initial user data");
	}
	PP_PDUP (Trans_Bind1988Argument, rts -> rts_data,
		"RTS88.BindArgument", PDU_READ);

	if (rts_decode_request (rts, &accept_pe, &res, mychan, 1) != OK) {
		PP_OPER (NULLCP, ("rts_decode_request failed"));
		(void) RtOpenResponse (rts_sd, ACS_PERMANENT, NULLOID,
				       NULLAEI, &ps -> ps_called, NULLPC,
				       ps -> ps_defctxresult,
				       int2prim (int_MTA_MTABindError_authentication__error),
				       rti);
		adios (NULLCP, "Connection aborted");
	}

	PP_PDUP (Trans_Bind1988Result, accept_pe,
		"RTS88.BindResult", PDU_WRITE);

	RTSFREE (rts);

	if (RtOpenResponse (rts_sd, ACS_ACCEPT, NULLOID, NULLAEI,
			    &ps -> ps_called, NULLPC, ps -> ps_defctxresult,
			    accept_pe, rti) == NOTOK)
		rts_adios (rta, "RT-OPEN.RESPONSE");

	free_Trans_Bind1988Result (res);
	if (RtSetUpTrans (rts_sd, rts_uptrans, rti) == NOTOK)
		rts_adios (rta, "RtSetUpTrans");

	return rts_sd;
}

rts_init_1984 (argc, argv)
int	argc;
char	**argv;
{
	struct RtSAPstart rtss;
	struct RtSAPstart *rts = &rtss;
	struct RtSAPindication rtis;
	struct RtSAPindication *rti = &rtis;
	struct RtSAPabort *rta = &rti -> rti_abort;
	struct type_RTS84_Request	*request = 0; /* XXX */
	int				retval;
	PE	accept_pe;
	struct SSAPaddr                 *sai = &rts -> rts_initiator.rta_addr;
	struct TSAPaddr                 *tai = &sai -> sa_addr;
	struct NSAPaddr                 *nai = tai -> ta_addrs;

	if (RtBInit (argc, argv, rts, rti) == NOTOK)
		rts_adios (rta, "RtBInit");

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

	PP_DBG (("more ....  <<%s, '%s' '%d'>, '%s' '%d'>",
		 na2str (nai),
		 &tai->ta_selector[0], tai->ta_selectlen,
		 &sai->sa_selector[0], sai->sa_selectlen));

	rts_sd = rts -> rts_sd;

	if (rts -> rts_data == NULLPE) {
		(void) RtBeginResponse (rts_sd, RTS_VALIDATE, NULLPE, rti);
		adios (NULLCP, "No initial user data");
	}

	PP_PDUP (RTS84_Request, rts -> rts_data,
		"RTS84.Request", PDU_READ);

	if ((retval = rts_decode_request (rts, &accept_pe,
					  &request, mychan, 0)) != OK) {
		PP_OPER (NULLCP, ("Can't decode incoming request"));
		RtBeginResponse (rts_sd, retval, NULLPE, rti);
		adios (NULLCP, "Connection aborted");
	}

	PP_PDUP (RTS84_Request, accept_pe, "RTS84.Response", PDU_WRITE);

	RTSFREE (rts);

	if (RtBeginResponse (rts_sd, RTS_ACCEPT, accept_pe, rti) == NOTOK)
		rts_adios (rta, "RT-BEGIN.RESPONSE");

	if (RtSetUpTrans (rts_sd, rts_uptrans, rti) == NOTOK)
		rts_adios (rta, "RtSetUpTrans");

	return rts_sd;
}

rts_receive (sd)
int	sd;
{
	int	result;
	struct RtSAPindication rtis;
	struct RtSAPindication *rti = &rtis;

	for (;;) {
		switch (result = RtWaitRequest (sd, NOTOK, rti)) {
		    case NOTOK:
		    case OK:
		    case DONE:
			rts_indication (sd, rti);
			break;

		    default:
			adios (NULLCP, "Unknown return from RoWaitRequest=%d",
			       result);
		}
	}
}

static int rts_indication (sd, rti)
int     sd;
register struct RtSAPindication *rti;
{
	switch (rti -> rti_type) {
	    case RTI_TURN:
		rts_turn (sd, &rti -> rti_turn);
		break;

	    case RTI_TRANSFER:
		if (data_bytes)
			timer_end (&data_timer, data_bytes,
				   "Transfer Completed");
		break;

	    case RTI_ABORT:
		rts_abort (sd, &rti -> rti_abort);
		break;

	    case RTI_CLOSE:
		rts_close (sd, &rti -> rti_close);
		break;

	    case RTI_FINISH:
		rts_finish (sd, &rti -> rti_finish);
		break;

	    default:
		adios (NULLCP, "unknown indication type=%d", rti -> rti_type);
	}
}

static rts_turn (sd, rtu)
int     sd;
register struct RtSAPturn *rtu;
{
	struct RtSAPindication  rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort *rta = &rti -> rti_abort;

	if (rtu -> rtu_please) {
		if (RtGTurnRequest (sd, rti) == NOTOK)
			rts_adios (rta, "RT-TURN-GIVE.REQUEST");
	}
}

static int  rts_abort (sd, rta)
int     sd;
register struct RtSAPabort *rta;
{
	if (rta -> rta_peer)
		rts_adios (rta, "RT-U-ABORT.INDICATION");

	if (RTS_FATAL (rta -> rta_reason))
		rts_adios (rta, "RT-P-ABORT.INDICATION");
	rts_advise (rta, "RT-P-ABORT.INDICATION");
}


static int  rts_close (sd, rtc)
int     sd;
struct RtSAPclose *rtc;
{
	struct RtSAPindication  rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort *rta = &rti -> rti_abort;

	advise (LLOG_NOTICE, NULLCP, "RT-END.INDICATION");

	if (stack == x400_1988)
		advise (LLOG_EXCEPTIONS, NULLCP, "rts_close - 1988mode!");

	if (RtEndResponse (sd, rti) == NOTOK)
		rts_adios (rta, "RT-END.RESPONSE");
	PP_NOTICE (("Normal disconnect from %s", remote_site));
	exit (0);
}

static int  rts_finish (sd, acf)
int     sd;
register struct AcSAPfinish *acf;
{
	struct RtSAPindication  rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort *rta = &rti -> rti_abort;

	if (stack == x400_1984)
		advise (LLOG_EXCEPTIONS, NULLCP, "rtsfinish - 1984 mode!");
	
	if (RtCloseResponse (sd, ACR_NORMAL, NULLPE, rti) == NOTOK)
		rts_adios (rta, "RT-CLOSE.RESPONSE");
	ACFFREE (acf);
	PP_NOTICE (("Normal disconnect from %s", remote_site));
	exit (0);
}

static void  rts_adios (rta, event)
register struct RtSAPabort *rta;
char   *event;
{
	rts_advise (rta, event);

	_exit (1);
}


static void  rts_advise (rta, event)
register struct RtSAPabort *rta;
char   *event;
{
	char    buffer[BUFSIZ];

	if (rta -> rta_cc > 0)
		(void) sprintf (buffer, "[%s] %*.*s",
				RtErrString (rta -> rta_reason),
				rta -> rta_cc, rta -> rta_cc, rta -> rta_data);
	else
		(void) sprintf (buffer, "[%s]",
				RtErrString (rta -> rta_reason));

	advise (LLOG_NOTICE, NULLCP, "%s: %s", event, buffer);
}

static int rts_uptrans (sd, type, addr, rti)
int                     sd;
int                     type;
caddr_t                 addr;
struct RtSAPindication  *rti;
{
	register struct SSAPactivity    *sv = (struct SSAPactivity *) addr;
	register struct SSAPsync        *sn = (struct SSAPsync *) addr;
	register struct SSAPreport      *sp = (struct SSAPreport *) addr;
	register struct qbuf            *qbp = (struct qbuf *) addr;
	PE                              pe = NULLPE;

	PP_TRACE (("rts_uptrans()"));

	switch (type) {
	    case SI_DATA:
		PP_DBG (("SI_DATA"));

		data_bytes += qbp -> qb_len;
		dump_pdu (qbp, "Incoming X.400 data");
		PP_TRACE (("Data %d bytes (%d so far)", qbp -> qb_len,
			   data_bytes));
		switch ((*asn_procfnx) (qbp)) {
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
		PP_DBG (("S-MINOR-SYNC.INDICATION: %ld", sn->sn_ssn));
		break;

	    case SI_ACTIVITY:
		switch (sv->sv_type) {
		    case SV_START:
			PP_DBG (("S-ACTIVITY-START.INDICATION"));
			data_bytes = 0;
			timer_start (&data_timer);
			asn_init (hdrproc, bodyproc,
				  stack == x400_1988 ? 1 : 0);
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
			PP_DBG (("S-ACTIVITY-END.INDICATION"));
			PP_TRACE (("Accumulated '%d'", data_bytes));
			if (msgfinished () == NOTOK)
				return rtsaplose (rti, RTS_TRANSFER, NULLCP,
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
		PP_LOG (LLOG_EXCEPTIONS,
			("Exception Report: %s",
			 SReportString (sp->sp_reason)));
		if (pe)
			pe_free (pe);
		break;

	    default:
		return (rtsaplose (rti, RTS_TRANSFER, NULLCP,
				   "unknown rts_uptrans type=0x%x", type));
	}

	return OK;
}


#if PP_DEBUG >= PP_DEBUG_SOME
static int do_debug_transfer ()
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

#include <stdarg.h>

static void adios (char *what, char* fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _ll_log (pp_log_oper, LLOG_FATAL, ap);
    _ll_log (pp_log_norm, LLOG_FATAL, ap);
    va_end (ap);
    _exit (1);
}

static void advise (char *what, char *fmt, ...)
{
    int     code;
    va_list ap;
    va_start (ap);
    code = va_arg (ap, int);
    _ll_log (pp_log_norm, code, ap);
    va_end (ap);
}

#define RC_BASE         0x80

static char *reason_err0[] = {
        "no specific reason stated",
        "user receiving ability jeopardized",
        "reserved(1)",
        "user sequence error",
        "reserved(2)",
        "local SS-user error",
        "unreceoverable procedural error"
        };


static int reason_err0_cnt = sizeof reason_err0 / sizeof reason_err0[0];

static char *reason_err8[] = {
        "demand data token"
        };

static int reason_err8_cnt = sizeof reason_err8 / sizeof reason_err8[0];

static char   *SReportString (code)
int     code;
{
        register int    fcode;
        static char     buffer[BUFSIZ];

        if (code == SP_PROTOCOL)
                return "SS-provider protocol error";

        code &= 0xff;
        if (code & RC_BASE)
                if ((fcode = code & ~RC_BASE) < reason_err8_cnt)
                        return reason_err8[fcode];
        else
                if (code < reason_err0_cnt)
                        return reason_err0[code];

        (void) sprintf (buffer, "unknown reason code 0x%x", code);
        return buffer;
}

sendrtsabort ()
{
	struct RtSAPindication  rtis;
	register struct RtSAPindication *rti = &rtis;
	register struct RtSAPabort *rta = &rti -> rti_abort;

	if(RtUAbortRequest (rts_sd, NULLPE, rti) == NOTOK)
		rts_advise (rta, "RT-U-ABORT.Request");
}
