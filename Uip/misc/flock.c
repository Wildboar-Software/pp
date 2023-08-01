/* flock.c: lock a file and execute a command */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/misc/RCS/flock.c,v 6.0 1991/12/18 20:39:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/misc/RCS/flock.c,v 6.0 1991/12/18 20:39:34 jpo Rel $
 *
 * $Log: flock.c,v $
 * Revision 6.0  1991/12/18  20:39:34  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <sys/wait.h>

#ifndef WEXITSTATUS
#define WEXITSTATUS(x) (((union wait *)&(x)) -> w_retcode)
#endif
#ifndef WTERMSIG
#define WTERMSIG(x) (((union wait *)&(x)) -> w_termsig)
#endif

main (argc, argv)
int	argc;
char	**argv;
{
	FILE	*fp;
	extern int errno;
	int	pid, cpid;
#ifdef SYS5
	int     status;
#else
	union wait status;
#endif

	uip_init (argv[0]);
	if (argc < 3) {
		fprintf (stderr, "Usage: %s file commands...\n", argv[0]);
		exit (1);
	}

	if ((fp = flckopen (argv[1], "a")) == NULL) {
		fprintf (stderr, "%s: Can't open file %s: %s\n",
			 argv[0], argv[1], sys_errname (errno));
		exit (1);
	}

	if ((pid = tryfork () ) == NOTOK) {
		fprintf (stderr, "%s: can't fork: %s\n", argv[0],
			 sys_errname (errno));
		exit (1);
	}

	if (pid == 0) {
		fclose (fp);

		execvp (argv[2], &argv[2]);
		_exit (1);
	}

#ifdef SYS5
	while ((cpid = wait( &status)) != pid && cpid != NOTOK)
#else
	while ((cpid = wait( &status.w_status)) != pid && cpid != NOTOK)
#endif
		continue;

	if (cpid == NOTOK)	/* ??? */
		exit (1);

	if (WIFSIGNALED(status))
		exit (1);

	if (WIFEXITED(status))
		exit (WEXITSTATUS(status));

	exit (1);
}
