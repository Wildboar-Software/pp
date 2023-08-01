/* tryfork: attempt to fork a child - makes several attempts */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/tryfork.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/tryfork.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: tryfork.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"

int tryfork()
{
	int     i, pid;

	for(i = 0; i < MAXFORK; sleep(1), i++)
		if((pid = fork()) != -1)
			break;
	return pid;
}
