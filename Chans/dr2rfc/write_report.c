/* write_report.c - Writes the DR struct as a Message -- */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/dr2rfc/RCS/write_report.c,v 6.0 1991/12/18 20:06:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/dr2rfc/RCS/write_report.c,v 6.0 1991/12/18 20:06:58 jpo Rel $
 *
 * $Log: write_report.c,v $
 * Revision 6.0  1991/12/18  20:06:58  jpo
 * Release 6.0
 *
 */



/* --- *** ---
 * These routines create 2 text files containing body part info:
 * The files are called "hdr.822.uk" and "1.ia5". These files are
 * placed by submit into the $Q/msg directory
 * --- *** --- */


#include "head.h"
#include "q.h"
#include "or.h"
#include "dr.h"
#include "tb_bpt84.h"
#include <isode/cmd_srch.h>
#include <isode/usr.dirent.h>
#include "sys.file.h"
#include <sys/stat.h>

extern CHAN		*mychan;
extern CMD_TABLE        bptbl_body_parts84 [/* body part types */];
extern char             *this_msg,
			*loc_or,
			*loc_dom,
			*loc_dom_mta,
			*loc_dom_site,
			*postmaster,
			*hdr_822_bp,
			*ia5_bp,
			*recip_err,
			*cont_822, error[];
extern Q_struct         *PPQuePtr;
extern int              order_pref, linked;
extern int		msgid2rfc(), reprecip2rfc(),
			globalid2rfc(), x400eits2rfc(), include_admin;
extern void		rfctxtfold();
extern UTC		utclocalise();
static int              wr_hdr_rest ();
static int              get_DR ();
static int		end_admin_info(), start_admin_info();
static int              wr_hdr_end ();
static int              wr_body ();
static int              copy ();
static int              DR_print ();
static int              DR_write ();
static DRmpdu           DRstruct;
static char		*getSubject();
static int		write_start(), write_cont(), write_end();
static ADDR             *to;
static int		linecopy();
static int		do_subject_intermediate_trace();
static int		get822Mailbox();
static char             message[BUFSIZ*2];  /* -- holds the Recip info -- */
static long		return_size;
extern CMD_TABLE        rr_rcode[],
			rr_dcode[],
			rr_tcode[];
int			forwarded;

static int		allSameType(), allSameMta();

static CMD_TABLE   rr_dcode_eng [] = {  /* -- diagnostic-codes -- */

"Unknown Address",
DRD_UNRECOGNISED_OR,

"Ambiguous Address",			
DRD_AMBIGUOUS_OR,

"Message rejected due to congestion",
DRD_MTA_CONGESTION,

"Message looping detected (please contact local administrator)", 
DRD_LOOP_DETECTED,

"Recipient's Mailbox unavailable",
DRD_UA_UNAVAILABLE,

"Message timed out",
DRD_MAX_TIME_EXPIRED,

"Some of your message could not be displayed by the recipient's system",
DRD_ENCINFOTYPES_NOTSUPPORTED,

"The message is too big for delivery",
DRD_CONTENT_TOO_LONG,

"Unable to perform the conversion required for delivery", 
DRD_CONVERSION_IMPRACTICAL,

"Not allowed to perform the conversion required for delivery",
DRD_CONVERSION_PROHIBITED,

"Don't know how to convert for delivery",
DRD_IMPLICITCONV_NOTREGISTERED,
	
"One or more of the message parameters is invalid",
DRD_INVALID_PARAMETERS,

"A syntax error was detected in the header of the message",
DRD_CONTENT_SYNTAX_ERROR,
	
"X.400 protocol error - constraint violation (please contact local administrator)",
DRD_SIZE_CONSTRAINT_VIOLATION,

"X.400 protocol error - missing argument (please contact local administrator)",
DRD_PROTOCOL_VIOLATION,

"The recipient site could not handle the content type of the message",
DRD_CONTENT_TYPE_NOT_SUPPORTED,

"The message specified too many recipients for the system to cope with",
DRD_TOO_MANY_RECIPIENTS,

"Incompatibility between two sites on the route of the message (please contact local administrator)",
DRD_NO_BILATERAL_AGREEMENT,
	
"A critical function required for the transfer or delivery of the message is not supported by the originating site of this report",
DRD_UNSUPPORTED_CRITICAL_FUNCTION,
	
"Not allowed to perform the conversion required for delivery as it would result in loss of information",
DRD_CONVERSION_WITH_LOSS_PROHIBITED,
	
"Some lines of the message were too long to be delivered",
DRD_LINE_TOO_LONG,
	
"Some pages of the message were too long to be delivered",
DRD_PAGE_SPLIT,

"Some pictorial symbols would have been lost if the message was delivered",
DRD_PICTORIAL_SYMBOL_LOSS,

"Some punctuation symbols would have been lost if the message was delivered",
DRD_PUNCTUATION_SYMBOL_LOSS,

"Some alphabetic characters would have been lost if the message was delivered",
DRD_ALPHABETIC_CHARACTER_LOSS,

"Some information would have been lost if the message was delivered",
DRD_MULTIPLE_INFORMATION_LOSS,

"The message could not be delivered because the originator of the message prohibited redirection",
DRD_RECIPIENT_REASSIGNMENT_PROHIBITED,
	
"The message could not be redirected to an alternate recipient because that recipient had previously redirected the message (redirection loop)",
DRD_REDIRECTION_LOOP_DETECTED,
	
"The message could not be delivered because the originator of the message prohibited the expansion of distribution lists",
DRD_DL_EXPANSION_PROHIBITED,
	
"The originator of the message does not have permission to submit messages to the recipient distribution list",
DRD_NO_DL_SUBMIT_PERMISSION,
	
"The expansion of a distribution list could not be completed",
DRD_DL_EXPANSION_FAILURE,
	
"The postal access unit does not support the physical format required",
DRD_PHYSICAL_RENDITION_ATTRIBUTES_NOT_SUPPORTED,
	
"The message was undeliverable because the specified recipient postal address was incorrect",
DRD_UNDLIV_PD_ADDRESS_INCORRECT,
	
"The message was undeliverable because the physical delivery office identified by the specified postal address was incorrect or invalid",
DRD_UNDLIV_PD_OFFICE_INCORRECT_OR_INVALID,

"The message was undeliverable because the specified recipient postal address was incompletely specified",
DRD_UNDLIV_PD_ADDRESS_INCOMPLETE,

"The message was undeliverable because the recipient specified in the postal address was not known at that address",
DRD_UNDLIV_RECIPIENT_UNKNOWN,

"The message was undeliverable because the recipient specified in the postal address is deceased",
DRD_UNDLIV_RECIPIENT_DECEASED,

"The message was undeliverable because the recipient organization specified in the postal address is no longer registered",
DRD_UNDLIV_ORGANIZATION_EXPIRED,

"The message was undeliverable because the recipient specified in the postal address refused to accept it",
DRD_UNDLIV_RECIPIENT_REFUSED_TO_ACCEPT,

"The message was undeliverable because the recipient specified in the postal address did not collect the mail",
DRD_UNDLIV_RECIPIENT_DID_NOT_CLAIM,

"The message was undeliverable because the recipient specified in the postal address had changed address permanently ('moved') and forwarding was not applicable",
DRD_UNDLIV_RECIPIENT_CHANGED_ADDRESS_PERMANENTLY,

"The message was undeliverable because the recipient specified in the postal address had changed address temporarily ('on holiday') and forwarding was not applicable",
DRD_UNDLIV_RECIPIENT_CHANGED_ADDRESS_TEMPORARILY,

"The message was undeliverable because the recipient specified in the postal address had changed temporary address ('departed') and forwarding was not applicable",
DRD_UNDLIV_RECIPIENT_CHANGED_TEMPORARY_ADDRESS,

"The message was undeliverable because the recipient has moved and the recipient's new address is unknown",
DRD_UNDLIV_NEW_ADDRESS_UNKNOWN,

"The message was undeliverable because delivery would have required physical forwarding which the recipient did not want",
DRD_UNDLIV_RECIPIENT_DID_NOT_WANT_FORWARDING,

"The physical forwarding required for the message to be delivered has been prohibited by the originator of the message",
DRD_UNDLIV_ORIGINATOR_PROHIBITED_FORWARDING,

"The message could not be progressed because it would violate the security policy in force",
DRD_SECURE_MESSAGING_ERROR,

"The downgrading of the message's content required for delivery could not be performed",
DRD_UNABLE_TO_DOWNGRADE,
	0,					-1
};

