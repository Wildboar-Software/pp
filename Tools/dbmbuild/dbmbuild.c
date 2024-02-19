/* dbmbuild: build the database of information */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/dbmbuild/RCS/dbmbuild.c,v 6.0 1991/12/18 20:29:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/dbmbuild/RCS/dbmbuild.c,v 6.0 1991/12/18 20:29:34 jpo Rel $
 *
 * $Log: dbmbuild.c,v $
 * Revision 6.0  1991/12/18  20:29:34  jpo
 * Release 6.0
 *
 */



/* ---------------------------------------------------------------------------


	Database builder for PP address database

	Command line:
		dbmbuild [-flags] [outfile]

		where flags are:
		d   -   debugging output
		v   -   verbose output
		k   -   keep going if a table is missing
		n   -   don't make a new database file

	output-file is name of EXISTING dbm file to insert entires.
	Note it is two files, an output-file.dir and an output-file.pag.

--------------------------------------------------------------------------- */


#include        "head.h"
#include        "util.h"
#include        "chan.h"
#include        "dbase.h"
#include 	"sys.file.h"
#include <stdarg.h>

extern int      errno;
extern char     *tbldfldir,
		*ppdbm;

#ifdef GDBM
GDBM_FILE	thedb;
#define store(x,y)	gdbm_store (thedb, x, y, GDBM_REPLACE)
#define fetch(x)	gdbm_fetch (thedb, x)
#else
#ifndef GCC_DBM_OK
#if sparc && defined(__GNUC__)	/* work around bug in gcc 1.37 sparc version */
 #error GCC and dbm do not get along on a sparc - compile with cc.
#endif
#endif
#endif

#ifdef NDBM
DBM             *thedb;
#define         store(x,y)      dbm_store (thedb, (x), (y), DBM_REPLACE)
#define         fetch(x)        dbm_fetch (thedb, (x))
#endif


int             error,
		Debug,
		Verbose,
		newflag = 1,
		keepgoing;

char            dbfile [128],
		*dblock;

static char *myname;
static short    tb_nopen;   /* -- no of opened file descriptors -- */
void    adios (char *, char *, ...), advise (char *, char *, ...);

static int theinit (), theend ();
static int dbfinit (), dbfclose ();
static void process ();
static int install ();
static int prdatum ();
static int check ();
static int tb_open (), tb_close (), tb_free ();
static void cleanup ();

/* ---------------------  Begin  Routines  -------------------------------- */


/*
Process arguments, set flags and invoke file processing.
then clean up and exit properly.
*/


main (argc, argv)
char                    **argv;
{
	register Table      *tp;
	char                *p,
	*outfile = NULLCP;
	int                 ind;
	int		opt;
	extern char *optarg;
	extern int optind;

	myname = argv[0];
	sys_init (myname);

	while ((opt = getopt (argc, argv, "dknv")) != EOF) {
		switch (opt) {
		    case 'd':
			Debug++;
			break;
		    case 'k':
			keepgoing++;
			break;
		    case 'n':
			newflag = 0;
			break;
		    case 'v':
			Verbose++;
			break;
		    default:
			adios (NULLCP, "Usage: %s [-vnkd] [database]", myname);
		}
	}

	if (optind < argc)
		optarg = argv[optind++];

	/* -- use default database -- */

	if (outfile == NULLCP) {
		if (!isstr (ppdbm)) {
			adios (NULLCP,
			       "no default data base, in 'ppdbm' variable");
		}
		outfile = ppdbm;
	}


	/* -- check for existence first -- */

	error = 0;

	if (optind >= argc) {

		for (ind = 0; ch_all[ind] != NULLCHAN; ind++) {
			if (ch_all[ind] -> ch_table != NULLTBL)
				error += check (ch_all[ind] -> ch_table);

			if (Debug)
				fprintf (stderr, "ch_show = '%s' - %d\n",
					 ch_all[ind] -> ch_show, error);
		}


		for (ind = 0; (tp = tb_all[ind]) != NULLTBL; ind++) {
			error += check (tp);
			if (Debug)
				fprintf (stderr, "tb_show = '%s' - %d\n",
					 tp -> tb_show, error);
		}
	}
	else {
		int	ac = optind;
		while (ac < argc) {
			if ((tp = tb_nm2struct (argv[ac++])) == NULLTBL) {
				advise (NULLCP, "Table '%s' is unknown",
					argv[ac - 1]);
				error++;
			}
			error += check (tp);
		}
	}

	if (error)
		adios (NULLCP, "Some tables are missing.  dbmbuild aborted.");

	getfpath (tbldfldir, outfile, dbfile);

	if (Verbose || Debug) 
		fprintf (stderr, 
"database info:\n\ttables from %s\n\tdatabase = %s{.dir,.pag}\n\tpath = %s\n",
			 tbldfldir, outfile, dbfile);

	dblock = multcat (dbfile, ".lck", NULLCP);

	(void) close (creat (dblock, 0444 ));

	if (!theinit (newflag))
		cleanup (NOTOK);

	if (optind >= argc) {

		/* -- process for real! -- */

		for (ind = 0; ch_all[ind] != NULLCHAN; ind++)
			process (ch_all[ind] -> ch_table);


		for (ind = 0; (tp = tb_all[ind]) != NULLTBL; ind++) {

			/* -- in case table used by chan -- */

			if (tp -> tb_fp != (FILE *)EOF) {
				if (Debug)
					fprintf (stderr, "Table '%s'\n", tp -> tb_name);
				process (tp);
			}
		}

	}
	else
		while (optind < argc) {

			if ((tp = tb_nm2struct (argv[optind++])) == NULLTBL) {
				(void) fflush (stdout);
				adios (NULLCP, "Table '%s' is unknown",
				       argv[optind-1]);
			}
			process (tp);
		}

	cleanup (theend (newflag));
}


