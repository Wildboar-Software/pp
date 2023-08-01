/* mta.c: mta functions */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/lconsole/RCS/mtas.c,v 6.0 1991/12/18 20:27:16 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/mtas.c,v 6.0 1991/12/18 20:27:16 jpo Rel $
 *
 * $Log: mtas.c,v $
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

static char *mtatolist;

static void display_mta_info (mip)
MtaInfo *mip;
{
	printf ("%*sMTA %s on channel %s\n",
		indent * 2, "",
		mip -> mta, mip -> channel);
	indent ++;
	printf ("%*s%s message%s, ", indent * 2, "",
		datasize (mip -> numberMessages, ""),
		plural(mip -> numberMessages,"s"));
	printf ("%s delivery report%s, ",
		datasize (mip->numberReports, ""),
		plural(mip->numberReports,"s"));
	printf ("%s data\n",
		datasize (mip -> volumeMessages, ""));
	if (mip -> active)
		printf ("%*sMTA currently being delivered\n",
			indent * 2, "");
	display_status (mip -> status);

	if (mip -> oldestMessage && (mip -> numberReports > 0 ||
				     mip -> numberMessages > 0))
		printf ("%*sOldest message is %s", indent * 2, "",
			ctime (&mip -> oldestMessage));
	indent --;
}

static void display_all_mtas (mtas)
MtaInfo *mtas;
{
	MtaInfo *mip;
	int width, w;
	int count;
	char **cpp;

	for (count = 0, mip = mtas; mip; mip = mip -> next)
		count ++;
	cpp = (char **) smalloc (count * sizeof (char *));

	for (width = count = 0, mip = mtas; mip; mip = mip -> next) {
		char buf[BUFSIZ];
		char c[2];

		c[1] = NULL;
		c[0] = ':';
		if (mip -> active)
			c[0] = '*';
		if (mip -> status -> enabled == 0)
			c[0] = '!';

		if (mip -> numberReports != 0)
			(void) sprintf (buf, "%s%s%d+%d",
					mip -> mta, c,
					mip -> numberMessages,
					mip -> numberReports);
		else if (mip -> numberMessages != 0)
			(void) sprintf (buf, "%s%s%d",
					mip -> mta, c,
					mip -> numberMessages);
		else	(void) sprintf (buf, "%s%s", mip -> mta,
					c[0] == ':' ? "" : c);
		if ((w = strlen (buf)) > width)
			width = w;
		cpp[count++] = strdup (buf);
	}
	w = ncols (stdout) / (width+1);
	for (count = 0, mip = mtas; mip; mip = mip -> next) {
		printf ("%-*s%s", width, cpp[count],
			(count + 1) % w == 0 ? "\n" : " ");
		free (cpp[count++]);
	}
	if (count % w != 0)
		putchar ('\n');
	free ((char *)cpp);

}


/* ARGSUSED */
int mta_update(mta, id)
MtaInfo *mta;
int id;
{
	int found = 0;
	MtaInfo *mip;
	char *cp;

	if (mta == NULL)
		advise (NULLCP, "No MTA's found");
	else if (mtatolist) {
		found = 0;
		if ((cp = re_comp (mtatolist)) != NULLCP) {
			advise (NULLCP, "Expression %s: %s",
				mtatolist, cp);
			free_MtaInfo (mta);
			return OK;
		}
		for (mip = mta; mip; mip = mip -> next) {
			if (re_exec (mip -> mta) == 1) {
				display_mta_info (mip);
				found ++;
			}
		}
		if (found == 0)
			advise (NULLCP, "MTA %s not found", mtatolist);
	}
	else if (mta -> next == NULL)
		display_mta_info (mta);
	else
		display_all_mtas (mta);

	free_MtaInfo (mta);
	return OK;
}

int f_mta (vec)
char **vec;
{
	char errbuf[BUFSIZ];
	int pid;
	char buffer[BUFSIZ];
	char *av[NVARGS];

	if (*++vec == NULLCP) {
		if (getline ("Channel to list MTA's on: ", buffer) == NOTOK ||
		    str2vec (buffer, av) < 1)
			return OK;
		vec[1] = NULLCP;
	}
	else av[0] = *vec;
	av[0] = fullchanname (av[0]);
	if (av[0] == NULLCP)
		return NOTOK;

	if (*++vec == NULLCP) {
		mtatolist = NULLCP;
	}
	else {
		mtatolist = *vec;
	}
	
	if (initiate_op (console_fd, operation_Qmgr_mtaread,
			 av, &pid, mta_update, errbuf) == NOTOK) {
		advise (NULLCP, "status operation failed: %s", errbuf);
		return NOTOK;
	}

	if (result_op (console_fd, &pid, errbuf) == NOTOK) {
		advise (NULLCP, "result_op: %s", errbuf);
		return NOTOK;
	}
	
	return OK;
}
