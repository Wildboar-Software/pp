/* rd_prm.c: read in a prm_vars struct from fp */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_prm.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_prm.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: rd_prm.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "tb_prm.h"


/* ---------------------  Begin  Routines  -------------------------------- */


static char *save_file;
static int save_level;

int rd_prm (pp, fp)
register struct prm_vars        *pp;
FILE                            *fp;
{
	char    buf[BUFSIZ],
		*argv[50];
	int     argc,
		retval;

	PP_DBG (("Lib/io/rd_prm()"));

	prm_init (pp);
	if (save_file) {
		free (pp_log_norm -> ll_file);
		pp_log_norm -> ll_file = save_file;
		pp_log_norm -> ll_stat = save_level;
		save_file = NULLCP;
	}

	for (;;) {
		retval = txt2buf (buf, sizeof buf, fp);

		if (rp_isbad (retval)) {
			PP_DBG (("Lib/rd_prm txt2buf retval (%d - %s)",
			      retval, rp_valstr (retval)));
			return (retval);
		}

		argc = str2arg (buf, 50, argv);
		if (argc == 0) {
			PP_LOG (LLOG_EXCEPTIONS, ("Lib/rd_prm err (str2arg)"));
			return (RP_PARM);
		}

		retval = txt2prm (pp, argv, argc);
		if (retval == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS, ("Lib/rd_prm err (txt2prm)"));
			return (RP_PARM);
		}
		if (retval == PRM_END)
			break;
	}

	if (pp -> prm_logfile || pp -> prm_loglevel) {
		save_file = pp_log_norm -> ll_file;
		save_level = pp_log_norm -> ll_stat;
		if (pp -> prm_logfile)
			pp_log_norm -> ll_file = strdup(pp -> prm_logfile);
		if (pp -> prm_loglevel)
			pp_log_norm -> ll_events = pp -> prm_loglevel;
		(void) ll_close (pp_log_norm);
	}
	return RP_OK;
}
