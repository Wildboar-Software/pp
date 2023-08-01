/* gen_ndr.c: generate the Non Delivery Notifications from submit */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_ndr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_ndr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: gen_ndr.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "dr.h"
#include <isode/cmd_srch.h>

extern char             msg_fullpath[];
extern char		*loc_dom_site;
extern char		*msg_unique;
extern CMD_TABLE	rr_tcode[],
			rr_dcode[],
			rr_rcode[];
extern void		err_abrt();

/* -- local routines -- */
void			gen_ndr();
static void		gen_ndrFORrecip();
static void		gen_ndrFORsender();
static void		set_DRvalues();




/*----------------------  Begin  Routines  -------------------------------- */


#define isSet(x, str)	((str) ? str : x  -> ad_value)
#define gval(x)                 ((x -> ad_type) == AD_X400_TYPE ? \
				 (isSet(x, x -> ad_r400adr)) : \
				 (isSet(x, x -> ad_r822adr)))

void gen_ndr(qp)
Q_struct *qp;
{
	PP_TRACE (("submit/gen_ndr()"));

	if (qp -> Oaddress -> ad_status == AD_STAT_DRREQUIRED)
		gen_ndrFORsender(qp);
	else 
		gen_ndrFORrecip(qp);
}




static void gen_ndrFORsender(qp)
Q_struct *qp;
{
	register ADDR           *ad,
				*sender;
	char			errbuf[LINESIZE];

	PP_TRACE (("submit/gen_ndrFORsender()"));

	sender = qp -> Oaddress;
	sender -> ad_status = AD_STAT_DONE;
	(void) sprintf (errbuf,
		"Bad sender address '%s' [%s], all recips are rejected",
		gval(sender), sender -> ad_parse_message);

	if (qp -> queuetime == NULLUTC)
		qp -> queuetime = utcnow();

	for (ad = qp -> Raddress; ad != NULLADDR; ad = ad -> ad_next) {
		if (ad -> ad_status == AD_STAT_DONE)
			continue;

		ad -> ad_status = AD_STAT_DRREQUIRED;
		ad -> ad_reason = DRR_NO_REASON;
		if (ad -> ad_add_info)
			free (ad -> ad_add_info);
		ad -> ad_add_info = NULLCP;
		set_DRvalues (qp, sender, ad, errbuf);
	}

	if (wr_q2drfile (qp, msg_fullpath, 0) != RP_OK)
		err_abrt(RP_AGN, "Error writing delivery report");

	return;
}




static void gen_ndrFORrecip(qp)
Q_struct *qp;
{
	register ADDR           *ad,
				*sender;
	int                     recip_err = FALSE;

	PP_TRACE (("submit/gen_ndrFORrecip()"));

	sender = qp -> Oaddress;
	if (qp -> queuetime == NULLUTC)
		qp -> queuetime = utcnow();

	for (ad = qp -> Raddress; ad != NULLADDR; ad = ad -> ad_next) {
		if (ad -> ad_status != AD_STAT_PEND &&
		    ad -> ad_status != AD_STAT_DRREQUIRED)
			continue;

		if (rp_isbad (ad -> ad_parse_stat) ||
		    ad -> ad_status == AD_STAT_DRREQUIRED)
		{
			recip_err = TRUE;
			set_DRvalues (qp, sender, ad, NULLCP);
		}
	}


	if (recip_err == FALSE) {
		PP_TRACE (("submit/gen_ndrFORrecip (not generated)"));
		return;
	}


	if (wr_q2drfile (qp, msg_fullpath, 0) != RP_OK)
		err_abrt (RP_AGN, "Error writing DR");

	return;
}

static void set_DRvalues (qp, sender, recip, errstr)
Q_struct *qp;
ADDR	*sender;
ADDR	*recip;
char	*errstr;
{
	char	*str, boundadd[BUFSIZ];
	int	freeit = 0;

	recip -> ad_type = sender -> ad_type;

	if (recip -> ad_reason == DRR_NO_REASON) {
		recip -> ad_reason = DRR_UNABLE_TO_TRANSFER;
		recip -> ad_diagnostic = DRD_UNRECOGNISED_OR;
	}

	if ((str = recip -> ad_add_info) == NULLCP) {
		if (errstr)
			str = errstr;
		else 
			(void) sprintf (str = boundadd,
					"Bad recipient address '%s' [%s]",
					gval(recip),
					rp_valstr (recip -> ad_parse_stat));
	}
	else
		freeit = 1;
	set_1dr (qp, recip -> ad_no, msg_unique,
		 recip -> ad_reason,
		 recip -> ad_diagnostic, str);
	if (freeit)
		free (str);
}
