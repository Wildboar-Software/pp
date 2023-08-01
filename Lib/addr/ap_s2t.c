/* ap_s2t.c: convert a string into an address tree */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_s2t.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_s2t.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_s2t.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "ap.h"


static char *ap_str_ptr;




/* ---------------------  Static  Routines  ------------------------------- */



static int get_a_char()        /* -- get next character from string -- */
{

	if (*ap_str_ptr == '\0')
		/* -- end of the string -- */
		return (-1);

	return (*ap_str_ptr++);
}




/* ---------------------  Begin  Routines  -------------------------------- */



/* -- parse a string into an address tree -- */

AP_ptr ap_s2t (thestr)
char            *thestr;
{
	int     got_one;
	AP_ptr  thetree;


	PP_DBG (("Lib/addr/ap_s2t (%s)", thestr));

	ap_str_ptr = thestr;

	if ((thetree = ap_pinit (get_a_char)) == BADAP)
		return( NULLAP);

	ap_clear();

	got_one = 0;

	for (;;)
		switch (ap_1adr()) {
		case NOTOK:
			(void) ap_sqdelete (thetree, NULLAP);
			ap_free (thetree);
			return (NULLAP);

		case OK:
			/* -- more to process -- */
			got_one++;
			continue;

		case DONE:
			if (got_one != 1){
				(void) ap_sqdelete(thetree, NULLAP);
				ap_free(thetree);
				thetree = BADAP;
			}
			return (thetree);
		}
}
