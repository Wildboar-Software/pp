/* f_gentxt.c: General Text  Encoder/Decoder Function Routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/f_gentxt.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/f_gentxt.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: f_gentxt.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "asn.h"
#include "oid.h"
#include "asn1/GenTxt-types.h"




/* ------------------------  Start Routines --------------------------------- */




decode_gentext(body)
ASNBODY		**body;
{
	PE	pe;
	PE	pe_prm = NULLPE;
	PE	pe_data = NULLPE;
	int	retval;
	struct type_GenTxt_GenTxtData	*asndata;

	PP_TRACE (("decode_gentext()"));	

	pe = asn_rd_stdin (NULL);

	external2pe (pe, &pe_prm, &pe_data, GENERALTEXT);
	pe_free (pe_prm);

	retval = decode_GenTxt_GenTxtData (pe_data, 1, NULLIP,
						NULLVP, &asndata);
	if (retval == NOTOK)
		pe_done (pe_data);

	pe_free (pe_data);

	/* -- set ASNBODY -- */
	qbuf2body (asndata, body);

	free_GenTxt_GenTxtData (asndata);
}




encode_gentext(pe, body)
PE	*pe;
ASNBODY	*body;
{
	int					retval = NOTOK;
	PE					pe_data = NULLPE;
	PE					pe_prm = NULLPE;
	struct type_GenTxt_GenTxtBodyPart	*info = NULL;

	PP_TRACE (("encode_gentext()"));	

	/* -- set the Data structure -- */
	set_gentext_data (&info, body);

	retval = encode_GenTxt_GenTxtParameters (&pe_prm, 1, NULLIP,
						NULLVP, info -> parameters);
	if (retval == NOTOK)
		pe_done (pe_prm);	/* -- exit -- */

	retval = encode_GenTxt_GenTxtData (&pe_data, 1, NULLIP,
						NULLVP, info -> data);
	if (retval == NOTOK)
		pe_done (pe_data);	/* -- exit -- */

	free_GenTxt_GenTxtBodyPart (info);
	pe2external (pe, pe_prm, pe_data, GENERALTEXT); 
	return OK;
}




/* -----------------------  Static  Routines  ------------------------------- */




static set_gentext_data (dstruct, body)
struct type_GenTxt_GenTxtBodyPart	**dstruct;
ASNBODY		*body;
{
	struct type_GenTxt_GenTxtBodyPart	*tbptr;
	struct type_GenTxt_GenTxtParameters	*parameters;


	PP_TRACE (("set_gentext_data()"));

	/* -- malloc main structure -- */
	tbptr = (struct type_GenTxt_GenTxtBodyPart *)
		smalloc (sizeof *tbptr);
	parameters = (struct type_GenTxt_GenTxtParameters *)
		smalloc (sizeof *parameters);

	bzero (tbptr, sizeof *tbptr);
	bzero (parameters, sizeof *parameters);

	tbptr -> parameters = parameters;
	body2qbuf (body, &tbptr -> data);	
	*dstruct = tbptr;
}
