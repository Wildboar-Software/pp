/* cmdbsrch.c: binary search of tables */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/cmdbsrch.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/cmdbsrch.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: cmdbsrch.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include <isode/cmd_srch.h>

int cmdbsrch (str, cmd, entries)        /* binary version of cmdsrch */
char *str;			/* test string  */
CMD_TABLE cmd[];		/* table of known commands */
int entries;			/* size of cmd table */
{
	register int hi, lo, mid;
	register int diff;
	char *p;

	hi = entries-1;
	lo = 0;

	for (mid=(hi+lo)/2 ; hi >= lo; mid=(hi+lo)/2)
	{
		p = cmd[mid].cmd_key;
		if ((diff = chrcnv[*str] - chrcnv[*p]) == 0 &&
		    (diff = lexequ(str, p))==0)
			return(cmd[mid].cmd_value);

		if (diff < 0)
			hi = mid - 1;
		else
			lo = mid+1;
	}

	return(cmd[entries].cmd_value);
}
