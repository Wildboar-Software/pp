/* tx_a.c: handles address structures see manual page QUEUE (5) */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2adr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2adr.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: txt2adr.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include	<isode/psap.h>
#include        <isode/cmd_srch.h>
#include        "adr.h"
#include        "tb_com.h"
#include        "tb_a.h"
#include	"list_bpt.h"
#include	"list_rchan.h"

#define	txt2crit(n)	cmd_srch ((n), tbl_crit)
#define txt2int(n)	atoi(n)

extern CMD_TABLE
		tbl_bool [],
		tbl_crit [],
		tbl_redir[],
		atbl_pd_modes [],
		atbl_reg_mail [],
		atbl_rdm [],
		atbl_ctrl_addrs [/* Env-ctrl-addresses */],
		atbl_addr [/* address-lines */],
		atbl_status [/* recipient-status */],
		atbl_mtarreq [/* mta-report-request */],
		atbl_usrreq [/* user-report-request */],
		atbl_expconversion [/* explicit-conversion */],
		atbl_types [/* address-type */],
		atbl_subtypes [/* address-subtype */];

extern int n_atbl_addr, n_atbl_expconversion, n_atbl_pd_modes,
	n_atbl_rdm;

static int A_txt2adln ();
static int A_txt2address ();

extern	struct qbuf *hex2qb ();
extern X400_Extension *txt2extension ();

/* -------------------  Text File -> Memory  ------------------------------ */



int txt2adr (base, startlineoffset, argv, argc)  /* Txt -> Env-ctrl-addresses */
register ADDR           **base;
long                    startlineoffset;
char                    **argv;
int                     argc;
{
	PP_DBG (("Lib/pp/txt2adr (%s)", argv[0]));

	if (--argc < 1)  return (NOTOK);

	switch (cmd_srch (argv[0], atbl_ctrl_addrs)) {
	case AD_ORIGINATOR:
	case AD_RECIPIENT:
		return A_txt2address (base, startlineoffset,
				      &argv[1], argc);
	}

	PP_LOG (LLOG_EXCEPTIONS,
		("Lib/pp/txt2adr Unable to parse '%s'", argv[0]));

	return (NOTOK);
}


static int A_txt2address (base, startlineoffset, argv, argc)
ADDR    **base;
long	startlineoffset;
char    **argv;
int     argc;
{
	ADDR    *adr_new(),
		*adr;
	int     n_args;


	adr = adr_new (NULLCP, NULL, NULL);

	for (n_args=0; n_args < argc;) {

		PP_DBG (("Lib/pp/A_txt2address (%s)", 
			 argv[n_args + 1] ? argv[n_args + 1] : "<none>"));

		if (argv[n_args] == NULLCP)  break;
		if (*argv[n_args] == '=') {
			if (A_txt2adln (adr, argv[n_args + 1],
					argv[n_args + 2]) == NOTOK)
				return (NOTOK);
			n_args += 3;
		}
		else {
			if (A_txt2adln (adr, argv[n_args], NULLCP) == NOTOK)
				return NOTOK;
			n_args ++;
		}

	}

	/*  fixed format: Recip status=XXXX reform-done=999 */
	/*		  012345678901234567890123456789012 */

/* XXXX Remove after 6.0 */
	if (lexequ(argv[1], "rno") == 0) { /* old format */
		adr->ad_stat_offset = startlineoffset + 21;
		adr->ad_rcnt_offset = startlineoffset + 38;
	}
	else {
		adr->ad_stat_offset = startlineoffset + 13;
		adr->ad_rcnt_offset = startlineoffset + 30;
	}

	adr_add (base, adr);

	return (OK);

}

static	Redirection *A_txt2redir ();
static int A_txt2stat ();
static int A_txt2fmtchan ();
static int A_txt2adrtype ();
static int A_txt2adrsubtype ();
static int A_txt2req_del ();
static int A_txt2modes ();

