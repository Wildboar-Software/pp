/* asn - A body part converter */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/asn.c,v 6.0 1991/12/18 20:15:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/asn.c,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: asn.c,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"asn.h"

char		*myname;


static ASNCMD	*Asnfilt;
static ASNCMD	*asnfilt_new();




/* -------------------------  Main Routine  --------------------------------- */



/* ARGSUSED */
main(argc, argv)
int	argc;
char	**argv;
{
	int	retval;

	/* -- Program Initialisation -- */
	myname = *argv++;
	sys_init(myname);

	/* -- Malloc -- */
	Asnfilt = asnfilt_new();

	/* -- Read the command line and set struct fields -- */
	opt_cmdln (argv, Asnfilt);

	/* -- Set encoder/decoder functs from cmd line asn vals -- */
	opt_functs (Asnfilt);

	/* -- Process info received from stdin, and output to stdout -- */
	asn_process (Asnfilt);

	exit(0);
}




/* ----------------------  Static  Routines  -------------------------------- */




static ASNCMD *asnfilt_new ()   /* -- create new asn struct -- */
{
	register ASNCMD	*ap;

	PP_TRACE (("asnfilt_new ()"));

	ap = (ASNCMD *) smalloc (sizeof(ASNCMD));
	bzero ((char *) ap, sizeof (*ap));
	return(ap);
}
