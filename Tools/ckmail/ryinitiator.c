/* ryinitiator.c - generic interactive initiator */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ckmail/RCS/ryinitiator.c,v 6.0 1991/12/18 20:29:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ckmail/RCS/ryinitiator.c,v 6.0 1991/12/18 20:29:15 jpo Rel $
 *
 * $Log: ryinitiator.c,v $
 * Revision 6.0  1991/12/18  20:29:15  jpo
 * Release 6.0
 *
 */



#include <stdio.h>
#include <stdarg.h>
#include "util.h"
#include "qmgr.h"
#include "ryinitiator.h"


/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */



static int 	invoke();
/*    DATA */

static char *myname = "ckmail";


extern char *isodeversion;
extern int	verbose;

/*    INITIATOR */

ryinitiator (name, host, argc, argv, myservice, ops, dispatches, quit)
char	*name,
	*host;
int	argc;
char  **argv,
       *myservice;
struct RyOperation ops[];
struct client_dispatch *dispatches;
IFP	quit;
{
    int	    sd;
    register struct client_dispatch   *ds;
    struct SSAPref sfs;
    register struct SSAPref *sf;
    register struct PSAPaddr *pa;
    struct AcSAPconnect accs;
    register struct AcSAPconnect   *acc = &accs;
    struct AcSAPindication  acis;
    register struct AcSAPindication *aci = &acis;
    register struct AcSAPabort *aca = &aci -> aci_abort;
    AEI	    aei;
    OID	    ctx,
	    pci;
    struct PSAPctxlist pcs;
    register struct PSAPctxlist *pc = &pcs;
    struct RoSAPindication rois;
    register struct RoSAPindication *roi = &rois;
    register struct RoSAPpreject *rop = &roi -> roi_preject;
    
    if (name != NULLCP)
	    myname = strdup(name);
    if ((aei = _str2aei (host, myservice, QMGR_CTX_OID, 
			 0, dap_user, dap_passwd)) == NULLAEI)
	adios (NULLCP, "%s-%s: unknown application-entity",
		host, myservice);
    if ((pa = aei2addr (aei)) == NULLPA)
	adios (NULLCP, "address translation failed");

    if ((ctx = oid_cpy (QMGR_AC)) == NULLOID)
	adios (NULLCP, "out of memory");
    if ((pci = oid_cpy (QMGR_PCI)) == NULLOID)
	adios (NULLCP, "out of memory");
    pc -> pc_nctx = 1;
    pc -> pc_ctx[0].pc_id = 1;
    pc -> pc_ctx[0].pc_asn = pci;
    pc -> pc_ctx[0].pc_atn = NULLOID;

    if ((sf = addr2ref (host)) == NULL) {
	sf = &sfs;
	(void) bzero ((char *) sf, sizeof *sf);
    }

    for (ds = dispatches; ds -> ds_name; ds++)
	    if (strcmp (ds -> ds_name, "readmsginfo") == 0)
		    break;
    if (ds -> ds_name == NULL)
	    adios (NULLCP, "unknown operation \"%s\"", "newmessage");

    if (AcAssocRequest (ctx, NULLAEI, aei, NULLPA, pa, pc, NULLOID,
		0, ROS_MYREQUIRE, SERIAL_NONE, 0, sf, NULLPEP, 0, NULLQOS,
		acc, aci)
	    == NOTOK)
	acs_adios (aca, "A-ASSOCIATE.REQUEST");

    if (acc -> acc_result != ACS_ACCEPT) {
	    adios (NULLCP, "Association rejected: [%s]",
		   AcErrString (acc -> acc_result));
    }
    sd = acc -> acc_sd;
    ACCFREE (acc);

    if (RoSetService (sd, RoPService, roi) == NOTOK)
	ros_adios (rop, "set RO/PS fails");
    if (verbose == TRUE) {
	    printf("connected\n");
	    fflush(stdout);
    }
    invoke (sd, ops, ds, argv);

    (*quit) (sd, (struct client_dispatch *) NULL, (char **) NULL, (caddr_t *) NULL);
}

