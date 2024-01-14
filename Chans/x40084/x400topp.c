/* x400topp.c: X400(1984) protocol to submit format - inbound */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40084/RCS/x400topp.c,v 6.0 1991/12/18 20:13:50 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/x400topp.c,v 6.0 1991/12/18 20:13:50 jpo Rel $
 *
 * $Log: x400topp.c,v $
 * Revision 6.0  1991/12/18  20:13:50  jpo
 * Release 6.0
 *
 */



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

extern char		*remote_site;
extern char		*postmaster;
extern int		submit_running;
extern CHAN		*mychan;
extern Q_struct		*PPQuePtr;
extern DRmpdu		*DRptr;
extern char		*pp_myname;
extern char		*cont_p2;
int			log_msgtype = 0;  /* -- logging of msgtypes -- */

enum errstate { 
	st_normal, st_dr, st_probe, st_err_asn, st_err_submit, st_err_junk
};
static enum errstate	state =  st_normal;
static int		do_extra_encodedtypes ();



/* -- local routines -- */
int			bodyproc();
int			hdrproc();
int			msgfinished();
static int		do_extra_encodedtypes();
static int		splatfnx();
static int		rebuild_eits(), rebuild_dreits();
static void		resetforpostie();




/* -------------------  Begin  Routines  ------------------------------------ */




