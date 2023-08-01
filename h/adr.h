/* adr.h: address structure definition */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/adr.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: adr.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_ADR
#define _H_ADR


#include "list_rchan.h"
#include "extension.h"
#include "mta.h"
#include "aparse.h"
#include <isode/psap.h>

typedef struct ad_redirection {
	struct ad_redirection *rd_next;
	char	*rd_addr;	/* O/R Name */
	char	*rd_dn;		/* Distinguished Name */
	UTC	rd_time;
	int	rd_reason;
#define RDR_RECIP_ASSIGNED 	0
#define RDR_ORIG_ASSIGNED	1
#define RDR_MD_ASSIGNED		2
} Redirection;

/* Address-types */

#define AD_ORIGINATOR                   1
#define AD_RECIPIENT                    2

typedef struct	adr_struct {
	int		ad_no;		/* recipient number */
					/* PP's key to this adr */

					/* Next 3 parms may be modfiied */
					/* in Q.  Fixed text encoding */
	int		ad_status;	/* recipient status */
#define  AD_STAT_UNKNOWN                0
#define  AD_STAT_PEND                   1
#define  AD_STAT_DRREQUIRED             2       /* DR required */
#define  AD_STAT_DRWRITTEN              3       /* DR written */
#define  AD_STAT_DONE                   4
	int		ad_rcnt;	/* reformatters done count  */

	/* -- MTS Service Parms */

	char		*ad_value;	/* address-original */
	char		*ad_dn;		/* Directory Distinguished Name */
					/* QUIPU string encoding */

	int		ad_usrreq;	/* user-report-request bit */
#define  AD_USR_NOREPORT                0
#define  AD_USR_BASIC                   1
#define  AD_USR_CONFIRM                 2
#define  AD_USR_NONE                    3
	int		ad_explicitconversion;	/* explicit conversion */
#define AD_EXP_NONE			-1
#define	AD_EXP_IA5_TEXT_TO_TELETEX	0
#define	AD_EXP_TELETEX_TO_TELEX		1
#define	AD_EXP_TELEX_TO_IA5_TEXT	2
#define	AD_EXP_TELEX_TO_TELETEX		3
#define	AD_EXP_TELEX_TO_G4_CLASS_1	4
#define	AD_EXP_TELEX_TO_VIDEOTEX	5
#define	AD_EXP_IA5_TEXT_TO_TELEX	6
#define	AD_EXP_TELEX_TO_G3_FACSIMILE	7
#define	AD_EXP_IA5_TEXT_TO_G3_FACSIMILE	8
#define	AD_EXP_IA5_TEXT_TO_G4_CLASS_1	9
#define	AD_EXP_IA5_TEXT_TO_VIDEOTEX	10
#define	AD_EXP_TELETEX_TO_IA5_TEXT	11
#define	AD_EXP_TELETEX_TO_G3_FACSIMILE	12
#define	AD_EXP_TELETEX_TO_G4_CLASS_1	13
#define	AD_EXP_TELETEX_TO_VIDEOTEX	14
#define	AD_EXP_VIDEOTEX_TO_TELEX	15
#define	AD_EXP_VIDEOTEX_TO_IA5_TEXT	16
#define	AD_EXP_VIDEOTEX_TO_TELETEX	17

	int		ad_type;	/* address-type */
#define  AD_X400_TYPE                   1
#define  AD_822_TYPE                    2
#define	 AD_ANY_TYPE			3

	char		*ad_orig_req_alt; /* originator requested alternate */
					/* recipient - std encoded */
	char		ad_orig_req_alt_crit;

#define AD_RDM_MAX	4
	int		ad_req_del[AD_RDM_MAX];	/* requested delivery method */
#define AD_RDM_NOTUSED  -1
#define AD_RDM_ANY 	0
#define AD_RDM_MHS 	1
#define AD_RDM_PD	2
#define AD_RDM_TLX	3
#define AD_RDM_TTX	4
#define AD_RDM_G3	5
#define AD_RDM_G4	6
#define AD_RDM_TTY	7
#define AD_RDM_VTX	8
	char		ad_req_del_crit;

	char		ad_phys_forward; /* boolean - is physical 	*/
					/* forwarding allowed */
	char		ad_phys_forward_crit;

	char		ad_phys_fw_ad_req;
	char		ad_phys_fw_ad_crit;
					/* boolean - request for phys */
					/* foward address */

	int		ad_phys_modes;
	char		ad_phys_modes_crit;
#define AD_PM_ORD	0x1
#define AD_PM_SPEC	0x2
#define AD_PM_EXPR	0x4
#define AD_PM_CNT	0x8
#define AD_PM_CNT_PHONE	0x10
#define AD_PM_CNT_TLX	0x20
#define AD_PM_CNT_TTX	0x40
#define AD_PM_CNT_BUREAU	0x80
#define AD_PM_MAX	0x80
					
	int		ad_reg_mail_type;
	char		ad_reg_mail_type_crit;
#define AD_RMT_UNSPECIFIED	-1
#define AD_RMT_NON_REG		0
#define AD_RMT_REG		1
#define AD_RMT_PERSON		2

	char		*ad_recip_number_for_advice;
	char		ad_recip_number_for_advice_crit;

	OID		ad_phys_rendition_attribs;
	char		ad_phys_rendition_attribs_crit;
	
	int		ad_pd_report_request;
	char		ad_pd_report_request_crit;
#define AD_PRR_UNSPECIFIED -1
#define AD_PRR_UNDELIV_PDS	0
#define AD_NTF_PDS		1
#define AD_NTF_MHS		2
#define AD_NTF_BOTH		3
	Redirection	*ad_redirection_history;
	char		ad_redirection_history_crit;

	struct qbuf	*ad_message_token;
	char		ad_message_token_crit;

	struct qbuf	*ad_content_integrity;
	char		ad_content_integrity_crit;

	int 		ad_proof_delivery;
	char		ad_proof_delivery_crit;
				/* boolean.  Is proof of delivery */
				/* requested */

	/* -- MTA Service Params (calculated for MTS AS) */
	
	int		ad_extension;	/* extension-id */
	int		ad_resp;	/* responsibility bit */
	int		ad_mtarreq;	/* mta-report-request bit */
#define  AD_MTA_NONE                    0
#define  AD_MTA_BASIC                   1
#define  AD_MTA_CONFIRM                 2
#define  AD_MTA_AUDIT_CONFIRM           3

	/* -- This may be supplied as a hint  but might get changed */
	
	int		ad_subtype;	/* address-subtype */
#define  AD_NOSUBTYPE                   0
#define  AD_JNT                         1
#define  AD_REAL733                     2
#define  AD_REAL822                     3
#define  AD_X400_84			4  /* Use this if compatible */
#define  AD_X400_88			5


	X400_Extension	*ad_per_recip_ext_list;
					/* bucket for new and private */
					/* odds and sods */

	/* -- parameters calculated by PP -- */
	/* -- Note: the outbound mta is held in ad_outchan -- */

	char		*ad_r400adr;	/* x400-addr */
	char		*ad_r822adr;	/* rfc-addr */
	
	/* -- should only be set for inbound x400 channels -- */
	char		*ad_r400orig;	/* original form of x400 address */

	/* -- next two should only be set for -- */
	/* -- unroutable originator addresses -- */
	char		*ad_r400DR;	/* x400 address for DRs */
	char		*ad_r822DR;	/* 822 address for DRs */

	char		*ad_content;	/* outgoing content type */

	LIST_RCHAN	*ad_fmtchan;	/* reformatting channels */
	LIST_RCHAN	*ad_outchan;	/* outbound channel structure */
	LIST_BPT	*ad_eit;	/* outgoing eit's */


	/* -- from here onwards extra PP specific info -- */
	/* -- Diagnostics for any address parsing failures -- */

	int		ad_parse_stat;	/* address parsing err status */
	char		*ad_parse_message; /* address parsing err message */
	int		ad_reason;	/* DR reason failure */
	int		ad_diagnostic;	/* DR diagnostic code */
	char		*ad_add_info;	/* DR supplementary info */

	Aparse		*aparse;	/* parsing bucket */

	/* -- Address Control File offsets for fixed length variables -- */

	off_t		ad_no_offset;	/* offset param for recip number */
	off_t		ad_stat_offset; /* offset param for recip status */
	off_t		ad_rcnt_offset; /* offset param for reformat next */


	struct adr_struct   *ad_next;
} ADDR;

#define NULLADDR ((ADDR *)0)

extern ADDR	*adr_new ();
extern void	adr_init ();
extern void	adr_free ();
extern void	adr_tfree ();
extern void	adr_add ();
extern void	redirection_free ();

#endif
