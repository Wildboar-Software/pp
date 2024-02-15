/* assoc.c: association functions for clients */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/qmgr/RCS/assoc.c,v 6.0 1991/12/18 20:27:38 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/assoc.c,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: assoc.c,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "types.h"
#include "qmgr.h"
#include "isode/psap2.h"
#include "isode/acsap.h"
#include "isode/rosap.h"

int	assoc_start (myservice, fd)
char	*myservice;
int	*fd;
{
	extern char *pptsapd_addr;
	static struct PSAPctxlist pcs;
	struct SSAPref sfs;
	register struct SSAPref *sf;
	struct TSAPaddr *ta;
	static struct PSAPaddr pas, *pa;
	static OID     ctx,
		pci;
	struct AcSAPconnect accs;
	register struct AcSAPconnect   *acc = &accs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;
	struct RoSAPindication rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject *rop = &roi -> roi_preject;
	int	result;

	if (pa == NULLPA) {
		if ((pa = str2paddr (pptsapd_addr)) == NULLPA) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't translate %s",
				 pptsapd_addr));
			return NOTOK;
		}
		pas = *pa;
		pa = &pas;
	}
        ta = &pa -> pa_addr.sa_addr;
	ta -> ta_selectlen = str2sel (myservice, 1, ta -> ta_selector, TSSIZE);

	if (ctx == NULLOID && (ctx = oid_cpy (CHANNEL_AC)) == NULLOID) {
		PP_LOG (LLOG_EXCEPTIONS, ("out of memory"));
		return NOTOK;
	}
	if (pci == NULLOID) {
		if ((pci = oid_cpy (CHANNEL_PCI)) == NULLOID) { 
			PP_LOG (LLOG_EXCEPTIONS, ("out of memory"));
			return NOTOK;
		}
	}
	pcs.pc_nctx = 1;
	pcs.pc_ctx[0].pc_id = 1;
	pcs.pc_ctx[0].pc_asn = pci;
	pcs.pc_ctx[0].pc_atn = NULLOID;

	if ((sf = addr2ref (PLocalHostName ())) == NULL) {
		sf = &sfs;
		(void) bzero ((char *) sf, sizeof *sf);
	}
	switch (result = AcAsynAssocRequest (ctx, NULLAEI, NULLAEI, NULLPA,
					     pa, &pcs, NULLOID, 0,
					     ROS_MYREQUIRE,
					     SERIAL_NONE, 0, sf, NULLPEP, 0,
					     NULLQOS, acc, aci, 1)) {
	    case NOTOK:
		acs_advise (aca, "A-ASSOCIATE.REQUEST");
		return NOTOK;
	    case CONNECTING_1:
	    case CONNECTING_2:
		*fd = acc -> acc_sd;
		ACCFREE (acc);
		PP_TRACE (("Association initiated"));
		return result;
	    case DONE:
		if (acc -> acc_result != ACS_ACCEPT) {
			PP_LOG (LLOG_EXCEPTIONS, 
				("Association rejected: [%s]",
				 AcErrString (acc -> acc_result)));
			return NOTOK;
		}
		*fd = acc -> acc_sd;
		ACCFREE (acc);
		PP_TRACE (("Association established"));
		if (RoSetService (*fd, RoPService, roi) == NOTOK) {
			ros_advise (rop, "set RO/PS fails");
			return NOTOK;
		}
		PP_TRACE (("Service set"));
		return DONE;
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Bad response from AcAsynAssocRequest"));
		return NOTOK;
	}
}

int acsap_retry (fd)
int	fd;
{
	struct AcSAPconnect accs;
	register struct AcSAPconnect   *acc = &accs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;
	struct RoSAPindication rois;
	register struct RoSAPindication *roi = &rois;
	register struct RoSAPpreject *rop = &roi -> roi_preject;
	int	result;

	PP_TRACE (("acsap_retry(%d)", fd));
	switch (result = AcAsynRetryRequest (fd, acc, aci)) {
	    case CONNECTING_1:
	    case CONNECTING_2:
		ACCFREE (acc);
		return result;
	    case NOTOK:
		acs_advise (aca, "A-ASSOCIATE.REQUEST");
		return NOTOK;
	    case DONE:
		if (acc -> acc_result != ACS_ACCEPT) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Association Rejected: [%s]",
				 AcErrString (acc -> acc_result)));
			return NOTOK;
		}
		ACCFREE (acc);
		if (RoSetService (fd, RoPService, roi) == NOTOK) {
			ros_advise (rop, "set RO/PS fails");
			return NOTOK;
		}
		return DONE;
	    default:
		adios (NULLCP, "Bad response from AcAsynRetryRequest");
		return NOTOK;
	}
}

int  assoc_release (sd)
int     sd;
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

	if (AcRelRequest (sd, ACF_NORMAL, NULLPEP, 0, NOTOK, acr, aci) == NOTOK) {
		acs_advise (aca, "A-RELEASE.REQUEST");
		return DONE;
	}

	if (!acr -> acr_affirmative) {
		(void) AcUAbortRequest (sd, NULLPEP, 0, aci);
		PP_NOTICE (("Release rejected by peer: %d",
			    acr -> acr_reason));
	}

	ACRFREE (acr);
	PP_TRACE (("Association released"));

	return DONE;
}
