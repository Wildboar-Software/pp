/* Ximage2PPbitmap.c: converts as X image to a PP bitmap */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/X2PPbitmap.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/X2PPbitmap.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: X2PPbitmap.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */

#include	"util.h"
#include	<stdio.h>
#include 	<X11/Xlib.h>
#include 	<X11/Xutil.h>
#include        <X11/Intrinsic.h>
#include        <X11/StringDefs.h>
#include        <X11/Shell.h>
#include	"fonts.h"
#include	<isode/cmd_srch.h>

#define 	OPT_IN	1
#define		OPT_OUT 2

CMD_TABLE	tbl_options [] = {
	"-in",	OPT_IN,
	"-out",	OPT_OUT,
	0,	-1,
};

typedef struct _cmdLineResources {
        char    *in;
	char	*out;
} CmdLineResources;

static CmdLineResources cmdLine_resources;
extern BitMap	new_bitmap();

main(argc, argv)
int	argc;
char	**argv;
{      
	Arg     arg[1];
	Display	*display;
	XtAppContext    appContext;
	int	wid, ht, hot_x, hot_y;
	Pixmap	pixmap;
	XImage	*image;
	unsigned long	black, white;
	int	row, col, i;
	FILE	*fp;
	BitMap	bitmap;
	char	*ix;

	for (i = 1; i < argc; i++) {
		switch (cmd_srch(argv[i], tbl_options)) {
		    case OPT_IN:
			if (++i >= argc) {
				printf("No input file given with flag '%s'\n",
				       argv[i-1]);
				exit (1);
			}
			cmdLine_resources.in = strdup(argv[i]);
			break;
		    case OPT_OUT:
			if (++i >= argc) {
				printf("No output file given with flag '%s'\n",
				       argv[i-1]);
				exit (1);
			}
			cmdLine_resources.out = strdup(argv[i]);
			break;
		    default:
			printf("Unknown flag '%s'\n",
			       argv[i]);
			exit(1);
		}
	}

	if (cmdLine_resources.in == NULLCP) {
		printf("No input file given\n");
		exit(1);
	}

	XtToolkitInitialize();

        appContext = XtCreateApplicationContext();

        display = XtOpenDisplay(appContext,
			     (String) NULL,
			     (String) argv[0],
			     (String) "PPCONVERTERS",
                             NULL, 0,
			     &argc,
			     argv);

	if (display == NULL) {
                printf("%s\n", "unable to open display");
                exit(1);
        }
	
	black = XBlackPixel(display, XDefaultScreen(display));
	white = XWhitePixel(display, XDefaultScreen(display));

	if (XReadBitmapFile(display, 
			    XRootWindow(display, XDefaultScreen(display)),
			    cmdLine_resources.in, &wid, &ht,
			    &pixmap, &hot_x, &hot_y) != BitmapSuccess) {
		printf("Unable to create pixmap from bitmap file '%s'\n",
		       cmdLine_resources.in);
		exit(1);
	}

	image = XGetImage (display,
			   pixmap,
			   0,
			   0,
			   wid, ht,
			   XAllPlanes(),
			   XYPixmap);
	
	bitmap = (BitMap) new_bitmap(wid, ht);
	
	for (row = 0; row < ht; row++) {
		for (col = 0; col < wid; col++) {
			if (XGetPixel(image, col, row) == white) 
				BL_CLR(col, bitmap[row]);
			else
				BL_SET(col, bitmap[row]);
		}
	}

	if (cmdLine_resources.out == NULL)
		fp = stdout;
	else if ((fp = fopen(cmdLine_resources.out, "w")) == NULL) {
		printf("unable to open output file '%s'\n", 
		       cmdLine_resources.out);
		exit(1);
	}
	bitmap2file (fp, bitmap, wid, ht);
	fclose(fp);
}