int     dirlen;


/* ---------------------  Begin  Routines  -------------------------------- */




write_report (ap, DRno, recip)
ADDR    *ap;
int     DRno;
ADDR	*recip;
{
	int     retval = RP_BAD;

	PP_TRACE (("write_report: %s (%d) (%s)",
		ap->ad_value, DRno, this_msg));

	if (ap == NULLADDR)
		return retval;
	else
		to = ap;

	/* -- initialise -- */
	bzero (message, sizeof (message));
	dr_init (&DRstruct);

	if (rp_isbad (get_DR (DRno)))
		goto write_report_end;

	if (rp_isbad (wr_hdr_start()))
		goto write_report_end;

	if (rp_isbad (wr_hdr_rest()))
		goto write_report_end;

	if (rp_isbad (wr_hdr_end()))
		goto write_report_end;

	if (rp_isbad (wr_body(ap, recip)))
		goto write_report_end;

	retval = RP_OK;

write_report_end: ;
	dr_free (&DRstruct);
	if (recip_err) {
		free(recip_err);
		recip_err = NULLCP;
	}
	return retval;

}




/*  --------------------  Static  Routines  ------------------------------- */


static int write_p1_trace (trace)
Trace	*trace;
{
	char	abuf[BUFSIZ];

	if (trace == NULL)
		return RP_OK;
	if (rp_isbad(write_p1_trace (trace -> trace_next)))
		return RP_BAD;
	if (x400trace2rfc (trace, abuf) == NOTOK)
		return RP_BAD;
	if (rp_isbad(DR_write("X400-Received: ",abuf)))
		return RP_BAD;
	return RP_OK;
}

