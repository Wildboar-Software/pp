/* t-rfc8222uu.c: test the rfc8222uu routine */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/uucp/RCS/t-rfc8222uu.c,v 6.0 1991/12/18 20:13:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/uucp/RCS/t-rfc8222uu.c,v 6.0 1991/12/18 20:13:06 jpo Rel $
 *
 * $Log: t-rfc8222uu.c,v $
 * Revision 6.0  1991/12/18  20:13:06  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"chan.h"

CHAN	*mychan;

main(argc, argv)
int	argc;
char	**argv;
{
	char	*into;

	if (argc != 3) {
		printf("usage: %s host adr",argv[0]);
		exit(1);
	}
	sys_init(argv[0]);
	if ((mychan = ch_nm2struct("uucp-out")) == NULLCHAN) {
		printf("unable to find uucp-out channel\n");
		exit(1);
	}
	if (rfc8222uu(argv[1], argv[2], &into) != OK) 
		printf ("failed to convert '%s' at %s\n",argv[2],argv[1]);
	else {
		printf("%s at %s\n\t-> %s\n",argv[2],argv[1],into);
		free(into);
	}
}