/* ---------------------  Static  Routines  ------------------------------- */




static int theinit (new)   /* -- initialize the dbm files -- */
int             new;
{
    char        tmpfile [100];


    /* -- start with fresh files -- */

    if (new) {
	if (Verbose || Debug)
	    fprintf (stderr, "Temporary database:  %s$\n", dbfile);
#ifdef DBM
	(void) sprintf (tmpfile, "%s$.pag", dbfile);

	if (Debug)
	    fprintf (stderr, "creating '%s'\n", tmpfile);


	if (close (creat (tmpfile, 0644)) < 0)
	    adios (tmpfile, "could not creat");

	(void) chmod (tmpfile, 0644);   /* -- in case umask screwed us -- */

	(void) sprintf (tmpfile, "%s$.dir", dbfile);

	if (Debug)
	    fprintf (stderr, "creating '%s'\n", tmpfile);


	/* -- create and/or zero the file -- */

	if (close (creat (tmpfile, 0644)) < 0)
	    adios (tmpfile, "could not creat");

	(void) chmod (tmpfile, 0644);   /* -- in case umask screwed us -- */
#endif
#ifdef GDBM
	(void) sprintf (tmpfile, "%s$.gdbm", dbfile);
#else
	(void) sprintf (tmpfile, "%s$", dbfile);
#endif
	return (dbfinit (tmpfile));
    }

    return (dbfinit (dbfile));
}




static theend (new)               /* cleanup the dbm files */
int             new;
{
    char        fromfile [100],
		tofile [100];


    dbfclose();

    /* -- started with fresh files -- */

    if (new) {
	if (Verbose || Debug)
	    fprintf (stderr, "Moving to database:  %s\n", dbfile);

#ifdef GDBM
	(void) sprintf (fromfile, "%s$.gdbm", dbfile);
	(void) sprintf (tofile, "%s.gdbm", dbfile);
	if (Debug)
		fprintf (stderr, "moving '%s'\n", fromfile);
	if (rename (fromfile, tofile) == NOTOK)
		adios (tofile, "could not rename %s to ", fromfile);
#else
	(void) sprintf (fromfile, "%s$.pag", dbfile);

	(void) sprintf (tofile, "%s.pag", dbfile);

	if (Debug)
	    fprintf (stderr, "moving '%s'\n", fromfile);

	(void) unlink (tofile);

	/* -- create and/or zero the file -- */

	if (rename (fromfile, tofile ) < 0 )
		adios (tofile, "could not rename %s to ", fromfile);

	(void) sprintf (fromfile, "%s$.dir", dbfile);

	(void) sprintf (tofile, "%s.dir", dbfile);

	if (Debug)
	    fprintf (stderr, "moving '%s'\n", fromfile);

	(void) unlink (tofile);

	/* -- create and/or zero the file -- */

	if (rename (fromfile, tofile ) == NOTOK)
		adios (tofile, "could not rename %s to ", fromfile);
#endif

	return (TRUE);
    }

    return (TRUE);
}




