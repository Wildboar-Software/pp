/* auth.h: authorisation */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/auth.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: auth.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_AUTH
#define _H_AUTH

#include "list_bpt.h"


#define MAX_AUTH_ARGS			100
#define AUTH_SEPERATOR			'|'


/* command table values */
#define	AUTH_LOGLEVEL_LOW	1
#define	AUTH_LOGLEVEL_MEDIUM	2
#define	AUTH_LOGLEVEL_HIGH	3


typedef	struct	list_regex {
	char			*li_regex;
	struct list_regex	*li_next;
	} LIST_REGEX;

#define NULLIST_REGEX		(LIST_REGEX *)0



typedef	struct	list_chan_rights {
	CHAN				*li_chan;
	int				li_rights;
	struct list_chan_rights		*li_next;
	} LIST_CHAN_RIGHTS;

#define NULLIST_CHAN_RIGHTS		(LIST_CHAN_RIGHTS *)0




typedef	struct	list_auth_chan {
	CHAN				*li_chan;
	int				li_found;

	int				policy;
#define AUTH_CHAN_FREE          	1
#define AUTH_CHAN_NEGATIVE      	2
#define AUTH_CHAN_BLOCK         	3
#define AUTH_CHAN_NONE          	4
#define AUTH_CHAN_WARNS         	5
#define AUTH_CHAN_WARNR         	6
#define AUTH_CHAN_SIZELIMIT     	7
#define AUTH_CHAN_TEST			8

	char				*warnsender;
	char				*warnrecipient;
	long				sizelimit;
	int				test; 
	struct 	list_auth_chan		*li_next;

	} LIST_AUTH_CHAN;


#define NULLIST_AUTHCHAN		(LIST_AUTH_CHAN *)0





typedef	struct	list_auth_mta {
	char				*li_mta;
	int				li_found;
	int				rights;
	LIST_REGEX			*requires;
	LIST_REGEX			*excludes;
	LIST_BPT			*content_excludes;
	LIST_CHAN_RIGHTS		*li_cr;
	long				sizelimit;
	struct 	list_auth_mta		*li_next;
	} LIST_AUTH_MTA;

#define NULLIST_AUTHMTA			(LIST_AUTH_MTA *)0



typedef	struct	auth_user {
	int				found;
	int				rights;
	LIST_BPT			*content_excludes;
	LIST_CHAN_RIGHTS		*li_cr;
	long				sizelimit;
	} AUTH_USER;

#define NULL_AUTHUSR			(AUTH_USER *)0



typedef	struct	auth {   /* holds all details for each channel, mta and user */

	int			status;
#define	AUTH_OK			0
#define AUTH_FAILED		1
#define AUTH_DR_OK		2
#define AUTH_DR_FAILED		3
#define AUTH_FAILED_TEST	4

	int			stage;
#define AUTH_STAGE_1		1	/* stage 1 tests performed */
#define AUTH_STAGE_2		2	/* stage 2 tests performed */

	int			warnings;
#define AUTH_NOTWARNED		0	/* msg not generated yet */
#define AUTH_WARNED		1	/* msg has been generated */
#define AUTH_WARNERROR		2	/* msg could not be generated */

	char			*reason;

	LIST_AUTH_CHAN		*chan;
	LIST_AUTH_MTA		*mta;
	AUTH_USER		*user;


	int			mta_inrights;
	int			mta_outrights;
	int			user_inrights;
	int			user_outrights;
#define AUTH_RIGHTS_IN          1
#define AUTH_RIGHTS_OUT         2
#define AUTH_RIGHTS_BOTH        3
#define AUTH_RIGHTS_NONE        4
#define AUTH_RIGHTS_UNSET       5
				
	} AUTH;

#define NULL_AUTH		(AUTH *)0

#endif
