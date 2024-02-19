/* show.c: filter matching functions */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/lconsole/RCS/show.c,v 6.0 1991/12/18 20:27:16 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/show.c,v 6.0 1991/12/18 20:27:16 jpo Rel $
 *
 * $Log: show.c,v $
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

extern char *msgtolist;
extern int msglist_update();

static CMD_TABLE tb_filters[] = {
#define F_CONTENT	1
	QFILTER_CONTENT,	F_CONTENT,
#define F_EIT		2
	QFILTER_EIT,		F_EIT,
#define F_PRIO		3
	QFILTER_PRIO,		F_PRIO,
#define F_RECENT	4
	QFILTER_MORERECENT,	F_RECENT,
#define F_EARLIER	5
	QFILTER_EARLIER,	F_EARLIER,
#define F_MAXSIZE	6
	QFILTER_MAXSIZE,	F_MAXSIZE,
#define F_ORIG		7
	QFILTER_ORIG,		F_ORIG,
#define	F_RECIP		8
	QFILTER_RECIP,		F_RECIP,
#define F_CHANNEL	9
	QFILTER_CHANNEL,	F_CHANNEL,
#define F_MTA		10
	QFILTER_MTA,		F_MTA,
#define F_QUEID		11
	QFILTER_QUEUEID,	F_QUEID,
#define F_MPDU		12
	QFILTER_MPDU,		F_MPDU,
#define F_UA		13
	QFILTER_UA,		F_UA,
	NULLCP,			NOTOK
	};

static int rebuild_list (vec)
char **vec;
{
	CMD_TABLE *cmd, *ep;
	char *cp;
	int i, n, width, w;
	int cols, lines;
	char buf[BUFSIZ];

	for(i = 0; vec[i]; i++) {
		if ((cp = index(vec[i], '=')) == NULLCP) {
			advise (NULLCP, "Missing '=' in %s", vec[i]);
			return NOTOK;
		}

		*cp ++ = '\0';
		switch (n = tb_match(vec[i], tb_filters)) {
		    case TB_AMBIG:
			n = strlen (vec[i]);
			cp = buf;
			for (cmd = tb_filters; cmd -> cmd_key; cmd ++) {
				if (lexnequ (vec[i], cmd -> cmd_key, n) == 0) {
					(void) sprintf (cp, "%s%s",
							cp == buf ? "" : ", ",
							cmd -> cmd_key);
					cp += strlen(cp);
				}
			}	
			advise (NULLCP, "%s is ambiguous, it matches %s",
				vec[i], buf);
			return NOTOK;

		    case TB_NONE:
			advise (NULLCP, "%s doesn't match any known key, use one of:",
				vec[i]);
			for (width = 0, ep = tb_filters; ep -> cmd_key; ep++)
				if ((n = strlen (ep -> cmd_key)) > width)
					width = n;
			if ((cols = ncols(stdout) /
			     (width = (width + 8) & ~7)) == 0)
				cols = 1;
			lines = ((ep - tb_filters) + cols - 1) / cols;
			
			for (n = 0; n < lines; n++)
				for (i = 0; i < cols; i++) {
					cmd = tb_filters + i * lines + n;
					printf ("%s", cmd -> cmd_key);
					if (cmd + lines >= ep) {
						printf ("\n");
						break;
					}
					for (w = strlen(cmd -> cmd_key);
					     w < width; w = (w + 8) & ~7)
						(void) putchar ('\t');
				}
			return NOTOK;

		    default:
			(void) sprintf (buf, "%s=%s",
					rcmd_srch (n, tb_filters),
					cp);
			vec[i] = strdup(buf);
			break;
		}
	}
	return OK;
}

/* ARGSUSED */
static int showmsglist(msgl, id)
MsgList *msgl;
int id;
{
	MsgInfo *mip;

	indent = 0;
	if (msgl == NULL || msgl -> msgs == NULL)
		advise (NULLCP, "No messages found");
	else
		for (mip = msgl -> msgs; mip; mip = mip -> next)
			display_msg_info (mip);
	free_MsgList (msgl);
	return OK;
}

int f_show (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	char buffer[BUFSIZ];
	int pid;

	if (*++vec == NULLCP) {
		if (_getline ("expression: ", buffer) == NOTOK ||
		    str2vec (buffer, vec) < 1)
			return OK;
	}
	if (rebuild_list(vec) == NOTOK)
		return OK;
	msgtolist = ".";
	if (initiate_op (console_fd, operation_Qmgr_readmsginfo,
		       vec, &pid, showmsglist, errbuf) == NOTOK) {
		advise (NULLCP, "filter operation failed: %s", errbuf);
		return NOTOK;
	}
	while (*vec)
		free (*vec++);
	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	return OK;
}
