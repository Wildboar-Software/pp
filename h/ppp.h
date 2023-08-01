/* ppp.h: ppp interface */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/ppp.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: ppp.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_PPP
#define _H_PPP

#define	PPP_STATUS_DONE			1
#define PPP_STATUS_CONNECT_FAILED	2
#define PPP_STATUS_PERMANENT_FAILURE	3
#define PPP_STATUS_TRANSIENT_FAILURE	4

int	ppp_init ();
int	ppp_getnextmessage ();
int	ppp_getdata ();
int	ppp_status ();
void	ppp_terminate ();


#endif
