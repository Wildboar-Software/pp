/* drive.c: main routine for line based console */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/drive.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/drive.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: drive.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */
#include	"console.h"

extern Command get_command();
extern int	connected, info_shown;
extern char	*host;

int	total_volume, total_number_messages, total_number_reports;
int	assocD = -1;
time_t	currentTime;
Command	comm = Unknown;

main(argc, argv)
int	argc;
char	**argv;
{
	init(argc, argv);
	top_level_menu();
	QuitProc();
}

#define ARG_QUEUE	1
#define	ARG_USER	2
#define ARG_AUTHORISED	3

int bypassHostQuestions = FALSE;
extern char	*username;
extern int	authorised;

static CMD_TABLE	tb_args [] = { /* all possible cmdline args */
	"-queue",	ARG_QUEUE,
	"-queu",	ARG_QUEUE,
	"-que",		ARG_QUEUE,
	"-qu",		ARG_QUEUE,
	"-q",		ARG_QUEUE,
	
	"-user",	ARG_USER,
	"-use",		ARG_USER,
	"-us",		ARG_USER,
	"-u",		ARG_USER,

	"-authorised",	ARG_AUTHORISED,
	"-authorise",	ARG_AUTHORISED,
	"-authoris",	ARG_AUTHORISED,
	"-authori",	ARG_AUTHORISED,
	"-author",	ARG_AUTHORISED,
	"-autho",	ARG_AUTHORISED,
	"-auth",	ARG_AUTHORISED,
	"-aut",		ARG_AUTHORISED,
	"-au",		ARG_AUTHORISED,
	"-a",		ARG_AUTHORISED,

	0,	-1
	};

init(argc, argv)
int	argc;
char	**argv;
{
	int	i;
	isodetailor(argv[0], 1);
	init_host();
	init_pager();
	for (i = 1; i < argc; i++) {
		switch(cmd_srch(argv[i], tb_args)) {
		    case ARG_QUEUE:
			if (++i >= argc)
				fprintf(stderr, 
					"No hostname given with flag '%s'\n",
					argv[i-1]);
			else {
				if (host) free(host);
				host = strdup(argv[i]);
				comm = Connect;
				bypassHostQuestions = TRUE;
			}
			break;
		    case ARG_USER:
			if (++i >= argc)
				fprintf(stderr, 
					"No username given with flag '%s'\n",
					argv[i-1]);
			else {
				if (username) free(username);
				username = strdup(argv[i]);
			}
			break;
			
		    case ARG_AUTHORISED:
			authorised = TRUE;
			break;

		    default:
			fprintf(stderr,
				"Unknown flag '%s'\n",
				argv[i]);
		}
	}
	init_assoc();
}

extern char	prompt[], *level2str();
extern int	compat;
Level	lev = top;

top_level_menu()
{
	reset_prompt_regex();
	if (comm == Unknown)
		comm = get_command(top);
	while (comm != Quit) {
		switch (comm) {
		    case Connect:
			set_host();
			ConnectProc();
			if (connected == TRUE) {
				if (!compat)
					my_invoke(qmgrStatus, (char **) NULL);
				channel_level_menu();
				lev = top;
				reset_prompt_regex();
			}
			break;
		    case Disconnect:
			DisconnectProc();
			comm = Unknown;
			break;
		    case Heuristics:
			heuristics_menu();
			break;
		    case Status:
			print_status();
			break;
		    case Set:
			variables();
			break;
		    default:
			fprintf(stderr,
				"Unknown command '%s' in top level menu\n",
				command2str(comm));
			break;
		}
		if (comm != Quit
		    && comm != Disconnect)
			comm = get_command(top);
		
	}
}

channel_level_menu()
{
	
	int	cont = TRUE;
	lev = channel;
	reset_prompt_regex();
	comm = List; /* get_command(channel);*/
	info_shown = FALSE;

	while (cont == TRUE) {
		switch (comm) {
		    case Disconnect:
		    case Quit:
			cont = FALSE;
			break;
		    case Heuristics:
			heuristics_menu();
			break;
		    case Status:
			print_status();
			break;
		    case Set:
			variables();
			break;
		    case Refresh:
			channel_do_refresh();
			break;
		    case Quecontrol:
			Qcontrol();
			break;
		    case Current:
			if (channel_set_current() == OK)
				reset_prompt_regex();
			break;
		    case Down:
			if (channel_set_current() == OK) {
				mta_level_menu();
				lev = channel;
				reset_prompt_regex();
			}
			break;
		    case Next:
			channel_next();
			reset_prompt_regex();
			channel_info_regex();
			break;
		    case Previous:
			channel_previous();
			reset_prompt_regex();
			channel_info_regex();
			break;
		    case Info:
			if (channel_set_current_regex() == OK) {
				reset_prompt_regex();
				channel_info_regex();
			}
			break;
		    case List:
			channel_list();
			break;
		    case Enable:
			if (channel_set_current_regex() == OK) {
				reset_prompt_regex();
				channel_enable_regex();
			}
			break;
		    case Disable:
			if (channel_set_current_regex() == OK) {
				reset_prompt_regex();
				channel_disable_regex();
			}
			break;
		    case Delay:
			if (channel_set_current_regex() == OK) {
				reset_prompt_regex();
				channel_delay_regex();
			}
			break;
		    case Clear:
			if (channel_set_current_regex() == OK) {
				reset_prompt_regex();
				channel_clear_regex();
			}
			break;
		    case BelowClear:
			if (channel_set_current_regex() == OK) {
				reset_prompt_regex();
				channel_below_clear_regex();
			}
			break;
		    default:
			fprintf(stderr,
				"Unknown command '%s'  in channel level menu\n",
				command2str(comm));
			break;
		}
		if (cont == TRUE) {
			if (comm != Quit
			    && comm != Disconnect)
				comm = get_command(channel);
			else
				cont = FALSE;
		}
	}
	clear_channel_level();
}

