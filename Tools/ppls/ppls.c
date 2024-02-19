/* ppls: pp version of ls to test directory routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ppls/RCS/ppls.c,v 6.0 1991/12/18 20:32:36 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ppls/RCS/ppls.c,v 6.0 1991/12/18 20:32:36 jpo Rel $
 *
 * $Log: ppls.c,v $
 * Revision 6.0  1991/12/18  20:32:36  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include <stdarg.h>
#include <sys/stat.h>
#include <isode/usr.dirent.h>


char	*myname;
static int	selection ();
static void adios (char *, char *, ...), advise (char *, char *, ...);
extern int alphasort ();
void dols();

main (argc, argv)
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
			fprintf (stderr, "Usage: %s", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		dols (".");
	else while (argc-- > 0) {
		dols(*argv++);
	}
	exit (0);
}

void dols (directory)
char	*directory;
{
	struct stat st;
	struct dirent **namelist, **dp;
	int num, i;

	if (stat (directory, &st) == NOTOK)
		adios (directory, "Can't stat ");

	if ((st.st_mode & S_IFMT) != S_IFDIR)
		printf ("%s\n", directory);

	num = scandir (directory, &namelist, selection, alphasort);
	
	if (num == 0)
		return;

	for (i = 0, dp = namelist; i < num; i++, dp++)
		printf ("%s\n", (*dp) -> d_name);
	free ((char *)namelist);
}

static int selection (dp)
struct dirent *dp;
{
	if (*dp -> d_name == '.')
		return 0;
	return 1;
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
