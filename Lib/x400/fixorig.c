/* fixorig.c: routine to lie about originator address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/x400/RCS/fixorig.c,v 6.0 1991/12/18 20:25:37 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/x400/RCS/fixorig.c,v 6.0 1991/12/18 20:25:37 jpo Rel $
 *
 * $Log: fixorig.c,v $
 * Revision 6.0  1991/12/18  20:25:37  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "adr.h"
#include "or.h"

x400_fixorig (orig, rewrite)
ADDR	*orig;
char	*rewrite;
{
	OR_ptr or, or1, or2;
	OR_ptr o1, o2, o3;
	char	*p;
	char	buf[BUFSIZ];

	if ((p = index (rewrite, '-')) == NULL ||
	    *(p+1) != '>') {
		PP_LOG (LLOG_EXCEPTIONS, ("rewrite rule \"%s\" missing ->",
					  rewrite));
		return;
	}

	*p = '\0';
	if ((or = or_std2or(orig -> ad_r400adr)) == NULLOR)
		return;

	if ((or1 = or_std2or (rewrite)) == NULLOR ||
	    (or2 = or_std2or (p + 2)) == NULLOR) {
		*p = '-';
		PP_LOG (LLOG_EXCEPTIONS, ("Error in or str '%s'", rewrite));
		return;
	}
	*p = '-';

	for (o1 = or, o2 = or1, o3 = or2; o2 && o1 && o3;
	     o1 = o1 -> or_next, o2 = o2 ->or_next, o3 = o3 -> or_next) {
		if (or_cmp (o1, o2)) {
			if (o1 -> or_type == o3 -> or_type) {
				free (o1 -> or_value);
				o1 -> or_value = strdup (o3 -> or_value);
			}
		}
	}
	if (o2 == NULLOR && o3 == NULLOR) { /* we matched! */
		or_or2std (or, buf, 0);
		if (buf[0] != '\0') {
			free (orig -> ad_r400adr);
			orig -> ad_r400adr = strdup (buf);
		}
	}
	or_free (or);
	or_free (or1);
	or_free (or2);
}