int hdrproc (pe, type)
PE	pe;
int	type;
{
	extern char *body_string;
	extern int body_len;
	ADDR			*ap;
	struct prm_vars		prm;
	RP_Buf			rps, *rp = &rps;


	PP_TRACE (("hdrproc (type = '%d')", type));

	if (type == NOTOK) {
		PP_OPER (NULLCP, ("Bad message"));
		resetforpostie (st_err_junk, pe, type, "Transfer is not ASN.1");
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

	if (rp_isbad (io_wprm (&prm, rp)))
		adios (NULLCP, "io_wpm failed %s", rp -> rp_line);


	q_init (PPQuePtr);
	dr_init (DRptr);
	PPQuePtr -> msgtype = log_msgtype = type;


	switch (type) {
	case MT_UMPDU:
		PP_PDU (print_P1_UMPDUEnvelope, pe,
			"P1.UMPDUEnvelope", PDU_READ);

		PPQuePtr -> inbound = list_rchan_new (remote_site,
						      NULL);
		PPQuePtr -> inbound -> li_chan = ch_mta2struct (pp_myname,
								remote_site);

		if (do_P1_UMPDUEnvelope (pe, 1, NULLIP, NULLVP, PPQuePtr)
		    == NOTOK) {
			char buf[BUFSIZ];
			(void) sprintf (buf, "Parse of P1 failed [%s]", PY_pepy);
			PP_OPER (NULLCP, ("%s", buf));
			resetforpostie (st_err_asn, pe, type,
					buf);
			return OK;
		}

		(void) rebuild_eits (&PPQuePtr -> encodedinfo,
				     &PPQuePtr -> orig_encodedinfo,
				     PPQuePtr -> trace);

		do_extra_encodedtypes (&PPQuePtr -> encodedinfo);
		break;
		

	case MT_PMPDU:
		PP_PDU (print_P1_MPDU, pe,
			"P1.MPDU(Probe)", PDU_READ);

		if (do_P1_MPDU (pe, 1, NULLIP, NULLVP, NULL)
		    == NOTOK) {
			char buf[BUFSIZ];
			(void) sprintf (buf, "Parse of P1 probe failed [%s]", PY_pepy);
			PP_OPER (NULLCP, ("%s", buf));
			resetforpostie (st_err_asn, pe, type,
					buf);
			return OK;
		}

		state = st_probe;
		(void) rebuild_eits (&PPQuePtr -> encodedinfo,
				     &PPQuePtr -> orig_encodedinfo,
				     PPQuePtr -> trace);
		do_extra_encodedtypes (&PPQuePtr -> encodedinfo);
		break;


	case MT_DMPDU:
		PP_PDU (print_P1_MPDU, pe,
			"P1.MPDU(DR)", PDU_READ);

		body_string = NULL;
		if (do_P1_MPDU (pe, 1, NULLIP, NULLVP, NULL)
		    == NOTOK) {
			char buf[BUFSIZ];
			(void) sprintf (buf, "Parse of P1 DR failed [%s]", PY_pepy);
			PP_OPER (NULLCP, ("%s", buf));
			resetforpostie (st_err_asn, pe, type,
					buf);
			return OK;
		}

		state = st_dr;
		(void) rebuild_dreits (&PPQuePtr -> encodedinfo, DRptr);
		if (body_string != NULLCP) {
			do_extra_encodedtypes (&PPQuePtr -> encodedinfo);
			PPQuePtr -> content_return_request = TRUE;
		}
		else {
			list_bpt_free (PPQuePtr -> encodedinfo.eit_types);
			PPQuePtr -> encodedinfo.eit_types = NULL;
		}
		PPQuePtr -> cont_type = strdup (cont_p2);
		break;


	default:
		adios (NULLCP, "Unknown type of structure %d", type);
	}



	set_msg_adrs();

	if (rp_isbad (io_wrq (PPQuePtr, rp)))
		adios (NULLCP, "io_wrq %s", rp -> rp_line);


	PP_NOTICE (("Originator %s", PPQuePtr -> Oaddress -> ad_value));


	if (rp_isbad (io_wadr (PPQuePtr -> Oaddress, AD_ORIGINATOR, rp))) {
		char ebuf[BUFSIZ];
		do_reason ("io_wadr %s", rp -> rp_line);

		if (rp_gbval(rp -> rp_val) == RP_BNO) {
			(void) sprintf (ebuf, "Bad originator address %s: %s",
					PPQuePtr -> Oaddress -> ad_value,
					rp -> rp_line);
			resetforpostie (st_err_submit, pe, type, ebuf);
			return OK;
		}

		exit (NOTOK);
	}



	for (ap = PPQuePtr -> Raddress; ap; ap = ap -> ad_next) {

		if (ap -> ad_resp)
			PP_NOTICE (("Recipient Address %s", ap -> ad_value));
		else
			PP_NOTICE (("Recipient (no responsibility) %s",
				    ap -> ad_value));

		if (rp_isbad (io_wadr (ap, AD_RECIPIENT, rp))) {
			PP_LOG(LLOG_EXCEPTIONS, ("io_wadr %s", rp -> rp_line));
			return NOTOK;
		}
	}

	if (rp_isbad (io_adend (rp))) 
		adios (NULLCP, "io_adend %s", rp -> rp_line);


	switch (PPQuePtr -> msgtype) {
	    case MT_UMPDU:
		if (rp_isbad (io_tinit (rp)))
			adios (NULLCP, "io_tinit %s", rp -> rp_line);
	
		if (rp_isbad (io_tpart (PPQuePtr -> cont_type, FALSE, rp)))
			adios (NULLCP, "io_tpart %s %s",
				PPQuePtr -> cont_type, rp -> rp_line);
		break;

	    case MT_DMPDU:
		if (rp_isbad (io_wdr (DRptr, rp)))
			adios (NULLCP, "io_wdr %s", rp -> rp_line);
		if (rp_isbad (io_tinit (rp))) {
			advise (LLOG_EXCEPTIONS, NULLCP,
				"io_tinit %s", rp -> rp_line);
			return NOTOK;
		}
		if (body_string) {
			if (rp_isbad (io_tpart (PPQuePtr -> cont_type,
						FALSE, rp))) {
                                advise (LLOG_EXCEPTIONS, NULLCP,
                                        "io_tpart %s %s",
                                        PPQuePtr -> cont_type, rp -> rp_line);
                                return NOTOK;
                        }
			if (rp_isbad (io_tdata (body_string, body_len))) {
				advise (LLOG_EXCEPTIONS, NULLCP,
					"io_tdata failed");
				return NOTOK;
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




int bodyproc (str, len)
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




int msgfinished ()
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




/* -------------------  Static Routines  ------------------------------------ */




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

	PP_TRACE (("splatfnx '%s'", buffer));

	if (pps_txt (buffer, &rps) == NOTOK)
		adios (NULLCP, "Write fails: %s", rps.rp_line);
	va_end (ap);
}




static int rebuild_dreits (eits, dr)
EncodedIT *eits;
DRmpdu *dr;
{
	Trace	*tp;
	Rrinfo *rr;
	LIST_BPT *ep;

	for (rr = dr -> dr_recip_list; rr; rr = rr-> rr_next)  {
		if (rr -> rr_report.rep_type == DR_REP_FAILURE &&
		    rr -> rr_converted)
			for (ep = rr -> rr_converted->eit_types; ep;
			     ep = ep->li_next)
				if (list_bpt_find(eits -> eit_types, ep -> li_name) == NULL)
					list_bpt_add (&eits -> eit_types,
						      list_bpt_new (ep->li_name));
	}
	return OK;
}

static int rebuild_eits (eits, orig, trace)
EncodedIT *eits, *orig;
Trace	*trace;
{
	Trace	*tp;
	LIST_BPT *lasteit = NULL;

	for (tp = trace; tp; tp = tp -> trace_next)
		if (tp -> trace_DomSinfo.dsi_converted.eit_types)
			lasteit = tp -> trace_DomSinfo.dsi_converted.eit_types;
	if (lasteit)
		list_bpt_add (&eits -> eit_types, list_bpt_dup (lasteit));
	else
		list_bpt_add (&eits -> eit_types,
			      list_bpt_dup (orig -> eit_types));
	return OK;
}




static void resetforpostie (st, pe, type, str)
enum errstate st;
PE	pe;
int	type;
char	*str;
{
	static char line[] = "\n\n----------------------------------------\n\n";
	char	*msg = "<Error>";
	RP_Buf	rps, *rp = &rps;
	PS	ps;

	PP_NOTICE (("Resending the %s to Postmaster", 
			type == MT_DMPDU ? "Delivery Report" : 
			type == MT_PMPDU ? "Probe" : "Message"));

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
			print_P1_MPDU(pe, 1, NULLIP, NULLVP, NULLCP);
			break;
		    case MT_UMPDU:
			print_P1_UMPDUEnvelope (pe, 1, NULLIP,
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




static int do_extra_encodedtypes (ep)
EncodedIT	*ep;
{
	extern char *hdr_p2_bp;
	LIST_BPT	*base = NULLIST_BPT,
			*new;


	if ((new = list_bpt_new (hdr_p2_bp)) == NULLIST_BPT)
		return NOTOK;
	list_bpt_add (&base, new);

	if (ep ->eit_types)
		list_bpt_add (&base, ep->eit_types);

	ep->eit_types = base;

	return OK;
}
