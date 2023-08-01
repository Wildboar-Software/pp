/* xalert.c: X based alert program */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/xalert.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/xalert.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: xalert.c,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 */



#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>	
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Cardinals.h>	
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Paned.h>
#include <pwd.h>

#include <isode/manifest.h>
#include <isode/psap.h>
#include <sys/types.h>
#include "sys.file.h"
#include <sys/stat.h>
#include <utmp.h>
#include <sys/time.h>
#include <ctype.h>
#include <signal.h>

#include "data.h"
#include "image.h"

#define check_width 9
#define check_height 8
static char check_bits[] = {
   0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00,
   0x31, 0x00, 0x1b, 0x00, 0x0e, 0x00, 0x04, 0x00
};

#define	BASIC_WIDTH 400

static struct _app_resources {
	char	*filename;
	char	*user;
	int	port;
	Boolean	bell;
	Boolean autoraise;
	Boolean photos;
} app_resource;

static int	defaultPort = 0;
char	homedir[BUFSIZ];
char filename[] = ".alert";

char a_filename[]="-filename";
char a_user[]	= "-user";
char a_port[]	= "-port";
char a_d[]	= "-d";
char a_f[]	= "-f";
char a_u[]	= "-u";
char a_p[]	= "-p";
char a_s[]	= "-s";
char a_t[]	= "-t";
char a_w[]	= "-w";
char a_x[]	= "-x";

#define offset(x)	XtOffset (struct _app_resources *, x)
static XtResource resources[] = {
{"fileName",	"FileName",	XtRString,	sizeof (String),
	 offset(filename),	XtRString, 	(caddr_t) filename },
{"userName",	"UserName",	XtRString,	sizeof (String),
	 offset(user),		XtRString,	(caddr_t) NULL },
{"port",	"Port",		XtRInt,		sizeof (int),
	 offset(port),		XtRInt,		(caddr_t) &defaultPort },
{"bell",	"Bell",		XtRBoolean,	sizeof(Boolean),
	 offset(bell),		XtRImmediate,	(caddr_t)1},
{"autoRaise",	"AutoRaise",	XtRBoolean,	sizeof(Boolean),
	 offset(autoraise),	XtRImmediate,	(caddr_t)0},
#ifndef NOPHOTOS
{"photos",	"Photos",	XtRBoolean,	sizeof(Boolean),
	 offset(photos),	XtRImmediate,	(caddr_t)1},
#endif
};

static XrmOptionDescRec table[] = {
{a_filename,	"fileName",	XrmoptionSepArg,	(caddr_t) NULL},
{a_user,	"userName",	XrmoptionSepArg,	(caddr_t) NULL},
{a_port,	"port",		XrmoptionSepArg,	(caddr_t) NULL},
{"-nobell",	"Bell",		XrmoptionNoArg,		(caddr_t)"False"},
{"-bell",	"Bell",		XrmoptionNoArg,		(caddr_t)"True"},
{"-autoraise",	"AutoRaise",	XrmoptionNoArg,		(caddr_t)"True"},
{"-noautoraise","AutoRaise",	XrmoptionNoArg,		(caddr_t)"False"},
#ifndef NOPHOTOS
{"-nophotos",	"Photos",	XrmoptionNoArg,		(caddr_t)"False"},
{"-photos",	"Photos",	XrmoptionNoArg,		(caddr_t)"True"},
#endif
};

XtAppContext context;
Display *dpy;
Screen 	*scrn;
Widget 	toplevel, pane, box, qb, topform, txt, photow, photolbl, db, optionw, pbox;
static Pixmap check;

char	username[128];
int	replysock;
char	twfilename[BUFSIZ];
char	hostname[128];
char	*extra = "NEW: ";
char	*format = "%extra(%size) %20from %subject << %50body";
static char	*myname;
int	width = 80;
int	debug = 0;
int	stdoutonly = 0;
int	no_x = 0;
int	termwidth=80;

