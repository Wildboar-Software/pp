/* strip_addr.c: simpify addresses */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/misc/RCS/strip_addr.c,v 6.0 1991/12/18 20:39:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/misc/RCS/strip_addr.c,v 6.0 1991/12/18 20:39:34 jpo Rel $
 *
 * $Log: strip_addr.c,v $
 * Revision 6.0  1991/12/18  20:39:34  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "ap.h"

static int no_routes = 0;

main (argc, argv)
int	argc;
char	**argv;
{
	int	opt;
	extern int optind;

	while ((opt = getopt (argc, argv, "rp")) != EOF)
		switch (opt) {
		    case 'r':
			no_routes ++;
			break;
		    case 'p':
			ap_use_percent ();
			break;
		    default:
			break;
		}

	argc -= optind; argv += optind;


	if (argc <= 0) {
		char	buf[BUFSIZ];

		while (fgets (buf, sizeof buf, stdin) )
			do_addr (buf);
	}
	else
		while (argc-- > 0)
			do_addr (*argv++);
	exit (0);
}


do_addr (str)
char	*str;
{
	char	*cp;
	AP_ptr local, domain, route, tree;

	tree = ap_s2t (str);

	switch ((int)tree) {
	    case NOTOK:
	    case 0:
		printf ("%s\n", str);
		return;
	}
	(void) ap_t2p (tree, (AP_ptr *)0, (AP_ptr *)0,
		       &local, &domain, &route);
	cp = ap_p2s_nc (NULLAP, NULLAP, local, domain,
			no_routes ? NULLAP : route);
	printf ("%s\n", cp);

	free (cp);
}