wr_postmaster_hdr(useDR)
int	useDR;
{
	char	buf[BUFSIZ],
		temp[BUFSIZ],
		part[BUFSIZ];
	RP_Buf          rps,
			*rp = &rps;
	UTC             now, localnow;

	PP_TRACE(("dr2rfc/wr_postmaster_hdr()"));

	if (rp_isbad (io_tpart (hdr_822_bp, FALSE, rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("dr2rfc/wr_hdr/io_tpart blows %s", rp -> rp_line));
		stop_io();
		(void) sprintf (error,
				"io_tpart error [%s]",
				rp -> rp_line);
		return RP_BAD;
	}

	if (rp_isbad (DR_write ("From: ", postmaster)))
		return RP_BAD;


	/* -- To -- */
	if (useDR == TRUE && isstr(to->ad_r822DR)) {
		char	*tmp;
		tmp = to->ad_r822adr;
		to->ad_r822adr = to->ad_r822DR;
		if (adr2rfc (to, temp, order_pref) == RP_BAD)
			return RP_BAD;
		if (rp_isbad (DR_write ("To: ", temp)))
			return RP_BAD;
		to->ad_r822adr = tmp;
	} else {
		if (rp_isbad (DR_write ("To: ", postmaster)))
			return RP_BAD;
	}

	/* -- Subject -- */
	if (adr2rfc (to, temp, order_pref) == RP_BAD)
		return RP_BAD;

	(void) sprintf (buf, 
			"Unable to send delivery report to %s",
			temp);

	if (rp_isbad (DR_write ("Subject: ", buf)))
		return RP_BAD;

	if (rp_isbad(DR_write("Message-Type: ", "Delivery Report")))
		return RP_BAD;

	now = utcnow();
	localnow = utclocalise(now);
	if (UTC2rfc(localnow, buf) == NOTOK)
		return RP_BAD;
	if (now) free((char *) now);
	if (localnow) free((char *) localnow);

	if (rp_isbad (DR_write ("Date: ", buf)))
		return RP_BAD;

	if (rp_isbad (io_tdend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("dr2rfc/wr_hdr_end/tdend blows %s", rp -> rp_line));
		(void) sprintf (error,
				"io_tdend error [%s]",
				rp -> rp_line);
		stop_io();
		return RP_BAD;
	}
	
	(void) sprintf(part, "1.%s", ia5_bp);

	if (rp_isbad (io_tpart (part, FALSE, rp))) {
		PP_TRACE (("dr2rfc/wr_body/tpart blows %s", rp -> rp_line));
		(void) sprintf (error,
				"io_tpart error [%s]",
				rp -> rp_line);
		stop_io();
		return RP_BAD;
	}
	
	if (useDR == TRUE)
		(void) sprintf (buf, "The originally sender '%s' can not be reached from %s\nThe delivery report follows:\n\n",
				to->ad_value, loc_dom_site);
	else
		(void) sprintf(buf, "%s was unable to construct a delivery report\n\taddressed to '%s'.\nThe reason given was '%s'.\n\nThe delivery report follows:\n\n",
		       	mychan->ch_name,
		       	temp,
			recip_err);
	if (rp_isbad(io_tdata (buf, strlen(buf)))) {
		PP_LOG  (LLOG_EXCEPTIONS, ("Error writing message"));
		(void) sprintf(error,
			       "io_tdata error [%s]",
			       rp -> rp_line);
		stop_io();
		return RP_BAD;
	}

	if (rp_isbad (io_tdend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/wr_body/io_tdend fails"));
		(void) sprintf (error,
				"io_tdend error");
		stop_io();
		return RP_BAD;
	}

	return RP_OK;
}	
	
wr_hdr_start ()
{
	RP_Buf          rps, *rp = &rps;
	char            buffer[BUFSIZ], part[BUFSIZ],
	*pmailbox;
	UTC             now, localnow;
	Rrinfo		*ix;
	if (rp_isbad (io_tinit (rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("dr2rfc/wr_hdr/tinit blows %s", rp -> rp_line));
		(void) sprintf (error,
				"io_tinit error [%s]",
				rp -> rp_line);
		stop_io();
		return RP_BAD;
	}

	if (recip_err != NULLCP) {
		if (rp_isbad(wr_postmaster_hdr(FALSE)))
			return RP_BAD;
		(void) sprintf(part, "2.ipm/%s", hdr_822_bp);
		forwarded = TRUE;
	} else if  (isstr(to->ad_r822DR)) {
		if (rp_isbad(wr_postmaster_hdr(TRUE)))
			return RP_BAD;
		(void) sprintf(part, "2.ipm/%s", hdr_822_bp);
		forwarded = TRUE;
	} else {
		(void) sprintf(part, "%s", hdr_822_bp);
		forwarded = FALSE;
	}
	
	if (rp_isbad (io_tpart (part, FALSE, rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("dr2rfc/wr_hdr/io_tpart blows %s", rp -> rp_line));
		(void) sprintf (error,
				"io_tpart error [%s]",
				rp -> rp_line);
		stop_io();
		return RP_BAD;
	}

	if (DRstruct.dr_trace != NULL
	    && DRstruct.dr_trace -> trace_next != NULL)
		/* not local (x400 ?) */
		if (rp_isbad(write_p1_trace(DRstruct.dr_trace)))
			return RP_BAD;

	/* -- From -- */
	if (rp_isbad (DR_write ("From: ", postmaster)))
		return RP_BAD;


	/* -- To -- */
	if (adr2rfc (to, buffer, order_pref) == RP_BAD)
		return RP_BAD;
	if (rp_isbad (DR_write ("To: ", buffer)))
		return RP_BAD;

	/* -- Subject -- */
	(void) sprintf (buffer, "Delivery Report");
	
	ix = DRstruct.dr_recip_list;
	if (allSameType(ix, DR_REP_SUCCESS) == TRUE) 
		(void) sprintf (buffer, "%s (success)", buffer);
	else if (allSameType(ix, DR_REP_FAILURE) == TRUE)
		(void) sprintf (buffer, "%s (failure)", buffer);
	else
		(void) sprintf (buffer, "%s (success and failures)", buffer);

	if (ix && ix->rr_next == NULL
	    && !rp_isbad(get822Mailbox(ix, &pmailbox))) {
		(void) sprintf (buffer, "%s for %s", 
				buffer, pmailbox);
		free(pmailbox);
	} else if (allSameMta(ix, &pmailbox) == TRUE) {
		(void) sprintf (buffer, "%s for MTA %s",
				buffer, pmailbox);
		free(pmailbox);
	}


	if (rp_isbad (DR_write ("Subject: ", buffer)))
		return RP_BAD;

	if (rp_isbad(DR_write("Message-Type: ", "Delivery Report")))
		return RP_BAD;

	now = utcnow();
	localnow = utclocalise(now);
	if (UTC2rfc(localnow, buffer) == NOTOK)
		return RP_BAD;
	if (now) free((char *) now);
	if (localnow) free((char *) localnow);

	if (rp_isbad (DR_write ("Date: ", buffer)))
		return RP_BAD;

	return RP_OK;
}




static int wr_hdr_rest ()
{
	char            buffer [BUFSIZ],
			fbuf [BUFSIZ];

	PP_TRACE (("dr2rfc/wr_hdr_rest ()"));

	/* -- P1-Message-ID -- */
	if (DRstruct.dr_mpduid && DRstruct.dr_mpduid->mpduid_string) {
		(void) sprintf (fbuf, "Message-ID: ");
		(void) sprintf (buffer, "<\"%s\"@%s>",
				DRstruct.dr_mpduid->mpduid_string,
				loc_dom_site);
		if (rp_isbad (DR_write (fbuf, buffer)))
			return RP_BAD;
	}

	/* -- UAContentId (optional) -- */
	if (PPQuePtr -> ua_id) {
		(void) sprintf (fbuf, "Content-Identifier: ");
		if (rp_isbad (DR_write (fbuf, PPQuePtr -> ua_id)))
			return RP_BAD;
	}

	return RP_OK;
}

static int get_DR (DRno)
int     DRno;
{
	int     retval;

	PP_TRACE (("get_DR for %s (%d)", this_msg, DRno));

	retval = get_dr (DRno, this_msg, &DRstruct);
	if (rp_isbad(retval) && retval != RP_EOF)
		goto get_DR_free;

	return (RP_OK);

get_DR_free: ;
	return RP_BAD;
}





static wr_hdr_end()
{
	RP_Buf          rps,
			*rp = &rps;

	PP_TRACE (("dr2rfc/wr_hdr_end()"));
	if (rp_isbad (io_tdend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("dr2rfc/wr_hdr_end/tdend blows %s", rp -> rp_line));
		(void) sprintf (error,
				"io_tdend error [%s]",
				rp -> rp_line);
		stop_io();
		return RP_BAD;
	}
	return RP_OK;
}

extern long	max_include_msg;
extern int	num_include_lines;
extern int	include_all;

static wr_body (ap, recip)
ADDR    *ap;
ADDR	*recip;
{
	RP_Buf  rps,
		*rp = &rps;
	char    *dir = NULLCP;
	char    filename[MAXPATHLENGTH],
		*pmailbox, *cix, *str,
		buffer[BUFSIZ],
		buffer2[BUFSIZ],
		part[BUFSIZ],
		temp[BUFSIZ];
	int     retval, avail, correlator,
		nosubject = FALSE, allSuccess;
	Rrinfo  *ix;
	ADDR    *ad;
	UTC     now;
	UTC	loctime;

	if (forwarded == TRUE)
		(void) sprintf(part, "2.ipm/1.%s", ia5_bp);
	else
		(void) sprintf(part, "1.%s", ia5_bp);

	if (rp_isbad (io_tpart (part, FALSE, rp))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("dr2rfc/wr_body/tpart blows %s", rp -> rp_line));
		(void) sprintf (error,
				"io_tpart error [%s]",
				rp -> rp_line);
		stop_io();
		return RP_BAD;
	}

/* dr-summary */
	if (PPQuePtr -> pp_content_correlator == NULL
	    && PPQuePtr -> ua_id == NULL)
		correlator = FALSE;
	else
		correlator = TRUE;

	if (correlator == TRUE) {
		if (rp_isbad(write_start("This report relates to your message:", "")))
			return RP_BAD;
		/* content correlator */

		if (rp_isbad(pretty_output((PPQuePtr -> pp_content_correlator) ? PPQuePtr -> pp_content_correlator : PPQuePtr -> ua_id, 2)))
			return RP_BAD;

	} else {
		char	*subject;

		if ((subject = getSubject(this_msg, recip)) != NULLCP) {
			if (rp_isbad(write_start("This report relates to your message with subject:", subject)))
				return RP_BAD;
			free(subject);
		} else if (rp_isbad(write_start("This report relates to your message", ""))) {
			nosubject = TRUE;
			return RP_BAD;
		}

	}
	if (DRstruct.dr_subject_intermediate_trace != NULL) {
		Trace	*tix = DRstruct.dr_subject_intermediate_trace;
		while (tix->trace_next != NULL)
			tix = tix->trace_next;
		loctime = utclocalise(tix->trace_DomSinfo.dsi_time);
		if (UTC2rfc(loctime, temp) == NOTOK)
			return RP_BAD;
		if (loctime) free((char *) loctime);
		(void) sprintf(buffer, "of %s\n",temp);
		if (rp_isbad(write_cont(buffer, FALSE)))
			return RP_BAD;
	}
	if (nosubject == TRUE 
	    && rp_isbad(write_cont("(no subject)", FALSE)))
		return RP_BAD;

	if (rp_isbad(write_end()))
		return RP_BAD;

	/* dr-recipients */
	ix = DRstruct.dr_recip_list;
	allSuccess = TRUE;

	while (ix != NULL) {
		if (rp_isbad(get822Mailbox(ix, &pmailbox)))
			return RP_BAD;
		if (ix -> rr_report.rep_type == DR_REP_SUCCESS) {
			if (rp_isbad(write_start("Your message was successfully delivered to", "")))
				return RP_BAD;	
			if (rp_isbad(write_cont(pmailbox, FALSE)))
				return RP_BAD;
			if (ix-> rr_supplementary) {
				(void) sprintf (buffer, "%s", ix -> rr_supplementary);
				if (rp_isbad(write_cont(ix -> rr_supplementary, TRUE)))
					return RP_BAD;
			}

			if (ix -> rr_arrival) {
				loctime = utclocalise(ix->rr_arrival);
				if (UTC2rfc (loctime, temp) == NOTOK)
					return RP_BAD;
				if (loctime) free ((char *) loctime);
				(void) sprintf(buffer, "at %s", temp);
				if (rp_isbad(write_cont(temp, FALSE)))
					return RP_BAD;
			}
			if (rp_isbad(write_cont("\n", FALSE)))
			    return RP_BAD;
			if (rp_isbad(write_end()))
				return RP_BAD;
		} else {
			char	*diag;
			allSuccess = FALSE;
			if (rp_isbad(write_start("Your message was not delivered to ", "")))
				return RP_BAD;
			if (rp_isbad(write_cont(pmailbox, FALSE)))
				return RP_BAD;
			if (rp_isbad(write_cont("for the following reason:", FALSE)))
				return RP_BAD;

			if ((diag = rcmd_srch(ix -> rr_report.rep.rep_ndinfo.nd_dcode, rr_dcode_eng)) != NULL)
				(void) sprintf(buffer, "%s", diag);
			else
				(void) sprintf(buffer, "%s", 
					       rcmd_srch(ix -> rr_report.rep.rep_ndinfo.nd_rcode, rr_rcode));
				
			if (rp_isbad(write_cont(buffer, TRUE)))
				return RP_BAD;

			if (ix-> rr_supplementary) {
				(void) sprintf (buffer, "%s", ix -> rr_supplementary);
				if (rp_isbad(write_cont(ix -> rr_supplementary, TRUE)))
					return RP_BAD;
			}
			if (rp_isbad(write_cont("\n", FALSE))
			    || rp_isbad(write_end()))
				return RP_BAD;
		}
		if (pmailbox != NULLCP) {
			free(pmailbox); 
			pmailbox = NULLCP;
			}
		ix = ix -> rr_next;
	}

/* dr-adminstrator-info-envelope */
	if (allSuccess == FALSE
	    || include_admin == TRUE) {
		start_admin_info();
		if (DRstruct.dr_trace != NULL) {
			Trace	*tix = DRstruct.dr_trace;

			if (tix -> trace_mta != NULL) {
				(void) sprintf(buffer, "by: mta %s",
					       tix -> trace_mta);
				if (rp_isbad(write_start("DR generated",buffer)))
					return RP_BAD;
				if (globalid2rfc(&(tix -> trace_DomId),
						 temp) == NOTOK)
					return RP_BAD;
				(void) sprintf(buffer, "in %s", temp);
				if (rp_isbad(write_cont(buffer, TRUE)))
					return RP_BAD;
			} else {
				if (globalid2rfc(&(tix -> trace_DomId),
						 temp) == NOTOK)
					return RP_BAD;
				(void) sprintf(buffer, "in %s", temp);
				if (rp_isbad(write_start("DR generated",buffer)))
					return RP_BAD;
			}

			loctime = utclocalise (tix -> trace_DomSinfo.dsi_time);
			if (UTC2rfc(loctime, temp) == NOTOK)
				return RP_BAD;
			if (loctime) free((char *) loctime);

			(void) sprintf(buffer,"at %s\n*",temp);
			if (rp_isbad(write_cont(buffer, FALSE))
			    || rp_isbad(write_end()))
				return RP_BAD;
		}

		now = utcnow();	
		loctime = utclocalise(now);
		if (UTC2rfc(loctime, temp) == NOTOK)
			return RP_BAD;
		if (now) free((char *) now);
		if (loctime) free((char *) loctime);

		sprintf(buffer, "foo@%s", loc_dom_mta);
		ap_s2s (buffer, &str, order_pref);
		if (str != NULLCP
		    && (cix = index(str, '@')) != NULLCP)
			(void) sprintf(buffer, "at %s", ++cix);
		else
			(void) sprintf(buffer, "at %s",loc_dom_mta);
		if (str) free(str);

		if (rp_isbad(write_start("Converted to RFC 822",
					 buffer)))
			return RP_BAD;
		(void) sprintf(buffer, "at %s\n*", temp);
		if (rp_isbad(write_cont(buffer, FALSE))
		    || rp_isbad(write_end()))
			return RP_BAD;

		/* dr-extra-information */
		(void) sprintf(buffer, "* Delivery Report Contents:\n*\n");
		if (rp_isbad(io_tdata (buffer, strlen(buffer)))) {
			PP_LOG  (LLOG_EXCEPTIONS, ("Error writing message"));
			(void) sprintf (error,
					"io_tdata error");
			       
			stop_io();
			return RP_BAD;
		}

		/* drc-field-list */
		if (globalid2rfc(&(PPQuePtr->msgid.mpduid_DomId), buffer2) == NOTOK)
			return RP_BAD;
		if (PPQuePtr->msgid.mpduid_string != NULL)
			(void) sprintf(buffer, "[%s;%s]",
				       buffer2,
				       PPQuePtr->msgid.mpduid_string);
		else
			(void) sprintf(buffer, "[%s]", buffer2);
		if (rp_isbad (DR_write("Subject-Submission-Identifier: ",
				       buffer)))
			return RP_BAD;

		if (PPQuePtr -> ua_id != NULL
		    && rp_isbad (DR_print("Content-Identifier: ",
					  PPQuePtr -> ua_id)))
			return RP_BAD;

		if (PPQuePtr -> cont_type != NULL
		    && strcmp(PPQuePtr -> cont_type, cont_822) != 0
		    && rp_isbad (DR_print("Content-Type: ",
					  PPQuePtr -> cont_type)))
			return RP_BAD;

		if (PPQuePtr -> orig_encodedinfo.eit_types != 0) {
			if (x400eits2rfc (&(PPQuePtr -> orig_encodedinfo), &buffer[0]) == NOTOK)
				return RP_BAD;
			if (buffer[0] != '\0')
				if (rp_isbad
				    (DR_print("Original-Encoded-Information-Types: ",
					      buffer)))
					return RP_BAD;
		}

		if (DRstruct.dr_subject_intermediate_trace != NULL) {
			if (rp_isbad(do_subject_intermediate_trace(DRstruct.dr_subject_intermediate_trace)))
				return RP_BAD;
		}

		if (DRstruct.dr_dl_history != NULL) {
			DLHistory       *dlix = DRstruct.dr_dl_history;
			buffer[0] = '\0';

			if (rp_isbad(write_start("Originator-and-DL-Expansion-History: ", "")))
				return RP_BAD;
		
			while (dlix != NULL) {
				if (dlix->dlh_next != NULL) {
					(void) sprintf(buffer, "%s,", dlix -> dlh_addr);
					if (rp_isbad(write_cont(buffer, FALSE)))
						return RP_BAD;
				} else if (rp_isbad(write_cont(dlix -> dlh_addr, FALSE)))
					return RP_BAD;
				dlix = dlix -> dlh_next;
			}
			if (rp_isbad(write_end()))
				return RP_BAD;
		}

		if (DRstruct.dr_reporting_dl_name != NULL
		    && DRstruct.dr_reporting_dl_name -> fn_addr != NULL
		    && rp_isbad(DR_print("Reporting-DL-Name: ",
					 DRstruct.dr_reporting_dl_name -> fn_addr)))
			return RP_BAD;

		if (PPQuePtr -> pp_content_correlator != NULL
		    && (rp_isbad(write_start("Content-Correlator:", ""))
			|| rp_isbad(pretty_output(PPQuePtr -> pp_content_correlator,
						  strlen("Content-Correlator:")))))
			return RP_BAD;

		ix = DRstruct.dr_recip_list;
		while (ix != NULL) {
			char    *str = NULLCP;
			for (ad = PPQuePtr -> Raddress; ad; ad = ad->ad_next)
				if (ad -> ad_no == ix -> rr_recip)
					break;
			if (ad == NULL) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("unable to find recip '%d'", ix -> rr_recip));
				return RP_BAD;
			}
			ap_s2s(ad -> ad_r822adr, &str, order_pref);
			if (str != NULLCP) {
				(void) sprintf(buffer, "%s,", str);
				free(str);
			} else
				(void) sprintf(buffer, "%s,", ad -> ad_r822adr);
			if (rp_isbad(write_start("Recipient-Info:", buffer)))
				return RP_BAD;

			(void) sprintf(buffer, "%s;", ad -> ad_r400adr);
			if (rp_isbad(write_cont(buffer, FALSE)))
				return RP_BAD;
		
			if (ix -> rr_report.rep_type == DR_REP_SUCCESS) {
				char    delivered[BUFSIZ],
				*type;
				loctime = utclocalise (ix -> rr_report.rep.rep_dinfo.del_time);
				if (UTC2rfc(loctime, delivered) == NOTOK)
					return RP_BAD;
				if (loctime) free ((char *) loctime);
				(void) sprintf(buffer, "SUCCESS delivered at %s;",
					       delivered);
				if (rp_isbad(write_cont(buffer, FALSE)))
					return RP_BAD;
			
				if (ix -> rr_report.rep.rep_dinfo.del_type != DRT_PUBLIC
				    && (type = rcmd_srch(ix -> rr_report.rep.rep_dinfo.del_type, rr_tcode)) != NULL) {
					(void) sprintf(buffer, "type of MTS user %s (%d);",
						       type,
						       ix -> rr_report.rep.rep_dinfo.del_type);
					if (rp_isbad(write_cont(buffer, FALSE)))
						return RP_BAD;
				}
			} else {
				char    *diag;

				(void) sprintf(buffer, "FAILURE reason %s (%d);",
					       rcmd_srch(ix -> rr_report.rep.rep_ndinfo.nd_rcode, rr_rcode),
					       ix -> rr_report.rep.rep_ndinfo.nd_rcode);
				if (rp_isbad(write_cont(buffer, FALSE)))
					return RP_BAD;
				if ((diag = rcmd_srch(ix -> rr_report.rep.rep_ndinfo.nd_dcode, rr_dcode)) != NULL) {
					(void) sprintf(buffer,"diagnostic %s (%d);",
						       diag,
						       ix -> rr_report.rep.rep_ndinfo.nd_dcode);
					if (rp_isbad(write_cont(buffer, FALSE)))
						return RP_BAD;
				}
			}

			if (DRstruct.dr_subject_intermediate_trace != NULL) {
				(void) sprintf(buffer,"last trace");
				if (PPQuePtr -> encodedinfo.eit_types != 0) {
					if(x400eits2rfc(&PPQuePtr -> encodedinfo, 
							&(temp[0])) == NOTOK)
						return RP_BAD;
					(void) sprintf(buffer,"%s (%s)", buffer, temp);
				}
				loctime = utclocalise (DRstruct.dr_subject_intermediate_trace->trace_DomSinfo.dsi_time);
				if (UTC2rfc(loctime, &temp[0]) == NOTOK)
					return RP_BAD;
				if (loctime) free ((char *) loctime);

				(void) sprintf(buffer,"%s %s;",buffer, temp);
				if (rp_isbad(write_cont(buffer, FALSE)))
					return RP_BAD;
			}

			if (ix -> rr_converted != NULL) {
				if (x400eits2rfc (ix -> rr_converted, &temp[0]) == NOTOK)
					return RP_BAD;
				if (temp[0] != '\0') {
					(void) sprintf(buffer, "converted eits %s;",
						       temp);
					if (rp_isbad(write_cont(buffer, TRUE)))
						return RP_BAD;
				}
			}

			if (ix -> rr_originally_intended_recip != NULL) {
				if (ix -> rr_originally_intended_recip -> fn_dn)
					(void) sprintf(buffer, "originally intended recipient %s, %s;",
						       ix -> rr_originally_intended_recip -> fn_dn,
						       ix -> rr_originally_intended_recip -> fn_addr);
				else
					(void) sprintf(buffer, "originally intended recipient %s;",
						       ix -> rr_originally_intended_recip -> fn_addr);

				if (rp_isbad(write_cont(buffer, TRUE)))
					return RP_BAD;
			}
		
			if (ix -> rr_supplementary != NULL) {
				(void) sprintf(buffer, "supplementary info \"%s\";",
					       ix -> rr_supplementary);
				if (rp_isbad(write_cont(buffer, TRUE)))
					return RP_BAD;
			}
		
			if (ix -> rr_redirect_history != NULL) {
				Redirection     *rix = ix -> rr_redirect_history;
				temp[0] = '\0';
				if (rp_isbad(write_cont("redirection history", FALSE)))
					return RP_BAD;
				while (rix != NULL) {
					if (rix -> rd_next != NULL) 
						(void) sprintf(buffer, "%s,", rix -> rd_addr);
					else
						(void) sprintf(buffer, "%s;", rix -> rd_addr);
					if (rp_isbad(write_cont(buffer, TRUE)))
						return RP_BAD;
					rix = rix -> rd_next;
				}
			}

			if (ix -> rr_physical_fwd_addr != NULL) {
				(void) sprintf(buffer, "physical forwarding address %s;",
					       ix -> rr_physical_fwd_addr -> fn_addr);
				if (rp_isbad(write_cont(buffer, FALSE)))
					return RP_BAD;
			}
			if (rp_isbad(write_end()))
				return RP_BAD;
			ix = ix -> rr_next;
		}
		end_admin_info();
	}

	/* dr-content-return */

	avail = FALSE;
	if (allSuccess != TRUE &&
	    PPQuePtr -> content_return_request == TRUE) {
		if (qid2dir (this_msg, PPQuePtr->Oaddress, TRUE, &dir) == NOTOK) {
			PP_LOG(LLOG_EXCEPTIONS, ("qid2dir failed for originator"));
			dir = NULLCP;
		} else if (msg_rinit(dir) == NOTOK)
			PP_LOG(LLOG_EXCEPTIONS, ("msg_rinit failed"));
		else if ((retval = msg_rfile (filename)) == RP_OK) {
			/* check for correct content type i.e presence of hdr.822 */
			if ((cix = rindex(filename, '/')) == NULLCP)
				cix = filename;
			else
				cix++;
			
			if (strncmp(cix, 
				    hdr_822_bp, 
				    strlen(hdr_822_bp)) != 0)  {
				PP_LOG(LLOG_DEBUG, 
				       ("Unable to find a hdr.822 in directory '%s'", dir));
				(void) msg_rend();
			} else
				avail = TRUE;
		} else 
			(void) msg_rend();
	}

	if (TRUE == allSuccess) 
		(void) sprintf(buffer,
			       "\nThe Original Message is not returned with positive reports\n");
	else if (avail == TRUE) {
		dirlen = strlen(dir) + 1;
		if (include_all == TRUE
		    || PPQuePtr -> msgsize <= max_include_msg)
			(void) sprintf(buffer, "\nThe Original Message follows:\n\n");
		else
			(void) sprintf(buffer, "\nThe Original Message follows but for size reasons has been truncated:\n\n");
	} else 
		(void) sprintf(buffer, "\nThe Original Message is not available\n");

	if (avail == FALSE || allSuccess == TRUE) {
		if (rp_isbad(io_tdata (buffer, strlen(buffer)))) {
			PP_LOG  (LLOG_EXCEPTIONS, ("Error writing message"));
			(void) sprintf(error,
				       "io_tdata error");
			stop_io();
			return RP_BAD;
		}
	}

	if (rp_isbad (io_tdend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/wr_body/io_tdend fails"));
		(void) sprintf(error,
			       "io_tdend error");
		stop_io();
		return RP_BAD;
	}

	if (avail == FALSE || allSuccess == TRUE) {
		if (rp_isbad (io_tend (rp))) {
			PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/wr_body/io_tend fails"));
			(void) sprintf(error,
				       "io_tend error");
			stop_io();
			return RP_BAD;
		} else
			return RP_OK;
	}

	return_size = 0;

	/* copy header no matter what size */
	if (rp_isbad (retval = copy (filename)))  {
		PP_LOG (LLOG_EXCEPTIONS, ("failed to copy header '%s'", filename));
	       (void) sprintf (error,
			       "Failed to submit header '%s'",
			       filename);
		return RP_BAD;
	} 	

	if (allSuccess == FALSE) {
		if (include_all == TRUE
		    || PPQuePtr -> msgsize <= max_include_msg) 
			while ((retval = msg_rfile (filename)) == RP_OK) {
				if (rp_isbad (retval = copy (filename)))
					break;
			}
		else {
			int numLines = 0;
			while ((retval = msg_rfile (filename)) == RP_OK
			       && numLines < num_include_lines
			       && return_size < max_include_msg) {
				if (rp_isbad (retval = linecopy (filename, &numLines)))
					break;
			}
		}
	}
	if (dir) free(dir);

	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("message copy failed"));
		return RP_BAD;
	}

	if (rp_isbad (retval = msg_rend ())) {
		PP_LOG (LLOG_EXCEPTIONS, ("msg_rend () failed"));
		(void) sprintf (error,
				"msg_rend failed");
		return RP_BAD;
	}

	if (rp_isbad (io_tend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/wr_body/io_tend fails"));
		(void) sprintf (error,
				"io_tend failed");
		stop_io();
		return RP_BAD;
	}

	return RP_OK;
}

static int do_subject_intermediate_trace(trace)
Trace	*trace;
{
	char	*j, oldj, buffer[BUFSIZ];
	if (trace->trace_next != NULL
	    && rp_isbad(do_subject_intermediate_trace(trace->trace_next)))
		return RP_BAD;
		
	if (trace2rfc(trace, buffer) == NOTOK)
		return RP_BAD;

	j = buffer;
	while (*j != '\0' && *j != ';') j++;
	if (*j != '\0') j++;
	oldj = *j;
	*j = '\0';
	if (rp_isbad(write_start("Subject-Intermediate-Trace-Information: ", buffer)))
		return RP_BAD;
	if (oldj != '\0') {
		*j = oldj;
		if (rp_isbad(write_cont(j,FALSE)))
			return RP_BAD;
	}
	if (rp_isbad(write_end()))
		return RP_BAD;
	return RP_OK;
}

static int linecopy (file, pnumlines)
char    *file;
int	*pnumlines;
{
	FILE    *fp;
	RP_Buf  rps,
		*rp = &rps;
	int     retval = RP_OK;
	char    buf[BUFSIZ], *ix, *top;

	ix = strdup(file + dirlen);

	/* strip of following -us or -uk from hdr_822_bp's */
	if ((top = rindex(ix, '/')) == NULLCP)
		top = ix;
	else
		top++;

	if (lexnequ(top, hdr_822_bp, strlen(hdr_822_bp)) == 0) 
		*(top+strlen(hdr_822_bp)) = '\0';

	if (forwarded == TRUE)
		(void) sprintf(buf, "2.ipm/2.ipm/%s", ix);
	else
		(void) sprintf(buf, "2.ipm/%s", ix);

	free(ix);

	if (rp_isbad(io_tpart (buf, FALSE, rp))) {
		PP_TRACE (("dr2rfc/wr_body/tpart blows %s", rp -> rp_line));
		stop_io();
		return RP_BAD;
	}

	if ((fp = fopen (file, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, file,
		      ("Can't open file"));
		(void) sprintf (error,
				"Can't open file '%s'",
				file);
		stop_io();
		return RP_FOPN;
	}

	while (*pnumlines < num_include_lines
	       && return_size < max_include_msg
	       && fgets (buf, sizeof buf, fp) != NULL) {
		if (io_tdata (buf, strlen(buf)) != RP_OK) {
			PP_LOG (LLOG_EXCEPTIONS, ("io_tdata error"));
			(void) sprintf (error,
					"io_tdata error");
			stop_io();
			retval = RP_FIO;
			break;
		}
		(*pnumlines)++;
		return_size += strlen(buf);
	}
	
	if (*pnumlines >= num_include_lines
	    || return_size >= max_include_msg) {
		if (io_tdata("......(message truncated)\n",
			     strlen("......(message truncated)\n")) != RP_OK) {
			PP_LOG (LLOG_EXCEPTIONS, ("io_tdata error"));
			(void) sprintf (error,
					"io_tdata error");
			stop_io();
			retval = RP_FIO;
		}
	}
		
	(void) fclose (fp);
	if (rp_isbad (io_tdend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/wr_body/io_tdend fails"));
		(void) sprintf (error,
				"io_tdend fails");
		stop_io();
		return RP_BAD;
	}
	return retval;
}

static int copy (file)
char    *file;
{
	FILE    *fp;
	RP_Buf  rps,
		*rp = &rps;
	int     retval = RP_OK;
	extern char	*hdr_822_bp;
	char    buf[BUFSIZ], *ix, *top;
	int     n;
	struct stat st;

	ix = strdup(file + dirlen);

	/* strip of following -us or -uk from hdr_822_bp's */
	if ((top = rindex(ix, '/')) == NULLCP)
		top = ix;
	else
		top++;

	if (lexnequ(top, hdr_822_bp, strlen(hdr_822_bp)) == 0) 
		*(top+strlen(hdr_822_bp)) = '\0';

	if (stat(file, &st) != 0) {
		PP_SLOG(LLOG_EXCEPTIONS, file,
			("Can't open file"));
		return RP_BAD;
	}
	
	if (linked == TRUE) {
		if (forwarded == TRUE)
			(void) sprintf(buf, "2.ipm/2.ipm/%s %s", ix, file);
		else
			(void) sprintf(buf, "2.ipm/%s %s", ix, file);
		free(ix);
		if (rp_isbad(io_tpart (buf, TRUE, rp))) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("dr2rfc/wr_body/tpart blows %s", rp -> rp_line));
			stop_io();
			return RP_BAD;
		}
	} else {
		if (forwarded == TRUE)
			(void) sprintf(buf, "2.ipm/2.ipm/%s", ix);
		else
			(void) sprintf(buf, "2.ipm/%s", ix);
		free(ix);
		if (rp_isbad(io_tpart (buf, FALSE, rp))) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("dr2rfc/wr_body/tpart blows %s", rp -> rp_line));
			stop_io();
			return RP_BAD;
		}

		if ((fp = fopen (file, "r")) == NULL) {
			PP_SLOG (LLOG_EXCEPTIONS, file,
				 ("Can't open file"));
			(void) sprintf (error,
					"Can't open file '%s'",
					file);
			stop_io();
			return RP_FOPN;
		}

		while ((n = fread (buf, sizeof (char), sizeof buf, fp)) > 0) {
			if (io_tdata (buf, n) != RP_OK) {
				PP_LOG (LLOG_EXCEPTIONS, ("io_tdata error"));
				(void) sprintf(error,
					       "io_tdata error");
				stop_io();
				retval = RP_FIO;
				break;
			}
		}
		(void) fclose (fp);
		if (rp_isbad (io_tdend (rp))) {
			PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/wr_body/io_tdend fails"));
			(void) sprintf(error,
				       "io_tdend fails");
			stop_io();
			return RP_BAD;
		}
	}

	return_size += (long) st.st_size;
	return retval;
}

