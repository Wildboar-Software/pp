/* xfont_conv.c:convert an X font to PP nonX format */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/Xfont2PPfont.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/Xfont2PPfont.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: Xfont2PPfont.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */
#include	"util.h"
#include 	<stdio.h>
#include 	<X11/Xlib.h>
#include 	<X11/Xutil.h>
#include        <X11/Intrinsic.h>
#include        <X11/StringDefs.h>
#include        <X11/Shell.h>
#include	"fonts.h"
#include	<isode/cmd_srch.h>

#define	OPT_FONT	1
#define OPT_FILE	2

CMD_TABLE	tbl_options [] = {
	"-font",	OPT_FONT,
	"-file",	OPT_FILE,
	0,		-1,
};

extern PPFontPtr	new_font();
extern CharPtr		new_char();
extern BitMap		new_bitmap();
#define	MAXASCII	127

typedef struct _cmdLineResources {
        char    *font;
	char	*file;
} CmdLineResources;

static CmdLineResources cmdLine_resources;

clear_pixmap(display, pixmap, gc,wd,ht)
Display	*display;
Pixmap	pixmap;
GC	gc;
unsigned int	wd, ht;
{
    /* clear the pixmap */
    XSetFunction(display, gc, GXclear);

    XFillRectangle(display,
		   pixmap,
		   gc,
		   0, 0,
		   wd,ht);

    XSetFunction(display, gc, GXcopy);
}

main(argc, argv)
int	argc;
char	**argv;
{      
	Arg     arg[1];
	XFontStruct	*fontstruct;
	Display	*display;
	XtAppContext    appContext;
	PPFontPtr	ppfont;
	FILE		*fp;
	int	wid, ht, base, i;
	char	ch;
	Pixmap	pixmap;
	XGCValues	gcv;
	GC		gc;
	unsigned long	black, white;
	XImage	*image;
	int	row, col;

	for (i = 1; i < argc; i++) {
		switch (cmd_srch(argv[i], tbl_options)) {
		    case OPT_FILE:
			if (++i >= argc) {
				printf("No file name given with flag '%s'\n",
				       argv[i-1]);
				exit (1);
			}
			cmdLine_resources.file = strdup(argv[i]);
			break;
		    case OPT_FONT:
			if (++i >= argc) {
				printf("No font name given with flag '%s'\n",
				       argv[i-1]);
				exit (1);
			}
			cmdLine_resources.font = strdup(argv[i]);
			break;
		    default:
			printf("Unknown flag '%s'\n",
			       argv[i]);
			exit(1);
		}
	}

	if (cmdLine_resources.font == NULLCP) {
		printf("No font specified\n");
		exit(0);
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

	if ((fontstruct = XLoadQueryFont(display, 
					 cmdLine_resources.font)) == NULL) {
		printf("display %s doesn't know about font '%s'\n",
		       DisplayString(display), cmdLine_resources.font);
		exit(1);
	}
	
	ht = fontstruct->max_bounds.ascent 
		+ fontstruct->max_bounds.descent;
	wid = fontstruct->max_bounds.lbearing
		+ fontstruct->max_bounds.rbearing;
	base = fontstruct->max_bounds.ascent;

	pixmap = XCreatePixmap(display,
			       XRootWindow(display, XDefaultScreen(display)),
			       wid, ht,
			       (unsigned int)1);


	gcv.font = fontstruct->fid;

	gcv.foreground = black;
	gcv.background = white;

	gc = XCreateGC(display, pixmap, 
		       (GCFont | GCForeground |GCBackground), &gcv);

	ppfont = new_font(MAXASCII+1);

	for (i = 0;i < MAXASCII+1;i++) {
		/* step through ascii codes */
		CharPtr	temp = new_char();
		temp->ascii = i;
		
		if (isprint(ch = (char) i)) {
			char	buf[1];
			buf[0] = ch;

			clear_pixmap(display, pixmap, gc, 
				     wid, ht);
			
			temp->wid = XTextWidth(fontstruct,
					       buf,
					       1);
			temp->ht = ht;

			XDrawImageString(display,
					 pixmap,
					 gc,
					 0,
					 base,
					 buf,
					 1);

			image = XGetImage(display,
					  pixmap,
					  0, 0,
					  temp->wid, temp->ht,
					  XAllPlanes(),
					  XYPixmap);
			
			/* convert x bitmap into pp bitmap */
			temp->bits = new_bitmap(temp->wid, temp->ht);

			for (row = 0; row < temp->ht; row++) {
				for (col = 0; col < temp->wid; col++) {
					if (XGetPixel(image, col, row) == white) 
						BL_CLR(col, temp->bits[row]);
					else
						BL_SET(col, temp->bits[row]);
				}
			}
		}
		ppfont->chars[i] = temp;
	}
	
	if (cmdLine_resources.file) {
		if ((fp = fopen(cmdLine_resources.file, "w")) == NULL) {
			printf("unable to open output file '%s'",
			       cmdLine_resources.file);
			exit (1);
		}
	} else
		fp = stdout;

	font2file (fp, ppfont);

	if (fp != stdout)
		fclose(fp);
	exit(0);
}
