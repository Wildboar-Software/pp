/* dbmedit: edit a dbm file */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/dbmedit/RCS/dbmedit.c,v 6.0 1991/12/18 20:29:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/dbmedit/RCS/dbmedit.c,v 6.0 1991/12/18 20:29:41 jpo Rel $
 *
 * $Log: dbmedit.c,v $
 * Revision 6.0  1991/12/18  20:29:41  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "util.h" 
#include "dbase.h"
#include "sys.file.h"
#include <ctype.h>

extern char *ppdbm;
extern char *dupfpath();
extern char *multcat(char *, ...);

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

# ifdef NDBM
# define        fetch(x)        dbm_fetch(thedb, (x))
# define        store(x, y)     dbm_store(thedb, (x), (y), DBM_REPLACE)
# define        delete(x)       dbm_delete(thedb, (x))
DBM     *thedb;
# endif


datum   dcons();

char    ibuf[LINESIZE];
#define MARGS   100
char    *cargv[MARGS];                /* max number of args on a line */
int     cargc;

int     help(), add(), del(), change(), print(), quit();

struct  cmds    {
	char    *cm_name;
	int     (*cm_func)();
	char    *cm_help;
} cmds[] = {
	"help", help,   "Print out help text",
	"add",  add,    "Add an entry",
	"delete", del,  "Delete an entry",
	"change", change, "Change an entry",
	"print", print, "Print an entry",
	"quit", quit,   "Quit the program",
	"h",    help,   0,              /* single character aliases */
	"a",    add,    0,
	"d",    del,    0,
	"c",    change, 0,
	"p",    print,  0,
	"q",    quit,   0,
	"?",    help,   0,
}, *curcmd;

int     verbose;

/*
 * dbm expanded structure
 */

#define MDB     50

int     ndbents;

struct  db      {
	char    *d_table;
	char    *d_value;
} dbs[MDB], *lastdb;

main(argc, argv)
int argc;
char **argv;
{
	int opt;
	extern int optind;
	extern char *optarg;

	sys_init(argv[0]);

	while ((opt = getopt (argc, argv, "vd:")) != EOF) {
		switch (opt) {
		    case 'v':
			verbose++;
			break;
		    case 'd':
			ppdbm = optarg;
			break;
		    default:
			fprintf(stderr, "dbmedit: unknown argument '%c'\n",
				opt);
			exit(NOTOK);
		}
	}
	argv += optind;
	argc -= optind;
	if(!isstr(ppdbm)) {
		fprintf(stderr, "dbmedit: cannot find database path\n");
		exit(NOTOK);
	}
	dbfinit(ppdbm);

	/* if got command line arguments process these instead */
	if(argc > 0){
		avfix(argc, argv);
		if(findcmd() < 0)
			exit(1);
		exit((*curcmd->cm_func)());
	}

	for(;;){
		printf("dbmedit> ");
		(void) fflush(stdout);
		if (gets(ibuf) == NULL)
			exit(0);
		if (!isstr(ibuf))
			continue;
		if ((cargc = sstr2arg(ibuf, MARGS, cargv, " \t")) == 0)
			continue;
		if(findcmd() < 0)
			continue;
		(*curcmd->cm_func)();
	}
}

avfix(ac, av)
int ac;
register char   **av;
{
	register char   **ap;

	for(ap = cargv; ac ; ac--, cargc++)
		*ap++ = *av++;
}

findcmd()
{
	register struct cmds *cp;

	for(cp = cmds ; cp < &cmds[sizeof(cmds)/sizeof(cmds[0])] ; cp++)
		if(lexequ(cp->cm_name, cargv[0]) == 0){
			curcmd = cp;
			return(0);
		}
	fprintf(stderr, "dbmedit: unknown command '%s'\n", cargv[0]);
	return(-1);
}

help()
{
	register struct cmds *cp;

	for(cp = cmds ; cp < &cmds[sizeof(cmds)/sizeof(cmds[0])] ; cp++)
		if(cp->cm_help != 0)
			printf("%s\t\t%s\n", cp->cm_name, cp->cm_help);
}

datum
dcons(value)
char    *value;
{
	datum d;

	d.dptr = value;
	d.dsize = strlen(value)+1;
	return(d);
}

