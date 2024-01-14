/* lconsole.c: driver and main routine for the line console */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/lconsole/RCS/lconsole.c,v 6.0 1991/12/18 20:27:16 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/lconsole.c,v 6.0 1991/12/18 20:27:16 jpo Rel $
 *
 * $Log: lconsole.c,v $
 * Revision 6.0  1991/12/18  20:27:16  jpo
 * Release 6.0
 *
 */

#include "lconsole.h"
#include "qmgr-int.h"
#include <stdarg.h>

static char	*myname;

int verbose;
int authenticate;
int console_fd = NOTOK;
int accessmode = 0;
int indent = 0;

char *host = NULLCP;
char *realhost = NULLCP;
char *remoteversion = NULLCP;
char *user = NULLCP;
char *passwd = NULLCP;
int	batch = 0;

void main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	char *vec[NVARGS];
	int vecp;
	char *myhost, *mypasswd, *myuser;
	char **op = NULLVP;
	int status;

	myname = argv[0];
	myhost = mypasswd = myuser = NULLCP;
	if (isatty(2))
		verbose = 1;
	while((opt = getopt(argc, argv, "bQ:u:p:")) != EOF)
		switch (opt) {
		    case 'Q':
			myhost = optarg;
			break;

		    case 'u':
			authenticate = TRUE;
			myuser = optarg;
			break;

		    case 'p':
			authenticate = TRUE;
			mypasswd = optarg;
			break;

		    case 'b':
			batch = 1;
			break;

		    default:
			fprintf (stderr, "Usage: %s\n", myname);
			break;
		}
	argc -= optind;
	argv += optind;
	if (argc > 0)
		op = argv;

	if (myhost) {
		vecp = 0;
		vec[vecp++] = "open";
		vec[vecp++] = myhost;
		if (myuser) {
			vec[vecp++] = myuser;
			if (mypasswd)
				vec[vecp++] = mypasswd;
		}
		vec[vecp] = NULLCP;

		if (console_loop (vec, NOTOK) != OK && op)
			exit (1);
	}

	if(op) {
		for (vecp = 0; *op; op ++)
			vec[vecp++] = *op;
		vec[vecp] = NULLCP;

		if (console_loop (vec, NOTOK) != OK)
			status = 1;
	}
	else
		status = interactive ();

	if (console_fd != NOTOK) {
		vecp = 0;
		vec[vecp++] = "close";
		vec[vecp++] = NULLCP;

		(void) console_loop (vec, NOTOK);
	}

	exit(status); /* NOTREACHED */
}

int interactive ()
{
	char *vec[NVARGS];
	int status;
	char buffer[BUFSIZ];
	int i;

	for (;;) {
		if (get_line ("%s> ", buffer) == NOTOK)
			break;
		if (buffer[0] == NULL || buffer[0] == '#')
			continue;

		for (i = 0; i < NVARGS; i++)
			vec[i] = NULLCP;
		if (sstr2arg (buffer, NVARGS, vec, " \t") < 1 ||
		    *vec[0] == NULL)
			continue;

		switch (console_loop (vec, OK)) {
		    case NOTOK:
			status = 1;
			break;

		    case OK:
			continue;

		    case DONE:
			status = 0;
			break;
		}
		break;
	}
	return status;
}

int console_loop (vec, retval)
char **vec;
int retval;
{
	struct lc_dispatch *ds;

	if ((ds = getds (*vec)) == NULL)
		return retval;


	if (console_fd == NOTOK) {
		if (ds -> flags & LC_OPEN) {
			advise (NULLCP, "not connected to a queue manager");
			return retval;
		}
	}
	else if (ds -> flags & LC_CLOSE) {
		advise (NULLCP, "already connected to a queue manager");
		return retval;
	}
	if ((ds -> flags & LC_AUTH) && accessmode != FULL_ACCESS) {
		advise (NULLCP, "Operation requires full access rights");
		return retval;
	}

	switch ((*ds -> fnx)(vec)) {
	    default:
	    case NOTOK:
		return retval;

	    case OK:
		return OK;

	    case DONE:
		return DONE;
	}
}


int get_line (prompt, buffer)
char *prompt, *buffer;
{
	int c;
	char *cp, *ep;

	if (!batch) {
		printf (prompt, console_fd == NOTOK ? myname : realhost);
		(void) fflush (stdout);
	}

	for (ep = (cp = buffer) + BUFSIZ - 1; (c = getc (stdin)) != '\n';) {
		if (c == EOF) {
			if (!batch)
				putchar ('\n');
			clearerr(stdin);
			return NOTOK;
		}
		if (cp < ep)
			*cp ++ = c;
		
	}
	*cp = NULL;
	return OK;
}


static void _advise (va_list ap);

void adios (char *what, char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	_advise (ap);
	va_end (ap);
	_exit (1);
}

void advise (char *what, char *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	_advise (ap);
	va_end (ap);
}

static void _advise (va_list ap)
{
	char    buffer[BUFSIZ];
	asprintf (buffer, ap);
	(void) fflush (stdout);
	fprintf (stderr, "%s: ", myname);
	(void) fputs (buffer, stderr);
	(void) fputc ('\n', stderr);
	(void) fflush (stderr);
}

char *datasize (n, units)
int n;
char *units;
{
	static char dbuf[512];

	if (n < 1000)
		(void) sprintf (dbuf, "%d%s", n, units);
	else if (n < 100000)
		(void) sprintf (dbuf, "%.2lfK%s", (double)n/1000.0, units);
	else
		(void) sprintf (dbuf, "%.2lfM%s", (double)n/1000000.0, units);
	return dbuf;
}
