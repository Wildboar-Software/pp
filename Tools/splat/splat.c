/* splat.c: produces a ASN.1 dump */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/splat/RCS/splat.c,v 6.0 1991/12/18 20:32:50 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/splat/RCS/splat.c,v 6.0 1991/12/18 20:32:50 jpo Rel $
 *
 * $Log: splat.c,v $
 * Revision 6.0  1991/12/18  20:32:50  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <stdarg.h>
void advise (char *what, char *fmt, ...);




/* ---------------------  Begin  Routines  -------------------------------- */
char	*myname;
int	process1 (), process2 ();


void main (argc, argv, envp)
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
    IFP		fnx = process1;

    myname = argv[0];
    while((opt = getopt(argc, argv, "d")) != EOF)
	switch (opt) {
	case 'd':
	    fnx = process2;
	    break;
	default:
	    fprintf (stderr, "Usage: %s", myname);
	    break;
	}
    argc -= optind;
    argv += optind;

    if (argc == 0)
	status = (*fnx) ("(stdin)", stdin);
    else
	while (cp = *argv++) {
	    if ((fp = fopen (cp, "r")) == NULL) {
		advise (cp, "unable to read");
		status++;
		continue;
	    }
	    status += (*fnx) (cp, fp);
	    (void) fclose (fp);
	}

    exit (status);
}



/* ---------------------  Static  Routines  ------------------------------- */



static int  process1 (file, fp)
register char *file;
register FILE *fp;
{
    register PE     pe;
    register PS     ps;
    PS              ps_out;

    if ((ps = ps_alloc (std_open)) == NULLPS) {
	advise ("process", "ps_alloc");
	return 1;
    }
    if (std_setup (ps, fp) == NOTOK) {
	advise (NULLCP, "%s: std_setup loses", file);
	return 1;
    }

    if ((ps_out = ps_alloc (std_open)) == NULLPS)
	printf ("failed on ps_alloc\n");
    if (std_setup (ps_out, stdout) != OK)
	printf ("failed to asssign stdout\n");

    for (;;) {
	if ((pe = ps2pe (ps)) == NULLPE)
	    if (ps -> ps_errno) {
		advise (NULLCP, "ps2pe: %s", ps_error(ps -> ps_errno));
		ps_free (ps);
		return 1;
	    }
	    else {
		ps_free (ps);
		return 0;
	    }


	pe2pl (ps_out, pe);

	pe_free (pe);
    }
}

#define ps_advise(ps, str) advise (NULLCP, "%s: %s", str, ps_error(ps -> ps_errno));

int process2 (file, fp)
char	*file;
FILE	*fp;
{
    register PE     pe;
    register PS     ps;
    PS              ps_out;

    if ((ps = ps_alloc (std_open)) == NULLPS) {
	advise ("process", "ps_alloc");
	return 1;
    }
    if (std_setup (ps, fp) == NOTOK) {
	advise (NULLCP, "%s: std_setup loses", file);
	return 1;
    }

    for (;;) {
	if (read_stuff (ps, 0) == NOTOK)
	    return 1;
    }
}


read_stuff (ps, depth)
PS	ps;
int	depth;
{
    PElementClass class;
    PElementForm form;
    PElementID id;
    int len;
    
    if (ps_read_id (ps, 1, &class, &form, &id) == NOTOK) {
	ps_advise (ps, "ps_read_id");
	return NOTOK;
    }
    printf ("%*sClass %s, form %s, id %d ", depth * 2, "",
	    pe_classlist[class],
	    form == PE_FORM_PRIM ? "primitive" : "constructor",
	    id);
    if (ps_read_len (ps, &len) == NOTOK) {
	ps_advise (ps, "ps_read_len");
	return NOTOK;
    }
    printf (len == PE_LEN_INDF ? "length indefinite\n" : "length %d\n", len);
    if (form == PE_FORM_PRIM) {
	if (len == 0) {
	    if (class == PE_CLASS_UNIV && id == 0)
		return DONE;
	    else return OK;
	}
	else {
	    char *str = malloc (len);
	    if (ps_read (ps, str, len) == NOTOK) {
		ps_advise (ps, "ps_read");
		return NOTOK;
	    }
	    hexdump (str, len, depth);
	    free (str);
	}
    }
    else {
	if (len == PE_LEN_INDF) {
	    int res;

	    for (;;) {
		res = read_stuff (ps, depth + 1);
		if (res == NOTOK)
		    return res;
		if (res == DONE)
		    break;
	    }
	}
	else {
	    int cc = ps -> ps_byteno + len;

	    for (;;) {
		if (read_stuff (ps, depth + 1) == NOTOK)
		    return NOTOK;
		if (ps -> ps_byteno >= cc)
		    break;
	    }
	}
		    
    }
		
    return OK;
	
}

void hexdump (char *s, int n, int depth)
{
    int		i;
    char	*hexstr = "0123456789ABCDEF";

    printf ("%*s", depth * 2, "");
    for (i = depth * 2; n > 0; n--, i+=2, s++) {
	putchar (hexstr[(*s>>4) & 0xf]);
	putchar (hexstr[(*s)&0xf]);
	if (i > 70) {
	    putchar ('\n');
	    i = depth;
	    printf ("%*s", depth * 2, "");
	}
    }
    putchar ('\n');
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

