/* or_or2std.c: convert interanl list to standard format or name */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2std.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2std.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_or2std.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"

void or_or2std (or, buf, nicepn)
OR_ptr          or;
char            *buf;
int             nicepn;   /* pretty pn required ? */
{
	OR_ptr		ptr;
	char		tbuf[LINESIZE];
	int		gotpn,
			addslash;


	PP_DBG (("or_or2std ()"));

	buf[0] = '\0';

	if (nicepn) {
		gotpn = or_getpn (or, buf);
		if (!gotpn)
			buf[0] = '\0';
	}
	else
		gotpn = FALSE;

	ptr = or_lastpart (or);

	for (addslash = FALSE ; ptr != NULLOR; ptr = ptr -> or_prev ) {

		PP_DBG ((
		"or_or2std: Comp type='%d' ddname='%s' value='%s'",
			 ptr -> or_type, 
			 ptr-> or_ddname ? ptr -> or_ddname : "<none>", 
			 ptr -> or_value ? ptr -> or_value : "<none>"
		));

		if (gotpn && (ptr -> or_type == OR_S ||
				ptr -> or_type == OR_G ||
				ptr -> or_type == OR_I))
					continue;

		addslash = TRUE;

		(void) strcat (buf, "/");

		if (ptr -> or_type == OR_DD) {
	    		if (or_ddvalid_chk (ptr -> or_ddname, &tbuf[0]) == NOTOK)
			{
				(void) strcat (buf, "DD.");
				qstrcat (buf, ptr -> or_ddname );
		    	}
	    		else
				qstrcat (buf, tbuf);

	    		(void) strcat (buf, "=");
	    		qstrcat (buf, ptr -> or_value);
		}
		else {
			char	*name;
			if ((name = or_type2name (ptr -> or_type)) != NULLCP)
				(void) strcat (buf, name);
			else 
				(void) strcat (buf, "BOGUS");
	    		(void) strcat (buf, "=" );
	    		qstrcat (buf, ptr -> or_value);
		}
	}

	if (addslash)
		(void) strcat (buf, "/");

	PP_DBG (("or_or2std returns ('%s')", buf));
}
