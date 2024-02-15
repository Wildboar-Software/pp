/* dr2rfc.c - The PP DR structure to an RFC message */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/dr2rfc/RCS/dr2rfc.c,v 6.0 1991/12/18 20:06:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/dr2rfc/RCS/dr2rfc.c,v 6.0 1991/12/18 20:06:58 jpo Rel $
 *
 * $Log: dr2rfc.c,v $
 * Revision 6.0  1991/12/18  20:06:58  jpo
 * Release 6.0
 *
 */



/****************************************************************************\
*                                                                            *
*  The following checks are done:                                            *
*       -  The message should not as yet be delivered.                       *
*       -  The Recipient number has a DR associated with it.                 *
*       -  All specified recipients have the same DR number.                 *
*                                                                            *
\****************************************************************************/




#include <sys/param.h>
#include "head.h"
#include "prm.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "q.h"
#include "tb_a.h"
#include "ap.h"

extern  char                            *quedfldir;
extern void	rd_end(), chan_init(), err_abrt(), or_myinit();

/* -- static variables -- */
static Q_struct                         Qstruct;
Q_struct                                *PPQuePtr = &Qstruct;
static struct prm_vars                  PRMstruct;
static int                              DRno = 0;
char                                    *this_msg = NULLCP;
int					order_pref;
int					io_running;
int					linked = TRUE;
extern	int				ap_outtype;
static int initproc ();
static int endfunc();
static void dirinit ();
static struct type_Qmgr_DeliveryStatus *process ();
static int adr_checks ();
static int Deliver ();
static void display_ProcMsg ();
char	error[BUFSIZ];
/* -- used by other files -- */
CHAN                                    *mychan;

#define	DEFAULT_MAX_INCLUDE	5000
#define DEFAULT_NUM_INCLUDE_LINES	10
	
long	max_include_msg = DEFAULT_MAX_INCLUDE;
int	num_include_lines = DEFAULT_NUM_INCLUDE_LINES;
int	include_all = FALSE;
int	include_admin = FALSE;

/* ---------------------  Begin  Routines  -------------------------------- */




main (argc, argv)
int     argc;
char    **argv;
{
	char            *myname = argv[0];

	if (*myname == '/') {
		myname = rindex (myname, '/');
		myname++;
	}

	/* -- read pp tailor file -- */
	chan_init (myname);

	/* -- needed for oid_table -- */
	dsap_init ((char ***) NULL, (int *) NULL);

	/* -- check that the Q dirs are ok -- */
	dirinit();

#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0)
	       debug_channel_control (argc, argv, initproc, process, endfunc);
	else
#endif
	       channel_control (argc, argv, initproc, process, endfunc);


	exit (0);
}




/* ---------------------  Static  Routines  ------------------------------- */




static void dirinit()   /* -- Change into pp queue space -- */
{
	PP_TRACE (("dirinit()"));
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, "Unable to change to dir '%s'", quedfldir);
}

static void do_ch_info_flags(info)
char	*info;
{
	char	*info_copy, *margv[20];
	int	margc, ix;

	if (info == NULLCP) return;

	info_copy = strdup(info);
	if ((margc = sstr2arg(info_copy, 20, margv, "= \t,")) > 0) {
		for (ix = 0; ix < margc; ix++) {
			if (lexequ(margv[ix], "order") == 0) {
				if (lexequ(margv[ix+1], "uk") == 0) {
					order_pref = CH_UK_PREF;
					ap_outtype |= AP_PARSE_BIG;
				}
				ix ++;
			} else if (lexequ(margv[ix], "submit") == 0) {
				if (lexequ(margv[ix+1], "linked") == 0)
					linked = TRUE;
				else if (lexequ(margv[ix+1], "copy") == 0)
					linked = FALSE;
				ix ++;
			} else if (lexequ(margv[ix], "admininfo") == 0) {
				if (lexequ(margv[ix+1], "true") == 0)
					include_admin = TRUE;
				else if (lexequ(margv[ix+1], "false") == 0)
					include_admin = FALSE;
				ix++;
			} else if (lexequ(margv[ix], "return") == 0) {
				if (lexequ(margv[ix+1], "all") == 0) 
					include_all = TRUE;
				else {
					/* number letter */
					char	*ich = margv[ix+1];
					
					while (isdigit(*ich)) ich++;
					
					switch (*ich) {
					    case 'l':
					    case 'L':
						if (*ich == margv[ix+1][0])
							num_include_lines = 1;
						else
							num_include_lines = 
								atoi(margv[ix+1]);
						break;
						
					    case 'K':
					    case 'k':
						if (*ich == margv[ix+1][0])
							max_include_msg = (long) 1000;
						else
							max_include_msg =
								(long) atoi(margv[ix+1]) * (long) 1000;
						break;
						
					    default:
						PP_LOG(LLOG_EXCEPTIONS,
						       ("Unable to parse LHS as numberletter '%s=%s'", margv[ix], margv[ix+1]));
					}
				}
				ix++;
			} else 
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown info flag '%s'",
					margv[ix]));
		}
	}
	free(info_copy);
}

