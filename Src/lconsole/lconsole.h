/* lconsole.h: line console definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/lconsole.h,v 6.0 1991/12/18 20:27:16 jpo Rel $
 *
 * $Log: lconsole.h,v $
 * Revision 6.0  1991/12/18  20:27:16  jpo
 * Release 6.0
 *
 *
 */

#include "util.h"

extern int verbose;
extern int authenticate;
extern int console_fd;
extern int accessmode;
extern int batch;
extern int indent;

extern FILE *fp_in;

extern char *host;
extern char *user;
extern char *passwd;
extern char *realhost;
extern char *remoteversion;

struct lc_dispatch {
	char	*name;
	IFP	fnx;

	int	flags;
#define LC_NONE		0x00
#define LC_OPEN		0x01
#define LC_CLOSE	0x02
#define LC_AUTH		0x04

	char *help;
};

struct lc_dispatch *getds();

#define NVARGS	10

void advise (), adios ();
extern char *datasize ();
extern void display_status ();
extern void free_chanlist ();
extern char *re_comp ();
extern int re_exec ();
extern char *fullchanname ();

#define TB_AMBIG	-1
#define TB_NONE		0
