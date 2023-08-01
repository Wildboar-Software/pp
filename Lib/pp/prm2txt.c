/* tx_prm.c: handles the MessageManagementParameter structures,
	     see manaul page QUEUE (5).
*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/prm2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/prm2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: prm2txt.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include        "prm.h"
#include        "tb_prm.h"

extern	void	genreset ();
extern	int	argv2fp ();

extern CMD_TABLE
		prmtbl_ln [/* message-management-parameters */],
		prmtbl_opts [/* options */];


/* -------------------  Memory -> Text File  ------------------------------ */

static void PRM_opts2txt ();

int prm2txt (fp, pp)  /* MessageManagementParameters -> Txt */
FILE                    *fp;
struct prm_vars         *pp;
{
	extern char     *int2txt ();
	char    *argv[100];
	int     argc;

	genreset ();
	PP_DBG (("Lib/pp/prm2txt()"));

	if (pp->prm_logfile != NULLCP) {
		argv[0] = rcmd_srch (PRM_LOGFILE, prmtbl_ln);
		argv[1] = pp -> prm_logfile;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (pp->prm_loglevel != NULL) {
		argv[0] = rcmd_srch (PRM_LOGLEVEL, prmtbl_ln);
		argv[1] = int2txt (pp->prm_loglevel);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (pp->prm_opts != NULL) {
		argv[0] = rcmd_srch (PRM_OPTS, prmtbl_ln);
		argc = 1;
		PRM_opts2txt (pp->prm_opts, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (pp -> prm_passwd != NULL) {
		argv[0] = rcmd_srch (PRM_PASSWD, prmtbl_ln);
		argv[1] = pp -> prm_passwd;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
		
	(void) fprintf (fp, "%s\n", rcmd_srch (PRM_END, prmtbl_ln));

	return (ferror (fp) ? NOTOK : OK);
}


static void PRM_opts2txt (options, argv, argcp)  /* Param options -> Txt */
int     options;
char    *argv[];
int     *argcp;
{
	int     i, narg= *argcp;

	PP_DBG (("Lib/pp/PRM_opts2txt(%d)", options));

	for (i=0; i < PRM_OPTS_TOTAL; i++)
		if (options & (1<<i))
			argv[narg++] = rcmd_srch (1<<i, prmtbl_opts);

	argv[narg] = 0;
	*argcp = narg;
}
