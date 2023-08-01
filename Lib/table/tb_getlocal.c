/* tb_getlocal.c: convert address to user reference */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getlocal.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getlocal.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_getlocal.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "chan.h"
#include 	"loc_user.h"
#include 	<ctype.h>
#include	<pwd.h>
#include <isode/cmd_srch.h>

#define         MAX_USER_ARGS           50
extern char *mboxname, *mailfilter, *sysmailfilter;
extern CMD_TABLE tbl_bool[];

static CMD_TABLE key_tbl[] = {
#define T_UID		1
	"uid",		T_UID,
#define T_GID		2
	"gid",		T_GID,
#define T_USERNAME	3
#define T_START		T_USERNAME	 /* start backwards compat  */
	"username",	T_USERNAME,
#define T_DIRECTORY	4
	"directory",	T_DIRECTORY,
#define T_MAILBOX	5
#define T_FINISH	T_MAILBOX 	/* end backwards compat */
	"mailbox",	T_MAILBOX,
#define T_SHELL		6
	"shell",	T_SHELL,
#define T_HOME		7
	"home",		T_HOME,
#define T_MAILFORMAT	8
	"mailformat",	T_MAILFORMAT,
#define T_RESTRICTED	9
	"restricted",	T_RESTRICTED,
#define T_MAILFILTER	10
	"mailfilter",	T_MAILFILTER,
	"usermailfilter",T_MAILFILTER,
#define T_SYSMAILFILTER	11
	"sysmailfilter",T_SYSMAILFILTER,
#define T_SEARCHPATH	12
	"searchpath",	T_SEARCHPATH,
	"PATH",		T_SEARCHPATH,
#define T_OPTS		13
	"opts",		T_OPTS,
	0, -1
	};

static CMD_TABLE mf_tbl[] = {
	"pp",	MF_PP,
	"mmdf",	MF_PP,
	"sendmail", MF_UNIX,
	"unix",	MF_UNIX,
	"auto",	MF_AUTO,
	0,	-1
};


static char def[] = "default";
static char none[] = "none";
static int parse_entry ();

extern void getfpath ();
/* ---------------------  Begin  Routines  -------------------------------- */

LocUser	*tb_getlocal (key, channel)
char            *key;
CHAN		*channel;
{
	LocUser	*loc;
	char    buf [BUFSIZ];


	PP_DBG (("Lib/tb_getlocal (%s, %s)", key, channel -> ch_name));

	if (channel -> ch_table == NULL) {
		PP_LOG (LLOG_EXCEPTIONS,
 			("Channel %s (%s) doesn't have a table",
			channel -> ch_name, channel -> ch_show));
		return NULL;
	}

	loc = (LocUser *) smalloc (sizeof *loc);
	bzero ((char *)loc, sizeof *loc);
	loc -> restricted = FALSE;
	loc -> mailformat = MF_PP;

	if (tb_k2val (channel -> ch_table, def, buf, TRUE) == OK)
		if (parse_entry (loc, buf, def) == NOTOK) {
			free_loc_user (loc);
			return NULL;
		}
			

	if (lexequ (def, key) == 0)
		return loc;

	if (tb_k2val (channel -> ch_table, key, buf, TRUE) == NOTOK) {
		free_loc_user (loc);
		return NULL;
	}

	if (parse_entry (loc, buf, key) == NOTOK) {
		free_loc_user (loc);
		return NULL;
	}

	/* see if the result is valid */
	if (loc -> directory == NULLCP) {
		if (loc -> home)
			loc -> directory = strdup(loc->home);
		else {
			PP_LOG (LLOG_EXCEPTIONS,
				("No directory or home specified for %s", key));
			free_loc_user (loc);
			return NULL;
		}
	}
	if (loc -> home == NULLCP)
		loc -> home = strdup(loc->directory);

	if (loc->mailfilter == NULLCP)
		loc -> mailfilter = strdup (mailfilter);
	if (lexequ(loc -> mailfilter, none) == 0) {
		free (loc -> mailfilter);
		loc -> mailfilter = NULLCP;
	}
	else if (strcmp (loc->directory, loc -> home) != 0) {
		getfpath (loc -> home, loc -> mailfilter, buf);
		free (loc -> mailfilter);
		loc -> mailfilter = strdup(buf);
	}
	if (loc -> sysmailfilter == NULLCP)
		loc -> sysmailfilter = strdup (sysmailfilter);
	if (lexequ(loc -> sysmailfilter, none) == 0) {
		free (loc -> sysmailfilter);
		loc -> sysmailfilter = NULLCP;
	}

	return loc;
}

