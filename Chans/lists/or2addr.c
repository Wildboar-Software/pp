/* or2addr.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/or2addr.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/or2addr.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: or2addr.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include "retcode.h"
#include "or.h"
#include "adr.h"
#include <isode/quipu/attrvalue.h>

extern char * dn2str ();

static ADDR *OR_Ptr2ADDR (or,parse,orig) 
OR_ptr or;
char parse;
char orig;
{
        ADDR * ptr;
        char buf[BUFSIZ];
        RP_Buf rp;

	or_or2std(or,buf,0);

	ptr = adr_new (buf,AD_X400_TYPE,1);
	
	if (orig) {
	  ptr->ad_resp = FALSE;
	  ptr->ad_status = AD_STAT_DONE;
	}

        if (parse && (rp_isbad(ad_parse(ptr, &rp, CH_USA_ONLY))))
                return NULLADDR;

	return ptr;

}

ADDR *dn2ADDR (dn,orig) 
DN dn;
char orig;
{
Attr_Sequence as;
OR_ptr newor, as2or ();
ADDR * ptr;

	if (dn2addr (dn, &as) != OK)
		return NULLADDR;

	newor = as2or (as);
	if (newor) {
		if ((ptr = OR_Ptr2ADDR (newor,TRUE,orig)) == NULLADDR)
			return NULLADDR;
		if (dn)
			ptr->ad_dn = dn2str (dn);

/*
		if (as->attr_value->avseq_next != NULLAV) {
			newor = (OR_ptr) as->attr_value->avseq_next->avseq_av.av_struct;
			or_or2std(newor,buf,0);
			ptr->ad_orig_req_alt = strdup (buf);
		}
*/

		return ptr;
	} 
	return NULLADDR;
}

ADDR *ORName2ADDR (or,dnlookup) 
ORName *or;
char dnlookup;	/* allow DN look up if addr missing */
{
ADDR * ptr;

	if (or->on_or) {
		ptr = OR_Ptr2ADDR (or->on_or,FALSE,FALSE);
		if (or->on_dn)
			ptr->ad_dn = dn2str (or->on_dn);
	} else if (or->on_dn) {
		if (dnlookup) 
			if ((ptr = dn2ADDR(or->on_dn,FALSE)) != NULLADDR)
				return ptr;

		ptr = (ADDR *) smalloc (sizeof(ADDR));
		adr_init (ptr);
		ptr->ad_type = AD_ANY_TYPE;
		ptr->ad_dn = dn2str (or->on_dn);
	}

	return ptr;
}


orAddr_check (or,buf)
OR_ptr or;
char * buf;
{
ADDR * ad;
RP_Buf rp;
char adr[BUFSIZ];

	or_or2std(or,adr,0);

	ad = adr_new (adr,AD_X400_TYPE,1);

	if (rp_isbad(ad_parse(ad, &rp, CH_USA_ONLY))) {
		(void) strcpy (buf,ad->ad_parse_message);
		return NOTOK;
	}

	adr_free (ad);

	return OK;
}

