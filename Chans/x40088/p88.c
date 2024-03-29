/* p88 : pretty print an X.400 1988 message */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/p88.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/p88.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: p88.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */


#include "Trans-types.h"
#include "util.h"
#include <isode/psap.h>

char	*myname;
void	adios (char *, char *, ...), advise (char *, char *, ...);

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
			fprintf (stderr, "Usage: %s\n", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		while (argc -- > 0) {
			FILE	*fp;

			if ((fp = fopen (*argv, "r")) == NULL)
				adios (*argv, "Can't open file");
			process (fp);
			(void) fclose (fp);
			argv ++;
		}
	}
	else
		process (stdin);
	exit (0);
}

process (fp)
FILE	*fp;
{
	PS	ps;
	PE	pe;

	if ((ps = ps_alloc (std_open)) == NULLPS)
		adios (NULLCP, "Can't allocate PS stream");

	if (std_setup (ps, fp) == NOTOK)
		adios (NULLCP, "Can't setup stream");

	if ((pe = ps2pe(ps)) == NULLPE)
		adios (NULLCP, "Error reading stream [%s]",
		       ps_error (ps -> ps_errno));

	PY_pepy[0] = NULL;
	print_Trans_MtsAPDU (pe, 1, 0, NULLCP, NULL);
	if (PY_pepy[0])
		advise (NULLCP, "Error : %s", PY_pepy);

	ps_free (ps);
	pe_free (pe);
}

#include <stdarg.h>

static void _advise (char *, char *, va_list ap);
void adios (char *what, char* fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
	_advise (what, fmt, ap);
    va_end (ap);
    _exit (1);
}

void advise (char *what, char *fmt, ...)
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