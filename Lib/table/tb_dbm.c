/* tb_dbm.c: table lookup routines ([n]dbm based) */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_dbm.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_dbm.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_dbm.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "dbase.h"
#include "table.h"
#include "sys.file.h"

extern char *ppdbm, *tbldfldir;
extern	void	getfpath ();
extern	void	err_abrt ();

#ifndef       GCC_DBM_OK
      /* GCC_DBM_OK should be declared if you KNOW that your (n)dbm library
       * returns structures in the same was as gcc exxpects them.
       */
# ifdef       __GNUC__
#ifdef sparc
 #error GCC and sparc architecture dont get on very well with structures compile this file with cc
#endif
      /* most pcc based compilers screw up the return of structures.
       * They actually return a pointer to a static struct, which is copied.
       * Here is a HACK to get such a pcc version of the library to work
       * with the gnu structure returning convention. NB: need a cpp which
       * allows recursive definitions (such as gcc)
       */
#  ifdef NDBM
#   ifndef dbm_fetch
 #    error You are using gcc with ndbm, but have not fixed <ndbm.h>
 #    error (or defined GCC_DBM_OK if the dbm library is OK asis)
#   endif  /* dbm_fetch */
#  else       /* NDBM */
#   ifndef fetch
 #    error You are using gcc with dbm, but have not fixed <dbm.h>
 #    error (or defined GCC_DBM_OK if the dbm library is OK asis)
#   endif  /* fetch */
#  endif /* NDBM */
# endif /* __GNUC__ */
#endif /* GCC_DBM_OK */


/* ---------------------  Begin  Routines  -------------------------------- */

static int tb_fetch ();

/* -- find the value (address), given its key (hostname) -- */

int tb_dbmk2val (table, name, buf, first)
register Table          *table;
char                    name[],   /* -- name of ch "member" / key -- */
			*buf;     /* -- put value in this buffer -- */
int	first;
{
	static Dbvalue         dbm;
	static Dbptr  dp = NULL;
	register char   *cp;

	if (first) {
		if (tb_fetch (name, dbm))
			dp = dbm;
		else dp = NULL;
	}
	else if (dp)
		dp ++;

	for (; dp && (cp = dp->db_table) != NULLCP; dp++) {
		if (lexequ (cp, table -> tb_name) != 0)
			continue;
		if (buf != 0) {
			if (dp->db_value == NULL)
				*buf = '\0';
			else
				(void) strcpy (buf, dp->db_value);
		}
		return (OK);
	}
	if (buf != 0)
		(void) strcpy (buf, " (ERROR)");

	return (NOTOK);
}



/* ---------------------  Static  Routines  ------------------------------- */



static int tb_fetch (name, dbm)
char            *name;     /* -- use this key to fetch entry  -- */
Dbvalue         dbm;       /* -- put the entry here -- */
{
	static char     dbvalue[ENTRYSIZE];
	register Dbptr  dp;
	datum           key,
			value;
	register char   *p,
			*cp;
	int             cnt;

#ifdef GDBM
	static GDBM_FILE thedb;
#else
#ifdef NDBM
	static DBM      *thedb;
#else
	static int	savedirf, savepagf;
	int	tdirf, tpagf;
#endif
#endif
	static int      dbmopen = FALSE;


       if (dbmopen == FALSE) {
		char    filename [BUFSIZ];

		getfpath (tbldfldir, ppdbm, filename);

#ifdef GDBM
		(void) strcat (filename, ".gdbm");
		if ((thedb = gdbm_open (filename, 0, GDBM_READER, 0, NULL)) == NULL)
		     err_abrt (RP_FIO, "Error opening database '%s`",
			       filename);
#else
#ifdef NDBM
		if ((thedb = dbm_open (filename, O_RDONLY, 0)) == NULL)
			err_abrt (RP_FIO,
				 "Error opening database '%s'", filename);
#else
		if (dbminit (filename) < 0)
			err_abrt (RP_FIO,
				 "Error opening database '%s'", filename);
		savedirf = dirf;
		savepagf = pagf;
#endif
#endif
		dbmopen = TRUE;
	}


	key.dptr = dbvalue;
	key.dsize = strlen (name) + 1;
	for (cp = dbvalue, p = name; *p; p++)
		*cp++ = uptolow (*p);
	*cp = 0;


#ifdef GDBM
        value = gdbm_fetch (thedb, key);		    
#else
#ifdef NDBM
	value = dbm_fetch (thedb, key);
#else
	tpagf = pagf; tdirf = dirf;
	pagf = savepagf; dirf = savedirf;
	value = fetch (key);
	pagf = tpagf; dirf = tdirf;
#endif
#endif


	if (value.dptr == NULL
#ifdef NDBM
		|| dbm_error (thedb)
#endif NDBM
		)
	{
#ifdef NDBM
		dbm_clearerr (thedb);
#endif NDBM

		PP_DBG (("Lib/tb_fetch/FALSE (%s, %d)",
				key.dptr, key.dsize));
		return (FALSE);
	}



	(void) strcpy (dbvalue, value.dptr);

	cnt = MAXDBENTRIES-1;
	for (p = dbvalue, dp = dbm; *p && cnt ; cnt--, dp++) {
		for (dp->db_table = p ; *p ; p++)
			if (*p == ' ') {      /* get table name */
				*p++ = '\0';
				break;
			}
		for (dp->db_value = (*p ? p : NULL); *p; p++)
			if (*p == FS) {      /* get value-part */
				*p++ = '\0';
				break;
		}
	}
	for (; cnt ; cnt--, dp++)
		dp->db_table = dp->db_value = NULL;

	return (TRUE);
}