/*
Initialize the dbm file. Fetch the local name datum
Init to 1 and store it if there is no datum by that name.
*/

static dbfinit (filename)
char    *filename;
{
#ifdef GDBM
	if ((thedb = gdbm_open (filename, 0, GDBM_NEWDB, 0644, NULL)) == NULL)
#else
#ifdef NDBM
	if ((thedb = dbm_open (filename, O_CREAT|O_RDWR, 0644)) == NULL)
#else
	if (dbminit (filename) < 0)
#endif
#endif
	    adios (filename, "could not initialize data base");
	return (1);
}




/*
Close the dbm datafile.
Can't close the file because the library does not provide the call...
*/

static dbfclose()
{
#ifdef NDBM
	dbm_close (thedb);
#endif
#ifdef GDBM
	gdbm_close (thedb);
#endif
}




/*
Process a sequential file and insert items into the database
Opens argument assumes database is initialized.
*/

static void process (tp)
Table           *tp;
{
    datum       key,
		value;
    char        tbkey [LINESIZE],
		tbvalue [LINESIZE],
		*cp;
    int status;

    if (tp == NULLTBL)
	return;

    if (Verbose || Debug)
	fprintf (stderr, tp -> tb_fp == (FILE *)EOF ?
			"%s (%s) already done\n" : "%s (%s)\n",
			tp -> tb_show, tp -> tb_name);

    if (tp -> tb_fp == (FILE *)EOF)
	return;


    /* -- gain access to a channel table -- */

    if (!tb_open (tp)) {
	fprintf (stderr, "could not open table \"%s\" (%s, file = '%s'):\n\t",
			tp -> tb_show, tp -> tb_name,
			tp -> tb_file);
	perror ("");

	/* -- don't try again -- */
	tp -> tb_fp = (FILE *)EOF;

	if (keepgoing)
	    return;
	cleanup (NOTOK);
    }


    tbkey[0] = tbvalue[0] = '\0';
    while ((status = tab_fetch (tp -> tb_fp, tbkey, tbvalue)) != DONE) {

	if (status == NOTOK) {
		if (!keepgoing)
			adios (NULLCP, "Bad record in file %s (last was %s:%s)",
				 tp -> tb_name, tbkey, tbvalue);
		advise (NULLCP, "Bad record in file %s (last was %s:%s)", 
				tp -> tb_name, tbkey, tbvalue);
		continue;
	}

	value.dptr = tbvalue;
	value.dsize = strlen (tbvalue) + 1;

	/* -- all keys are lower case -- */
	for (cp = tbkey; *cp != 0; cp++)
	    *cp = uptolow (*cp);

	key.dptr = tbkey;
	key.dsize = strlen (tbkey) + 1;

	if (Debug)
	    fprintf (stderr, " (%d)'%s': (%d)'%s'\n",
		     key.dsize, key.dptr, value.dsize, value.dptr);

	install (key, value, tp -> tb_name);
    }

    if (ferror (tp -> tb_fp))
	advise ("", "i/o error with table %s (%s, file = %s)",
			    tp -> tb_show, tp -> tb_name,
			    tp -> tb_file);

    (void) tb_close (tp);

    /* -- don't try again -- */
    tp -> tb_fp = (FILE *)EOF;
}




/*
Install a datum into the database.
Fetch entry first to see if we have to append name for building entry.
*/

static install (key, value, tbname)
datum           key,
		value;
char            tbname[];
{
    datum       old;
    char        newentry [ENTRYSIZE],
		*p,
		*q;

    p = newentry;

    old = fetch (key);

    if (old.dptr != NULLCP) {
	if (Debug) {
	    fprintf (stderr, "\tFound old entry\n\t");
	    prdatum (old);
	}
	for (p = newentry, q = old.dptr ; *p++ = *q++;);
	*(p-1) = FS;
    }
    else
	*p = '\0';

    (void) sprintf (p, "%s %s", tbname, value.dptr);

    old.dptr = newentry;
    old.dsize = strlen (newentry)+1;

    if (Debug) {
	fprintf (stderr, "\tNew datum\n\t");
	prdatum (old);
    }


    /* -- put the datum back -- */

    if (store (key, old) < 0) {
	advise ("",
		"store failed; key='%s'", key.dptr);
	prdatum (old);
	cleanup (NOTOK);
    }

}




