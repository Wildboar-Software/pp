/*  nsapexpand.c: expand macro forms of NSAP to full form */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/nsapexpand/RCS/nsapexpand.c,v 6.0 1991/12/18 20:32:03 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/nsapexpand/RCS/nsapexpand.c,v 6.0 1991/12/18 20:32:03 jpo Rel $
 *
 * $Log: nsapexpand.c,v $
 * Revision 6.0  1991/12/18  20:32:03  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/isoaddrs.h>

static char	*myname;
int	abortonerror = 0;

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;

	myname = argv[0];
	sys_init (*argv);

	while((opt = getopt(argc, argv, "a")) != EOF)
		switch (opt) {
		    case 'a':
			abortonerror = TRUE;
			break;

		    default:
			fprintf (stderr, "Usage: %s [-a] [files]\n", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	if (argc <= 0)
		process (stdin, "standard input");
	else {
		while (argc > 0) {
			FILE	*fp = fopen (*argv, "r");
			if (fp == NULL) {
				fprintf (stderr, "%s: Can't open file %s\n",
					 myname, *argv);
				exit(1);
			}
			process (fp, *argv);
			(void) fclose (fp);

			argv ++;
			argc --;
		}
	}
	exit (0);
}

process (fp, name)
FILE	*fp;
char	*name;
{
	char	key[BUFSIZ], value[BUFSIZ];
	struct PSAPaddr *pa;
	char	*cp;

	while (tab_fetch (fp, key, value) == OK) {
		if (lexequ (key, "default") == 0) {
			printf ("%s:%s\n", key, value);
			continue;
		}
		if ((pa = str2paddr (key)) == NULLPA) {
			PP_OPER (NULLCP, ("Bad psap address <%s> in %s",
					  key, name));
			fprintf (stderr, "Bad psap address <%s> in %s\n",
					  key, name);
			if (abortonerror)
				exit (1);
			continue;
		}
		cp = _paddr2str (pa, NULLNA, -1);
		if (cp == NULLCP) {
			PP_OPER (NULLCP,
				 ("Can't convert psap <%s> to full form",
				  key));
			fprintf (stderr,
				 "Can't convert psap <%s> to full form\n",
				 key);
			if (abortonerror)
				exit (1);
			continue;
		}
		printf ("%s:%s\n", cp, value);
	}
}

		
