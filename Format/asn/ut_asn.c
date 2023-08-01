/* ut_asn.c - Common routines to manipulate ASNBODY */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/ut_asn.c,v 6.0 1991/12/18 20:16:07 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/ut_asn.c,v 6.0 1991/12/18 20:16:07 jpo Rel $
 *
 * $Log: ut_asn.c,v $
 * Revision 6.0  1991/12/18  20:16:07  jpo
 * Release 6.0
 *
 */




#include	"head.h"
#include	"asn.h" 




/* ------------------------  Start Routines --------------------------------- */




asnbody_add (base, value, len)
ASNBODY	**base;
char	*value;
int	len;
{
	ASNBODY		*ap = *base; 
	ASNBODY		*new;


	PP_TRACE (("asnbody_add (%d)", len));


	new = (ASNBODY *) smalloc (sizeof(ASNBODY));
	bzero ((char *) new, sizeof (*new));
	new -> line   = value;
	new -> length = len;


	if (ap == NULLASNBODY)
		*base = new;
	else {
		while (ap -> next != NULLASNBODY)
			ap = ap -> next;
		ap -> next = new;
	}
}




asnbody_free (data)
ASNBODY	*data;
{
	ASNBODY	*ap;

	for (; data; data = ap) {
		ap = data -> next;
		if (data -> line)
			free (data -> line);
		free ((char *)data);
	}
}
