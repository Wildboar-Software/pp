/* asn_enc.c - encode ASN after body part conversion */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_enc.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_enc.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: asn_enc.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"asn.h"




/* ------------------------  Start Routine  --------------------------------- */




asn_encode(func_encode, data)
int	(*func_encode) ();
ASNBODY	*data;
{
	PE	pe;

	PP_TRACE (("asn_encode()"));

	/* --- *** ---
	write body part in plain or wrapped mode.
	if ffunc is set then wrapped.
	--- *** --- */


	if (func_encode == NULL) {
		wr_stdout (data);
		return;
	}
	

	/* -- call encoder function -- */
	(*func_encode)(&pe, data);

	asn_wr_stdout (pe);
	pe_free(pe);
}




/* ------------------------  Static Routines  ------------------------------- */




static asn_wr_stdout(pe)
PE	pe;
{
	register PS	ps;


	PP_TRACE (("asn_wr_stdout(%x)", pe));

	if (pe == NULLPE) {
		PP_LOG(LLOG_EXCEPTIONS, ("pe is null"));
		exit (1);
	}

	if ((ps = ps_alloc(std_open)) == NULLPS) {
		PP_LOG(LLOG_EXCEPTIONS, ("ps_alloc(stdout) error"));
		exit (1);
	}

	if (std_setup(ps, stdout) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS, ("std_setup(stdout) error"));	
		exit (1);
	}

	(void) ps_get_abs(pe);

	if (pe2ps(ps, pe) == NOTOK) {
		if (ps->ps_errno)
			PP_LOG(LLOG_EXCEPTIONS,
				("ps2pe: %s", ps_error(ps->ps_errno)));
		else 
			PP_LOG(LLOG_EXCEPTIONS, ("ps2pe error"));
		exit (1);
	}

	ps_free(ps);
}





static wr_stdout (data)
ASNBODY	*data; 
{
	ASNBODY	*dp;
	int	n;
	char	*c;

	PP_TRACE (("wr_stdout()"));

	for (dp = data; dp; dp = dp -> next)
		for (n = dp -> length, c = dp -> line; n > 0; n--, c++) 
			putchar (*c);
			
	fflush (stdout);
}
