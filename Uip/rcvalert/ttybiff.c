/* ttybiff.c: tty alert program using the UDP syntax stuff */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/ttybiff.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/ttybiff.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: ttybiff.c,v $
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

char	username[40];
char	hostname[128];
char	homedir[512];
char	*filename = ".alert";
short	port = 0;
char	*extra = "NEW: ";
char	*format = "%extra(%size) %20from %subject << %50body";
int	width = 80;
int	debug = 0;
int	stdoutonly = 0;

char	*myname;

extern char	*index ();

main (argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	int	sd;

	myname = argv[0];
	while((opt = getopt(argc, argv, "f:p:u:s")) != EOF)
		switch (opt) {
		    case 's':
			stdoutonly = 1;
			break;
		    case 'f':
			filename = optarg;
			break;
		    case 'p':
			port = atoi(optarg);
			break;
		    case 'u':
			(void) strcpy (username, optarg);
			break;
		    default:
			fprintf (stderr,
				 "Usage: %s [-f file] [-p port] [-u user]",
				 myname);
			break;
		}
	argc -= optind;
	argv += optind;

	init_stuff ();

	sd = udp_start (port, filename, homedir);

	for (;;) {
		fd_set rfds;
		char	buffer[BUFSIZ];
		char	from[BUFSIZ];
		int	n;

		FD_ZERO (&rfds);
		FD_SET (sd, &rfds);
		if(select (sd + 1, &rfds, NULLFD, NULLFD,
			   (struct timeval *)0) <= 0)
			continue;

		n = sizeof (buffer) - 1;
		if (FD_ISSET (sd, &rfds) &&
		    getdata (sd, buffer, &n, username, from)) {
			buffer[n] = 0;
			if (stdoutonly ||
			    (notify_normal (buffer, username) == NOTOK &&
			    isatty (1)))
				printf ("\n\r%s%c\n\r",
					buffer, '\007');
		}
	}
}

init_stuff ()
{
	struct passwd *pwd;
	char	*cp, *getenv ();

	if ((pwd = getpwuid (getuid())) == NULL) {
		fprintf (stderr, "No user name\n");
		exit(-1);
	}
	if (username[0] == NULL)
		(void) strcpy (username, pwd -> pw_name);

	if ((cp = getenv ("HOME")) != NULL && *cp)
		(void) strcpy (homedir, cp);
	else
		(void) strcpy (homedir, pwd -> pw_dir);
}
