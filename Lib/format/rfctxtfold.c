/* rfctxtfold.c - does simple folding for a string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfctxtfold.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfctxtfold.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfctxtfold.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"



/* -------------  Begin  Routines  ---------------------------------------- */




void rfctxtfold (input, output, linelen)
char    *input;
char    *output;
int     linelen;
{
	int     n       = 0,
		nsav    = 0;
	char    *insav  = NULLCP,
		*outsav = NULLCP;


	*output = NULL;
	if (input == NULLCP)      return;

	while (*input) {
		switch (*input) {

		    case '\t':
			n += 7;

		    case ' ':
			if (n == linelen && *(input+1)) {
				/* -- max len reached & more to come -- */
				input++;
				*output ++ = '\n';
				*output ++ = '\t';
				n = 8;
				break;
			}
			else if (n > linelen && *(input+1)) {
				/* -- max len exceeded & more to come -- */
				if (nsav && insav) {
					/* --- *** ---
					if values have been previously saved
					then go back to the previous spaces
					--- *** --- */
					n = 8;
					input = ++insav;
					*outsav ++ = '\n';
					*outsav ++ = '\t';
					output = outsav;
					nsav = 0;
					insav = outsav = NULLCP;
				}
				else
					goto rfctxtfold_save;
			}
			else {
rfctxtfold_save:;
				/* -- save info and then fall -- */
				nsav = n;
				insav = input;
				outsav = output;
			}

		    default:
			*output ++ = *input ++;
			n ++;
			break;
		}
	}

	if (n > 0 && *(output - 1) != '\n')
		*output ++ = '\n';
	*output = NULL;
}
