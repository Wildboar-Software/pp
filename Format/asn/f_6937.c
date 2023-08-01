/* f_6937.c: motis-86-6937 Encoder/Decoder Function Routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/f_6937.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/f_6937.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: f_6937.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */




#include "head.h"
#include "IOB-types.h"
#include "asn.h"






/* ------------------------  Start Routines --------------------------------- */




decode_motis_6937(body)
ASNBODY		**body;
{
	PE	pe;
	int	retval;
	struct type_IOB_ISO6937Data *asndata;

	PP_TRACE (("decode_motis6937()"));	

	pe = asn_rd_stdin (IGNORE_PRMS);

	retval = decode_IOB_ISO6937Data (pe, 1, NULLIP, NULLVP, &asndata);

	if (retval == NOTOK)
		pe_done (pe);

	pe_free (pe);

	/* -- set ASNBODY -- */
	struct2body (asndata, body);

	free_IOB_ISO6937Data (asndata);
}




encode_motis_6937(ppe, body)
PE	*ppe;
ASNBODY	*body;
{
	int	retval = NOTOK;
	PE	pe;
	struct type_IOB_ISO6937TextBodyPart	*info = NULL;

	PP_TRACE (("encode_motis6937()"));	

	/* -- set Data structure -- */
	set_6937_data (&info, body);

	retval = encode_IOB_ISO6937TextBodyPart (&pe, 1, NULLIP, NULLVP, info);
	if (retval == NOTOK)
		pe_done (pe);	/* -- exit -- */

	pe -> pe_class	= PE_CLASS_CONT;
	pe -> pe_id	= 13;
	*ppe = pe;

	free_IOB_ISO6937TextBodyPart (info);
	return OK;
}




/* -----------------------  Static  Routines  ------------------------------- */




static set_6937_data (dstruct, body)
struct type_IOB_ISO6937TextBodyPart	**dstruct;
ASNBODY		*body;
{
	struct type_IOB_ISO6937TextBodyPart	*tbptr;


	PP_TRACE (("set_6937_data()"));

	/* -- malloc main structure -- */
	tbptr = (struct type_IOB_ISO6937TextBodyPart *)
		smalloc (sizeof *tbptr);
	bzero (tbptr, sizeof *tbptr);

	tbptr -> parameters = (struct type_IOB_ISO6937Parameters *)
		smalloc (sizeof *tbptr -> parameters);
	bzero (tbptr -> parameters, sizeof *tbptr -> parameters);

	body2struct (body, &tbptr -> data);
	*dstruct = tbptr;
}
