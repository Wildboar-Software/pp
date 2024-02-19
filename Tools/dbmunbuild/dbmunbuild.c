/* dbmunbuild.c: dump database into files. */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/dbmunbuild/RCS/dbmunbuild.c,v 6.0 1991/12/18 20:30:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/dbmunbuild/RCS/dbmunbuild.c,v 6.0 1991/12/18 20:30:06 jpo Rel $
 *
 * $Log: dbmunbuild.c,v $
 * Revision 6.0  1991/12/18  20:30:06  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "table.h" 
#include "dbase.h"
#include "sys.file.h"
#include <ctype.h>
#include <stdarg.h>

extern char *ppdbm;
extern char *dupfpath();
extern char *multcat(char *, ...);

extern Table **tb_all;

#ifdef GDBM
GDBM_FILE	thedb;
#define store(x,y)	gdbm_store (thedb, x, y, GDBM_REPLACE)
#define fetch(x)	gdbm_fetch (thedb, x)
#define firstkey()	gdbm_firstkey(thedb)
#define nextkey(x)	gdbm_nextkey(thedb, (x))
#else
#ifndef GCC_DBM_OK
#if sparc && defined(__GNUC__)	/* work around bug in gcc 1.37 sparc version */
 #error GCC and dbm do not get along on a sparc - compile with cc.
#endif
#endif
#endif

#ifdef NDBM
#define        fetch(x)        dbm_fetch(thedb, (x))
#define        store(x, y)     dbm_store(thedb, (x), (y), DBM_REPLACE)
#define        delete(x)       dbm_delete(thedb, (x))
#define firstkey()	dbm_firstkey(thedb)
#define nextkey(x)	dbm_nextkey(thedb)
DBM     *thedb;
# endif

#define TB_FLAG_MAGIC		0x100
#define TB_FLAG_NOTTHISONE	0x200

/*
 * dbm expanded structure
 */

#define MDB     50

static int     ndbents;

struct  db      {
	char    *d_table;
	char    *d_value;
} dbs[MDB], *lastdb;

static int verbose;
static int markedtbls;
#define MARKED_IN	1
#define MARKED_OUT	2

static char *myname;
static char *Usage = "Usage: %s [-d database] [-t table] [-v] directory";

static void dumpdb ();
static void write_records ();
static void write_entry ();
static void dssplit ();
static void dbfinit ();
static void open_table ();
static void close_a_file ();
static void mark_table ();
static void adios (char *what, char* fmt, ...);
static void advise (char *what, char *fmt, ...);

