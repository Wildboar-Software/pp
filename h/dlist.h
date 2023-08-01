/* dlist.h: */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/dlist.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: dlist.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_DIRLIST_DLIST
#define _H_DIRLIST_DLIST

#include	"util.h"
#include	<isode/quipu/ufn.h>
#include	"or.h"
#include 	"DL-types.h"
#include 	"MD-types.h"

typedef struct dl_policy {
	char	dp_expand;

	int	dp_convert;
#define DP_ORIGINAL	1	/* default */
#define DP_FALSE	2
#define DP_TRUE		3	

	int	dp_priority;
/* #define DP_ORIGINAL	1 */
#define DP_LOW		2	/* default */
#define DP_NORMAL	3
#define DP_HIGH		4

} dl_policy;



typedef struct dl_permit {

	int 	dp_type;
#define DP_INDIVIDUAL	1
#define DP_MEMBER	2
#define DP_PATTERN	3
#define DP_GROUP	4

	union {
		ORName	* dp_un_or;
		DN	dp_un_dn;
	} dp_un;

#define dp_or dp_un.dp_un_or
#define dp_dn dp_un.dp_un_dn

} dl_permit;



#endif
