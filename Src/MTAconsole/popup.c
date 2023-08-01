/* popup.c: pop up and down routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/popup.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/popup.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: popup.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include		"console.h"

extern Popup_menu 	*yesno,
			*one,
			*two,
			*three,
			*config,
			*qcontrol;
extern Widget		refresh_toggle,
			reconnect_toggle,
			backoff_toggle,
			top,
			mainform;
extern XColor		black,
			white;
extern Pixel		bg, fg;
Popup_menu *currentpopup = NULL;
extern Operations	currentop;
extern Heuristic	newheur;

Popup(this, op, x,y)
Popup_menu	*this;
Operations	op;
int		x,
		y;
{
	int	i;
	currentpopup = this;
	this->op = op;
	XtVaSetValues(this->popup,
		  XtNx,	x,
		  XtNy, y,
		  NULL);
	if (this != yesno 
	    && this != qcontrol) {
		switch (currentop) {
		    case msgclear:
		    case msgstart:
		    case msgstop:
		    case msgforce:
			this->selected = 1;
			break;
		    default:			
			this->selected = 0;
			break;
		}

		for (i = 0; i < this -> numberOftuples; i++)
			if (i != this -> selected)
				XtVaSetValues(this->tuple[i].text,
					  XtNborderColor, bg,
					  NULL);
		XtSetKeyboardFocus(this->form,
				   this->tuple[this->selected].text);
		XtSetKeyboardFocus(top, 
				   this->tuple[this->selected].text);
		XtVaSetValues(this->tuple[this->selected].text,
			  XtNborderColor, fg,
			  NULL);

	} else {
		XtSetKeyboardFocus(top, this->form);
	}
	XtPopup(this->popup, XtGrabExclusive);
}

Popdown(this)
Popup_menu	*this;
{
	currentpopup = NULL;
	if (this != NULL) {
		XtPopdown(this->popup);
		XtSetKeyboardFocus(top, mainform);
	}
}

/*  */
/* routines to move selected fields in popups */

extern int		newauth;
extern Popup_menu	*connectpopup;

/* ARGSUSED */
void	previousField(w, event, params, num_params)
Widget	w;
XEvent	*event;
char   	**params;
int	num_params;
{
	int old;

	if (currentpopup == NULL
	    || currentpopup->numberOftuples < 2)
		return;

	old = currentpopup->selected;
		
	currentpopup->selected--;
	if (currentpopup->selected < 0) {
		if (currentpopup == connectpopup
		    && newauth == False)
			currentpopup->selected = 1;
		else if (currentpopup == config) {
			switch(newheur) {
			    case percentage:
				currentpopup->selected = MINBADMTA;
				break;
			    case line:
				currentpopup->selected = LINEMAX;
				break;
			    case all:
				currentpopup->selected = CONNECTMAX;
				break;
			    case chanonly:
				currentpopup->selected = CONNECTMAX;
				break;
			}
		} else
			currentpopup->selected = 
				currentpopup->numberOftuples -1;
	} else if (currentpopup == config
		   && old == LINEMAX
		   && newheur == line)
		currentpopup->selected = CONNECTMAX;

	XtSetKeyboardFocus(top,
			   currentpopup->tuple[currentpopup->selected].text);
	XtSetKeyboardFocus(currentpopup->form,
			   currentpopup->tuple[currentpopup->selected].text);
	XtVaSetValues(currentpopup->tuple[currentpopup->selected].text,
		  XtNborderColor, fg,
		  NULL);
	XtVaSetValues(currentpopup->tuple[old].text,
		  XtNborderColor, bg,
		  NULL);
}

