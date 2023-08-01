/* adr2txt.c: converts address structures to text */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/adr2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/adr2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: adr2txt.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include        "adr.h"
#include        "tb_com.h"
#include        "tb_a.h"

#define crit2txt(n)	rcmd_srch ((n), tbl_crit)

extern int argv2fp();
extern int arg2vstr();
extern void genreset();
extern void repreq2txt();
extern char *qb2hex();
extern char *time2txt();

extern CMD_TABLE
		tbl_bool [],
		tbl_crit [],
		tbl_redir [],
		atbl_reg_mail [],
		atbl_pd_modes [],
		atbl_rdm [],
		atbl_ctrl_addrs [/* Env-ctrl-addresses */],
		atbl_addr [/* address-lines */],
		atbl_status [/* recipient-status */],
		atbl_mtarreq [/* mta-report-request */],
		atbl_usrreq [/* user-report-request */],
		atbl_expconversion [/* explicit-conversion */],
		atbl_types [/* address-type */],
		atbl_subtypes [/* address-subtype */];

static char *A_extension(), *A_req_del(), *A_modes2txt(), *A_redir2txt();

static char    *A_put(),
		*A_chanlist2txt(),
		*A_outchan2txt(),
		*A_outhost2txt(),
		*A_no2txt(),
		*A_stat2txt(),
		*A_adrtype2txt(),
		*A_aext (),
		*A_adrsubtype2txt();

static int A_adrln2txt();
char	*no2txt3();




int adr2txt (fp, ap, type)  /* Env-ctrl-addresses -> Txt */
FILE    *fp;
ADDR    *ap;
int     type;
{
	char    *argv[200];
	char    sbuf[8*BUFSIZ];

	PP_DBG (("Lib/pp/adr2txt (%d)", type));

	switch (type) {
	    case AD_ORIGINATOR:
	    case AD_RECIPIENT:
		argv[0] = rcmd_srch (type, atbl_ctrl_addrs);
		if (A_adrln2txt (sbuf, argv, 1, ap) == NOTOK)
			return NOTOK;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		return (ferror (fp) ? NOTOK : OK);
	}
	return (NOTOK);
}

