/* conf.c: basic configuration information */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/conf.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/conf.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: conf.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#ifndef TAILOR
#define TAILOR "."
#endif

#ifndef CMDDIR
#define CMDDIR "."
#endif

#ifndef LOGDIR
#define LOGDIR "."
#endif

#ifndef QUEDIR
#define QUEDIR "."
#endif

#ifndef TBLDIR
#define TBLDIR "."
#endif

#ifndef NIFTPQUEDIR
#define NIFTPQUEDIR "."
#endif

#ifndef NIFTPCPF
#define NIFTPCPF "."
#endif


char    *pptailor       = TAILOR;
char    *cmddfldir      = CMDDIR;
char    *logdfldir      = LOGDIR;
char    *quedfldir      = QUEDIR;
char    *tbldfldir      = TBLDIR;
char    *niftpquedir    = NIFTPQUEDIR;
char    *niftpcpf       = NIFTPCPF;
