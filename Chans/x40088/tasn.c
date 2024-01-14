/* tasn.c : test the incremental asn1 reading */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/tasn.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/tasn.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: tasn.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */



#include "MTA-types.h"
#include "Trans-types.h"
#include "util.h"
#include "q.h"
#include <isode/psap.h>

char	*myname;
void	adios (), advise ();
int	numbytes = 40;
extern IFP asn_procfnx;
char	*remote_site = "remote-site";
int	log_msgtype = MT_UMPDU;
int	ext;

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;

	myname = argv[0];
	sys_init (myname);
	while((opt = getopt(argc, argv, "Ddxn:")) != EOF)
		switch (opt) {
		    case 'n':
			numbytes = atoi (optarg);
			break;
		    case 'x':
			ext = 1;
			break;
		    case 'd':
			pp_log_norm -> ll_events |= (LLOG_TRACE|LLOG_NOTICE);
			break;
		    case 'D':
			pp_log_norm -> ll_events = LLOG_ALL;
			break;
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

static int hdrprint (pe, type)
PE	pe;
int	type;
{
	switch (type) {
	    case MT_UMPDU:
		advise (NULLCP, "Message");
		if (print_MTA_MessageTransferEnvelope (pe, 1, NULLINTP,
						       NULLVP, NULL)
		    == NOTOK) {
			advise (NULLCP,
				"print MessageTransferEnvelope failed: [%s]",
				PY_pepy);
			vunknown (pe);
		}
		break;

	    case MT_PMPDU:
		advise (NULLCP, "Probe");
		if (print_MTA_ProbeTransferEnvelope (pe, 1, NULLINTP,
						       NULLVP, NULL)
		    == NOTOK) {
			advise (NULLCP,
				"print MessageTransferEnvelope failed: [%s]",
				PY_pepy);
			vunknown (pe);
		}
		break;

	    case MT_DMPDU:
		advise (NULLCP, "Delivery Report");
		if (print_Transfer_ReportAPDU (pe, 1, NULLINTP,
					       NULLVP, NULL)
		    == NOTOK) {
			advise (NULLCP,
				"print Transfer_ReportAPDU failed: [%s]",
				PY_pepy);
			vunknown (pe);
		}
		break;
	}
	return OK;
}

static int total_count;

copycount (str, len)
char	*str;
int	len;
{
	total_count += len;
	return OK;
}

process (fp)
FILE	*fp;
{
	char	buf[BUFSIZ];
	int n;
	struct qbuf *qb;
	int	result = NOTOK;
	int	extra;

	total_count = 0;
	asn_init (hdrprint, copycount, ext);
	while ((n = fread (buf, 1, numbytes, fp)) > 0) {
		qb = str2qb (buf, n, 1);
		result = (*asn_procfnx) (qb);
		qb_free (qb);
		switch (result) {
		    default:
		    case NOTOK:
			adios (NULLCP, "Error");
			break;
		    case OK:
		    case DONE:
			break;
		}
	}
	if (result != DONE)
		adios (NULLCP, "Read message - not parsed");
	else	advise (NULLCP, "Message received, %d bytes of content",
			total_count);
	extra = 0;
	while ((n = fread (buf, 1, numbytes, fp)) > 0)
		extra += n;
	if (extra)
		advise (NULLCP, "Message had %d extra trailing bytes", extra);
}


#include <stdarg.h>

static void _advise (va_list ap);

static void adios (char *what, char* fmt, ...)
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

static void  _advise (va_list ap)
{
    char    buffer[BUFSIZ];

    asprintf (buffer, ap);

    (void) fflush (stdout);

    fprintf (stderr, "%s: ", myname);
    (void) fputs (buffer, stderr);
    (void) fputc ('\n', stderr);

    (void) fflush (stderr);
}
