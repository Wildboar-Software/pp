/* qmgr_start.c: routines to read in Q from addr dir and pass on to Qmgr */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/qmgr-start.c,v 6.0 1991/12/18 20:11:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/qmgr-start.c,v 6.0 1991/12/18 20:11:26 jpo Rel $
 *
 * $Log: qmgr-start.c,v $
 * Revision 6.0  1991/12/18  20:11:26  jpo
 * Release 6.0
 *
 */


#include "util.h"
#include "retcode.h"
#include "Qmgr-ops.h"           /* operation definitions */
#include "Qmgr-types.h"         /* type definitions */
#include "ryinitiator.h"        /* for generic interactive initiators */

/* Sumbit types */
#include "q.h"
#include "prm.h"

/*  */
/* Outside routines */
extern struct type_Qmgr_MsgStruct 	*qstruct2qmgr();
extern void	qinit(), sys_init(), rd_end();


/*  */
/* DATA */
static char *myservice = 	"pp qmgr";

/* OPERATIONS */
static int 	do_newmessage(),
		do_help(),
		do_quit();

/* RESULTS */
int 	newmessage_result();
	
/* ERRORS */
int 	general_error ();

#define newmessage_error 	general_error

static struct client_dispatch dispatches[] = {
{
	"newmessage", operation_Qmgr_newmessage,
#ifdef	PEPSY_VERSION
	do_newmessage, &_ZQmgr_mod, _ZPseudo_newmessageQmgr,
#else 
	do_newmessage, free_Qmgr_newmessage_argument,
#endif
	newmessage_result, newmessage_error,
	"Re-add message to the queue"
},
{
    "help",     0,      
    do_help,
#ifdef	PEPSY_VERSION
    NULL, 0, 
#else
    NULLIFP, 
#endif
    NULLIFP, NULLIFP,
    "print this information",
},
{
    "quit",     0,      do_quit,
#ifdef	PEPSY_VERSION
    NULL, 0,
#else
    NULLIFP,
#endif
    NULLIFP, NULLIFP,
    "terminate the association and exit",
},
{
    NULL
}
};


extern char *pptailor;

/*  */
/* MAIN */

/* ARGSUSED */

main (argc, argv, envp)
int 	argc;
char 	**argv,
	**envp;
{
	int opt;
	extern char *optarg;
	extern int optind;
	char	*myname,
		*hostname;

	if (myname = rindex (argv[0], '/'))
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = argv[0];

	hostname = PLocalHostName ();

	while ((opt = getopt (argc, argv, "h:t:")) != EOF) {
		switch (opt) {
		    case 'h':
			hostname = optarg;
			break;
		    case 't':
			pptailor = optarg;
			break;
		    default:
			fprintf(stderr,
				"usage: %s [-h hostname] [-t tailor] msg ...\n",
				myname);
			exit(1);
		}
	}
	argv += optind;
	argc -= optind;
	if (argc < 1) {
		fprintf(stderr,
			"usage: %s [-h hostname] [-t tailor] msg ...\n",
			myname);
		exit(1);
	}

	sys_init(myname);

	(void) ryinitiator (myname, hostname,
			    argc, argv, myservice,
			    table_Qmgr_Operations, dispatches, do_quit);
	
	exit(0);	/* NOT REACHED */
}

/*  */
/* OPERATIONS */

/* ARGSUSED */
static int do_newmessage (sd, ds, args, arg)
int 				sd;
struct client_dispatch 		*ds;
char 				**args;
struct type_Qmgr_MsgStruct 	**arg;
{
	char 		*file = NULL;
	struct prm_vars	prm;
	Q_struct	que;
	ADDR		*sender = NULL;
	ADDR		*recips = NULL;
	int 		rcount, result;
	
	result = OK;

	qinit (&que);
	bzero ((char *)&prm, sizeof(prm));

	if ((file = *args++) == NULLCP)
		result = NOTOK;

	if ((result == OK) 
	    && (rp_isbad(rd_msg(file,&prm,&que,&sender,&recips,&rcount)))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/qmgr-load/xqstart rd_msg err: '%s'",file));
		result = NOTOK;
	}

	/* unlock message */
	rd_end();

	if ((result == OK) 
	    && ((*arg = qstruct2qmgr(file,&prm,&que,sender,recips,rcount)) == NULL)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/qmgr-load/xqstart qstruct2qmgr err: '%s'",file));
		result = NOTOK;
	}
	
	/* free all storage */
	q_init(&que);

	return result;
}

/* ARGSUSED */
static int do_help (sd, ds, args)
int 				sd;
register struct client_dispatch *ds;
char 				**args;
{
	printf("\nCommands are:\n");
	for (ds = dispatches; ds->ds_name; ds++)
		printf("%s\t%s\n", ds->ds_name, ds->ds_help);
	return NOTOK;
}

/* ARGSUSED */
static int  do_quit (sd, ds, args)
int     sd;
struct client_dispatch *ds;
char  **args;
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

	if (AcRelRequest (sd, ACF_NORMAL, NULLPEP, 0, 
			  NOTOK, acr, aci) == NOTOK)
		acs_adios (aca, "A-RELEASE.REQUEST");

	if (!acr -> acr_affirmative) {
		(void) AcUAbortRequest (sd, NULLPEP, 0, aci);
		adios (NULLCP, "Release rejected by peer: %d", acr -> acr_reason);
	}

	ACRFREE (acr);

	exit (0);

}

/*  */
/* RESULTS */

/* ARGSUSED */
newmessage_result (sd, id, dummy, result, roi)
int					sd,
	                                id,
	                                dummy;
struct type_Qmgr_Pseudo__newmessage 	*result;
struct RoSAPindication			*roi;
{
	return OK;
}

/*  */
/* ERRORS */

/* ARGSUSED */
general_error (sd, id, error, parameter, roi)
int     sd,
	id,
	error;
caddr_t parameter;
struct RoSAPindication *roi;
{
	register struct RyError *rye;

	if (error == RY_REJECT) {
		advise (NULLCP, "%s", RoErrString ((int) parameter));
		return OK;
	}

	if (rye = finderrbyerr (table_Qmgr_Errors, error))
		advise (NULLCP, "%s", rye -> rye_name);
	else
		advise (NULLCP, "Error %d", error);

	return OK;
}
