/* io.c: i/o routines for linebased console */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/io.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/io.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: io.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */
#include	"console.h"
#include	<signal.h>

/*  */
/* paging routines */

char 	*cmd_argv[100];
int	cmd_argc;
	
char	*pager;
FILE	*out;
extern FILE *popen ();
SFP     oldpipe;
int	pipeopen, pageron, fileopen;

init_pager()
{
	if ((pager = getenv("PAGER")) == NULL)
		pager = strdup("more");
	else 
		pager = strdup(pager);
	pageron = TRUE;
}

/* ARGSUSED */
static void piped(sig)
int sig;
{
        pipeopen = 0;
}

void openpager()
{
	FILE	*fp;
	fileopen = FALSE;
	if (cmd_argc > 1) {
		int	i;
		for (i = 1; i < cmd_argc; i++) {
			if (lexequ(cmd_argv[i], ">") == 0
			    || lexequ(cmd_argv[i], ">>") == 0) {
				int	append = (lexequ(cmd_argv[i], ">") == 0) ? FALSE : TRUE;
				i++;
				if (i >= cmd_argc)
					fprintf(stderr,
						"redirect argument (%s) given but no destination specified\n",
						cmd_argv[i-1]);
				else if ((fp = fopen(cmd_argv[i], 
						     (append == TRUE) ? "a" : "w")) == NULL)
					fprintf(stderr,
						"unable to open redirect destination '%s' for %s\n",
						cmd_argv[i],
						(append == TRUE) ? "appending" : "writing");
				else {
					fileopen = TRUE;
					out = fp;
					i = cmd_argc;
				}
			} 
		}
	}
	
	if (fileopen == TRUE)
		return;

	if (pageron == TRUE) {
		oldpipe = signal(SIGPIPE, piped);
		if (isatty(1) == 0
		    || pager == NULLCP)
			out = stdout;
		else if ((out = popen(pager,"w")) == NULL) {
			fprintf(stderr, "unable to start pager '%s': %s\n",
				pager, sys_errname (errno));
			out = stdout;
		}
		pipeopen = 1;
	} else
		out = stdout;
}

void closepager()
{
        if (out != stdout) {
		if (fileopen == TRUE)
			fclose(out);
		else
			pclose(out);
                out = stdout;
        }
        pipeopen = 0;
	if (pageron == TRUE)
		signal(SIGPIPE, oldpipe);
}

