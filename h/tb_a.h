/* tb_a.h: definitions used for I/O Address Lines in queues */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tb_a.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tb_a.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TB_A
#define _H_TB_A




/* Address-Lines */

#define  AD_822                         1
#define  AD_ADDTYPE                     2
#define  AD_DR_NO                       3
#define  AD_END                         4
#define  AD_EXPLICITCONVERSION          5
#define  AD_EXTENSION_ID                6
#define  AD_MTA_REP_REQ                 7
#define  AD_ORIG                        8
#define  AD_OUTCHAN                     9
#define  AD_OUTHOST                     10
#define  AD_RECIP_NO                    11
#define  AD_REFORM_LIST                 12
#define  AD_REFORM_DONE                 13
#define  AD_RESPONSIBILITY              14
#define  AD_STATUS                      15
#define  AD_SUBTYPE                     16
#define  AD_USR_REP_REQ                 17
#define  AD_X400                        18
#define  AD_CONTENT                     19
#define  AD_EITS                        20
#define	AD_DN				21
#define AD_ORIG_REQ_ALT			22
#define AD_ORIG_REQ_ALT_CRIT		23
#define	AD_PHYS_FORWARD 		24
#define	AD_PHYS_FORWARD_CRIT 		25
#define	AD_PHYS_FW_AD 			26
#define	AD_PHYS_FW_AD_CRIT 		27
#define	AD_PHYS_MODES 			28
#define	AD_PHYS_MODES_CRIT 		29
#define	AD_REG_MAIL 			30
#define	AD_REG_MAIL_CRIT 		31
#define	AD_RECIP_NUMBER_ADVICE 		32
#define	AD_RECIP_NUMBER_ADVICE_CRIT	33
#define	AD_PHYS_RENDITION		34
#define	AD_PHYS_RENDITION_CRIT		35
#define	AD_PD_REPORT_REQUEST		36
#define	AD_PD_REPORT_REQUEST_CRIT	37
#define	AD_REDIRECTION_HISTORY		38
#define	AD_REDIRECTION_HISTORY_CRIT	39
#define	AD_MESSAGE_TOKEN		40
#define	AD_MESSAGE_TOKEN_CRIT		41
#define	AD_CONTENT_INTEGRITY		42
#define	AD_CONTENT_INTEGRITY_CRIT	43
#define	AD_PROOF_DELIVERY		44
#define	AD_PROOF_DELIVERY_CRIT		45
#define AD_EXTENSION			46
#define AD_REQ_DEL			47
#define AD_REQ_DEL_CRIT			48
#undef AD_AEXT
#define AD_X400DR			50
#define AD_822DR			51
#define AD_X400ORIG			52
/* ------------------------------------------------------------------------ */

#endif