/*  */

static	invoke (sd, ops, ds, args)
int	sd;
struct RyOperation ops[];
register struct client_dispatch *ds;
char  **args;
{
    int	    result;
    caddr_t in;
    struct RoSAPindication  rois;
    register struct RoSAPindication *roi = &rois;
    register struct RoSAPpreject   *rop = &roi -> roi_preject;

    in = NULL;

    if (ds -> ds_argument && (*ds -> ds_argument) (sd, ds, args, &in) != OK) {
	    printf("Failed in attempting %s\n",*args);
    }

    switch (result = RyStub (sd, ops, ds -> ds_operation, RyGenID (sd),
			     NULLIP, in, ds -> ds_result, 
			     ds -> ds_error, ROS_SYNC, roi)) {
	case NOTOK:		/* failure */
	    if (ROS_FATAL (rop -> rop_reason))
		    ros_adios (rop, "STUB");
	    ros_advise (rop, "STUB");
	    break;

	case OK:		/* got a result/error response */
	    break;
	    
	case DONE:		/* got RO-END? */
	    adios (NULLCP, "got RO-END.INDICATION");
	    /* NOTREACHED */

	default:
	    adios (NULLCP, "unknown return from RyStub=%d", result);
	    /* NOTREACHED */
    }
    
#ifdef PEPSY_VERSION
    if (ds -> ds_fr_mod && in)
	fre_obj (in, ds -> ds_fr_mod -> md_dtab[ds -> ds_fr_index],
		 ds -> ds_fr_mod);
#else
    if (ds -> ds_free && in)
	(*ds -> ds_free)(in);
#endif
}

/*  */

void	ros_adios (rop, event)
register struct RoSAPpreject *rop;
char   *event;
{
    ros_advise (rop, event);

    _exit (1);
}


void	ros_advise (rop, event)
register struct RoSAPpreject *rop;
char   *event;
{
    char    buffer[BUFSIZ];

    if (rop -> rop_cc > 0)
	(void) sprintf (buffer, "[%s] %*.*s", RoErrString (rop -> rop_reason),
		rop -> rop_cc, rop -> rop_cc, rop -> rop_data);
    else
	(void) sprintf (buffer, "[%s]", RoErrString (rop -> rop_reason));

    advise (NULLCP, "%s: %s", event, buffer);
}

/*  */

void	acs_adios (aca, event)
register struct AcSAPabort *aca;
char   *event;
{
    acs_advise (aca, event);

    _exit (1);
}


void	acs_advise (aca, event)
register struct AcSAPabort *aca;
char   *event;
{
    char    buffer[BUFSIZ];

    if (aca -> aca_cc > 0)
	(void) sprintf (buffer, "[%s] %*.*s",
		AcErrString (aca -> aca_reason),
		aca -> aca_cc, aca -> aca_cc, aca -> aca_data);
    else
	(void) sprintf (buffer, "[%s]", AcErrString (aca -> aca_reason));

	advise (NULLCP, "%s: %s (source %d)", event, buffer,
		aca -> aca_source);
}

static void	_advise ();

void adios (char *what, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _advise (what, fmt, ap);
    va_end (ap);
    _exit (1);
}

void advise (char *what, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _advise (what, fmt, ap);
    va_end (ap);
}

static void _advise (char *what, char *fmt, va_list ap)
{
    char    buffer[BUFSIZ];

    _asprintf (buffer, what, fmt, ap);

    (void) fflush (stdout);

    fprintf (stderr, "%s: ", myname);
    (void) fputs (buffer, stderr);
    (void) fputc ('\n', stderr);

    (void) fflush (stderr);
}

void ryr_advise (char *what, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _advise (what, fmt, ap);
    va_end (ap);
}
