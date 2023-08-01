/* lmnpq_dish.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/lmnpq_dish.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/lmnpq_dish.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: lmnpq_dish.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include <isode/quipu/dua.h>
#include <isode/quipu/read.h>

extern Entry    current_entry;
extern char 	frompipe;
extern DN       dn, current_dn;

#define	OPT	(!frompipe || rps -> ps_byteno == 0 ? opt : rps)
#define	RPS	(!frompipe || opt -> ps_byteno == 0 ? rps : opt)
extern	PS	opt, rps;

call_dlist (argc, argv)
int             argc;
char          **argv;
{
int             x;
char		dn_check = FALSE;
char		or_check = FALSE;
char		or_update = FALSE;
	
	for (x=1; x<argc; x++) {
		if (test_arg (argv[x], "-dncheck",1))
			dn_check = TRUE;
		else if (test_arg (argv[x], "-orcheck",3))
			or_check = TRUE;
		else if (test_arg (argv[x], "-check",1)) {
			dn_check = TRUE;
			or_check = TRUE;
		} else if (test_arg (argv[x], "-orupdate",3)) {
			or_update = TRUE;
			dn_check = TRUE;
		} else if (test_arg (argv[x], "-update",1)) {
			or_update = TRUE;
			or_check = TRUE;
			dn_check = TRUE;
		} else if (move (argv[x]) != OK)
			continue;
		shuffle_up (argc--,argv,x--);
	}

	if (argc != 1) {
		ps_printf (OPT,"Unknown option %s\n",argv[1]);
		Usage (argv[0]);
		return -1;
	}

	if ((argc = read_cache (argc, argv)) < 0)
		return OK;

	if (dn_check && (rebind () != OK))
		return(-2);

	if (dn_check || or_check) {
                check_dl_members (RPS,dn,current_entry->e_attributes,dn_check,or_check,or_update,FALSE);
		return OK;
	}

	dl_print (RPS,current_dn);
	return OK;
}
