/* ntail.h: */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Tools/ntail/RCS/ntail.h,v 6.0 1991/12/18 20:32:10 jpo Rel $
 *
 * $Log: ntail.h,v $
 * Revision 6.0  1991/12/18  20:32:10  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_NTAIL
#define _H_NTAIL

#include "util.h"	/* to get system defn */

/*
 * @(#) xtail.h 2.1 89/07/26 19:16:49
 *
 * Package:	xtail version 2
 * File:	xtail.h
 * Description:	header definitions
 *
 * Mon Jul 10 02:56:22 1989 - Chip Rosenthal <chip@vector.Dallas.TX.US>
 *	Original composition.
 */



/*****************************************************************************
 *
 * Start of Site-Specific Customizations
 *
 *****************************************************************************/

/*
 * Define one of the following.  It says how to use your "directory" library.
 */
/*#define DIR_XENIX	/* include <sys/ndir.h>, use "struct direct"	*/
#define DIR_BSD	/* include <ndir.h>, use "struct direct"	*/
/*#define DIR_SYSV	/* include <dirent.h>, use "struct dirent"	*/

/*
 * Define one of the following.  It specifies the return type of "signal()".
 */
/*#define SIGTYPE	int	/* declare as "int (*signal)()"		*/
/*#define SIGTYPE	void	/* declare as "void (*signal)()"	*/
#if !defined(SVR3) && !defined(SVR4) && !defined(SUNOS4) && !defined(BSD44) && !defined(ultrix)
#define SIGTYPE     int
#else
#define SIGTYPE	void
#endif

/*
 * STATUS_ENAB	If defined, a SIGINT causes a summary of the opened files to
 *		be displayed, and a SIGQUIT terminates the program.  If not
 *		defined, these signals act normally.
 */
#define STATUS_ENAB	/**/

/*
 * SLEEP_TIME	An iteration through the checking loop is performed once
 *		per this many seconds.
 */
#define SLEEP_TIME	1

/*
 * MAX_OPEN	This number of most recently changed files is kept open, and
 *		they are checked every iteration through the checking loop.
 *		Keeping these files open improves the performance because we
 *		can use "fstat()" rather than "stat()".  Keeping too many
 *		files open may overflow your open file table, and will reduce
 *		performance by checking more files more frequently.
 */
#define MAX_OPEN	6

/*
 * CHECK_COUNT	Everything besides open files are checked once per this
 *		many iterations through the checking loop.
 */
#define CHECK_COUNT	5

/*
 * MAX_ENTRIES	The maximum number of entries in any list.  It can be fairly
 *		large -- each unused entry only eats 3*sizeof(char*) bytes.
 */
#define MAX_ENTRIES	512


/*****************************************************************************
 *
 * End of Site-Specific Customizations
 *
 *****************************************************************************/


#define TRUE 1
#define FALSE 0

#define Dprintf		if ( !Debug ) ; else (void) fprintf


/*
 * Codes returned by the "stat_entry()" procedure.
 */
#define ENTRY_ERROR	0	/* stat error or permissions error	*/
#define ENTRY_SPECIAL	1	/* entry is a special file		*/
#define ENTRY_FILE	2	/* entry is a regular file		*/
#define ENTRY_DIR	3	/* entry is a directory			*/
#define ENTRY_ZAP	4	/* specified entry doesn't exist	*/


/*
 * Diagnostic message codes.
 *   The ordering of codes must correspond to the "mssg_list[]" defined below.
 */
#define MSSG_NONE	0	/* no message - just reset header	*/
#define MSSG_BANNER	1	/* display banner for file output	*/
#define MSSG_CREATED	2	/* file has been created		*/
#define MSSG_ZAPPED	3	/* file has been deleted		*/
#define MSSG_TRUNC	4	/* file has been truncated		*/
#define MSSG_NOTAFIL	5	/* error - not a regular file or dir	*/
#define MSSG_STAT	6	/* error - stat() failed		*/
#define MSSG_OPEN	7	/* error - open() failed		*/
#define MSSG_SEEK	8	/* error - lseek() failed		*/
#define MSSG_READ	9	/* error - read() failed		*/
#define MSSG_UNKNOWN	10	/* unknown error - must be last in list */


