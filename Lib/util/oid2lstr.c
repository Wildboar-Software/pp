/* oid2lstr.c: oid to labeled string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/oid2lstr.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/oid2lstr.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: oid2lstr.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include <isode/quipu/oid.h>

char	*oid2lstr(oid)
OID	oid;
{
	static char	buf[BUFSIZ];
	char	tmp[BUFSIZ];
	char	*oidstr, *num, *ix, *label;
	int	getLabel;

	num = oidstr = sprintoid(oid);

	if ('\0' == oidstr[0])
		return NULLCP;

	buf[0] = '\0';
	getLabel = TRUE;

	while (NULLCP != num) {
		if ((ix = index(num, '.')) != NULLCP)
			*ix = '\0';
		if (TRUE == getLabel) {
			if (num == oidstr) {
				/* first one put hack in */
				switch (atoi(num)) {
				    case 0:
					(void) strcpy(tmp, "ccitt (0)");
					break;
				    case 1:
					(void) strcpy(tmp, "iso (1)");
					break;
				    case 2:
					(void) strcpy(tmp, "joint (2)");
					break;
				    default:
					PP_LOG(LLOG_EXCEPTIONS,
					       ("First element of OID not ccitt, iso or joint"));
					(void) sprintf(tmp, "(%d)", atoi(num));
					getLabel = FALSE;
				}
			} else {
				OID	toid = oid_cpy(str2oid(oidstr));
				label = oid2name (toid, OIDPART);
				oid_free(toid);
				if (index(label, '.') != NULLCP) {
					getLabel = FALSE;
					(void) sprintf (tmp, "(%d)", atoi(num));
				} else 
					(void) sprintf(tmp, "%s (%d)",
						label,
						atoi(num));
			}
		} else
			(void) sprintf (tmp, "(%d)", atoi(num));
		if (ix != NULLCP) {
			*ix = '.';
			num = ix + 1;
		} else
			num = NULLCP;
		if ('\0' != buf[0])
			(void) strcat(buf, " ");
		(void) strcat(buf, tmp);
	}

	return buf;
}
	
