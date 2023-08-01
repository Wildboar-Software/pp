/* auto.c:routines for auto things */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/auto.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/auto.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: auto.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"

unsigned long 	timeoutFor = 300000,
		inactiveFor = 600000,
		connectFor,
		connectInit = 300000,
		connectInc = 60000,
		connectMax = 600000;
int	backoff	= TRUE;
caddr_t  retryMask = (caddr_t) XtInputWriteMask;
XtIntervalId	refreshId = 0;
XtIntervalId	refreshStatusId = 0;
XtIntervalId	connectId = 0;
XtIntervalId	inactiveId = 0;
extern XtAppContext	appContext;
extern Display		*disp;
extern Widget		top;
extern char		*hostname;
extern void 		ChangeMode();
extern int	autoReconnect,
		userConnected,
		autoRefresh;
extern State	connectState;

static int InitRefreshStatusTimeOut(), TermRefreshStatusTimeOut();

char	*top_of_refresh_list();

/* ARGSUSED */
ConnectTimeOut(data, id)
caddr_t	data;	/* NOT USED */
XtIntervalId	*id;
{
	/* try connect */
	connectId = 0;
	if (connectState == notconnected && autoReconnect == TRUE) {
		construct_event(connect);
	}
}

InitConnectTimeOut()
{
	if (connectId != 0) XtRemoveTimeOut(connectId);
	if (connectState == notconnected && autoReconnect == TRUE) {
		connectFor = connectInit;
		connectId = XtAppAddTimeOut(appContext,
					    (userConnected == TRUE) ? connectFor : 500,
					    ConnectTimeOut,	
					    NULL);
	}
}

TermConnectTimeOut()
{
	if (connectId != 0) {
		XtRemoveTimeOut(connectId);
		connectId = 0;
	}
}

/*  */
extern Mode	mode;
extern int	errorUp;
extern Widget	error;

/* ARGSUSED */
Inactive(data, id)
caddr_t		data;
XtIntervalId	*id;
{
	inactiveId = 0;
	if (mode == control && connectState == connected)
		ChangeMode((Widget)NULL, (caddr_t) NULL, (caddr_t) NULL);
}


ResetInactiveTimeout()
{
	if (errorUp > 0) {
		errorUp--;
		if (errorUp == 0)
			XtSetMappedWhenManaged(error, False);
	}
	if (inactiveId != 0) XtRemoveTimeOut(inactiveId);
	if (mode == control && connectState == connected)
		inactiveId = XtAppAddTimeOut(appContext,
					     inactiveFor,
					     Inactive,
					     NULL);
	else
		inactiveId = 0;
}

/*  */
extern int compat;
extern Widget statistics;
/* ARGSUSED */
RefreshTimeOut(data,id)
caddr_t		data;	
XtIntervalId	*id;
{
	refreshId = 0;
	if (connectState == connected ) {
		construct_event(chanread);
		if (autoRefresh == TRUE) 
			refreshId = XtAppAddTimeOut(appContext,
						    timeoutFor,
						    RefreshTimeOut,
						    NULL);
	}
}
		
InitRefreshTimeOut(tx)
unsigned long	tx;
{
	if (refreshId != 0) XtRemoveTimeOut(refreshId);
	if (autoRefresh == TRUE && connectState == connected) {
		refreshId = XtAppAddTimeOut(appContext,
					    (tx == 0) ? timeoutFor : tx,
					    RefreshTimeOut,
					    NULL);
		if (!compat)
			InitRefreshStatusTimeOut(tx);
	} else 
		refreshId = 0;
}

TermRefreshTimeOut()
{
	if (refreshId != 0){
		XtRemoveTimeOut(refreshId);
		refreshId = 0;
	}
	TermRefreshStatusTimeOut();
}

/*  */

/* ARGSUSED */
static RefreshStatusTimeOut(data,id)
caddr_t		data;	
XtIntervalId	*id;
{
	refreshStatusId = 0;
	if (connectState == connected ) {
		my_invoke(qmgrStatus, (char **) NULL);
		refreshStatusId = XtAppAddTimeOut(appContext,
						  timeoutFor / 2,
						  RefreshStatusTimeOut,
						  NULL);
	}
}
		
static InitRefreshStatusTimeOut(tx)
unsigned long	tx;
{
	if (refreshStatusId != 0) XtRemoveTimeOut(refreshStatusId);
	if (connectState == connected)
		refreshStatusId = XtAppAddTimeOut(appContext,
					    (tx == 0) ? (timeoutFor/2) : tx / 2,
					    RefreshStatusTimeOut,
					    NULL);
	else 
		refreshStatusId = 0;
}

static TermRefreshStatusTimeOut()
{
	if (refreshStatusId != 0){
		XtRemoveTimeOut(refreshStatusId);
		refreshStatusId = 0;
	}
}

/*  */

int construct_event(op)
Operations	op;
{
  XClientMessageEvent	event;

  event.type = ClientMessage;
  event.send_event = True;
  event.display = disp;
  event.window = XtWindow(top);
  event.format = 8;
  event.data.b[0] = (char) op;
	  
  XSendEvent(disp,
	     XtWindow(top),
	     False,
	     NoEventMask,
	     (XEvent *) &event);
/*  if (result == 0)
	  temp = result;*/
}

