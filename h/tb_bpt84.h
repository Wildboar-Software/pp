/* tb_bpt84.h: Body part definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tb_bpt84.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tb_bpt84.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TB_BP
#define _H_TB_BP


/* -- Unique Body Part types -- */

#define	BPT_HDR_P2			20
#define	BPT_HDR_822			21

#define BPT_ODIF			22

#define BPT_P2_DLIV_TXT			23

#define BPT_HDR_IPN			24

/* -- Repeated Body Part types -- */

#define	 BPT_UNDEFINED			-1
#define	 BPT_IA5			0
#define	 BPT_TLX			1
#define	 BPT_VOICE			2
#define	 BPT_G3FAX			3
#define	 BPT_TIF0			4
#define	 BPT_TTX			5
#define	 BPT_VIDEOTEX			6
#define	 BPT_NATIONAL			7
#define	 BPT_ENCRYPTED			8
#define	 BPT_IPM			9
#define	 BPT_SFD			10
#define	 BPT_TIF1			11
#endif