/* ARGSUSED */
void	thisField(w, event, params, num_params)
Widget	w;
XEvent	*event;
char	**params;
int	num_params;
{
	int	ix;
	
	if (currentpopup == NULL
	    || currentpopup->numberOftuples < 2)
		return;

	ix = 0;
	while (ix < currentpopup->numberOftuples
	       && currentpopup->tuple[ix].text != w)
		ix++;
	
	if (ix >= currentpopup->numberOftuples)
		return;
	
	XtVaSetValues(currentpopup->tuple[currentpopup->selected].text,
			  XtNborderColor, bg,
			  NULL);
	currentpopup->selected = ix;
	XtVaSetValues(currentpopup->tuple[currentpopup->selected].text,
			  XtNborderColor, fg,
			  NULL);
	XtSetKeyboardFocus(top,
			   currentpopup->tuple[currentpopup->selected].text);
	XtSetKeyboardFocus(currentpopup->form,
			   currentpopup->tuple[currentpopup->selected].text);
}

/* ARGSUSED */
void	nextField(w, event, params, num_params)
Widget	w;
XEvent	*event;
char   	**params;
int	num_params;
{
	int old;

	if (currentpopup == NULL
	    || currentpopup->numberOftuples < 2)
		return;

	old = currentpopup->selected;
		
	currentpopup->selected++;
	
	if (currentpopup->selected >= currentpopup->numberOftuples)
		currentpopup->selected = 0;

	if (currentpopup == connectpopup
	    && newauth == False
	    && currentpopup->selected > 1)
		currentpopup->selected = 0;

	if (currentpopup == config
	    && currentpopup->selected > CONNECTMAX) {
		switch (newheur) {
		    case percentage:
			if (currentpopup->selected > MINBADMTA)
				currentpopup->selected = 0;
			break;
		    case line:
			currentpopup->selected = LINEMAX;
			break;
		    case all:
		    case chanonly:
			currentpopup->selected = 0;
		}
	}


	XtSetKeyboardFocus(top,
			   currentpopup->tuple[currentpopup->selected].text);
	XtSetKeyboardFocus(currentpopup->form,
			   currentpopup->tuple[currentpopup->selected].text);
	XtVaSetValues(currentpopup->tuple[currentpopup->selected].text,
		  XtNborderColor, fg,
		  NULL);
	XtVaSetValues(currentpopup->tuple[old].text,
		  XtNborderColor, bg,
		  NULL);
}

/* ARGSUSED */
void	mymenupopdown(w, event)
Widget	w;
XEvent	*event;
{
	Popdown(currentpopup);
}

extern char	password[];
#define STRBUFSIZE	100
static XComposeStatus compose_status = {NULL, 0};
/* ARGSUSED */
void	myinsert_char(w, event)
Widget	w;
XKeyEvent	*event;
{
	char	strbuf[STRBUFSIZE],
		xbuf[STRBUFSIZE];
	int	ix;
	KeySym	keycode;
	int	length, passwdlen;
	length = XLookupString(event, strbuf, STRBUFSIZE,
			       &keycode, &compose_status);
	if (length == 0) return;
	passwdlen = strlen(password);
	strncat(password, strbuf, length);
	password[length+passwdlen] = '\0';
	
	for (ix = 0; ix < length + passwdlen; ix++)
	     xbuf[ix] = 'X';
	xbuf[ix] = NULL;
#ifdef notdef
	if (length == 1 && strbuf[0] == '\?')
		text.ptr = &strbuf[0];
	else
		text.ptr = &xbuf[0];
#endif
	XtVaSetValues (w, XtNstring, xbuf, 0);
}

/* ARGSUSED */
void	mydelete_char(w, event)
Widget	w;
XEvent	*event;
{
	char	xbuf[STRBUFSIZE];
	int	ix;
	int	passwdlen;

	passwdlen = strlen(password) -1;
	if (passwdlen >= 0) {
		password[passwdlen] = '\0';
		for (ix = 0; ix < passwdlen; ix++)
			xbuf[ix] = 'X';
		xbuf[passwdlen] = '\0';
		textdisplay(&(connectpopup->tuple[3]), xbuf);
	}
}
