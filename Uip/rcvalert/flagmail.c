/* client.c: client for alert program */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/flagmail.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/flagmail.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: flagmail.c,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 */



#include <stdio.h>
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/internet.h>
#include <sys/time.h>
#include <pwd.h>
#include <ctype.h>

#include "data.h"

char	displayline[BUFSIZ];
char	from[BUFSIZ];
char	username[40];
char	hostname[BUFSIZ];
char	homedir[512];
char	*filename = ".alert";
char	*extra = "NEW: ";
char	*format = "%extra(%size) %20from %subject << %80body";
int	width = 80;
int	debug = 0;
int	theport = 0;

char	*myname;

extern char	*index ();

main (argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	int	opt;
	int	size = 0;
	int	use_tty = 0;
	int	use_x	= 0;

	myname = argv[0];
	pp_initialise (myname, 0);
	while((opt = getopt(argc, argv, "de:f:h:p:s:txw:")) != EOF)
		switch (opt) {
		    case 'e':
			extra = optarg;
			break;

		    case 'f':
			format = optarg;
			break;

		    case 'd':
			debug = 1;
			break;

		    case 'h':
			filename = optarg;
			break;

		    case 's':
			size = atoi (optarg);
			break;

		    case 'p':
			theport = atoi (optarg);
			break;

		    case 't':
			use_tty = 1;
			break;

		    case 'w':
			width = atoi (optarg);
			break;

		    case 'x':
			use_x = 1;
			break;

		    default:
			fprintf (stderr, "Usage: %s [options]\n",
				 myname);
			break;
		}
	argc -= optind;
	argv += optind;

	init_stuff ();
	
	parse_msg (displayline, from, format, BUFSIZ-1, size, extra);
	chop(displayline);

	if (debug == 0 && fork () > 0)
		exit (1);
	if (use_tty || (notify_x () == 0 && ! use_x))
	{	if (width > 0 && width < sizeof displayline)
			displayline[width] = 0;
		chop(displayline);
		(void) notify_normal (displayline, username);
	}
	exit (1);
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


notify_x ()
{
	struct data data;
	struct sockaddr_in sin;
	struct hostent *hp;
	int	i;
	int	sd;
	
	if (!findhost(hostname))
		return 0;

	if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror ("socket");
		exit (1);
	}

	bzero ((char *)&sin, sizeof sin);

	sin.sin_family = AF_INET;
	sin.sin_port = htons (theport);

	bzero ((char *)&data, sizeof data);
	data.refid = htonl(getpid ());
	(void) strcpy (data.user, username);
	(void) strncpy (data.data, displayline, sizeof (data.data) - 1);
	data.data[sizeof data.data - 1] = 0;
	(void) strncpy (data.from, from, sizeof(data.from) - 1);
	data.from[sizeof data.from - 1] = 0;

	if ((hp = gethostbyname (hostname)) == NULL) {
		fprintf (stderr, "No such host %s\n", hostname);
		return 0;
	}
	bcopy (hp -> h_addr, (char *)&sin.sin_addr, hp -> h_length);
	for (i = 0; i < N_RETRYS; i++) {
		if (debug) printf("Send to %s.%d\n", hostname, theport);
		if (sendto (sd, (char *)&data, sizeof data, 0,
			    (struct sockaddr *)&sin, sizeof sin) < 0)
			perror ("sendto");

		if (replys (sd))
			return 1;
	}
	if (debug) printf("No reply from %s.%d\n", hostname, theport);
	return 0;
}

replys (sd)
int	sd;
{
	fd_set rfds;
	char	buffer[20];
	struct timeval timer;
	struct sockaddr_in nsin;
	int	len;

	timer.tv_usec = 0;
	timer.tv_sec = 10;
	FD_ZERO (&rfds);
	FD_SET (sd, &rfds);

	if (select (sd + 1, &rfds, NULLFD, NULLFD, &timer) <= 0)
		return 0;

	if (FD_ISSET (sd, &rfds)) {
		len = sizeof nsin;

		if (recvfrom (sd, buffer, 3, 0,
			      (struct sockaddr *)&nsin, &len) < 0)
			perror ("recvfrom");
		return 1;
	}
	if (debug) printf("No reply from %s.%d\n", hostname, theport);
	return 0;
}

findhost (host)
char	*host;
{
	FILE	*fp;
	char	*cp;
	char	buf[BUFSIZ];

	if (*filename == '/' || strncmp (filename, "./", 2) == 0 ||
	    strncmp (filename, "../", 3) == 0)
		(void) strcpy (buf, filename);
	else
		(void) sprintf (buf, "%s/%s", homedir, filename);

	if ((fp = fopen (buf, "r")) == NULL &&
	    (fp = fopen (filename, "r")) == NULL)
		return 0;

	if (fgets (host, BUFSIZ, fp) == NULL) {
		(void) fclose (fp);
		return 0;
	}
	(void) fclose (fp);
	if (cp = index (hostname, '\n'))
		*cp = '\0';
	if (cp = index (hostname, ' ')) {
		*cp++ = '\0';
		if (theport == 0)
			theport = atoi (cp);
	}

	if (theport == 0) {
		fprintf (stderr, "No port!\n");
		return 0;
	}
		
	return 1;
}

chop(string)
char *string;
{	char *end = string + strlen(string) -1;
	while (end != string && (*end == ' ' || *end == '\t')) *end-- = 0;
}
