/* faxenq1.c: send an enquire 1 to Panasonic */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/tools/RCS/faxenq1.c,v 6.0 1991/12/18 20:08:25 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/tools/RCS/faxenq1.c,v 6.0 1991/12/18 20:08:25 jpo Rel $
 *
 * $Log: faxenq1.c,v $
 * Revision 6.0  1991/12/18  20:08:25  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "chan.h"
#include <stdio.h>
#include "../../faxgeneric.h"
#include "../ps250.h"
#include <stdarg.h>

void	adios (char *, char *, ...), advise (char *, char *, ...);

char	*myname;
static char *localid = 0;

extern FaxCtlr	*init_faxctrlr();
extern int	fax_debug;
CHAN	*mychan = NULLCHAN;
int	table_verified = FALSE;

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	FaxCtlr	*faxctl = init_faxctrlr();
	PsModem	*psm = PSM(faxctl);

	myname = argv[0];
	while((opt = getopt(argc, argv, "sdf:l:")) != EOF)
		switch (opt) {
		    case 'd':
			fax_debug = 1;
			break;
		    case 's':
			psm->softcar = TRUE;
			break;
		    case 'f':
			sprintf(psm->devName, "%s", optarg);
			break;
		    default:
			adios (NULLCP, "Usage: %s [-s] [-d] [-f device]", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	enq1 (faxctl);
	exit (0);
}

enq1 (faxctl)
FaxCtlr	*faxctl;
{
	Stat1	stat1;
	Stat2	s2;
	int	fd;
	time_t	now;
	PsModem	*psm = PSM(faxctl);

	if (faxctl->open != NULLIFP
	    && (*faxctl->open) (faxctl) != OK) {
		fprintf(stderr,
			"fax open failed [%s]\n",
			faxctl->errBuf);
		exit(1);
	}

	if (fax_sendenq1 (psm->fd) == NOTOK)
		adios (NULLCP, "enq1 failed");

	if (fax_getstat1 (psm->fd, &stat1) == NOTOK)
		adios ("fax_getstat1", "failed");
	fax_pstat1 (&stat1);

	if (fax_sendenq2 (psm->fd) == NOTOK)
		adios ("send enq2", "failed");

	if (fax_getstat2 (psm->fd, &s2) == NOTOK)
		adios ("read packet", "failed");

	if (faxctl->close !=  NULLIFP)
		(*faxctl->close) (faxctl);

	printf ("Fax clock: %s\n", s2.clock);
	printf ("Fax local ID:	%s\n", s2.local);
}
		

#ifndef	lint
static void	_advise ();


void	adios (char *what, char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);

    _advise (what, fmt, ap);

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
void	advise (char *what, char *fmt, ...)
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
#else
/* VARARGS */

void	advise (what, fmt)
char   *what,
       *fmt;
{
    advise (what, fmt);
}
#endif
