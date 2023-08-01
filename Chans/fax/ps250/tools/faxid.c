/* faxid.c: get id from Panasonic */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/tools/RCS/faxid.c,v 6.0 1991/12/18 20:08:25 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/tools/RCS/faxid.c,v 6.0 1991/12/18 20:08:25 jpo Rel $
 *
 * $Log: faxid.c,v $
 * Revision 6.0  1991/12/18  20:08:25  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "chan.h"
#include <stdio.h>
#include "../../faxgeneric.h"
#include "../ps250.h"
#include <varargs.h>

void	adios (), advise ();

char	*myname;

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
	while((opt = getopt(argc, argv, "sdf:")) != EOF)
		switch (opt) {
		    case 'd':
			fax_debug = 1;
			break;
		    case 'f':
			sprintf(psm->devName, "%s", optarg);
			break;
		    case 's':
			psm->softcar = TRUE;
			break;
		    default:
			adios (NULLCP, "Usage: %s [-s] [-d] [-f device]", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	test1(faxctl);
	exit (0);
}

test1(faxctrl)
FaxCtlr	*faxctrl;
{
	PsModem	*psm = PSM(faxctrl);
	Faxcomm pack;
	Stat2	s2;
	int	fd;

	if (faxctrl->open != NULLIFP
	    && (*faxctrl->open) (faxctrl) != OK) {
		fprintf(stderr,
			"fax open failed [%s]\n",
			faxctrl->errBuf);
		exit(1);
	}

	if (fax_sendenq2 (psm->fd) == NOTOK)
		adios ("send enq2", "failed");

	if (fax_getstat2 (psm->fd, &s2) == NOTOK)
		adios ("read packet", "failed");

	if (faxctrl->close !=  NULLIFP)
		(*faxctrl->close) (faxctrl);

	printf ("Fax clock: %s\n", s2.clock);
	printf ("Fax local ID: %s\n", s2.local);
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
