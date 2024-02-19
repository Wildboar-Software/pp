/* io.h: input output */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/io.h,v 6.1 2023/08/02 07:44:11 jmw Rel $
 *
 * $Log: io.h,v $
 * Revision 6.1  2023/08/02  07:44:11  jmw
 * Release 6.1
 *
 *
 */



#ifndef _H_IO
#define _H_IO

#include "head.h"
#include "q.h"
#include "dr.h"

int io_init (RP_Buf *rp);
int io_wprm (struct prm_vars *prm, RP_Buf *rp);
int io_wrq (Q_struct *qp, RP_Buf *rp);
int io_wdr (DRmpdu *dp, RP_Buf *rp);
int io_wadr (ADDR *ap, int type, RP_Buf *rp);
int io_adend (RP_Buf *rp);
int io_tinit (RP_Buf *rp);
int io_tpart (char *section, int links, RP_Buf *rp);
int io_tdata (char *buf, int len);
int io_tdend (RP_Buf *rp);
int io_tend (RP_Buf *rp);
int io_end (int type);

#endif
