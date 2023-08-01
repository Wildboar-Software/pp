/* ean2real.c: convert ean funny representation to real P1 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ean2real/RCS/ean2real.c,v 6.0 1991/12/18 20:30:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ean2real/RCS/ean2real.c,v 6.0 1991/12/18 20:30:18 jpo Rel $
 *
 * $Log: ean2real.c,v $
 * Revision 6.0  1991/12/18  20:30:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <varargs.h>
void advise();




/* ---------------------  Begin  Routines  -------------------------------- */
char	*myname;
int	process ();
int	skiphdr = 0;

main (argc, argv, envp)
int     argc;
char  **argv,
      **envp;
{
    extern char	*optarg;
    extern int	optind;
    int	opt;
    register char  *cp;
    register    FILE * fp;
    int         status = 0;

    myname = argv[0];
    while((opt = getopt(argc, argv, "ils")) != EOF)
	switch (opt) {
	case 'i':
	    ps_len_strategy = PS_LEN_INDF;
	    break;
	case 'l':
	    ps_len_strategy = PS_LEN_LONG;
	    break;
	case 's':
	    skiphdr = 1;
	    break;
	default:
	    fprintf (stderr, "Usage: %s [-s] [file...]", myname);
	    break;
	}
    argc -= optind;
    argv += optind;

    if (argc == 0)
	status = process ("(stdin)", stdin);
    else
	while (cp = *argv++) {
	    if ((fp = fopen (cp, "r")) == NULL) {
		advise (cp, "unable to read");
		status++;
		continue;
	    }
	    status += process (cp, fp);
	    (void) fclose (fp);
	}

    exit (status);
}



/* ---------------------  Static  Routines  ------------------------------- */



static int  process (file, fp)
register char *file;
register FILE *fp;
{
    register PE     pe, pe2;
    register PS     ps;

    if (skiphdr) {
	int i;
	for (i = 0; i < 12; i++)
	    (void) getc (fp);	/* remove junk */
    }


    if ((ps = ps_alloc (std_open)) == NULLPS) {
	advise ("process", "ps_alloc");
	return 1;
    }
    if (std_setup (ps, fp) == NOTOK) {
	advise (NULLCP, "%s: std_setup loses", file);
	return 1;
    }

    if ((pe = ps2pe (ps)) == NULLPE) {
	if (ps -> ps_errno) 
	    advise (NULLCP, "ps2pe: %s", ps_error(ps -> ps_errno));
	ps_free (ps);
	return 1;
    }
    pe = prim2seq (pe);
    if ((pe2 = ps2pe(ps)) == NULLPE) {
	if (ps -> ps_errno)
	    advise (NULLCP, "ps2pe: %s", ps_error(ps -> ps_errno));
	ps_free (ps);
	return 1;
    }
    ps_free (ps);

    if ((ps = ps_alloc(str_open)) == NULLPS) {
	advise ("process", "ps_alloc");
	return 1;
    }
    if (str_setup (ps, NULLCP, 0, 0) == NOTOK) {
	advise (NULLCP, "str_setup loses");
	return 1;
    }
    if (pe2ps (ps, pe2) == NOTOK) {
	if (ps -> ps_errno)
	    advise (NULLCP, "ps2pe: %s", ps_error(ps -> ps_errno));
	ps_free (ps);
	return 1;
    }
    pe_free (pe2);
    if((pe2 = oct2prim (ps -> ps_base, ps -> ps_ptr - ps -> ps_base)) == NULLPE ||
       seq_add (pe, pe2, NOTOK) == NOTOK) {
	advise (NULLCP, "Can't build new pe");
	return 1;
    }

    ps_free (ps);
    if ((ps = ps_alloc (std_open)) == NULLPS) {
	advise ("process", "ps_alloc");
	return 1;
    }
    if (std_setup (ps, stdout) == NOTOK) {
	advise (NULLCP, "%s: std_setup loses", file);
	return 1;
    }

    (void) ps_get_abs (pe);
    if (pe2ps (ps, pe) == NOTOK) {
	if (ps -> ps_errno)
	    advise (NULLCP, "ps2pe: %s", ps_error(ps -> ps_errno));
	ps_free (ps);
	return 1;
    }
    ps_free (ps);
    pe_free (pe);
    return 0;
}

#ifndef	lint
void	_advise ();


void	adios (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _advise (ap);

    va_end (ap);

    _exit (1);
}
#else
/* VARARGS */

void	adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif


#ifndef	lint
void	advise (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _advise (ap);

    va_end (ap);
}


static void  _advise (ap)
va_list	ap;
{
    char    buffer[BUFSIZ];

    asprintf (buffer, ap);

    (void) fflush (stdout);

    fprintf (stderr, "%s: ", myname);
    (void) fputs (buffer, stderr);
    (void) fputc ('\n', stderr);

    (void) fflush (stderr);
}
#else
/* VARARGS */

void	advise (what, fmt)
char   *what,
       *fmt;
{
    advise (what, fmt);
}
#endif

