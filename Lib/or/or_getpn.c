/* or_getpn.c: get personal name out of the tree */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_getpn.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_getpn.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_getpn.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"


extern void fixup_space();

/* --- *** ---
Get PN out of tree and put into buf
--- *** --- */


int or_getpn (or, buf)
OR_ptr			or;
char			*buf;
{
	OR_ptr		ptr;

	PP_DBG (("or_util.c/or_getpn ()"));
	buf[0] = '\0';


	if ((ptr = or_find (or, OR_G, NULLCP)) != NULLOR) {
		if ((int)strlen (ptr -> or_value) > 1
	    		&& index (ptr -> or_value, '.') == NULLCP) {
				(void) strcat (buf, ptr -> or_value);
				(void) strcat (buf, ".");
		}
		else
	    		return FALSE;
	}


	if ((ptr = or_find (or, OR_I, NULLCP)) != NULLOR) {
		char	*p,
			*q;

		q = &buf[strlen(buf)];
		for (p = ptr -> or_value; *p != '\0'; p++) {
			if (!isalpha (*p))
				return FALSE;
			*q++ = *p;
			*q++ = '.';
		}
		*q = '\0';
	}



	if ((ptr = or_find (or, OR_S, NULLCP)) != NULLOR) {
		if (index (ptr -> or_value, '.') != NULLCP) {
	    		/* Surname with "." - yuk */
	    		if (buf[0] == '\0'
				|| (int)strlen (ptr -> or_value) < 3
				|| ptr -> or_value[0] == '.'
				|| ptr -> or_value[1] == '.' )
					return FALSE;
		}
		(void) strcat (buf, ptr -> or_value);
	}
	else
		return FALSE;

	fixup_space(buf, buf);
	PP_DBG (("or_util.c/or_getpn: Got PN '%s'", buf));
	return TRUE;
}
