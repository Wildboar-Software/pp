/* bit2ch.c - converts one 8 bit string at a time into a one byte character */

# ifndef lint
static char Rcsid[] = "@(#)$Header$";
# endif

/*
 * $Header$
 *
 * $Log$
 */


#include <stdio.h>
#include "charset.h"

#define BITS	8


main (argc, argv)
int	argc;
CHAR8U	*argv[];
{
	CHAR8U	buf[BUFSIZ];	
	int	i = 0;

	bzero (buf, sizeof (buf)); 

	if (argc == 1)
		while (gets (buf) != NULL) {
			convert (&buf[0]);	
			bzero (buf, sizeof (buf)); 
		}
	else
		while (i < argc-1)
			convert (argv[++i]);

	printf ("\n");
}



/* ---------------------  Static  Routines  ------------------------------- */


static convert(str)
CHAR8U	*str;
{
	int	i;
	CHAR8U	c=0;

	for (i=BITS-1; i >= 0; i--) {
		if (str[i] == '1')
			c |= 1 << (BITS - (i+1));
		else if (str[i] == '0')
			continue;
		else {
			printf ("**** Error this is not a bitstring\n");
			return;
		}
	}

	printf ("%d - %c", c, c);
}
