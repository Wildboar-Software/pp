/* auth_ut.c: authorisation utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/auth_ut.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/auth_ut.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: auth_ut.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "q.h"
#include <stdarg.h>

extern char		auth2submit_msg[];


/* -- local routines -- */
void			authchan_add();
void			authmta_add();
void			do_reason(AUTH *, char *, ...);
AUTH			*auth_new();
AUTH_USER		*authusr_new();
LIST_AUTH_CHAN		*authchan_new();
LIST_AUTH_MTA		*authmta_new();
static LIST_AUTH_CHAN	*authchan_malloc();
static LIST_AUTH_MTA	*authmta_malloc();




/* ------------------------  Begin  Routines  ------------------------------- */




#ifdef lint
/*VARARGS2*/
void do_reason (au, str)
AUTH   *au;
char   *str;
{
	do_reason (au, str);
}

#else
void do_reason (AUTH *au, char *str, ...)
{
	va_list		ap;

	va_start (ap, str);

	_asprintf (auth2submit_msg, NULLCP, str, ap);

	au -> reason = strdup (auth2submit_msg);
	va_end (ap);
}
#endif




LIST_AUTH_CHAN *authchan_new (value, def)	
CHAN   			*value;
LIST_AUTH_CHAN		*def;
{
	LIST_AUTH_CHAN	*list;


	PP_TRACE (("authchan_new (%x)", value)); 

	list = authchan_malloc();

	if (def)
		*list = *def;	/* --- structure copy --- */
	if (value) 
		list -> li_chan = value;

	return (list);
}




void authchan_add (list, item)
LIST_AUTH_CHAN		**list;
LIST_AUTH_CHAN		*item;
{
	LIST_AUTH_CHAN	*lp = *list;

	PP_TRACE (("authchan_add()"));

	if (lp == NULLIST_AUTHCHAN) {
		*list = item;
		return;
	}

	while (lp -> li_next != NULLIST_AUTHCHAN)
		lp = lp -> li_next;
	lp -> li_next = item;
}




void authmta_add (list, item)
LIST_AUTH_MTA		**list;
LIST_AUTH_MTA		*item;
{
	LIST_AUTH_MTA	*lp = *list;

	PP_TRACE (("authmta_add()"));

	if (lp == NULLIST_AUTHMTA) {
		*list = item;
		return;
	}

	while (lp -> li_next != NULLIST_AUTHMTA)
		lp = lp -> li_next;
	lp -> li_next = item;
}




LIST_AUTH_MTA *authmta_new (value)
char   *value;
{
	LIST_AUTH_MTA	*list;

	if (value == NULLCP)
		return (NULLIST_AUTHMTA);

	PP_TRACE (("authmta_new ('%s')", value));

	list = authmta_malloc();
	list -> li_mta = value;
	return (list);
}




AUTH_USER *authusr_new()
{
	AUTH_USER *new;

	PP_TRACE (("authusr_new()"));
	new = (AUTH_USER *) malloc (sizeof *new);
	bzero ((char *)new, sizeof *new);

	new -> found		= FALSE;
	new -> rights		= AUTH_RIGHTS_UNSET;
	return (new);
}




AUTH *auth_new(isdr)
int isdr;
{
	AUTH   *new;

	PP_TRACE (("auth_new()"));

	new = (AUTH *) malloc (sizeof *new);
	bzero ((char *)new, sizeof *new);

	if (isdr)
		new -> status	= AUTH_DR_OK;
	else
		new -> status 	= AUTH_OK;

	new -> stage 		= AUTH_STAGE_1;
	new -> warnings		= AUTH_NOTWARNED;
	new -> mta_inrights	= AUTH_RIGHTS_UNSET;
	new -> mta_outrights	= AUTH_RIGHTS_UNSET;
	new -> user_inrights	= AUTH_RIGHTS_UNSET;
	new -> user_outrights	= AUTH_RIGHTS_UNSET;
	return (new);
}




/* ------------------------  Static Routines  ------------------------------- */




static LIST_AUTH_CHAN *authchan_malloc()
{
	LIST_AUTH_CHAN	*new;

	PP_TRACE (("authchan_malloc()"));
	new = (LIST_AUTH_CHAN *) malloc (sizeof *new);
	bzero ((char *)new, sizeof *new);

	new -> li_found		= FALSE;
	new -> policy		= AUTH_CHAN_FREE;
	return (new);
}




static LIST_AUTH_MTA *authmta_malloc()
{
	LIST_AUTH_MTA *new;

	PP_TRACE (("authmta_malloc()"));

	new = (LIST_AUTH_MTA *) malloc (sizeof *new);
	bzero ((char *)new, sizeof *new);
	
	new -> li_found		= FALSE;
	new -> rights		= AUTH_RIGHTS_UNSET;
	return (new);
}