#define	DEFAULT_FOLD_LENGTH	70

int	chnum, tab = 8, foldlength = DEFAULT_FOLD_LENGTH;
static	int	admin_info = FALSE;

static int start_admin_info ()
{
	write_start ("***** The following information is directed towards the local administrator\n***** and is not intended for the end user\n*", "");
	write_end ();
	admin_info = TRUE;
}

static int end_admin_info()
{
	admin_info = FALSE;
	write_start("****** End of administration information", "");
	write_end();
}

static int write_start(key, value)
char	*key,
	*value;
{
	int	retval;
	char	buffer[BUFSIZ];
	if (key == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS, ("Null key in output"));
		return RP_BAD;
	}

	(void) sprintf(buffer, "%s%s %s", 
		       (admin_info == TRUE) ? "* " : "",
		       key, value);
	if (rp_isbad(retval = io_tdata(buffer, strlen(buffer)))) {
		PP_LOG(LLOG_EXCEPTIONS, ("io_tdata error"));
		(void) sprintf(error,
			       "io_tdata error");
		stop_io();
	}
	chnum = strlen(buffer);

	return retval;
}

static int write_cont(value, fold)
char	*value;
int	fold;
{
	int	retval, len;
	char	buffer[BUFSIZ], *ix, *base, *lastix, *foldpt, savech;
	
	if (value == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS, ("Null value in output"));
		return RP_BAD;
	}
	
	len = strlen(value);
	
	bzero(buffer, sizeof buffer);
	
	if (fold == FALSE) {
		if (chnum + len > foldlength) {
			(void) sprintf(buffer, "\n%s%*s%s", 
				       (admin_info == TRUE) ? "* ": "",
				       tab, "", value);
			chnum = tab;
		} else
			(void) sprintf(buffer, " %s", value);
		chnum += len;
	} else {
		ix = lastix = base = value;

		/* write_cont assume big start new line */
		(void) sprintf (buffer, "\n%s%*s",
				(admin_info == TRUE) ? "* " : "",
				tab, "");
		chnum = tab;

		while (ix != NULLCP && *ix != '\0') {
			while (*ix != '\0' && !isspace(*ix)) 
				ix++;

			if (chnum + (ix - base) > foldlength) {
				/* need to fold */
				if (lastix == base)
					foldpt = ix;
				else
					foldpt = lastix;
				savech = *foldpt;
				*foldpt = '\0';
				(void) sprintf (buffer, "%s%s\n%s%*s",
						buffer, base,
						(admin_info == TRUE) ? "* " : "",
						tab, "");
				*foldpt = savech;
				base = foldpt;
				if (savech != '\0')
					/* skip last space */
					base++;
				chnum = tab;
			}
			lastix = ix;
			if (*ix != '\0') ix++;
		}
		/* flush rest */
		if (base != ix) {
			if (chnum + (ix - base) > foldlength) {
				(void) sprintf(buffer, "%s\n%s%*s%s",
					       buffer,
					       (admin_info == TRUE) ? "* " : "",
					       tab, "", base);
				chnum = tab;
			} else 
				(void) strcat (buffer, base);
			chnum += ix - base;
		}
	}

	if (rp_isbad(retval = io_tdata(buffer, strlen(buffer)))) {
		PP_LOG(LLOG_EXCEPTIONS, ("io_tdata error"));
		(void) sprintf(error,
			       "io_tdata error");
		stop_io();
	}
	return retval;
}

