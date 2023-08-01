/* dr.h: delivery report definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/dr.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: dr.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_DR
#define _H_DR
#include "extension.h"
#include "mta.h"

typedef struct	DeliveredInfo {
	UTC	del_time;
	int	del_type;
#define DRT_PUBLIC				0
#define DRT_PRIVATE				1
#define DRT_MS					2
#define DRT_DL					3
#define DRT_PDAU				4
#define DRT_PHYSICAL_RECIPIENT			5
#define DRT_OTHER				6
} Delinfo;


typedef struct	NonDeliveredInfo {
	int	nd_rcode;
#define  DRR_NO_REASON                          -1
#define  DRR_TRANSFER_FAILURE                   0
#define  DRR_UNABLE_TO_TRANSFER                 1
#define  DRR_CONVERSION_NOT_PERFORMED           2
#define  DRR_PHYS_RENDITION_NOT_PERFORMED	3
#define	 DRR_PHYS_DELIVERY_NOT_PERFORMED	4
#define  DRR_RESTRICTED_DELIVERY		5
#define  DRR_DIRECTORY_OP_UNSUCCESSFUL		6

	int	nd_dcode;
#define  DRD_UNRECOGNISED_OR                    0
#define  DRD_AMBIGUOUS_OR                       1
#define  DRD_MTA_CONGESTION                     2
#define  DRD_LOOP_DETECTED                      3
#define  DRD_UA_UNAVAILABLE                     4
#define  DRD_MAX_TIME_EXPIRED                   5
#define  DRD_ENCINFOTYPES_NOTSUPPORTED          6
#define  DRD_CONTENT_TOO_LONG                   7
#define  DRD_CONVERSION_IMPRACTICAL             8
#define  DRD_CONVERSION_PROHIBITED              9
#define  DRD_IMPLICITCONV_NOTREGISTERED         10
#define  DRD_INVALID_PARAMETERS                 11
#define	DRD_CONTENT_SYNTAX_ERROR	12
#define	DRD_SIZE_CONSTRAINT_VIOLATION	13
#define	DRD_PROTOCOL_VIOLATION	14
#define	DRD_CONTENT_TYPE_NOT_SUPPORTED	15
#define	DRD_TOO_MANY_RECIPIENTS	16
#define	DRD_NO_BILATERAL_AGREEMENT	17
#define	DRD_UNSUPPORTED_CRITICAL_FUNCTION	18
#define	DRD_CONVERSION_WITH_LOSS_PROHIBITED	19
#define	DRD_LINE_TOO_LONG	20
#define	DRD_PAGE_SPLIT	21
#define	DRD_PICTORIAL_SYMBOL_LOSS	22
#define	DRD_PUNCTUATION_SYMBOL_LOSS	23
#define	DRD_ALPHABETIC_CHARACTER_LOSS	24
#define	DRD_MULTIPLE_INFORMATION_LOSS	25
#define	DRD_RECIPIENT_REASSIGNMENT_PROHIBITED	26
#define	DRD_REDIRECTION_LOOP_DETECTED	27
#define	DRD_DL_EXPANSION_PROHIBITED	28
#define	DRD_NO_DL_SUBMIT_PERMISSION	29
#define	DRD_DL_EXPANSION_FAILURE	30
#define	DRD_PHYSICAL_RENDITION_ATTRIBUTES_NOT_SUPPORTED	31
#define	DRD_UNDLIV_PD_ADDRESS_INCORRECT	32
#define	DRD_UNDLIV_PD_OFFICE_INCORRECT_OR_INVALID	33
#define	DRD_UNDLIV_PD_ADDRESS_INCOMPLETE	34
#define	DRD_UNDLIV_RECIPIENT_UNKNOWN	35
#define	DRD_UNDLIV_RECIPIENT_DECEASED	36
#define	DRD_UNDLIV_ORGANIZATION_EXPIRED	37
#define	DRD_UNDLIV_RECIPIENT_REFUSED_TO_ACCEPT	38
#define	DRD_UNDLIV_RECIPIENT_DID_NOT_CLAIM	39
#define	DRD_UNDLIV_RECIPIENT_CHANGED_ADDRESS_PERMANENTLY	40
#define	DRD_UNDLIV_RECIPIENT_CHANGED_ADDRESS_TEMPORARILY	41
#define	DRD_UNDLIV_RECIPIENT_CHANGED_TEMPORARY_ADDRESS	42
#define	DRD_UNDLIV_NEW_ADDRESS_UNKNOWN	43
#define	DRD_UNDLIV_RECIPIENT_DID_NOT_WANT_FORWARDING	44
#define	DRD_UNDLIV_ORIGINATOR_PROHIBITED_FORWARDING	45
#define	DRD_SECURE_MESSAGING_ERROR	46
#define	DRD_UNABLE_TO_DOWNGRADE	47
	} NonDelinfo;


typedef struct	report	{
	char	rep_type;
#define  DR_REP_SUCCESS                         0
#define  DR_REP_FAILURE                         1
	union	{
		Delinfo		rep_dinfo;
		NonDelinfo	rep_ndinfo;
		} rep;
	} Report;



typedef struct	ReportedRecipientInfo {
	struct ReportedRecipientInfo *rr_next;
	int		rr_recip;	/* reference to adr struct */
	Report		rr_report;
	EncodedIT	*rr_converted;
	FullName	*rr_originally_intended_recip;
	char		*rr_supplementary;

	Redirection	*rr_redirect_history;
	char		rr_redirect_history_crit;

	FullName	*rr_physical_fwd_addr;
	char		rr_physical_fwd_addr_crit;

	struct qbuf	*rr_recip_certificate;
	char		rr_recip_certificate_crit;

	struct qbuf	*rr_report_origin_authentication_check;
	char		rr_report_origin_authentication_check_crit;

	UTC		rr_arrival;
	X400_Extension	*rr_per_recip_extensions;
	} Rrinfo;



					/* Originator, MPDU ID, content id  */
					/* Content correlator, EITs */
					/* be stored in associated addr */
					/* structure - no repeat	*/
typedef struct	DeliveryReportMPDU {
	MPDUid		*dr_mpduid;	/* envelope mpdu id */
	Trace		*dr_trace;	/* envelope trace */
	Trace		*dr_subject_intermediate_trace; /* content trace */

	DLHistory 	*dr_dl_history;
	char		dr_dl_history_crit;

	FullName 	*dr_reporting_dl_name; 
	char		dr_reporting_dl_name_crit;

	struct qbuf 	*dr_security_label;
	char		dr_security_label_crit;

	struct qbuf 	*dr_reporting_mta_certificate;
	char		dr_reporting_mta_certificate_crit;

	struct qbuf 	*dr_report_origin_auth_check;
	char		dr_report_origin_auth_check_crit;

	X400_Extension	*dr_per_envelope_extensions;
	X400_Extension	*dr_per_report_extensions;
	Rrinfo		*dr_recip_list;
} DRmpdu;

/* couple of useful defines */
#define DR_FILENO_DEFAULT                       1

extern void	dr_init ();
extern void	dr_free ();
extern void	rrinfo_free ();

#endif
