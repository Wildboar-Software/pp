/* opt_functs.c: sets the Encoder/Decoder Function Routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/opt_functs.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/opt_functs.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: opt_functs.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */




#include	"head.h"
#include	"asn.h"



typedef struct funct_tbl {
	char	*key;
	int 	(*func_encode) ();
	int	(*func_decode) ();
	} TBL_FUNCTS;


extern int	encode_gentext();
extern int	decode_gentext();
extern int	encode_teletex();
extern int	decode_teletex();
extern int	encode_motis_6937();
extern int	decode_motis_6937();


static TBL_FUNCTS  tbl_functs [] = {/* -- Encoder/Decoder of an asn1 -- */
	"ia5",			0,			0,
	"generaltext",		encode_gentext,		decode_gentext,
	"motis-86-6937",	encode_motis_6937,	decode_motis_6937,
	"teletex",		encode_teletex,		decode_teletex,
	"none",			0,			0,
	0,			0,			0
	};



#define IN	1
#define OUT	2



/* ------------------------  Start Routine  --------------------------------- */



opt_functs (ap)
ASNCMD	*ap;
{

	PP_TRACE (("opt_functs()"));

	if (ap->in_asn.name)
		tbl_srch (ap->in_asn.name, IN, &ap->in_asn.ffunc);

	if (ap->out_asn.name)
		tbl_srch (ap->out_asn.name, OUT, &ap->out_asn.ffunc);
}





/* ----------------------  Static  Routines  -------------------------------- */



static tbl_srch (str, type, base)
char	*str;
int	type;
int	(**base) ();
{
	TBL_FUNCTS	*tbl = tbl_functs;

	PP_TRACE (("tbl_srch (%s)", str));

	for(; tbl->key != NULLCP; tbl++)
		if(lexequ(str, tbl->key) == 0) {
			switch (type) { 
			case IN:
				*base = tbl->func_decode;
				return;
			case OUT:
				*base = tbl->func_encode;
				return;
			default:
				PP_LOG (LLOG_EXCEPTIONS,
					("Error: Unknown type '%d'", type));
				exit(1);
			}
		}

	PP_LOG (LLOG_EXCEPTIONS, ("Error: Unknown ASN1 '%s'", str));
	exit(1);
}
