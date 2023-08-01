/* recrm: recursive remove */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/recrm.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/recrm.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: recrm.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include <isode/usr.dirent.h>
#include <sys/stat.h>

static char currentdir[BUFSIZ];

int rmFiles(entry)
struct dirent *entry;
{
	struct stat statbuf;
	char cbuf[BUFSIZ];

	if (*entry->d_name == '.' &&
	    (strcmp(entry->d_name,".") == 0)
	    || (strcmp(entry->d_name,"..") == 0))
		return 0;
	(void) strcpy (cbuf, currentdir);
	(void) strcat (cbuf, "/");
	(void) strcat (cbuf, entry -> d_name);

	if (stat(cbuf, &statbuf) != OK) {
		PP_SLOG (LLOG_EXCEPTIONS, cbuf, ("Can't stat file"));
		return 0;
	}
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
		return 1;
	else {
		if (unlink(cbuf) == NOTOK) /* ignore rmdir will catch it */
			PP_SLOG(LLOG_EXCEPTIONS, cbuf,
				("can't remove file"));
		else
			PP_TRACE(("Removed '%s'", cbuf));
		return 0;
	}
}

/* recursive rmdir */
recrm (dir)
char    *dir;
{
	struct dirent **namelist, **ix;
	int             noOfSubdirs, i;
	char		buf[BUFSIZ];
	int		retval = OK;
	char 		*cdp;

	(void) strcpy (buf, dir); /* for our use */
	(void) strcpy (currentdir, dir); /* for rmFiles use */
	cdp = buf + strlen(buf);

	noOfSubdirs = _scandir(buf, &namelist, rmFiles, NULLIFP);

	for (i = 0, ix = namelist; i++ < noOfSubdirs && *ix; ix++) {
		*cdp ++ = '/';
		(void) strcpy (cdp, (*ix) -> d_name);
		if (recrm (buf) == NOTOK) {
			retval = NOTOK;
			break;
		}
		
		if (rmdir (buf) == NOTOK) {
			PP_SLOG (LLOG_EXCEPTIONS, buf,
				 ("Can't remove directory"));
			retval = NOTOK;
			break;
		}
		PP_TRACE(("Removed directory '%s'", buf));
		*-- cdp = 0;
	}

	if (namelist) free((char *) namelist);
	return retval;
}

