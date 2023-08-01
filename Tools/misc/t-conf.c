/* t-conf.c: tests the Lib/pp/conf.c variables */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-conf.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-conf.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-conf.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include <stdio.h>

extern char     *pptailor,
		*cmddfldir,
		*logdfldir,
		*quedfldir,
		*tbldfldir,
		*niftpquedir,
		*niftpcpf;


main ()
{
	sys_init ("testconf");
	printf ("tailor    = %s\n", pptailor);
	printf ("cmddfldir = %s\n", cmddfldir);
	printf ("logdfldir = %s\n", logdfldir);
	printf ("quedfldir = %s\n", quedfldir);
	printf ("tbldfldir = %s\n", tbldfldir);
	printf ("niftpquedir = %s\n", niftpquedir);
	printf ("niftpcpf = %s\n", niftpcpf);
	return;
}
