/* t-RFCtoP2.c: 822 -> P2 test program */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/t-RFCtoP2.c,v 6.0 1991/12/18 20:20:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/t-RFCtoP2.c,v 6.0 1991/12/18 20:20:34 jpo Rel $
 *
 * $Log: t-RFCtoP2.c,v $
 * Revision 6.0  1991/12/18  20:20:34  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "util.h"

extern void	or_myinit(), sys_init();

char    *usage = "t-822toP2 <822> <P2> [<extfile>]\n" ;

char  *rfc_in;
char  *P2_out;
char  *P2_ext_out;

main(argc, argv)
char    **argv;
int     argc;
{
	extern int optind, opterr;
	extern char     *optarg;
	int     opt;
	char	*error = NULLCP;
	sys_init(argv[0]);
	or_myinit ();

	fprintf (stderr, "Welcome to RFC 822 -> P2\n");

	opterr = 0;
	while((opt = getopt(argc, argv, "d:s:")) != EOF)
		switch(opt)
		{
			case 'd': 
					break;
			default:
				fprintf(stderr, "Unknown option %c\n", opt);
				fputs(usage, stderr);
				exit(1);
		}

	if (argc - optind < 2 ||
		argc - optind > 3)
	{
		fprintf (stderr, usage);
		exit (1);
	}
	rfc_in = argv[optind++];
	P2_out = argv[optind++];
	if (argc > optind)
		P2_ext_out = argv[optind];

	fprintf (stderr, "Initialised\n");

	RFCtoP2 (rfc_in, P2_out, P2_ext_out, &error, 1);

}



void    advise (what, fmt, a, b, c, d, e, f, g, h, i, j)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f,
       *g,
       *h,
       *i,
       *j;
{
    (void) fflush (stdout);

    fprintf (stderr, "RFCtoP2 test");
    fprintf (stderr, fmt, a, b, c, d, e, f, g, h, i, j);
    if (what)
	(void) fputc (' ', stderr), perror (what);
    else
	(void) fputc ('\n', stderr);

    (void) fflush (stderr);
}


/* VARARGS 2 */
void    adios (what, fmt, a, b, c, d, e, f, g, h, i, j)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f,
       *g,
       *h,
       *i,
       *j;
{
    advise (what, fmt, a, b, c, d, e, f, g, h, i, j);
    _exit (1);
}
