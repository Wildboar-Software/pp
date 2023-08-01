/* t-rdmsg.c - tests for erroneous messages */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-rdmsg.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-rdmsg.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-rdmsg.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "q.h"


main (argc, argv)
int     argc;
char    *argv[];
{
	struct prm_vars prm;
	Q_struct        que;
	ADDR            *sender = NULL;
	ADDR            *recips = NULL;
	int             retval, rcount;
	char            *name = argv[1];


	if (argc != 2) {
		printf ("\n\nUsage:  t-rdmsg msg.XXXX\n\n");
		return;
	}

	sys_init (argv[0]);

	prm_init (&prm);
	q_init (&que);

	retval = rd_msg (name,&prm,&que,&sender,&recips,&rcount);
	printf ("%s: %s %s\n", argv[0], argv[1], rp_valstr (retval));
	rd_end();
	return;
}
