/* x400inmsg.c: X400(1988) handling of body and envelope */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400inmsg.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/x400inmsg.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: x400inmsg.c,v $
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
#include "prm.h"
#include "dr.h"
#include "retcode.h"
#include <isode/rtsap.h>
#include <stdarg.h>

#define bit_ison(x,y)	(bit_test(x,y) == 1)

extern char *remote_site;
extern char *undefined_bodyparts;
extern char *postmaster;
extern char *myname;
extern char *cont_p2, hdr_p2_bp;

enum errstate { st_normal, st_dr, st_probe, st_err_asn, st_err_submit, st_err_junk };
static enum errstate state =  st_normal;

extern	CHAN	*mychan;
extern int submit_running;
extern int canabort;
extern int rts_sd;

static int splatfnx (va_alist)
va_dcl
{
	char	buffer[BUFSIZ];
	caddr_t	junk;
	RP_Buf rps;

	va_list ap;

	va_start (ap);

	junk = va_arg (ap, caddr_t);

	_asprintf (buffer, NULLCP, ap);

	if (pps_txt (buffer, &rps) == NOTOK)
		adios (NULLCP, "Write fails: %s", rps.rp_line);
	va_end (ap);
}

int hdrproc (pe, type)
PE	pe;
int	type;
{
	struct type_Trans_MtsAPDU *parm;
	struct type_MTA_MessageTransferEnvelope *envelope;
	Q_struct qs, *qp = &qs;
	DRmpdu drs, *dr = &drs;
	struct prm_vars prm;
	RP_Buf	rps, *rp = &rps;
	ADDR	*ap;
	struct qbuf *qbret, *qb;

	if (type == NOTOK) {
		PP_OPER (NULLCP, ("Bad message"));
		resetforpostie (st_err_junk, pe, type,
				"Transfer is not ASN.1");
		return OK;
	}
	state = st_normal;

	if (submit_running == 0) {
		if (rp_isbad (io_init (rp)))
			adios (NULLCP, "Can't initialise submit: %s",
			       rp -> rp_line);
		submit_running = 1;
	}

	prm_init (&prm);
	prm.prm_opts = PRM_ACCEPTALL | PRM_NOTRACE;
	if (rp_isbad (io_wprm (&prm, rp))) {
		advise (LLOG_EXCEPTIONS, NULLCP, "io_wpm %s", rp -> rp_line);
		return NOTOK;
	}

	q_init (qp);
	switch (type) {
	    case MT_UMPDU:
		PP_PDUP (MTA_MessageTransferEnvelope, pe,
			"MTA.MessageTransferEnvelope", PDU_READ);
		if (decode_MTA_MessageTransferEnvelope
		    (pe, 1, NULLIP, NULLVP, &envelope)
		    == NOTOK) {
			PP_OPER(NULLCP, ("Parse of P1 failed [%s]", PY_pepy));
			resetforpostie (st_err_asn, pe, type,
					"ASN.1 is NOT P1");
			return OK;
		}
		else {
			switch (load_qstruct (qp, envelope)) {
			    case NOTOK:
				adios (NULLCP, "Transient problem with ASN");
			    case OK:
				break;
			    case DONE:
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Some problem with the P1"));
				resetforpostie (st_err_submit, pe, type,
						"ASN.1 has a problem");
				return OK;
			}
		}
		break;
		
	    case MT_PMPDU:
		PP_PDUP (Trans_MtsAPDU, pe,
			"Trans.MtsAPDU", PDU_READ);
		if (decode_Trans_MtsAPDU (pe, 1, NULLIP, NULLVP, &parm)
		    == NOTOK) {
			PP_OPER (NULLCP, ("Parse of P1 failed [%s]", PY_pepy));
			resetforpostie (st_err_asn, pe, type,
					"ASN.1 is not a Probe");
			return OK;
		}
		else {
			switch (load_probe (qp, parm -> un.probe)) {
			    case NOTOK:
				adios (NULLCP, "Transient problem with ASN");
			    case OK:
				break;
			    case DONE:
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Some problem with the P1"));
				resetforpostie (st_err_submit, pe, type,
						"ASN.1 has a problem");
				return OK;
			}
		}
		state = st_probe;
		break;

	    case MT_DMPDU:
		PP_PDUP (Trans_MtsAPDU, pe,
			"Trans.MtsAPDU", PDU_READ);
		if (decode_Trans_MtsAPDU (pe, 1, NULLIP, NULLVP, &parm)
		    == NOTOK) {
			PP_OPER (NULLCP, ("Parse of P1 failed [%s]", PY_pepy));
			resetforpostie (st_err_asn, pe, type,
					"ASN.1 is not a DR");
			return OK;
		}
		else {
			dr_init (dr);
			switch (load_dr (qp, dr, parm -> un.report)) {
			    case NOTOK:
				adios (NULLCP, "Transient problem with ASN");
			    case OK:
				break;
			    case DONE:
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Some problem with the P1"));
				resetforpostie (st_err_submit, pe, type,
						"ASN.1 has a problem");
				return OK;
			}
			if ( parm -> un.report -> content ->
			    returned__content) {
				if (list_bpt_find (qp -> encodedinfo.eit_types,
						   hdr_p2_bp) == NULL)
					list_bpt_add(&qp -> encodedinfo.eit_types,
						     list_bpt_new(hdr_p2_bp));
				qp -> content_return_request = TRUE;
			}
			else {
				list_bpt_free (&qp -> encodedinfo.eit_types);	
				qp -> encodedinfo.eit_types = NULL;
			}
			if (qp -> cont_type == NULLCP)
				qp -> cont_type = strdup(cont_p2);
		}
		state = st_dr;
		break;
	    default:
		PP_OPER (NULLCP, ("Unknown type of structure %d",
				  type));
		exit (1);
	}
	qp -> inbound = list_rchan_new (remote_site, NULL);
	qp -> inbound -> li_chan = ch_mta2struct (myname, remote_site);
		

	if (rp_isbad (io_wrq (qp, rp))) {
		advise (LLOG_EXCEPTIONS, NULLCP, "io_wrq %s", rp -> rp_line);
		return NOTOK;
	}

	if (rp_isbad (io_wadr (qp -> Oaddress, AD_ORIGINATOR, rp))) {
		char ebuf[BUFSIZ];
		advise (LLOG_EXCEPTIONS, NULLCP, "io_wadr %s", rp -> rp_line);
		if (rp_gbval(rp -> rp_val) == RP_BNO) {
			(void) sprintf (ebuf, "Can't set sender %s: %s",
					qp -> Oaddress -> ad_value,
					rp -> rp_line);
			resetforpostie (st_err_submit, pe, type, ebuf);
			return OK;
		}
		return NOTOK;
	}
	PP_NOTICE (("Originator %s", qp -> Oaddress -> ad_value));

	for (ap = qp -> Raddress; ap; ap = ap -> ad_next) {
		if (ap -> ad_resp == NO)   ap -> ad_status = AD_STAT_DONE;

		if (rp_isbad (io_wadr (ap, AD_RECIPIENT, rp))) {
			advise (LLOG_EXCEPTIONS, NULLCP,
				"io_wadr %s", rp -> rp_line);
			return NOTOK;
		}
		if (ap -> ad_resp)
			PP_NOTICE (("Recipient Address %s", ap -> ad_value));
		else
			PP_NOTICE (("Recipinet (no responsibility) %s",
				    ap -> ad_value));
	}

	if (rp_isbad (io_adend (rp))) {
		advise (LLOG_EXCEPTIONS, NULLCP, "io_adend %s",
			rp -> rp_line);
		return NOTOK;
	}

	switch (qp -> msgtype) {
	    case MT_UMPDU:
		if (rp_isbad (io_tinit (rp))) {
			advise (LLOG_EXCEPTIONS, NULLCP,
				"io_tinit %s", rp -> rp_line);
			return NOTOK;
		}
	
		if (rp_isbad (io_tpart (qp -> cont_type, FALSE, rp))) {
			advise (LLOG_EXCEPTIONS, NULLCP, "io_tpart %s %s",
				qp -> cont_type, rp -> rp_line);
			return NOTOK;
		}
		break;

	    case MT_DMPDU:
		if (rp_isbad (io_wdr (dr, rp))) {
			advise (LLOG_EXCEPTIONS, NULLCP,
				"io_wdr %s", rp -> rp_line);
			return NOTOK;
		}
		if (rp_isbad (io_tinit (rp))) {
			advise (LLOG_EXCEPTIONS, NULLCP,
				"io_tinit %s", rp -> rp_line);
			return NOTOK;
		}
		if ((qbret = parm -> un.report -> content ->
		    returned__content) != NULL) {

			if (rp_isbad (io_tpart (qp -> cont_type, FALSE, rp))) {
				advise (LLOG_EXCEPTIONS, NULLCP,
					"io_tpart %s %s",
					qp -> cont_type, rp -> rp_line);
				return NOTOK;
			}
			for (qb = qbret -> qb_forw; qb != qbret;
			     qb = qb -> qb_forw) {
				if (rp_isbad (io_tdata (qb -> qb_data,
							qb -> qb_len))) {
					advise (LLOG_EXCEPTIONS, NULLCP,
						"io_tdata failed");
					return NOTOK;
				}
			}
			if (rp_isbad (io_tdend (rp))) {
				advise (LLOG_EXCEPTIONS, NULLCP,
					"io_tdend %s", rp -> rp_line);
				return NOTOK;
			}
		}
		if (rp_isbad (io_tend (rp))) {
			advise (LLOG_EXCEPTIONS, NULLCP,
				"io_tend %s", rp -> rp_line);
			return NOTOK;
		}
		break;

	    case MT_PMPDU:
		break;
	}
	return OK;
}

