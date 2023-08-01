/* f_teletex.c: teletex Encoder/Decoder Function Routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/f_teletex.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/f_teletex.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: f_teletex.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "IOB-types.h"
#include "asn.h"






/* ------------------------  Start Routines --------------------------------- */




decode_teletex(body)
ASNBODY		**body;
{
	PE	pe;
	int	retval;
	struct type_IOB_TeletexData	*asndata;

	PP_TRACE (("decode_teletex()"));	

	pe = asn_rd_stdin (IGNORE_PRMS);

	retval = decode_IOB_TeletexData (pe, 1, NULLIP, NULLVP, &asndata);

	if (retval == NOTOK)
		pe_done (pe);

	pe_free (pe);

	/* -- set ASNBODY -- */
	struct2body (asndata, body);

	free_IOB_TeletexData (asndata);
}




encode_teletex(ppe, body)
PE	*ppe;
ASNBODY	*body;
{
	int	retval = NOTOK;
	PE	pe;
	struct type_IOB_TeletexBodyPart	*info = NULL;

	PP_TRACE (("encode_teletex()"));	

	/* -- set the Data structure -- */
	set_teletex_data (&info, body);

	retval = encode_IOB_TeletexBodyPart (&pe, 1, NULLIP, NULLVP, info);
	if (retval == NOTOK)
		pe_done (ppe);	/* -- exit -- */

	pe -> pe_class = PE_CLASS_CONT;
	pe -> pe_id    = 5;
	*ppe = pe;

	free_IOB_TeletexBodyPart (info);
	return OK;
}




/* -----------------------  Static  Routines  ------------------------------- */




static set_teletex_data (dstruct, body)
struct type_IOB_TeletexBodyPart	**dstruct;
ASNBODY		*body;
{
	struct type_IOB_TeletexBodyPart	*tbptr;


	PP_TRACE (("set_teletex_data()"));

	/* -- malloc main structure -- */
	tbptr = (struct type_IOB_TeletexBodyPart *)
		smalloc (sizeof *tbptr);
	bzero (tbptr, sizeof *tbptr);

	tbptr -> parameters = (struct type_IOB_TeletexParameters *)
		smalloc (sizeof *tbptr -> parameters);
	bzero (tbptr -> parameters, sizeof *tbptr -> parameters);

	body2struct (body, &tbptr -> data);
	*dstruct = tbptr;
}
