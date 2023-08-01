/* drive.c: driving routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/drive.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/drive.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: drive.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"

#ifdef	notdef
#include	"IntrinsicP.h"
#include	"TranslateI.h"
#include	"ConvertI.h"
#include	"InitialI.h"
#endif
#include	<sys/stat.h>
#include	<pwd.h>

struct stat	statbuf;
#define MAXMTAS		10	/* shouldn't get anything longer than that ? */

#define	DEFAULT_NORMALFONT	"*-medium-r-normal--14-*"
#define DEFAULT_ACTIVEFONT	"*-bold-r-normal--14-*"
#define DEFAULT_DISABLEDFONT	"*-medium-i-normal--14-*"

typedef struct _cmdLineResources {
	int	percentage;
	int	lowerbound;
	int	totalnumber;
	int	totalvolume;
	int	maxvert;
	Boolean	confirm;
	Boolean	ukorder;
	Boolean	compat;
	Boolean	ignoreInactiveInbounds;
	int	ncolours;
	char	*connectHost;
	char	*heuristic;
	XFontStruct	*defaultFont;
	char	*refresh;
	char	*inactive;
} CmdLineResources;

static CmdLineResources cmdLine_resources;

static XtResource resources[] = {
{"xtDefaultFont", "XtDefaultFont", XtRFontStruct, sizeof(XFontStruct *),
	 XtOffset(struct _cmdLineResources *, defaultFont),
	 XtRImmediate, (caddr_t) 0},
{"percentage", "Percentage", XtRInt, sizeof(int),
	 XtOffset(struct _cmdLineResources *, percentage),
	 XtRImmediate, (caddr_t) 0},
{"lowerbound", "Lowerbound", XtRInt, sizeof(int),
	 XtOffset(struct _cmdLineResources *, lowerbound),
	 XtRImmediate, (caddr_t) 0},
{"totalnumber", "Totalnumber", XtRInt, sizeof(int),
	 XtOffset(struct _cmdLineResources *, totalnumber),
	 XtRImmediate, (caddr_t) 0},
{"totalvolume", "Totalvolume", XtRInt, sizeof(int),
	 XtOffset(struct _cmdLineResources *, totalvolume),
	 XtRImmediate, (caddr_t) 0},
{"maxvert", "Maxvert", XtRInt, sizeof(int),
	 XtOffset(struct _cmdLineResources *, maxvert),
	 XtRImmediate, (caddr_t) 0},
{"confirm", "Confirm", XtRBoolean, sizeof(Boolean),
	 XtOffset(struct _cmdLineResources *, confirm),
	 XtRImmediate, (caddr_t) TRUE},
{"ukorder", "UkOrder", XtRBoolean, sizeof(Boolean),
	 XtOffset(struct _cmdLineResources *, ukorder),
	 XtRImmediate, (caddr_t) FALSE},
{"ncolours", "Ncolours", XtRInt, sizeof(int),
	 XtOffset(struct _cmdLineResources *, ncolours),
	 XtRImmediate, (caddr_t) 0},
{"connectHost", "ConnectHost", XtRString, sizeof(String),
	 XtOffset(struct _cmdLineResources *, connectHost),
	 XtRImmediate, (caddr_t) NULLCP},
{"compat", "Compat", XtRBoolean, sizeof(Boolean),
	 XtOffset(struct _cmdLineResources *, compat),
	 XtRImmediate, (caddr_t) FALSE},
{"ignoreInactiveInbounds", "IgnoreInactiveInbounds", XtRBoolean, sizeof(Boolean),
	 XtOffset(struct _cmdLineResources *, ignoreInactiveInbounds),
	 XtRImmediate, (caddr_t) FALSE},
{"heuristic", "Heuristic", XtRString, sizeof(String),
	 XtOffset(struct _cmdLineResources *, heuristic),
	 XtRImmediate, (caddr_t) NULLCP},
{"refresh", "Refresh", XtRString, sizeof(String),
	 XtOffset(struct _cmdLineResources *, refresh),
	 XtRImmediate, (caddr_t) NULLCP},
{"inactiveTime", "InactiveTime", XtRString, sizeof(String),
	 XtOffset(struct _cmdLineResources *, inactive),
	 XtRImmediate, (caddr_t) NULLCP},
};

