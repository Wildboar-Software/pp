/* lstr2oid.c: labelled string -> OID */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/lstr2oid.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/lstr2oid.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: lstr2oid.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include <isode/quipu/oid.h>

OID	lstr2oid (str)
char	*str;
{
	char	buf[BUFSIZ];
	char	*ix;
	int	iy;
	int	copy = FALSE;

	for (ix = str, iy = 0;
	     NULLCP != ix && '\0' != *ix;
	     ix++) {
		if ('(' == *ix) {
			copy = TRUE;
			if (iy != 0)
				buf[iy++] = '.';
		} else if (')' == *ix) 
			copy = FALSE;
		else if (TRUE == copy)
			buf[iy++] = *ix;
	}
	buf[iy] = '\0';
	
	return str2oid(buf);
}
