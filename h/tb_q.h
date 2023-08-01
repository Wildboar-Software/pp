/* tb_q.h: definitions used for I/O QUEUE/ADDR Control Files */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tb_q.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tb_q.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TB_Q
#define _H_TB_Q


/* MessageEnvelopeParameters */
#define Q_MSGTYPE               	1
#define Q_MSGSIZE               	2
#define Q_QUEUETIME             	3
#define Q_DEPARTIME             	4
#define Q_DEFERREDTIME          	5
#define Q_INCHANNEL             	6
#define Q_INHOST                	7
#define Q_NWARNS                	8
#define Q_WARNINTERVAL          	9
#define Q_RETINTERVAL           	10
#define Q_CONTENT_TYPE          	11
#define Q_ENCODED_INFO          	12
#define Q_PRIORITY              	13
#define Q_UA_ID                 	14
#define Q_MSGID                 	16
#define Q_TRACE                 	17
#define Q_DISCLOSE_RECIPS		18
#define Q_IMPICIT_CONVERSION		19
#define Q_ALTERNATE_RECIP_ALLOWED	20
#define Q_CONTENT_RETURN_REQUEST  	21
#define Q_LATESTTIME			22
#define	Q_RECIP_REASSIGN_PROHIBITED	23
#define	Q_DL_EXP_PROHIBITIED		24
#define Q_PP_CONT_CORR			25
#define Q_GEN_CONT_CORR			26
#define Q_END                   	27
#define	Q_IMPLICIT_CONVERSION		28
#define	Q_CONV_WITH_LOSS		29
#define	Q_ORIG_RETURN_ADDRESS		30
#define	Q_FORWARDING_REQUEST		31
#define	Q_ORIGINATOR_CERT		32
#define	Q_ALGORITHM_ID			33
#define	Q_MESSAGE_ORIGIN_AUTH_CHECK	34
#define	Q_SECURITY_LABEL		35
#define	Q_PROOF_OF_SUB			36
#define	Q_MESSAGE_EXTENSIONS		37
#define	Q_DL_EXP_HISTORY		38
#define	Q_DL_EXP_HIST_CRIT		39
#define Q_ORIG_ENCODED_INFO		40
#endif
