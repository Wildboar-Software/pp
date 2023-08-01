/* misc.c : routines that wouldn't fit anywhere else */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/misc.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/misc.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: misc.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"
#include	<X11/cursorfont.h>

#include "back.bit"
Pixmap	backpixmap;

struct color_item 	*colors = NULL;
XColor		white,
		exact_white,
		black,
		exact_black;
Pixel		fg,
		bg;
extern int	compat;

create_pixmaps(disp)
Display	*disp;
{
	Window	root;
	root = XDefaultRootWindow(disp);
	backpixmap = XCreatePixmapFromBitmapData(disp, root,
					   back_bits, back_width, back_height,
					   fg,
					   bg, 
					   XDefaultDepth(disp,XDefaultScreen(disp)));
}

static Colormap	color_map;

extern int	num_colors;
extern Widget	quit_command;

#define MAX_COLOR_VALUE 65535

/* setup the colours used */
/* ARGSUSED */
SetColours(disp, win)
Display	*disp;
Window	win;
{
	int	i,
		badness;
	Colormap	cmap2;
	double	fract;

	XtVaGetValues (quit_command,
		   XtNbackground, &bg,
		   XtNforeground, &fg,
		   NULL);

	color_map = DefaultColormap(disp,
				     XDefaultScreen(disp));

	XAllocNamedColor(disp, color_map,
			 "white",
			 &white,
			 &exact_white);
	
	XAllocNamedColor(disp, color_map,
			 "black",
			 &black,
			 &exact_black);

	colors = (struct color_item *) calloc((unsigned) num_colors,
						   sizeof(struct color_item));
	if (num_colors == 1) {
		colors[0].badness = 0;
		colors[0].colour.pixel = fg;
	} else {
		for (i = 0; i < num_colors; i++) {
			badness = (i * max_bad_channel/num_colors);
			colors[i].badness = badness;

			colors[i].colour.pixel = 0;
			colors[i].colour.blue = 0;

			if (badness > max_bad_channel/2)
				colors[i].colour.red = MAX_COLOR_VALUE;
			else {
				fract = (double) badness * 2.0 / (double) max_bad_channel;
				colors[i].colour.red = fract * MAX_COLOR_VALUE;
			}
			if (colors[i].colour.red > MAX_COLOR_VALUE)
				colors[i].colour.red = MAX_COLOR_VALUE;

			if (max_bad_channel - badness > max_bad_channel/2)
				colors[i].colour.green = MAX_COLOR_VALUE;
			else {
				fract = ((double) (max_bad_channel - badness) * 2.0) / (double) max_bad_channel;
				colors[i].colour.green = fract * MAX_COLOR_VALUE;
			}
			if (colors[i].colour.green > MAX_COLOR_VALUE)
				colors[i].colour.green = MAX_COLOR_VALUE;

			colors[i].colour.flags = DoRed | DoGreen | DoBlue;
		}
		for (i = 0; i < num_colors; i++) {
			if (XAllocColor(disp, color_map, &(colors[i].colour)) == 0) {
				cmap2 = XCopyColormapAndFree(disp, color_map);
				for (; i < num_colors; i++) 
					XAllocColor(disp, 
						    cmap2, 
						    &(colors[i].colour));
				color_map = cmap2;
				break;
			}
		}
		XSetWindowColormap(disp, win, color_map);
	}
	create_pixmaps(disp);
}

char	*stripstr(str)
char	*str;
{	
	char	*ret,
		*ix;

	ix = str;
	while (*ix != '\0' && isspace(*ix)) ix++;
	ret = ix;
	while(*ix != '\0' && *ix != '\n') ix++;
	if (*ix == '\n') *ix = '\0';

	return ret;
}

int is_loc_chan(chan)
struct chan_struct	*chan;
{
	if (chan->chantype == int_Qmgr_chantype_mts
	    && chan->outbound > 0)
		return	TRUE;
	else
		return FALSE;
}

extern Display	*disp;
extern Widget	top;
Window		waitWindow = NULL;

StartWait()
{
	if (waitWindow == (Window)NULL) {
		XSetWindowAttributes	attributes;
		attributes.cursor = XCreateFontCursor(disp, XC_watch);
		attributes.do_not_propagate_mask = (KeyPressMask | 
						    KeyReleaseMask |
						    ButtonPressMask | 
						    ButtonReleaseMask |
						    PointerMotionMask);
		waitWindow = XCreateWindow(disp,
					   XtWindow(top),
					   0, 0,
					   XDisplayWidth(disp,
							 XDefaultScreen(disp)),
					   XDisplayHeight(disp,
							  XDefaultScreen(disp)),
					   (unsigned int) 0,
					   CopyFromParent,
					   InputOnly,
					   CopyFromParent,
					   (CWDontPropagate | CWCursor),
					   &attributes);
	}
	(void) XMapRaised(disp, waitWindow);
	(void) XFlush(disp);
}

EndWait()
{
	(void) XUnmapWindow(disp, waitWindow);
}

extern Widget	time_label;
extern Widget	statistics;

undisplay_time_label()
{
  XtSetMappedWhenManaged(time_label, False);
  if (statistics != NULL)
	  XtSetMappedWhenManaged(statistics, False);
}

extern Widget compat_stat_pane;

redo_statistics_compat()
{
	if (statistics != NULL) {
		if (compat) {
			XtSetMappedWhenManaged(compat_stat_pane, False);
			XtSetMappedWhenManaged(statistics, False);
		} else {
			XtSetMappedWhenManaged(statistics, True);
			XtSetMappedWhenManaged(compat_stat_pane, True);
		}
	}
}

display_time_label()
{
  update_time_label();
  XtSetMappedWhenManaged(time_label, True);
  if (!compat && statistics != NULL)
	  XtSetMappedWhenManaged(statistics, True);
}

extern time_t currentTime;
extern char   *time_t2RFC();
extern int	currChans, maxChans;
extern int	messagesIn, messagesOut, addrIn, addrOut;
extern time_t	boottime;

update_time_label()
{
  char	*str, buf[BUFSIZ];
  time(&currentTime);
  str = time_t2RFC(currentTime);
  (void) sprintf(buf, "%d channel%s running (max %d) at %s", 
		 currChans, (currChans == 1) ? "" : "s",
		 maxChans, str);
  free(str);
  XtVaSetValues(time_label, 
	    XtNlabel, buf,
	    NULL);
  
  if (!compat && statistics != NULL) {
#define	NUMBUF	15
	  char	adIn[NUMBUF], mgIn[NUMBUF], adOut[NUMBUF], mgOut[NUMBUF];
	  str = time_t2RFC(boottime);
	  num2unit(addrIn, adIn);
	  num2unit(messagesIn, mgIn);
	  num2unit(addrOut, adOut);
	  num2unit(messagesOut, mgOut);
	  (void) sprintf(buf, "running since %s\nInbound %s Message%s to %s Recipient%s\nOutbound %s Message%s to %s Recipient%s",
			 str, mgIn, (messagesIn != 1) ? "s" : "",
			 adIn, (addrIn != 1) ? "s" : "",
			 mgOut, (messagesOut != 1) ? "s" : "",
			 adOut, (addrOut != 1) ? "s" : "");
	  free(str);
	  XtVaSetValues(statistics,
			XtNlabel, buf,
			NULL);
  }
}

extern State	connectState;

/* ARGSUSED */	    
void setLoad (w, ptr_load, load)
Widget		w;
double		*ptr_load;
double		*load;
{
	if (connectState == connected)
		*load = *ptr_load / 100;
	else
		*load = 0;
}
	
