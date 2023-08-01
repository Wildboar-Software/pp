/* tb_getauth.c: retrieve authorisation information */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getauth.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getauth.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_getauth.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"util.h"
#include	"chan.h"
#include	"auth.h"
#include	"list_rchan.h"
#include	"adr.h"
#include	<isode/cmd_srch.h>



/* -- statics -- */
static Table	*Chan_auth	= NULLTBL;
static Table	*Mta_auth	= NULLTBL;
static Table	*User_auth	= NULLTBL;



/* -- command tables -- */

static  CMD_TABLE authtbl_chan[] = {
	"free",			AUTH_CHAN_FREE,
	"negative",		AUTH_CHAN_NEGATIVE,
	"block",		AUTH_CHAN_BLOCK,
	"none",			AUTH_CHAN_NONE,
	"warnsender",		AUTH_CHAN_WARNS,
	"warnrecipient",	AUTH_CHAN_WARNR,
	"sizelimit",		AUTH_CHAN_SIZELIMIT,
	"test",			AUTH_CHAN_TEST,
	0,              	-1
	};



#define AUTH_MTA_DEFAULT        1
#define AUTH_MTA_REQUIRES       2
#define AUTH_MTA_EXCLUDES       3
#define AUTH_MTA_CONTENTEX      4
#define AUTH_MTA_SIZELIMIT      5

static  CMD_TABLE authtbl_mta[] = {
	"default",		AUTH_MTA_DEFAULT,
	"requires",		AUTH_MTA_REQUIRES,
	"excludes",		AUTH_MTA_EXCLUDES,
	"content-excludes",	AUTH_MTA_CONTENTEX,
	"bodypart-excludes",	AUTH_MTA_CONTENTEX,
	"sizelimit",		AUTH_MTA_SIZELIMIT,
	0,			-1
	};



#define AUTH_USER_DEFAULT       1
#define AUTH_USER_CONTENTEX     2
#define AUTH_USER_SIZELIMIT     3

static  CMD_TABLE authtbl_user[] = {
	"default",		AUTH_USER_DEFAULT,
	"content-excludes",	AUTH_USER_CONTENTEX,
	"bodypart-excludes",	AUTH_USER_CONTENTEX,
	"sizelimit",		AUTH_USER_SIZELIMIT,
	0,			-1
	};



CMD_TABLE authtbl_rights[] = {	/* NOT static - used in auth.c */
	"in",			AUTH_RIGHTS_IN,
	"out",			AUTH_RIGHTS_OUT,
	"both",			AUTH_RIGHTS_BOTH,
	"none",			AUTH_RIGHTS_NONE,
	"unset",		AUTH_RIGHTS_UNSET,
	0,              	-1
	};


extern CHAN			*ch_inbound;	/* -- chan calling submit -- */
extern char			*chan_auth_tbl;
extern char			*mta_auth_tbl;
extern char			*user_auth_tbl;
extern char			*ad_getlocal();
extern long			atol ();


/* -- local routines */
int				tb_getauthchan();
int				tb_getauthmta();
int				tb_getauthusr();
void				tb_parse_authchan();

static LIST_CHAN_RIGHTS		*list_chan_rights_new();
static LIST_REGEX		*list_regex_new();
static int 			tb_authrights();
static void			add_cont_excludes();
static void			add_chanrights();
static void			add_regex();
static void			list_chan_rights_add();
static void			list_regex_add();
static void			tb_getauthchan_aux();
static void			tb_parse_authmta();




/* --------------------- Begin  Routines ------------------------------------ */




int tb_getauthchan (ochan)
LIST_AUTH_CHAN		*ochan;
{
	char		key[MAXPATHLENGTH];


	ochan -> li_found = FALSE;

	if (Chan_auth == NULLTBL)
		if ((Chan_auth = tb_nm2struct (chan_auth_tbl)) == NULLTBL) {
			PP_LOG (LLOG_EXCEPTIONS,
				 ("Lib/tb_getauthchan missing %s",
				chan_auth_tbl));
			return OK;
		}


	(void) sprintf (key, "%s->%s",
			ch_inbound -> ch_name, ochan -> li_chan -> ch_name);
	(void) tb_getauthchan_aux (key, ochan);
	if (ochan -> li_found == TRUE)	return OK;


	(void) sprintf (key, "%s->*", ch_inbound -> ch_name);
	(void) tb_getauthchan_aux (key, ochan);
	if (ochan -> li_found == TRUE)	return OK;


	(void) sprintf (key, "*->%s", ochan -> li_chan -> ch_name);
	(void) tb_getauthchan_aux (key, ochan);

	return OK;
}