static int A_adrln2txt (sbuf, argv, argc, adr)  /* Address Line -> Txt */
char    *sbuf;
char    **argv;
int     argc;
ADDR    *adr;
{
	extern char *listbpt2txt();
	Redirection *rp;
	char	*cp;
	char    buf[8*BUFSIZ],
		*lnptr = sbuf;
	X400_Extension	*ext;


	genreset();

	PP_DBG (("Lib/pp/A_adrln2txt (%s)", argv[0]));

	*lnptr = 0;
	lnptr = A_put (lnptr, AD_STATUS, A_stat2txt (adr->ad_status),
		       argv, &argc);

	lnptr = A_put (lnptr, AD_REFORM_DONE, no2txt3 (adr->ad_rcnt, buf),
		       argv, &argc);

	lnptr = A_put (lnptr, AD_RECIP_NO, A_no2txt (adr->ad_no, buf),
		       argv, &argc);

	if (adr -> ad_fmtchan)
		lnptr = A_chanlist2txt (lnptr, AD_REFORM_LIST, adr->ad_fmtchan,
					argv, &argc);

	lnptr = A_outchan2txt (lnptr, AD_OUTCHAN, adr->ad_outchan, argv,
				&argc);

	lnptr = A_outhost2txt (lnptr, AD_OUTHOST, adr->ad_outchan, argv,
				&argc);

	lnptr = A_put (lnptr, AD_EXTENSION_ID,
		       A_no2txt (adr->ad_extension, buf),
		       argv, &argc);

	repreq2txt (adr->ad_resp, adr->ad_mtarreq,
			      adr->ad_usrreq, argv, &argc);

	if (adr -> ad_explicitconversion != AD_EXP_NONE)
		lnptr = A_put (lnptr,
			       AD_EXPLICITCONVERSION,
			       rcmd_srch (adr->ad_explicitconversion,
					  atbl_expconversion),
			       argv, &argc);

	lnptr = A_put (lnptr,
			AD_ADDTYPE, A_adrtype2txt (adr->ad_type), argv,
		       &argc);

	if (adr -> ad_subtype != AD_NOSUBTYPE)
		lnptr = A_put (lnptr,
			       AD_SUBTYPE, A_adrsubtype2txt (adr->ad_subtype),
			       argv, &argc);

	if (adr->ad_eit)
		lnptr = A_put (lnptr, AD_EITS, listbpt2txt (adr->ad_eit, buf),
			       argv, &argc);
	if (adr->ad_content)
		lnptr = A_put (lnptr, AD_CONTENT, adr->ad_content,
			       argv, &argc);


	if (adr -> ad_dn)
		lnptr = A_put (lnptr, AD_DN, adr -> ad_dn, argv, &argc);

	if (adr -> ad_orig_req_alt)
		lnptr = A_put (lnptr, AD_ORIG_REQ_ALT, adr -> ad_orig_req_alt,
			       argv, &argc);

#define docritical(x, n)       if (x) \
			       lnptr = A_put (lnptr, (n), crit2txt ((x)), \
					      argv, &argc)

	docritical (adr -> ad_orig_req_alt_crit, AD_ORIG_REQ_ALT_CRIT);

	if (adr -> ad_req_del[0] != AD_RDM_NOTUSED)
		lnptr = A_req_del (lnptr, adr -> ad_req_del, argv, &argc);

	docritical (adr -> ad_req_del_crit, AD_REQ_DEL_CRIT);

	if (adr -> ad_phys_forward)
		lnptr = A_put (lnptr, AD_PHYS_FORWARD,
			       rcmd_srch(TRUE, tbl_bool), argv, &argc);

	docritical (adr -> ad_phys_forward_crit, AD_PHYS_FORWARD_CRIT);

	if (adr -> ad_phys_fw_ad_req)
		lnptr = A_put (lnptr, AD_PHYS_FW_AD,
			       rcmd_srch (TRUE, tbl_bool), argv, &argc);

	docritical (adr -> ad_phys_fw_ad_crit, AD_PHYS_FW_AD_CRIT);

	if (adr -> ad_phys_modes)
		lnptr = A_modes2txt (lnptr, adr -> ad_phys_modes,
				     argv, &argc);

	docritical (adr -> ad_phys_modes_crit, AD_PHYS_MODES_CRIT);

	if (adr -> ad_reg_mail_type)
		lnptr = A_put (lnptr, AD_REG_MAIL,
			       rcmd_srch (adr -> ad_reg_mail_type,
					 atbl_reg_mail),
			       argv, &argc);

	docritical (adr -> ad_reg_mail_type_crit, AD_REG_MAIL_CRIT);

	if (adr -> ad_recip_number_for_advice)
		lnptr = A_put (lnptr, AD_RECIP_NUMBER_ADVICE,
			       adr -> ad_recip_number_for_advice,
			       argv, &argc);
	docritical (adr -> ad_recip_number_for_advice_crit,
		    AD_RECIP_NUMBER_ADVICE_CRIT);

	if (adr -> ad_phys_rendition_attribs)
		lnptr = A_put (lnptr, AD_PHYS_RENDITION,
			       sprintoid (adr -> ad_phys_rendition_attribs),
			       argv, &argc);

	docritical (adr -> ad_phys_rendition_attribs_crit,
		    AD_PHYS_RENDITION_CRIT);

	if (adr -> ad_pd_report_request)
		lnptr = A_put (lnptr, AD_PD_REPORT_REQUEST,
			       rcmd_srch (TRUE, tbl_bool),
			       argv, &argc);
	
	docritical (adr -> ad_pd_report_request_crit,
		    AD_PD_REPORT_REQUEST_CRIT);

	for (rp = adr -> ad_redirection_history; rp; rp = rp -> rd_next)
		lnptr = A_redir2txt (lnptr, rp, argv, &argc);

	docritical (adr -> ad_redirection_history_crit,
		    AD_REDIRECTION_HISTORY_CRIT);

	if (adr -> ad_message_token) {
		lnptr = A_put (lnptr, AD_MESSAGE_TOKEN,
			       cp = qb2hex (adr -> ad_message_token),
			       argv, &argc);
		free (cp);
	}

	docritical (adr -> ad_message_token_crit,
		    AD_MESSAGE_TOKEN_CRIT);

	if (adr -> ad_content_integrity) {
		lnptr = A_put (lnptr, AD_CONTENT_INTEGRITY,
			       cp = qb2hex (adr -> ad_content_integrity),
			       argv, &argc);
		free (cp);
	}

	docritical (adr -> ad_content_integrity_crit,
		    AD_CONTENT_INTEGRITY_CRIT);

	if (adr -> ad_proof_delivery)
		lnptr = A_put (lnptr, AD_PROOF_DELIVERY,
			       rcmd_srch (TRUE, tbl_bool),
			       argv, &argc);

	docritical (adr -> ad_proof_delivery_crit, AD_PROOF_DELIVERY_CRIT);

#undef docritical
	for (ext = adr -> ad_per_recip_ext_list; ext; ext = ext -> ext_next)
		lnptr = A_extension (lnptr, ext, argv, &argc);


	lnptr = A_put (lnptr, AD_ORIG, adr->ad_value, argv, &argc);

	lnptr = A_put (lnptr, AD_X400, adr->ad_r400adr, argv, &argc);

	lnptr = A_put (lnptr, AD_822, adr->ad_r822adr, argv, &argc);

	if (adr->ad_r400DR)
		lnptr = A_put (lnptr, AD_X400DR, adr->ad_r400DR, argv, &argc);

	if (adr->ad_r822DR)
		lnptr = A_put (lnptr, AD_822DR, adr->ad_r822DR, argv, &argc);

	if (adr->ad_r400orig)
		lnptr = A_put (lnptr, AD_X400ORIG, adr->ad_r400orig, 
			       argv, &argc);

	lnptr = A_put (lnptr, AD_END, NULLCP, argv, &argc);

	return OK;
}