/*  */
/* routine to read in commands */
static CMD_TABLE	tb_commands [] = { /* all possible commands */
	"aboveclear",	(int) AboveClear,
	"aboveclea",	(int) AboveClear,
	"abovecle",	(int) AboveClear,
	"abovecl",	(int) AboveClear,
	"abovec",	(int) AboveClear,
	"above",	(int) AboveClear,
	"abov",		(int) AboveClear,
	"abo",		(int) AboveClear,
	"ab",		(int) AboveClear,
	"a",		(int) AboveClear,

	"belowclear",	(int) BelowClear,
	"belowclea",	(int) BelowClear,
	"belowcle",	(int) BelowClear,
	"belowcl",	(int) BelowClear,
	"belowc",	(int) BelowClear,
	"below",	(int) BelowClear,
	"belo",		(int) BelowClear,
	"bel",		(int) BelowClear,
	"be",		(int) BelowClear,
	"b",		(int) BelowClear,


	"clear",	(int) Clear,
	"clea",		(int) Clear,
	"cle",		(int) Clear,
	"cl",		(int) Clear,

	"connect",	(int) Connect,
	"connec",	(int) Connect,
	"conne",	(int) Connect,
	"conn",		(int) Connect,
	"con",		(int) Connect,
	"co",		(int) Connect,

	"current",	(int) Current,
	"curren",	(int) Current,
	"curre",	(int) Current,
	"curr",		(int) Current,
	"cur",		(int) Current,
	"cu",		(int) Current,

	"c",		(int) Ambiguous,

	"delay",	(int) Delay,
	"dela",		(int) Delay,
	"del",		(int) Delay,
	"de",		(int) Delay,

	"disable",	(int) Disable,
	"disabl",	(int) Disable,
	"disab",	(int) Disable,
	"disa",		(int) Disable,
	
	"disconnect", 	(int) Disconnect,
	"disconnec", 	(int) Disconnect,
	"disconne", 	(int) Disconnect,
	"disconn", 	(int) Disconnect,
	"discon", 	(int) Disconnect,
	"disco", 	(int) Disconnect,
	"disc", 	(int) Disconnect,

	"dis",		(int) Ambiguous,

	"down",		(int) Down,
	"dow",		(int) Down,
	"do",		(int) Down,
	
	"d",		(int) Ambiguous,

	"enable",	(int) Enable,
	"enabl",	(int) Enable,
	"enab",		(int) Enable,
	"ena",		(int) Enable,
	"en",		(int) Enable,
	"e",		(int) Enable,

	"heuristics",	(int) Heuristics,
	"heuristic",	(int) Heuristics,
	"heuristi",	(int) Heuristics,
	"heurist",	(int) Heuristics,
	"heuris",	(int) Heuristics,
	"heuri",	(int) Heuristics,
	"heur",		(int) Heuristics,
	"heu",		(int) Heuristics,
	"he",		(int) Heuristics,
	"h",		(int) Heuristics,

	"informatio",	(int) Info,
	"informati",	(int) Info,
	"informat",	(int) Info,
	"informa",	(int) Info,
	"inform",	(int) Info,
	"infor",	(int) Info,
	"info",		(int) Info,
	"inf",		(int) Info,
	"in",		(int) Info,
	"i",		(int) Info,


	"list",		(int) List,
	"lis",		(int) List,
	"li",		(int) List,
	"l",		(int) List,

	"next",		(int) Next,
	"nex",		(int) Next,
	"ne",		(int) Next,
	"n",		(int) Next,

	"previous",	(int) Previous,
	"previou",	(int) Previous,
	"previo",	(int) Previous,
	"previ",	(int) Previous,
	"prev",		(int) Previous,
	"pre",		(int) Previous,
	"pr",		(int) Previous,
	"p",		(int) Previous,

	"quecontrol",	(int) Quecontrol,
	"quecontro",	(int) Quecontrol,
	"quecontr",	(int) Quecontrol,
	"quecont",	(int) Quecontrol,
	"quecon",	(int) Quecontrol,
	"queco",	(int) Quecontrol,
	"quec",		(int) Quecontrol,
	"que",		(int) Quecontrol,

	
	"quit",		(int) Quit,
	"qui",		(int) Quit,
	
	"qu",		(int) Ambiguous,
	"q",		(int) Ambiguous,

	"refresh",	(int) Refresh,
	"refres",	(int) Refresh,
	"refre",	(int) Refresh,
	"refr",		(int) Refresh,
	"ref",		(int) Refresh,
	"re",		(int) Refresh,
	"r",		(int) Refresh,
	
	"status",	(int) Status,
	"statu",	(int) Status,
	"stat",		(int) Status,
	"sta",		(int) Status,
	"st",		(int) Status,

	"set",		(int) Set,
	"se",		(int) Set,

	"s",		(int) Ambiguous,

	"up",		(int) Up,
	"u",		(int) Up,

	"unknown",	(int) Unknown,
	0,		-1
	};

Command str2command();
char	*level2str();
void	command_options();
extern int	connected;
extern int	authorised;
extern char	*host;
extern Command	comm;
extern Level	lev;
extern struct chan_struct	*currentchan;
extern struct mta_struct	*currentmta;
char	prompt[BUFSIZ];

extern char	*chan_match, *mta_match, *msg_match;

reset_prompt_regex()
{
	switch(lev) {
	    case top:
		sprintf(prompt, "%s", level2str(lev));
		break;
	    case channel:
		sprintf(prompt, "%s", 
			(chan_match) ? chan_match : level2str(lev));
		break;
	    case mta:
		sprintf(prompt, "%s.%s",
			(currentchan) ? currentchan->channelname : level2str(channel),
			(mta_match) ? mta_match : level2str(lev));
		break;
	    case msg:
		sprintf(prompt, "%s.%s.%s",
			(currentchan) ? currentchan->channelname : level2str(channel),
			(currentmta) ? currentmta->mta : level2str(mta),
			(msg_match) ? msg_match : level2str(lev));
		break;
	}
}

int	info_shown;

static void set_info(com)
Command	com;
{
	switch(com) {
	    case Info:
	    case Next:
	    case Previous:
		info_shown = TRUE;
	    case Refresh:
	    case Heuristics:
	    case Status:
		break;
	    default:
		info_shown = FALSE;
		break;
	}
}

