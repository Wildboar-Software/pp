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
	int	i;

	bzero (buf, sizeof (buf)); 

	if (argc == 1)
		while (gets (buf) != NULL) {
			convert (&buf[0]);	
			bzero (buf, sizeof (buf)); 
		}
	else
		while (i < argc-1)
			convert (argv[++i]);
}



/* ---------------------  Static  Routines  ------------------------------- */


static convert(str)
CHAR8U	*str;
{
	while (*str != 0) {
		printf ("%c(d=%d x=%x o=%o) ", *str, *str, *str, *str);
		str++;
	}

	printf ("\n");
	return;	
}
