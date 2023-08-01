-- mpp88.py: - test out PEPY


-- @(#) $Header: /xtel/pp/pp-beta/Tools/mpp88/RCS/mpp88.py,v 6.0 1991/12/18 20:31:43 jpo Rel $
--
-- $Log: mpp88.py,v $
-- Revision 6.0  1991/12/18  20:31:43  jpo
-- Release 6.0
--
--
--


MPP DEFINITIONS   ::=

%{
#ifndef lint
static char *rcsid = "$Header: /xtel/pp/pp-beta/Tools/mpp88/RCS/mpp88.py,v 6.0 1991/12/18 20:31:43 jpo Rel $";
#endif  lint

#include <stdio.h>
#include "Trans-types.h"
#include "IOB-types.h"


#define ps_advise(ps, f) \
        advise (NULLCP, "%s: %s", (f), ps_error ((ps) -> ps_errno))

/*    DATA */
int		unpack = 1;
static char	*myname = "mpp88";
static enum	{ ps2mpp, pl2mpp } mode = ps2mpp;
static enum	format { p1, p2, p2hdr, ipn} topfmt = p2;
void		adios ();


/*    MAIN */

/* ARGSUSED */

main (argc, argv, envp)
int     argc;
char  **argv,
      **envp;
{
    register int    status = 0;
    register char  *cp;
    register FILE  *fp;

    if (myname = rindex (argv[0], '/'))
        myname++;
    if (myname == NULL || *myname == NULL)
        myname = argv[0];

    for (argc--, argv++; cp = *argv; argc--, argv++)
        if (*cp == '-') {
            if (strcmp (cp + 1, "ps") == 0) {
                mode = ps2mpp;
                continue;
            }
            if (strcmp (cp + 1, "pl") == 0) {
                mode = pl2mpp;
                continue;
            }
            if (strcmp (cp + 1, "p1") == 0) {
                topfmt = p1;
                continue;
            }
            if (strcmp (cp + 1, "p2") == 0) {
                topfmt = p2;
                continue;
            }
	    if (strcmp (cp + 1, "nounpack") == 0) {
		unpack = 0;
		continue;
	    }
	    if (strcmp (cp + 1, "p2hdr") == 0) {
		topfmt = p2hdr;
		continue;
	    }
	    if (strcmp (cp + 1, "ipn") == 0) {
		topfmt = ipn;
		continue;
	    }
            adios (NULLCP, "usage: %s [ -ps | -pe ] [-p1 | -p2 | -p2hdr | -ipn ] [ files... ]",
                    myname);
        }
        else
            break;

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

    exit (status);              /* NOTREACHED */
}

/*  */

static int  process (file, fp)
register char *file;
register FILE *fp;
{
    enum format curfmt = topfmt;
    register PE     pe;
    register PS     ps;

    if ((ps = ps_alloc (std_open)) == NULLPS) {
        ps_advise (ps, "ps_alloc");
        return 1;
    }
    if (std_setup (ps, fp) == NOTOK) {
        advise (NULLCP, "%s: std_setup loses", file);
        return 1;
    }

    for (;;) {
        switch (mode) {
            case ps2mpp: 
                if ((pe = ps2pe (ps)) == NULLPE)
                    if (ps -> ps_errno) {
                        ps_advise (ps, "ps2pe");
                you_lose: ;
			if (pe != NULLPE)
				pe_free (pe);
			ps_free (ps);
                        return 1;
                    }
                    else {
                done:   ;
                        ps_free (ps);
                        return 0;
                    }
                break;

            case pl2mpp: 
                if ((pe = pl2pe (ps)) == NULLPE)
                    if (ps -> ps_errno) {
                        ps_advise (ps, "pl2pe");
                        goto you_lose;
                    }
                    else
                        goto done;
                break;
        }

        switch (curfmt) {
            case p1:
            default:
		(void) print_Trans_MtsAPDU (pe, 1, 0, NULLCP, NULL);
                break;

            case p2:
                (void) print_IOB_InformationObject (pe, 1, NULLIP, NULLVP, NULLCP);
                break;
	    case p2hdr:
		(void) print_IOB_Heading (pe, 1, NULLIP, NULLVP, NULLCP);
		break;
	    case ipn:
		(void) print_IOB_IPN (pe, 1, NULLIP, NULLVP, NULLCP);
		break;
        }

        pe_free (pe);
    }
}

/*  */

%}

BEGIN

END

%{
/*    DEBUG */

#ifdef  DEBUG
testdebug (pe, s)
register PE pe;
register char *s;
{
    static int  debug = OK;
    char   *cp;
    register PS     ps;

    switch (debug) {
        case NOTOK: 
            break;

        case OK: 
            if ((debug = (cp = getenv ("PEPYDEBUG")) && *cp ? atoi (cp)
                        : NOTOK) == NOTOK)
                break;
            fprintf (stderr, "%s made with %s\n", myname, pepyid);

        default: 
            fprintf (stderr, "%s\n", s);

            if ((ps = ps_alloc (std_open)) == NULLPS)
                return;
            if (std_setup (ps, stderr) != NOTOK)
                (void) pe2pl (ps, pe);
            fprintf (stderr, "--------\n");
            ps_free (ps);
            break;
    }
}
#endif  DEBUG

/*    ERRORS */

/* VARARGS2 */

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

/*  */

/* VARARGS2 */

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

    fprintf (stderr, "%s: ", myname);
    fprintf (stderr, fmt, a, b, c, d, e, f, g, h, i, j);
    if (what)
        (void) fputc (' ', stderr), perror (what);
    else
        (void) fputc ('\n', stderr);
    (void) fflush (stderr);
}

%}
