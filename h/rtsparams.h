/* RtsParams.h: RTS parameter structures */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/rtsparams.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: rtsparams.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_RTSPARAMS
#define _H_RTSPARAMS

typedef struct RtsParams {
	char			*their_name;
	char			*their_passwd;
	char			*their_internal_ppname;
	char			*their_address;

	char			*our_name;
	char			*our_passwd;
	char			*our_address;

	char			*md_info;
	char			*try_next;

	char			*info_mode;
	char			*info_undefined;

	int			rts_mode;

	int			type;
#define RTSP_1984		1
#define RTSP_1988_X410MODE	2
#define RTSP_1988_NORMAL	3

	int			trace_type;
#define RTSP_TRACE_ADMD		1
#define RTSP_TRACE_NOINT	2
#define RTSP_TRACE_LOCALINT	3
#define RTSP_TRACE_ALL		4
	char			*fix_orig;
} RtsParams;


RtsParams       *tb_rtsparams();
void            RPfree();

#endif