datum
dconskey(value)
char    *value;
{
	char	*p;

	for (p = value; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	return(dcons(value));
}

dssplit(str)
register char   *str;
{
	register struct db      *dp;
	static  char    ti[512];

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

add()
{
	datum   d;
	register struct db      *dp;

	if(cargc != 4){
		fprintf(stderr, "Usage: add key table value\n");
		return(9);
	}
	d = fetch(dconskey(cargv[1]));
	if(d.dptr == 0){
		if (verbose)
			printf("Creating new key \"%s\" value \"%s %s\"\n",
						cargv[1], cargv[2], cargv[3]);
		dp = lastdb = dbs;
		goto newalias;
	}
	dssplit(d.dptr);
	for(dp = dbs ; dp < lastdb ; dp++){
		if(lexequ(dp->d_table, cargv[2]) == 0){
			fprintf(stderr,
			  "key \"%s\" already has a value in table \"%s\"\n",
							cargv[1], cargv[2]);
			return(99);
		}
	}
newalias:;
	lastdb++;
	dp->d_table = cargv[2];
	dp->d_value = cargv[3];
	return(dscons());
}

del()
{
	int     i;
	datum   d;
	register struct db      *dp;
	int     changed = 0;

	if (cargc <= 2) {
		fprintf(stderr, "Usage: delete key table|* [table ...]\n");
		return(9);
	}

	d = fetch(dconskey(cargv[1]));
	if (d.dptr == 0) {
		fprintf(stderr, "key \"%s\" not found in database\n", cargv[1]);
		return(99);
	}
	if (strcmp(cargv[2], "*") == 0) {        /* all entries */
		if (verbose)
			printf("Deleteing key \"%s\"\n", cargv[1]);
		goto delall;
	}
	dssplit(d.dptr);
	for(i = 2 ; i < cargc ; i++) {
		for(dp = dbs ; dp < lastdb ; dp++) {
			if(dp->d_table && lexequ(dp->d_table, cargv[i]) == 0) {
				if (verbose)
					printf("deleting \"%s\" from table \"%s\"\n",
							cargv[1], cargv[i]);
				changed++;
				dp->d_table = 0;
				break;
			}
		}
		if(dp >= lastdb){
			fprintf(stderr,
				"\"%s\" does not have a value in table \"%s\"\n",
							cargv[1], cargv[i]);
			continue;
		}
	}
	if(!changed)
		return(99);
	if(lastdb - changed <= dbs) {
		if (verbose)
			printf("All values deleted for \"%s\" - deleting key\n",
								cargv[1]);
delall:;
		if(delete(dcons(cargv[1])) < 0)
			fprintf(stderr, "Delete failed\n");
		return(0);
	}
	return(dscons());
}

change()
{
	datum   d;
	register struct db      *dp;

	if(cargc < 3 || cargc > 4) {
		fprintf(stderr, "Usage: change key table [value]\n");
		return(9);
	}

	d = fetch(dconskey(cargv[1]));
	if(d.dptr == 0) {
		if (verbose)
			fprintf(stderr, "key \"%s\" not found in database\n",
				cargv[1]);
		if (cargc == 4)
			return(add());
		return(0);
	}
	dssplit(d.dptr);
	for(dp = dbs ; dp < lastdb ; dp++)
		if(lexequ(dp->d_table, cargv[2]) == 0){
			if (cargc == 3)
				return(del());
			if (verbose)
				printf("changing \"%s\" to \"%s\"\n",
						dp->d_value, cargv[3]);
			dp->d_value = cargv[3];
			break;
		}

	if(dp >= lastdb){
		if (cargc == 4)
			return(add());
		return(0);
	}
	return(dscons());
}

print()
{
	datum   d;
	register struct db      *dp;

	if(cargc < 2){
		fprintf(stderr, "Usage: print key [table]\n");
		return(9);
	}

	d = fetch(dconskey(cargv[1]));
	if(d.dptr == 0){
		fprintf(stderr, "key \"%s\" not found in database\n", cargv[1]);
		return(99);
	}
	dssplit(d.dptr);
	for(dp = dbs ; dp < lastdb ; dp++)
		if (cargc == 2 || lexequ(dp->d_table, cargv[2]) == 0 ||
		    lexequ("*", cargv[2]) == 0)
			printf("key \"%s\": table \"%s\": value \"%s\"\n",
				cargv[1], dp->d_table, dp->d_value);
	return(0);
}

dscons()
{
	char    tbuf[ENTRYSIZE];
	datum   d;
	register struct db      *dp;
	register char   *p, *q;
	int     changed = 0;

	for(p = tbuf, dp = dbs ; dp <lastdb ; dp++){
		if((q = dp->d_table) == 0)
			continue;
		if(p != tbuf)
			*(p-1) = FS;
		changed++;
		while(*p++ = *q++);
		*(p-1) = ' ';
		for(q = dp->d_value ; *p++ = *q++;);
	}
	if (!changed) {
		fprintf(stderr, "dbmedit: internal error 'no change'\n");
		return(99);
	}
	d = dcons (tbuf);

	if (store(dconskey(cargv[1]), d) < 0) {
		fprintf(stderr, "dbmedit: cannot store new value for '%s'\n", cargv[1]);
	}
	return(0);
}

/*
 * Initialize the dbm file.
 */

dbfinit(filename)
char *filename;
{
#ifdef GDBM
	char name[BUFSIZ];
	(void) sprintf (name, "%s.gdbm", filename);
	filename = name;
	if ((thedb = gdbm_open (filename, 0, GDBM_WRITER, 0666, NULL)) == NULL) {
#else
# ifdef NDBM
	if((thedb = dbm_open(filename, O_RDWR, 0666)) == NULL) {
# else
	if(dbminit(filename) < 0) {
# endif
#endif
	    fprintf (stderr, "could not initialize data base '%s'", filename);
	    perror("");
	    exit(99);
	}
}

quit()
{
	exit(0);
}
