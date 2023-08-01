/* adr2rfc.c - Converts an X400 address into an RFC one */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/adr2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/adr2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: adr2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "or.h"
#include "adr.h"
#include <isode/cmd_srch.h>


/* -------------------- Begin Routines  ----------------------------------- */

extern char	*oid2lstr();

extern CMD_TABLE atbl_rdm[], atbl_pd_modes[], atbl_reg_mail[];

int adr2rfc (ap, buffer, order_pref)
ADDR    *ap;
char    *buffer;
int	order_pref;
{
	OR_ptr  or;
	char    lbuf[LINESIZE];
	char	*str = NULLCP;

	/* -- RFC or X400 address ? -- */

	if (ap->ad_r822adr)
		(void) strcpy (buffer, ap->ad_r822adr);
	else if (ap->ad_type == AD_822_TYPE)
		return NOTOK;

	else if (ap->ad_r400adr) {
		if ((or = or_std2or (ap->ad_r400adr)) == NULLOR)
			return NOTOK;
		if (or_or2rfc (or, lbuf) == NOTOK)
			return NOTOK;
		or_free (or);
		(void) strcpy (buffer, lbuf);
	}
	else
		return NOTOK;


	ap_s2s(buffer, &str, order_pref);
	if (str != NULLCP) {
		(void) strcpy (buffer, str);
		free (str);
	}
	if (ap -> ad_no != 0) /* don't do for Originator */
		return adr2comments(ap, buffer);
	return OK;
}

int adr2comments(ap, buffer)
ADDR	*ap;
char	*buffer;
{
	char    lbuf[LINESIZE];
	char	*type;

	/* do 1148 stuff */
	if (ap -> ad_redirection_history != NULL) {
		char		*reason;
		Redirection	*ix = ap -> ad_redirection_history;
		(void) strcat(buffer, " (");
		while (ix != NULL) {
			switch (ix->rd_reason) {
			    case RDR_RECIP_ASSIGNED:
				reason = "Recipient Assigned Alternate Recipient";
				break;
			    case RDR_ORIG_ASSIGNED:
				reason = "Originator Requested Alternate Recipient";
				break;
			    case RDR_MD_ASSIGNED:
				reason = "Recipient MD Assigned Alternate Recipient";
				break;
			    default:
				reason = NULLCP;
				break;
			}

			if (ix->rd_addr != NULLCP
			    || ix->rd_dn != NULLCP) {
				if (ix == ap->ad_redirection_history)
					(void) strcat(buffer, "Originally To: ");
				else
					(void) strcat (buffer, " ");
				(void) strcat (buffer,
					       (ix -> rd_addr != NULLCP) ?
					       ix -> rd_addr : ix -> rd_dn);
			}
			if (ix == ap->ad_redirection_history)
				(void) strcat (buffer, " Redirected on ");
			else
				(void) strcat (buffer, " Redirected Again on ");
				
			if (UTC2rfc (ix->rd_time, lbuf) != OK)
				return NOTOK;
			(void) strcat (buffer, lbuf);
			(void) strcat (buffer, " To: ");
			if (reason != NULLCP)
				(void) strcat (buffer, reason);
			ix = ix -> rd_next;
		}
		(void) strcat (buffer, ")");
	}

	if (ap -> ad_req_del[0] != AD_RDM_NOTUSED
	    && ap -> ad_req_del[0] != AD_RDM_ANY) {
		int ix = 0, first = OK;

		(void) sprintf (buffer,
				"%s (Requested Delivery Methods ",
				buffer);
		while (ix < AD_RDM_MAX 
		       && ap -> ad_req_del[ix] != AD_RDM_NOTUSED
		       && ap -> ad_req_del[ix] != AD_RDM_ANY) {
			if ((type = rcmd_srch(ap -> ad_req_del[ix],
					      atbl_rdm)) != NULLCP)
				(void) sprintf(buffer,
					       "%s%s%s",
					       buffer,
					       (first == OK) ? "" : ", ",
					       type);
			ix++;
			first = NOTOK;
		}
		(void) sprintf (buffer, "%s)", buffer);
	}
		
	if (ap -> ad_phys_forward)
		(void) sprintf (buffer,
				"%s (Physical Forwarding Prohibited)",
				buffer);
		
	if (ap -> ad_phys_fw_ad_req)
		(void) sprintf (buffer,
				"%s (Physical Forwarding Address Requested)",
				buffer);
		
	if (ap -> ad_phys_modes
	    && ap -> ad_phys_modes != AD_PM_ORD
	    && (type = rcmd_srch (ap -> ad_phys_modes, atbl_pd_modes)) != NULLCP)
		(void) sprintf (buffer,
				"%s (Physical Delivery Mode %s)",
				buffer, type);
		
	if (ap -> ad_reg_mail_type
	    && ap -> ad_reg_mail_type != AD_RMT_UNSPECIFIED
	    && (type = rcmd_srch (ap -> ad_reg_mail_type, atbl_reg_mail)) != NULLCP)
		(void) sprintf (buffer,
				"%s (Registered Mail Type %s)",
				buffer, type);
		
	if (ap -> ad_recip_number_for_advice != NULLCP)
		(void) sprintf (buffer,
				"%s (Recipient Number For Advice %s)",
				buffer, 
				ap -> ad_recip_number_for_advice);
		
	if (ap -> ad_phys_rendition_attribs != NULL)
		(void) sprintf (buffer,
				"%s (Physical Rendition Attributes %s)",
				buffer, 
				oid2lstr(ap -> ad_phys_rendition_attribs));

	if (ap -> ad_pd_report_request)
		(void) sprintf (buffer,
				"%s (Physical Delivery Report Requested)",
				buffer);

	if (ap -> ad_proof_delivery)
		(void) sprintf (buffer,
				"%s (Proof of Delivery Requested)",
				buffer);
		
	PP_DBG (("Lib/adr2rfc returns (%s)", buffer));
	return OK;
}