static int write_end()
{
	int retval;
	if (rp_isbad(retval = io_tdata("\n", strlen("\n")))) {
		PP_LOG(LLOG_EXCEPTIONS, ("io_tdata error"));
		(void) sprintf(error,
			       "io_tdata error");
		stop_io();
	}
	return retval;
}

static int DR_write (key, value)
char    *key;
char    *value;
{
	char    buffer[BUFSIZ];
	int     retval;

	if (value == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, ("No value for key %s", key));
		return RP_BAD;
	}

	bzero (buffer, sizeof buffer);
	if (admin_info == TRUE)
		(void) strcat (buffer, "* ");
	if (key)
		(void) strcat (buffer, key);
	(void) strcat (buffer, value);
	(void) strcat (buffer, "\n");
	if (rp_isbad (retval = io_tdata (buffer, strlen (buffer)))) {
		PP_LOG (LLOG_EXCEPTIONS, ("io_tdata error"));
		(void) sprintf (error,
				"io_tdata error");
		stop_io();
	}
	return retval;
}


static int DR_print (key, value)
char    *key;
char    *value;
{
	char    buffer [BUFSIZ];
	char    obuf[BUFSIZ];
	int     retval;

	if (value == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, ("No value for key %s", key));
		return RP_BAD;
	}

	bzero (buffer, sizeof buffer);
	if (admin_info == TRUE)
		(void) strcat (buffer, "* ");
	if (key)
		(void) strcat (buffer, key);
	(void) strcat (buffer, value);
	rfctxtfold (buffer, obuf, 66);
	if (rp_isbad (retval = io_tdata (obuf, strlen (obuf)))) {
		PP_LOG (LLOG_EXCEPTIONS, ("io_tdata error"));
		(void) sprintf (error,
				"io_tdata error");
		stop_io();
	}
	return retval;
}