static int initproc (arg)
struct type_Qmgr_Channel        *arg;
{
	char                    *myname = qb2str (arg);
	RP_Buf  rps, *rp = &rps;

	PP_TRACE (("initproc (%s)", myname));

	if ((mychan = ch_nm2struct (myname)) == NULLCHAN)
		err_abrt (RP_PARM, "Channel '%s' not known", myname);

	
	/* -- other initialisations -- */
	order_pref = CH_USA_PREF;
	ap_outtype = AP_PARSE_SAME;
	linked = TRUE;
	max_include_msg = DEFAULT_MAX_INCLUDE;
	num_include_lines = DEFAULT_NUM_INCLUDE_LINES;
	include_all = FALSE;
	include_admin = FALSE;

	if (mychan->ch_out_info != NULLCP)
		do_ch_info_flags(mychan->ch_out_info);
	
	pp_setuserid();

	or_myinit();

	if (rp_isbad (io_init(rp)))
		err_abrt (RP_PARM, "io_init err %s", rp -> rp_line);
	else
		io_running = TRUE;

	PP_NOTICE (("Channel %s initialised", myname));
	free (myname);
	q_init (PPQuePtr);
	prm_init (&PRMstruct);
	return OK;
}

/* ARGSUSED */
static int endfunc (arg)
struct type_Qmgr_Channel *arg;
{
	if (io_running == TRUE)
		io_end(OK);
	io_running = FALSE;
}

stop_io()
{
	if (io_running == TRUE)
		io_end(NOTOK);
	io_running = FALSE;
}

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg                *arg;
{

	struct type_Qmgr_UserList       *up;
	ADDR                            *ad_list        = NULLADDR,
					*ad_sender      = NULLADDR,
					*ad_recip       = NULLADDR,
					*alp            = NULLADDR,
					*ap             = NULLADDR;
	int                             naddrs          = 0,
					ad_count,
					retval;

	if (this_msg)
		free (this_msg);

	this_msg = qb2str (arg->qid);
	PP_TRACE (("process (msg = '%s')", this_msg));
	(void) delivery_init (arg->users);
	(void) delivery_setall (int_Qmgr_status_messageFailure);

#ifdef PP_DEBUG
	display_ProcMsg (arg);
#endif


	/* -- Initialisation --*/

	DRno = 0;
	q_init (PPQuePtr);
	prm_init (&PRMstruct);


	/* -- read the addr control file -- */

	retval = rd_msg (this_msg, &PRMstruct, PPQuePtr,
			&ad_sender, &ad_recip, &ad_count);

	if (rp_isbad (retval)) {
		rd_end();
		PP_LOG (LLOG_EXCEPTIONS, ("rd_msg error: %s", this_msg));
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Can't read message");
	}



	/* -- create a new recip list + perform addr check -- */

	for (ap = ad_recip; ap; ap = ap -> ad_next) {
		for (up = arg->users; up; up = up->next) {
			if (up -> RecipientId -> parm != ap -> ad_no)
				continue;

			if (adr_checks (ap) != OK)
				break;

			if (ad_list == NULLADDR)
				ad_list = alp =
					(ADDR *) calloc(1, sizeof *alp);
			else {
				alp -> ad_next =
					(ADDR *) calloc(1, sizeof *alp);
				alp = alp -> ad_next;
			}

			/* -- struct copy -- */
			*alp = *ap;
			alp -> ad_next = NULLADDR;
			naddrs++;
			break;
		}
	}


	if (naddrs == 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipients in the user list"));
		rd_end ();
		return deliverystate;
	}


	/* -- deliverystate set in deliver -- */
	if (Deliver (ad_sender, ad_list) != RP_OK)
		PP_LOG (LLOG_EXCEPTIONS, ("resubmission failed"));
	rd_end();

	/* -- free ad_list -- */
	for (alp = ad_list; alp; alp = ap) {
		ap = alp -> ad_next;
		free ((char *)alp);
	}

	q_free (PPQuePtr);
	prm_free (&PRMstruct);

	return deliverystate;
}




