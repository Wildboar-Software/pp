/* tai_chan.c: channel specific tailoring code */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/tai/RCS/tai_chan.c,v 6.0 1991/12/18 20:24:59 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/tai/RCS/tai_chan.c,v 6.0 1991/12/18 20:24:59 jpo Rel $
 *
 * $Log: tai_chan.c,v $
 * Revision 6.0  1991/12/18  20:24:59  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	<isode/cmd_srch.h>
#include	"chan.h"
#include	"adr.h"



extern void err_abrt ();
extern void tai_error ();
extern	CMD_TABLE		qtbl_con_type[],
				atbl_types[],
				atbl_subtypes[];
static char chn_str[] =		"chan";
static int ch_numchans = 0;
static int ch_maxchans = 0;

/* tables */
#define CH_NAME			1
#define CH_PROGNAME		2
#define CH_SHOW			3
#define CH_CHAN_TYPE		4
#define CH_CONTENT_IN		5
#define CH_DRCHAN		6
#define CH_CONTENT_OUT		7
#define CH_COST			8
#define CH_SORT			9
#define CH_IN_INFO		10
#define CH_AD_TYPE		11
#define CH_AD_SUBTYPE		12
#define CH_AD_ORDER		13
#define CH_BPT_IN		14
#define CH_BPT_OUT		15
#define CH_HDR_IN		16
#define CH_HDR_OUT		17
#define CH_TABLE		18
#define CH_XMTA			19
#define CH_ACCESS		20
#define CH_PROBE		21
#define CH_AUTHTBL		22
#define CH_DOMPARSE		23
#define CH_CONV			24
#define CH_MAXPROC		25
#define CH_KEY			26
#define CH_OUT_INFO		27
#define CH_OUT_AD_TYPE		28
#define CH_IN_AD_TYPE		29
#define CH_IN_SUBTYPE		30
#define CH_OUT_SUBTYPE		31
#define CH_MTA_TABLE		32
#define CH_TRACE_TYPE		33
#define CH_IN_TABLE		34
#define CH_SOLO_PROC		35
#define CH_BADSENDER_POLICY	36
#define CH_CHECK_MODE		37

static CMD_TABLE   chtbl_key[] = {
	"access",		CH_ACCESS,
	"adr",			CH_AD_TYPE,
	"adr-order",		CH_AD_ORDER,
	"auth-tbl",		CH_AUTHTBL,
	"bad-sender-policy",	CH_BADSENDER_POLICY,
	"bptin",		CH_BPT_IN,
	"bptout",		CH_BPT_OUT,
	"check",		CH_CHECK_MODE,
	"content-in",		CH_CONTENT_IN,
	"content-out",		CH_CONTENT_OUT,
	"conv",			CH_CONV,
	"cost",			CH_COST,
	"domain-norm",		CH_DOMPARSE,
	"drchan",		CH_DRCHAN,
	"hdrin",		CH_HDR_IN,
	"hdrout",		CH_HDR_OUT,
	"in-info",		CH_IN_INFO,
	"ininfo",		CH_IN_INFO,
	"inadr",		CH_IN_AD_TYPE,
	"insubadr",		CH_IN_SUBTYPE,
	"intable",		CH_IN_TABLE,
	"key",			CH_KEY,
	"maxproc",		CH_MAXPROC,
	"mta",			CH_XMTA,
	"mtatable",		CH_MTA_TABLE,
	"name",			CH_NAME,
	"out-info",		CH_OUT_INFO,
	"outinfo",		CH_OUT_INFO,
	"outadr",		CH_OUT_AD_TYPE,
	"outsubadr",		CH_OUT_SUBTYPE,
	"outtable",		CH_TABLE,
	"probe",		CH_PROBE,
	"prog",			CH_PROGNAME,
	"show",			CH_SHOW,
	"solo-proc",		CH_SOLO_PROC,
	"sort",			CH_SORT,
	"subadr",		CH_AD_SUBTYPE,
	"trace",		CH_TRACE_TYPE,
	"type",			CH_CHAN_TYPE,
	0,			-1
	};
#define N_CHANTBLENT	((sizeof(chtbl_key)/sizeof(CMD_TABLE)) - 1)