/* --- *** ---
outchan contains only one value, this is a temporary measure until
next PP version - then replace with A_chanlist2txt.
--- *** --- */

static char *A_outchan2txt (lnptr, type, chanlist, argv, argc)
char                    *lnptr;
int                     type;
LIST_RCHAN              *chanlist;
char                    *argv[];
int                     *argc;
{
	LIST_RCHAN      *fp;
	char            buf[LINESIZE];


	PP_DBG (("Lib/A_outchan2txt()"));

	if (chanlist == NULLIST_RCHAN || chanlist->li_chan == NULLCHAN) {
		lnptr = A_put (lnptr, type, NULLCP, argv, argc);
		return (lnptr);
	}

	buf[0] = '\0';

	for (fp = chanlist; fp; fp = fp -> li_next)
		if (fp -> li_chan)
			if (fp -> li_chan -> ch_name) {
				(void) strcat (buf, fp -> li_chan -> ch_name);
				break;
			}


	if (buf[0] == '\0')
		lnptr = A_put (lnptr, type, NULLCP, argv, argc);
	else
		lnptr = A_put (lnptr, type, buf, argv, argc);

	return (lnptr);
}




static char *A_chanlist2txt (lnptr, type, chanlist, argv, argc)
char                    *lnptr;
int                     type;
LIST_RCHAN              *chanlist;
char                    *argv[];
int                     *argc;
{
	LIST_RCHAN      *fp;
	char            buf[LINESIZE];


	PP_DBG (("Lib/A_chanlist2txt()"));

	if (chanlist == NULLIST_RCHAN || chanlist->li_chan == NULLCHAN) {
		lnptr = A_put (lnptr, type, NULLCP, argv, argc);
		return (lnptr);
	}

	buf[0] = '\0';

	for (fp = chanlist; fp; fp = fp -> li_next)
		if (fp -> li_chan)
			if (fp -> li_chan -> ch_name) {
				if (buf[0] != '\0')
					(void) strcat (buf, ",");
				(void) strcat (buf, fp -> li_chan -> ch_name);
				if (fp -> li_dir) {
					(void) strcat(buf, "|");
					(void) strcat(buf, fp -> li_dir);
				}
			}


	if (buf[0] == '\0')
		lnptr = A_put (lnptr, type, NULLCP, argv, argc);
	else
		lnptr = A_put (lnptr, type, buf, argv, argc);

	return (lnptr);
}




