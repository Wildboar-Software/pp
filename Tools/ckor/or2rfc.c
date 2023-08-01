/* or2rfc.c: Parses an RFC address using the tables or, or2rfc and rfc2or */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ckor/RCS/or2rfc.c,v 6.0 1991/12/18 20:29:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ckor/RCS/or2rfc.c,v 6.0 1991/12/18 20:29:26 jpo Rel $
 *
 * $Log: or2rfc.c,v $
 * Revision 6.0  1991/12/18  20:29:26  jpo
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
	char            x400buf [BUFSIZ];
	OR_ptr          or = NULLOR;

	or_error[0] = x400buf[0] = rfcbuf[0] = '\0';

	if ((or = or_std2or (str)) == NULLOR)
		goto error;

	if (or_or2rfc (or, rfcbuf) == NOTOK)
		goto error;

	if (isstr (or_error))
		goto error;

	or_free (or);
	or = NULLOR;

	if (or_rfc2or (rfcbuf, &or) == NOTOK)
		goto error;

	if (isstr (or_error))
		goto error;

	or_or2std (or, x400buf, FALSE);

	printf ("\n%s -> (rfc822) %s\n%-*s -> (x400)   %s\n",
		str, rfcbuf, strlen(str), "", x400buf);

	goto finish;


error:  ;
	printf ("\nAddress parsing failed \nReason : %s\n", or_error);
	if (rfcbuf[0] != '\0')
		printf ("%s -> (rfc822) %s\n", str, rfcbuf);
	if (x400buf[0] != '\0')
		printf ("%s -> (x400) %s\n", str, x400buf);

finish:   ;
	printf ("\n");
	fflush (stdout);
	if (or)
		or_free (or);
	return;
}