#ifdef INTERN
#   define EXTERN
#else
#   define EXTERN extern
#endif


/*
 * Each item we are watching is stored in a (struct entry_descrip).  These
 * entries are placed in lists, which are managed as (struct entry_list).
 *
 * There are three lists maintained:
 *
 * List_file	All of the regular files we are watching.  We will try to
 *		keep the MAX_OPEN most recently modified files open, and
 *		they will be checked more frequently.
 *
 * List_dir	All of the directories we are watching.  If a file is created
 *		in one of these directories, we will add it to "List_file".
 *
 * List_zap	All the entries which don't exist.  When something appears
 *		under one of these names, the entry will be moved to either
 *		"List_file" or "List_dir", as appropriate.
 */

struct entry_descrip {
    char *name;		/* pathname to the entry			*/
    int fd;		/* opened fd, or <= 0 if not opened		*/
    long size;		/* size of entry last time checked		*/
    long mtime;		/* modification time last time checked		*/
};

struct entry_list {
    struct entry_descrip *list[MAX_ENTRIES];
    int num;
};

/*
 * The lists of entries being watched.
 */
EXTERN struct entry_list List_file;	/* regular files		*/
EXTERN struct entry_list List_dir;	/* directories			*/
EXTERN struct entry_list List_zap;	/* nonexistent entries		*/


/*
 * List sorting status.
 *   This flag indicates that "List_file" is sorted, and the right entries
 *   are open.  Anything which possibly effects this state (e.g. an entry
 *   is added to "List_file", the mtime of a file is changed, etc.) must set
 *   this flag FALSE.  We will periodically check this flag and call the
 *   "fixup_open_files()" procedure to resort and organize the list.
 */
EXTERN int Sorted;


/*
 * Entry status control flag.
 *   The procedures which manipulate entries will reset the status information
 *   if this flag is TRUE.  When initializing the lists we want this FALSE.
 *   For example, consider the file size.  When initializing we want to use
 *   the current file size, otherwise we would dump the file from the beginning.
 *   However, later when we notice things are created we want to reset the
 *   size to zero so that we do dump from the beginning.
 */
EXTERN int Reset_status;


/*
 * Debugging output flag.
 */
EXTERN int Debug;


/*
 * Diagnostic messages produced by the "message()" procedure.
 *   The first "%s" is the entry name.  The second "%s" is the errno descrip.
 */
#ifdef INTERN
    char *mssg_list[] = {
	NULL,							/*MSSG_NONE   */
	"\n*** %s ***\n",					/*MSSG_BANNER */
	"\n*** '%s' has been created ***\n",			/*MSSG_CREATED*/
	"\n*** '%s' has been deleted ***\n",			/*MSSG_ZAPPED */
	"\n*** '%s' has been truncated - rewinding ***\n",	/*MSSG_TRUNC  */
	"\n*** error - '%s' not a file or dir - removed ***\n",	/*MSSG_NOTAFIL*/
	"\n*** error - couldn't stat '%s' (%s) - removed ***\n",/*MSSG_STAT   */
	"\n*** error - couldn't open '%s' (%s) - removed ***\n",/*MSSG_OPEN   */
	"\n*** error - couldn't seek '%s' (%s) - removed ***\n",/*MSSG_SEEK   */
	"\n*** error - couldn't read '%s' (%s) - removed ***\n",/*MSSG_READ   */
	"\n*** error - unknown error on file '%s' ***\n",	/*MSSG_UNKNOWN*/
    };
#else
    extern char *mssg_list[];
#endif


/*
 * Entry managment procedures.
 */
struct entry_descrip *new_entry();	/* create a new entry and add to list */
void move_entry();			/* move an entry between lists	      */
void rmv_entry();			/* remove an entry from a list	      */
int stat_entry();			/* get the inode status for an entry  */
int open_entry();			/* open an entry		      */

/*
 * Miscelaneous procedures.
 */
void fixup_open_files();		/* manage the open files	      */
int scan_directory();			/* scan a dir for files not on a list */
void message();				/* standard message interface	      */
void show_status();			/* display currently opened files     */


#endif
