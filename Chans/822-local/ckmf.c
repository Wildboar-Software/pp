/* chkmf.c: check local delivery file stuff */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/822-local/RCS/ckmf.c,v 6.0 1991/12/18 20:04:31 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/822-local/RCS/ckmf.c,v 6.0 1991/12/18 20:04:31 jpo Rel $
 *
 * $Log: ckmf.c,v $
 * Revision 6.0  1991/12/18  20:04:31  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <varargs.h>
#include "expand.h"
#include "retcode.h"
#include <sys/stat.h>
#include <pwd.h>
#include <isode/tailor.h>	/* getlocalhost defn */

#define PROTMODE	(0022)

#define MAXVARS 100
Expand	variables[MAXVARS];
int	nvars;
static int fakeuserid;
static int change_user = 0;

char	*myname;
extern int debug, yydebug;
void	adios (), advise ();

main (argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;
	extern FILE	*yyin;
	int	opt;
	int	n;
	char	msg_size[20];
	struct stat st;

	myname = argv[0];
	while((opt = getopt(argc, argv, "du:i:")) != EOF)
		switch (opt) {
		    case 'd':
			yydebug = debug = 1;
			break;

		    case 'u':
			{
				struct passwd *pwd;

				if ((pwd = getpwnam (optarg)) == NULL) {
					fprintf (stderr,
						 "Unknown user name %s",
						 optarg);
					exit (1);
				}
				fakeuserid = pwd -> pw_uid;
				change_user = 1;
			}
			break;
		    case 'i':
			fakeuserid = atoi(optarg);
			change_user = 1;
			break;

		    default:
			fprintf (stderr, "Usage: %s [-d] mailfilter\n",
				 myname);
			break;
		}
	argc -= optind;
	argv += optind;
	if (change_user)
		do_the_change (fakeuserid);

	if (argc != 1)
		adios (NULLCP, "Usage: %s [-d] mailfilter", myname);
	if ((yyin = fopen (argv[0], "r")) == NULL)
		adios (argv[0], "Can't open file");
	if (fstat (fileno (yyin), &st) == NOTOK)
		adios (argv[0], "Can't fstat file");
	if (st.st_mode & PROTMODE) {
		fprintf (stderr, "File %s has wrong mode (%o)\n",
			 argv[0], st.st_mode & 0777);
		fprintf (stderr,
			 "Should not have group or general write permission\n");
	}
		
	initialise ();

	yyparse();

	if (fstat (0, &st) == -1)
		(void) strcpy (msg_size, "???");
	else	(void) sprintf (msg_size, "%d", st.st_size);

	n = 0;
	variables[n].macro = strdup("size");
	variables[n].expansion = strdup (msg_size);
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("return-path");
	variables[n].expansion = strdup ("sender@remote");
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("mailbox");
	variables[n].expansion = strdup ("themailbox");
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("recipient");
	variables[n].expansion = strdup ("therecipient");
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("userid");
	if (getenv ("USER"))
		variables[n].expansion = strdup (getenv("USER"));
	else {
		(void) sprintf (msg_size, "%d", getuid ());
		variables[n].expansion = strdup (msg_size);
	}
	create_var (&variables[n]);
	n ++;

	(void) sprintf (msg_size, "%d", getgid ());
	variables[n].macro = strdup ("groupid");
	variables[n].expansion = strdup (msg_size);
	create_var (&variables[n]);
	n ++;

	variables[n].macro = strdup ("channelname");
	variables[n].expansion = strdup ("thechannel");
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("hostname");
	variables[n].expansion = strdup (getlocalhost ());
	create_var (&variables[n]);
	n ++;
	

	nvars = parse_hdr (0, variables + 3, MAXVARS - 3) + 3;

	switch (run ()) {
	    case RP_OK:
		printf ("Result: delivered OK\n");
		break;
	    case RP_BAD:
		printf ("Result: delivery failed (permanently)\n");
		break;
	    case RP_AGN:
		printf ("Result: delivered failed (temporarily)\n");
		break;
	    case RP_NOOP:
		printf ("Result: no delivery - other methods will be tried\n");
		break;
	    default:
		printf ("Result: Dunno!\n");
		break;
	}

	exit (0);
	/* NOTREACHED */
}

do_the_change (id)
int	id;
{
	int myuid = getuid ();
	struct passwd *pwd;
	extern char *pplogin;

	pwd = getpwnam (pplogin);
	if (myuid == 0 ||
	    (pwd && myuid == pwd -> pw_uid)) {
#ifdef BSD42
		if (setreuid (id, id) == NOTOK)
#else
		if (setuid (id) == NOTOK)
#endif
			adios ("setreuid", "Can't set ids to %d", id);
	}
	else
		adios (NULLCP, "Not running as root or PP");
}


#ifndef	lint
static void	_advise ();


void	adios (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _advise (ap);

    va_end (ap);

    _exit (1);
}
#else
/* VARARGS */

void	adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif


#ifndef	lint
void	advise (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _advise (ap);

    va_end (ap);
}


static void  _advise (ap)
va_list	ap;
{
    char    buffer[BUFSIZ];

    asprintf (buffer, ap);

    (void) fflush (stdout);

    fprintf (stderr, "%s: ", myname);
    (void) fputs (buffer, stderr);
    (void) fputc ('\n', stderr);

    (void) fflush (stderr);
}
#else
/* VARARGS */

void	advise (what, fmt)
char   *what,
       *fmt;
{
    advise (what, fmt);
}
#endif

putunixfile (str)
char	*str;
{
	char	buf[BUFSIZ];
	expand (buf, str, variables);
	printf ("append to file (sendmail format) '%s'\n", buf);
	return RP_OK;
}

putfile (str)
char	*str;
{
	char	buf[BUFSIZ];
	expand (buf, str, variables);
	printf ("append to file '%s'\n", buf);
	return RP_OK;
}

putpipe (str)
char	*str;
{
	char	buf[BUFSIZ];

	install_vars (variables, nvars, MAXVARS);
	expand (buf, str, variables);
	printf ("pipe into process '%s'\n", buf);
	return RP_OK;
}

printit (s)
char	*s;
{
	fputs (s, stdout);
}