resetforpostie (st, pe, type, str)
enum errstate st;
PE	pe;
int	type;
char	*str;
{
	static char line[] = "\n\n----------------------------------------\n\n";
	char	*msg = "<Error>";
	PS	ps;
	RP_Buf	rps, *rp = &rps;

	if (submit_running) {
		io_end (NOTOK);
		submit_running = 0;
	}
			
	switch (state = st) {
	    case st_err_submit:
		msg = "Submission Error";
		break;
	    case st_err_asn:
		msg = "ASN.1 Parsing error";
		break;
	    case st_err_junk:
		msg = "Invalid ASN.1";
		break;
	    default:
		adios (NULLCP, "Bad state in resetforpostie %d", st);
	}
	if (canabort) {
		sendrtsabort ();
		adios (NULLCP, "Aborted submission: inbound error %s - %s",
		       msg, str);
	}

	if (pps_1adr (msg, postmaster, rp) == NOTOK)
		adios (NULLCP, "Can't initialize submit for error report: %s",
		       rp -> rp_line);

	if (pps_txt ("X.400 inbound error detected\n\t", rp) == NOTOK ||
	    pps_txt (str, rp) == NOTOK ||
	    pps_txt ("\nThe message was received from ", rp) == NOTOK ||
	    pps_txt (remote_site ? remote_site : "<unknown-site>", rp) == NOTOK)
		adios (NULLCP, "Error writing data to submit: %s",
		       rp -> rp_line);

	switch (st) {
	    case st_err_asn:
		msg = "\n\nA dump of the ASN.1 follows:\n\n";
		break;
	    case st_err_junk:
		msg = "\n\nA hex dump of the incoming message follows:\n\n";
		break;
	    case st_err_submit:
		msg = "\n\nA trace of the P1 envelope follows:\n\n";
		break;
	}
	if (pps_txt (msg, rp) == NOTOK ||
	    pps_txt (line, rp) == NOTOK)
		adios (NULLCP, "Error writing to submit: %s", rp -> rp_line);

	if (st == st_err_junk)
		return;

	switch (type) {
	    case MT_DMPDU:
		vpushpp (stdout, splatfnx, pe, "DR MPDU", PDU_WRITE);
		break;
	    case MT_PMPDU:
		vpushpp (stdout, splatfnx, pe, "Probe MPDU", PDU_WRITE);
		break;
	    case MT_UMPDU:
		vpushpp (stdout, splatfnx, pe, "User MPDU", PDU_WRITE);
		break;
	}
	switch (st) {
	    case st_err_asn:
		vunknown (pe);
		break;

	    case st_err_submit:
		switch (type) {
		    case MT_DMPDU:
		    case MT_PMPDU:
			print_Trans_MtsAPDU(pe, 1, NULLIP, NULLVP, NULLCP);
			break;
		    case MT_UMPDU:
			print_MTA_MessageTransferEnvelope (pe, 1, NULLIP,
							   NULLVP, NULLCP);
			break;
		}
	}
	vpopp ();
	if (pps_txt ("\n\nHEX dump of this data now follows", rp) == NOTOK ||
	    pps_txt (line, rp) == NOTOK)
		adios (NULLCP, "Can't write to submit: %s", rp -> rp_line);
	if ((ps = ps_alloc (str_open)) == NULLPS)
		adios (NULLCP, "Can't allocate PS stream");
	if (str_setup (ps, NULLCP, 0, 0) == NOTOK)
		adios (NULLCP, "Can't setup PS stream");
	if (pe2ps (ps, pe) == NOTOK)
		adios (NULLCP, "pe2ps failed: %s", ps_error (ps -> ps_errno));
	bodyproc (ps -> ps_base, ps -> ps_ptr - ps -> ps_base);
	ps_free (ps);
	
	if (type == MT_UMPDU) {
		if (pps_txt ("\n\nP2 hex dump follows", rp) == NOTOK ||
		    pps_txt (line, rp) == NOTOK)
			adios (NULLCP, "Can't write to submit: %s",
			       rp -> rp_line);
	}
}

