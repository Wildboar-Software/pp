/* prog.h: structures for the program/shell channel */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Chans/shell/RCS/prog.h,v 6.0 1991/12/18 20:11:52 jpo Rel $
 *
 * $Log: prog.h,v $
 * Revision 6.0  1991/12/18  20:11:52  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_SHELL_PROG
#define _H_SHELL_PROG


#include <pwd.h>

typedef struct prog_struct {
	unsigned int		timeout_in_secs;	/* in seconds */
	int			solo;
	char			*cmd_line;
	int			uid;
	int			gid;

	char			*username;
	char			*home;
	char			*shell;
}  ProgInfo;


#define NULLPROG		((ProgInfo *) 0)

ProgInfo			*tb_getprog();
void				prog_free();


#endif
