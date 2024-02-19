/* miscfuncs.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ntail/RCS/miscfuncs.c,v 6.0 1991/12/18 20:32:10 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ntail/RCS/miscfuncs.c,v 6.0 1991/12/18 20:32:10 jpo Rel $
 *
 * $Log: miscfuncs.c,v $
 * Revision 6.0  1991/12/18  20:32:10  jpo
 * Release 6.0
 *
 */



/*
 * @(#) miscfuncs.c 2.1 89/07/26 19:16:50
 *
 * Package:	ntail version 2
 * File:	miscfuncs.c
 * Description:	miscelaneous support procedures
 *
 * Mon Jul 10 02:56:22 1989 - Chip Rosenthal <chip@vector.Dallas.TX.US>
 *	Original composition.
 */

#ifndef LINT
static char SCCSID[] = "@(#) miscfuncs.c 2.1 89/07/26 19:16:50";
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <isode/usr.dirent.h>
#include "ntail.h"

#ifdef M_XENIX
# undef  NULL
# define NULL 0
#endif

extern int errno;

extern char *strerror(int);


/*
 * Scan a directory for files not currently on a list.
 */
int scan_directory(dirname)
char *dirname;
{
    register int i;
    register struct dirent *dp;
    register struct entry_descrip **elist, *entryp;
    char *basename;
    struct stat sbuf;
    DIR *dirp;
    static char pathname[MAXNAMLEN];

    Dprintf(stderr, ">>> scanning directory '%s'\n", dirname);
    if ( (dirp=opendir(dirname)) == NULL )
	return -1;

    (void) strcat( strcpy(pathname,dirname), "/" );
    basename = pathname + strlen(pathname);

#define SKIP_DIR(D) \
    ( D[0] == '.' && ( D[1] == '\0' || ( D[1] == '.' && D[2] == '\0' ) ) )

    while ( (dp=readdir(dirp)) != NULL ) {

	if ( SKIP_DIR(dp->d_name) )
	    continue;
	(void) strcpy( basename, dp->d_name );
	if ( stat(pathname,&sbuf) != 0 )
	    continue;
	if ( (sbuf.st_mode&S_IFMT) != S_IFREG )
	    continue;

	for ( i=List_file.num, elist=List_file.list ; i > 0 ; --i, ++elist ) {
	    if ( strcmp( (*elist)->name, pathname ) == 0 )
		break;
	}
	if ( i > 0 )
	    continue;

	for ( i=List_zap.num, elist=List_zap.list ; i > 0 ; --i, ++elist ) {
	    if ( strcmp( (*elist)->name, pathname ) == 0 )
		break;
	}
	if ( i > 0 )
	    continue;

	entryp = new_entry( &List_file, pathname );
	if ( Reset_status ) {
	    message( MSSG_CREATED, entryp );
	} else {
	    entryp->mtime = sbuf.st_mtime;
	    entryp->size = sbuf.st_size;
	}

    }

    (void) closedir(dirp);
    return 0;

}


/*
 * Compare mtime of two entries.  Used by the "qsort()" in "fixup_open_files()".
 */
static int ecmp(ep1,ep2)
register struct entry_descrip **ep1, **ep2;
{
    return ( (*ep2)->mtime - (*ep1)->mtime );
}

/*
 * Manage the open files.
 *   A small number of entries in "List_file" are kept open to minimize
 *   the overhead in checking for changes.  The strategy is to make sure
 *   the MAX_OPEN most recently modified files are all open.
 */
void fixup_open_files()
{
    register int i;
    register struct entry_descrip **elist;
    extern void qsort();

    Dprintf(stderr, ">>> resorting file list\n");
    (void) qsort(
	(char *) List_file.list,
	List_file.num,
	sizeof(struct entry_descrip *),
	ecmp
    );
    Sorted = TRUE;

    /*
     * Start at the end of the list.
     */
    i = List_file.num - 1;
    elist = &List_file.list[i];

    /*
     * All the files at the end of the list should be closed.
     */
    for ( ; i >= MAX_OPEN ; --i, --elist ) {
	if ( (*elist)->fd > 0 ) {
	    (void) close( (*elist)->fd );
	    (*elist)->fd = 0;
	}
    }

    /*
     * The first MAX_OPEN files in the list should be open.
     */
    for ( ; i >= 0 ; --i, --elist ) {
	if ( (*elist)->fd <= 0 )
	    (void) open_entry( &List_file, i );
    }

}


/*
 * Standard message interface.
 *   There are two reasons for this message interface.  First, it provides
 *   consistent diagnostics for all the messages.  Second, it manages the
 *   filename banner display whenever we switch to a different file.
 *   Warning - "errno" is used in some of the messages, so care must be
 *   taken not to step on it before message() can be called.
 */
void message(sel,e)
int sel;
struct entry_descrip *e;
{
    static char *ofile = NULL;

    /*
     * Don't display the file banner if the file hasn't changed since last time.
     */
    if ( sel == MSSG_BANNER && ofile != NULL && strcmp(ofile,e->name) == 0 )
	return;

    /*
     * Make sure the message selector is within range.
     */
    if ( sel < 0 || sel > MSSG_UNKNOWN )
	sel = MSSG_UNKNOWN;

    /*
     * Display the message.
     */
    if ( mssg_list[sel] != NULL )
	(void) printf(mssg_list[sel], e->name, strerror(errno));

    ofile = ( sel == MSSG_BANNER ? e->name : NULL );
}


/*
 * Display currently opened files.
 */
void show_status()
{
    int i, n;
    struct tm *tp;
    static char *monname[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    extern struct tm *localtime();

    (void) printf("\n*** recently changed files ***\n");
    for ( i = 0, n = 0 ; i < List_file.num ; ++i ) {
	if ( List_file.list[i]->fd > 0 ) {
	    tp = localtime(&List_file.list[i]->mtime);
	    (void) printf("%4d  %2d-%3s-%02d %02d:%02d:%02d  %s\n",
		++n,
		tp->tm_mday, monname[tp->tm_mon], tp->tm_year,
		tp->tm_hour, tp->tm_min, tp->tm_sec,
		List_file.list[i]->name
	    );
	}
    }

    (void) printf( 
	"currently watching:  %d files  %d dirs  %d unknown entries\n",
	List_file.num, List_dir.num, List_zap.num);

    message( MSSG_NONE, (struct entry_descrip *) NULL  );

}

