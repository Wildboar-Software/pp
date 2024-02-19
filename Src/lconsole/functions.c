/* functions.c: things that do the work */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/lconsole/RCS/functions.c,v 6.0 1991/12/18 20:27:16 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/functions.c,v 6.0 1991/12/18 20:27:16 jpo Rel $
 *
 * $Log: functions.c,v $
 * Revision 6.0  1991/12/18  20:27:16  jpo
 * Release 6.0
 *
 */

#include "lconsole.h"
#include "qmgr.h"
#include "Qmgr-ops.h"
#include "Qmgr-types.h"
#include "qmgr-int.h"
#include <isode/cmd_srch.h>
#ifdef  BSD42
#include <sys/ioctl.h>
#endif

#define plural(n,s)	((n) == 1 ? "" : (s))

extern FILE *fp_in = NULL;

extern int	chan_update ();
extern int	mta_update ();
extern int	msglist_update ();

extern int	f_channel ();
extern int	f_mta ();
extern int	f_msg ();
extern int	f_show ();

static int	f_clear ();
static int      f_close ();
static int	f_delay ();
static int	f_decrease ();
static int	f_disable ();
static int	f_enable ();
static int	f_help ();
static int	f_increase ();
static int      f_open ();
static int      f_quit ();
static int	f_rereadqueue ();
static int	f_restart ();
static int	f_shutdown ();
static int	f_source ();
static int      f_status ();

extern char *isodeversion, *ppversion;

static struct lc_dispatch disp[] = {
{	"?", 		f_help,		LC_NONE, 
		"give help on a specific command"},
{	"channel",	f_channel,	LC_OPEN,
		"display channels or information about one"},
{	"clear",	f_clear,	LC_AUTH|LC_OPEN,
		"clear the delay on a channel, mta or message"},
{	"close", 	f_close,	LC_OPEN, 
		"close a connection to a queue manager"},
{	"decrease",	f_decrease,	LC_AUTH|LC_OPEN,
		"decrease the number of runnable channels"},
{	"delay",	f_delay,	LC_AUTH|LC_OPEN,
		"delay a channel, mta or message"},
{	"disable",	f_disable,	LC_AUTH|LC_OPEN,
		"disable a channel, mta, message, all or submission"},
{	"enable",	f_enable,	LC_AUTH|LC_OPEN,
		"enable a channel, mta, message, all or submission"},
{	"help", 	f_help,		LC_NONE,
		"give help on a specific command"},
{	"increase",	f_increase,	LC_AUTH|LC_OPEN,
		"increase the number of runnable channels"},
{	"msg",		f_msg,		LC_OPEN,
		"display msgs or information about a message"},
{	"mta",		f_mta,		LC_OPEN,
		"display mtas or information about one mta"},
{	"open", 	f_open,		LC_CLOSE,
		"open a connection to a queue manager" },
{	"quit", 	f_quit,		LC_NONE,
		"terminate the connection and exit" },
{	"rereadqueue",	f_rereadqueue, 	LC_AUTH|LC_OPEN,
		"reread the queue"},
{	"restart", 	f_restart,	LC_AUTH|LC_OPEN,
		"restart the queue manager" },
{	"show",		f_show,		LC_OPEN,
		"show messages meeting certain criteria"},
{	"shutdown",	f_shutdown,	LC_AUTH|LC_OPEN,
		"shutdown the queue manager"},
{	"source",	f_source,	LC_NONE,
		"read a file of commands"},
{	"status",	f_status,	LC_OPEN,
		"print the status of the queue manager"},
{	NULLCP, 	NULLIFP,	LC_NONE,
		NULLCP }
};

struct lc_dispatch *getds (name)
char *name;
{
	register int    longest, nmatches;
	register char  *p, *q;
	char    buffer[BUFSIZ];
	register struct lc_dispatch *ds, *fs;

	longest = nmatches = 0;
	for (ds = disp; p = ds -> name; ds++) {
		for (q = name; *q == *p++; q++)
			if (*q == NULL)
				return ds;
		if (*q == NULL)
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				fs = ds;
			}
			else
				if (q - name == longest)
					nmatches++;
	}


	switch (nmatches) {
	    case 0:
		advise (NULLCP, "unknown operation \"%s\"", name);
		return NULL;

	    case 1:
		return fs;

	    default:
		for (ds = disp, p = buffer; q = ds -> name; ds++)
			if (strncmp (q, name, longest) == 0) {
				(void) sprintf (p, "%s \"%s\"",
						p != buffer ? "," : "", q);
				p += strlen (p);
			}
		advise (NULLCP, "ambiguous operation, it could be one of:%s",
                        buffer);
		return NULL;
	}
}