/* output string padding newlines with some indentation */

pretty_output(str, padding)
char    *str;
int	padding;
{
	char    buffer[BUFSIZ];
	int     i = 0, ix;
	if (str == NULL)
		return RP_BAD;

	while (*str != '\0') {
		if (i == BUFSIZ-1) {
			/* flush buffer */
			buffer[i] = '\0';
			if (rp_isbad(io_tdata(buffer, strlen(buffer)))) {
				PP_LOG  (LLOG_EXCEPTIONS, ("Error writing message"));
				(void) sprintf (error,
						"io_tdata error");
				stop_io();
				return RP_BAD;
			}
			i = 0;
		}

		buffer[i++] = *str;

		if (*str++ == '\n') {
			if (i >= BUFSIZ - 3) {
				/* flush buffer */
				buffer[i] = '\0';
				if (rp_isbad(io_tdata(buffer, strlen(buffer)))) {
					PP_LOG  (LLOG_EXCEPTIONS, ("Error writing message"));
					(void) sprintf (error,
							"io_tdata error");
					stop_io();
					return RP_BAD;
				}
				i = 0;
			}

			if (admin_info == TRUE)
				buffer[i++] = '*';

			/* pad newlines */
			for (ix = 0; ix < padding; ix++)
				buffer[i++] = ' ';
		}

	}

	if (i > 0) {
		/* flush buffer */
		buffer[i] = '\0';
		if (rp_isbad(io_tdata(buffer, strlen(buffer)))) {
			PP_LOG  (LLOG_EXCEPTIONS, ("Error writing message"));
			stop_io();
			return RP_BAD;
		}
	}

	return RP_OK;
}

