/* probe.c: Generates a probe */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/probe/RCS/probe.c,v 6.0 1991/12/18 20:32:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/probe/RCS/probe.c,v 6.0 1991/12/18 20:32:41 jpo Rel $
 *
 * $Log: probe.c,v $
 * Revision 6.0  1991/12/18  20:32:41  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "q.h"
#include <isode/cmd_srch.h>



/* --- externals --- */
extern char				*loc_dom_site,
					*local_822_chan,
					*cont_822;
extern ADDR				*make_adr_new();


/* --- queue variables --- */
static struct prm_vars                  PRMstruct;
static Q_struct                         Qstruct;
static struct prm_vars                  *PrmPtr = &PRMstruct;
Q_struct                                *QuePtr = &Qstruct;


/* --- other statics  --- */
static char				*myname = NULLCP;

#define	OPT_TO				1
#define OPT_SIZE			2
#define	OPT_EIT				3
#define	OPT_UAID			4
#define	OPT_IMPCONV			5
#define OPT_ALTRECIP			6
#define OPT_DEBUG			7

static int				param = OPT_TO;

CMD_TABLE  probe_options[] = { /* probe commandline options */
	"-t",				OPT_TO,
	"-s",				OPT_SIZE,
	"-e",				OPT_EIT,
	"-u",				OPT_UAID,
	"-i",				OPT_IMPCONV,
	"-a",				OPT_ALTRECIP,
	0,				-1
	};



static void		probe_set_rest(), probe_checks(), usage();	





/* ---------------------  Begin  Routines  -------------------------------- */




main (argc, argv)
int     argc;
char    **argv;
{
	char	*filename;
	ADDR	*ap;
	int	retval;


	bzero ((char *)PrmPtr, sizeof(*PrmPtr));
	bzero ((char *)QuePtr, sizeof(*QuePtr));
	(void) set_from();

	myname = argv[0];

	if (argc > 16)
		usage();

	if (argc == 1) {
		(void) prompt_probe_stdin();	
		probe_process();
	}
		

	while (--argc > 0) {
		argv ++;
		if (**argv == '-')
			switch (param = cmd_srch (*argv, probe_options)) {

			case OPT_TO:
			case OPT_SIZE:
			case OPT_EIT:
			case OPT_UAID:
				break;

			case OPT_IMPCONV:
				QuePtr -> implicit_conversion_prohibited = YES;
				break;
			case OPT_ALTRECIP:
				QuePtr->alternate_recip_allowed = YES;
				break;
			default:
                        	printf ("\n***Error: Unknown option '%s'\n",
					*argv);
                        	usage();
			}   /* -- end of switch -- */

		else {
			switch (param) {
			case OPT_TO:
				ap = make_adr_new (*argv, AD_RECIPIENT);
                		adr_add (&QuePtr -> Raddress, ap);
				break;
			case OPT_SIZE:
				QuePtr -> msgsize = atoi(*argv);
				break;
			case OPT_EIT:
				retval = set_encoded (*argv);
				if (retval == NOTOK) {
                        		printf ("\n***Error: eits! '%s'\n",
					*argv);
                        		usage();
				}
				break;
			case OPT_UAID:
				QuePtr -> ua_id = strdup (*argv);
				break;
			default:
                        	printf ("\n***Error: Unknown option '%s'\n",
					*argv);
				usage();

			}  /* -- end of switch -- */

		}   /* -- end of else -- */

	}  /* -- end of while -- */

	
	probe_process();
}






/* ---------------------  Static  Routines  ------------------------------- */






static void usage()
{
	printf ("\n\n");
	printf ("Usage:   %s\n", myname); 
	printf ("	  	[-t Recipient]\n");
	printf ("		[-s MessageSize]\n");
	printf ("	  	[-e EncodedInfoTypes]\n");
	printf ("		[-u UAid]\n");
	printf ("	  	[-i ImplicitConversion]\n");
	printf ("	  	[-a AlternateRecipientAllowed]\n");
	printf ("\n\n");
	exit (1);
}





static int probe_process()
{
	int	retval; 

	(void) probe_checks();

	sys_init (myname);
	if (or_init() == NOTOK)	 exit (1);
	or_myinit();
	(void) probe_set_rest();

	printf ("Processing ...\n");
	retval = probe_submit();

	if (rp_isbad (retval)) {
		printf ("*** Error: Unable to submit ***\n\n");
		exit (1);
	}
		
	exit (0);
}




static void probe_set_rest()
{
	prm_init (PrmPtr);
        PrmPtr->prm_opts = PRM_ACCEPTALL;

	QuePtr -> msgtype	= MT_PMPDU;
	QuePtr -> cont_type	= strdup(cont_822);
	QuePtr -> priority	= PRIO_URGENT;
	QuePtr -> inbound 	= list_rchan_new (loc_dom_site, local_822_chan);

	if (QuePtr -> msgsize == 0)
	QuePtr -> msgsize 	= 500;

	if (QuePtr -> encodedinfo.eit_types == 0)
		if (set_encoded (NULLCP) == NOTOK) {
			printf ("\n***Error: Unable to set eit\n");
			exit (1);
		}
	(void) MPDUid_new (&QuePtr -> msgid);
}




static void probe_checks()
{

	ADDR	*ap;
	int	err = FALSE;

        if (isstr (QuePtr -> ua_id) && (int)strlen (QuePtr -> ua_id) > 16) {
                printf ("*** Error:  Invalid UA size (max 16) '%s' ***\n",
                        QuePtr -> ua_id);
               	exit (1);
	}

        if (QuePtr -> msgsize < 0) {
                printf ("*** Error:  Invalid Message Size '%d' ***\n",
                        QuePtr -> msgsize);
               	exit (1);
	}
}




static int probe_submit()
{
	RP_Buf          rp;
	ADDR            *ap;
	int		submit_status = NOTOK;

	if (rp_isbad (io_init(rp))) {
		printf ("*** Error: probe_submit:  io_init\n");
		exit (1);
	}

	if (rp_isbad (io_wprm (PrmPtr, &rp))) {
		printf ("*** Error: probe_submit:  io_wprm\n");
		goto  probe_submit_end;
	}

	if (rp_isbad (io_wrq (QuePtr, &rp))) {
		printf ("*** Error: probe_submit:  io_wrq\n");
		goto  probe_submit_end;
	}

	if (rp_isbad (io_wadr (QuePtr->Oaddress, AD_ORIGINATOR, &rp))) {
		printf ("*** Error: probe_submit:  io_wadr ('%s')\n",
			QuePtr->Oaddress->ad_value);
		goto  probe_submit_end;
	}


	for (ap = QuePtr->Raddress; ap; ap = ap->ad_next) {

		/* -- Red Book X.411 Clause 4.3.2.2 -- */ 
		ap -> ad_mtarreq = AD_MTA_CONFIRM;
		ap -> ad_usrreq = AD_USR_CONFIRM;

		if (rp_isbad (io_wadr (ap, AD_RECIPIENT, &rp))) {
			printf ("*** Error: probe_submit:  io_wadr ('%s')\n",
				ap->ad_value);
			goto  probe_submit_end;
		}
	}


	if (rp_isbad (io_adend (&rp))) {
		printf ("*** Error: probe_submit:  io_adend\n");
		goto  probe_submit_end;
	}
	else printf ("%s\n", rp.rp_line);

	submit_status = OK;

probe_submit_end: ;
	io_end (submit_status);
	return submit_status;
}