/* --- *** ---
outhost contains only one value, this is a temporary measure until
next PP version - then redo as a list.
--- *** --- */

static char *A_outhost2txt (lnptr, type, hostlist, argv, argc)
char                    *lnptr;
int                     type;
LIST_RCHAN              *hostlist;
char                    *argv[];
int                     *argc;
{
	LIST_RCHAN      *fp;
	char            buf[LINESIZE];


	PP_DBG (("Lib/A_outhost2txt()"));

	if (hostlist == NULLIST_RCHAN || hostlist->li_mta == NULLCP) {
		lnptr = A_put (lnptr, type, NULLCP, argv, argc);
		return (lnptr);
	}

	buf[0] = '\0';

	for (fp = hostlist; fp; fp = fp -> li_next)
		if (fp -> li_mta) {
			(void) strcat (buf, fp -> li_mta);
			break;
		}

	if (buf[0] == '\0')
		lnptr = A_put (lnptr, type, NULLCP, argv, argc);
	else
		lnptr = A_put (lnptr, type, buf, argv, argc);

	return (lnptr);
}




static char     *A_put (lnptr, key, val, argv, argc)
char    *lnptr;
int     key;
char    *val;
char    *argv[];
int     *argc;
{

	char   *lnbegin=lnptr;
	int     n = *argc;

	if (key != AD_END)
		argv[n++] = "=";

	argv[n++] = rcmd_srch (key, atbl_addr);

	if (key != AD_END) {
		argv[n++] = lnptr;
		if (val == NULLCP || *val == '\0')
			lnptr = Bf_put (lnptr, EMPTY);
		else
			lnptr = Bf_put (lnptr, val);

		*lnptr++ = '\0';
	}

	argv[n] = NULLCP;
	*argc = n;
	PP_DBG (("Lib/pp/A_put (%.999s)", lnbegin));

	return (lnptr);
}


static char *A_no2txt (val, buf)  /* Number -> Txt */
int     val;
char    *buf;
{
	(void) sprintf (buf, "%d", val);
	return buf;
}


char *no2txt3 (value, buf)   /* Number -> Txt (max of 3 chars) */
int     value;
char    *buf;
{
	char    *bufstart = buf,
		temp[100];
	int     maxlen=3,
		i, j,
		diff,
		len;

	PP_DBG (("Lib/pp/no2txt3 (%d)", value));

	(void) sprintf (&temp[0], "%d", value);
	len = strlen (&temp[0]);

	if (len == maxlen)
		for (i=j=0;  i<maxlen;  buf[i++]=temp[j++]);
	else if (len > maxlen)
		(void) sprintf (buf, "%s", "999");
	else if (len < maxlen) {
		diff = maxlen - len;
		for (i=0;  i < diff;  buf[i]='0', i++);
		for (i=diff, j=0;  i <maxlen;  buf[i++]=temp[j++]);
		}

	buf[maxlen] = '\0';
	return (bufstart);
}


static char *A_stat2txt (status)
int     status;
{
	PP_DBG (("Lib/pp/A_stat2txt (%d)", status));

	switch (status) {
	case AD_STAT_PEND:
	case AD_STAT_DONE:
	case AD_STAT_DRREQUIRED:
	case AD_STAT_DRWRITTEN:
		return (rcmd_srch (status, atbl_status));
	}
	return (rcmd_srch (AD_STAT_UNKNOWN, atbl_status));
}


