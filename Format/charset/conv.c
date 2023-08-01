/* conv.c - Reads a character set from stdin converts and outputs to stdout */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/charset/RCS/conv.c,v 6.0 1991/12/18 20:19:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/charset/RCS/conv.c,v 6.0 1991/12/18 20:19:34 jpo Rel $
 *
 * $Log: conv.c,v $
 * Revision 6.0  1991/12/18  20:19:34  jpo
 * Release 6.0
 *
 */


#include <isode/general.h>
#include <stdio.h>
#include "charset.h"

extern  int		errno;

#define	SLEN		256	/* max length in octets of images */
static  CHARSET		*in;
static  CHARSET		*out;
static  int		MnemonicsRequired = FALSE;


/* -- input args -- */ 
static	char		arg_Iset [SLEN];
static  char		arg_Oset [SLEN];
static  INT16S		arg_Idef = DEFAULT_ESCAPE;
static  INT16S		arg_Odef = DEFAULT_ESCAPE;



main(argc, argv)
int	   argc;
char	   **argv;
{
	extern int	optind;
	extern char	*optarg;
	int		opt;
	int		len, i;
	CHAR8U		s[SLEN], r[SLEN*4];


	/* -- initialise -- */
	bzero (arg_Iset, SLEN);
	bzero (arg_Oset, SLEN);


	while ((opt = getopt (argc, argv, "I:O:i:o:xm")) != EOF)
		switch (opt) {
		case 'x':
			MnemonicsRequired = FALSE;
			break;
		case 'm':
			MnemonicsRequired = TRUE;
			break;
		case 'i':
			(void) strcpy (arg_Iset, optarg);
			break;
		case 'o':
			(void) strcpy (arg_Oset, optarg);
			break;
		case 'I':
			arg_Idef = (INT16S) atoi (optarg);
			break;
		case 'O':
			arg_Odef = (INT16S) atoi (optarg);
			break;
		default:
			print_usage (argv[0]);
		}



	if (arg_Iset[0] == NULL || arg_Oset[0] == NULL)
		print_usage (argv[0]);


	in  = getchset (arg_Iset, arg_Idef);
	out = getchset (arg_Oset, arg_Odef);

	if (in == NULL || out == NULL) {
		fprintf (stderr, "\n\n*** Error: Unknown Charset/s\n\n");
		exit (1);
	}
	

	/* should output records of unlimited length by
	   outputting one part at a time */


	while ((len = read (0, s, SLEN)) > 0) {

		s[len] = '\0';
		bzero (r, SLEN);

		switch (MnemonicsRequired) {
		case TRUE:
			len = strncnv (out,in,r,s,SLEN*4,TRUE);
			break;
		default:	
			len = strncnv (out,in,r,s,len,FALSE);
			break;	
		}

		for (i = 0; i < len; i++)
			fprintf (stdout, "%c", r[i]);
	}


	fflush (stdout);
	exit (0);
}




static print_usage (prog)
char	*prog;
{
	fprintf (stderr, "\n\n");
	fprintf (stderr, 
		"Usage:  %s  -i charsetin  -o charsetout  [-x | -m]", prog); 
	fprintf (stderr, "\n\n");
	exit (1);
}
