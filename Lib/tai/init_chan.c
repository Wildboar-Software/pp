/* init_chan.c: initialisation for channel programs */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/tai/RCS/init_chan.c,v 6.0 1991/12/18 20:24:59 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/tai/RCS/init_chan.c,v 6.0 1991/12/18 20:24:59 jpo Rel $
 *
 * $Log: init_chan.c,v $
 * Revision 6.0  1991/12/18  20:24:59  jpo
 * Release 6.0
 *
 */


void chan_init (pname)
char    *pname;
{
	(void) sys_init (pname);
}
