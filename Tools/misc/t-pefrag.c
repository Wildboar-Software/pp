/* pe_frag.c: test out the fragmentation stuff */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-pefrag.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-pefrag.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-pefrag.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/psap.h>

#define DEF_FRAGSIZE 128

char	*myname;
int	fragsize = DEF_FRAGSIZE;

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	int	i;

	myname = argv[0];
	sys_init (myname);

	while((opt = getopt(argc, argv, "f:")) != EOF)
		switch (opt) {
		    case 'f':
			fragsize = atoi (optarg);
			if (fragsize <= 0)
				fragsize = DEF_FRAGSIZE;
			break;
		    default:
			fprintf (stderr, "Usage: %s [-f size] [file]", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		FILE	*fp;
		if ((fp = fopen (*argv, "r")) == NULL) {
			fprintf (stderr, "Can't open file");
			perror (&argv);
			exit(1);
		}
		process (fp);
		(void) fclose (fp);
	}
	else	process (stdin);
	exit (0);
}

process (fp)
FILE	*fp;
{
	PS	ps;
	PE	pe;

	if ((ps = ps_alloc (std_open)) == NULLPS)
		ps_adios (ps, "ps_alloc (read)");

	if (std_setup (ps, fp) == NOTOK)
		ps_adios (ps, "Setup failed (read)");

	if ((pe = ps2pe (ps)) == NULLPE)
		ps_adios (ps, "ps2pe");

	ps_free (ps);

	if (pe_fragment (pe, fragsize) == NOTOK)
		ps_adios (ps, "pe_fragment");

	if ((ps = ps_alloc (std_open)) == NULLPS)
		ps_adios (ps, "ps_alloc (write)");

	if (std_setup (ps, stdout) == NOTOK)
		ps_adios (ps, "Setup failed (write)");

	if (pe2ps (ps, pe) == NOTOK)
		ps_adios (ps, "pe2ps");

	ps_free (ps);
}

ps_adios (ps, str)
PS	ps;
char	*str;
{
	fprintf (stderr, "%s: %s", myname, str);
	if (ps -> ps_errno)
		fprintf (stderr, " %s", ps_error (ps->ps_errno));
	putc ('\n', stderr);
	(void) fflush(stderr);
	exit(1);
}
