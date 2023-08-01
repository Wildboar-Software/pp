/* lock.c: file locking subroutines  */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/lock.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/lock.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: lock.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "sys.file.h"
#ifdef SUNOS4 
#include <fcntl.h>
#endif
#if defined(sun) || defined(SYS5)
#include <unistd.h>
#endif
#include <sys/stat.h>


static int lk_file ();
static int lk_unlck ();

static struct {
	char *name;
	int mode;
} locks [FD_SETSIZE];

extern char	*lockdir;
extern int	lockstyle;
extern int	lock_break_time;	/* 30 minutes grace ? */
extern time_t time ();

FILE	*flckopen (name, mode)
char	*name;
char	*mode;
{
	FILE	*fp;

	if ((fp = fopen (name, mode)) == NULL)
		return NULL;

	locks[fileno(fp)].mode = (*mode == 'r' ? O_RDONLY : O_WRONLY);
	switch (lockstyle) {
	    case LOCK_FLOCK:
#ifdef LOCK_EX
		if (flock (fileno (fp), LOCK_EX | LOCK_NB) == NOTOK) {
			(void) fclose (fp);
			return NULL;
		}
		return fp;
#else
		break;
#endif
		
	    case LOCK_FCNTL:
#ifdef F_RDLCK
		{
			struct flock l;

			l.l_len = 0;
			l.l_start = 0;
			l.l_whence = 0;

			if (*mode == 'r' && mode[1] != '+')
				l.l_type = F_RDLCK;
			else
				l.l_type = F_WRLCK;
				
			if (fcntl (fileno(fp), F_SETLK, &l) < 0) {
				(void) fclose (fp);
				return NULL;
			}
		}
		return fp;
#else
		break;
#endif
	    case LOCK_LOCKF:
#ifdef F_TLOCK
		if (lockf (fileno (fp), F_TLOCK, 0L) == NOTOK) {
			(void) fclose (fp);
			return NULL;
		}
		return fp;
#else
		break;
#endif	
	    default:
	    case LOCK_FILE:
		if (lk_file (name, fileno (fp)) == NOTOK) {
			(void) fclose (fp);
			return NULL;
		}
		break;
	}
	PP_OPER (NULLCP, ("Specified styple of locking (%d) not supported",
			  lockstyle));
	return NULL;
}

static int lk_file (name, n)
char	*name;
int	n;
{
	char	locktmp[MAXPATHLENGTH];
	char	lockname[MAXPATHLENGTH];
	struct stat st;
	int	fd;
	time_t	now;

	if (n < 0 || n >= FD_SETSIZE)
		return NOTOK;

	if (fstat (n, &st) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, name, ("Can't fstat %d file", n));
		return NOTOK;
	}

	(void) sprintf (locktmp, "%s/LCK%05.5o+%o.t", lockdir,
			st.st_dev, st.st_ino);
	(void) sprintf (lockname, "%s/LCK%05.5o+%o",
			lockdir, st.st_dev, st.st_ino);

	if ((fd = open (locktmp, O_RDWR|O_CREAT, 0)) < 0) {
		PP_SLOG (LLOG_EXCEPTIONS, locktmp,
			 ("Can't create lock temp file"));
		return NOTOK;
	}
	(void) close (fd);

	if (stat (lockname, &st) != NOTOK) {
		(void) time (&now);
		if (st.st_mtime + lock_break_time < now) {
			PP_NOTICE (("Forcing lock for %s", lockname));
			(void) unlink (lockname);
		}
	}
	if (link (locktmp, lockname) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, lockname,
			 ("Can't lock by linking %s to ", locktmp));
		(void) unlink (locktmp);
		return NOTOK;
	}
	(void) unlink (locktmp);
	locks[fd].name = strdup (lockname);
	return OK;
}

int	flckclose (fp)
FILE	*fp;
{
	switch (lockstyle) {
	    case LOCK_FCNTL:
	    case LOCK_FLOCK:
		if (locks[fileno(fp)].mode == O_WRONLY)
			return check_close (fp);
		else return fclose(fp);

	    default:
	    case LOCK_FILE:
		return lk_unlck (fp);
	}
}

static int lk_unlck (fp)
FILE	*fp;
{
	int	n = fileno (fp);

	if (n >= 0 && n < FD_SETSIZE && locks[n].name) {
		(void) unlink (locks[n].name);
		free (locks[n].name);
		locks[n].name = NULLCP;
		return check_close (fp);
	}
	(void) fclose (fp);
	return NOTOK;
}
