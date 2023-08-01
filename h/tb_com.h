/* tb_com.h: common definitions used in the tb_tables ... */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tb_com.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tb_com.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TB_COMM
#define _H_TB_COMM


extern char*            Bf_put();
extern char*            Bf_app();

/*
Used in sequencing
*/

#define EOU             -9
#define EOB             -99
#define EOUNIT          "End-of-unit"
#define EOBLOCK         "End-of-block"
#define EMPTY           "empty"


#endif
