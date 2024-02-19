/* t-P2toRFC.c: P2 -> 822  test program */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/t-P2toRFC.c,v 6.0 1991/12/18 20:20:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/t-P2toRFC.c,v 6.0 1991/12/18 20:20:34 jpo Rel $
 *
 * $Log: t-P2toRFC.c,v $
 * Revision 6.0  1991/12/18  20:20:34  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "util.h"
#include "q.h"

extern void	sys_init(), or_myinit();

char    *usage = "t-P2to822 <822> <P2> [<extfile>]\n";
int	order_pref = CH_USA_PREF;
char  *rfc_out;
char  *P2_in;
char  *P2_ext_in;
char  *ipn_body_out;

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


	fprintf (stderr, "Welcome to P2 -> RFC 822 \n");

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

	P2_in = argv[optind++];
	rfc_out = argv[optind++];
	if (argc > optind)
		P2_ext_in = argv[optind];
	ipn_body_out = NULL;
	fprintf (stderr, "Initialised\n");

	if (P2toRFC (P2_in, P2_ext_in, (Q_struct *) 0, rfc_out, 
		     ipn_body_out, &error, 1) != OK)
		fprintf (stderr, "Blew it %s\n",
			 (error == NULLCP) ? "" : error );

}



void    _advise (char *what, char *fmt, va_list ap)
{
    (void) fflush (stdout);

    fprintf (stderr, "RFCtoP2 test");
    vfprintf (stderr, fmt, ap);
    if (what)
	(void) fputc (' ', stderr), perror (what);
    else
	(void) fputc ('\n', stderr);

    (void) fflush (stderr);
}

void    advise (char *what, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	_advise(what, fmt, ap);
	va_end(ap);
}

/* VARARGS 2 */
void    adios (char *what, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	_advise(what, fmt, ap);
	va_end(ap);
    _exit (1);
}