void	free_loc_user (loc)
LocUser	*loc;
{
	if (loc -> username)
		free (loc->username);
	if (loc -> directory)
		free (loc -> directory);
	if (loc -> mailbox)
		free (loc -> mailbox);
	if (loc -> shell)
		free (loc -> shell);
	if (loc -> home)
		free (loc -> home);
	if (loc -> mailfilter)
		free (loc -> mailfilter);
	if (loc -> sysmailfilter)
		free (loc -> sysmailfilter);
	if (loc -> searchpath)
		free (loc -> searchpath);
	if (loc -> opts)
		free (loc -> opts);
	free ((char *) loc);
}

static int parse_entry (loc, buf, key)
LocUser *loc;
char	*buf;
char	*key;
{
	int count = T_START - 1;
	int	state;
	int	argc;
	int	val;
	char *argvl[MAX_USER_ARGS], **argv;
	int	seenuid = 0, seengid = 0;
	char	*p;

	if ((argc = sstr2arg (buf, MAX_USER_ARGS, argvl, " \t,")) == NOTOK)
		return (NOTOK);
	
	for(argv = argvl; argc > 0; argc--, argv++) {
		char ** resp = NULLVP;
		
		if (!isstr(*argv))
			continue;

		if ((p = index (*argv, '=')) == NULL) {
			if (++count > T_FINISH) {
				PP_LOG (LLOG_EXCEPTIONS,
					("Info string `%s' for %s not in key=val format",
					 *argv, key));
				return NOTOK;
			}
			state = count;
			p = *argv;
		}
		else {
			*p++ = '\0';
			state = cmd_srch (*argv, key_tbl);
		}
			
		switch (state) {
		    case T_UID:
			loc -> uid = atoi(p);
			seenuid = 1;
			continue;
		    case T_GID:
			loc -> gid = atoi(p);
			seengid = 1;
			continue;
		    case T_USERNAME:
			resp = &(loc -> username);
			break;
		    case T_DIRECTORY:
			resp = &(loc -> directory);
			break;
		    case T_MAILBOX:
			resp = &(loc -> mailbox);
			break;
		    case T_SHELL:
			resp = &(loc -> shell);
			break;
		    case T_HOME:
			resp = &(loc -> home);
			break;
		    case T_MAILFORMAT:
			if ((val = cmd_srch (p, mf_tbl)) != NOTOK)
				loc -> mailformat = val;
			else
				PP_LOG (LLOG_EXCEPTIONS,
					("Bad value for mailformat %s in %s",
					 p, key));
			continue;
		    case T_RESTRICTED:
			if ((val = cmd_srch (p, tbl_bool)) != NOTOK)
				loc -> restricted = val;
			else
				PP_LOG (LLOG_EXCEPTIONS,
					("Bad value for restricted %s in %s",
					 p, key));
			continue;
		    case T_MAILFILTER:
			resp = &(loc -> mailfilter);
			break;
		    case T_SYSMAILFILTER:
			resp = &(loc -> sysmailfilter);
			break;
		    case T_SEARCHPATH:
			resp = &(loc -> searchpath);
			break;
		    case T_OPTS:
			resp = &(loc -> opts);
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown key in user info for %s '%s' (%s)",
				 key, *argv, p));
			continue;
		}
		if (resp)
		{
			if (*resp)
				free (*resp);
			*resp = strdup(p);
		}
	}
	/* start fixing up special cases */
	if (loc -> mailbox == NULLCP)
		loc -> mailbox = strdup (mboxname);


	if (loc -> username) {
		if (isdigit (*loc -> username) &&
		    (p = index (loc -> username, '/'))) {
			*p ++ = NULL;
			loc -> uid = atoi (loc -> username);
			loc -> gid = atoi (p);
			free (loc -> username);
			loc -> username = NULLCP;
		}
		else {
			struct passwd *pwd;

			if ((pwd = getpwnam (loc -> username)) == NULL) {
				PP_LOG (LLOG_EXCEPTIONS, 
					("No such local user %s", loc -> username));
				return NOTOK;
			}

			if (seenuid == 0)
				loc -> uid = pwd -> pw_uid;
			if (seengid == 0)
				loc -> gid = pwd -> pw_gid;
			if (loc -> home == NULLCP)
				loc -> home = strdup (pwd -> pw_dir);
			if (loc -> shell == NULLCP)
				loc -> shell = strdup (pwd -> pw_shell);
			if (loc -> directory == NULLCP)
				loc -> directory = strdup (pwd -> pw_dir);
		}
	}
	return OK;
}
