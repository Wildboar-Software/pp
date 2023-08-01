/* rp_valstr: convery values to strings */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/rp_valstr.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/rp_valstr.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: rp_valstr.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include        "head.h"

char    *
rp_valstr (val)           /* return text string for reply value */
int     val;
{
	static char noval[] = "*** Illegal:  0000";
				  /* (noval[0] == '*') => illegal       */

	switch (val){
	case RP_DONE:
		return ("DONE");
	case RP_OK:
		return ("OK");
	case RP_MOK:
		return ("MOK");
	case RP_HOK:
		return ("HOK");
	case RP_DOK:
		return ("DOK");
	case RP_MAST:
		return ("MAST");
	case RP_SLAV:
		return ("SLAV");
	case RP_AOK:
		return ("AOK");
	case RP_NET:
		return ("NET");
	case RP_BHST:
		return ("BHST");
	case RP_DHST:
		return ("DHST");
	case RP_LIO:
		return ("LIO");
	case RP_NIO:
		return ("NIO");
	case RP_LOCK:
		return ("LOCK");
	case RP_EOF:
		return ("EOF");
	case RP_NS:
		return ("NS");
	case RP_AGN:
		return ("AGN");
	case RP_PARSE:
		return ("PARSE");
	case RP_TIME:
		return ("TIME");
	case RP_NOOP:
		return ("NOOP");
	case RP_FIO:
		return ("FIO");
	case RP_FCRT:
		return ("FCRT");
	case RP_PROT:
		return ("PROT");
	case RP_RPLY:
		return ("RPLY");
	case RP_MECH:
		return ("MECH");
	case RP_NO:
		return ("NO");
	case RP_NDEL:
		return ("NDEL");
	case RP_HUH:
		return ("HUH");
	case RP_NCMD:
		return ("NCMD");
	case RP_PARM:
		return ("PARM");
	case RP_UCMD:
		return ("UCMD");
	case RP_USER:
		return ("USER");
	case RP_FOPN:
		return ("FOPN");
	case RP_NAUTH:
		return ("NAUTH");
	case RP_BAD:
		return ("BAD");
	default:
		break;
	}
					  /* print illegal octal value */
	noval[15] = rp_gbbit (val) + '0';
	noval[16] = rp_gcbit (val) + '0';
	noval[17] = rp_gsbit (val) + '0';
	return (noval);
}