/*  */

static char *parseSubject(file)
char	*file;
{
	char	buf[BUFSIZ], *ret = NULLCP, *ix;
	int	subject = FALSE, len;
	FILE	*fd;
	if ((fd = fopen(file, "r")) == NULL)
		return NULLCP;

	while (subject == FALSE
	       && fgets(buf, BUFSIZ, fd) != NULLCP) {
		if ((ix = index(buf, ':')) != NULLCP) {
			*ix = '\0';
			if (lexequ("subject",buf) == 0) {
				subject = TRUE;
				ix++;
				while (*ix && isspace(*ix)) ix++;
				len = strlen(ix);
				*(ix + len - 1) = '\0';
				ret = strdup(ix);
			}
		}
	}
	(void) fclose(fd);
	return ret;
}

extern char	*hdr_822_bp; 

static int  isHdr(entry)
struct dirent *entry;
{
	if (strncmp(entry->d_name, hdr_822_bp, strlen(hdr_822_bp)) == 0)
		return 1;
	return 0;
}

static char *getSubject(msg_id, recip)
char	*msg_id;
ADDR	*recip;
{
	char	*dir = NULLCP, hdr[MAXPATHLENGTH], *subject = NULLCP;
	struct dirent **namelist = NULL;
	int	gothdr = FALSE,
		rcnt = recip->ad_rcnt;
	recip->ad_rcnt = 0;
	while (gothdr == FALSE && recip->ad_rcnt <= rcnt) { 
		if (qid2dir(msg_id, recip, TRUE, &dir) == OK) {
			if (_scandir(dir, &namelist, isHdr, NULLIFP) > 0)
				gothdr = TRUE;
		}
		recip->ad_rcnt++;
	}

	recip->ad_rcnt = rcnt;
	if (gothdr == FALSE)
		return NULLCP;
	if ((*namelist)->d_name[0] != '/')
		(void) sprintf(hdr, "%s/%s", dir, (*namelist)->d_name);
	else
		(void) strcpy(hdr, (*namelist)->d_name);

	subject = parseSubject(hdr);
	free((char *) namelist);
	free(dir);
	return subject;
}