static void	Quit ();
static void	QuitCancel ();
static void	Delete ();
static void	TextIn ();
static void	catch_exit ();
static int	EQuit ();
static int	EQuit2 ();
static void	Cancel ();
static void	Ok ();
static void create_menu ();
#ifndef NOPHOTOS
static void 	lookup_photo ();
static void	PhotoClr ();
static void photo_clr ();

#endif

XtActionsRec actionTable[] = {
{"Ok",	Ok},
{"Cancel",	Cancel}
};

XrmDatabase fred;

main (argc, argv)
int	argc;
char	*argv[];
{
	int sd;
	struct passwd *pwd;
	char	*cp, *getenv ();
	long	ptr;
	XrmValue value;

	value.size=sizeof(ptr);
	value.addr= (caddr_t) (&ptr);

	myname = argv[0];
	if ((pwd = getpwuid (getuid ())) == NULL) {
		fprintf (stderr, "Unknown user");
	}
	else {
		(void) strcpy (username, pwd -> pw_name);
		if ((cp = getenv ("HOME")) && *cp)
			(void) strcpy (homedir, cp);
		else
			(void) strcpy (homedir, pwd -> pw_dir);
	}

	pre_scan(argc, argv);

	if (! no_x) {
		XtToolkitInitialize();
		context = XtCreateApplicationContext();
		dpy = XtOpenDisplay(context, (String)NULL, "xalert", "XAlert",
				    table, XtNumber(table), &argc, argv);
		if (dpy == (Display *)0) {
			fprintf (stderr, "Unable to open display, so using tty\n");
			no_x++;
		}
	}

	if (no_x) {
		if (!app_resource.filename) app_resource.filename = filename;
		if (!app_resource.user) app_resource.user = username;
		signal(SIGHUP, catch_exit);
		signal(SIGTERM, catch_exit);
		signal(SIGINT, catch_exit);
		sd = udp_start (app_resource.port, app_resource.filename, homedir);
		for (;;) {
			fd_set rfds;
			char	buffer[BUFSIZ];
			char	from[BUFSIZ];
			int	n;
	
			FD_ZERO (&rfds);
			FD_SET (sd, &rfds);
			if(select (sd + 1, &rfds, NULLFD, NULLFD,
				   (struct timeval *)0) <= 0)
				continue;
	
			n = sizeof (buffer) - 1;
			if (FD_ISSET (sd, &rfds) &&
			    getdata (sd, buffer, &n, app_resource.user,
				     from)) {
				buffer[n] = 0;
				if (termwidth < n) buffer[termwidth] = 0;
				if (stdoutonly || (notify_normal (buffer, app_resource.user) == NOTOK &&
						   isatty (1)))
					printf ("\n\r%s%c\n\r",
						buffer, '\007');
			}
		}
	}

	scrn = DefaultScreenOfDisplay(dpy);

	XtAppAddActions(context, actionTable, XtNumber(actionTable));

	toplevel = XtAppCreateShell("xalert", "XAlert", 
				    applicationShellWidgetClass, dpy, NULL,0);

	XtGetApplicationResources (toplevel, (caddr_t)&app_resource, resources,
				   XtNumber(resources), NULL, 0);

	if (app_resource.user == NULL)
		app_resource.user = username;

	sd = udp_start (app_resource.port, app_resource.filename, homedir);

	pane = XtVaCreateManagedWidget ("toppane",
					panedWidgetClass,
					toplevel,
					NULL);

	topform = XtVaCreateManagedWidget("topform",
					  formWidgetClass,
					  pane,
					  NULL);
	box = XtVaCreateManagedWidget ("box",
				       formWidgetClass,
				       topform,
				       NULL);

        qb = XtVaCreateManagedWidget ("Quit",
				      commandWidgetClass,
				      box,
				      NULL);
	XtAddCallback(qb, XtNcallback, QuitCancel, NULL);

	db = XtVaCreateManagedWidget ("Clear",
				      commandWidgetClass,
				      box,
				      NULL);
	XtAddCallback (db, XtNcallback, Delete, NULL);

	optionw = XtVaCreateManagedWidget ("Options",
					   menuButtonWidgetClass,
					   box,
					   XtNmenuName, "Options",
					   NULL);

	create_menu (toplevel, "Options", optionw);

	txt = XtVaCreateManagedWidget("text",
				      asciiTextWidgetClass,
				      topform,
				      NULL);
#ifndef NOPHOTOS
	pbox = XtVaCreateWidget ("pbox",
				 formWidgetClass,
				 pane,
				 NULL);

	photolbl = XtVaCreateManagedWidget ("photolabel",
					    labelWidgetClass,
					    pbox,
					    NULL);

	photow = XtVaCreateManagedWidget ("photo",
					  commandWidgetClass,
					  pbox,
					  XtNlabel,	"",
					  0);
	XtAddCallback (photow, XtNcallback, PhotoClr, NULL);

	if(app_resource.photos) {
	    XtManageChild(pbox);
	}

#endif

	XSetErrorHandler (EQuit);
	XSetIOErrorHandler (EQuit2);

	(void) XtAppAddInput (context, sd, XtInputReadMask, TextIn, NULL);
	
	XtInstallAllAccelerators(txt, toplevel);

	XtRealizeWidget(toplevel);

	XtSetKeyboardFocus(toplevel, txt);

	XtAppMainLoop (context);
}