static int f_open (vec)
char **vec;
{
	char buffer[BUFSIZ];
	char prompt[BUFSIZ];
	BindResult *br;
	fd_set wfds, rfds;

	free_chanlist ();

	if (*++vec == NULLCP) {
		if (_getline ("host: ", buffer) == NOTOK ||
		    str2vec (buffer, vec) < 1)
			return OK;
	}

	if (host) free (host);
	host = strdup (*vec);

	if (*++vec == NULLCP && authenticate) {
		if (user == NULLCP)
			user = strdup ("");
		(void) sprintf (prompt, "user (%s:%s): ", host, user);
		if (_getline (prompt, buffer) == NOTOK)
			return OK;

		if (str2vec (buffer, vec) < 1) {
			authenticate = 0;
			if (user) free (user);
			user = NULLCP;
		}
	}
	else if (*vec && **vec) {
		if (user != NULLCP)
			free (user);
		user = strdup(*vec);
		authenticate = 1;
	}

	if (authenticate && *++vec == NULLCP) {
		(void) sprintf (prompt, "password (%s:%s): ",
				host, user);
		passwd = getpassword (prompt);
		passwd = strdup(passwd);
	}

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	if (verbose) {
		printf ("Connecting to %s...", host);
		(void) fflush(stdout);
	}
	switch (init_connect (host, user, passwd, authenticate == 1 ?
			      type_Qmgr_BindArgument_weakAuthentication :
			      type_Qmgr_BindArgument_noAuthentication,
			      &console_fd, buffer, &br)) {
	    default:
	    case NOTOK:
		if (verbose)
			printf ("failed: %s\n", buffer);
		console_fd = NOTOK;
		return NOTOK;

	    case CONNECTING_1:
		FD_SET (console_fd, &wfds);
		break;

	    case CONNECTING_2:
		FD_SET (console_fd, &rfds);
		break;

	    case DONE:
		break;
	}
	while (1) {
		fd_set ifds, ofds;
		ifds = rfds;
		ofds = wfds;

		if (xselect (console_fd + 1, &ifds, &ofds, NULLFD, 1) <= 0) {
			(void) putchar ('.');
			(void) fflush (stdout);
			continue;
		}
		switch (retry_connect (console_fd, buffer, &br)) {
		    case NOTOK:
			console_fd = NOTOK;
			if (verbose)
				printf ("failed: %s\n", buffer);
			return NOTOK;

		    case CONNECTING_1:
			FD_CLR (console_fd, &rfds);
			FD_SET (console_fd, &wfds);
			continue;

		    case CONNECTING_2:
			FD_CLR (console_fd, &wfds);
			FD_SET (console_fd, &rfds);
			continue;

		    case DONE:
			break;
		}
		break;
	}
	accessmode = br -> access;
	if (realhost) free (realhost);
	realhost = strdup (br -> info);
	if (remoteversion) free (remoteversion);
	remoteversion = strdup(br -> version);
	free_BindResult (br);

	if (verbose)
		printf ("succeeded\n");

	return OK;
}

/* ARGSUSED */
static int f_close (vec)
char **vec;
{
	char errbuf[BUFSIZ];

	if (console_fd == NOTOK)
		return NOTOK;

	if (release_connect(console_fd, errbuf) != DONE)
		return NOTOK;

	free_chanlist ();
	console_fd = NOTOK;
	return OK;
}


