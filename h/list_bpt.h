/* list_bpt.h: A definition of a List structure for Body Part Types */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/list_bpt.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: list_bpt.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_LIST_BPT
#define _H_LIST_BPT


typedef struct list_bpt {
	char                    *li_name;
	struct list_bpt         *li_next;
} LIST_BPT;



#define NULLIST_BPT             ((LIST_BPT *)0)


extern  LIST_BPT                *list_bpt_dup();
extern  LIST_BPT                *list_bpt_find();
extern  LIST_BPT                *list_bpt_nfind();
extern  LIST_BPT                *list_bpt_malloc();
extern  LIST_BPT                *list_bpt_new();
extern	void			list_bpt_add ();
extern	void			list_bpt_free ();

#endif
