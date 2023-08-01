/* ut_fn.c: FullName support routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_fn.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_fn.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_fn.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"mta.h"

FullName	*fn_new (addr, dn)
char	*addr;
char	*dn;
{
	FullName	*fn;

	fn = (FullName *) smalloc (sizeof *fn);

	fn -> fn_addr = addr ? strdup (addr) : NULLCP;
	fn -> fn_dn = dn ? strdup (dn) : NULLCP;
	return fn;
}

void	fn_free (fn)
FullName	*fn;
{
	if (fn == NULL)
		return;

	if (fn -> fn_addr)
		free (fn -> fn_addr);
	if (fn -> fn_dn)
		free (fn -> fn_dn);
	free ((char *)fn);
}

FullName *fn_dup (fn)
FullName	*fn;
{
	return fn_new (fn -> fn_addr, fn -> fn_dn);
}
		      