/* ARGSUSED */
static int status_update (st, id)
QmgrStatus *st;
{
	time_t now;
	time_t secs;

	(void) time (&now);
	secs = now - st -> boottime;

	printf ("Status at %s", ctime (&now));
	printf ("\tConnected to qmgr on %s\n",realhost);
	printf ("\tversion %s\n", remoteversion);
	printf ("\trunning since %s", ctime(&st->boottime));
	if (accessmode == FULL_ACCESS)
		printf ("\tfull access rights\n");

	printf ("\nSummary\n");
	printf ("Channels: %d channel%s running (maximum of %d)\n",
		st -> currChans, plural(st -> currChans,"s"),
		st -> maxChans);
	printf ("Messages: %s Messages, ", datasize(st -> totalMsgs,""));
	printf ("%s Delivery Reports, ", datasize(st -> totalDRs, "")),
	printf ("Volume %s\n", datasize(st -> totalVolume, "b"));
	printf ("Load: %.2lf ops/sec %.2lf runnable chans\n",
		(double)st -> opsPerSec/100.0,
		(double)st -> runnableChans / 100.0);
	printf ("%5s %.2lf msgs in/s %.2f msgs out/s\n", "",
		(double) st -> msgsInPerSec / 100.0,
		(double) st -> msgsOutPerSec / 100.0);
	printf ("\n%-10s %10s %10s %10s %10s\n", "", "Total In", "Avrg In/s",
		"Total Out", "Avrg Out/s");
	printf ("%-10s %10s %10.2f ",
		"Messages",
		datasize(st->messagesIn,""),
		(double)st -> messagesIn / (double)secs);
	printf ("%10s %10.2lf\n",
		datasize(st -> messagesOut, ""),
		(double) st -> messagesOut / (double)secs);
	printf ("%-10s %10s %10.2lf ",
		"Addresses",
		datasize(st -> addrIn,""),
		(double)st -> addrIn / (double)secs);
	printf ("%10s %10.2lf\n",
		datasize(st -> addrOut, ""),
		(double)st -> addrOut / (double)secs);
	free_QmgrStatus (st);
	return OK;
}