void tb_parse_authchan (c, argc, argv)
LIST_AUTH_CHAN		*c;
int			argc;
char			**argv;
{
	int		i, retval;
	char		*p;

	for (i = 0; i < argc; i++) {
		if (!isstr(argv[i]))
			continue;
		if ((p = index (argv[i], '=')) == NULLCP) {

			PP_DBG (("Lib/tb_parse_authchan %s", argv[i]));

			switch (retval = cmd_srch (argv[i], authtbl_chan)) {
			    case AUTH_CHAN_FREE:
				c -> policy = AUTH_CHAN_FREE;
				break;
			    case AUTH_CHAN_NEGATIVE:
				c -> policy = AUTH_CHAN_NEGATIVE;
				break;
			    case AUTH_CHAN_BLOCK:
				c -> policy = AUTH_CHAN_BLOCK;
				break;
			    case AUTH_CHAN_NONE:
				c -> policy = AUTH_CHAN_NONE;
				break;
			    case AUTH_CHAN_TEST:
				c -> test   = AUTH_CHAN_TEST;
				break;
			    default:
				PP_OPER (NULLCP,
					 ("Lib/tb_parse_authchan error %d %s",
					  retval, argv[i]));
				break;
			}
		}
		else {
			*p++ = '\0';
			PP_DBG (("Lib/tb_parse_authchan %s = %s", argv[i], p));

			switch (retval = cmd_srch (argv[i], authtbl_chan)) {
			    case AUTH_CHAN_WARNS:
				c -> warnsender = strdup (p);
				break;
			    case AUTH_CHAN_WARNR:
				c -> warnrecipient = strdup (p);
				break;
			    case AUTH_CHAN_SIZELIMIT:
				c -> sizelimit = atol (p);
				break;
			    default:
				PP_OPER(NULLCP,
					("Lib/tb_parse_authchan error %d %s %s",
					 retval, argv[i], p));
				break;
			}
		}
	}
}




int tb_getauthmta (omta)
LIST_AUTH_MTA	*omta;
{
	char	*argv [MAX_AUTH_ARGS];
	char	buf[BUFSIZ];
	int	argc;

	PP_DBG (("Lib/tb_getauthmta (%s)", omta -> li_mta));

	omta -> li_found = FALSE;

	if (Mta_auth == NULLTBL)
		if ((Mta_auth = tb_nm2struct (mta_auth_tbl)) == NULLTBL) {
			PP_LOG (LLOG_EXCEPTIONS,
				 ("Lib/tb_getauthmta missing %s",
				 mta_auth_tbl));
			return OK;
		}

	if (tb_k2val (Mta_auth, omta -> li_mta, buf, TRUE) == NOTOK)
		return OK;

	if ((argc = sstr2arg (buf, MAX_AUTH_ARGS, argv, " \t,")) == NOTOK)
		return OK;

	omta -> li_found = TRUE;

	(void) tb_parse_authmta (omta, argc, argv);
	return OK;
}




