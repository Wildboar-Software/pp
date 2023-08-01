/* rfc-hdr.h: Gives basic list of rfc header types */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/rfc-hdr.h,v 6.0 1991/12/18 20:20:34 jpo Rel $
 *
 * $Log: rfc-hdr.h,v $
 * Revision 6.0  1991/12/18  20:20:34  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_RFC1148_RFCHDR
#define _H_RFC1148_RFCHDR

#include	<isode/cmd_srch.h>


/* ---  generic values --- */
#define HDR_ERROR		-1
#define HDR_EOH			0
#define HDR_ILLEGAL		1
#define HDR_IGNORE		2

/* --- 822 fields with specific mappings --- */
#define HDR_MID			4
#define HDR_FROM		5
#define HDR_SENDER		6
#define HDR_REPLY_TO		7
#define HDR_TO			8
#define HDR_CC			9
#define HDR_BCC			10
#define HDR_IN_REPLY_TO		11
#define HDR_REFERENCES		12
#define HDR_SUBJECT		13
#define HDR_EXTENSIONS		14
#define HDR_COMMENT		15
#define HDR_INCOMPLETE_COPY	16
#define HDR_LANGUAGE		17
#define HDR_EXPIRY_DATE		18
#define HDR_REPLY_BY		19
#define HDR_OBSOLETES		20
#define HDR_IMPORTANCE		21
#define HDR_SENSITIVITY		22
#define HDR_AUTOFORWARDED	23
#endif
