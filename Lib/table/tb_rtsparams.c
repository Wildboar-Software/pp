/* tb_rtsparams.c: fetch X.400 RTS parameters */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_rtsparams.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_rtsparams.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_rtsparams.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "chan.h"
#include "rtsparams.h"
#include <isode/rtsap.h>
#include <isode/cmd_srch.h>


CMD_TABLE	rts_tbl[] = {
#define RTSP_LMTA	1
	"lmta",		RTSP_LMTA,
#define RTSP_RMTA	2
	"rmta",		RTSP_RMTA,
#define RTSP_LPASS	3
	"lpass",	RTSP_LPASS,
#define RTSP_RPASS	4
	"rpass",	RTSP_RPASS,
#define RTSP_RPPNAME	5
	"rname",	RTSP_RPPNAME,
#define RTSP_MDINFO	6
	"mdinfo",	RTSP_MDINFO,
#define RTSP_LPSAP	7
	"lpsap",	RTSP_LPSAP,
#define RTSP_RPSAP	8
	"rpsap",	RTSP_RPSAP,
#define RTSP_MODE	9
	"mode",		RTSP_MODE,
#define RTSP_TYPE	10
	"type",		RTSP_TYPE,
#define RTSP_OTHER	11
	"other",	RTSP_OTHER,
#define RTSP_INFO	12
	"info",		RTSP_INFO,
#define RTSP_TRYNEXT	13
	"trynext",	RTSP_TRYNEXT,
#define RTSP_TRACE	14
	"tracing",	RTSP_TRACE,
#define RTSP_FIXORIG	15
	"fix-orig",	RTSP_FIXORIG,
	NULLCP,		-1
};




CMD_TABLE rts_info[] = {
#define RTSINFO_MODE		1
	"mode",			RTSINFO_MODE,
#define RTSINFO_UNDEFINED	2
	"undefined",		RTSINFO_UNDEFINED,
	NULLCP,			-1
};




static char	def[] = "default";

static int	conv_entry(), conv_info();



/* ---------------------  Begin	 Routines  -------------------------------- */




RtsParams *tb_rtsparams(tbl, remote_site)
Table	*tbl;
char	*remote_site;
{
	RtsParams      *rp = NULL;
	char		buffer[BUFSIZ];

	if (tbl == NULLTBL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("No incomming table specified for %s", remote_site));
		return NULL;
	}

	/* --- initialisation --- */
	rp = (RtsParams *) smalloc(sizeof *rp);
	bzero((char *) rp, sizeof *rp);
	rp->rts_mode = RTS_MONOLOGUE;
	rp->type = RTSP_1984;
	rp->trace_type = RTSP_TRACE_ALL;
	rp->our_passwd = strdup("");
	rp->our_name = strdup("");


	/* --- retrieve & convert the default entry --- */
	if (tb_k2val(tbl, def, buffer, TRUE) == OK)
		if (conv_entry(rp, buffer, def) == NOTOK)
			goto tb_rtsparams_error;

	if (lexequ(def, remote_site) == 0)
		return rp;


	/* --- retrieve & convert the actual remote site entry --- */
	if (tb_k2val(tbl, remote_site, buffer, TRUE) == NOTOK)
		goto tb_rtsparams_error;

	if (conv_entry(rp, buffer, remote_site) == NOTOK)
		goto tb_rtsparams_error;

	return rp;


tb_rtsparams_error:;
	RPfree(rp);
	return NULL;
}




