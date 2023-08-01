/* tx_prm.c: handles the MessageManagementParameter structures,
	     see manaul page QUEUE (5).
*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2prm.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: txt2prm.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include        "prm.h"
#include        "tb_com.h"
#include        "tb_prm.h"

#define 	txt2int(n)	atoi(n)

extern CMD_TABLE
		prmtbl_ln [/* message-management-parameters */],
		prmtbl_opts [/* options */];




/* -------------------  Text File -> Memory  ------------------------------ */

static int PRM_txt2opts ();


int txt2prm (pp, argv, argc)  /* Txt -> MessageManagementParameters */
struct prm_vars         *pp;
char                    **argv;
int                     argc;
{
	int             key;


	PP_DBG (("Lib/pp/txt2prm(%s)", argv[0]));

	key = cmd_srch (argv[0], prmtbl_ln);

	if (--argc < 1)
		if (key != PRM_END)
			return (NOTOK);

	switch (key) {
	case PRM_LOGFILE:
		pp->prm_logfile = strdup (argv[1]);
		return (OK);
	case PRM_LOGLEVEL:
		pp->prm_loglevel = txt2int(argv[1]);
		return (OK);
	case PRM_PASSWD:
		pp->prm_passwd = strdup (argv[1]);
		return (OK);
	case PRM_OPTS:
		return (PRM_txt2opts (argc, &argv[1], &pp->prm_opts));
	case PRM_END:
		return (PRM_END);
	}

	PP_LOG (LLOG_EXCEPTIONS,
		("Lib/pp/txt2prm Unable to parse '%s'", argv[0]));
	return (NOTOK);
}


static int PRM_txt2opts (argc, argv, options)  /* Txt -> Praram options */
char	**argv;
int	argc;
int     *options;
{

	int     i;
	int     type;

	PP_DBG (("Lib/pp/txt2opts(%s)", argv[0]));

	/* read string - delimitted by ',' e.g nocheck etc... */
	*options = 0;
	for (i=0; i < argc; i++) {
		if ((type = cmd_srch (argv[i], prmtbl_opts))==NOTOK)
			continue;
		else
			*options |= type;
	}
	PP_DBG (("options = 0x%x", *options));
	return (OK);
}

