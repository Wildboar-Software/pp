/* -- tb_dr.c Delivery Report tables, files located in Q/MSG -- */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_dr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_dr.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"dr.h"
#include	"tb_com.h"
#include	"tb_dr.h"
#include	<isode/cmd_srch.h>


/* ------------------------------------------------------------------------ */

CMD_TABLE   drtbl[] = { /* -- delivery-notification -- */
	"Dl-Exp-Hist",			DR_DL_EXP_HIST,
	"Dl-Exp-Hist-Crit",		DR_DL_EXP_HIST_CRIT,
	"Msgid",			DR_MSGID,
	"Per-Envelope-Extension",	DR_PER_ENVELOPE_EXTENSIONS,
	"Per-Report-Extension",		DR_PER_REPORT_EXTENSIONS,
	"Report-End",			DR_END,
	"Report-Hdr-End",		DR_END_HDR,
	"Report-Origin-Auth-Check",	DR_REPORT_ORIGIN_AUTH_CHECK,
	"Reporting-Mta-Certificate",	DR_REPORTING_MTA_CERTIFICATE,
	"Reporting-Name",		DR_REPORTING_DL_NAME,
	"Security-Label",		DR_SECURITY_LABEL,
	"Subject-Intermediate-Trace",	DR_SI_TRACE,
	"Trace",			DR_TRACE,
	0,				-1
};
int n_drtbl = ((sizeof(drtbl)/sizeof(CMD_TABLE)) -1);

CMD_TABLE rrtbl[] = {
	"Arrival-Time",			RR_ARRIVAL,
	"Converted_Eit",		RR_CONVERTED,
	"DR-Failure",			RR_FAILURE,
	"DR-Success",			RR_SUCCESS,
	"Originally-Intended-Recip",	RR_ORIGINALLY_INTENDED_RECIP,
	"Per-Recip-Extension",		RR_PER_RECIP_EXTENSIONS,
	"Physical-Fwd",			RR_PHYSICAL_FWD,
	"Recip-Certificate",		RR_RECIP_CERTIFICATE,
	"Recip-No",			RR_RECIP,
	"Redirect-History",		RR_REDIRECT_HISTORY,
	"Redirect-History-Crit",	RR_REDIRECT_HISTORY_CRIT,
	"Report-End",			RR_END,
	"Report-Origin-Check",		RR_REPORT_ORIGIN_CHECK,
	"Reported-Recipient-End",	RR_END_RECIP,
	"Supplementary-Info",		RR_SUPPLEMENTARY,
	0,				-1
};
int n_rrtbl = ((sizeof(rrtbl)/sizeof(CMD_TABLE)) -1);

CMD_TABLE  rr_tcode [] = {  /* -- MTS user type codes -- */
   "Public",				DRT_PUBLIC,
   "Private",				DRT_PRIVATE,
   "Ms",				DRT_MS,
   "Dl",				DRT_DL,
   "Pdau",				DRT_PDAU,
   "Physical-Recipient",		DRT_PHYSICAL_RECIPIENT,
   "Other",				DRT_OTHER,
   0,					-1,
   };

CMD_TABLE  rr_rcode [] = {  /* -- reason-codes -- */
   "Conversion-Not-Performed",		DRR_CONVERSION_NOT_PERFORMED,
   "Directory-Operation-Unsuccessful",	DRR_DIRECTORY_OP_UNSUCCESSFUL,
   "Phys-Delivery-Not-Performed",	DRR_PHYS_DELIVERY_NOT_PERFORMED,
   "Phys-Rendition-Not-Performed",	DRR_PHYS_RENDITION_NOT_PERFORMED,
   "Restricted-Delivery",		DRR_RESTRICTED_DELIVERY,
   "Transfer-Failure",			DRR_TRANSFER_FAILURE,
   "Unable-To-Transfer",		DRR_UNABLE_TO_TRANSFER,
   0,						-1
   };
int n_rr_rcode = ((sizeof(rr_rcode)/sizeof(CMD_TABLE)) - 1);

