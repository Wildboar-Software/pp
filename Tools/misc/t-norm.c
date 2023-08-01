/* norm.c: test out normalisation stuff */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-norm.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-norm.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-norm.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "chan.h"
#include <isode/cmd_srch.h>

static CMD_TABLE   chtbl_ad_order[] = {
        "usa",                  CH_USA_ONLY,
        "uk",                   CH_UK_ONLY,
        "usapref",              CH_USA_PREF,
        "ukpref",               CH_UK_PREF,
        0,                      -1
};

char	*myname;

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	int	i;
	int	type = CH_USA_PREF;
	int	all_types = 0;

	myname = argv[0];
	sys_init (myname);

	while((opt = getopt(argc, argv, "at:")) != EOF)
		switch (opt) {
		    case 'a':
			all_types = 1;
			break;
		    case 't':
			i = cmd_srch (optarg, chtbl_ad_order);
			if (i != -1)
				type = i;
			else	fprintf (stderr, "Bad type %s\n", optarg);
			break;
		    default:
			fprintf (stderr, "Usage: %s [-a] [-t type] domain", myname);
			break;
		}
	argc -= optind;
	argv += optind;

	for (i = 0; i < argc; i++) {
		if (all_types) {
			CMD_TABLE *cp;

			for (cp = chtbl_ad_order; cp -> cmd_key; cp ++)
				do_domain (argv[i], cp -> cmd_value);
		}
		else
			do_domain (argv[i], type);
	}
	exit (0);
}

do_domain (str, type)
char	*str;
int	type;
{
	char	chanbuf[BUFSIZ];
	char	normalised[BUFSIZ];
	char	*subdom = NULLCP;
	printf ("Domain %s (type %s) -> ",
		str, rcmd_srch (type, chtbl_ad_order));
	if (tb_getdomain (str, chanbuf, normalised, type, &subdom) == NOTOK)
		printf ("failed\n");
	else {
		printf ("normalised=%s chan=%s\n", normalised, chanbuf);
		if (subdom != NULLCP) {
			printf ("loacl subdomain=%s\n", subdom);
			free(subdom);
		}
	}
}