mta_level_menu()
{
	
	int	cont = TRUE;
	lev = mta;
	reset_prompt_regex();
	comm = List; /* get_command(mta);*/
	info_shown = FALSE;

	while (cont == TRUE) {
		switch (comm) {
		    case Disconnect:
		    case Quit:
		    case Up:
			cont = FALSE;
			break;
		    case Heuristics:
			heuristics_menu();
			break;
		    case Status:
			print_status();
			break;
		    case Set:
			variables();
			break;
		    case Current:
			if (mta_set_current() == OK)
				reset_prompt_regex();
			break;
		    case Refresh:
			mta_do_refresh();
			break;
		    case Down:
			if (mta_set_current() == OK) {
				msg_level_menu();
				lev = mta;
				reset_prompt_regex();
			}
			break;
		    case Next:
			mta_next();
			reset_prompt_regex();
			mta_info_regex();
			break;
		    case Previous:
			mta_previous();
			reset_prompt_regex();
			mta_info_regex();
			break;
		    case Info:
			if (mta_set_current_regex() == OK) {
				reset_prompt_regex();
				mta_info_regex();
			}
			break;
		    case List:
			mta_list();
			break;
		    case Enable:
			if (mta_set_current_regex() == OK) {
				reset_prompt_regex();
				mta_enable_regex();
			}
			break;
		    case Disable:
			if (mta_set_current_regex() == OK) {
				reset_prompt_regex();
				mta_disable_regex();
			}
			break;
		    case Delay:
			if (mta_set_current_regex() == OK) {
				reset_prompt_regex();
				mta_delay_regex();
			}
			break;
		    case Clear:
			if (mta_set_current_regex() == OK) {
				reset_prompt_regex();
				mta_clear_regex();
			}
			break;
		    case BelowClear:
			if (mta_set_current_regex() == OK) {
				reset_prompt_regex();
				mta_clear_below_regex();
			}
			break;
		    case AboveClear:
			if (mta_set_current_regex() == OK) {
				reset_prompt_regex();
				mta_clear_above_regex();
			}
			break;
		    default:
			fprintf(stderr,
				"Unknown command '%s'  in mta level menu\n",
				command2str(comm));
			break;
		}
		if (cont == TRUE) {
			if (comm != Quit
			    && comm != Disconnect)
				comm = get_command(mta);
			else
				cont = FALSE;
		}
	}
	clear_mta_level();
}

msg_level_menu()
{
	
	int	cont = TRUE;
	lev = msg;
	reset_prompt_regex();
	comm = List; /* get_command(msg); */
	info_shown = FALSE;

	while (cont == TRUE) {
		switch (comm) {
		    case Disconnect:
		    case Quit:
		    case Up:
			cont = FALSE;
			break;
		    case Next:
			msg_next();
			reset_prompt_regex();
			msg_info_regex();
			break;
		    case Previous:
			msg_previous();
			reset_prompt_regex();
			msg_info_regex();
			break;
		    case Heuristics:
			heuristics_menu();
			break;
		    case Status:
			print_status();
			break;
		    case Set:
			variables();
			break;
		    case Refresh:
			msg_do_refresh();
			break;
		    case Current:
			if (msg_set_current() == OK)
				reset_prompt_regex();
			break;
		    case Info:
			if (msg_set_current_regex() == OK) {
				reset_prompt_regex();
				msg_info_regex();
			}
			break;
		    case List:
			msg_list();
			break;
		    case Enable:
			if (msg_set_current_regex() == OK) {
				reset_prompt_regex();
				msg_enable_regex();
			}
			break;
		    case Disable:
			if (msg_set_current_regex() == OK) {
				reset_prompt_regex();
				msg_disable_regex();
			}
			break;
		    case Delay:
			if (msg_set_current_regex() == OK) {
				reset_prompt_regex();
				msg_delay_regex();
			}
			break;
		    case Clear:
			if (msg_set_current_regex() == OK) {
				reset_prompt_regex();
				msg_clear_regex();
			}
			break;
		    case AboveClear:
			if (msg_set_current_regex() == OK) {
				reset_prompt_regex();
				msg_clear_above_regex();
			}
			break;
		    default:
			fprintf(stderr,
				"Unknown command '%s'  in msg level menu\n",
				command2str(comm));
			break;
		}
		if (cont == TRUE) {
			if (comm != Quit
			    && comm != Disconnect)
				comm = get_command(msg);
			else
				cont = FALSE;
		}
	}
	clear_msg_level();
}

