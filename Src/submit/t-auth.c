/* t-auth.c: test authorisation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/t-auth.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/t-auth.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: t-auth.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "q.h"
#include "adr.h"
#include "retcode.h"

char	*myname;

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;

	myname = argv[0];
	sys_init (myname);

	while((opt = getopt(argc, argv, "")) != EOF)
		switch (opt) {
		    default:
			fprintf (stderr, "Usage: %s\n", myname);
			exit (1);
		}
	argc -= optind;
	argv += optind;

	pp_log_stat -> ll_stat |= LLOGTTY;
	pp_log_stat -> ll_file = "-";

	if (argc > 0) {
		while (argc-- > 1) {
			do_auth (*argv, *(argv+1));
			argv += 2;
		}
	}
	else {
		char buf[BUFSIZ];
		char buf2[BUFSIZ];
		char *cp;

		while (fgets (buf, sizeof buf, stdin) &&
		       fgets (buf2, sizeof buf2, stdin)) {
			if ((cp = index(buf, '\n')) == NULL)
				*cp = '\0';
			if ((cp = index(buf2, '\n')) == NULL)
				*cp = '\0';
			
			do_auth (buf, buf2);
		}
	}
	exit (0);
}


do_auth (str1, str2)
char *str1;
char *str2;
{
	ADDR *sender, *recip;
	RP_Buf rp;
	int retval;
	int type = AD_822_TYPE;

	sender = adr_new (str1, type, 0);

	if (rp_isbad (retval = ad_parse (sender, &rp, CH_USA_PREF))) {
		printf ("Address parsing failed:\n\t%s\n",
			sender -> ad_parse_message);
		adr_free (sender);
		return;
	}

	if (rp_isbad (retval = ad_parse (recip, &rp, CH_USA_PREF))) {
		printf ("Address parsing failed:\n\t%s\n",
			recip -> ad_parse_message);
		adr_free (sender);
		adr_free (recip);
		return;
	}

	auth_start (sender, recip);
	auth_finish (sender, recip);
}