CMD_TABLE   rr_dcode [] = {  /* -- diagnostic-codes -- */
	"Alphabetic-Character-Loss",		DRD_ALPHABETIC_CHARACTER_LOSS,
	"Ambiguous-ORName",			DRD_AMBIGUOUS_OR,
	"Content-Syntax-Error",			DRD_CONTENT_SYNTAX_ERROR,
	"Content-Too-Long",			DRD_CONTENT_TOO_LONG,
	"Content-Type-Not-Supported",		DRD_CONTENT_TYPE_NOT_SUPPORTED,
	"Conversion-Impractical",		DRD_CONVERSION_IMPRACTICAL,
	"Conversion-Prohibited",		DRD_CONVERSION_PROHIBITED,
	"Conversion-With-Loss-Prohibited",	DRD_CONVERSION_WITH_LOSS_PROHIBITED,
	"Dl-Expansion-Failure",			DRD_DL_EXPANSION_FAILURE,
	"Dl-Expansion-Prohibited",		DRD_DL_EXPANSION_PROHIBITED,
	"Encoded-Information-Types-Unsupported",DRD_ENCINFOTYPES_NOTSUPPORTED,
	"Implicit-Conversion-Not-Registered",	DRD_IMPLICITCONV_NOTREGISTERED,
	"Invalid-Parameters",			DRD_INVALID_PARAMETERS,
	"Line-Too-Long",			DRD_LINE_TOO_LONG,
	"Loop-Detected",			DRD_LOOP_DETECTED,
	"Maximum-Time-Expired",			DRD_MAX_TIME_EXPIRED,
	"MTA-Congestion",			DRD_MTA_CONGESTION,
	"Multiple-Information-Loss",		DRD_MULTIPLE_INFORMATION_LOSS,
	"No-Bilateral-Agreement",		DRD_NO_BILATERAL_AGREEMENT,
	"No-Dl-Submit-Permission",		DRD_NO_DL_SUBMIT_PERMISSION,
	"Page-Split",				DRD_PAGE_SPLIT,
	"Physical-Rendition-Attributes-Not-Supported", DRD_PHYSICAL_RENDITION_ATTRIBUTES_NOT_SUPPORTED,
	"Pictorial-Symbol-Loss",		DRD_PICTORIAL_SYMBOL_LOSS,
	"Protocol-Violation",			DRD_PROTOCOL_VIOLATION,
	"Punctuation-Symbol-Loss",		DRD_PUNCTUATION_SYMBOL_LOSS,
	"Recipient-Reassignment-Prohibited",	DRD_RECIPIENT_REASSIGNMENT_PROHIBITED,
	"Redirection-Loop-Detected",		DRD_REDIRECTION_LOOP_DETECTED,
	"Secure-Messaging-Error",		DRD_SECURE_MESSAGING_ERROR,
	"Size-Constraint-Violation",		DRD_SIZE_CONSTRAINT_VIOLATION,
	"Too-Many-Recipients",			DRD_TOO_MANY_RECIPIENTS,
	"UA-Unavailable",			DRD_UA_UNAVAILABLE,
	"Unable-To-Downgrade",			DRD_UNABLE_TO_DOWNGRADE,
	"Undliv-New-Address-Unknown",		DRD_UNDLIV_NEW_ADDRESS_UNKNOWN,
	"Undliv-Organization-Expired",		DRD_UNDLIV_ORGANIZATION_EXPIRED,
	"Undliv-Originator-Prohibited-Forwarding",	DRD_UNDLIV_ORIGINATOR_PROHIBITED_FORWARDING,
	"Undliv-Pd-Address-Incomplete",		DRD_UNDLIV_PD_ADDRESS_INCOMPLETE,
	"Undliv-Pd-Address-Incorrect",		DRD_UNDLIV_PD_ADDRESS_INCORRECT,
	"Undliv-Pd-Office-Incorrect-Or-Invalid",DRD_UNDLIV_PD_OFFICE_INCORRECT_OR_INVALID,
	"Undliv-Recipient-Changed-Address-Permanently",	DRD_UNDLIV_RECIPIENT_CHANGED_ADDRESS_PERMANENTLY,
	"Undliv-Recipient-Changed-Address-Temporarily",	DRD_UNDLIV_RECIPIENT_CHANGED_ADDRESS_TEMPORARILY,
	"Undliv-Recipient-Changed-Temporary-Address",	DRD_UNDLIV_RECIPIENT_CHANGED_TEMPORARY_ADDRESS,
	"Undliv-Recipient-Deceased",		DRD_UNDLIV_RECIPIENT_DECEASED,
	"Undliv-Recipient-Did-Not-Claim",	DRD_UNDLIV_RECIPIENT_DID_NOT_CLAIM,
	"Undliv-Recipient-Did-Not-Want-Forwarding",	DRD_UNDLIV_RECIPIENT_DID_NOT_WANT_FORWARDING,
	"Undliv-Recipient-Refused-To-Accept",	DRD_UNDLIV_RECIPIENT_REFUSED_TO_ACCEPT,
	"Undliv-Recipient-Unknown",		DRD_UNDLIV_RECIPIENT_UNKNOWN,
	"Unrecognised-ORName",			DRD_UNRECOGNISED_OR,
	"Unsupported-Critical-Function", DRD_UNSUPPORTED_CRITICAL_FUNCTION,
	0,					-1
};
int n_rr_dcode = ((sizeof(rr_dcode)/sizeof(CMD_TABLE)) - 1);