static int    	get822Mailbox(ix, pmailbox)
Rrinfo	*ix;
char	**pmailbox;
{	
	ADDR	*ad;
	char	buf[BUFSIZ];
	*pmailbox = NULLCP;
	for (ad = PPQuePtr -> Raddress; ad; ad = ad->ad_next)
		if (ad -> ad_no == ix -> rr_recip)
			break;
	if (ad == NULL)
		(void) sprintf(buf,"recipient %d",
			       ix -> rr_recip);
	else if (ad -> ad_r822adr != NULL) {
		char    *str = NULLCP;
		ap_s2s (ad -> ad_r822adr, &str, order_pref);
		if (str != NULLCP && *str != '\0') {
			(void) sprintf(buf, "%s", str);
			free(str);
		} else
			(void) sprintf(buf, "%s", 
				       ad -> ad_r822adr);
	} else if (ad -> ad_value != NULL)
		(void) sprintf(buf, "%s", ad -> ad_value);
	else if (ad -> ad_type == AD_X400_TYPE
		   && ad -> ad_r400adr != NULL)
		(void) sprintf(buf, "%s", ad -> ad_r400adr);
	else {
		PP_LOG (LLOG_EXCEPTIONS, ("Error no address string given for recip '%d'\n", ix -> rr_recip));
		return RP_BAD;
	}
	*pmailbox = strdup(buf);
	return RP_OK;
}

static int allSameType(ix, type)
Rrinfo	*ix;
int	type;
{
	if (ix == NULL) return FALSE;
	while (ix != NULL && ix->rr_report.rep_type == type)
		ix = ix -> rr_next;
	if (ix == NULL)
		return TRUE;
	else
		return FALSE;
}

static int allSameMta(ix, pmta)
Rrinfo	*ix;
char	**pmta;
{
	ADDR	*ad;
	int	first = TRUE, allSame = TRUE;

	
	while (ix != NULL) {
		for (ad = PPQuePtr -> Raddress; ad; ad = ad->ad_next)
			if (ad -> ad_no == ix -> rr_recip)
				break;
		if (ad != NULL
		    && ad -> ad_outchan != NULL
		    && ad -> ad_outchan -> li_mta) {
			if (first == TRUE) {
				*pmta = strdup(ad -> ad_outchan -> li_mta);
				first = FALSE;
			} else if (lexequ(*pmta, ad -> ad_outchan -> li_mta) != 0)
				allSame = FALSE;
		}
		ix = ix -> rr_next;
	}
	if (first == FALSE && allSame == TRUE)
		return TRUE;
	return FALSE;
}
