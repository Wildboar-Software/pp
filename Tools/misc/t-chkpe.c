/* t-chkpe.c: Checks the pe2or and or2pe routines using an RFC address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-chkpe.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-chkpe.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-chkpe.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "or.h"

extern char             or_error[];




/* ---------------------  Begin  Routines  -------------------------------- */



main (argc, argv)
int     argc;
char    *argv[];
{
	char    buf[BUFSIZ];
	int     i = 1;

	sys_init (argv[0]);

	if (or_init() == NOTOK ) {
		printf ("or_init() failed\n");
		exit (1);
	}

	if (argc == 1)
		while (gets (buf) != NULL)
			parse_address (buf);
	else
		while (i < argc)
			parse_address (argv[i++]);
}


/* ---------------------  Static  Routines  ------------------------------- */



static int parse_address (str)
char    *str;
{
	char            rfcbuf [BUFSIZ];
	char            x400buf_orig [BUFSIZ];
	char            x400buf_after [BUFSIZ];
	OR_ptr          or = NULLOR;
	PE              pe = NULLPE;

	x400buf_orig[0] = x400buf_after[0] = rfcbuf[0] = '\0';

	/* --- Perform the x400 addressing --- */
	if ((or_rfc2or (str, &or) != OK) || (or == NULLOR))
		goto error;
	or_or2std (or, x400buf_orig, FALSE);


	/* --- Do the pe routines -- */
	pe = ora2pe (or);
	if (pe == NULLPE)
		goto error;

	or_free (or);
	or = pe2ora (pe);
	pe_free (pe);

	if (or == NULLOR)
		goto error;

	/* --- Perform the rfc822 address parsing --- */
	or_or2std (or, x400buf_after, FALSE);
	if (or_or2rfc (or, rfcbuf) == NOTOK)
		goto error;

	printf ("PE check Succeded\n");
	printf ("%s -> (x400 orig)  %s\n%-*s -> (x400 after) %s\n%-*s -> (rfc822)     %s\n",
		str, x400buf_orig, strlen(str), "", x400buf_after,
		strlen(str), "", rfcbuf);

	goto finish;


error:  ;
	printf ("PE check Failed \nReason : %s\n", or_error);
	if (x400buf_orig[0] != '\0')
		printf ("%s -> (x400 orig) %s\n", str, x400buf_orig);
	if (x400buf_after[0] != '\0')
		printf ("%s -> (x400 after) %s\n", str, x400buf_after);
	if (rfcbuf[0] != '\0')
		printf ("%s -> (rfc822) %s\n", str, rfcbuf);


finish: ;
	fflush (stdout);
	if (or)         or_free (or);
	if (pe)         pe_free (pe);
	return;
}


int advise ()
{
	return;
}
