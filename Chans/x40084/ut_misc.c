/* ut_misc.c - Other miscellaneous routines */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40084/RCS/ut_misc.c,v 6.0 1991/12/18 20:13:50 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/ut_misc.c,v 6.0 1991/12/18 20:13:50 jpo Rel $
 *
 * $Log: ut_misc.c,v $
 * Revision 6.0  1991/12/18  20:13:50  jpo
 * Release 6.0
 *
 */


#include	"util.h"
#include	<isode/cmd_srch.h>
#include	"tb_bpt84.h"
#include	"list_bpt.h"


extern CMD_TABLE	bptbl_body_parts84 [/* body part types */];



/* ---------------------  Begin  Routines  -------------------------------- */



int mem2enctypes (lptr)   /* Memory -> EncodedInformationTypesBitString */
LIST_BPT		*lptr;
{
	int		i,
			field = 0;
	LIST_BPT	*list;


	PP_TRACE (("x40084/mem2enctypes ()"));

	for (list = lptr; list; list = list->li_next) {

		PP_TRACE (("x40084/mem2enctypes (%s)", list->li_name));

		if (strncmp (list->li_name, "hdr.", 4) == 0)
			continue;

		switch (cmd_srch (list->li_name, bptbl_body_parts84)) {
		default:
		case BPT_UNDEFINED:
			/* --- if not found then set to undefined -- */
			i = 0; 
			break;
		case BPT_TLX:
			i = 1;
			break;
		case BPT_IA5:
			i = 2;
			break;
		case BPT_G3FAX:
			i = 3;
			break;
		case BPT_TIF0:
			i = 4;
			break;
		case BPT_TTX:
			i = 5;
			break;
		case BPT_VIDEOTEX:
			i = 6;
			break;
		case BPT_VOICE:
			i = 7;
			break;
		case BPT_SFD:
			i = 8;
			break;
		case BPT_TIF1:
			i = 9;
			break;
		}

		field |= (1 << i);
	}


	PP_TRACE (("x40084/mem2enctypes (%d)", field));

	return (field);
}