static void	Reset ();
static void	ToggleBell ();
static void 	AutoRaise ();
#ifndef NOPHOTOS
static void	TogglePhotos ();
#endif

struct MenuEntries {
	char *name;
	void (*fnx)();
	Boolean	*state;
};

struct MenuEntries menus[] = {
	"Reset", 	Reset,		NULL,
	"Bell",		ToggleBell,	&app_resource.bell,
	"AutoRaise", 	AutoRaise,	&app_resource.autoraise,
#ifndef NOPHOTOS
	"Photos",	TogglePhotos,	&app_resource.photos,
#endif
	NULLCP,		NULL,		NULL,
	};

static void create_menu (top, name, button)
Widget top;
char	*name;
Widget button;
{
	Widget mw, bsb;
	struct MenuEntries *mp;

	check = XCreateBitmapFromData (dpy,
				       RootWindowOfScreen(XtScreen(top)),
				       check_bits,
				       check_width,
				       check_height);
	mw = XtVaCreatePopupShell (name,
				   simpleMenuWidgetClass,
				   button,
				   NULL);

	for (mp = menus; mp -> name; mp ++) {
		bsb = XtVaCreateManagedWidget (mp -> name,
					       smeBSBObjectClass,
					       mw,
					       XtNleftMargin, check_width + 5,
					       NULL);
		XtAddCallback (bsb, XtNcallback, mp -> fnx, button);
		if (mp -> state && *mp ->state)
			XtVaSetValues (bsb,
				       XtNleftBitmap, check,
				       NULL);
	}
}

/* ARGSUSED */
static void	TextIn (cdata, fdp, iid)
caddr_t	cdata;
int	*fdp;
XtInputId *iid;
{
	char	buffer[BUFSIZ];
	char	from[BUFSIZ];
	int	n;

	if (*fdp == 0) {
		if ((n = read (*fdp, buffer, sizeof buffer)) <= 0)
			Quit (toplevel, (caddr_t)0, (caddr_t)0);
		buffer[n] = 0;
		AddText (buffer, n);
	}
	else {
		if (getdata (*fdp, buffer, &n, app_resource.user, from)) {
			AddText (buffer, n);
#ifndef NOPHOTOS
			lookup_photo(from);
#endif
		}
		
	}
}

/* ARGSUSED */
static int	EQuit (disp, error)
Display *disp;
XErrorEvent *error;
{
	Quit (toplevel, (caddr_t)0, (caddr_t)0);
}

/* ARGSUSED */
static int	EQuit2 (disp)
Display *disp;
{
	Quit (toplevel, (caddr_t)0, (caddr_t)0);
}

/* ARGSUSED */
static void Quit(widget, closure, callData)
Widget widget;
caddr_t closure;
caddr_t callData;
{
	XtDestroyWidget(toplevel);
	catch_exit();
}

static void catch_exit()
{
	if (twfilename[0])
		(void) unlink (twfilename);
	udp_cleanup ();

	exit(0);
}

