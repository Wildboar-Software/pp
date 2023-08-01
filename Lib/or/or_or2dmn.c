/* or_or2dmn: convert or list into domain form */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2dmn.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2dmn.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_or2dmn.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"

void fixup_space(str, dest)
char	*str;
char *dest;
{
	int 	seen_space = TRUE;
	char	*sp, *dp;

	for (sp = str, dp = dest; *sp; sp++) {
		if (*sp == ' ') {
			if (seen_space == FALSE)
				seen_space = TRUE;
			else
				continue;
		} else
			seen_space = FALSE;
		*dp++ = *sp;
	}
	if (dp > dest && dp[-1] == ' ')
	    dp --;
	if (dp == dest)
	    (void) strcpy (dest, " ");
	else
	    *dp = NULL;
}

void or_or2dmn (first, last, buf)
OR_ptr          first;
OR_ptr          last;
char            *buf;
{
    OR_ptr      or;

    PP_DBG (("or_util.c/or_or2dmn ()"));

    /*
    We print from BOTTOM of tree
    */

    if (last == NULLOR)
	or = or_lastpart (first);
    else
	or = last;

    buf[0] = '\0';

    for (; or != NULLOR; or = or -> or_prev ) {

	PP_DBG (("or_util.c/or_or2dmn Comp type='%d' ddname='%s' val='%s'",
		or -> or_type, or-> or_ddname, or -> or_value));

	if (buf[0] != '\0')
		(void) strcat (buf, ".");

	if (or -> or_type == OR_DD) {
		(void) strcat (buf, "~");
		dstrcat (buf, or -> or_ddname );
		(void) strcat (buf, "$");
		dstrcat (buf, or -> or_value);
	}
	else {
		char	*name;
		char tbuf[BUFSIZ];
		if ((name = or_type2name (or -> or_type)) != NULLCP)
			(void) strcat (buf, name);
		else
			(void) strcat (buf, "BOGUS");
		(void) strcat (buf, "$" );
		fixup_space (or -> or_value, tbuf);
		dstrcat (buf, tbuf);
	}
    }

    PP_DBG (("or_util.c/or_or2dmn Returns: '%s'", buf));
}