static CMD_TABLE   chtbl_types[] = {
	"both",			CH_BOTH,
	"debris",		CH_DEBRIS,
	"delete",		CH_DELETE,
	"in",			CH_IN,
	"out",			CH_OUT,
	"qmgrload",		CH_QMGR_LOAD,
	"shaper",		CH_SHAPER,
	"split",		CH_SPLITTER,
	"timeout",		CH_TIMEOUT,
	"warn",			CH_WARNING,
	0,			-1
	};
#define N_CHANTYPES	((sizeof(chtbl_types)/sizeof(CMD_TABLE)) - 1)


static CMD_TABLE   chtbl_sort[] = {
	"mta",			CH_SORT_MTA,
	"priority",		CH_SORT_PRIORITY,
	"size",			CH_SORT_SIZE,
	"time",			CH_SORT_TIME,
	"user",			CH_SORT_USR,
	"none",			CH_SORT_NONE,
	0,			-1
	};
#define N_CHANSORT	((sizeof(chtbl_sort)/sizeof(CMD_TABLE)) - 1)

static CMD_TABLE   chtbl_ad_order[] = {
	"usa",			CH_USA_ONLY,
	"uk",			CH_UK_ONLY,
	"usapref",		CH_USA_PREF,
	"ukpref",		CH_UK_PREF,
	0,			-1
	};

static CMD_TABLE chtbl_access[] = {
	"mta",		CH_MTA,
	"mts",		CH_MTS,
	0,			-1
	};

static CMD_TABLE chtbl_domparse[] = {
	"full",		CH_DOMAIN_NORM_ALL,
	"partial",	CH_DOMAIN_NORM_PARTIAL,
	0,		-1
	};

static CMD_TABLE chtbl_conv[] = {
	"none",		CH_CONV_NONE,
	"1148",		CH_CONV_1148,
	"conv",		CH_CONV_CONVERT,
	"loss",		CH_CONV_WITHLOSS,
	0,		-1
	};

static CMD_TABLE chtbl_trace[] = {
	"via",		CH_TRACE_VIA,
	"received",	CH_TRACE_RECEIVED,
	"x400",		CH_TRACE_X400,
	0,		-1
	};

static CMD_TABLE chtbl_badsender_policy[] = {
	"strict",	CH_BADSENDER_STRICT,
	"sloppy",	CH_BADSENDER_SLOPPY,
	0, -1
	};

static CMD_TABLE chtbl_check[] = {
	"strict", 	CH_STRICT_CHECK,
	"sloppy",	CH_SLOPPY_CHECK,
	NULLCP,	NOTOK
	};

/* ---------------------  Begin	 Routines  -------------------------------- */