static int A_txt2adln (adr, keywd, val)   /* Txt -> Address Line */
ADDR    *adr;
char    *keywd;
char    *val;
{
	PP_DBG (("Lib/pp/A_txt2adln (%s '%s')", 
		 keywd ? keywd : "<none>", 
		 val ? val : "<none>"));

	switch (cmdbsrch (keywd, atbl_addr, n_atbl_addr)) {
	    case AD_RECIP_NO:
		adr->ad_no = txt2int (val);
		return (OK);
	    case AD_STATUS:
		return (A_txt2stat (&adr->ad_status, val));
	    case AD_REFORM_DONE:
		adr->ad_rcnt = txt2int (val);
		return (OK);
	    case AD_REFORM_LIST:
		if (lexequ (val, EMPTY) == 0)
			return (OK);
		return (A_txt2fmtchan (val, &adr->ad_fmtchan));
	    case AD_OUTCHAN:
		if (lexequ (val, EMPTY) == 0)
			return (OK);

		if (adr->ad_outchan == NULLIST_RCHAN)
			adr->ad_outchan = list_rchan_new (NULLCP, val);
		else
			return list_rchan_schan (adr->ad_outchan, val);
		return (OK);
	    case AD_OUTHOST:
		if (lexequ (val, EMPTY) == 0)
			return (OK);

		if (adr->ad_outchan == NULLIST_RCHAN)
			adr->ad_outchan = list_rchan_new (val, NULLCP);
		else
			return list_rchan_ssite (adr->ad_outchan, val);
		return (OK);
	    case AD_EXTENSION_ID:
		adr->ad_extension = txt2int (val);
		return (OK);
	    case AD_RESPONSIBILITY:
		return P1_txt2resp (&adr -> ad_resp, val);
	    case AD_MTA_REP_REQ:
		return P1_txt2mtarreq (&adr->ad_mtarreq, val);
	    case AD_USR_REP_REQ:
		return P1_txt2usrreq (&adr -> ad_usrreq, val);
	    case AD_EXPLICITCONVERSION:
		if (lexequ (val, EMPTY) == 0) {
			adr->ad_explicitconversion = NULL;
			return (OK);
		}
		adr->ad_explicitconversion = cmdbsrch (val,
						       atbl_expconversion,
						       n_atbl_expconversion);
		return (OK);
	    case AD_ADDTYPE:
		return (A_txt2adrtype (&adr->ad_type, val));
	    case AD_SUBTYPE:
		if (lexequ (val, EMPTY) == 0) {
			adr->ad_subtype = AD_NOSUBTYPE;
			return (OK);
		}
		return (A_txt2adrsubtype (&adr->ad_subtype, val));
	    case AD_EITS:
		return txt2listbpt (&adr->ad_eit, val);
	    case AD_CONTENT:
		adr->ad_content = strdup(val);
		return OK;

	    case AD_DN:
		adr ->ad_dn = strdup (val);
		return OK;

	    case AD_ORIG_REQ_ALT:
		adr -> ad_orig_req_alt = strdup (val);
		return OK;

	    case AD_ORIG_REQ_ALT_CRIT:
		adr -> ad_orig_req_alt_crit = txt2crit(val);
		return OK;

	    case AD_REQ_DEL:
		return A_txt2req_del(adr -> ad_req_del, val);

	    case AD_REQ_DEL_CRIT:
		adr ->ad_req_del_crit = txt2crit (val);
		return OK;

	    case AD_PHYS_FORWARD:
		adr -> ad_phys_forward = cmd_srch (val, tbl_bool);
		return OK;

	    case AD_PHYS_FORWARD_CRIT:
		adr -> ad_phys_forward_crit = txt2crit (val);
		return OK;

	    case AD_PHYS_FW_AD:
		adr -> ad_phys_fw_ad_req = cmd_srch (val, tbl_bool);
		return OK;

	    case AD_PHYS_FW_AD_CRIT:
		adr -> ad_phys_fw_ad_crit = txt2crit (val);
		return OK;

	    case AD_PHYS_MODES:
		adr -> ad_phys_modes = A_txt2modes (val);
		return OK;

	    case AD_PHYS_MODES_CRIT:
		adr -> ad_phys_modes_crit = txt2crit (val);
		return OK;

	    case AD_REG_MAIL:
		adr -> ad_reg_mail_type = cmd_srch (val, atbl_reg_mail);
		return OK;

	    case AD_REG_MAIL_CRIT:
		adr -> ad_reg_mail_type_crit = txt2crit (val);
		return OK;

	    case AD_RECIP_NUMBER_ADVICE:
		adr -> ad_recip_number_for_advice = strdup (val);
		break;

	    case AD_RECIP_NUMBER_ADVICE_CRIT:
		adr -> ad_recip_number_for_advice_crit = txt2crit(val);
		break;

	    case AD_PHYS_RENDITION:
		adr -> ad_phys_rendition_attribs = oid_cpy (str2oid (val));
		return OK;

	    case AD_PHYS_RENDITION_CRIT:
		adr -> ad_phys_rendition_attribs_crit = txt2crit (val);
		return OK;

	    case AD_PD_REPORT_REQUEST:
		adr ->  ad_pd_report_request = cmd_srch (val, tbl_bool);
		return OK;

	    case AD_PD_REPORT_REQUEST_CRIT:
		adr -> ad_pd_report_request_crit = txt2crit (val);
		break;

	    case AD_REDIRECTION_HISTORY:
		{
			Redirection *rp, **rpp;

			rp = A_txt2redir (val);
			for (rpp = &adr -> ad_redirection_history;
			     *rpp; rpp = &(*rpp) -> rd_next)
				continue;
			*rpp = rp;
		}
		return OK;

	    case AD_REDIRECTION_HISTORY_CRIT:
		adr -> ad_redirection_history_crit = txt2crit (val);
		return OK;

	    case AD_MESSAGE_TOKEN:
		adr -> ad_message_token = hex2qb (val);
		return OK;

	    case AD_MESSAGE_TOKEN_CRIT:
		adr -> ad_message_token_crit = txt2crit (val);
		return OK;

	    case AD_CONTENT_INTEGRITY:
		adr -> ad_content_integrity = hex2qb(val);
		return OK;

	    case AD_CONTENT_INTEGRITY_CRIT:
		adr -> ad_content_integrity_crit = txt2crit (val);
		return OK;

	    case AD_PROOF_DELIVERY:
		adr -> ad_proof_delivery = cmd_srch (val, tbl_bool);
		return OK;

	    case AD_PROOF_DELIVERY_CRIT:
		adr -> ad_proof_delivery_crit = txt2crit (val);
		return OK;

	    case AD_EXTENSION:
		{
			X400_Extension *ext, **extp,
				*A_txt2extension ();

			ext = A_txt2extension (val);

			for (extp = &adr -> ad_per_recip_ext_list;
			     *extp; extp = &(*extp) -> ext_next)
				continue;
			*extp = ext;
		}
		return OK;

	    case AD_ORIG:
		if (val == NULLCP || *val == '\0')
			return (NOTOK);
		if (lexequ (val, EMPTY) == 0) return (OK);
		adr->ad_value = strdup(val);
		return (OK);
	    case AD_X400:
		if (val == NULLCP || *val == '\0')
			return (NOTOK);
		if (lexequ (val, EMPTY) == 0) {
			adr->ad_r400adr = NULLCP;
			return (OK);
		}
		adr->ad_r400adr = strdup (val);
		return (OK);
	    case AD_822:
		if (val == NULLCP || *val == '\0') 
			return NOTOK;
		if (lexequ (val, EMPTY) == 0) {
			adr->ad_r822adr = NULLCP;
			return (OK);
		}
		adr->ad_r822adr = strdup (val);
		return (OK);

	    case AD_X400DR:
		if (!isstr(val))
			return NOTOK;
		if (lexequ (val, EMPTY) == 0)
			adr->ad_r400DR = NULLCP;
		else
			adr->ad_r400DR = strdup(val);
		return (OK);

	    case AD_822DR:
		if (!isstr(val))
			return NOTOK;
		if (lexequ (val, EMPTY) == 0)
			adr->ad_r822DR = NULLCP;
		else
			adr->ad_r822DR = strdup(val);
		return (OK);

	    case AD_X400ORIG:
		if (!isstr(val))
			return NOTOK;
		if (lexequ (val, EMPTY) == 0)
			adr->ad_r400orig = NULLCP;
		else
			adr->ad_r400orig = strdup(val);
		return (OK);

	    case AD_END:
		return (OK);
	}
	return NOTOK;
}




