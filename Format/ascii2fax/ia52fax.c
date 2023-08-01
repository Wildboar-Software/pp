/* ia52fax: convert ia5 to fax */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/ia52fax.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/ia52fax.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: ia52fax.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */


#include <stdio.h>
#include "util.h"
#include "IOB-types.h"
#include <isode/cmd_srch.h>
#include "table.h"
#include "fonts.h"
#include "pg_sizes.h"

#define OPT_TABLE	1
#define OPT_FONT	2

CMD_TABLE	tbl_options [] = {
	"-table",	OPT_TABLE,
	"-font",	OPT_FONT,
	0,		-1
	};

char	*display_str;
char	*myname;
Table	*table = NULLTBL;
char	*font = NULLCP;

extern BitMap	new_bitmap();
extern PPFontPtr	file2font();

main(argc, argv)
int	argc;
char	**argv;
{
	PPFontPtr	ppfont;
	int		cont = TRUE;
	struct type_IOB_G3FacsimileBodyPart	*p2;
	BitMap		page;
	int		y;
	FILE		*fp;
	char    buffer[BUFSIZ], *start = NULLCP, *postCtrlL = NULLCP;

	myname = *argv++;

	sys_init(myname);

	p2 = (struct type_IOB_G3FacsimileBodyPart *) 
		calloc (1, sizeof(struct type_IOB_G3FacsimileBodyPart));

	p2 -> parameters = (struct type_IOB_G3FacsimileParameters *)
		calloc (1, sizeof(struct type_IOB_G3FacsimileParameters));
    
	p2 -> parameters -> optionals = 
		opt_IOB_G3FacsimileParameters_number__of__pages;


	while (*argv != NULL) {
		switch (cmd_srch(*argv,tbl_options)) {
		    case OPT_TABLE:
			if (*(argv+1) == NULLCP)
				fprintf(stderr,
					"no table given with flag %s", *argv);
			else {
				argv++;
				if (lexequ(*argv, "none") != 0
				    && ((table = tb_nm2struct (*argv)) == NULLTBL)) {
					fprintf(stderr,
						"Cannot initialise table '%s'\n", *argv);
					exit (1);
				}
			}
			break;

		    case OPT_FONT:
			if (*(argv+1) == NULLCP)
				fprintf(stderr,
					"no argument given with flag %s", *argv);
			else {
				argv++;
				font = *argv;
			}
			break;

		    default:
			fprintf(stderr,
				"unknown option '%s'\n", *argv);
			exit(1);
		}
		argv++;
	}

	/* Load the font to use */
	
	initialise_globals();

	if (font == NULLCP) {
		fprintf(stderr,
			"No font specified");
		exit(1);
	}
	if ((fp = fopen(font, "r")) == NULL
	    || (ppfont = file2font(fp)) == (PPFontPtr) NOTOK) {
		fprintf(stderr,
		       "unable to read in font from file '%s'",font);
		exit(1);
	}

	page = new_bitmap(FAX_WIDTH_LINES, FAX_HEIGHT_LINES);

	/* read the stdin a line at a time */
	while( cont == TRUE ) {
		/* do a page */
		clr_bitmap(page,FAX_WIDTH_LINES, FAX_HEIGHT_LINES);
		 
		y = TOP_PAD_LINES;
	    
		while (y + ppfont-> max_ht <= TEXT_HEIGHT_LINES 
		       && cont == TRUE) {
			
			if (start == NULLCP) {
				if (fgets(buffer, BUFSIZ, stdin) == NULL)
					cont = FALSE;
				else {
					/* knock off trailing \n */
					if ((start = rindex(buffer, '\n')) != NULLCP)
						*start = '\0';
					start = &(buffer[0]);
				}
			}
			if (cont == TRUE) {
				if (*start == '\0') {
					y += ppfont->max_ht;
					start = NULLCP;
				} else {
					int	dy, dx, i = 0;
					char	*ix;
					if ((postCtrlL = index(start, '')) != NULLCP)
						*postCtrlL++ = '\0';
						
					ix = start;
					while (ix - start < (int)strlen(start) 
					       && y + ppfont->max_ht <= TEXT_HEIGHT_LINES) {
						i = str_into_bitmap(page,
								    ppfont,
								    LEFT_PAD_LINES,
								    y,
								    &dx,
								    &dy,
								    FAX_WIDTH_LINES - LEFT_PAD_LINES,
								    ix,
								    strlen(ix));
						ix += i;
						y += dy;
					}
					
					if (ix - start >= (int) strlen(start))
						start = NULLCP;
					else
						start = ix;

					if (postCtrlL != NULLCP) {
						/* force new page */
						if (*postCtrlL == '\0')
							start = NULLCP;
						else
							start = postCtrlL;
						postCtrlL = NULLCP;
						y = TEXT_HEIGHT_LINES;
					}
				}
			}
		}
		encodeImage(page, 
			    FAX_WIDTH_LINES, FAX_HEIGHT_LINES,
			    p2);
	}
    
	outputFax(p2, stdout);
	exit(0);
} 

int
initialise_globals()
{
	char	buffer[BUFSIZ];

	if (table != NULLTBL) {
		if (font == NULLCP
		    && tb_k2val (table, "textfont", buffer, TRUE) != NOTOK)
			font = strdup(buffer);
	}
}
	
