/* program/subroutines name: brief description of what this is */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-txt2dr.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-txt2dr.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-txt2dr.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include         "q.h"
#include         "dr.h"

main (ac, av)
int     ac;
char    *av[];
{
	char    buffer[BUFSIZ];
	char    c;
	FILE    *fp;

	fp = stdin;

	if (fgets (&buffer[0], BUFSIZ, fp) == NULL)
		return;

	do_routine (buffer);
}




/* ---------------------  Static  Routines  ------------------------------- */




static do_routine (str)
char    *str;
{
	DRmpdu  drmpdu;
	dr_init (&drmpdu);
	if (txt2dr (&drmpdu, str) == NOTOK)
		printf ("NOTOK\n");
	else
		printf ("OK\n");
}
