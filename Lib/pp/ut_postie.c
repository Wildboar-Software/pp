/* ut_postmaster.c: utility routines to get at postmaster address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_postie.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_postie.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_postie.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "adr.h"
#include "chan.h"
#include "retcode.h"

static ADDR	*postmasterAddr;
extern char	*postmaster;
char	*getpostmaster(type)
int	type;
{
	if (postmasterAddr == NULLADDR) {
		RP_Buf	rp;
		postmasterAddr = adr_new(postmaster, AD_822_TYPE, 0);
		postmasterAddr -> ad_resp = NO;
#ifdef UKORDER
		if (rp_isbad(ad_parse(postmasterAddr, &rp, CH_UK_PREF))) {
#else
		if (rp_isbad(ad_parse(postmasterAddr, &rp, CH_USA_PREF))) {
#endif
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Postmaster address '%s' does not parse [%s]",
				postmaster, postmasterAddr->ad_parse_message));
			adr_free(postmasterAddr);
			postmasterAddr = NULLADDR;
			return NULLCP;
		}
	}
	switch (type) {
	    case AD_822_TYPE:
		return postmasterAddr->ad_r822adr;
	    case AD_X400_TYPE:
		return postmasterAddr->ad_r400adr;
	    default:
		return postmasterAddr->ad_value;
	}
}

static ADDR	*admin_Req_AltAddr;
extern char	*adminstration_req_alt;
char	*getadmin_Req_Alt(type)
int	type;
{
	if (admin_Req_AltAddr == NULLADDR) {
		RP_Buf	rp;
		admin_Req_AltAddr = adr_new(adminstration_req_alt, 
					    AD_822_TYPE, 0);
		admin_Req_AltAddr -> ad_resp = NO;
#ifdef UKORDER
		if (rp_isbad(ad_parse(admin_Req_AltAddr, &rp, CH_UK_PREF))) {
#else
		if (rp_isbad(ad_parse(admin_Req_AltAddr, &rp, CH_USA_PREF))) {
#endif
			PP_LOG(LLOG_EXCEPTIONS,
			       ("adminstration_req_alt address '%s' does not parse [%s]",
				adminstration_req_alt, admin_Req_AltAddr->ad_parse_message));
			adr_free(admin_Req_AltAddr);
			admin_Req_AltAddr = NULLADDR;
			return NULLCP;
		}
	}
	switch (type) {
	    case AD_822_TYPE:
		return admin_Req_AltAddr->ad_r822adr;
	    case AD_X400_TYPE:
		return admin_Req_AltAddr->ad_r400adr;
	    default:
		return admin_Req_AltAddr->ad_value;
	}
}