static XrmOptionDescRec options[] = {
{"-percentage",	"percentage",	XrmoptionSepArg,	0},
{"-lowerbound",	"lowerbound",	XrmoptionSepArg,	0},
{"-number", "totalnumber",	XrmoptionSepArg,	0},
{"-volume", "totalvolume",	XrmoptionSepArg,	0},
{"-downLines", "maxvert",	XrmoptionSepArg,	0},
{"-NoConfirm", "confirm",	XrmoptionNoArg,		"False"},
{"-ukorder", "ukorder",		XrmoptionNoArg,		"True"},
{"-colours", "ncolours",	XrmoptionSepArg,	0},
{"-Queue", "connectHost",	XrmoptionSepArg,	NULLCP},
{"-compat", "compat",		XrmoptionNoArg, 	"True"},
{"-ignoreInactiveInbounds", "ignoreInactiveInbounds", XrmoptionNoArg,	"True"},
{"-heuristic", "heuristic",	XrmoptionSepArg,	NULLCP},
{"-refresh", "refresh",		XrmoptionSepArg,	NULLCP},
{"-inactiveTime", "inactiveTime",XrmoptionSepArg,	NULLCP},
};

XtAppContext	appContext;
Display		*disp;
char		*hostname,
		tailorfile[MAXPATHLENGTH];
int		confirm = TRUE,
		uk_order = FALSE,
		doConnect = FALSE,
		userConnected = FALSE,
		autoRefresh = TRUE,
		compat = FALSE,
		displayInactIns = TRUE,
		autoReconnect = TRUE,
		auth = FALSE,
		lower_bound_mtas,
		percent;
Heuristic	heuristic;
State		connectState = notconnected;
Authentication  authentication = limited;
char		*Qinformation = NULLCP,
		*Qversion = NULLCP;
extern unsigned long	timeoutFor, inactiveFor;
time_t		currentTime;
XFontStruct	*disabledFont = NULL,
		*activeFont = NULL,
		*normalFont = NULL,
		*defaultFont = NULL;

static void general_initialise();
extern Widget	header, error, top;
#define		WAIT_CONNECTION		"Please wait for connection to the queue manager"

char	*myname;
char	password[100],
	username[100];

extern double	ub_total_number, ub_total_volume;
extern ConnectRetry();

main(argc, argv)
int	argc;
char	**argv;
{
	int	uid;
	struct passwd *pwd;
	uid = getuid();
	pwd = getpwuid(uid);
	strcpy(username, pwd->pw_name);
	strcpy(password, "dummy");
	strcpy(tailorfile,".MTAconsole");
	if ((myname = rindex (argv[0], '/')) != NULL)
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = argv[0];

	fillin_passwdpep(username, NULLCP, 0);
	general_initialise(argc, argv);
	initiate_assoc(argc, argv);
	create_widgets(disp, appContext);
	
	SetColours(disp, XtWindow(top));
	if (doConnect == TRUE) {
		XtVaSetValues(header,
			  XtNlabel, WAIT_CONNECTION,
			  NULL);
		InitConnectTimeOut();
	}
	XtAppMainLoop(appContext);
}

#define		DEFAULT_MAX_BORDER		5
#define 	DEFAULT_NUM_COLORS 		10
#define		DEFAULT_UL_MAX_LINES		4
int	max_chan_border = DEFAULT_MAX_BORDER,
	max_mta_border = DEFAULT_MAX_BORDER,
	max_msg_border = DEFAULT_MAX_BORDER;
int 	num_colors = DEFAULT_NUM_COLORS;
int	max_horiz_mtas,
	max_vert_lines;

extern time_t	parsetime();
extern void	advise();

