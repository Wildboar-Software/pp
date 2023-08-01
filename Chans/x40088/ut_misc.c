/* ut_misc.c - Other miscellaneous routines */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/ut_misc.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/ut_misc.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: ut_misc.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */


#include	"util.h"
#include	<isode/cmd_srch.h>
#include	"tb_bpt88.h"
#include	"list_bpt.h"


extern CMD_TABLE	bptbl_body_parts88 [/* body part types */];



/* ---------------------  Begin  Routines  -------------------------------- */



int bodypart2value (bodypart)   /* bodypart -> value */
char	*bodypart;
{

	PP_TRACE (("x40088/bodypart2value (%s)", bodypart));

	if (bodypart == NULLCP)
		return NOTOK;

	switch (cmd_srch (bodypart, bptbl_body_parts88)) {
	case BPT_UNDEFINED:
		return 0;
	case BPT_TLX:
		return 1;
	case BPT_IA5:
		return 2;
	case BPT_G3FAX:
		return 3;
	case BPT_TIF0:
		return 4;
	case BPT_TTX:
		return 5;
	case BPT_VIDEOTEX:
		return 6;
	case BPT_VOICE:
		return 7;
	case BPT_SFD:
		return 8;
	case BPT_TIF1:
		return 9;
	default:
		if (strncmp (bodypart, "oid.", 4) == 0)
			return 10;
	}

	return NOTOK;
}
