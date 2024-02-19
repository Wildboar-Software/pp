/* or_valid.c: check to see that OR tree matches with standard */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_valid.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_valid.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_valid.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "or.h"

/* see table 10 in X.402 */
/* forms of O/R address */

static int or_valid_term(), or_valid_post(), or_valid_mnem(), or_valid_none(), or_valid_numr();

int or_valid_or (or)
OR_ptr	or;
{
	switch (or_form_type(or)) {
	    case OR_FORM_TERM:
		return or_valid_term(or);
	    case OR_FORM_POST:
		return or_valid_post(or);
	    case OR_FORM_NUMR:
		return or_valid_numr(or);
	    case OR_FORM_MNEM:
		return or_valid_mnem(or);
	    default:
		return or_valid_none(or);
	}
}

/*  */
static int or_valid_term(or)
OR_ptr	or;
{
	int	country, admd, x121, prmd, tid, dda;
	country = admd = x121 = prmd = tid = dda = 0;

#define too_many(x)	{ PP_LOG (LLOG_EXCEPTIONS, ("Too many %s components", (x))); return NOTOK; }

	while (or != NULLOR) {
		switch (or->or_type) {
		    case OR_C:
			if (country++ > 0)
				too_many("Country");
			break;
		    case OR_ADMD:
			if (admd++ > 0)
				too_many("ADMD");
			break;
		    case OR_X121:
			if (x121++ > 0)
				too_many ("X121");
			break;
		    case OR_PRMD:
			if (prmd++ > 0)
				too_many ("PRMD");
			break;
		    case OR_TID:
			if (tid++ > 0)
				too_many ("TID");
			break;
		    case OR_DD:
			if (dda++ > 3)
				too_many ("DD");
			break;
		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Invalid  component '%s=%s' for term form OR",
				or_type2name(or->or_type),
				or->or_value));
			return NOTOK;
		}
		or = or -> or_next;
	}

	if (x121 < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for term form OR",
			or_type2name(OR_X121)));
		return NOTOK;
	}
	return OK;
}

/**/
static int or_valid_post(or)
OR_ptr	or;
{
	int 	country, admd, prmd, pdsname, pd_c, postcode,
		or_comps, pd_comps, lpa, pdo_name, pdo_num, pd_o,
		pd_pn, po_box, pra, street, upa_pa, upn;

	country = admd = prmd = pdsname = pd_c = postcode
		= or_comps = pd_comps = lpa = pdo_name = pdo_num = pd_o
                = pd_pn = po_box = pra = street = upa_pa = upn = 0;

	while (or != NULLOR) {
		switch (or->or_type) {
		    case OR_C:
			country++;
			break;
		    case OR_ADMD:
			admd++;
			break;
		    case OR_PRMD:
			if (prmd++ > 0)
				too_many ("prmd");
			break;
		    case OR_PDSNAME:
			if (pdsname++ > 0)
				too_many ("pdsname");
			break;
		    case OR_PD_C:
			pd_c++;
			break;
		    case OR_POSTCODE:
			postcode++;
			break;
		    case OR_OR_COMPS:
			or_comps++;
			break;
		    case OR_PD_COMPS:
			pd_comps++;
			break;
		    case OR_LPA:
			lpa++;
			break;
		    case OR_PDO_NAME:
			pdo_name++;
			break;
		    case OR_PDO_NUM:
			pdo_num++;
			break;
		    case OR_PD_O:
			pd_o++;
			break;
		    case OR_PD_PN:
			pd_pn++;
			break;
		    case OR_PO_BOX:
			po_box++;
			break;
		    case OR_PRA:
			pra++;
			break;
		    case OR_STREET:
			street++;
			break;
		    case OR_UPA_PA:
			upa_pa++;
			break;
		    case OR_UPN:
			upn++;
			break;
		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Invalid  component '%s=%s' for postal form OR",
				or_type2name(or->or_type),
				or->or_value));
			return NOTOK;
		}
		or = or -> or_next;
	}

	if (country < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for postal form OR",
			or_type2name(OR_C)));
		return NOTOK;
	}

	if (admd < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for postal form OR",
			or_type2name(OR_ADMD)));
		return NOTOK;
	}

	if (pd_c < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for postal form OR",
			or_type2name(OR_PD_C)));
		return NOTOK;
	}

	if (postcode < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for postal form OR",
			or_type2name(OR_POSTCODE)));
		return NOTOK;
	}

	if (upa_pa == 1 &&
	    (or_comps > 0 || pd_comps > 0 || lpa > 0 || pdo_name > 0
	     || pdo_num > 0 || pd_o > 0 || pd_pn > 0 || po_box > 0
	     || pra > 0 || street > 0 || upn > 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Illegal postal components in an unformatted postal OR"));
		return NOTOK;
	}

	return OK;
}

