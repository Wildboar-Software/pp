/* aparse.h: holdall structure for various parsing entities for one address */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/aparse.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: aparse.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */

#ifndef _H_APARSE
#define _H_APARSE

#include "ap.h"
#include "or.h"
#include "chan.h"
#include "auth.h"
#include "list_rchan.h"

typedef struct  _bindings {
        LIST_RCHAN      *outchan;
        LIST_RCHAN      *fmtchans;
        LIST_BPT        *bpts;
        int             cost;
        struct _bindings        *next;
} Bindings;

typedef struct _aliasList {
	char	*alias;
	struct _aliasList	*next;
} aliasList;

typedef struct a_parse {

	int	ad_type;

/*-- rfc 822 entities --*/

	AP_ptr	ap_tree, 	/* complete tree  */
		ap_group,	/* start of 822 group */
		ap_name,	/* start of 822 name */
		ap_local,	/* start of 822 local */
		ap_domain,	/* start of 822 domain */
		ap_route;	/* start of 822 route */

	int	normalised;	/* degree of normalisation done */
#define	APARSE_NORM_NONE	0 
#define APARSE_NORM_NEXTHOP	1
#define APARSE_NORM_ALL		2

	int	dmnorder;	/* order required for domain ordering */
	int	full;		/* boolean indicating whether to parse fully */
	int	percents;	/* boolean indicating whether percents */
				/* are recognised as having routing properties */

	char	*r822_str,	/* string representation of address */
		*r822_local,	/* string representation of local part of address */
		*r822_error;	/* error message from 822 parse and validate */
/*-- x400 entities --*/
	ORName	*orname;	/* OR tree and DN representation of x400 address */
	OR_ptr	lastMatch;	/* where we matched to in or table */
	char	*x400_dn,	/* string representation of DN */
		*x400_str,	/* string representation of ORaddress */
		*x400_local,	/* string representation of local part of ORaddress */
		*x400_error;	/* error message from x400 parse and validate */
/*-- authorisation related entities --*/

	AUTH	*auth;

/*-- delivery related entities --*/
	aliasList	*aliases;	/* list of aliases unwound so far */
	char	*local_hub_name;	/* name of local hub */
	int	recurse_count;		/* number of times have recursed */
	int	dont_use_aliases;	/* whether to use aliases table or not */
					/* set after an external entry */
	int	internal;		/* used in header address normalisation */
					/* to specify whether to follow external entries */
					   
	Bindings	*channels;	/* all possible channels that can */
					/* be used to delivery to the address */
} Aparse, *Aparse_ptr;

extern Aparse_ptr 	aparse_new();
extern void		aparse_rewindr822();
extern void		aparse_rewindx400();
extern void		aparse_rewindlocal();
extern void		aparse_free();
extern void		aparse_copy_for_recursion();

extern void		aparse_addToAliasList();
extern int		aparse_inAliasList();
extern void		aparse_freeAliasList();
extern aliasList	*aparse_aliasDup();
#endif
