/* bin2hex: Converts a binary file to hex */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/bin2hex/RCS/bin2hex.c,v 6.0 1991/12/18 20:28:39 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/bin2hex/RCS/bin2hex.c,v 6.0 1991/12/18 20:28:39 jpo Rel $
 *
 * $Log: bin2hex.c,v $
 * Revision 6.0  1991/12/18  20:28:39  jpo
 * Release 6.0
 *
 */



#include <stdio.h>




/* ---------------------  Begin  Routines  -------------------------------- */


main(argc, argv)
int argc;
char **argv;
{
	if (argc > 1) {
		while (--argc > 0) {
			FILE *fp;

			if ((fp = fopen (*++argv, "r")) == NULL) {
				fprintf (stderr, "Can't open file %s", *argv);
				perror("");
				exit (1);
			}
			bin2hex (fp);
			(void) fclose (fp);
		}
	}
	else
		bin2hex (stdin);

}

bin2hex (fp)
FILE *fp;
{
	int zone, fold;

	fold = 0;

	while ((zone = getc(fp)) != EOF) {

		fold += 3;

		if (fold > 80) {
			putchar ('\n');
			fold = 3;
		}

		printf ("%02x ", zone);	
	}
	if (fold)
		putchar ('\n');
}