int tb_getauthusr (ousr, ad)
AUTH_USER	*ousr;
ADDR		*ad;
{
	char		*argv [MAX_AUTH_ARGS];
	char		buf[BUFSIZ];
	char		*p, *uname;
	int		argc, i, rights, retval = RP_BAD, found;
	CHAN		*c;


	ousr -> found = FALSE;

	if (User_auth == NULLTBL)
		if ((User_auth = tb_nm2struct (user_auth_tbl)) == NULLTBL) {
			PP_LOG (LLOG_EXCEPTIONS,
				 ("Lib/tb_getauthusr missing %s",
				 user_auth_tbl));
			return OK;
		}

	found = FALSE;
	
	if (ad->aparse) {
		if (found == FALSE
		    && ad->aparse->r822_str) {
			if (tb_k2val (User_auth, 
				      ad->aparse->r822_str,
				      buf, TRUE) != NOTOK) {
				uname = ad->aparse->r822_str;
				found = TRUE;
			}
		}
		if (found == FALSE
		    && ad->aparse->r822_local) {
			if (tb_k2val (User_auth,
				      ad->aparse->r822_local,
				      buf, TRUE) != NOTOK) {
				uname = ad->aparse->r822_local;
				found = TRUE;
			}
		}
		if (found == FALSE
		    && ad->aparse->x400_str) {
			if (tb_k2val (User_auth,
				      ad->aparse->x400_str,
				      buf, TRUE) != NOTOK) {
				uname = ad->aparse->x400_str;
				found = TRUE;
			}
		}
		if (found == FALSE
		    && ad->aparse->x400_local) {
			if (tb_k2val (User_auth,
				      ad->aparse->x400_local,
				      buf, TRUE) != NOTOK) {
				uname = ad->aparse->x400_local;
				found = TRUE;
			}
		}
	}
	if (found == FALSE) {
		PP_TRACE(("no authorisation entry found for user '%s'",
			ad->ad_value));
		return OK;
	}
		
	PP_DBG (("Lib/tb_getauthusr (%s, '%s')", uname, buf));


	if ((argc = sstr2arg (buf, MAX_AUTH_ARGS, argv, " \t,")) == NOTOK)
		return OK;

	ousr -> found = TRUE;


	for (i = 0; i < argc; i++) {
		if (!isstr(argv[i])) 
			continue;

		if ((p = index (argv[i], '=')) == NULLCP) {
			PP_OPER(NULLCP,
				("Lib/tb_getauthusr error %d %s",
				 retval, argv[i]));
			return NOTOK;

		}

		/* -- all of form xxx=yyy  -- */
		*p++ = '\0';

		PP_DBG (("Lib/tb_getauthusr %s = %s", argv[i], p));
			
		switch (retval = cmd_srch (argv[i], authtbl_user)) {
		    case AUTH_USER_DEFAULT:
			if ((rights = tb_authrights (p)) != NOTOK)
				ousr -> rights = rights;
			break;
		    case AUTH_USER_CONTENTEX:
			add_cont_excludes (&ousr -> content_excludes, 
					   p);
			break;
		    case AUTH_USER_SIZELIMIT:
			ousr -> sizelimit = atol (p);
			break;

		    default:
			if ((rights = tb_authrights (p)) != NOTOK) 
				if ((c = ch_nm2struct (argv[i])) != NULLCHAN)
					add_chanrights (&ousr -> li_cr,
							c, rights);
			break;
		}
	}

	return OK;
}




/* --------------------- Static  Routines ----------------------------------- */




static void tb_getauthchan_aux (str, ochan)
char			*str;
LIST_AUTH_CHAN		*ochan;
{
	int		argc;
	char		*argv[MAX_AUTH_ARGS];
	char		buf[BUFSIZ];

 	PP_DBG (("Lib/tb_getauthchan_aux: try key %s", str));

	if (tb_k2val (Chan_auth, str, buf, TRUE) == NOTOK)
		return;

	if ((argc = sstr2arg (buf, MAX_AUTH_ARGS, argv, " \t,")) == NOTOK)
		return;

	ochan  -> li_found = TRUE;
	(void) tb_parse_authchan (ochan, argc, argv);

	return;
}




