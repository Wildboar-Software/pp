/* dl_permit.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl_permit.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl_permit.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: dl_permit.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include "adr.h"
#include "or.h"
#include <isode/quipu/attrvalue.h>

extern AttributeType at_Permit;
extern AttributeType at_GroupMember;
extern AttributeType at_Member;
extern ORName *addr2orname();

static check_group (addr,dn)
ADDR * addr;
DN dn;
{
Attr_Sequence as;
Attr_Sequence tmp;
AV_Sequence avs;
DN ad_dn;

	if (! addr->ad_dn)
		return NOTOK;

	if (read_group_entry (dn, &as) != OK)
		return NOTOK;

	if ((tmp = as_find_type(as,at_GroupMember)) == NULLATTR)
		return NOTOK;

	if ((ad_dn = str2dn (addr->ad_dn)) == NULLDN)
		return NOTOK;

	for (avs=tmp->attr_value; avs!= NULLAV; avs=avs->avseq_next)
		if (dn_cmp (ad_dn,(DN)avs->avseq_av.av_struct) == 0) {
			dn_free (ad_dn);
			return OK;
		}

	dn_free (ad_dn);
	return NOTOK;
}

static check_individual (addr,or)
ADDR * addr;
ORName * or;
{
ORName * newor;
int res;

	newor = addr2orname (addr);

	res = orName_cmp_user (or,newor);

	ORName_free (newor);

	return res; 
}

static check_member (addr,or)
ADDR * addr;
ORName * or;
{
Attr_Sequence tmp;
AV_Sequence avs;
Attr_Sequence as;

	if (or->on_dn == NULLDN) 
		return NOTOK;

	if (dir_getdl_aux (or->on_dn, &as) != OK)
		return NOTOK;

	if ((tmp = as_find_type(as,at_Member)) == NULLATTR)
		return NOTOK;

	for (avs=tmp->attr_value; avs!= NULLAV; avs=avs->avseq_next)
		if (check_individual (addr,(ORName *)avs->avseq_av.av_struct) == OK)
			return OK;

	return NOTOK;
}

static or_cmp_prefix (a,b)
OR_ptr a,b;
{
	for (; (a != NULLOR) && (b != NULLOR); b = b->or_next) {
		if (or_cmp (a,b) == TRUE)
			a = a->or_next;
	}

	if (a == NULLOR)
		return OK;
	else
		return NOTOK;
}

static check_pattern (addr,or)
ADDR * addr;
ORName * or;
{
ORName * newor;

	newor = addr2orname (addr);

	if (or->on_dn) {
		if (!newor->on_dn)
			goto out;
		if (dn_cmp_prefix (or->on_dn,newor->on_dn) == NOTOK)
			goto out;
	}

	if (or->on_or) {
		if (!newor->on_or)
			goto out;

		if (or_cmp_prefix (or->on_or,newor->on_or) == NOTOK)
			goto out;
	}

	ORName_free (newor);
	return OK;

out:;
	ORName_free (newor);
	return NOTOK;


}

check_dl_permission (addr, as)
ADDR * addr;
Attr_Sequence as;
{
Attr_Sequence tmp;
AV_Sequence loop;
struct dl_permit * ptr;
int res;

	if ((tmp = as_find_type (as,at_Permit)) == NULLATTR) 
		return NOTOK; 	/* Attribute is mandatory */

	for (loop= tmp->attr_value;loop != NULLAV;loop = loop->avseq_next) {

		ptr = (struct dl_permit *)loop->avseq_av.av_struct;

		switch (ptr->dp_type) {
		case DP_GROUP:
			res = check_group (addr,ptr->dp_dn);
			break;
		case DP_INDIVIDUAL:
			res = check_individual (addr,ptr->dp_or);
			break;
		case DP_MEMBER:
			res = check_member (addr,ptr->dp_or);
			break;
		case DP_PATTERN:
			res = check_pattern (addr,ptr->dp_or);
			break;
		}

		if (res == OK)
			return OK;
	}

	return NOTOK;
}