static int A_txt2fmtchan (str, fmt_chan)
char                    *str;
LIST_RCHAN              **fmt_chan;
{
	char    *np, *dp,
		*cp = str;
	int     keep_going = TRUE;
	LIST_RCHAN      *new;


	PP_DBG (("Lib/A_txt2fmtchan (%s)", str));


	/* --- *** ---
	Format used:
		chan_name1|directory, ... , chan_nameN|directory

	Note:
		For ad_fmtchan no site name is specified.
		This is only specified for ad_outchan.
	--- *** --- */


	while (keep_going) {
		if ((np = index (cp, ',')) != NULLCP)
			*np++ = '\0';
		else
			keep_going = FALSE;
		if ((dp = index(cp, '|')) != NULLCP)
			*dp ++ = '\0';

		if ((new = list_rchan_new (NULLCP, cp)) == NULLIST_RCHAN) {
			list_rchan_free (*fmt_chan);
			*fmt_chan = NULLIST_RCHAN;
			return (NOTOK);
		}
		if (dp)
			new -> li_dir = strdup(dp);

		list_rchan_add (fmt_chan, new);

		cp = np;
		if (cp == NULL || *cp == '\0')
			break;
	}

	return (OK);
}




static int A_txt2stat (status, keywd)    /* Txt -> Recipient_status */
int     *status;
char    *keywd;
{
	PP_DBG (("Lib/pp/A_txt2stat (%s)", keywd));

	switch (*status = cmd_srch (keywd, atbl_status)) {
	case AD_STAT_PEND:
	case AD_STAT_DONE:
	case AD_STAT_DRREQUIRED:
	case AD_STAT_DRWRITTEN:
		return (OK);
	}
	return (NOTOK);
}


