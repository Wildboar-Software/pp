/* tb_auth.h: auth definitions of tables used by submit*/

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tb_auth.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tb_auth.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TB_AUTH
#define _H_TB_AUTH

#define AUR_UNKNOWN                     1
#define AUR_CH_NONE                     2
#define AUR_CH_FREE                     3
#define AUR_IMTA_MISSING_SNDR           4
#define AUR_IMTA_EXCLUDES_SNDR          5
#define AUR_OMTA_MISSING_RECIP          6
#define AUR_OMTA_EXCLUDES_RECIP         7
#define AUR_MSGSIZE_FOR_CHAN            8
#define AUR_MSGSIZE_FOR_MTA             9
#define AUR_MSGSIZE_FOR_USR             10
#define AUR_MTA_BPT                     11
#define AUR_USR_BPT                     12
#define AUR_CHBIND                      13
#define AUR_DUPLICATE_REMOVED           14

#endif
