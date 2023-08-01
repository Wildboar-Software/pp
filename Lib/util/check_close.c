/* check_close.c: close an fopen'd fd with lots of checks. */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/check_close.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/check_close.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: check_close.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

#include "util.h"

extern int use_fsync;

int check_close (fp)
FILE *fp;
{

	if (fflush(fp) == EOF || ferror(fp))
		return NOTOK;
#ifdef HAS_FSYNC
	if (use_fsync && fsync(fileno(fp)) == NOTOK)
		return NOTOK;
#endif
	if (fclose (fp) == EOF)
		return NOTOK;
	return OK;
}