static int A_txt2adrtype (type, keywd)    /* Txt -> Address type */
int     *type;
char    *keywd;
{
	PP_DBG (("Lib/pp/A_txt2adrtype (%s)", keywd));

	switch (*type = cmd_srch (keywd, atbl_types)) {
	case AD_X400_TYPE:
	case AD_822_TYPE:
	case AD_ANY_TYPE:
		break;
	default:
		*type = AD_ANY_TYPE;
	}

	PP_DBG (("Lib/pp/A_txt2adrtype (%s %d)", keywd, *type));
	return (OK);
}


static int A_txt2adrsubtype (type, keywd)   /* Txt -> Address subtype */
int     *type;
char    *keywd;
{
	PP_DBG (("Lib/pp/A_txt2adrsubtype (%s)", keywd));

	if ((*type = cmd_srch (keywd, atbl_subtypes)) == -1)
		*type = AD_NOSUBTYPE;
	return (OK);
}

static Redirection *A_txt2redir (str)
char	*str;
{
	char	*argv[10];
	Redirection *rp;

	if (sstr2arg (str, 10, argv, ", \t\n") != 4)
		return NULL;
	rp = (Redirection *) smalloc (sizeof *rp);
	rp -> rd_addr = isstr (argv[0])? strdup (argv[0]) : NULLCP;
	rp -> rd_dn = isstr (argv[1]) ? strdup (argv[1]) : NULLCP;
	(void) txt2time (argv[2], &rp -> rd_time);
	rp -> rd_reason = cmd_srch (argv[3], tbl_redir);
	rp -> rd_next = (Redirection *) NULL;
	return rp;
}

static int A_txt2modes (str)
char	*str;
{
	char	*argv[20];
	int	argc, i, n;
	int	modes = 0;
	

	argc = sstr2arg (str, 20, argv, ", \t\n");

	for (i = 0; i < argc; i++)
		if ((n = cmdbsrch (argv[i], atbl_pd_modes,
				   n_atbl_pd_modes)) != -1)
			modes |= n;
	return modes;
}

static int A_txt2req_del (rdm, str)
int	rdm[];
char	*str;
{
	char	*argv[10];
	int	argc;
	int	n, rc = 0, i;

	argc = sstr2arg (str, 10, argv, ", \t\n");
	for (i = 0; i < argc && rc < AD_RDM_MAX; i++)
		if ((n = cmdbsrch(argv[i], atbl_rdm, n_atbl_rdm)) != -1)
			rdm[rc++] = n;
	return OK;
}


	

X400_Extension *A_txt2extension (str)
char	*str;
{
	X400_Extension	*ext;
	char	*argv[10];
	
	if (sstr2arg (str, 10, argv, ", \t\n") != 4)
		return NULL;
	ext = (X400_Extension *)smalloc (sizeof *ext);
	ext -> ext_int = txt2int (argv[0]);
	ext -> ext_oid = str2oid (argv[1]);
	ext -> ext_value = hex2qb (argv[2]);
	ext -> ext_criticality = cmd_srch (argv[3], tbl_crit);

	return ext;
}