static int adr_checks (ap)
ADDR    *ap;
{
	PP_TRACE (("adr_checks (%s)", ap->ad_value));

	switch (ap->ad_status) {
	    case AD_STAT_DRWRITTEN:
		break;
	    case AD_STAT_DONE:
		(void) delivery_set(ap->ad_no,
				    int_Qmgr_status_success);
		return NOTOK;
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("adr_checks - wrong status"));
		(void) delivery_setstate(ap->ad_no,
				    int_Qmgr_status_messageFailure,
				    "adr_checks - wrong status");
	       return NOTOK;
	}

	if (ap -> ad_resp == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("responsibility bit not set for addr %d",
			 ap -> ad_no));
		(void) delivery_setstate (ap -> ad_no,
				     int_Qmgr_status_messageFailure,
				     "responsibility bit not set for addr");
		return NOTOK;
	}


	if (DRno == 0)
		DRno = ap->ad_no;
	return OK;
}




static int Deliver (orig, recip)
ADDR    *orig;
ADDR    *recip;
{
	int     retval = RP_BAD;
	int     value = int_Qmgr_status_messageFailure;
	ADDR    *ap;
	RP_Buf	rps, *rp = &rps;
	PP_TRACE (("Deliver (%s)", orig->ad_value));
	error[0] = '\0';
	if (orig->ad_outchan &&
	    orig->ad_outchan->li_chan &&
	    orig->ad_outchan->li_chan->ch_name &&
	    lexequ(orig->ad_outchan->li_chan->ch_name, mychan->ch_name) == 0) {
		   
		if (io_running == FALSE)
			if (rp_isbad(io_init(rp))) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("io_init err %s", rp -> rp_line));
				(void) sprintf (error,
						"io_init error [%s]",
						rp -> rp_line);
				goto Deliver_free;
			} else
				io_running = TRUE;
			
		if (rp_isbad (write_queue (orig)))
			goto Deliver_free;

		if (rp_isbad (write_report (orig, DRno, recip)))
			goto Deliver_free;


		value = int_Qmgr_status_success;
		retval = RP_OK;
	} else {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Illegal setting of outbound channel for addr %d",
			orig->ad_no));
		(void) sprintf (error,
				"Illegal setting of outbound channel");
	}

    Deliver_free: ;
	for (ap = recip; ap; ap = ap -> ad_next) {
		if (ap -> ad_resp) {
			(void) delivery_setstate (ap -> ad_no, value, error);
			if (value == int_Qmgr_status_success)
				wr_ad_status (ap, AD_STAT_DONE);
		}

	}

	if (retval == RP_OK) {
		PP_NOTICE (("Wrote DR to  '%s'", orig->ad_value));
		PP_NOTICE ((">>> %s %s done", mychan -> ch_name, this_msg));
	}
	else {
		PP_OPER (NULLCP, ("dr2rfc has failed for %s", this_msg));
		PP_NOTICE ((">>> %s %s has not been done",
			    mychan -> ch_name, this_msg));
		stop_io ();
	}

	return retval;
}




/*  --------------------  Debug Proceedures  ------------------------------ */




#ifdef PP_DEBUG
static void display_ProcMsg (pm)
struct type_Qmgr_ProcMsg        *pm;
{
	char                    *str;
	struct type_Qmgr_UserList *pmu;

	str = qb2str (pm -> qid);

	for (pmu = pm -> users; pmu; pmu = pmu -> next)
		PP_TRACE (("%s:  user(%d)",
			str, pmu -> RecipientId->parm));

	if (str) free (str);

	return;
}
#endif