/* ARGSUSED */
static int f_status (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	int pid;

	if (initiate_op (console_fd, operation_Qmgr_qmgrStatus,
			 NULLVP, &pid, status_update, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}

	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	return OK;
}

/* ARGSUSED */
static int f_increase (vec)
char **vec;
{
	char *av[2];
	int pid;
	char errbuf[BUFSIZ];

	av[1] = NULLCP;
	av[0] = QCTRL_INCMAX;
	if (initiate_op (console_fd, operation_Qmgr_qmgrControl,
			 av, &pid, NULLIFP, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}
	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	return OK;
}

/* ARGSUSED */
static int f_decrease (vec)
char **vec;
{
	char *av[2];
	int pid;
	char errbuf[BUFSIZ];

	av[1] = NULLCP;
	av[0] = QCTRL_DECMAX;
	if (initiate_op (console_fd, operation_Qmgr_qmgrControl,
			 av, &pid, NULLIFP, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}
	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	return OK;
}

static int f_quit (vec)
char **vec;
{
	if (console_fd != NOTOK)
		(void) f_close (vec);
	return DONE;
}


void display_status (st)
ProcStatus *st;
{
	if (st -> enabled == 0)
		printf ("%*sStatus: disabled\n", indent, "");

	if (st -> lastAttempt)
		printf ("%*sLast attempted delivery: %s",
			indent * 2, "",
			ctime(&st -> lastAttempt));

	if (st -> lastSuccess)
		printf ("%*sLast successful delivery: %s",
			indent * 2, "",
			ctime(&st->lastSuccess));

	if (st -> cachedUntil)
		printf ("%*sDelayed until %s", indent * 2, "",
			ctime (&st -> cachedUntil));
}


#ifndef TIOCGWINSZ
/* ARGSUSED */
#endif
int     ncols (fp)
FILE *fp;
{
#ifdef  TIOCGWINSZ
	int     i;
	struct winsize ws;

	if (ioctl (fileno (fp), TIOCGWINSZ, (char *) &ws) != NOTOK
            && (i = ws.ws_col) > 0)
		return i;
#endif

	return 80;
}


static int f_help (vec)
char **vec;
{
	register int    i, j, w;
	int     columns, width, lines;
	register struct lc_dispatch   *ds, *es; 

	for (width = 0,es = disp; es -> name; es++) {
		if ((w = strlen(es->name)) > width)
			width = w;
	}
	
	if (*++vec == NULL) {
		if ((columns = ncols (stdout) /
		     (width = (width + 8) & ~7)) == 0)
			columns = 1;
		lines = ((es - disp) + columns - 1) / columns;

		printf ("Operations:\n");
		for (i = 0; i < lines; i++)
			for (j = 0; j < columns; j++) {
				ds = disp + j * lines + i;
				printf ("%s", ds -> name);
				if (ds + lines >= es) {
					(void) putchar ('\n');
					break;
				}
				for (w = strlen (ds -> name);
				     w < width; w = (w + 8) & ~7)
					(void) putchar ('\t');
			}
		printf ("\nversion info:\t%s\n\t\t%s\n",
			ppversion, isodeversion);

		return OK;
	}

	for (; *vec; vec++)
		if (strcmp (*vec, "?") == 0) {
			for (ds = disp; ds -> name; ds++)
				printf ("%-*s\t- %s\n", width,
					ds -> name, ds -> help);

			break;
		}
		else
			if (ds = getds (*vec))
				printf ("%-*s\t- %s\n",
					width, ds -> name, ds -> help);

	return OK;
}

/* ARGSUSED */
static int f_restart (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	int pid;

	vec[0] = QCTRL_RESTART;
	if (initiate_op (console_fd, operation_Qmgr_qmgrControl,
			 vec, &pid, NULLIFP, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}

	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	return OK;
}

static int f_shutdown (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	int pid;
	char *av[2];

	av[1] = NULLCP;
	av[0] = QCTRL_GRACEFUL;
	if (*++vec != NULLCP) {
		if (strncmp(*vec, "abort", strlen(*vec)) == 0)
			vec[0] = QCTRL_ABORT;
	}

	if (initiate_op (console_fd, operation_Qmgr_qmgrControl,
			 av, &pid, NULLIFP, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}

	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	return OK;
}


/* ARGSUSED */
static int f_rereadqueue (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	int pid;
	char *av[2];

	av[0] = QCTRL_REREAD;
	av[1] = NULLCP;
	if (initiate_op (console_fd, operation_Qmgr_qmgrControl,
			 av, &pid, NULLIFP, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}

	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	return OK;
}

int tb_match (name, tbl)
char	*name;
CMD_TABLE *tbl;
{
	int longest, nmatches;
	CMD_TABLE *tp, *stp;
	char *q, *p;

	longest = nmatches = 0;
	for (tp = tbl; p = tp -> cmd_key; tp ++) {
		for (q = name; *q == *p++; q++)
			if (*q == NULL)
				return tp -> cmd_value;
		if (*q == NULL)
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				stp = tp;
			}
			else
				if (q - name == longest)
					nmatches++;
	}
	if (nmatches == 0)
		return TB_NONE;
	if (nmatches == 1)
		return stp -> cmd_value;
	return TB_AMBIG;
}

static CMD_TABLE de_tbl[] = {
#define DE_ALL	1
	"all",	DE_ALL,
#define DE_SUBMISSION 2
	"submission", DE_SUBMISSION,
#define DE_MESSAGE 3
	"message", DE_MESSAGE,
#define DE_MTA	4
	"mta",	DE_MTA,
#define DE_CHANNEL	5
	"channel", DE_CHANNEL,
	NULL
	};

#define OP_DISABLE 	1
#define OP_ENABLE  	2
#define OP_CLEAR	3

static int de_generic (vec, type, str, cstr)
char **vec;
int type;
char *str, *cstr;
{
	int style;
	char buffer[BUFSIZ], buffer2[BUFSIZ];
	char *av[NVARGS];
	int pid;
	char errbuf[BUFSIZ], prompt[BUFSIZ];

	switch (style = tb_match (*vec, de_tbl)) {
	    case TB_NONE:
		advise (NULLCP, "%s is not a known action", *vec);
		return NOTOK;
	    case TB_AMBIG:
		advise (NULLCP, "'%s' is an ambiguous keyword", *vec);
		return NOTOK;

	    case DE_ALL:
	    case DE_SUBMISSION:
		if (type == OP_DISABLE) {
			av[0] = style == DE_ALL ? QCTRL_DISALL :
				QCTRL_DISSUB;
		}
		else if (type == OP_ENABLE)
			av[0] = style == DE_ALL ? QCTRL_ENAALL :
				QCTRL_ENASUB;
		else {
			advise (NULLCP, "%s is not a known action", *vec);
			return NOTOK;
		}
		
		if (initiate_op (console_fd, operation_Qmgr_qmgrControl,
				 av, &pid, NULLIFP, errbuf) == NOTOK) {
			advise (NULLCP, "status operation failed: %s", errbuf);
			return NOTOK;
		}

		if (result_op (console_fd, &pid, errbuf) == NOTOK) {
			advise (NULLCP, "result_op: %s", errbuf);
			return NOTOK;
		}
		break;

	    case DE_CHANNEL:
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "%s channel: ", str);
			if (_getline (prompt, buffer) == NOTOK ||
			    str2vec(buffer, vec) < 1)
				return OK;
		}
		if ((vec[0] = fullchanname(vec[0])) == NULLCP)
			return OK;
		vec[1] = cstr;
		vec[2] = NULLCP;

		if (initiate_op (console_fd, operation_Qmgr_channelcontrol,
				 vec, &pid, chan_update, errbuf) == NOTOK) {
			advise (NULLCP, "status operation failed: %s", errbuf);
			return NOTOK;
		}

		if (result_op (console_fd, &pid, errbuf) == NOTOK) {
			advise (NULLCP, "result_op: %s", errbuf);
			return NOTOK;
		}
		break;

	    case DE_MTA:
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "channel containing MTA: ");
			if (_getline (prompt, buffer) == NOTOK ||
			    str2vec(buffer, vec) < 1)
				return OK;
		}
		if ((vec[0] = fullchanname(vec[0])) == NULLCP)
			return OK;
		if (vec[1] == NULLCP) {
			(void) sprintf (prompt,
					"%s (on channel %s) MTA: ", str,
					vec[0]); 
			if (_getline (prompt, buffer2) == NOTOK ||
			    str2vec(buffer2, &vec[1]) < 1)
				return OK;
		}

		vec[2] = cstr;
		vec[3] = NULLCP;

		if (initiate_op (console_fd, operation_Qmgr_mtacontrol,
				 vec, &pid, mta_update, errbuf) == NOTOK) {
			advise (NULLCP, "status operation failed: %s", errbuf);
			return NOTOK;
		}

		if (result_op (console_fd, &pid, errbuf) == NOTOK) {
			advise (NULLCP, "result_op: %s", errbuf);
			return NOTOK;
		}
		break;

	    case DE_MESSAGE:
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "%s message qid: ", str);
			if (_getline (prompt, buffer) == NOTOK ||
			    str2vec(buffer, av) < 1)
				return OK;
			vec[1] = NULLCP;
		}
		else av[0] = *vec;
		av[1] = NULLCP;
		av[2] = cstr;
		
		if (*++vec == NULLCP) {
			if (_getline ("list of user numbers: ", buffer2) == NOTOK ||
			    str2vec (buffer2, &av[3]) < 1)
				return OK;
		}
		else {
			int i = 3;
			while(*vec)
				av[i++] = *++vec;
			av[i] = NULLCP;
		}

		if (initiate_op (console_fd, operation_Qmgr_msgcontrol,
				 av, &pid, NULLIFP, errbuf) == NOTOK) {
			advise (NULLCP, "status operation failed: %s", errbuf);
			return NOTOK;
		}

		if (result_op (console_fd, &pid, errbuf) == NOTOK) {
			advise (NULLCP, "result_op: %s", errbuf);
			return NOTOK;
		}
		break;
		
	}
	return OK;
}