int construct_refresh_event(widget)
Widget	widget;
{
  XClientMessageEvent	event;

  event.type = ResizeRequest;
  event.send_event = True;
  event.display = disp;
  event.window = XtWindow(widget);
	  
  XSendEvent(disp,
	     XtWindow(widget),
	     False,
	     VisibilityChangeMask,
	     (XEvent *)&event);
/*  XSync(XtDisplay(widget), False);*/
}

/* ARGSUSED */
void client_msg_handler(w, client_data, event)
Widget	w;
caddr_t	client_data;
XClientMessageEvent	*event;
{
	char	*name;
	extern char	*msginfo_args[];
	if (event -> type == ClientMessage &&
	    connectState == connected) {
		switch ((Operations) event -> data.b[0]) {
		    case chanread:
			if (!compat)
				my_invoke(qmgrStatus, (char **) NULL);
			my_invoke(chanread, (char **) NULL);
			break;
		    case mtaread:
			if ((name = top_of_refresh_list()) != NULLCP) {
				my_invoke(mtaread, &name);
				construct_event(mtaread);
			}

			break;
		    case readchannelmtamessage:
			my_invoke(readchannelmtamessage, msginfo_args);
			break;
			
		    default:
			break;
		}

	} else if (event -> type == ClientMessage &&
		   connectState == notconnected &&
		   autoReconnect == TRUE &&
		   (Operations) event -> data.b[0] == connect) {
		XSync(disp, 0);
		Connect(hostname);
		if (connectState == notconnected) {
			if (backoff == TRUE && connectFor < connectMax)
				connectFor += connectInc;
		}
	}
}

/*  */
/* refresh list stuff */
typedef struct mta_refresh_struct {
	char	*name;
	struct mta_refresh_struct *next;
} Mta_refresh_struct;

struct mta_refresh_struct	*mta_refresh_list = NULL;

Mta_refresh_struct	*new_mta_refresh_struct(name)
char	*name;
{
	Mta_refresh_struct * ret =
		(Mta_refresh_struct *) calloc(1, sizeof (Mta_refresh_struct));
	ret -> name = strdup(name);
	return ret;
}

free_mta_refresh_list(list)
Mta_refresh_struct	*list;
{
	if (list == NULL)
		return;

	if (list -> next != NULL)
		free_mta_refresh_list(list -> next);

	if (list -> name != NULLCP)
		free(list -> name);

	free((char *) list);
}

clear_mta_refresh_list()
{

	free_mta_refresh_list(mta_refresh_list) ;
	mta_refresh_list = NULL;
}

add_mta_refresh_list(name)
char	*name;
{
	Mta_refresh_struct	*ix = mta_refresh_list;

	if (mta_refresh_list == NULL)
		mta_refresh_list = new_mta_refresh_struct(name);
	else {
		while (ix -> next != NULL)
			ix = ix -> next;
		ix -> next = new_mta_refresh_struct(name);
	}
}

char	*top_of_refresh_list()
{
	if (mta_refresh_list != NULL)
		return mta_refresh_list -> name;
	return NULLCP;
}

char	*pop_from_mta_refresh_list()
{
	char	*ret = NULL;
	Mta_refresh_struct	*temp;

	if ((temp = mta_refresh_list) != NULL) {
		ret = temp -> name;
		mta_refresh_list = mta_refresh_list -> next;
		free((char *) temp);
	}

	return ret;
}

/*  */
XtInputId	retryId = 0,
		assocId = 0;

/* ARGSUSED */
ConnectRetry(data, fd, id)
caddr_t		data;
int		*fd;
XtInputId	*id;
{
	int	result;
	int	myfd = *fd;


	XtRemoveInput(*id);
	retryId = 0;
	switch( result = acsap_retry(myfd)) {
#ifdef	CONNECTING_1
	    case CONNECTING_1:
	    case CONNECTING_2:
		Connecting(result);
		break;
#else
	    case OK:
		retryMask = (caddr_t) XtInputReadMask;
		Connecting(result);
		break;
#endif
	    case DONE:
		Connected();
		break;
	    case NOTOK:
	    default:
		NotConnected();
		break;
	}
}

TermConnectRetry()
{
	if (retryId != 0) {
		XtRemoveInput(retryId);
		retryId = 0;
	}
}

InitConnectRetry(sd, res)
int	sd,
	res;
{
	XSync(disp, 0);
#ifdef CONNECTING_1
	if (res == CONNECTING_1)
		retryMask = (caddr_t) XtInputWriteMask;
	else
		retryMask = (caddr_t) XtInputReadMask;
#endif

	retryId = XtAppAddInput(appContext,
				sd,
				retryMask,
				ConnectRetry,
				NULL);
}



/*  */

/* ARGSUSED */ 
Listen(data, fd, id)
caddr_t		data;	/* unused */
int		*fd;	/* where to read from */
XtInputId	*id;	/* unused */
{
	if (ros_work(*fd) == NOTOK) 
		return;
}

InitListen(fd)
int	fd;
{
	/* set Listen to listen on fd */
	if (assocId != 0) XtRemoveInput(assocId);
	assocId = XtAppAddInput(appContext, fd, 
				XtInputReadMask,
				Listen,
				NULL);
}

/* ARGSUSED */ 
TermListen()
{
	/* unset Listen which is listening on fd */
	if (assocId != 0) {
		XtRemoveInput(assocId);
		assocId = 0;
	}
}