AddText (s, n)
char	*s;
int	n;
{
	XawTextPosition      last;
	XawTextBlock text;

	XawTextSetInsertionPoint (txt, (XawTextPosition)9999999);
	last = XawTextGetInsertionPoint (txt);

	text.ptr = s;
	text.firstPos = 0;
	text.length = n;
	text.format = FMT8BIT;

	if (XawTextReplace (txt, last, last, &text) != XawEditDone)
		XBell (XtDisplay (txt), 0);
	else {
		XawTextPosition newend;

		XawTextSetInsertionPoint(txt, last + text.length);
		newend = XawTextGetInsertionPoint(txt);
		if (text.ptr[text.length-1] != '\n') {
			text.ptr = "\n";
			text.length = 1;
			XawTextReplace(txt, newend, newend, &text);
			XawTextSetInsertionPoint(txt, newend += 1);
		}
	}
	if (app_resource.bell)
		XBell (XtDisplay (toplevel), 50);
	if (app_resource.autoraise)
		XMapRaised (dpy, XtWindow(toplevel));
}

/* ARGSUSED */
static void Reset (widget, closure, callData)
Widget widget;
caddr_t closure;
caddr_t callData;
{
	makeredirfile (app_resource.filename, homedir);
}

/* ARGSUSED */
static void ToggleBell (widget, closure, callData)
Widget widget;
caddr_t closure;
caddr_t callData;
{
	app_resource.bell = !app_resource.bell;
	if (app_resource.bell)
		XtVaSetValues (widget, XtNleftBitmap, check, NULL);
	else	XtVaSetValues (widget, XtNleftBitmap, None, NULL);
}

/* ARGSUSED */
static void AutoRaise (widget, closure, callData)
Widget widget;
caddr_t closure;
caddr_t callData;
{
	app_resource.autoraise = !app_resource.autoraise;
	if (app_resource.autoraise)
		XtVaSetValues (widget, XtNleftBitmap, check, NULL);
	else	XtVaSetValues (widget, XtNleftBitmap, None, NULL);
}

/* ARGSUSED */
static void Delete (widget, closure, callData) 
Widget widget;
caddr_t closure;
caddr_t callData;
{
	XtVaSetValues (txt, XtNstring, "", NULL);
#ifndef NOPHOTOS
	photo_clr ();
#endif
}


/* YUCK ! This should use XrmParseCommand etc ... */
pre_scan (argc, argv)
int	argc;
char	*argv[];
{
	int i;

	for (i=1; i<argc; i++)
	{	if (!strcmp(argv[i], a_filename) || !strcmp(argv[i], a_f))
			app_resource.filename = argv[++i];
		else if (!strcmp(argv[i], a_user) || !strcmp(argv[i], a_u))
			app_resource.user = username;
		else if (!strcmp(argv[i], a_port) || !strcmp(argv[i], a_p))
			app_resource.port = atoi(argv[++i]);
		else if (!strcmp(argv[i], a_s))
			stdoutonly = 1;
		else if (!strcmp(argv[i], a_t))
			no_x = 1;
		else if (!strcmp(argv[i], a_w))
			termwidth = atoi(argv[++i]);
		else if (!strcmp(argv[i], a_x))
			no_x = 0;
		else if (!strcmp(argv[i], a_d))
			debug = 1;
	}
}

#ifndef NOPHOTOS
/*ARGSUSED*/
static void TogglePhotos (widget, closure, callData)
Widget widget;
caddr_t closure;
caddr_t callData;
{
	app_resource.photos = !app_resource.photos;

	if (app_resource.photos) {
	    XtVaSetValues (widget, XtNleftBitmap, check, NULL);
	    XtManageChild(pbox);
	    photo_clr();
	}
	else {
	    XtVaSetValues (widget, XtNleftBitmap, None, NULL);
	    XtUnmanageChild(pbox);
	}
}


static void photo_clr ()
{
	XtVaSetValues (photow, XtNbitmap, None, NULL);
	XtVaSetValues (photow, XtNlabel, "", NULL);
	XtVaSetValues (photolbl, XtNlabel, "", NULL);
}

