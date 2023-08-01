/* ap_s2p.c: convert string into major address parts */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_s2p.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_s2p.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_s2p.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "ap_lex.h"
#include "ap.h"


static char     *s2p_txt;       /* -- hdr_in() passes to alst() -- */




/* ---------------------  Static  Routines  ------------------------------- */


/* -- adrs extracted from component text -- */

static int s2p_in()
{
	/* -- nothing to give it -- */

	if (s2p_txt == NULLCP)
		return (EOF);

	switch (*s2p_txt) {
	case '\0':
	case '\n':
		/* -- end of string, drop on through -- */
		s2p_txt = NULLCP;
		return (0);
	}

	return (*(s2p_txt++));
}




/* ---------------------  Begin  Routines  -------------------------------- */


/* -- One of the main uses of this routine is to replace adrparse.c -- */


char *ap_s2p (str_ptr, tree, group, name, local, domain, route)
char            *str_ptr;
AP_ptr          *tree,
		*group,
		*name,
		*local,
		*domain,
		*route;
{
	s2p_txt = str_ptr;

	*tree = ap_pinit (s2p_in);

	switch (ap_1adr()) {
	case DONE:
		return ((char *) DONE);
	case OK:
		(void) ap_t2p (*tree, group, name, local, domain, route);
		/* -- so they can parse the next chunk -- */
		return (s2p_txt);
	}

	return ((char *) NOTOK);
}
