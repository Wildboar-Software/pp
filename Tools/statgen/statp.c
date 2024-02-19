/* statp.c: produce awk-able stats */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/statgen/RCS/statp.c,v 6.0 1991/12/18 20:33:14 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/statgen/RCS/statp.c,v 6.0 1991/12/18 20:33:14 jpo Rel $
 *
 * $Log: statp.c,v $
 * Revision 6.0  1991/12/18  20:33:14  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <stdarg.h>

char	*myname;
int	lineno;
static char *curfile;

static void process (FILE *fp, char *name);
static int parseline (char *line);
static void adios (char *what, char* fmt, ...);
static void advise (char *what, char *fmt, ...);

void main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;

	myname = argv[0];
	while((opt = getopt(argc, argv, "")) != EOF)
		switch (opt) {
		    default:
			fprintf (stderr, "Usage: %s\n", myname);
			break;
		}
	argc -= optind;
	argv += optind;
	if (argc > 0) {
		while (argc > 0) {
			FILE	*fp;

			if ((fp =fopen (*argv, "r")) == NULL)
				adios (*argv, "Can't open file");
			process (fp, *argv);
			fclose (fp);
			argv ++; argc --;
		}
	}
	else
		process (stdin, "standard input");
	exit (0);
}

static void process (FILE *fp, char *name)
{
	char	linebuf[BUFSIZ*4];

	curfile = name;
	lineno = 1;
	while (fgets (linebuf, sizeof linebuf, fp)) {
		parseline (linebuf);
		lineno ++;
	}
}

static int parseline (char *line)
{
	int	month, day;
	int	hour, mins, sec;
	char	*cp, *p;
	char	*argv[100];
	int	argc, n;

	month = atoi (line);
	if ((cp = index(line, '/')) == NULL)
		return badline (line, "No / date separator");

	day = atoi (++cp);

	while (*cp && isspace (*cp))
		cp ++;
	while (*cp && isdigit(*cp))
		cp ++;
	while (*cp && isspace (*cp))
		cp ++;

	if (sscanf (cp, "%d:%d:%d", &hour, &mins, &sec) != 3)
		return badline (cp, "No hour:min:sec field");

	if ((cp = index (line, ')')) == NULL)
		return badline (line, "No ')' in line");

	cp ++;
	if (index (cp, '|'))
		return badline (cp, "Line contains a '|'");


	printf ("%d:%d:%d:%02d:%02d|", month, day, hour, mins, sec);


	if ((p = index (cp, '>')) != NULL && p[-1] == '-')
		p[-1] = *p = ' ';
	argc = sstr2arg (cp, 100, argv, " \t\n=");

	if (lexequ (argv[0], "DR") == 0 )
		return do_dr (argc, argv);
	else if (lexequ (argv[0], "ok") == 0)
		return do_basic (argc, argv);
	else if (lexequ (argv[0], "deliv") == 0)
		 return do_deliv (argc, argv);
	else 
		return badline (argv[0], "unknown key");
}

int do_basic (argc, argv)
int	argc;
char	**argv;
{
	int n;

#define NF_NORM	18
	if (argc != NF_NORM) {
		char	buf[BUFSIZ];
		(void) sprintf (buf,
				"Incorrect number of fields %d, expecting %d",
				argc, NF_NORM);
		return badline (argv[0], buf);
	}

	printf ("%s|%s|%s|%s|",
		argv[0], argv[1], argv[2], argv[3]);

	n = 4;
	if (lexequ (argv[n], "p1msgid"))
		return badline (argv[n], "p1msgid missing");
	printf ("%s|", argv[++n]);
	n ++;

	if (lexequ(argv[n], "size"))
		return badline(argv[n], "size missing");
	printf ("%s|", argv[++n]);
	n++;

	if (lexequ (argv[n], "sender"))
		return badline (argv[n], "sender missing");
	printf ("%s|%s|", argv[n+1], argv[n+2]);
	n += 3;

	if (lexnequ (argv[n], "recip", 5))
		return badline (argv[n], "recip missing");
	printf ("%s|%s|%s|", argv[n+1], argv[n+2], argv[n+3]);
	n += 4;

	if (lexequ (argv[n], "submit-time"))
		return badline (argv[n], "submit-time missing");
	printf ("%s\n", argv[n+1]);

	return OK;
}	

