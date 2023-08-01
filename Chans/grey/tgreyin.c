/* tgreyin.c: test */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/grey/RCS/tgreyin.c,v 6.0 1991/12/18 20:10:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/grey/RCS/tgreyin.c,v 6.0 1991/12/18 20:10:19 jpo Rel $
 *
 * $Log: tgreyin.c,v $
 * Revision 6.0  1991/12/18  20:10:19  jpo
 * Release 6.0
 *
 */

#include "util.h"

static char	*myname;
static char *host = "uk.co.xtel", *chan = "gb-janet";


main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;

	myname = argv[0];
	while((opt = getopt(argc, argv, "c:h:")) != EOF)
		switch (opt) {
		    case 'c':
			chan = optarg;
			break;
		    case 'h':
			host = optarg;
			break;
		    default:
			fprintf (stderr, "Usage: %s\n", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		while (argc --) {
			FILE *fp;
			char *ap = *argv++;

			if ((fp = fopen (ap, "r")) == NULL) {
				fprintf (stderr, "Can't read %s\n", ap);
				exit (1);
			}
			doit (fp);
			fclose (fp);
		}
	else
		doit (stdin);
	exit (0);
}

doit (fp)
FILE *fp;
{
	char *errstr;
	char buf[BUFSIZ];
	int n;

	printf ("pp_open(%s,%s)\n", chan, host);
	if (pp_open (chan, host, &errstr) != OK) {
		printf ("FAILED in pp_open: %s\n", errstr);
		return;
	}
	while ((n = fread (buf, 1, sizeof buf, fp)) > 0) {
		printf ("pp_write (0, %*.*s, %d)\n", n, n, buf, n);
		if (pp_write (0, buf, n) != n) {
			printf ("FAILED in pp_write: %s\n", errstr);
			return;
		}
	}

	printf ("pp_close (0)\n");
	if (pp_close (0) != OK)
		printf ("FAILED in pp_close: %s\n", errstr);
}
