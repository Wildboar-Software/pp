/* dbase.h: Definitions for the dbm database */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/dbase.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: dbase.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_DBASE
#define _H_DBASE

#include "config.h"

/* most pcc based compilers screw up the return of structures.
 * They actually return a pointer to a static struct, which is copied.
 * Here is a HACK to get such a pcc version of the library to work with the
 * gnu structure returning convention. NB: need a cpp which allows recursive
 * definitions (such as gcc)
 */

#ifdef GDBM
#include "gdbm.h"

#else
#ifdef NDBM

#ifndef GCC_DBM_OK
#ifdef	      __GNUC__
#define		dbm_fetch	*dbm_fetch
#define		dbm_firstkey	*dbm_firstkey
#define		dbm_nextkey	*dbm_nextkey
#endif	      /* __GNUC__ */
#endif	      /* !GCC_DBM_OK */

#include	"ndbm.h"

#else

#ifndef GCC_DBM_OK
#ifdef __GNUC__
#define	      fetch	      *fetch
#define	      makdatum	      *makdatum
#define	      firstkey	      *firstkey
#define	      nextkey	      *nextkey
#define	      firsthash	      *firsthash
#endif	      /* __GNUC__ */
#endif	      /* !GCC_DBM_OK */

#include	<dbm.h>
#endif
#endif

#define		MAXDBENTRIES	32
#define		FS		'\034'	/* the dbm seperator character */
#define		ENTRYSIZE	1024	/* maximum database entry size */


typedef struct	{
	char	*db_table;
	char	*db_value;
}
  Dbvalue [MAXDBENTRIES],
  *Dbptr;



#endif
