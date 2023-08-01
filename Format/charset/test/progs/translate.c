/* converts certain characters to t.61-8bit ones */

# ifndef lint
static char Rcsid[] = "@(#)$Header$";
# endif

/*
 * $Header$
 *
 * $Log$
 */



#include <stdio.h>



/* --- *** ---
	#	->	0/0
	+	->	GS 
	<	-> 	10/11
	>	->	11/11
	$	-> 	12/1
	|	->	12/2
	~	->	12/4
	@	->	12/11
--- *** --- */



main()
{
	char	s[BUFSIZ];
	int	c;


	while ((c = getchar()) !=  EOF) {
		switch (c) {
		case '#':
			c = 0;
			break;
		case '+':
			c = 29;
			break;
		case '<':
			c = 171;
			break;
		case '>':
			c = 187;
			break;
		case '$':
			c = 193;
			break;
		case '|':
			c = 194; 
			break;
		case '~':
			c = 196;
			break;
		case '@':
			c = 206;
			break;
		}


		fprintf (stdout, "%c", c);
	}

	fflush (stdout);
	exit (0);
}
