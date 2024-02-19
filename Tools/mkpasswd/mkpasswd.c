/* mkpasswd.c: generate encryped/encoed passwd */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/mkpasswd/RCS/mkpasswd.c,v 6.0 1991/12/18 20:31:22 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/mkpasswd/RCS/mkpasswd.c,v 6.0 1991/12/18 20:31:22 jpo Rel $
 *
 * $Log: mkpasswd.c,v $
 * Revision 6.0  1991/12/18  20:31:22  jpo
 * Release 6.0
 *
 */



#include "util.h"

#ifdef BSD42
#define RAND random
#define SRAND srandom
#else
#define RAND rand
#define SRAND srand
#endif

char	*myname;

static void findpass ();

void main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	char	*pass;

	myname = argv[0];
	while((opt = getopt(argc, argv, "")) != EOF)
		switch (opt) {
		    default:
			fprintf (stderr, "Usage: %s", myname);
			break;
		}
	argc -= optind;
	argv += optind;


	findpass ();
	
}

char saltkeys[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";
extern char *crypt ();

static void findpass ()
{
	char *pass, *epass;
	char saltc[3];
	time_t now;

	(void) time (&now);
	SRAND (now);

	saltc[0] = saltkeys[RAND () % ((sizeof saltkeys) - 1)];
	saltc[1] = saltkeys[RAND () % ((sizeof saltkeys) - 1)];
	saltc[2] = 0;

	pass = getpassword ("Password: ");
	epass = crypt (pass, saltc);
	
	bzero (pass, strlen (pass));
	
	printf ("%s\n", epass);
}