/*
Print a datum.
Takes the datum arg & prints it so it can be read as either a key or entry.
*/

static prdatum (value)
datum           value;
{
	int     cnt;
	char    data [512];

	(void) strcpy (data, value.dptr);

	for (cnt = 0; cnt < value.dsize; cnt++) {
		if (value.dptr [cnt] >= ' ' && value.dptr [cnt] <= '~')
			putc (value.dptr [cnt], stderr);
		else
			fprintf (stderr, "<\\%03o>", value.dptr [cnt]);
	}

	putc ('\n', stderr);
}




static check (tp)
Table   *tp;
{

    /* -- its bad -- */
    if (tp == NULLTBL) {
	if (Debug)
		fprintf (stderr, "check's tp is NULL\n");
	return (1);
    }

    if (tp->tb_fp == (FILE *)EOF) {
	if (Debug)
		fprintf (stderr, "check's tp has a EOF FILE \n");
	return (1);
    }

    /* -- gain access to a channel table -- */
    if (!tb_open (tp)) {
	advise ("", "could not open table \"%s\" (%s, file = '%s')",
		tp -> tb_show, tp -> tb_name, tp -> tb_file);
	tp -> tb_fp = (FILE *)EOF;
	return (1);
    }

    (void) tb_close (tp);
    tp -> tb_fp = NULLFILE;
    return (0);
}




static void 	cleanup (exitval)
int     exitval;
{
	exit (exitval == TRUE? 0 : 1);  /* TRUE is non-zero */
}

/* -------------  Basic  File  Action  ------------------------------------ */

static tb_open (tp)
register Table  *tp;   /* -- table's state information -- */
{
    char        temppath [128];


    /*
    currently, there is one file (descriptor) per table.  for a large
    configuration, you will run out of fd's.  until/unless you run
    a single-file dbm-based version, there is a hack, below, which
    limits the number of open file-descriptors to 6.  Trying to open
    a 7th will cause a currently-opened one to be closed.
    */

    if (tp == NULLTBL) {
	if (Debug)
		fprintf (stderr, "tb_open's table is NULL\n");
	return (FALSE);
    }

    if (tb_nopen < 6 || tb_free()) {

	getfpath (tbldfldir, tp -> tb_file, temppath);

	/* -- add on directory portion -- */
	if ((tp -> tb_fp = fopen (temppath, "r")) != NULLFILE) {
		tb_nopen++;
		return (TRUE);
	}
    }

    printf ("cannot open %s\n", temppath);

    return (FALSE);
}




static tb_close (tp)
register Table  *tp;
{
	if (tp -> tb_fp == (FILE *)NOTOK || tp -> tb_fp == NULLFILE)
		return (FALSE);

	(void) fclose (tp -> tb_fp);

	/* -- mark as free -- */
	tp -> tb_fp = 0;

	tb_nopen--;

	return (TRUE);
}




static tb_free()   /* -- create a free file descriptor -- */
{
    register Table      **tpp;

    /*
    step from the lowest search priority to the highest, looking for a
    channel to close.  return success as soon as one is done.
    */

    for (tpp = tb_all; *tpp != 0; tpp++) {
	if (tb_close (*tpp))
		return (TRUE);   /* -- got a channel closed -- */
    }
    return (FALSE);   /* -- couldn't get any closed -- */
}


#ifndef lint
static void    _advise ();


void    adios (char *what, char* fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);

	_advise (what, fmt, ap);

	va_end (ap);

	_exit (1);
}
#else
/* VARARGS */

void    adios (what, fmt)
char   *what,
       *fmt;
{
	adios (what, fmt);
}
#endif


#ifndef lint

void    advise (char* what, char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);

	_advise (what, fmt, ap);

	va_end (ap);
}

static void  _advise (char *what, char *fmt, va_list ap)
{
	char    buffer[BUFSIZ];

	_asprintf (buffer, what, fmt, ap);

	(void) fflush (stdout);

	fprintf (stderr, "%s: ", myname);
	(void) fputs (buffer, stderr);
	(void) fputc ('\n', stderr);

	(void) fflush (stderr);
}
#else
/* VARARGS */

void    advise (what, fmt)
char   *what,
       *fmt;
{
	advise (what, fmt);
}
#endif
