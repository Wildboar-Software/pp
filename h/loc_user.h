/* loc_user.h: information about a local user*/

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/loc_user.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: loc_user.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_LOC_USER
#define _H_LOC_USER

typedef struct  loc_user {
	int	uid;
	int	gid;

	char	*username;

	char	*directory;
	char	*mailbox;
	char	*shell;
	char	*home;
	int	mailformat;
#define MF_UNIX	1		/* sendmail format */
#define MF_PP	2		/* pp/mmdf format */
#define MF_AUTO	3		/* ??? */
	int	restricted;
	char	*mailfilter;
	char	*sysmailfilter;
	char	*searchpath;
	char	*opts;
} LocUser;

extern LocUser	*tb_getlocal ();
extern void	free_loc_user ();

#endif
