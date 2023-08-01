/* data.h: data excahnge between rcvalert client/server */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/data.h,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: data.h,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_RCVALERT_DATA
#define _H_RCVALERT_DATA

struct data {
	long	refid;		/* network order */
	char	user[40];	/* username */
	char	data[512];	/* null terminated */
	char	from[64];	/* who its really from */
};

#define PORT		2345
#define N_RETRYS	3


#endif