/*  */
static int or_valid_numr(or)
OR_ptr	or;
{
	int	country, admd, prmd, uaid, dda;
	country = admd = prmd = uaid = dda = 0;

	while (or != NULLOR) {
		switch (or->or_type) {
		    case OR_C:
			country++;
			break;
		    case OR_ADMD:
			admd++;
			break;
		    case OR_PRMD:
			if (prmd++ > 0)
				too_many ("PRMD");
			break;
		    case OR_UAID:
			uaid++;
			break;
		    case OR_DD:
			if (dda++ > 3)
				too_many ("DD");
			break;
		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Invalid  component '%s=%s' for numeric form OR",
				or_type2name(or->or_type),
				or->or_value));
			return NOTOK;
		}
		or = or -> or_next;
	}

	if (country < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for numeric form OR",
			or_type2name(OR_C)));
		return NOTOK;
	}

	if (admd < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for numeric form OR",
			or_type2name(OR_ADMD)));
		return NOTOK;
	}

	if (uaid < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for numeric form OR",
			or_type2name(OR_UAID)));
		return NOTOK;
	}

	return OK;
}

/*  */
static int or_valid_mnem(or)
OR_ptr	or;
{
	int country, admd, prmd, cn, o, ou, s, g, i, gq, dda;
	country = admd = prmd = cn = o = ou = s = g = i = gq = dda = 0;

	while (or != NULLOR) {
		switch (or->or_type) {
		    case OR_C:
			country++;
			break;
		    case OR_ADMD:
			admd++;
			break;
		    case OR_CN:
			if (cn++ > 0)
				too_many ("CN");
			break;
		    case OR_PRMD:
			if (prmd++ > 0)
				too_many ("PRMD");
			break;
		    case OR_O:
			if (o++ > 0)
				too_many ("O");
			break;
		    case OR_OU:
		    case OR_OU1:
		    case OR_OU2:
		    case OR_OU3:
		    case OR_OU4:
			if (ou++ > 3)
				too_many("OU");
			break;
		    case OR_S:
			s++;
			break;
		    case OR_G:
			g++;
			break;
		    case OR_I:
			i++;
			break;
		    case OR_GQ:
			gq++;
			break;
		    case OR_DD:
			if (dda++ > 3)
				too_many ("DD");
			break;
		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Invalid  component '%s=%s' for mnemonic form OR",
				or_type2name(or->or_type),
				or->or_value));
			return NOTOK;
		}
		or = or -> or_next;
	}

	if (country < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for mnemonic form OR",
			or_type2name(OR_C)));
		return NOTOK;
	}

	if (admd < 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory component '%s' for mnemonic form OR",
			or_type2name(OR_ADMD)));
		return NOTOK;
	}

	if (s == 0 &&
	    (g > 0 || i > 0 || gq > 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory surname component '%s' for personal name",
			or_type2name(OR_S)));
		return NOTOK;
	}

	return OK;
}
/*  */
static int or_valid_none(or)
OR_ptr	or;
{
	int s, g, i, gq;
	s = g = i = gq = 0;

/* just check personal name */
	while (or != NULLOR) {
		switch (or->or_type) {
		    case OR_S:
			s++;
			break;
		    case OR_G:
			g++;
			break;
		    case OR_I:
			i++;
			break;
		    case OR_GQ:
			gq++;
			break;
		    default:
			/* don't know what form */
			break;
		}
		or = or -> or_next;
	}

	if (s == 0 &&
	    (g > 0 || i > 0 || gq > 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Missing mandatory surname component '%s' for personal name",
			or_type2name(OR_S)));
		return NOTOK;
	}

	return OK;
}
#undef too_many