int chan_tai (argc, argv)
int	argc;
char	**argv;
{
	register CHAN	*cp;
	char		*arg,
			*p,
			*ch_sort_arg[CH_MAX_SORT];
	int		ind,
			val,
			i, j,
			n_ch_sort;


	PP_DBG (("chan_tai()"));

	if (argc < 2 || lexequ (argv[0], chn_str) != 0)	 return (NOTOK);

	arg = *++argv;

	if (ch_maxchans == 0) {
		ch_maxchans = 10;
		ch_all = (CHAN **)smalloc (sizeof(CHAN *) * ch_maxchans);
	}
	else if (ch_numchans + 1 == ch_maxchans) {
		ch_maxchans += 10;
		ch_all = (CHAN **)realloc ((char *)ch_all,
					   (unsigned) sizeof (CHAN *) * ch_maxchans);
		if (ch_all == NULL)
			err_abrt (RP_MECH, "Out of space for channels");
	}
	ch_all[ch_numchans++] = cp = (CHAN *) smalloc ((sizeof (CHAN)));
	ch_all[ch_numchans] = NULLCHAN;


	/* -- Initialize the malloc'd channel -- */
	bzero ((char *)cp, sizeof(*cp));
/*	cp -> ch_sort[1]	= CH_SORT_TIME; */
	cp -> ch_name		= arg;
	cp -> ch_access		= CH_MTA;
	cp -> ch_ad_order	= CH_USA_ONLY;
	cp -> ch_in_ad_type	= AD_822_TYPE;
	cp -> ch_out_ad_type	= AD_822_TYPE;
	cp -> ch_domain_norm	= CH_DOMAIN_NORM_PARTIAL;
	cp -> ch_conversion	= CH_CONV_NONE;
	cp -> ch_badSenderPolicy = CH_BADSENDER_STRICT;
	cp -> ch_progname = NULLCP;

	argc -= 2;
	argv++;

	for (ind = 0; ind < argc; ind++) {

		if ((p = index (argv[ind], '=')) == NULLCP)
			continue;

		*p++ = '\0';
		PP_DBG (("tai/tai_chan %s = %s", argv[ind], p));

		switch (cmdbsrch (argv[ind], chtbl_key, N_CHANTBLENT)) {

		    case CH_NAME:
			cp->ch_name = p;
			break;

		    case CH_PROGNAME:
			cp->ch_progname = p;
			break;

		    case CH_SHOW:
			cp->ch_show = p;
			break;

		    case CH_KEY:
			if (txt2listbpt (&cp -> ch_key, p) == NOTOK)
				tai_error ("bad keys %s for %s",
					   p, cp -> ch_name);
			break;

		    case CH_CHAN_TYPE:
			val = cmdbsrch (p, chtbl_types, N_CHANTYPES);
			if (val == NOTOK)
				tai_error ("Unknown type %s in chan %s",
					   p, cp->ch_name);
			else
				cp -> ch_chan_type = val;
			break;

		    case CH_DRCHAN:
			cp->ch_drchan = p;
			break;

		    case CH_COST:
			cp->ch_cost = atoi(p);
			break;

		    case CH_SORT:
			n_ch_sort = sstr2arg
				(p, CH_MAX_SORT, ch_sort_arg, " ");

			if (n_ch_sort < 0 || n_ch_sort > CH_MAX_SORT) {
				tai_error ("Too many sort keys for %s",
					   cp -> ch_name);
				n_ch_sort = CH_MAX_SORT;
			}

			for (i=j=0; i < n_ch_sort; i++) {
				val = cmdbsrch (ch_sort_arg[i],
						chtbl_sort, N_CHANSORT);
				if (val == NOTOK)
					tai_error ("%s bad sort key for %s",
						   ch_sort_arg[i],
						   cp -> ch_name);
				else cp->ch_sort[j++] = val;
			}
			break;

		    case CH_IN_INFO:
			cp->ch_in_info = p;
			break;

		    case CH_OUT_INFO:
			cp->ch_out_info = p;
			break;

		    case CH_CONTENT_IN:
			cp->ch_content_in = p;
			break;

		    case CH_CONTENT_OUT:
			cp->ch_content_out = p;
			break;

		    case CH_AD_TYPE:
			val = cmd_srch (p, atbl_types);
			if (val == NOTOK)
				tai_error ("Unknown adr type %s in chan %s",
					   p, cp->ch_name);
			cp -> ch_in_ad_type =
				cp -> ch_out_ad_type = val;
			break;

		    case CH_IN_AD_TYPE:
			val = cmd_srch (p, atbl_types);
			if (val == NOTOK)
				tai_error ("Unknown inadr type %s in chan %s",
					   p, cp->ch_name);
			else
				cp -> ch_in_ad_type = val;
			break;

		    case CH_OUT_AD_TYPE:
			val = cmd_srch (p, atbl_types);
			if (val == NOTOK)
				tai_error ("Unknown adr out type %s in chan %s",
					   p, cp->ch_name);

			else
				cp -> ch_out_ad_type = val;
			break;

		    case CH_AD_SUBTYPE:
			val = cmd_srch (p, atbl_subtypes);
			if (val == NOTOK)
				tai_error ("Unknown ad subtype %s in chan %s",
					   p, cp->ch_name);

			else
				cp -> ch_in_ad_subtype =
					cp -> ch_out_ad_subtype = val;
			break;

		    case CH_IN_SUBTYPE:
			val = cmd_srch (p, atbl_subtypes);
			if (val == NOTOK)
				tai_error ("Unknown adr insubtype %s in chan %s",
					   p, cp->ch_name);
			else
				cp -> ch_in_ad_subtype = val;
			break;

		    case CH_OUT_SUBTYPE:
			val = cmd_srch (p, atbl_subtypes);
			if (val == NOTOK)
				tai_error ("Unknown adr outsubtype %s in chan %s",
					   p, cp->ch_name);
			else
				cp -> ch_out_ad_subtype = val;
			break;

		    case CH_AD_ORDER:
			val = cmd_srch (p, chtbl_ad_order);
			if (val == NOTOK)
				tai_error ("Unknown adr-order %s in chan %s",
					   p, cp->ch_name);

			else
				cp -> ch_ad_order = val;
			break;


		    case CH_BPT_IN:
			if (txt2listbpt (&cp -> ch_bpt_in, p) == NOTOK)
				tai_error ("Bad body part in for %s",
					   cp -> ch_name);
			break;

		    case CH_BPT_OUT:
			if (txt2listbpt (&cp -> ch_bpt_out, p) == NOTOK)
				tai_error ("Bad body part out for %s",
					   cp -> ch_name);
			break;

		    case CH_HDR_IN:
			if (txthdr2listbpt (&cp -> ch_hdr_in, p) == NOTOK)
				tai_error ("Bad hdr part in for %s",
					   cp -> ch_name);
			break;

		    case CH_HDR_OUT:
			if (txthdr2listbpt (&cp -> ch_hdr_out, p) == NOTOK)
				tai_error ("Bad hdr part out for %s",
					   cp -> ch_name);
			break;

		    case CH_TABLE:
			if ((cp -> ch_table = tb_nm2struct (p)) == NULLTBL)
				tai_error ("Unknown table %s for %s",
					   p, cp -> ch_name);
			break;

		    case CH_MTA_TABLE:
			if ((cp -> ch_mta_table = tb_nm2struct (p)) == NULLTBL)
				tai_error ("Unknown table %s for %s",
					   p, cp -> ch_name);
			break;
		    case CH_IN_TABLE:
			if ((cp -> ch_in_table = tb_nm2struct (p)) == NULLTBL)
				tai_error ("Unknown table %s for %s",
					   p, cp -> ch_name);
			break;

		    case CH_AUTHTBL:
			if ((cp -> ch_auth_tbl = tb_nm2struct (p)) == NULLTBL)
				tai_error ("Unknown table %s for %s",
					   p, cp -> ch_name);
			break;

		    case CH_XMTA:
			cp -> ch_mta = p;
			break;

		    case CH_ACCESS:
			val = cmd_srch (p, chtbl_access);
			if (val != NOTOK)
				cp -> ch_access = val;
			else
				tai_error ("Unknown access type %s in chan %s",
					   p, cp->ch_name);

			break;

		    case CH_PROBE:
			if (*p == 'y')
				cp -> ch_probe = TRUE;
			break;

		    case CH_DOMPARSE:
			val = cmd_srch (p, chtbl_domparse);
			if (val != NOTOK)
				cp -> ch_domain_norm = val;
			else
				tai_error ("Unknown domparse %s in chan %s",
					   p, cp->ch_name);

			break;

		    case CH_MAXPROC:
			cp -> ch_maxproc = atoi (p);
			break;
		    case CH_CONV:
			val = cmd_srch (p, chtbl_conv);
			if (val != NOTOK)
				cp -> ch_conversion = val;
			else
				tai_error ("Unknown conv type %s in chan %s",
					   p, cp->ch_name);

			break;
		    case CH_TRACE_TYPE:
			val = cmd_srch (p, chtbl_trace);

			if (val != NOTOK)
				cp -> ch_trace_type = val;
			else
				tai_error ("Unknown trace type %s in chan %s",
					   p, cp->ch_name);

			break;

		    case CH_SOLO_PROC:
			if (*p == 'y')
				cp -> ch_solo_proc = TRUE;
			break;
			
		    case CH_BADSENDER_POLICY:
			val = cmd_srch (p, chtbl_badsender_policy);

			if (val != NOTOK)
				cp -> ch_badSenderPolicy = val;
			else
				tai_error ("Unknown bad sender policy '%s' for chan %s",
					   p, cp->ch_name);
			break;

		    case CH_CHECK_MODE:
			val = cmd_srch (p, chtbl_check);
			if (val != NOTOK)
				cp -> ch_strict_mode = val;
			else val = CH_STRICT_CHECK;
			break;

		    default:
			tai_error("Unknown key '%s' for %s",
				  argv[ind], cp -> ch_name);
		}

	}


	return (OK);
}
