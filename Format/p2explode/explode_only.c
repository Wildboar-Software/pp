/* explode_alone:  standalone p2 explode

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/p2explode/RCS/explode_only.c,v 6.0 1991/12/18 20:20:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/p2explode/RCS/explode_only.c,v 6.0 1991/12/18 20:20:02 jpo Rel $
 *
 * $Log: explode_only.c,v $
 * Revision 6.0  1991/12/18  20:20:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"

/*
 * parameters -
 * argv[0] = program name
 * argv[1] = source directory
 * argv[2] = destination directory
 */
#include	"util.h"
extern int	unflatten();
static int	x40084;

main(argc, argv)
int	argc;
char    **argv;
{
	int result = OK;
	char	*error;
	if (isatty (fileno(stderr)))
		ll_dbinit(pp_log_norm, argv[0]);

	if(argc < 3)
		err_abrt(RP_MECH, "Usage: %s src-dir dest-dir", argv[0]);
	x40084 = FALSE;
	if (argc == 4)
		if (lexequ(argv[3], "-84") == 0) x40084 = TRUE;
	result = unflatten(argv[1],argv[2],x40084,NULL, &error);
	printf("Unflatten result was %s\n", (result == OK) ? "ok" : "notok");
}
