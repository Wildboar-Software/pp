/* pe_fragment.c: break up big pe_strings into smaller ones */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/pe_fragment.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/pe_fragment.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: pe_fragment.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include <isode/psap.h>

static int pe_frag_aux ();

int pe_fragment (pe, maxsize)
PE	pe;
int	maxsize;
{
	PE	p;

	switch (pe -> pe_form) {
	    case PE_FORM_ICONS:
		return OK;

	    case PE_FORM_CONS:
		switch (pe -> pe_id) {	/* dont refragment yet... */
		    case PE_PRIM_OCTS:
		    case PE_PRIM_BITS:
		    case PE_DEFN_IA5S:
			return OK;
		}

		for (p = pe -> pe_cons; p; p = p -> pe_next)
			if (pe_fragment (p, maxsize) == NOTOK)
				return NOTOK;

		return OK;

	    case PE_FORM_PRIM:
		return pe_frag_aux (pe, maxsize);

	}
	return OK;
}

static int pe_frag_aux (pe, maxsize)
PE	pe;
int	maxsize;
{
	char	*cp;
	PE	last, p;
	int	size, n;

	if (pe -> pe_len == PE_LEN_INDF)
		return OK;
	if (pe -> pe_len < maxsize)
		return OK;

	switch (pe -> pe_id) {
	    case PE_PRIM_OCTS:
	    case PE_DEFN_IA5S:
		cp = (char *) pe -> pe_prim;
		pe -> pe_form = PE_FORM_CONS;
		last = NULLPE;
		size = pe->pe_len;
		pe -> pe_len = 0;
		pe -> pe_prim = NULLPED;
		while (size > 0) {
			n = size > maxsize ? maxsize : size;
			p = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM,
				      PE_PRIM_OCTS);
			if (pe == NULLPE)
				return NOTOK;
			if ((p -> pe_prim = PEDalloc (n)) == NULLPED) {
				pe_free(p);
				return NOTOK;
			}
			p -> pe_len = n;
			PEDcpy (cp, p -> pe_prim, n);
			if (last == NULLPE) {
				if (seq_add (pe, p, -1) == NOTOK)
					return NOTOK;
			}
			else {
				if (seq_addon (pe, last, p) == NOTOK)
					return NOTOK;
			}
			cp += n;
			size -= n;
			last = p;
		}
		break;

	    default:
		break;
	}
	return OK;
}
		
		
