/* ttyalert.c: simple tty alert program */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/ttyalert.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/ttyalert.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: ttyalert.c,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 */



#include <stdio.h>
#include <isode/manifest.h>
#include "sys.file.h"
#include <sys/stat.h>
#include <utmp.h>
#include <sys/time.h>
#include <pwd.h>
#include <ctype.h>

#include "data.h"

#ifndef UTMP_FILE
#define UTMP_FILE	"/etc/utmp"
#endif

#define OKTONOTIFY	020	/* group write perm */

char	displayline[BUFSIZ];
char	from[BUFSIZ];
char	username[40];
char	homedir[512];
char	*extra = "NEW: ";
char	*format = "%extra(%size) %20from %subject << %50body";
int	width = 80;
int	debug = 0;

char	*myname;

extern char	*index ();

main (argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	int size = 0;

	myname = argv[0];
	while((opt = getopt(argc, argv, "de:f:s:w:")) != EOF)
		switch (opt) {
		    case 'd':
			debug = 1;
			break;

		    case 'e':
			extra = optarg;
			break;

		    case 's':
			size = atoi (optarg);
			break;

		    case 'f':
			format = optarg;
			break;

		    case 'w':
			width = atoi (optarg);
			break;

		    default:
			fprintf (stderr, "Usage: %s [-f string] hosts..",
				 myname);
			break;
		}
	argc -= optind;
	argv += optind;

	init_stuff ();

	parse_msg (displayline, from, format, width, size, extra);

	if (debug == 0 && fork () > 0)
		exit (0);
	notify_normal (displayline, username);
	exit (0);
}

init_stuff ()
{
	struct passwd *pwd;
	char	*cp, *getenv ();

	if ((pwd = getpwuid (getuid())) == NULL) {
		fprintf (stderr, "No user name\n");
		exit(-1);
	}

	(void) strcpy (username, pwd -> pw_name);
	if ((cp = getenv ("HOME")) != NULL && *cp)
		(void) strcpy (homedir, cp);
	else
		(void) strcpy (homedir, pwd -> pw_dir);
}
