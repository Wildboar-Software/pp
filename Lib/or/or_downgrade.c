/* or_downgrade: downgrade an 88 address to an 84 address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_downgrade.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_downgrade.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_downgrade.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */

#include	"util.h"
#include	"or.h"
#include	"ap.h"

extern char	*loc_or;

static int is84component(or)
OR_ptr	or;
{
	typestruct	*ptr;

	if (or->or_encoding == OR_ENC_PS
	    || or->or_encoding == OR_ENC_NUM) {
		for (ptr = typetab; (ptr -> ty_string) != NULLCP; ptr++)
			if (or->or_type == ptr->ty_int)
				return TRUE;
	}
	return FALSE;
}

void or_downgrade(ptree)
OR_ptr	*ptree;
{
	OR_ptr	ix = *ptree, ret, cn = NULLOR;
	int	x40088 = FALSE;

	while (ix != NULLOR && x40088 == FALSE) {
		if (is84component(ix) != TRUE) {
			if (ix -> or_type == OR_CN) {
				cn = ix;
				ix = ix -> or_next;
			} else
				x40088 = TRUE;
		} else 
			ix = ix -> or_next;
	}
	
	if (cn != NULLOR && x40088 != TRUE) {
		/* CN sole x40088 comp */
		char	buf[BUFSIZ], *sep;

		/* convert cn to DD.Common */
		switch(cn->or_encoding) {
		    case OR_ENC_PS:
			(void) strcpy(buf, cn->or_value);
			break;

		    case OR_ENC_TTX:
		    case OR_ENC_TTX_AND_OR_PS:
			if ((sep = index(cn->or_value, '*')) == NULLCP)
				(void) strcpy (buf, cn->or_value);
			else {
				*sep = '\0';
				if (sep != cn->or_value)
					/* ps encoding */
					(void) strcpy (buf, cn->or_value);
				else 
					/* ttx encoding */
					(void) or_asc2ps (sep+1, buf);
			}
			break;

		    default:
			/* give up on DD common */
			cn = NULLOR;
			x40088 = TRUE;
		}

		if (cn != NULLOR) {
			/* remove cn */
			if (cn->or_prev)
				cn->or_prev->or_next = cn->or_next;
			if (cn->or_next)
				cn->or_next->or_prev = cn->or_prev;
			if (cn == *ptree)
				*ptree = cn->or_next;
		
			cn->or_next = NULLOR;
			cn->or_prev = NULLOR;
			or_free(cn);

			*ptree = or_add(*ptree,
					or_new_aux (OR_DD, 
						    "Common", 
						    buf, OR_ENC_PS),
					TRUE);
		}
	}
	
	if (x40088 == TRUE) {
		char	lbuf[LINESIZE],
			dbuf[LINESIZE],
			*cp;
		AP_ptr	local,
			domain;
		OR_ptr	ptr;
				
		if (or_or2rbits(*ptree, lbuf, dbuf) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("or_downgrade: or_or2rbits failed"));
			return;
		}

		PP_TRACE (("or_downgrade lbuf='%s' dbuf='%s'",
			   lbuf, dbuf));

		local = ap_new (AP_MAILBOX, lbuf);
		domain = ap_new (AP_DOMAIN, dbuf);

		cp = ap_p2s(NULLAP, NULLAP, local, domain, NULLAP);
		
		
		ap_free(local);
		ap_free(domain);

		(void) or_asc2ps (cp, lbuf);
		
		ptr = or_new_aux (OR_DD, "X400-88", lbuf, OR_ENC_PS);

		ret = or_std2or(loc_or);
		ret = or_add(ret, ptr, TRUE);
		or_free(*ptree);
		*ptree = ret;
	}

}
	
		

