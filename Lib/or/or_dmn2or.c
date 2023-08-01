/* or_dmn2or: convert dmn form to or list */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_dmn2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_dmn2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_dmn2or.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"

OR_ptr or_dmn2or (istr)
char            *istr;
{
    OR_ptr      tree,
		part;
    char        *sargv[50],
		*valptr,
		str[LINESIZE];
    int         count,
		ortype,
		sargc;

    PP_DBG (("or_dmn2or (%s)", istr));

    (void) strcpy (str, istr);

    /*
    sstr2arg isn't nice, add cheks for legal char set
    */

    tree = NULLOR;
    sargc = sstr2arg (str, 49, sargv, ".");

    for (count = 0; count < sargc; count++) {

	PP_DBG (("or_dmn2or Component '%s'", sargv[sargc] ? sargv[sargc] : "<none>"));

	valptr = index (sargv[count], '$');

	if (valptr == NULLCP) {
	    PP_LOG (LLOG_EXCEPTIONS,
			("or_dmn2or No dollar in dmn encoding"));
	    or_free (tree);
	    return NULLOR;
	}

	*valptr = '\0';
	valptr++;

	/*
	domain defined
	*/

	if (sargv[count][0] == '~') {
		part = or_new (OR_DD, &sargv[count][1], valptr);
		if ((tree = or_add (tree, part, TRUE)) == NULLOR)
			return NULLOR;
	}
	else {
		ortype = or_name2type (sargv[count]);

		if (ortype == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("or_dmn2or Unknown type '%s'",
				sargv[count]));
			or_free (tree);
			return NULLOR;
		}

		part = or_new (ortype, NULLCP, valptr);
		if ((tree = or_add (tree, part, TRUE)) == NULLOR)
			return NULLOR;
	}

    }  /* end of for */

    return (tree);
}