static int conv_entry(rp, buffer, remote_site)
RtsParams	*rp;
char		*buffer;
char		*remote_site;
{
	int		argc;
	char	       *argv[100];
	int		n;

	if ((argc = str2arg(buffer, 100, argv)) < 2) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Badly formatted table entry for %s (%s)",
			remote_site, buffer));
		return NOTOK;
	}


	for (n = 0; n < argc;) {
		if (strcmp(argv[n], "=") != 0) {
			PP_LOG(LLOG_EXCEPTIONS,
			     ("Bad value %s for %s", argv[n], remote_site));
			n++;
			continue;
		}
		n++;
		switch (cmd_srch(argv[n], rts_tbl)) {
		case RTSP_LMTA:
			if (rp->our_name)
				free(rp->our_name);
			rp->our_name = strdup(argv[n + 1]);
			break;
		case RTSP_RMTA:
			if (rp->their_name)
				free(rp->their_name);
			rp->their_name = strdup(argv[n + 1]);
			break;
		case RTSP_LPASS:
			if (rp->our_passwd)
				free(rp->our_passwd);
			rp->our_passwd = strdup(argv[n + 1]);
			break;
		case RTSP_RPASS:
			if (rp->their_passwd)
				free(rp->their_passwd);
			rp->their_passwd = strdup(argv[n + 1]);
			break;
		case RTSP_RPPNAME:
			if (rp->their_internal_ppname)
				free(rp->their_internal_ppname);
			rp->their_internal_ppname = strdup(argv[n + 1]);
			break;
		case RTSP_MDINFO:
			if (rp->md_info)
				free(rp->md_info);
			rp->md_info = strdup(argv[n + 1]);
			break;
		case RTSP_LPSAP:
			if (rp->our_address)
				free(rp->our_address);
			rp->our_address = strdup(argv[n + 1]);
			break;
		case RTSP_RPSAP:
			if (rp->their_address)
				free(rp->their_address);
			rp->their_address = strdup(argv[n + 1]);
			break;
		case RTSP_MODE:
			if (lexequ(argv[n + 1], "twa") == 0)
				rp->rts_mode = RTS_TWA;
			else if (lexequ(argv[n + 1], "mon") == 0)
				rp->rts_mode = RTS_MONOLOGUE;
			break;
		case RTSP_TYPE:
			if (lexequ(argv[n + 1], "1988-X410") == 0)
				rp->type = RTSP_1988_X410MODE;
			else if (lexequ(argv[n + 1], "1988-NORMAL") == 0 ||
				 lexequ(argv[n + 1], "1988") == 0)
				rp->type = RTSP_1988_NORMAL;
			else if (lexequ(argv[n + 1], "1984") == 0)
				rp->type = RTSP_1984;
			break;

		case RTSP_OTHER:
		case RTSP_INFO:
			if (conv_info (rp, argv[n + 1], remote_site) == NOTOK)
				return NOTOK;
			break;

		case RTSP_TRACE:
			if (lexequ(argv[n + 1], "admd") == 0)
				rp->trace_type = RTSP_TRACE_ADMD;
			else if (lexequ(argv[n + 1], "nointernal") == 0)
				rp->trace_type = RTSP_TRACE_NOINT;
			else if (lexequ(argv[n + 1], "local-internal") == 0)
				rp->trace_type = RTSP_TRACE_LOCALINT;
			else
				rp->trace_type = RTSP_TRACE_ALL;
			break;
		case RTSP_TRYNEXT:
			if (rp->try_next)
				free(rp->try_next);
			rp->try_next = strdup(argv[n + 1]);
			break;
		case RTSP_FIXORIG:
			if (rp->fix_orig)
				free(rp->fix_orig);
			rp->fix_orig = strdup(argv[n + 1]);
			break;
		default:
			PP_LOG(LLOG_EXCEPTIONS,
			       (" Warning: Unknown keyword %s", argv[n]));
			break;
		}
		n += 2;
	}
	return OK;
}




static int conv_info(rp, buffer, remote_site)
RtsParams	*rp;
char		*buffer;
char		*remote_site;
{
	int		argc;
	char	       *argv[100];
	int		n;


	if ((argc = str2arg(buffer, 100, argv)) < 2) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Badly formatted table entry for %s (%s)",
			remote_site, buffer));
		return NOTOK;
	}


	for (n = 0; n < argc;) {
		if (strcmp(argv[n], "=") != 0) {
			PP_LOG(LLOG_EXCEPTIONS,
			     ("Bad value %s for %s", argv[n], remote_site));
			n++;
			continue;
		}
		n++;
		switch (cmd_srch(argv[n], rts_info)) {
		case RTSINFO_MODE:
			if (rp->info_mode)
				free(rp->info_mode);
			rp->info_mode = strdup(argv[n + 1]);
			break;
		case RTSINFO_UNDEFINED:
			if (rp->info_undefined)
				free(rp->info_undefined);
			rp->info_undefined = strdup(argv[n + 1]);
			break;
		default:
			PP_LOG(LLOG_EXCEPTIONS,
			       (" Warning: Unknown keyword %s", argv[n]));
			break;
		}
		n += 2;
	}
	return OK;
}




void RPfree(rp)
RtsParams	*rp;
{
	PP_DBG(("RPfree()"));
	if (rp == NULL)			return;
	if (rp->their_name)		free(rp->their_name);
	if (rp->their_passwd)		free(rp->their_passwd);
	if (rp->their_internal_ppname)	free(rp->their_internal_ppname);
	if (rp->their_address)		free(rp->their_address);
	if (rp->our_name)		free(rp->our_name);
	if (rp->our_passwd)		free(rp->our_passwd);
	if (rp->our_address)		free(rp->our_address);
	if (rp->md_info)		free(rp->md_info);
	if (rp->try_next)		free(rp->try_next);
	if (rp->info_mode)		free(rp->info_mode);
	if (rp->info_undefined)		free(rp->info_undefined);
	if (rp->fix_orig)		free(rp->fix_orig);
	free((char *) rp);
}
