/* syn_oraddr.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_oraddr.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_oraddr.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: syn_oraddr.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



/* LINTLIBRARY */

#include "util.h"
#include <varargs.h>
#include "retcode.h"
#include "adr.h"
#include "ap.h"
#include "dlist.h"
#include <isode/quipu/attrvalue.h>

int gl_addr_parse_full = 1; /* do full check */

OR_ptr orAddr_parse (ptr)
char * ptr;
{
OR_ptr or;
char * str;
ADDR * ad;
RP_Buf rp;

	if (gl_addr_parse_full == 0) {
		if ((or = or_std2or(ptr)) == NULLOR) {
			parse_error ("std2or failed %s",ptr);
			return (NULLOR);
		}
		return or;
	}

	ad = adr_new (ptr, (or_str_isps(ptr)) ? AD_X400_TYPE : AD_822_TYPE, 1);
	ad->ad_resp = NO;

#ifdef UKORDER
	(void) ad_parse(ad, &rp, CH_UK_PREF);
#else
	(void) ad_parse(ad, &rp, CH_USA_PREF);
#endif
	if (ad->ad_r400adr == NULLCP) {
		parse_error ("bad address %s",ad->ad_parse_message);
		return NULLOR;
	}

	if ((or = or_std2or(ad->ad_r400adr)) == NULLOR) {
		parse_error ("std2or failed %s",ad->ad_r400adr);
		adr_free (ad);
		return (NULLOR);
	}

	adr_free (ad);
	
	return (or);
}

OR_ptr orAddr_parse_user (ptr)
char * ptr;
{
OR_ptr or;
ADDR * ad;
RP_Buf rp;

	/* as above but normalize address */

	ad = adr_new (ptr, (*ptr == '/') ? AD_X400_TYPE : AD_822_TYPE, 0);
	ad->ad_resp = YES;

#ifdef UKORDER
	(void) ad_parse(ad, &rp, CH_UK_PREF);
#else
	(void) ad_parse(ad, &rp, CH_USA_PREF);
#endif
	if (ad->ad_r400adr == NULLCP) {
		parse_error ("bad address %s",ad->ad_parse_message);
		return NULLOR;
	}

	if ((or = or_std2or(ad->ad_r400adr)) == NULLOR) {
		parse_error ("std2or failed %s",ad->ad_r400adr);
		adr_free (ad);
		return (NULLOR);
	}

	adr_free (ad);
	
	return (or);
}

/* ARGSUSED */
orAddr_print (ps,or,format)
PS ps;
OR_ptr or;
int format;
{
char buf[LINESIZE];

/*
	if ((format == READOUT) || (format == UFNOUT))
		or_or2rfc (or,buf);
	else
*/
		or_or2std (or,buf,0);

	ps_printf (ps,"%s",buf);
}

OR_ptr or_cpy (a)
OR_ptr a;
{
OR_ptr top = NULLOR;

	for (; a != NULLOR; a=a->or_next) 
		top = or_add (top,or_dup(a),FALSE);

	return (top);
}

static int my_or_cmp (or1, or2)
OR_ptr          or1;
OR_ptr          or2;
{
int res;

    if (or1 -> or_type != or2 -> or_type) 
	return (or1 -> or_type > or2 -> or_type) ? 1 : -1;

    if ((res = lexequ (or1 -> or_value, or2 -> or_value)) != 0)
		return res;

    if (or1 -> or_type == OR_DD
	&& (res = lexequ (or1 -> or_ddname, or2 -> or_ddname)) != 0)
		return res;

    return 0;
}

orAddr_cmp (a,b)
OR_ptr a,b;
{
int res;

	for (; (a != NULLOR) && (b != NULLOR) ; a = a->or_next, b = b->or_next) 
		if ((res = my_or_cmp (a,b)) != 0) 
			return res;

	if (( a == NULLOR) && (b == NULLOR)) 
		return 0;
	else
		return (a == NULLOR) ? 1 : -1;
}

extern OR_ptr pe2ora();
extern PE ora2pe();

orAddr_syntax ()
{
	(void) add_attribute_syntax ("ORAddress",
		(IFP) ora2pe,	(IFP) pe2ora,
		(IFP) orAddr_parse,	orAddr_print,
		(IFP) or_cpy,		orAddr_cmp,
		(IFP) or_free,		NULLCP,
		NULLIFP,		TRUE);
}
