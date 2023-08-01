/* ttysbr.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/ttysbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/ttysbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: ttysbr.c,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 */



#include <config.h>
#include <isode/manifest.h>
#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef UTMP_FILE
#define UTMP_FILE	"/etc/utmp"
#endif
#ifdef XOS_2
struct utmp *getutent();
#endif

#define OKTONOTIFY	020	/* group write perm */

#ifdef nonuser
#define NONUSER(x)	nonuser(x)
#else
#ifdef XOS_2
#define NONUSER(x)    ((x)->ut_user[0] == 0)
#else
#define NONUSER(x)	((x).ut_name[0] == 0)
#endif
#endif

int notify_normal (str, username)
char	*str;
char	*username;
{
	FILE	*fp;
	int	done = NOTOK;
#ifdef XOS_2
	struct utmp *utmp;
#else
	struct utmp utmp;
#endif

#ifdef XOS_2
	while ((utmp = getutent ()) != (struct utmp *)NULL) {
		if (NONUSER (utmp))
			continue;
		if (strncmp (utmp->ut_user, username,
			     sizeof (utmp->ut_user)) == 0)
			if (notify_user (utmp, str) == OK)
				done = OK;
	}
#else
	if ((fp = fopen (UTMP_FILE, "r")) == NULL)
		return NOTOK;

	while ((int)fread ((char *)&utmp, sizeof utmp, 1, fp) > 0) {
		if (NONUSER (utmp))
			continue;
		if (strncmp (utmp.ut_name, username,
			     sizeof (utmp.ut_name)) == 0)
			if (notify_user (&utmp, str) == OK)
				done = OK;
	}
	(void) fclose (fp);
#endif
	return done;
}

notify_user (ut, str)
struct utmp *ut;
char	*str;
{
	char device[BUFSIZ];
	struct stat st;
	int	pid;
	int	fd;

	(void) sprintf (device, "/dev/%.*s", sizeof (ut -> ut_line),
			ut -> ut_line);

	if (stat (device, &st) == NOTOK)
		return NOTOK;
	if ((st.st_mode & OKTONOTIFY) == 0)
		return NOTOK;

	if ((pid = fork ()) == NOTOK)
	    return NOTOK;

	if (pid)
		return OK;

	if ((fd = open (device, O_WRONLY, 0)) == NOTOK)
		_exit(1);
	(void) write (fd, "\r\n\007", 3);
	(void) write (fd, str, strlen (str));
	(void) write (fd, "\r\n\007", 3);
	(void) close (fd);
	_exit (0);
	return NOTOK;
}
