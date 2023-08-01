/* asn_dec.c - decode ASN after reading from stdin */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_dec.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_dec.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: asn_dec.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"asn.h"

#define SLEN	128







/* ------------------------  Start Routine  --------------------------------- */




asn_decode(func_decode, data)
int	(*func_decode) ();
ASNBODY	**data;
{
	int		retval;

	PP_TRACE (("asn_decode()"));

	/* --- *** ---
	get body part from stdin this can be plain or wrapped
	if ffunc is set then it is wrapped.
	--- *** --- */

	if (func_decode == NULL) {
		rd_stdin (data);	
		return;
	}

	retval = (*func_decode)(data);
}




/* ------------------------  Static Routines  ------------------------------- */




static rd_stdin (data)
ASNBODY		**data;
{
	int	len;
	char	*buf;

	PP_TRACE (("rd_stdin()"));

	buf = smalloc (SLEN);
	bzero (buf, SLEN);

	while ((len = read (0, buf, SLEN)) > 0) {
		buf[len] = '\0';
		asnbody_add (data, buf, len);

		buf = smalloc (SLEN);
		bzero (buf, SLEN);
	}	

	free (buf);
}
