/* enctypes2mem.c - Stores X.400 encoded types into PP internal text form */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/x400/RCS/enctypes2mem.c,v 6.0 1991/12/18 20:25:37 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/x400/RCS/enctypes2mem.c,v 6.0 1991/12/18 20:25:37 jpo Rel $
 *
 * $Log: enctypes2mem.c,v $
 * Revision 6.0  1991/12/18  20:25:37  jpo
 * Release 6.0
 *
 */


#include	"util.h"
#include	"tb_bpt84.h"
#include	"list_bpt.h"
#include	"tb_p1.h"
#include	<isode/cmd_srch.h>


extern CMD_TABLE	bptbl_body_parts84 [/* body part types */];
static void		undefined2mem();




/* ---------------------  Begin  Routines  -------------------------------- */



void enctypes2mem (field, undef, ptr) 
int			field;
char			*undef;
LIST_BPT		**ptr;
{
	LIST_BPT	*new,
			*base = NULLIST_BPT;
	char		*value;
	int		i;


	PP_TRACE (("x40084/enctypes2mem (%d)", field));

	if (field == 0 || field < 0) {
		value = rcmd_srch (BPT_IA5, bptbl_body_parts84);
		*ptr = list_bpt_new (value);
		return;
	}


	/* This is a temp measure because of erroneous encoding from bull */
	if (field > 1023)
		field = 1;


	for (i = EI_TOTAL - 1; i >= 0; i--)
		if (field & (1 << i)) {
			switch (i) {
			case 0:
				undefined2mem (undef, &base);
				continue;
			case 1:
				value = rcmd_srch (BPT_TLX,
						   bptbl_body_parts84);
				break;
			case 2:
				value = rcmd_srch (BPT_IA5,
						   bptbl_body_parts84);
				break;
			case 3:
				value = rcmd_srch (BPT_G3FAX,
						   bptbl_body_parts84);
				break;
			case 4:
				value = rcmd_srch (BPT_TIF0,
						   bptbl_body_parts84);
				break;
			case 5:
				value = rcmd_srch (BPT_TTX,
						   bptbl_body_parts84);
				break;
			case 6:
				value = rcmd_srch (BPT_VIDEOTEX,
						   bptbl_body_parts84);
				break;
			case 7:
				value = rcmd_srch (BPT_VOICE,
						   bptbl_body_parts84);
				break;
			case 8:
				value = rcmd_srch (BPT_SFD,
						   bptbl_body_parts84);
				break;
			case 9:
				value = rcmd_srch (BPT_TIF1,
						   bptbl_body_parts84);
				break;
			default:
				PP_LOG (LLOG_EXCEPTIONS,
					("x40084/enctyp2mem (i=%d) err", i));
				continue;

			}   /* -- end of switch -- */

			new = list_bpt_new (value);
			list_bpt_add (&base, new);

		}   /* -- end of if -- */

	*ptr = base;
	return;
}




static void undefined2mem (undef, base)
char			*undef;
LIST_BPT		**base;
{
	LIST_BPT	*new;
	char		*value;
	char		*argv[100];
	char		buf [BUFSIZ];
	int		n, argc;


	PP_TRACE (("undefined2mem (%s)",
		undef ? undef : "undefined")); 


	/* -- if info_undefined not specified then set to "undefined" -- */
	if (!isstr (undef)) {
		value = rcmd_srch (BPT_UNDEFINED, bptbl_body_parts84);
		new = list_bpt_new (value);
		list_bpt_add (base, new);
		return;
	}


	/* -- otherwise set to info_undefined values -- */
	(void) sprintf (buf, "%s", undef);
	argc = str2arg (buf, 100, argv);
	for (n = 0; n < argc; n++) {
		new = list_bpt_new (argv[n]);
		list_bpt_add (base, new);
	}
}
