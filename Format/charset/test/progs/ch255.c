/* t-255.c - generates 8 bit characters having values 0-255 */

# ifndef lint
static char Rcsid[] = "@(#)$Header$";
# endif

/*
 * $Header$
 *
 * $Log$
 */

#include "charset.h"


main ()
{
	CHAR8U	c=0;
	int	col = 0, row = 0;
	int	val; 

	printf ("\n");

	for (row = 0; row < 16; row++) {
		/* printf ("%.2d  ", row); */

		for (col = 0; col < 16; col++) {
			val = (col << 4) | row;
			if (val == 0) val = ' ';
			printf ("%.3d=%c\n", val, val);
		}
		printf ("\n");
	}
}
