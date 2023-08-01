/* channel.c: channel functions */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/lconsole/RCS/channel.c,v 6.0 1991/12/18 20:27:16 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/channel.c,v 6.0 1991/12/18 20:27:16 jpo Rel $
 *
 * $Log: channel.c,v $
 * Revision 6.0  1991/12/18  20:27:16  jpo
 * Release 6.0
 *
 */

#include "lconsole.h"
#include "qmgr.h"
#include "qmgr-int.h"
#include <isode/cmd_srch.h>
#ifdef  BSD42
#include <sys/ioctl.h>
#endif

#define plural(n,s)	((n) == 1 ? "" : (s))

static CMD_TABLE *chanlist;
static char *chantolist;



static void display_chan_info (cip)
ChannelInfo *cip;
{
	printf ("%*sChannel %s (%s)\n", indent * 2, "",
		cip -> channelname,
		cip -> channeldescrip);
	indent ++;
	printf ("%*s%s message%s, ", indent *2, "",
		datasize (cip -> numberMessages, ""),
		plural(cip -> numberMessages,"s"));
	printf ("%s delivery report%s, ",
		datasize (cip->numberReports, ""),
		plural(cip->numberReports,"s"));
	printf ("%s MTA%s, ",
		datasize(cip->numberMtas, ""), plural(cip->numberMtas, "s"));
	printf ("%s data\n",
		datasize (cip -> volumeMessages, ""));
	if (cip -> numberActiveProcesses != 0) {
		printf ("%*s%d process%s running", indent * 2, "",
			cip -> numberActiveProcesses,
			plural(cip->numberActiveProcesses, "es"));
		if (cip -> maxprocs > 0)
			printf ("(max %d)", cip -> maxprocs);
		putchar ('\n');
	}
	display_status (cip -> status);
	if (cip -> oldestMessage && (cip -> numberMessages > 0 ||
				     cip -> numberReports > 0))
		printf ("%*sOldest message is %s",
			indent * 2, "",
			ctime (&cip->oldestMessage));
	indent --;
}

static void display_all_chans (chans)
ChannelInfo *chans;
{
	ChannelInfo *cip;
	int width, w;
	int count;
	char **cpp;

	for (count = 0, cip = chans; cip; cip = cip -> next)
		count ++;
	cpp = (char **) smalloc (count * sizeof (char *));

	for (width = count = 0, cip = chans; cip; cip = cip -> next) {
		char buf[BUFSIZ];
		char c[2];

		c[1] = NULL;
		c[0] = ':';

		if (cip -> numberActiveProcesses > 0)
			c[0] = '*';
		if (cip -> status -> enabled == 0)
			c[0] = '!';
		
		if (cip -> numberReports != 0)
			(void) sprintf (buf, "%s%s%d+%d",
					cip -> channelname, c,
					cip -> numberMessages,
					cip -> numberReports);
		else if (cip -> numberMessages != 0)
			(void) sprintf (buf, "%s%s%d",
					cip -> channelname, c,
					cip -> numberMessages);
		else	(void) sprintf (buf, "%s%s", cip -> channelname,
					c[0] == ':' ? "" : c);
		if ((w = strlen (buf)) > width)
			width = w;
		cpp[count++] = strdup (buf);
	}
	w = ncols (stdout) / (width+1);
	for (count = 0, cip = chans; cip; cip = cip -> next) {
		printf ("%-*s%s", width, cpp[count],
			(count + 1) % w == 0 ? "\n" : " ");
		free (cpp[count++]);
	}
	if (count % w != 0)
		putchar ('\n');

	free ((char *)cpp);
}

static void build_chanlist(chan)
ChannelInfo *chan;
{
	ChannelInfo *cip;
	int count;

	for (count = 0, cip = chan; cip; cip = cip -> next)
		count ++;

	chanlist = (CMD_TABLE *) calloc ((unsigned)count + 1,
					 sizeof *chanlist);

	for (count = 0, cip = chan; cip; cip = cip -> next) {
		chanlist[count].cmd_key = strdup (cip -> channelname);
		chanlist[count].cmd_value = count;
		count ++;
	}
	chanlist[count].cmd_key = NULLCP;
	chanlist[count].cmd_value = NOTOK;
}

void free_chanlist ()
{
	CMD_TABLE *ct;

	if (chanlist == NULL)
		return;

	for (ct = chanlist; ct -> cmd_key; ct++)
		free(ct -> cmd_key);
	free ((char *)chanlist);
	chanlist = NULL;
}

static int channel_noprint;

/* ARGSUSED */
int chan_update (chan, id)
ChannelInfo *chan;
int id;
{
	ChannelInfo *cip;
	int found;
	char *cp;

	if (chan == NULL)
		return NOTOK;

	if (chanlist == NULL)
		build_chanlist(chan);

	if (channel_noprint) {
		/* do nothing */;
	}
	else if (chantolist) {
		found = 0;
		if ((cp = re_comp (chantolist)) != NULLCP) {
			advise (NULLCP, "Expression %s: %s",
				chantolist, cp);
			free_ChannelInfo (chan);
			return OK;
		}
		for (cip = chan; cip; cip = cip -> next)
			if (re_exec (cip -> channelname) == 1) {
				display_chan_info (cip);
				found ++;
			}
		if (found == 0)
			advise (NULLCP, "Channel %s not found", chantolist);
	}
	else if (chan -> next == NULL) 
		display_chan_info (chan);
	else
		display_all_chans (chan);

	free_ChannelInfo(chan);
	return OK;
}

int f_channel (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	int pid;

	if (*++vec == NULLCP) {
		chantolist = NULLCP;
	}
	else {
		chantolist = *vec;
	}

	if (initiate_op (console_fd, operation_Qmgr_channelread,
			 NULLVP, &pid, chan_update, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}

	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	
	return OK;
}

char *fullchanname (str)
char *str;
{
	char	*cp;
	CMD_TABLE *tp, *fnd = NULL;
	int ambig = 0;
	int ch;
	char *vec[2];

	if (chanlist == NULL) {
		ch = channel_noprint, channel_noprint = 1;
		vec[0] = "channel";
		vec[1] = NULLCP;
		f_channel (vec);
		channel_noprint = ch;
	}

	if ((cp = re_comp(str)) != NULLCP) {
		advise (NULLCP, "Bad expression %s: %s", str, cp);
		return NULLCP;
	}

	for (tp = chanlist; tp -> cmd_key; tp ++)
		if (lexequ (tp -> cmd_key, str) == 0) {
			ambig = 0;
			fnd = tp; /* exact match */
			break;
		}
		else if (re_exec (tp -> cmd_key) == 1) {
			if (fnd == NULL)
				fnd = tp;
			else
				ambig = 1;
		}
	if (ambig) {
		char *p, buf[BUFSIZ];
		for (tp = chanlist, p = buf;
		     tp -> cmd_key; tp ++) {
			if (re_exec (tp -> cmd_key) == 1) {
				(void) sprintf (p, "%s%s",
						p == buf ? "" : ", ",
						tp -> cmd_key);
				p += strlen(p);
			}
		}
		advise (NULLCP, "%s is ambiguous, it matches %s", str, buf);
		return NULLCP;
	}

	if (fnd == NULL)
		advise (NULLCP, "%s does not match any channel", str);
	return fnd ? fnd -> cmd_key : NULLCP;
}
			
