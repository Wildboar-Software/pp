/* tjoin.h: used to build the PP tables */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/tjoin.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: tjoin.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TJOIN
#define _H_TJOIN

#include <isode/general.h>	/* for bzero */


#define		LOADS		1000
#define		STRINGSTORESIZE	1000
#define		MTASTORESIZE	1000
#define		CHANSTORESIZE	1000

typedef	struct	print	{
	struct	print	*pr_next;
	int		pr_cost;
	char		*pr_chan;
	char		*pr_nexthop;
	char		pr_relay [LOADS];
	} PRINT;

typedef	struct	string	{
	struct	string	*str_next;
	int		str_index;
	char		str_buffer [STRINGSTORESIZE];
	} SBUFF;

typedef	struct	chan	{
	struct	chan	*ch_next;
	union	{
		struct	chan	*chan;
		struct	mta	*mta;
		char		*name;
		}	ch;
	} CHAN;

typedef	struct	mta	{
	char		*mta_name;
	struct	mta	*mta_left;
	struct	mta	*mta_right;
	struct	chan	*mta_chan;
	struct	chan	*mta_arlist;
	struct	chan	*mta_domain;
	} MTA;

typedef	struct	mtst	{		/* MTA Store */
	struct	mtst	*mts_next;
	int		mts_index;
	MTA		mts_buffer [MTASTORESIZE];
	} MTST;

typedef	struct	chst	{		/* CHAN Store */
	struct	chst	*chs_next;
	int		chs_index;
	CHAN		chs_buffer [CHANSTORESIZE];
	} CHST;

typedef	struct	host	{
	struct	host	*hst_next;
	char		*hst_name;
	CHAN		*hst_chan;
	} HOST;


#endif
