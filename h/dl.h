/* distlist.h: distribution list structure definition */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/dl.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: dl.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_DL
#define _H_DL

typedef struct namelist_struct {
	char			*name;
	char			*file;  /* file this name comes from */
					/* NULL => in string name */
	struct namelist_struct 	*next;
} Name;

typedef struct distlist_struct {
	char			*dl_listname,
				*dl_desc,
				*dl_file;
	char			*dl_moderator;
	Name			*dl_uids;	/* uids allowed to modify */
	Name			*dl_list;
} dl;
	
	
#define NULLDL	((dl *)0)

/* list acceess modes */
#define FREEMODE 0666
#define PUBMODE 0744
#define PRIVMODE 0644
#define SECRMODE 0600
#define TOPMODE 0000

#endif