int do_dr (argc, argv)
int	argc;
char	**argv;
{
	int n;
	int noneg;
	char	*p;

#define NF_DROK	22
	if (argc < NF_DROK) {
		char	buf[BUFSIZ];
		(void) sprintf (buf,
				"Incorrect number of fields %d, expecting at kleast %d",
				argc, NF_DROK);
		return badline (argv[0], buf);
	}

	printf ("%s|%s|%s|",
		argv[0], argv[1], argv[2]);

	noneg = lexequ (argv[1], "negative") != 0;

	n = 3;
	if (lexequ (argv[n], "p1msgid"))
		return badline (argv[n], "p1msgid missing");
	printf ("%s|", argv[++n]);
	n++;

	if (lexequ(argv[n], "size"))
		return badline (argv[n], "size missing");
	printf ("%s|", argv[++n]);
	n++;

	if (lexequ(argv[n], "inchan"))
		return badline(argv[n], "inchan missing");
	printf ("%s|", argv[++n]);
	n++;

	if (lexequ (argv[n], "dr_dest"))
		return badline (argv[n], "recip missing");
	printf ("%s|%s|", argv[n+1], argv[n+2]);
	n += 3;

	if (lexequ (argv[n], "outchan"))
		return badline (argv[n], "outchan missing");
	printf ("%s|", argv[++n]);
	n++;

	if (lexequ (argv[n], "dr-src"))
		return badline (argv[n], "dr-src missing");
	printf ("%s|%s|", argv[n+1], argv[n+2]);
	n += 3;

	if (lexequ (argv[n], "cur-channel"))
		return badline (argv[n], "cur-channel missing");
	printf ("%s", argv[++n]);
	n ++;

	if (noneg) {
		if (lexequ (argv[n], "submit-time"))
			return badline(argv[n], "submit-time missing");
		printf ("|%s", argv[n+1]);
		n += 2;
		
		if (lexequ (argv[n], "queued-time"))
			return badline(argv[n], "queued-time missing");
		printf ("|%s", argv[n+1]);
		n += 2;
		
	}

	if (n + 2 >= argc) {
		putchar ('\n');
		return OK;
	}
	else
		putchar ('|');

	if (lexequ (argv[n], "reason"))
		return badline (argv[n], "reason missing");
	printf ("%s|", argv[++n]);
	n ++;

	if (lexequ (argv[n], "diag"))
		return badline (argv[n], "diag missing");
	printf ("%s", argv[++n]);
	n ++;

	if (n >= argc) {
		putchar ('\n');
		return OK;
	}
	else
		putchar ('|');

	for (p = argv[n]; *p; p++)
		if (*p == '\n')
			*p = ' ';
	printf ("%s\n", argv[n]);
	return OK;
}	

int do_deliv (argc, argv)
int	argc;
char	**argv;
{
	int n;

#define NF_DELIV	19
	if (argc != NF_DELIV) {
		char	buf[BUFSIZ];
		(void) sprintf (buf,
				"Incorrect number of fields %d, expecting %d",
				argc, NF_DELIV);
		return badline (argv[0], buf);
	}

	printf ("%s|%s|%s|%s|", argv[0], argv[1], argv[2], argv[3]);

	n = 4;
	if (lexequ (argv[n], "p1msgid"))
		return badline (argv[n], "p1msgid missing");
	printf ("%s|", argv[++n]);
	n++;

	if (lexequ (argv[n], "size"))
		return badline (argv[n], "size missing");
	printf ("%s|", argv[++n]);
	n++;

	if (lexequ(argv[n], "sender"))
		return badline(argv[n], "sender missing");
	printf ("%s|", argv[++n]);
	n++;

	printf ("%s|", argv[n]);
	n++;

	if (lexequ (argv[n], "recip"))
		return badline (argv[n], "recip missing");
	printf ("%s|", argv[++n]);
	n++;

	printf ("%s|", argv[n]);
	n++;

	if (lexequ(argv[n], "submit-time"))
		return badline(argv[n], "submit-time missing");
	printf ("%s|", argv[++n]);
	n ++;

	if (lexequ(argv[n], "queued-time"))
		return badline (argv[n], "queued-time missing");
	printf ("%s\n", argv[++n]);

	return OK;
}	

int	badline (line, mesg)
char	*line, *mesg;
{
	putchar ('\n');
	advise (NULLCP, "Bad stat line '%s' line %d in %s: %s",
		line, lineno, curfile, mesg);
	return NOTOK;
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
