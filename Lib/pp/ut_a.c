/* ut_a.c: Address utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_a.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_a.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_a.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "adr.h"
#include "dr.h"

extern	void extensions_free ();

ADDR *adr_new (orig, type, rno)   /* create new addr struct */
char    *orig;
int     type;
int     rno;
{
	register ADDR   *ap;

	PP_DBG ((
		"Lib/pp/ut_a.c/adr_new ('addr-orig=%s', 'type=%d' 'rno=%d')",
		orig ? orig : "<none>", 
		 type, rno));

	ap = (ADDR *) smalloc (sizeof(ADDR));
	adr_init (ap);

	/* set values for orig, type and rno if given */
	if (orig != NULLCP)
		ap->ad_value = strdup(orig);
	if (type)
		ap->ad_type = type;
	if (rno) {
		ap->ad_extension = rno;
		ap->ad_no = rno;
	}

	ap->aparse = aparse_new();
	ap->aparse->ad_type = ap->ad_type;

	return(ap);
}

void adr_init (ap)
ADDR	*ap;
{
	int	i;

	bzero ((char *) ap, sizeof (*ap));
	ap->ad_status = AD_STAT_PEND;           /* recipient status */
	ap->ad_resp = YES;                      /* responsibility flag */
	ap->ad_mtarreq = AD_MTA_BASIC;          /* mta-rep-req PRFlag */
	ap->ad_usrreq = AD_USR_BASIC;           /* usr-rep-req PRFlag */
	ap->ad_type = AD_ANY_TYPE;              /* address-type */
	ap->ad_reason = DRR_NO_REASON;          /* no reason */
	ap->ad_parse_stat = RP_AOK;             /* addr parsing error status*/
	ap->ad_explicitconversion = AD_EXP_NONE;
#ifdef notdef	/* these should not be set - unless the element is there */
	ap->ad_phys_modes = AD_PM_ORD;
	ap->ad_reg_mail_type = AD_RMT_UNSPECIFIED;
#endif
	for (i = 0; i < AD_RDM_MAX; i++)
		ap -> ad_req_del[i] = AD_RDM_NOTUSED;
}

void adr_add (base, new)
ADDR    **base;
ADDR    *new;
{
	ADDR    *ap = *base;

	PP_DBG (("Lib/pp/ut_a.c/adr_add ()"));

	if (ap == NULLADDR)
		*base = new;
	else {
		while (ap->ad_next != NULLADDR)
			ap = ap->ad_next;
		ap->ad_next = new;
	}
}

void adr_tfree (addr)	/* remove adr and recurse down the links */
ADDR    *addr;
{
	ADDR	*ap;

	for (; addr; addr = ap) {
		ap = addr -> ad_next;
		adr_free (addr);
		free ((char *)addr);
	}
}


void adr_free (ap)
register ADDR   *ap;
{
	PP_DBG (("Lib/pp/ut_a.c/UA_afree()"));

	if (ap->ad_value != NULLCP)
		free (ap->ad_value);
	if (ap->ad_r822adr != NULLCP)
		free (ap->ad_r822adr);
	if (ap->ad_r400adr != NULLCP)
		free (ap->ad_r400adr);
	if (ap->ad_add_info != NULLCP)
		free (ap->ad_add_info);
	if (ap->ad_content != NULLCP)
		free (ap->ad_content);
	if (ap->ad_parse_message != NULLCP)
		free(ap->ad_parse_message);
	if (ap->ad_outchan)
		list_rchan_free (ap->ad_outchan);
	if (ap->ad_fmtchan)
		list_rchan_free (ap->ad_fmtchan);
	if (ap -> ad_eit)
		list_bpt_free (ap -> ad_eit);

	if (ap -> ad_dn)
		free (ap -> ad_dn);
	if (ap -> ad_orig_req_alt)
		free (ap -> ad_orig_req_alt);
	if (ap -> ad_recip_number_for_advice)
		free (ap -> ad_recip_number_for_advice);
	if (ap -> ad_phys_rendition_attribs)
		oid_free (ap -> ad_phys_rendition_attribs);
	if (ap -> ad_redirection_history)
		redirection_free (ap -> ad_redirection_history);

	if (ap -> ad_message_token)
		qb_free (ap -> ad_message_token);
	if (ap -> ad_content_integrity)
		qb_free (ap -> ad_content_integrity);
	if (ap -> ad_per_recip_ext_list)
		extensions_free (ap -> ad_per_recip_ext_list);
	if (ap -> aparse)
		aparse_free(ap -> aparse);
}