static void tb_parse_authmta (omta, argc, argv)
LIST_AUTH_MTA	*omta;
int		argc;
char		**argv;
{
	int	i, retval = NOTOK, rights;
	char	*p;
	CHAN	*c;

	for (i = 0; i < argc; i++) {
		if (!isstr(argv[i]))
			continue;

		if ((p = index (argv[i], '=')) == NULLCP) {
			PP_OPER(NULLCP,
				("Lib/tb_parse_authchan error %d %s",
				 retval, argv[i]));
			return;
		}


		/* -- all of form xxx=yyy -- */

		*p++ = '\0';

		PP_DBG (("Lib/tb_getauthmta %s = %s", argv[i], p));

		switch (retval = cmd_srch (argv[i], authtbl_mta)) {

		    case AUTH_MTA_DEFAULT:
			if ((rights = tb_authrights (p)) != NOTOK)
				omta -> rights = rights;
			break;
		    case AUTH_MTA_REQUIRES:
			add_regex (&omta -> requires, p);
			break;
		    case AUTH_MTA_EXCLUDES:
			add_regex (&omta -> excludes, p);
			break;
		    case AUTH_MTA_CONTENTEX:
			add_cont_excludes (&omta -> content_excludes, 
					   p);
			break;
		    case AUTH_MTA_SIZELIMIT:
			omta -> sizelimit = atol (p);
			break;

		    default:
			if ((rights = tb_authrights (p)) != NOTOK)
				if ((c = ch_nm2struct (argv[i])) != NULLCHAN) 
					add_chanrights (&omta -> li_cr,
							c, rights);
			break;
		}
	}
}




static int tb_authrights (cp)
char	*cp;
{
	int	retval;

	retval = cmd_srch (cp, authtbl_rights);

	if (retval == NOTOK)
		PP_LOG (LLOG_NOTICE, ("Lib/tb_authrights not known <%s>", cp));

	PP_DBG (("Lib/tb_authrights for %s returns %d", cp, retval));
	return retval;
}




static void add_regex (lp, str)
LIST_REGEX	**lp;
char		*str;	/* 1 or more regular expressions */
{
	char	*cp;


	while (str) {
		cp = str;
		if (str = index (str, AUTH_SEPERATOR))
			*str++ = '\0';
		list_regex_add (lp, list_regex_new (cp));
	}
}




static void list_regex_add (list, item)	
LIST_REGEX	**list;
LIST_REGEX	*item;
{
	LIST_REGEX	*lp = *list;

	if (lp == NULLIST_REGEX) {
		*list = item;
		return;
	}

	while (lp -> li_next != NULLIST_REGEX)
		lp = lp -> li_next;
	lp -> li_next = item;
}



static LIST_REGEX  *list_regex_new (value)	
char	*value;
{
	LIST_REGEX	*lp;

	if (value == NULLCP)	return (NULLIST_REGEX);

	lp = (LIST_REGEX *) malloc (sizeof *lp);
	bzero ((char *)lp, sizeof *lp);
	lp -> li_regex = strdup (value);

	return (lp);
}




static void  add_cont_excludes (lp, str)
LIST_BPT	**lp;
char		*str;
{
	char	*cp;

	while (str) {
		cp = str;
		if ((str = index (str, AUTH_SEPERATOR)))
			*str++ = '\0';
		list_bpt_add (lp, list_bpt_new (cp));
	}
}




static void add_chanrights (list, chan, rights)
LIST_CHAN_RIGHTS	**list;
CHAN			*chan;
int			rights;
{
	list_chan_rights_add (list, list_chan_rights_new (chan, rights));
}




static void list_chan_rights_add (list, item)
LIST_CHAN_RIGHTS	**list;
LIST_CHAN_RIGHTS	*item;
{
	LIST_CHAN_RIGHTS	*lp = *list;

	if (lp == NULLIST_CHAN_RIGHTS) {
		*list = item;
		return;
	}

	while (lp -> li_next != NULLIST_CHAN_RIGHTS)
		lp = lp -> li_next;
	lp -> li_next = item;
}



static LIST_CHAN_RIGHTS	*list_chan_rights_new (chan, rights)	
CHAN		*chan;
int		rights;
{
	LIST_CHAN_RIGHTS	*lp;

	if (chan == NULLCHAN)	return (NULLIST_CHAN_RIGHTS);

	lp = (LIST_CHAN_RIGHTS *) malloc (sizeof *lp);
	bzero ((char *)lp, sizeof *lp);
	lp -> li_rights	= AUTH_RIGHTS_UNSET;
	lp -> li_chan 	= chan;
	lp -> li_rights	= rights;

	return (lp);
}
