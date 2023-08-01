/* flatten_alone: standalone flatten a P2 directory structure into a message again */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/p2flatten/RCS/flatten_only.c,v 6.0 1991/12/18 20:20:12 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/p2flatten/RCS/flatten_only.c,v 6.0 1991/12/18 20:20:12 jpo Rel $
 *
 * $Log: flatten_only.c,v $
 * Revision 6.0  1991/12/18  20:20:12  jpo
 * Release 6.0
 *
 */



#include "retcode.h"
#include "util.h"

static char *myname = "p2flatten_alone";

/*
 * parameters -
 * argv[0] = program name
 * argv[1] = source directory
 * argv[2] = destination directory
 */

static int x40084;

main(argc, argv)
int	argc;
char    **argv;
{
	int result = OK;
	if (isatty (fileno(stderr)))
		ll_dbinit(pp_log_norm, argv[0]);
	if (argc < 3)
		err_abrt(RP_MECH, "Usage: %s src-dir dest-dir", argv[0]);
	x40084 = FALSE;
	if (argc == 4)
		if (lexequ(argv[3], "-84") == 0)
			x40084 = TRUE;
	result = flatten(argv[1], argv[2], x40084);
	printf("Flatten result was %s\n", (result == OK) ? "ok" : "notok");
}
