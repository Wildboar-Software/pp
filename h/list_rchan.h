/* list_rchan.h: a definition of a Reformatter Channel List struct */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/list_rchan.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: list_rchan.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_LIST_RCHAN
#define _H_LIST_RCHAN

#include "chan.h"
#include "auth.h"

/*
li_mta  - is the remote site/mta e.g "stl.stc.co.uk" or "DLCMHS" etc
li_chan - is the channel pointer for that site e.g "PSS" "x400out84" etc


Note:
	That though this list structure is used both for the reformatting
	and the outgoing channels, Only the outgoing channels have the
	li_name array set.
*/

typedef struct list_rchan {
	char                    *li_mta; /* the MTA */
	CHAN                    *li_chan; /* the channel its on */
	union {
		AUTH            *li_un_auth; /* auth stuff used in submit */
		char		*li_un_dir; /* the directory associated */
	} list_rchan_un;
	struct list_rchan       *li_next; /* next in the chain */
} LIST_RCHAN;
#define li_auth list_rchan_un.li_un_auth
#define li_dir list_rchan_un.li_un_dir

#define NULLIST_RCHAN           ((LIST_RCHAN *)0)

extern  LIST_RCHAN              *list_rchan_new();
extern  LIST_RCHAN              *list_rchan_malloc();
extern	void			list_rchan_free ();
extern	int			list_rchan_ssite ();
extern	int			list_rchan_schan ();
extern	void			list_rchan_add ();
#endif
