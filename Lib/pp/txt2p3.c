/* tx2p3.c: txt format to p3params structure */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2p3.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2p3.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: txt2p3.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/cmd_srch.h>
#include "mta.h"
#include "tb_p3prm.h"

extern	int		argv2fp ();
extern struct qbuf	*hex2qb ();
extern CMD_TABLE	p3prm_tbl[];


int txt2p3prm (p3, argv, argc)
P3params	*p3;
char	**argv;
int	argc;
{
	int	key;
	
	PP_DBG(("Lib/pp/txt2p3.c(%s)", argv[0]));


	key = cmd_srch (argv[0], p3prm_tbl);

	if (argc <  1 && key != P3PRM_END)
		return NOTOK;

	switch (key) {
	    case P3PRM_MPDUID:
		if (txt2mpduid (&p3 -> mpduid, &argv[1], argc - 1) == NOTOK)
			return NOTOK;
		return OK;
	    case P3PRM_STIME:
		if (!isstr(argv[1])) return OK;
		return txt2time (argv[1], &p3 -> submit_time);
	    case P3PRM_CONTENT:
		if (!isstr(argv[1])) return OK;
		p3 -> content_type = strdup (argv[1]);
		return OK;
	    case P3PRM_ORIG_MTA_CERT:
		if (!isstr(argv[1])) return OK;
		p3 -> originating_mta_certificate = hex2qb (argv[1]);
		return OK;
	    case P3PRM_PROOF_OF_SUB:
		if (!isstr(argv[1])) return OK;
		p3 -> proof_of_submission = hex2qb (argv[1]);
		return OK;
	    case P3PRM_END:
		return P3PRM_END;
	}
	return NOTOK;
}
