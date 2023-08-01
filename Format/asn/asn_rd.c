/* asn_rd.c - reads ASN from stdin */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_rd.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/asn_rd.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: asn_rd.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"asn.h"



extern PE	ps2pe_aux();




/* ------------------------  Start Routine  --------------------------------- */




PE asn_rd_stdin (flag)
int	flag;
{
	register PE	pe;
	register PS	ps;


	PP_TRACE (("asn_rd_stdin()"));

	if ((ps = ps_alloc(std_open)) == NULLPS) {
		PP_LOG(LLOG_EXCEPTIONS, ("Error: ps_alloc(std_open)"));
		exit(1);
	}

	if (std_setup(ps, stdin) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS, ("Error: std_setup()"));
		exit(1);
	}


	if (flag == IGNORE_PRMS)
		ignore_parameters (ps);


	if ((pe = ps2pe(ps)) == NULLPE) {
		if (ps->ps_errno)
			PP_LOG(LLOG_EXCEPTIONS, ("Error: ps2pe: %s", 
						ps_error(ps->ps_errno)));
		else
			PP_LOG(LLOG_EXCEPTIONS, ("Error: ps2pe()")); 

		exit(1);	
	}

	ps_free(ps);
	return pe;
}




/* ------------------------  Static Routines  ------------------------------- */




static int ignore_parameters (psin)
PS	psin;
{

	PE	pe;

	PP_TRACE (("ignore_parameters()"));

	if ((pe = ps2pe_aux (psin, 1, 0)) == NULLPE) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Error: ps2pe_aux reading SEQUENCE info failed [%s]",
			ps_error(psin -> ps_errno)));
		exit (1);
	}


#ifdef DEBUG
	(void) testdebug (pe, "asnfilter/ignore_parameters (BEGIN)"); 
#endif

	PP_TRACE (("Ignoring (class=%d, id=%d)", pe -> pe_class, pe -> pe_id));
	pe_free (pe);


	if ((pe = ps2pe_aux (psin, 1, 0)) == NULLPE) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Error: ps2pe_aux reading SET info failed [%s]",
			ps_error(psin -> ps_errno)));
		exit (1);
	}



#ifdef DEBUG
	(void) testdebug (pe, "asnfilter/ignore_parameters (SET)");
#endif


	if (PE_ID(pe -> pe_class, pe -> pe_id) !=
	    PE_ID(PE_CLASS_UNIV, PE_CONS_SET)) {
		PP_LOG(LLOG_EXCEPTIONS, ("Unexpected pe: expected a SET"));
		exit (1);
	}


	PP_TRACE (("Ignoring (class=%d, id=%d)", pe -> pe_class, pe -> pe_id));
	pe_free (pe);
}