Command	get_command(lev)
Level	lev;
{
	Command	ret = Unknown;
	int	cont = TRUE;
	static  char	buf[BUFSIZ];
	cmd_argc = 0;
	while (cont == TRUE) {
		fprintf(stdout, "%s> ", prompt);
		fflush (stdout);
		
		if (gets(buf) == NULL)
			exit(OK);
		(void) compress(buf, buf);
		
		cmd_argc = sstr2arg(buf, 100, cmd_argv, " \t");

		if (cmd_argc == 0) {
			/* use same command as before */
			ret = comm;
			cont = FALSE;
		} else if ((ret = str2command(cmd_argv[0])) == Unknown) {
			fprintf(stdout,
				"Unknown command '%s'\n", cmd_argv[0]);
			command_options(lev);
		} else if (ret == Ambiguous) {
			fprintf(stdout,
				"Ambiguous command '%s'\n", cmd_argv[0]);
			command_options(lev);
			ret = Unknown;
		} else if (check_command(ret, lev) == NOTOK) {
			fprintf(stdout,
				"Command '%s' unavailable at '%s' level\n",
				command2str(ret), level2str(lev));
			command_options(lev);
		} else if (ret == Connect && connected == TRUE) 
			fprintf(stdout,
				"Already connected (to %s)\n", host);
		else if (ret == Disconnect && connected == FALSE)
			fprintf(stdout,
				"Already unconnected\n");
		else
			cont = FALSE;
	}
	set_info(ret);
	return ret;
}

/*  */
/* command recognition routines */

Command	str2command(str)
char	*str;
{
	Command	ret;
	
	if ((ret = (Command) cmd_srch(str, tb_commands)) == -1)
		ret = Unknown;
	return ret;
}

char *command2str(com)
Command	com;
{
	return rcmd_srch(com, tb_commands);
}

int check_command(com, lev)
Command	com;
Level	lev;
{
	switch(lev) {
	    case top:
		switch(com) {
		    case Connect:
		    case Disconnect:
		    case Heuristics:
		    case Quit:
		    case Status:
		    case Set:
			return OK;
		    default:
			return NOTOK;
		}
	    case channel:
		switch(com) {	
		    case Disconnect:
		    case Down:
		    case Heuristics:
		    case Quit:
		    case List:
		    case Current:
		    case Info:
		    case Next:
		    case Previous:
		    case Refresh:
		    case Status:
		    case Set:
			return OK;
		    case Enable:
		    case Disable:
		    case Delay:
		    case Clear:
		    case BelowClear:
		    case Quecontrol:
			if (authorised == TRUE)
				return OK;
		    default:
			return NOTOK;
		}
	    case mta:
		switch(com) {	
		    case Disconnect:
		    case Down:
		    case Up:
		    case Heuristics:
		    case Quit:
		    case List:
		    case Current:
		    case Info:
		    case Next:
		    case Previous:
		    case Refresh:
		    case Status:
		    case Set:
			return OK;
		    case Enable:
		    case Disable:
		    case Delay:
		    case Clear:
		    case BelowClear:
		    case AboveClear:
		    case Quecontrol:
			if (authorised == TRUE)
				return OK;
		    default:
			return NOTOK;
		}
	    case msg:
		switch(com) {	
		    case Disconnect:
		    case Up:
		    case Heuristics:
		    case Quit:
		    case List:
		    case Current:
		    case Info:
		    case Next:
		    case Previous:
		    case Refresh:
		    case Status:
		    case Set:
			return OK;
		    case Enable:
		    case Disable:
		    case Delay:
		    case Clear:
		    case AboveClear:
		    case Quecontrol:
			if (authorised == TRUE)
				return OK;
		    default:
			return NOTOK;
		}
	    default:
		return NOTOK;
	}
}

void command_options(lev)
Level	lev;
{
	fprintf(stdout,
		"Commands available are:");
	switch(lev) {
	    case top:
		fprintf(stdout,
			"connect, disconnect, quit, status, set\n\tand heuristics\n");
		break;
	    case channel:
		fprintf(stdout,
			"disconnect, quit, current, status, set, refresh\n\tdown, next, previous, list, info%sand heuristics\n",
			(authorised == TRUE) ? ",\n\tenable, disable, delay, clear\n\tbelowclear, quecontrol " : " ");
		break;
	    case mta:
		fprintf(stdout,
			"disconnect, quit, current, status, set, refresh\n\tdown, up, next, previous, list, info%sand heuristics\n",
			(authorised == TRUE) ? ",\n\tenable, disable, delay, clear\n\tbelowclear, aboveclear, quecontrol " : " ");
		break;
	    case msg:
		fprintf(stdout,
			"disconnect, quit, current, status, set, refresh\n\tup, next, previous, list, info%sand heuristics\n",
			(authorised == TRUE) ? ",\n\tenable, disable, delay, clear\n\taboveclear, quecontrol " : " ");
		break;
	}
}

/*  */
/* level manipulation routines */

char *level2str(lev)
Level	lev;
{
	switch(lev) {
	    case top:
		return "top";
	    case channel:
		return "channel";
	    case mta:
		return "mta";
	    case msg:
		return "msg";
	}
	return "";
}