main(argc, argv)
int argc;
char **argv;
{
	int opt;
	extern char *optarg;
	extern int optind;
	sys_init(myname = argv[0]);

	while ((opt = getopt (argc, argv, "d:t:T:v")) != EOF) {
		switch (opt) {
		    case 'd':
			ppdbm = optarg;
			break;
		    case 'v':
			verbose = 1;
			break;
		    case 't':
			if (markedtbls == MARKED_OUT)
				adios (NULLCP, "Can't have -T and -t");
			markedtbls = MARKED_IN;
			mark_table (optarg);
			break;
		    case 'T':
			if (markedtbls == MARKED_IN)
				adios (NULLCP, "Can't have -T and -t");
			markedtbls = MARKED_OUT;
			mark_table (optarg);
			break;

		    default:
			adios (NULLCP, Usage, myname);
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
		adios (NULLCP, Usage, myname);

	if(!isstr(ppdbm))
		adios (NULLCP, "cannot find database path\n");
	dbfinit(ppdbm);

	dumpdb (*argv);
	exit (0);
}
static void mark_table (str)
char *str;
{
	Table *tb;

	if ((tb = tb_nm2struct (str)) == NULL)
		adios (NULLCP, "Unknown table %s", str);
	tb -> tb_flags |= TB_FLAG_NOTTHISONE;
}

static void dumpdb (dir)
char *dir;
{
	datum key;
	datum value;

	for (key = firstkey(); key.dptr != NULL; key = nextkey(key)) {
		value = fetch(key);
		if (value.dptr == NULLCP)
			adios (NULLCP, "Database fetch failed");
		write_records (&key, &value, dir);
	}
	close_tables ();
}


static void write_records (key, val, dir)
datum *key, *val;
char *dir;
{
	struct db *dp;

	dssplit (val -> dptr);
	for (dp = dbs; dp < lastdb; dp++)
		write_entry (key, dp, dir);
}

static void write_entry (key, dp, dir)
datum *key;
struct db *dp;
char *dir;
{
	Table *tb;

	if ((tb = tb_nm2struct (dp -> d_table)) == NULL) {
		advise (NULLCP, "No such table %s", dp -> d_table);
		return;
	}
	if (markedtbls == MARKED_OUT &&
	    (tb -> tb_flags & TB_FLAG_NOTTHISONE))
		return;
	if (markedtbls == MARKED_IN &&
	    (tb -> tb_flags & TB_FLAG_NOTTHISONE) == 0)
		return;

	if (tb -> tb_fp == NULL)
		open_table (tb, dir);
	fprintf (tb -> tb_fp, "%s:%s\n", key -> dptr, dp -> d_value);
	if (ferror (tb -> tb_fp))
		adios (dp -> d_table, "Write error on table");
}


static void dssplit(str)
register char   *str;
{
	register struct db      *dp;
	static  char    ti[BUFSIZ];

	ndbents = 0;
	str = strcpy(ti, str);
	for(dp = dbs ; dp < &dbs[sizeof(dbs)/sizeof(dbs[0])]; dp++){
		if(*str == 0)
			break;
		dp->d_table = str;
		while(*str && *str != ' ')
			str++;
		*str++ = 0;
		dp->d_value = str;
		while(*str && *str != FS)
			str++;
		ndbents++;
		if(*str == FS)
			*str++ = 0;
	}
	lastdb = dbs + ndbents;
}

/*
 * Initialize the dbm file.
 */

static void dbfinit(filename)
char *filename;
{
#ifdef GDBM
	char name[BUFSIZ];
	(void) sprintf (name, "%s.gdbm", filename);
	filename = name;
	if ((thedb = gdbm_open (filename, 0, GDBM_READER, 0666, NULL)) == NULL) {
#else
# ifdef NDBM
	if((thedb = dbm_open(filename, O_RDONLY, 0666)) == NULL) {
# else
	if(dbminit(filename) < 0) {
# endif
#endif
		adios (filename, "Can't initialize data base");
	}
}


static void open_table (tb, dir)
Table *tb;
char *dir;
{
	char filename[MAXPATHLENGTH];
	char *mode;

	if (tb -> tb_fp != NULL)
		return;

	(void) sprintf (filename, "%s/%s", dir, tb -> tb_file);

	if (verbose)
		printf ("Opening %s\n", filename);
	mode = (tb -> tb_flags & TB_FLAG_MAGIC) ? "a" : "w";
	if ((tb -> tb_fp = fopen (filename, mode)) == NULL) {
		if (errno == EMFILE) {
			close_a_file ();
					
		}
		if ((tb -> tb_fp = fopen (filename, mode)) == NULL)
			adios (filename, "Can't open file");
	}
}

static void close_a_file ()
{

	static Table **tbstart;
	Table **tb;

	if (tbstart == NULL)
		tbstart = tb_all;

	tb = tbstart;

	for (;;) {
		if ((*tb) -> tb_fp != NULL) {
			if (fclose ((*tb) -> tb_fp) == EOF)
				adios ("failed", "Write");
			(*tb) -> tb_fp = NULL;
			(*tb) -> tb_flags |= TB_FLAG_MAGIC;
			break;
		}
		tb ++;
		if (*tb == NULL)
			tb = tb_all;
	}
	tbstart = tb;
}

close_tables ()
{
	Table **tb;

	for (tb = tb_all; *tb; tb++)
		if ((*tb) -> tb_fp)
			if (fclose ((*tb) -> tb_fp) == EOF)
				adios ("failed", "fclose");
}

static void _advise (char *what, char* fmt, va_list ap);

static void adios (char *what, char* fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _advise (what, fmt, ap);
    va_end (ap);
    _exit (1);
}

static void advise (char *what, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _advise (what, fmt, ap);
    va_end (ap);
}

static void _advise (char *what, char* fmt, va_list ap)
{
    char    buffer[BUFSIZ];

    _asprintf (buffer, what, fmt, ap);

    (void) fflush (stdout);

    fprintf (stderr, "%s: ", myname);
    (void) fputs (buffer, stderr);
    (void) fputc ('\n', stderr);

    (void) fflush (stderr);
}
