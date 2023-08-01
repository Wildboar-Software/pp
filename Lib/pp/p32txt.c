/* p32txt.c: converts p3params into text encoding */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/p32txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/p32txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: p32txt.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include        "q.h"
#include        "tb_p3prm.h"

extern	int	argv2fp ();
extern	void	mpduid2txt ();

/* -------------------  Memory -> Text File  ------------------------------ */

char	*qb2hex ();

extern char     *time2txt ();
extern CMD_TABLE p3prm_tbl[];

int p32txt (fp, p3)
FILE *fp;
register P3params *p3;
{
	char    *argv[100];
	int     argc;

	PP_DBG (("Lib/pp/p32txt"));


	argv[0] = rcmd_srch (P3PRM_MPDUID, p3prm_tbl);
	argc = 1;
	mpduid2txt (&p3->mpduid, argv, &argc);
	if (argc > 1) {
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (p3 -> submit_time) {
		argv[0] = rcmd_srch (P3PRM_STIME, p3prm_tbl);
		argv[1] = time2txt (p3 -> submit_time);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (p3 -> content_type != NULLCP) {
		argv[0] = rcmd_srch (P3PRM_CONTENT, p3prm_tbl);
		argv[1] = p3 -> content_type;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (p3 -> originating_mta_certificate) {
		argv[0] = rcmd_srch (P3PRM_ORIG_MTA_CERT, p3prm_tbl);
		argv[1] = qb2hex (p3 -> originating_mta_certificate);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (p3 -> proof_of_submission) {
		argv[0] = rcmd_srch (P3PRM_PROOF_OF_SUB, p3prm_tbl);
		argv[1] = qb2hex (p3 -> proof_of_submission);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	(void) fprintf (fp, "%s\n", rcmd_srch (P3PRM_END, p3prm_tbl));

	(void) fflush (fp);
	return (ferror (fp) ? NOTOK : OK);
}