/* ARGSUSED */
static void PhotoClr (widget, closure, callData) 
Widget widget;
caddr_t closure;
caddr_t callData;
{
	photo_clr ();
}

static void lookup_photo (from)
char *from;
{
	static int first_time = 0;
	char	*cp;
	char	user[BUFSIZ];
	Pixmap pix;
	struct image *image;

	if (app_resource.photos == False)
		return;

	if (first_time == 0) {
		extern int ufn_indent;
		first_time = 1;
		ufn_indent = 0;
		init_aka (myname, 0, NULLCP);
	}
	if (XtIsRealized(photow) == 0)
		return;

	photo_clr (photow);

	if ((cp = index(from, '@')) == NULL)
		return;

	(void) strncpy (user, from, cp - from);
	user[cp - from] = 0;

	if ((image = fetch_image (user, cp + 1)) == NULL)
		return;

	if (image -> data) {
		pix = XCreateBitmapFromData (dpy, XtWindow (photow),
					     image -> data -> qb_forw -> qb_data,
					     image -> width, image -> height);
		if (pix == None)
			return;
		XtVaSetValues (photow,
			       XtNbitmap, pix,
			       0);
	}
	else {
		char lbuf[BUFSIZ*2];

		lbuf[0] = 0;
		if (image -> phone) 
			(void) strcat (lbuf, image -> phone);
		if (image -> address)
			(void) strcat (lbuf, image -> address);
		if (image -> info)
			(void) strcat (lbuf, image -> info);
		if (image -> description)
			(void) strcat (lbuf, image -> description);
		XtVaSetValues (photow,
			       XtNlabel, lbuf,
			       NULL);
	}
	if (image -> ufnname)
		XtVaSetValues (photolbl,
			       XtNlabel, image -> ufnname,
			       NULL);
}

#endif



static void DestroyPopupPrompt(); /* the callback of the popup cancel button */
static void Ok();		/* an action proc calling ColorTheButton */


/*ARGSUSED*/
static void 
QuitCancel(button, client_data, call_data)
Widget	button;		
XtPointer client_data, call_data;
{
    Arg		args[5];
    Widget	popup, dialog;
    Position	x, y;
    Dimension	width, height;
    Cardinal	n;

    n = 0;
    XtSetArg(args[0], XtNwidth, &width); n++;
    XtSetArg(args[1], XtNheight, &height); n++;
    XtGetValues(button, args, n);
    XtTranslateCoords(button, (Position) (width / 2), (Position) (height / 2),
		      &x, &y);

    n = 0;
    XtSetArg(args[n], XtNx, x);				n++;
    XtSetArg(args[n], XtNy, y);				n++;

    popup = XtCreatePopupShell("prompt", transientShellWidgetClass, button,
			       args, n);

    dialog = XtVaCreateManagedWidget("dialog",
				     dialogWidgetClass,
				     popup,
				     NULL);

    /*
     * The prompting message's size is dynamic; allow it to request resize. 
     */

    XawDialogAddButton(dialog, "ok", Quit, (XtPointer) dialog);
    XawDialogAddButton(dialog, "cancel", DestroyPopupPrompt,(XtPointer)dialog);

    XtInstallAllAccelerators(dialog, dialog);    

    XtPopup(popup, XtGrabExclusive);

    XtSetKeyboardFocus(toplevel, dialog);
}
/*ARGSUSED*/
static void 
DestroyPopupPrompt(widget, client_data, call_data)
Widget	widget;		
XtPointer client_data, call_data;	
{
    Widget popup = XtParent( (Widget) client_data);
    XtDestroyWidget(popup);
}

/*ARGSUSED*/
static void 
Ok(widget, event, params, num_params)
Widget widget;		
XEvent *event;		
String *params;	
Cardinal *num_params;
{
	catch_exit ();
}

/*ARGSUSED*/
static void 
Cancel(widget, event, params, num_params)
Widget widget;		
XEvent *event;		
String *params;	
Cardinal *num_params;
{
	Widget dialog = XtParent(widget);

	DestroyPopupPrompt(widget, (XtPointer) dialog, (XtPointer) NULL);
}
