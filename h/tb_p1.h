/* tb_p1.h: definitions used in I/O Queue Lines */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tb_p1.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tb_p1.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TB_P1
#define _H_TB_P1


/* MsgId */
#define MPDUID_STRING                   1

/* Trace */
#define TRACE_MTA                       1


/* GlobalId */
#define GLOBAL_COUNTRY                  1
#define GLOBAL_ADMIN                    2
#define GLOBAL_PRIVATE                  3


/* DomainSuppliedInfo */
#define DSI_TIME                        1
#define DSI_DEFERRED                    2
#define DSI_ACTION                      3
#define DSI_CONVERTED                   4
#define DSI_ATTEMPTED                   5


/* DomainSuppliedInfo - Action          */
#define ACTION_RELAYED                  0
#define ACTION_ROUTED                   1


/* EncodedInformationTypes - Set */
#define EI_BIT_STRING                   1
#define EI_G3NONBASIC                   2
#define EI_TELETEXNONBASIC              3
#define EI_PRESENTATION                 4


/* EncodedInformationTypes - Bit string */
#define EI_TOTAL                        10


#endif
