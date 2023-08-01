typedef struct ad_redirection {
	struct ad_redirection *rd_next;
	char	*rd_addr;	/* O/R Name */
	char	*rd_dn;		/* Distinguished Name */
	UTC	rd_time;
	int	rd_reason;
} Redirection;

typedef struct	adr_struct {
	int		ad_no;		/* recipient number */
					/* PP's key to this adr */

					/* Next 3 parms may be modfiied */
					/* in Q.  Fixed text encoding */
	int		ad_status;	/* recipient status */
	int		ad_rcnt;	/* reformatters done count  */

	/* -- MTS Service Parms */

	char		*ad_value;	/* address-original */
	char		*ad_dn;		/* Directory Distinguished Name */
					/* QUIPU string encoding */

	int		ad_usrreq;	/* user-report-request bit */
	int		ad_explicitconversion;	/* explicit conversion */
	int		ad_type;	/* address-type */

	char		*ad_orig_req_alt; /* originator requested alternate */
					/* recipient - std encoded */
	char		ad_orig_req_alt_crit;

#define AD_RDM_MAX	4
	int		ad_req_del[AD_RDM_MAX];	/* requested delivery method */
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
					
	int		ad_reg_mail_type;
	char		ad_reg_mail_type_crit;
	char		*ad_recip_number_for_advice;
	char		ad_recip_number_for_advice_crit;

	OID		ad_phys_rendition_attribs;
	char		ad_phys_rendition_attribs_crit;
	
	int		ad_pd_report_request;
	char		ad_pd_report_request_crit;
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
	int		ad_subtype;	/* address-subtype */


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
