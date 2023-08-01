/* t-rd_rfchdr.c: small wrap around to test parseing of rfc hdrs in submit */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/t-rd_rfchdr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/t-rd_rfchdr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: t-rd_rfchdr.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"q.h"

Q_struct	Qstruct;
CHAN		*ch_inbound;

main (argc, argv)
int	argc;
char	**argv;
{
	if (argc < 3) {
		printf ("usage : %s channel filename", argv[0]);
		exit(0);
	}
	sys_init(argv[0]);
	or_myinit();
	if ((ch_inbound = ch_nm2struct(argv[1])) == NULL) {
		printf ("Unknown channel '%s'", argv[1]);
		exit(0);
	}
	q_init(&Qstruct);
	rd_rfchdr(argv[2]);
}
