/* ap_file.c: ap file handling routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_file.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_file.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_file.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "ap.h"

/*
parse state saving uses a linked list of state information, recorded in
ap_prevstruct structures. the list is manipulated as a simple stack.
*/

static struct ap_prevstruct     *ap_file; /* -- parse state top of stack -- */
extern int                      (*ap_gfunc)(); /* -- ptr to char get fn -- */
extern int                      ap_peek; /* -- basic parse state info -- */
extern int                      ap_perlev;
extern int                      ap_grplev;
extern AP_ptr                   ap_pstrt,
				ap_pcur;

int ap_ppush (gfunc)   /* -- save parse context, ap_iinit -- */
int     (*gfunc)();
{
	register struct ap_prevstruct  *tfil;

	if ((tfil = (struct ap_prevstruct *)
			malloc (sizeof (struct ap_prevstruct))) ==
				(struct ap_prevstruct *) NOTOK)
					return (NOTOK);

	tfil -> ap_opeek    = ap_peek;   /* -- save regular parse info -- */
	tfil -> ap_ogroup   = ap_grplev;
	tfil -> ap_operson  = ap_perlev;
	tfil -> ap_prvgfunc = ap_gfunc;
	tfil -> ap_next     = ap_file;   /* -- save previous stack entry -- */
	ap_file             = tfil;      /* -- save current stack entry -- */
	ap_iinit (gfunc);                /* -- create new parse state -- */
	return (OK);
}



void ap_ppop()  /* -- restore previous parse state -- */
{
	register struct ap_prevstruct  *tfil;

	tfil            = ap_file;
	ap_peek         = tfil -> ap_opeek;
	ap_grplev       = tfil -> ap_ogroup;
	ap_perlev       = tfil -> ap_operson;
	ap_gfunc        = tfil -> ap_prvgfunc;
	ap_file         = tfil -> ap_next;
	free ((char *) tfil);
}



/*
the next three routines handle most of the overhead for acquiring
the address list from a file.
*/


int ap_flget()  /* -- get character from included file -- */
{
	register int    c;

	c = getc (ap_file -> ap_curfp);

	if (c == '\n')
		return (',');   /* -- a minor convenience -- */

	return (c);
}



int ap_fpush (file)  /* -- indirect input from file -- */
char    file[];
{
	if (ap_ppush (ap_flget) == NOTOK)
		/* -- save current & set for file input -- */
		return (NOTOK);

	if ((ap_file -> ap_curfp = fopen (file, "r")) == NULLFILE) {
		/* -- couldn't get the file, tho -- */
		ap_ppop();
		return (NOTOK);
	}
	return (OK);
}



void ap_fpop()  /* -- pop the stack, if any input nested -- */
{
	if (ap_file -> ap_curfp != NULLFILE)
		(void) fclose (ap_file -> ap_curfp);
	ap_ppop();
}
