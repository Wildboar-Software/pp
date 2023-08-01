/* t-prf.c tests the PerrRecipientFlag */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-prf.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-prf.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-prf.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include "head.h"



/* ---------------------  Begin  Routines  -------------------------------- */



main (argc, argv)
int     argc;
char    *argv[];
{
	int     prf, resp, mta, usr;

	if (argc != 2) {
		printf ("\n\nUsage:  t-prf  value\n\n");
		exit (1);
	}

	prf = atoi (argv[1]);
	prf2mem (prf, &resp, &mta, &usr);
	printf ("\n%d - resp=%d mta=%d usr=%d\n", prf, resp, mta, usr);

	printf ("mem2prf returns %d\n\n", mem2prf(resp, mta, usr));

	return;
}