static void general_initialise(argc, argv)
int 	argc;
char	**argv;
{
	char	buf[BUFSIZ];
	Arg	arg[1];
	extern int optind;
	extern char *optarg;
	extern Widget	top;

	gethostname(buf,BUFSIZ);
	hostname = strdup(buf);

	isodetailor(myname, 1);

	XtToolkitInitialize();

	appContext = XtCreateApplicationContext();

	disp = XtOpenDisplay(appContext,
				(String) NULL,
				(String) myname,
				(String) APPLICATION_CLASS,
			     options, XtNumber(options),
				&argc,
				argv);
	if (disp == NULL) {
		printf("%s\n", "unable to open display");
	        exit(1);
	}
	/* to get round bug in XtRemoveInput */
	XtRemoveInput(XtAppAddInput(appContext,
				    0,
				    XtInputReadMask,
				    ConnectRetry,
				    NULL));

	XtSetArg(arg[0], XtNinput, True);
	/* create top level */
	top = XtAppCreateShell(myname,
			       APPLICATION_CLASS,
			       applicationShellWidgetClass,
			       disp,
			       arg,
			       0);

	
	if (DisplayCells (disp, DefaultScreen(disp)) <= 2)
		num_colors = 1;
	else 
		max_chan_border = max_mta_border = 2;

	max_horiz_mtas = MAXMTAS;
	max_vert_lines = DEFAULT_UL_MAX_LINES;
	heuristic = line;
	lower_bound_mtas = 20;
	percent = 50;

	XtGetApplicationResources (top, &cmdLine_resources,
				   resources, XtNumber(resources), NULL, 0);
	

	if (cmdLine_resources.defaultFont) {
		defaultFont = cmdLine_resources.defaultFont;
	}

	if (cmdLine_resources.percentage) {
		percent = cmdLine_resources.percentage;
	}
	if (cmdLine_resources.lowerbound) {
		lower_bound_mtas = cmdLine_resources.lowerbound;
	}
	if (cmdLine_resources.totalnumber)
		ub_total_number = cmdLine_resources.totalnumber;
	if (cmdLine_resources.totalvolume)
		ub_total_volume = cmdLine_resources.totalvolume;
	if (cmdLine_resources.maxvert)
		max_vert_lines = cmdLine_resources.maxvert;
	confirm = cmdLine_resources.confirm;
	uk_order = cmdLine_resources.ukorder;
	if (cmdLine_resources.ncolours)
		num_colors = cmdLine_resources.ncolours;
	if (cmdLine_resources.connectHost) {
		doConnect = TRUE;
		hostname = strdup(cmdLine_resources.connectHost);
	}
	compat = cmdLine_resources.compat;
	displayInactIns = (cmdLine_resources.ignoreInactiveInbounds == TRUE) ?
		FALSE : TRUE;
	if (cmdLine_resources.heuristic != NULLCP) {
		if (lexequ(cmdLine_resources.heuristic, "all") == 0)
			heuristic = all;	
		else if (lexnequ(cmdLine_resources.heuristic, 
				"percent", strlen("percent")) == 0)
			heuristic = percentage;
		else if (lexequ(cmdLine_resources.heuristic,
				 "line") == 0)
			heuristic = line;
		else if (lexequ(cmdLine_resources.heuristic, "mtaonly") == 0
			 || lexequ(cmdLine_resources.heuristic, "chanonly") == 0)
			heuristic = chanonly;
	}
	if (cmdLine_resources.refresh != NULLCP) 
		settimeoutFor(cmdLine_resources.refresh);
	if (cmdLine_resources.inactive != NULLCP)
		inactiveFor = parsetime(cmdLine_resources.inactive) * 1000;
	setfonts();
}

XFontStruct	*get_three_choice_font(font, one, two, three)
char	*font, *one, *two, *three;
{
	char	*retstr;
	XFontStruct	*retfont;

	if ((retstr = XGetDefault(disp, one, font)) != NULLCP
	    && (retfont = XLoadQueryFont(disp, retstr)) != NULL)
		return retfont;
	if ((retstr = XGetDefault(disp, two, font)) != NULLCP
	    && (retfont = XLoadQueryFont(disp, retstr)) != NULL)
		return retfont;
	if ((retfont = XLoadQueryFont(disp, three)) != NULL)
		return retfont;
	if (defaultFont == NULL) {
		printf("Unable to load font '%s'\n", font);
		exit(0);
	} 
	return defaultFont;
}
	
setfonts ()
{
	normalFont = get_three_choice_font("normalfont",
					   myname,
					   APPLICATION_CLASS,
					   DEFAULT_NORMALFONT);
	activeFont = get_three_choice_font("activefont",	
					   myname,
					   APPLICATION_CLASS,
					   DEFAULT_NORMALFONT);
	disabledFont = get_three_choice_font("disabledfont",
					     myname,
					     APPLICATION_CLASS,
					     DEFAULT_NORMALFONT);
}	
