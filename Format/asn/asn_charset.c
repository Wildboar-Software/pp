/* asn_charset.c - calls strncnv to perform the character set conversions */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_charset.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_charset.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: asn_charset.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include  "head.h"
#include  "charset.h"
#include  "asn.h"

static CHARSET	*Iset, *Oset;
extern int	MnemonicsRequired;



/* ------------------------  Start Routine  --------------------------------- */




asn_charset(inset, outset, body)
char		*inset;		/* -- in character set -- */
char		*outset;	/* -- out character set -- */
ASNBODY		*body;		/* -- body to be converted -- */
{
	ASNBODY	*bp, *bak;

	PP_TRACE(("asn_charset (%s, %s)", inset, outset));

	if ((Iset = getchset (inset, DEFAULT_ESCAPE)) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Unknown input charset %s", inset));
		exit (1);
	}

	if ((Oset = getchset (outset, DEFAULT_ESCAPE)) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Unknown output charset %s", outset));
		exit (1);
	}


	for (bp = body; bp; bp = bp -> next)
		call_strncnv (bp); 
}




/* ------------------------  Static Routines  ------------------------------- */




static call_strncnv (bp)
ASNBODY	*bp;
{
	char		*cp;
	int		blen, length; 

	PP_TRACE (("call_strncnv (%d)", bp -> length));

	if (bp -> length == 0 && bp -> line == 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("Error unable to convert"));
		exit (1);
	}
		
	blen = (bp -> length * 3) + 4;
	cp = malloc (blen);
	bzero (cp, blen);

	switch (MnemonicsRequired) {
	case TRUE:
		length = strncnv  (Oset, Iset, (CHAR8U*)cp, 
				  (CHAR8U*) bp->line, blen, TRUE);
		break;
	default:
		length = strncnv (Oset, Iset, (CHAR8U*)cp, 
				 (CHAR8U*) bp->line, bp -> length, FALSE);
		break;
	}

	free (bp -> line);
	bp -> line = realloc (cp, length);
	bp -> length = length;
}
