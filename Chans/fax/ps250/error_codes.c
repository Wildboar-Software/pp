/* error_codes.c: error codes for the SystemFax 250 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/error_codes.c,v 6.0 1991/12/18 20:07:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/error_codes.c,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: error_codes.c,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 */
#include "util.h"
#include <isode/cmd_srch.h>

CMD_TABLE	fax_errors [] = { /* fax error codes */
	"rcv paper jam (check paper path)",		2,
	"rcv paper path (reset cutter blue wheel)",	4,
	"no recording paper (load paper)",		10,
	"thermal head temp high (reset machine)",	20,
	"tx jam (document edge faulty or ADF requires adjustment see manual", 30,
	"tx jam (document exceeds 1m long check length of original or paper path)", 31,
	"tx guide pulled forward, paper loading door open, or bottom door open", 60,
	"call failed (single go) busy line or wrong no.", 400,
	"line faulty or incompatible fax machine", 	402,
	"polling not set up at other end", 		403,
	"probable jam at other end",   			404,
	"bad line unable to transmit",			405,
	"other end fault (jam, too many errors)",	407,
	"far end has too many errors on rcv'd doc",	408,
	"far end has too many errors on rcv'd doc and voice request required", 409,
	"faulty line",					410,
	"polling password wrong",			411,
	"paper jam/document too long at far end",	412,
	"polling not on at other fax or code wrong",	414,
	"polling password not correct",			415,
	"faulty line or doc too long at other end",	416,
	"rcv too many errors on document",		417,
	"rcv too many errors on document, other end requests voice request", 418,
	"rcv too many errors on document, your end requests voice request", 419,
	"rcv'd call is not a fax call or line faulty",	420,
	"bad line or non compatible G3 machine",	422,
	"bad line or non compatible G3 machine",	427,
	"bad line retry",				430,
	"bad line retry",				432,
	"paper jam at other end",			436,
	"faulty line",					451,
	"faulty line or paper jam at other end",	458,
	"faulty line",					459,
	"faulty line or non standard G2 machine",	464,
	"faulty line or non standard G2 machine",	465,
	"faulty line or non standard G2 machine and paper jam at other end", 466,
	"faulty line or non standard G2 machine",	467,
	"faulty line or non standard G2 machine",	468,
	"paper jam other end or line faulty",		469,
	"faulty line or non standard G2 machine",	473,
	"faulty line or non standard G2 machine",	474,
	"faulty line or non standard G2 machine",	476,
	"faulty line or non standard G2 machine",	478,
	"paper jam other end or line fault",		479,
	"paper jam other end or line fault",		480,
	"line fault",					481,
	"line fault or machine fault",			483,
	"64 errored lines over error count",		490,
	"line fault or machine fault",			492,
	"paper jam other end or line fault",		493,
	"line fault or paper jam other end",		494,
	"paper jam other end or line fault",		495,
	"line fault",					540,
	"line fault or other end dropped out",		541,
	"fault other end or line fault",		542,
	"other end fault or line fault",		543,
	"line fault tx not possible",			544,
	"other end fault",				550,
	"other end fault",				552,
	"other fax abnormal end inc. stop button",	553,
	"line fault",					554,
	"line fault, voice request from other fax",	555,
	"document not on ADF or an auto retry",		623,
	"three dial attempts to one number failed",	630,
#ifdef WHAT_DOCS_SAY
	"info code returned to i/f when dial command fails at each attempt", 693,
#endif
	"dialing attempt failed", 693,
	0,					        -1
	};

char	fax_err_buf[BUFSIZ];

char	*faxerr2str(err)
int	err;
{
	char	*ret;

	if ((ret = rcmd_srch(err, fax_errors)) != NULLCP)
		(void) sprintf(fax_err_buf, "%s", ret);
	else if (err >= 0 && err < 100) 
		(void) sprintf(fax_err_buf, "mechanical problems");
	else if (err >= 400 && err < 500)
		(void) sprintf(fax_err_buf, "communication problems");
	else if (err >= 500 && err < 600)
		(void) sprintf(fax_err_buf, "CCITT error correction mode problems");
	else if (err >= 600 && err < 700)
		(void) sprintf(fax_err_buf, "autodialling problems");
	else
		(void) sprintf(fax_err_buf, "unknown error number '%d'", err);

	return fax_err_buf;
}
		

