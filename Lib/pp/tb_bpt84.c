/* tb_bp.c: table of body parts see manual page BODYPARTS (5) */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_bpt84.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_bpt84.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_bpt84.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include <isode/cmd_srch.h>
#include "tb_bpt84.h"


CMD_TABLE  bptbl_body_parts84 [] = {  /* PP Body parts */

	/*
	Unique body part types
	*/

	"deliv.p2-txt",      BPT_P2_DLIV_TXT,
	"hdr.ipn",		BPT_HDR_IPN,
	"hdr.p2",		BPT_HDR_P2,
	"hdr.822",		BPT_HDR_822,

	/*
	Repeated body part types
	*/

	"encrypted",            BPT_ENCRYPTED,
	"g3fax",                BPT_G3FAX,
	"ia5",                  BPT_IA5,
	"ipm",                  BPT_IPM,
	"national",             BPT_NATIONAL,
	"sfd",                  BPT_SFD,
	"tif0",                 BPT_TIF0,
	"tif1",                 BPT_TIF1,
	"tlx",                  BPT_TLX,
	"ttx",                  BPT_TTX,
	"undefined",            BPT_UNDEFINED,
	"videotex",             BPT_VIDEOTEX,
	"voice",                BPT_VOICE,
	"odif",			BPT_ODIF,
	0,                      -1
	};