bodyproc (str, len)
char	*str;
int	len;
{
	char	hexbuf[82];
	int	i;
	RP_Buf	rps, *rp = &rps;

	PP_TRACE (("Copy %d bytes", len));
	switch (state) {
	    case st_normal:
		if (rp_isbad (io_tdata (str, len))) {
			PP_LOG (LLOG_EXCEPTIONS, ("data write failed"));
			return NOTOK;
		}
		break;
	    case st_probe:
	    case st_dr:
		PP_LOG (LLOG_EXCEPTIONS, ("Illegal state in bodyproc"));
		break;

	    case st_err_submit:
	    case st_err_asn:
	    case st_err_junk:
		for (i = 0; i <= len; i += 40) {
			int n = min (40, len - i);
			n = explode (hexbuf, str + i, n);
			hexbuf[n++] = '\n';
			hexbuf[n] = 0;
			if (pps_txt (hexbuf, rp) == NOTOK)
				adios (NULLCP, "Error writing to submit: %s",
				       rp -> rp_line);
		}
		break;
	}
	return OK;
}

msgfinished ()
{
	RP_Buf rps, *rp = &rps;

	switch (state) {
	    case st_normal:
		if (rp_isbad (io_tdend (rp)) ||
		    rp_isbad (io_tend (rp))) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Data termination failed: %s",
				 rp -> rp_line));
			return NOTOK;
		}
		PP_NOTICE (("<<< Message received from %s",
			    remote_site));
		break;
	    case st_probe:
		PP_NOTICE (("<<< Probe received from %s",
			    remote_site));
		break;
	    case st_dr:
		PP_NOTICE (("<<< DR received from %s",
			    remote_site));
		break;

	    case st_err_asn:
	    case st_err_submit:
	    case st_err_junk:
		PP_NOTICE (("<<< Invalid message received from %s",
			    remote_site));
		if (pps_end (OK, rp) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("final handshake failed: %s",
				 rp -> rp_line));
			return NOTOK;
		}
		break;
	}
	return OK;
}