static int f_enable (vec)
char **vec;
{
	if (*++vec == NULLCP) {
		advise (NULLCP,
			"enable {all | submission | message [args] | mta [args] | channel [args] }");
		return NOTOK;
	}
	return de_generic (vec, OP_ENABLE, "enable", CTRL_ENABLE);
}

static int f_disable (vec)
char **vec;
{

	if (*++vec == NULLCP) {
		advise (NULLCP,
			"disable {all | submission | message [args] | mta [args] | channel [args] }");
		return NOTOK;
	}

	return de_generic (vec, OP_DISABLE, "disable", CTRL_DISABLE);
}

static int f_clear (vec)
char **vec;
{
	if (*++vec == NULLCP) {
		advise (NULLCP,
			"clear {channel | mta | message}");
		return NOTOK;
	}

	return de_generic (vec, OP_CLEAR, "clear delay", CTRL_CACHECLEAR);
}

static int f_delay (vec)
char **vec;
{
	char buffer[BUFSIZ], buffer2[BUFSIZ];
	char *av[NVARGS];
	int pid;
	char errbuf[BUFSIZ], prompt[BUFSIZ];
	char *str;

	if (*++vec == NULLCP) {
		advise (NULLCP, "Specify one of channel, mta or message");
		return NOTOK;
	}

	str = "delay";
	switch (tb_match (*vec, de_tbl)) {
	    case TB_NONE:
		advise (NULLCP, "%s is not a known action", *vec);
		return NOTOK;
	    case TB_AMBIG:
		advise (NULLCP, "'%s' is an ambiguous keyword", *vec);
		return NOTOK;

	    case DE_CHANNEL:
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "%s channel: ", str);
			if (_getline (prompt, buffer) == NOTOK ||
			    str2vec(buffer, vec) < 1)
				return OK;
		}
		if ((av[0] = fullchanname(*vec)) == NULLCP)
			return OK;
		av[1] = CTRL_CACHEADD;
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "%s channel by: ", str);
			if (_getline (prompt, buffer2) == NOTOK ||
			    str2vec(buffer, vec) < 1)
				return OK;
		}
		av[2] = *vec;
		av[3] = NULLCP;

		if (initiate_op (console_fd, operation_Qmgr_channelcontrol,
				 av, &pid, chan_update, errbuf) == NOTOK) {
			advise (NULLCP, "status operation failed: %s", errbuf);
			return NOTOK;
		}

		if (result_op (console_fd, &pid, errbuf) == NOTOK) {
			advise (NULLCP, "result_op: %s", errbuf);
			return NOTOK;
		}
		break;

	    case DE_MTA:
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "channel containing MTA: ");
			if (_getline (prompt, buffer) == NOTOK ||
			    str2vec(buffer, vec) < 1)
				return OK;
		}
		if ((av[0] = fullchanname(*vec)) == NULLCP)
			return OK;

		if (*++vec == NULLCP) {
			(void) sprintf (prompt,
					"%s (on channel %s) MTA: ", str,
					vec[0]); 
			if (_getline (prompt, buffer2) == NOTOK ||
			    str2vec(buffer2, &vec[1]) < 1)
				return OK;
		}
		av[1] = *vec;
		vec[2] = CTRL_CACHEADD;
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "%s channel by: ", str);
			if (_getline (prompt, buffer2) == NOTOK ||
			    str2vec(buffer, vec) < 1)
				return OK;
		}
		av[3] = *vec;
		av[4] = NULLCP;

		if (initiate_op (console_fd, operation_Qmgr_mtacontrol,
				 vec, &pid, mta_update, errbuf) == NOTOK) {
			advise (NULLCP, "status operation failed: %s", errbuf);
			return NOTOK;
		}

		if (result_op (console_fd, &pid, errbuf) == NOTOK) {
			advise (NULLCP, "result_op: %s", errbuf);
			return NOTOK;
		}
		break;

	    case DE_MESSAGE:
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "%s message qid: ", str);
			if (_getline (prompt, buffer) == NOTOK ||
			    str2vec(buffer, av) < 1)
				return OK;
			vec[1] = NULLCP;
		}
		else av[0] = *vec;
		if (*++vec == NULLCP) {
			(void) sprintf (prompt, "%s channel by: ", str);
			if (_getline (prompt, buffer2) == NOTOK ||
			    str2vec(buffer, vec) < 1)
				return OK;
		}
		av[1] = *vec;
		av[2] = CTRL_CACHEADD;
		if (*++vec == NULLCP) {
			if (_getline ("list of user numbers: ", buffer2) == NOTOK ||
			    str2vec (buffer2, &av[3]) < 1)
				return OK;
		}
		else {
			int i = 3;
			while(*vec)
				av[i++] = *++vec;
			av[i] = NULLCP;
		}

		if (initiate_op (console_fd, operation_Qmgr_msgcontrol,
				 av, &pid, NULLIFP, errbuf) == NOTOK) {
			advise (NULLCP, "status operation failed: %s", errbuf);
			return NOTOK;
		}

		if (result_op (console_fd, &pid, errbuf) == NOTOK) {
			advise (NULLCP, "result_op: %s", errbuf);
			return NOTOK;
		}
		break;
		
	}
	return OK;
}

static int f_source (vec)
char **vec;
{
	int sb;
	FILE	*savein;
	char buffer[BUFSIZ];

	if (*++vec == NULLCP) {
		if (_getline ("file: ", buffer) == NOTOK ||
		    str2vec (buffer, vec) < 1)
			return OK;
	}
	savein = fp_in;
	if ((fp_in = fopen (vec[0], "r")) == NULL) {
		fp_in = savein;
		advise (vec[0], "Can't open file");
		return OK;
	}
	sb = batch;
	batch = 1;

	interactive ();

	fp_in = savein;
	batch = sb;

	return OK;
}
