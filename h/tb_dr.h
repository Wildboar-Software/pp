/* tb_dr.h: definitions used in I/O  for Delivery Reports */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tb_dr.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tb_dr.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TB_DR
#define _H_TB_DR

#define	DR_MSGID			1
#define	DR_TRACE			2
#define	DR_SI_TRACE			3
#define	DR_DL_EXP_HIST			4
#define	DR_DL_EXP_HIST_CRIT		5
#define	DR_REPORTING_DL_NAME		6
#define	DR_SECURITY_LABEL		7
#define	DR_REPORTING_MTA_CERTIFICATE	8
#define	DR_REPORT_ORIGIN_AUTH_CHECK	9
#define DR_PER_ENVELOPE_EXTENSIONS	10
#define	DR_PER_REPORT_EXTENSIONS	11
#define	DR_END_HDR			12
#define DR_END				13

#define	RR_RECIP			1
#define	RR_SUCCESS			2
#define	RR_FAILURE			3
#define	RR_CONVERTED			4
#define	RR_ORIGINALLY_INTENDED_RECIP	5
#define	RR_SUPPLEMENTARY		6
#define	RR_REDIRECT_HISTORY		7
#define	RR_REDIRECT_HISTORY_CRIT	8
#define	RR_PHYSICAL_FWD			9
#define	RR_RECIP_CERTIFICATE		10
#define	RR_REPORT_ORIGIN_CHECK		11
#define	RR_ARRIVAL			12
#define	RR_PER_RECIP_EXTENSIONS		13
#define	RR_END_RECIP			14
#define RR_END				15

#endif
