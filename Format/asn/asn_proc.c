/* asn_proc - decode/read stdin, call filter, encode/output stdout */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_proc.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_proc.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: asn_proc.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"asn.h"




/* ------------------------  Start Routine  --------------------------------- */




/* --- *** ---- 
1)	Read plain or ASN1 - and decode if required.
2)	Call the body part filter 
3)	Encode if required - and output the info on stdout.
--- *** --- */ 


asn_process(ap)
ASNCMD	*ap;
{
	ASNBODY		*data;

	PP_TRACE (("asn_process(%x)", ap));

	data = NULLASNBODY;

	asn_decode (ap->in_asn.ffunc, &data);

	switch (lexequ (ap -> in_charset, ap -> out_charset)) {
	case 0:
		PP_NOTICE (("'%s' = '%s' - No character set conversion done",
			    ap->in_charset, ap->out_charset));
		break;
	default:
		asn_charset (ap->in_charset, ap->out_charset, data );
		break;
	}

	asn_encode (ap->out_asn.ffunc, data);
}
