/* or_buildpn: build a personal name from string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_buildpn.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_buildpn.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_buildpn.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"

/*
Takes string and builds a tree
*/

OR_ptr or_buildpn (str)
char            *str;
{
    OR_ptr      or,
    		tor,
		tree;
    char        tbuf[LINESIZE],
		*q,
		*r;

    PP_DBG (("or_util.c/or_buildpn (%s)", str));

    tree = NULLOR;

    if ((q = index (str, '.')) == NULLCP)
	/*
	Only a surname is specified
	*/
	q = str;
    else {
	if ((q - str) > 1) {
		/*
		A given name is specified
		*/
		*q++ = '\0';
		tree = or_new (OR_G, NULLCP, str);
	}
	else
	    q = str;

	/*
	Do the initials from q
	*/

	r = tbuf;

	while (TRUE) {
		if (!isalnum (*q) || (*(q + 1) != '.'))
			break;
		*r++ = *q;
		q = q + 2;
	}

	*r = '\0';

	if (tbuf[0] != '\0') {
		if ((or = or_new (OR_I, NULLCP, tbuf)) == NULLOR)
			return NULLOR;

		if ((tor = or_add (tree, or, TRUE)) == NULLOR) {
			or_free (or);
			return NULLOR;
		}
		tree = tor;
	}
    }

    /*
    q now points to start of surname
    */

    if ((or = or_new (OR_S, NULLCP, q)) == NULLOR)
	return NULLOR;
    tor = or_add (tree, or, TRUE);
    if (tor == NULLOR) {
	    or_free (tree);
	    return NULLOR;
    }
    tree = tor;
    return tree;
}
