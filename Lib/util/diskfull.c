/* diskfull.c: check available space meets requirements */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/diskfull.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/diskfull.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: diskfull.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

#include "util.h"

#if defined(ultrix) && defined(mips)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>

#define FSTATFS fstatfs
#define STAT_FS	struct fs_data

/*
** Function Name : fstatfs
** Description   : Get file system statistics.  fstatfs returns
**		   information about the filesystem on which an open
**		   file resides.  See statfs(3) for more information.
** Arguments     : fd     - open file descriptor
**                 buffer - ptr to memory to store struct fs_data in
** Returns       : Upon successful completion, a value of 0 is returned.
**		   Otherwise, -1 is returned and the global variable errno
**		   is set to indicate the error.
** Author        : David Bonnell (bonnell@coral.cs.jcu.edu.au)
**               : Department of Computer Science, James Cook University
**		 : 7 October, 1991
*/
int
fstatfs(int fd, struct fs_data *buffer)
{
  struct stat   	buf;
  struct fs_data	mount[NMOUNT];
  int			i, nmount;

  if (fstat(fd, &buf) < 0) return -1;		/* get fs dev */

  i=0;
  nmount = getmountent(&i, mount, NMOUNT);	/* get sys mount table */
  if (nmount < 0) return -1;

  for (i=0; i<nmount; i++) {			/* search for req dev */
    if (mount[i].fd_req.dev == buf.st_dev) {
      *buffer = mount[i];
      return 0;
    }
  }

  errno = ENOENT;
  return -1;
}

#else
#if defined(BSD42) || defined(masscomp) || defined(HPUX) || defined(_AIX)
	/*BSD type of call */
#define CHECKDISK
#include <sys/vfs.h>
#define STAT_FS	struct statfs
#define FSTATFS fstatfs
#else
#ifdef SYS5			/* sys5 is different (of course!) */
#define CHECKDISK
#include <sys/statvfs.h>
#define STAT_FS struct statvfs
#define FSTATFS fstatvfs
#endif
	/* can't be handled */
#endif
#endif


fdiskfull (fd, blocks, percent)
int	fd;
int	blocks;
int	percent;
{
#ifdef CHECKDISK
	STAT_FS fs;

	if (blocks <= 0 && percent <= 0)
		return OK;

	if (FSTATFS (fd, &fs) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, "failed", ("%s on %d", FSTATFS, fd));
		return OK;	/* and hope... */
	}
#if	defined(ultrix) && defined(mips)
	if (blocks != NOTOK && fs.fd_req.bfree < blocks)
#else
	if (blocks != NOTOK && fs.f_bavail < blocks)
#endif
		return NOTOK;
	if (percent != NOTOK) {
		int tblks;	/* total blocks - reserved */

#if	defined(ultrix) && defined(mips)
		tblks = fs.fd_req.btot - (fs.fd_req.bfree - fs.fd_req.bfreen);
		if ((fs.fd_req.bfree * 100 / tblks) < percent)
#else
		tblks = fs.f_blocks - (fs.f_bfree - fs.f_bavail);
		if ((fs.f_bavail * 100 / tblks) < percent)
#endif
			return NOTOK;
	}
#endif
	return OK;		/* if we don't have the facility - lie! */
}