static char *A_adrtype2txt (value)    /* Address type -> Txt */
int     value;
{
	char    *keywd;

	PP_DBG (("Lib/pp/A_adrtype2txt (%d)", value));

	if ((keywd = rcmd_srch (value, atbl_types)) == NULLCP)
		keywd = rcmd_srch (AD_ANY_TYPE, atbl_types);
	return (keywd);
}


static char* A_adrsubtype2txt (value)   /* Txt -> Address subtype */
int     value;
{
	char    *keywd;

	PP_DBG (("Lib/pp/A_adrsubtype2txt (%d)", value));

	keywd = rcmd_srch (value, atbl_subtypes);
	return (keywd);
}

static char *A_extension (lnptr, ext, argv, argcp)
char	*lnptr;
X400_Extension *ext;
char	**argv;
int	*argcp;
{
	char extbuf[8*BUFSIZ];
	char	buf[20];
	char *av[5];

	av[0] = A_no2txt (ext -> ext_int, buf);
	av[1] = ext -> ext_oid ? sprintoid (ext -> ext_oid) : "";
	av[2] = qb2hex (ext -> ext_value);
	av[3] = rcmd_srch (ext -> ext_criticality, tbl_crit);
	av[4] = NULLCP;

	if (arg2vstr (0, sizeof extbuf, extbuf, av) == NOTOK)
		PP_LOG (LLOG_EXCEPTIONS, ("A_extension: arg2vstr failed"));
		
	free (av[2]);
	return A_put (lnptr, AD_EXTENSION, extbuf, argv, argcp);
}

static char	*A_redir2txt (lnptr, rp, argv, argcp)
char	*lnptr;
Redirection	*rp;
char	*argv[];
int	*argcp;
{
	char	*av[5];
	char	redbuf[BUFSIZ];

	av[0] = rp -> rd_addr ? rp -> rd_addr : "";
	av[1] = rp -> rd_dn ? rp -> rd_dn : "";
	av[2] = time2txt (rp -> rd_time);
	av[3] = rcmd_srch (rp -> rd_reason, tbl_redir);
	av[4] = NULLCP;

	if (arg2vstr (0, sizeof redbuf, redbuf, av) == NOTOK)
		PP_LOG (LLOG_EXCEPTIONS, ("A_redir2txt: arg2vstr failed"));
	return A_put (lnptr, AD_REDIRECTION_HISTORY, redbuf, argv, argcp);
}

static char *A_modes2txt (lnptr, md, argv, argcp)
char	*lnptr;
int	md;
char	*argv[];
int	*argcp;
{
	char	modebuf[BUFSIZ];
	int	i, n;
	char	*av[20];

	for (i = 1, n = 0; i <= AD_PM_MAX; i <<= 2) {
		if (md & i)
			av[n++] = rcmd_srch (i, atbl_pd_modes);
	}
	av[n] = NULLCP;

	if (arg2vstr (0, sizeof modebuf, modebuf, av) == NOTOK)
		PP_LOG (LLOG_EXCEPTIONS, ("A_modes2txt: arg2vstr failed"));

	return A_put (lnptr, AD_PHYS_MODES, modebuf, argv, argcp);
}

static char *A_req_del (lnptr, rdm, argv, argcp)
char	*lnptr;
int	rdm[];
char	*argv[];
int	*argcp;
{
	char	*av[AD_RDM_MAX+1];
	char	rdmbuf[BUFSIZ];
	int	n = 0, i;

	for (i = 0; i < AD_RDM_MAX; i++) {
		if (rdm[i] == AD_RDM_NOTUSED)
			break;

		av[n++] = rcmd_srch (rdm[i], atbl_rdm);
	}

	argv[n] = NULLCP;
	if (arg2vstr (0, sizeof rdmbuf, rdmbuf, av) == NOTOK) 
		PP_LOG (LLOG_EXCEPTIONS, ("A_req_del: arg2vstr failed"));
	return A_put (lnptr, AD_REQ_DEL, rdmbuf, argv, argcp);
}
