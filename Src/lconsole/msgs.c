/* msgs.c: msg functions */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/lconsole/RCS/msgs.c,v 6.0 1991/12/18 20:27:16 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/msgs.c,v 6.0 1991/12/18 20:27:16 jpo Rel $
 *
 * $Log: msgs.c,v $
 * Revision 6.0  1991/12/18  20:27:16  jpo
 * Release 6.0
 *
 */

#include "lconsole.h"
#include "qmgr.h"
#include "qmgr-int.h"
#include "Qmgr-ops.h"
#include <isode/cmd_srch.h>
#ifdef  BSD42
#include <sys/ioctl.h>
#endif

#define plural(n,s)	((n) == 1 ? "" : (s))

char *msgtolist;

msg_update() { }

static void display_all_msgs (mlp)
MsgList *mlp;
{
	MsgInfo *mip, *msgs = mlp -> msgs;
	int width, w;
	int count;
	char **cpp;

	for (count = 0, mip = msgs; mip; mip = mip -> next)
		count ++;
	cpp = (char **) smalloc (count * sizeof (char *));

	for (width = count = 0, mip = msgs; mip; mip = mip -> next) {
		if ((w = strlen (mip -> permsginfo -> qid)) > width)
			width = w;
		cpp[count++] = mip -> permsginfo -> qid;
	}
	w = ncols (stdout) / (width+1);
	for (count = 0, mip = msgs; mip; mip = mip -> next) {
		printf ("%-*s%s", width, cpp[count],
			(count + 1) % w == 0 ? "\n" : " ");
		count ++;
	}
	if (count % w != 0)
		putchar ('\n');
	free ((char *)cpp);
}

static void display_recip_list (rip)
RecipInfo *rip;
{
	Strlist *sl;
	int i;

	if (rip == NULL) return;

	if (rip -> id != 0)  {
		for (sl = rip -> channels, i = 0; i < rip -> channelsDone; i++)
			sl = sl -> next;

		printf ("%*sTo: %s (id %d)\n",
			indent * 2, "",
			rip -> user, rip -> id);

		indent ++;
		printf ("%*sMTA %s via channel %s\n",
			indent * 2, "", rip -> mta,
			sl ? sl -> str : "<unknown>");
		display_status (rip -> status);
		if (rip -> info)
			printf ("%*sError Info: %s\n",
				indent * 2, "", rip -> info);
		indent --;
	}
	display_recip_list (rip -> next);
}

void display_msg_info (mip)
MsgInfo *mip;
{
	PerMessageInfo *pmi = mip -> permsginfo;
	printf ("%*sMessage id %s\n",
		indent * 2, "", pmi -> qid);
	indent ++;
	printf ("%*sfrom %s\n", indent * 2, "", pmi -> originator);
	printf ("%*sSize %s priority %d queued since %s", indent * 2, "",
		datasize (pmi -> size, ""),
		pmi -> priority,
		ctime (&pmi -> age));
	if (pmi -> deferredTime)
		printf ("%*sDeferred until %s",
			indent * 2, "", ctime(&pmi -> deferredTime));
	printf ("%*sWill expire at %s", indent * 2, "",
		ctime(&pmi -> expiryTime));
	if (pmi -> uaContentId)
		printf ("%*sUA ID: %s\n", indent * 2, "", pmi -> uaContentId);
	if (pmi -> errorCount)
		printf ("%*sHas accumulated %d errors\n", indent * 2, "",
			pmi -> errorCount);
	if (pmi -> numberWarningsSent > 0)
		printf ("%*sHas had %d warning%s sent\n",
			indent * 2, "",
			pmi -> numberWarningsSent,
			plural(pmi -> numberWarningsSent, "s"));

	indent ++;
	display_recip_list (mip -> recips);
	indent --;
	indent --;
	putchar ('\n');
}

/* ARGSUSED */
int msglist_update(msgl, id)
MsgList *msgl;
int id;
{
	MsgInfo *mip;
	int found;
	char *cp;

	indent = 0;
	if (msgl == NULL)
		advise (NULLCP, "No messages found");
	else if (msgtolist) {
		found = 0;
		if ((cp = re_comp (msgtolist)) != NULLCP) {
			advise (NULLCP, "Expression %s: %s",
				msgtolist, cp);
			free_MsgList (msgl);
			return OK;
		}
		for (mip = msgl -> msgs; mip; mip = mip -> next)
			if (re_exec (mip -> permsginfo -> qid) == 1) {
				display_msg_info (mip);
				found ++;
			}
		if (found == 0)
			advise (NULLCP, "Message %s not found", msgtolist);
	}
	else if (msgl -> msgs && msgl -> msgs -> next == NULL) 
		display_msg_info (msgl -> msgs);
	else
		display_all_msgs (msgl);

	free_MsgList (msgl);
	return OK;
}


int f_msg (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	int pid;
	char buffer[BUFSIZ], buffer2[BUFSIZ];
	char *av[NVARGS];

	if (*++vec == NULLCP) {
		if (_getline ("Channel: ", buffer) == NOTOK ||
		    str2vec (buffer, vec) < 1)
			return OK;
	}
	av[0] = *vec;
	if ((av[0] = fullchanname (av[0])) == NULLCP)
		return OK;

	if (*++vec == NULLCP) {
		if (_getline ("Mta: ", buffer2) == NOTOK ||
		    str2vec(buffer2, vec) < 1)
			return OK;
	}
	av[1] = *vec;

	if (*++vec == NULLCP) {
		msgtolist = NULLCP;
	}
	else {
		msgtolist = *vec;
		*vec = NULLCP;
	}
	av[2] = NULLCP;
	if (initiate_op (console_fd, operation_Qmgr_readChannelMtaMessage,
			 av, &pid, msglist_update, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}

	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	
	return OK;
}
