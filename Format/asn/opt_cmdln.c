/* opt_cmdln.c: sets the ASNCMD structure to the command line values */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/opt_cmdln.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/opt_cmdln.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: opt_cmdln.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */




#include	"head.h"
#include	"asn.h"
#include	<isode/cmd_srch.h>


extern char	*myname;
int		MnemonicsRequired = FALSE;
int		CRline = FALSE;


/* -- command line option definitions -- */
#define OPT_ISET	1	/* -- input character set -- */
#define OPT_OSET	2	/* -- output character set -- */
#define OPT_IASN	3	/* -- input asn coding used -- */
#define OPT_OASN	4	/* -- output asn coding used -- */
#define OPT_X408	5
#define OPT_MNEMONICS	6
#define OPT_CR		7


static CMD_TABLE  tbl_cmdln[] = {/* -- command line options -- */
	"-is",		OPT_ISET,
	"-os",		OPT_OSET,
	"-ia",		OPT_IASN,
	"-oa",		OPT_OASN,
	"-x",		OPT_X408,
	"-m",		OPT_MNEMONICS,
	"-l",		OPT_CR,
	0,		-1
	};





/* ------------------------  Start Routine  --------------------------------- */



opt_cmdln (opts, ap)
char	**opts;
ASNCMD	*ap;
{

	int	retval;
	char	*key;

	PP_NOTICE (("%s", myname));
	
	while(*opts != NULL) {
		switch(cmd_srch(*opts, tbl_cmdln)) {
		case OPT_ISET:
			key = *opts;
			opts++;
			set_val (key, *opts, &ap -> in_charset); 
			break;
		case OPT_OSET:
			key = *opts;
			opts++;
			set_val (key, *opts, &ap -> out_charset); 
			break;
		case OPT_IASN:
			key = *opts;
			opts++;
			set_val (key, *opts, &ap -> in_asn.name);
			break;
		case OPT_OASN:
			key = *opts;
			opts++;
			set_val (key, *opts, &ap -> out_asn.name);
			break;
		case OPT_X408:
			MnemonicsRequired = FALSE;
			break;
		case OPT_MNEMONICS:
			MnemonicsRequired = TRUE;
			break;
		case OPT_CR:
			CRline = TRUE;
			break;
		default:
			PP_LOG(LLOG_EXCEPTIONS, ("unknown option '%s'", *opts));
			exit (1);	
		}
		opts++;
	}

	cmdln_chks (ap);
}




/* ----------------------  Static  Routines  -------------------------------- */




static cmdln_chks(ap)
ASNCMD	*ap;
{

	PP_TRACE (("cmdln_chks()"));

	if ((lexequ (ap -> in_charset, ap -> out_charset) == 0) &&
	   (lexequ (ap -> in_asn.name, ap -> out_asn.name) == 0)) {
		PP_LOG(LLOG_EXCEPTIONS, ("Error: no conversion is required"));
		exit (1);
	}

	/* -- outchar then must specify inchar -- */
	if ((ap -> out_charset) && (ap -> in_charset == NULLCP)) {
		PP_LOG(LLOG_EXCEPTIONS, ("Error: inchar not specified"));
		exit (1);
	}

	/* -- inchar then must specify outchar -- */
	if ((ap -> in_charset) && (ap -> out_charset == NULLCP)) {
		PP_LOG(LLOG_EXCEPTIONS, ("Error: outchar not specified"));
		exit (1);
	}

}



static int set_val (key, val, base)
char	*key;
char	*val;
char	**base;
{

	PP_NOTICE (("\t%s='%s'", key, val));


	if (val == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS, ("Error: No value specified"));
		exit (1);
	}

	if (val[0] == '-') {
		PP_LOG(LLOG_EXCEPTIONS, ("Error: Bad value '%s'", val));
		exit (1);
	}

	*base = strdup (val);
}
