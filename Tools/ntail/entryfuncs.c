/* entryfuncs.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ntail/RCS/entryfuncs.c,v 6.0 1991/12/18 20:32:10 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ntail/RCS/entryfuncs.c,v 6.0 1991/12/18 20:32:10 jpo Rel $
 *
 * $Log: entryfuncs.c,v $
 * Revision 6.0  1991/12/18  20:32:10  jpo
 * Release 6.0
 *
 */



/*
 * @(#) entryfuncs.c 2.1 89/07/26 19:16:49
 *
 * Package:	ntail version 2
 * File:	entryfuncs.c
 * Description:	procedures to manage individual entries
 *
 * Mon Jul 10 02:56:22 1989 - Chip Rosenthal <chip@vector.Dallas.TX.US>
 *	Original composition.
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include "ntail.h"

#ifdef M_XENIX
# undef  NULL
# define NULL 0
#endif

extern int errno;


static struct entry_descrip *E_append(listp,entryp)
struct entry_list *listp;
struct entry_descrip *entryp;
{
    if ( listp->num >= MAX_ENTRIES ) {
	(void) fprintf(stderr,"%s: too many entries (%d max)\n",
	    entryp->name, MAX_ENTRIES);
	(void) exit(2);
    }
    listp->list[listp->num++] = entryp;
    Sorted = FALSE;
    return entryp;
}


static void E_remove(listp,entryno)
struct entry_list *listp;
int entryno;
{
    while ( ++entryno < listp->num )
	listp->list[entryno-1] = listp->list[entryno];
    --listp->num;
    Sorted = FALSE;
}


static char *list_name(listp)		/* for debug output only */
struct entry_list *listp;
{
    if ( listp == &List_file )	return "<file>";
    if ( listp == &List_dir )	return "<dir>";
    if ( listp == &List_zap )	return "<zap>";
    return "?unknown?";
}


/*
 * Create a new entry description and append it to a list.
 */
struct entry_descrip *new_entry(listp,name)
struct entry_list *listp;
char *name;
{
    struct entry_descrip *entryp;
    static char malloc_error[] = "malloc: out of space\n";

    Dprintf(stderr, ">>> creating entry '%s' on %s list\n",
	name, list_name(listp));

    entryp = (struct entry_descrip *) malloc( sizeof(struct entry_descrip) );
    if ( entryp == NULL ) {
	(void) fputs(malloc_error,stderr);
	(void) exit(2);
    }

    entryp->name = malloc( (unsigned) strlen(name) + 1 );
    if ( entryp->name == NULL ) {
	(void) fputs(malloc_error,stderr);
	(void) exit(2);
    }
    (void) strcpy(entryp->name,name);

    entryp->fd = 0;
    entryp->size =  0;
    entryp->mtime = 0;

    return E_append(listp,entryp);
}


/*
 * Remove an entry from a list and free up its space.
 */
void rmv_entry(listp,entryno)
struct entry_list *listp;
int entryno;
{
    struct entry_descrip *entryp = listp->list[entryno];
    extern void free();

    Dprintf(stderr, ">>> removing entry '%s' from %s list\n",
	listp->list[entryno]->name, list_name(listp));
    E_remove(listp,entryno);
    if ( entryp->fd > 0 )
	(void) close(entryp->fd);
    free( entryp->name );
    free( (char *) entryp );
}


/*
 * Move an entry from one list to another.
 *	In addition we close up the entry if appropriate.
 */
void move_entry(dst_listp,src_listp,src_entryno)
struct entry_list *dst_listp;
struct entry_list *src_listp;
int src_entryno;
{
    struct entry_descrip *entryp = src_listp->list[src_entryno];

    Dprintf(stderr, ">>> moving entry '%s' from %s list to %s list\n",
	src_listp->list[src_entryno]->name,
	list_name(src_listp), list_name(dst_listp));
    if ( entryp->fd > 0 ) {
	(void) close(entryp->fd);
	entryp->fd = 0;
    }
    E_remove(src_listp,src_entryno);
    (void) E_append(dst_listp,entryp);
    if ( Reset_status ) {
	entryp->size = 0;
	entryp->mtime = 0;
    }
}


/*
 * Get the inode status for an entry.
 *	Returns code describing the status of the entry.
 */
int stat_entry(listp,entryno,sbuf)
struct entry_list *listp;
int entryno;
register struct stat *sbuf;
{
    register int status;
    register struct entry_descrip *entryp = listp->list[entryno];
    static int my_gid = -1;
    static int my_uid = -1;

    if ( my_gid < 0 ) {
	my_gid = getegid();
	my_uid = geteuid();
    }

    status = 
	( entryp->fd > 0 ? fstat(entryp->fd,sbuf) : stat(entryp->name,sbuf) );

    if ( status != 0 )
	return ( errno == ENOENT ? ENTRY_ZAP : ENTRY_ERROR );

    if (
	( ( sbuf->st_mode & 0004 ) == 0 ) &&
	( ( sbuf->st_mode & 0040 ) == 0 || sbuf->st_gid != my_gid ) &&
	( ( sbuf->st_mode & 0400 ) == 0 || sbuf->st_uid != my_uid )
    ) {
	errno = EACCES;
	return ENTRY_ERROR;
    }

    switch ( sbuf->st_mode & S_IFMT ) {
	case S_IFREG:	return ENTRY_FILE;
	case S_IFDIR:	return ENTRY_DIR;
	default:	return ENTRY_SPECIAL;
    }

    /*NOTREACHED*/
}


/*
 * Open an entry.
 *	Returns 0 if the open is successful, else returns errno.  In the case
 *	of an error, an appropriate diagnostic will be printed, and the entry
 *	will be moved or deleted as required.  If the entry is already opened,
 *	then no action will occur and 0 will be returned.
 */
int open_entry(listp,entryno)
struct entry_list *listp;
int entryno;
{
    struct entry_descrip *entryp = listp->list[entryno];

    if ( entryp->fd > 0 )
	return 0;

    Dprintf(stderr, ">>> opening entry '%s' on %s list\n",
	listp->list[entryno]->name, list_name(listp));
    if ( (entryp->fd=open(entryp->name,O_RDONLY)) > 0 )
	return 0;

    if ( errno == ENOENT ) {
	message( MSSG_ZAPPED, entryp );
	move_entry( &List_zap, listp, entryno );
    } else {
	message( MSSG_OPEN, entryp );
	rmv_entry( listp, entryno );
    }
    return -1;
}


