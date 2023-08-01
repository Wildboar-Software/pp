/* hex2bin: Converts a hex file to binary */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/hex2bin/RCS/hex2bin.c,v 6.0 1991/12/18 20:30:25 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/hex2bin/RCS/hex2bin.c,v 6.0 1991/12/18 20:30:25 jpo Rel $
 *
 * $Log: hex2bin.c,v $
 * Revision 6.0  1991/12/18  20:30:25  jpo
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
			hex2bin(fp);
			(void) fclose (fp);
		}
	}
	else
		hex2bin (stdin);


}

hex2bin (fp)
FILE *fp;
{
	int zone, first_zone, octet;

	first_zone = 1;
	octet = 0;
	while ((zone = getc(fp)) != EOF)
	{
		switch (zone)
		{
		    case '0' :
		    case '1' :
		    case '2' :
		    case '3' :
		    case '4' :
		    case '5' :
		    case '6' :
		    case '7' :
		    case '8' :
		    case '9' : zone -= '0';
			break;
		    case 'a' :
		    case 'b' :
		    case 'c' :
		    case 'd' :
		    case 'e' :
		    case 'f' : zone = zone - 'a' + 10;
			break;

		    case 'A' :
		    case 'B' :
		    case 'C' :
		    case 'D' :
		    case 'E' :
		    case 'F' : zone = zone - 'A' + 10;
			break;
		    default:
			continue;
		}
		if (first_zone)
		{
			octet = zone * 16;
			first_zone = 0;
		}
		else
		{
			octet += zone;
			putchar(octet);
			first_zone = 1;
		}
	}
	if (!first_zone)
		putchar(octet);
}
