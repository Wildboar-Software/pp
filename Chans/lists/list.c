/* list.c: list processor channel (both directory and table) */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/list.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/list.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: list.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "head.h"
#include "qmgr.h"
#include "chan.h"
#include "q.h"
#include "dr.h"
#include "or.h"
#include "prm.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef SYS5
#include <fcntl.h>
#endif


#ifdef	DIRLIST

#include "dlist.h"
#include <isode/quipu/attrvalue.h>

#else

#include "dl.h"

#endif

#include "retcode.h"
#include "sys.file.h"
#include <stdarg.h>

extern void 	rd_end(), sys_init(), err_abrt();
extern struct type_Qmgr_DeliveryStatus *delivery_resetDRs();

extern ADDR 	*adr_new();
extern char 	*ad_getlocal();
extern char	*quedfldir;
extern char	*loc_dom_site;
extern OR_ptr 	or_std2or(),
		or_default(),
		or_std2or();

extern LLog	*log_dsap;
#ifndef	DIRLIST
extern ADDR	*tb_getModerator();
#endif

static int 	expandList();
static char 	*get_adrstr();
static int	get_adrtype();
static void 	dirinit();
static void 	set_success();
static int 	initialise(), endfunc ();
static ADDR 	*getnthrecip();
static int 	submit_error();
static int 	processMsg();
static int	expansionLoop;
static struct type_Qmgr_DeliveryStatus 	*process();
static Q_struct				qs;

static int	processDR();

#ifdef	DIRLIST

static ADDR	*construct_sender();
Attr_Sequence	this_list = NULLATTR;
extern int		dir_getdl();
extern AttributeType 	at_Owner;
extern AttributeType 	at_Policy;
extern AttributeType 	at_Member;
extern ADDR    		*ORName2ADDR();

#endif

CHAN 		*mychan;
char		*this_msg = NULL, *this_chan = NULL;
int		start_submit;
int		first_successDR, first_failureDR;
int		adrno;
int		expandSublists = FALSE;
int		linked = TRUE;

/* -----------------------  Begin  Routines  -------------------------------  */


main (argc, argv)
int	argc;
char	**argv;
{
#ifdef  DIRLIST
int     n = 1;
#endif

#ifdef	PP_DEBUG
	char	pp_debug = FALSE;
#endif

	sys_init (argv[0]);
	or_myinit();
	dirinit();

#ifdef	DIRLIST
	quipu_syntaxes ();
	pp_syntaxes();
#endif

#ifdef PP_DEBUG
	if (argc>1 && (strcmp (argv[1], "debug") == 0))
		pp_debug = TRUE;
#endif

#ifdef	DIRLIST
	dsap_init (&n, &argv);
	log_dsap -> ll_file = strdup ("dsap.log");

	(void) pp_quipu_run ();
#endif

#ifdef	PP_DEBUG
	if (pp_debug)
		debug_channel_control (argc,argv,initialise,process,endfunc);
	else
#endif
		channel_control (argc,argv,initialise,process,endfunc);
}



/* -----------------------  Static Routines  -------------------------------  */



/* --- routine to move to correct place in file system --- */
static void dirinit()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, 
			"Unable to change directory to '%s'", quedfldir);
}


/* ARGSUSED */
static int endfunc (arg)
struct type_Qmgr_Channel *arg;
{
	if (start_submit == FALSE) 
		io_end (OK);
	start_submit = TRUE;

#ifdef	DIRLIST
	dl_unbind();
#endif
}



/* ---  channel initialise routine --- */

DN	bindas = NULLDN;
char	*bindpasswd = NULLCP;

static void do_ch_info_flags(info)
char	*info;
{
	char	*info_copy, *margv[20];
	int	margc, ix;

	if (info == NULLCP) return;

	info_copy = strdup(info);
	if ((margc = sstr2arg(info_copy, 20, margv, ",")) > 0) {
		for (ix = 0; ix < margc; ix++) {
			if (lexequ(margv[ix], "dosublists") == 0)
				expandSublists = TRUE;
			else if (lexequ(margv[ix], "notlinked") == 0)
				linked = FALSE;
			else if (lexequ(margv[ix], "linked") == 0)
				linked = TRUE;
#ifdef DIRLIST
			else if (lexnequ(margv[ix], "DN", strlen("DN")) == 0) {
				char	*dn = index(margv[ix], '=');
				
				if (dn == NULLCP)
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Incorrectly formatted info field '%s' expecting key=value",
						margv[ix]));
				else {
					dn++;
					if ((bindas = str2dn(dn)) == NULLDN)
						PP_LOG(LLOG_EXCEPTIONS,
						       ("Invalid DN '%s'",
							dn));
				}
			}
			else if (lexnequ(margv[ix], "passwd", strlen("passwd")) == 0) {
				char	*pd = index(margv[ix], '=');
				
				if (pd == NULLCP)
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Incorrectly formatted info field '%s' expecting key=value",
						margv[ix]));
				else {
					pd++;
					bindpasswd = strdup(pd);
				}
			}
#endif
			else 
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown ch_info flag '%s'",
					margv[ix]));
		}
	}
	free(info_copy);
}

static int initialise (arg)
struct type_Qmgr_Channel	*arg;
{
	char	*name;

	name = qb2str (arg);

	if ((mychan = ch_nm2struct (name)) == NULLCHAN) {
		PP_OPER (NULLCP, ("Channel '%s' not known", name));
		if (name != NULLCP) 
			free (name);
		return NOTOK;
	}

	start_submit = TRUE;
	if (mychan -> ch_out_info != NULLCP)
		do_ch_info_flags(mychan->ch_out_info);

	/* --- check if a list channel --- */
	if (name != NULLCP)
		free (name);

#ifdef	DIRLIST
	if (dl_bind (bindas, bindpasswd) != OK)
		return NOTOK;
#endif

	return OK;
}



/* ---  routine to check if allowed to list process this message --- */

static int security_check (msg)
struct type_Qmgr_ProcMsg	*msg;
{
	char 	*msg_file = NULLCP, 
		*msg_chan = NULLCP;
	int 	result;

	result   = TRUE;
	msg_file = qb2str (msg->qid);
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp (msg_chan,mychan->ch_name) !=0)) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("channel err: '%s'", msg_chan));
		result = FALSE;
	}

	if (msg_file != NULLCP)		free (msg_file);
	if (msg_chan != NULLCP) 	free (msg_chan);

	return result;
}




/* ---  routine called to do list processing --- */

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars			prm;
	Q_struct			que;
	ADDR				*sender = NULL;
	ADDR				*recips = NULL;
	int 				rcount,
					retval;
	struct type_Qmgr_UserList 	*ix;
	ADDR				*adr;
	RP_Buf				reply;


	bzero ((char *)&prm, sizeof (prm));
	bzero ((char *)&que, sizeof (que));
	
	delivery_init (arg->users);
	delivery_setall (int_Qmgr_status_messageFailure);
	first_failureDR = first_successDR = TRUE;

	if (security_check (arg) != TRUE)
		return deliverystate;

	if (this_msg != NULLCP)	  free (this_msg);
	if (this_chan != NULLCP)  free (this_chan);

	this_msg = qb2str (arg->qid);
	this_chan = qb2str (arg->channel);

	PP_LOG (LLOG_NOTICE,
	       ("processing msg '%s' through '%s'",this_msg, this_chan));

	if (rp_isbad (rd_msg (this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("Chans/list rd_msg err: '%s'",this_msg));
		rd_end();
		return delivery_setallstate(int_Qmgr_status_messageFailure,
					    "Can't read message");
	}

	for (ix = arg->users; ix; ix = ix->next) {
		if ((adr = getnthrecip (&que, ix->RecipientId->parm)) == NULL) {

			PP_LOG (LLOG_EXCEPTIONS,
				("failed to find recipient %d of msg '%s'",
				 ix->RecipientId->parm, this_msg));

			delivery_setstate (ix->RecipientId->parm,
					   int_Qmgr_status_messageFailure,
					   "Unable to find specified recipient");
			continue;
		}
		
		if (start_submit == TRUE && rp_isbad (io_init (&reply))) {
			submit_error (adr,"io_init",&reply);
			rd_end();
			return delivery_setallstate (int_Qmgr_status_messageFailure,
						     "Unable to start submit");
		}
		else
			start_submit = FALSE;
			
		switch (dchan_acheck (adr, sender, mychan, 1, (char **)NULL)) {
		    default:
		    case NOTOK:
			break;
		    case OK:
			switch (que.msgtype) {
			    case MT_UMPDU:
			    default:
				processMsg (this_msg,&prm,&que,adr,sender);
				break;
			    case MT_DMPDU:
				processDR (this_msg,&prm,&que,adr,sender);
				break;
			}
			break;
		}

	}

	if (rp_isbad (retval = wr_q2dr (&que, this_msg))) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure '%d'",mychan->ch_name,retval));
		(void) delivery_resetDRs (int_Qmgr_status_messageFailure);
	}

	rd_end();
	q_free (&que);
	prm_free(&prm);
	return deliverystate;
}




/* ---  --- */
static int submit_error (recip, proc, reply)
ADDR	*recip;
char	*proc;
RP_Buf	*reply;
{
	char	buf[BUFSIZ];
	PP_LOG (LLOG_EXCEPTIONS,
	       ("Chans/list %s failure [%s]", proc, reply->rp_line));

	if (recip != NULLADDR) {
		(void) sprintf (buf,
				"'%s' failure for '%s' [%s]",
				proc,
				this_msg,
				reply -> rp_line);
		PP_OPER(NULLCP,("%s", buf));
		delivery_setstate (recip->ad_no, 
				   int_Qmgr_status_messageFailure,
				   buf);
	}

	start_submit = TRUE;
	io_end (NOTOK);

	return OK;
}

static int processMsg (msg, prm, que, recip, origsender)
char		*msg;
struct prm_vars	*prm;
Q_struct	*que;
ADDR		*recip;
ADDR		*origsender;
{
	ADDR	*expanded,
		*ix,
		*sender;
	RP_Buf	reply;
	char	*msgdir = NULLCP,
		file[MAXPATHLENGTH],
		buf[BUFSIZ],
		*strippedname;
	int	dirlen,
		n,
		fd_in;
	int size;
	struct stat st;
	struct timeval data_time;
#ifdef	DIRLIST
	Attr_Sequence	policy_as;
#else
	char	*local;
#endif

	if (qid2dir (msg, recip, TRUE, &msgdir) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("msg dir not found for recip %d of msg '%s'",
			 recip->ad_no, msg));
		delivery_setstate (recip->ad_no, 
				   int_Qmgr_status_messageFailure,
				   "source directory not found");
		return 0;
	}

	if (que->dl_expansion_prohibited == TRUE) {
		delivery_set (recip -> ad_no,
			      (first_failureDR == TRUE) ? 
			      int_Qmgr_status_negativeDR : int_Qmgr_status_failureSharedDR);
		first_failureDR = FALSE;
		(void) sprintf (buf, "DL expansion prohibited for this message");
		set_1dr (que, recip -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER, DRD_DL_EXPANSION_PROHIBITED,
			 buf);
		return 0;
	}
	
	/* --- expand list and resubmit message --- */
	q_init (&qs);

	q_almost_dup (&qs, que);

	if (recip -> ad_content != NULLCP) {
		if (qs.cont_type) free(qs.cont_type);
		qs.cont_type = strdup(recip -> ad_content);
	} else if (qs.cont_type != NULLCP) {
		free(qs.cont_type);
		qs.cont_type = NULLCP;
	}

	if (recip -> ad_eit != NULLIST_BPT) 
		qs.encodedinfo.eit_types = list_bpt_dup (recip->ad_eit);
	else
		qs.encodedinfo.eit_types = list_bpt_dup (que->encodedinfo.eit_types);

	if (qs.ua_id) {
		free(qs.ua_id);
		qs.ua_id = NULLCP;
	}
	qs.priority = PRIO_NONURGENT;
	qs.disclose_recips = FALSE;
	adrno = 1;

	expansionLoop = FALSE;
	switch (expandList (recip,&expanded,& (qs.dl_expansion_history))) {
	    case NOTOK:
		delivery_set (recip->ad_no, 
			      (first_failureDR == TRUE) ? 
			      int_Qmgr_status_negativeDR : 
			      int_Qmgr_status_failureSharedDR);
		first_failureDR = FALSE;
		(void) sprintf (buf, "Unable to expand list '%s'",get_adrstr (recip));
		set_1dr (que, recip->ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER, DRD_DL_EXPANSION_FAILURE, buf);
		return 0;
	    case DONE:
		delivery_setstate (recip->ad_no,
				   (mychan -> ch_sort[0] == CH_SORT_USR) ?
				   int_Qmgr_status_mtaFailure : 
				   int_Qmgr_status_messageFailure,
				   "temporary list processing failure (better check the logs)");
		return 0;
	    default:
		break;
	}


	/* --- expands to nothing so done --- */
	if (expanded == NULLADDR) {
		if (expansionLoop == TRUE)
			set_success(recip, que, 0);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("list '%s' is an empty list", recip->ad_value));
		return 0;
	}

#ifdef	DIRLIST
	if (check_dl_permission (origsender,this_list) == NOTOK) {
		delivery_set(recip->ad_no, 
			     (first_failureDR == TRUE) ? int_Qmgr_status_negativeDR : int_Qmgr_status_failureSharedDR);
		first_failureDR = FALSE;
		(void) sprintf(buf, "Distribution list policy prevents expansion of '%s'",get_adrstr(recip));
		set_1dr(que, recip->ad_no,  this_msg,
		       DRR_UNABLE_TO_TRANSFER,
		       DRD_DL_EXPANSION_FAILURE,
		       buf);
		return 0;
	}

	if (((policy_as = as_find_type(this_list,at_Policy)) != NULLATTR) &&
	    (policy_as->attr_value != NULLAV)) {
		struct dl_policy * policy;

		policy = (struct dl_policy *) policy_as->attr_value->avseq_av.av_struct;

		if (policy->dp_expand)
			qs.dl_expansion_prohibited = FALSE;
		else
			qs.dl_expansion_prohibited = TRUE;

		switch (policy->dp_priority) {
		case DP_LOW:
			qs.priority = PRIO_NONURGENT; break; 
		case DP_HIGH:
			qs.priority = PRIO_URGENT; break; 
		case DP_NORMAL:
			qs.priority = PRIO_NORMAL; break; 
		case DP_ORIGINAL:
			qs.priority = que->priority; break; 
		}

		switch (policy->dp_convert) {
		case DP_ORIGINAL: 
			qs.implicit_conversion_prohibited = que->implicit_conversion_prohibited ;  break;
		case DP_FALSE: 
			qs.implicit_conversion_prohibited = FALSE; break;
		case DP_TRUE: 
			qs.implicit_conversion_prohibited = TRUE;  break;
		}

	} else {
		/* Assume default policy */
		qs.dl_expansion_prohibited = FALSE;
		qs.priority = PRIO_NONURGENT;
		qs.implicit_conversion_prohibited = que->implicit_conversion_prohibited;
	}

	if ((sender = construct_sender (this_list)) == NULLADDR) 
		return submit_error(recip,"construct sender",&reply);
#else
	if ((local = ad_getlocal (recip->ad_r822adr, AD_822_TYPE)) == NULLCP)
		sender = tb_getModerator(recip->ad_r822adr);
	else {
		sender = tb_getModerator (local);
		free (local);
	}
	if (sender == NULLADDR)
		return 0;
#endif
	qs.inbound = list_rchan_new (loc_dom_site,mychan->ch_name);
	sender->ad_status = AD_STAT_DONE;
	sender->ad_resp = NO;
	prm->prm_opts = prm->prm_opts | PRM_ACCEPTALL | PRM_NOTRACE;
	/* --- now resubmit --- */

	timer_start(&data_time);
	if (rp_isbad (io_wprm (prm, &reply))) 
		return submit_error (recip,"io_wprm",&reply);

	if (rp_isbad (io_wrq (&qs, &reply))) 
		return submit_error (recip,"io_wrq",&reply);

	if (sender->ad_redirection_history != (Redirection *) NULL) {
		/* free of existing redirection history */
		redirect_free(sender->ad_redirection_history);
		sender->ad_redirection_history = (Redirection *) NULL;
	}
	if (rp_isbad (io_wadr (sender, AD_ORIGINATOR, &reply))) {
		char	nbuf[BUFSIZ];
		(void) sprintf(nbuf, "io_wadr(%s)", sender->ad_value);
		return submit_error (recip,nbuf,&reply);
	}
	
	ix = expanded;
	while (ix != NULL) {
		if (ix->ad_redirection_history != (Redirection *) NULL) {
			/* free of existing redirection history */
			redirect_free(ix->ad_redirection_history);
			ix->ad_redirection_history = (Redirection *) NULL;
		}
		ix->ad_explicitconversion = recip->ad_explicitconversion;
		if (rp_isbad (io_wadr (ix, AD_RECIPIENT, &reply))) {
			char	nbuf[BUFSIZ];
			(void) sprintf(nbuf, "io_wadr(%s)",
				       ix -> ad_value);
			return submit_error (recip,nbuf, &reply);
		}
		ix = ix->ad_next;
	}

	if (rp_isbad (io_adend (&reply)))
		return submit_error (recip,"io_adend", &reply);

	/* --- send over body --- */
	if (rp_isbad (io_tinit (&reply)))
		return submit_error (recip,"io_tinit",&reply);

	dirlen = strlen (msgdir) +1;

	msg_rinit (msgdir);

	size = 0;
	while (msg_rfile (file) != RP_DONE) {

		/* --- transmit file --- */
		strippedname = file + dirlen;
		if (stat (file, &st) != NOTOK)
			size += st.st_size;
		if (linked == TRUE) {
			(void) sprintf(buf, "%s %s",strippedname, file);
			if (rp_isbad (io_tpart (buf, TRUE, &reply))) 
				return submit_error (recip,"io_tpart",&reply);
		} else {
			if (rp_isbad (io_tpart (strippedname, FALSE, &reply))) 
				return submit_error (recip,"io_tpart",&reply);

			if ((fd_in = open (file, O_RDONLY)) == -1) {
				(void) strcpy (reply.rp_line,file);
				return submit_error (recip,"open",&reply);
			}
			while ((n = read (fd_in, buf, BUFSIZ)) > 0) {
				if (rp_isbad (io_tdata (buf, n))) {
					(void) strcpy (reply.rp_line,"???");
					return submit_error (recip,"io_tdata",&reply);
				}
			}

			close (fd_in);
			if (rp_isbad (io_tdend (&reply)))
				return submit_error (recip,"io_tdend", &reply);
		}
	}
	msg_rend();
	if (rp_isbad (io_tend (&reply)))
		return submit_error (recip,"io_tend", &reply);
	set_success (recip,que, size);
	timer_end(&data_time, size, "Data submitted");
	return 0;
}



static void set_success (recip, que, size)
ADDR		*recip;
Q_struct	*que;
int		size;
{
	if (recip->ad_usrreq == AD_USR_CONFIRM ||
	    recip->ad_mtarreq == AD_MTA_CONFIRM ||
	    recip->ad_mtarreq == AD_MTA_AUDIT_CONFIRM) 
	{
		set_1dr (que, recip->ad_no, this_msg,
			 DRR_NO_REASON, -1, NULLCP);
		delivery_set (recip->ad_no, 
			     (first_successDR == TRUE) ? 
				int_Qmgr_status_positiveDR : 
				int_Qmgr_status_successSharedDR);
		first_successDR = FALSE;
	}
	else {
		 (void) wr_ad_status (recip, AD_STAT_DONE);
		 (void) wr_stat (recip, que, this_msg, size);
		 delivery_set (recip->ad_no, int_Qmgr_status_success);
	}
}



/* ---  --- */
static ADDR *getnthrecip (que, num)
Q_struct	*que;
int		num;
{
	ADDR *ix = que->Raddress;

	if (num == 0)
		return que->Oaddress;
	while ((ix != NULL) && (ix->ad_no != num))
		ix = ix->ad_next;
	return ix;
}


/*  */
#ifdef	DIRLIST
static ADDR *construct_sender(as)
Attr_Sequence 	as;
{
	ADDR 	*ret = NULLADDR;
	ADDR 	*get_manager();
	ADDR 	*get_postmaster();

	if ((ret = get_manager(as)) == NULLADDR)
		return (get_postmaster());

	return ret;
}
#endif

/* ---  --- */
static char *getORname (adr)
ADDR	*adr;
{
	OR_ptr 	tree = NULLOR,
		new = NULLOR;
	char	buf[BUFSIZ],
		*value;

	bzero (buf, BUFSIZ);
	if (adr -> ad_r400adr != NULLCP)
		return strdup(adr -> ad_r400adr);

	value = adr -> ad_value;

	if ((or_rfc2or (value, &tree) != OK) || tree == NULLOR) { 
		PP_LOG (LLOG_EXCEPTIONS, 
			 ("getORname: Failed to parse '%s'", value));
		if (adr->ad_dn)
			return strdup(adr->ad_dn);
		return NULLCP; 
	}

	new = or_default (tree);
	or_or2std (new, buf, 0);
	if (new) or_free (new);

	return strdup (buf);
}



static void postExpansion (adr, pdlh)
ADDR		*adr;
DLHistory	**pdlh;
{
	char	*orname;
	
	if ((orname = getORname (adr)) == NULLCP)
		return;
	dlh_add (pdlh,dlh_new (orname, NULLCP, NULLUTC));
	free (orname);
}



static int expandedBefore (adr, dlh)
ADDR		*adr;
DLHistory	*dlh;
{
	char	*orname;
	int	found = FALSE;

	if ((orname = getORname (adr)) == NULLCP)
		return FALSE;

	while (found == FALSE && dlh != NULL) {
		if (strcmp (orname, dlh->dlh_addr) == 0)
			found = TRUE;
		else 
			dlh = dlh -> dlh_next;
	}

	free (orname);
	return found;
} 


#ifdef	DIRLIST
static ADDR *dl2addr(as, pnum)
Attr_Sequence as;
int	*pnum;
{
Attr_Sequence tmp;
ADDR	*ret = NULLADDR;
ADDR    *next;
AV_Sequence avs;
int     num = 0;

	/* Find mhsDLmembers */
	if ((tmp = as_find_type(as,at_Member)) == NULLATTR)
		return NULLADDR;

	/* Convert to ADDR */
	for (avs=tmp->attr_value; avs!= NULLAV; avs=avs->avseq_next) {
		if ((next = ORName2ADDR ((ORName *)avs->avseq_av.av_struct,TRUE)) == NULLADDR)
			return NULLADDR;
		next->ad_extension = num;
		next->ad_no = num;
		num++;
		adr_add(&ret, next);
	}
        *pnum = num;
	return ret;

}
#else
static ADDR *dl2addr (indl, pnum)
dl	*indl;
int	*pnum;
{
	ADDR	*ret = NULLADDR;
	Name	*list = indl -> dl_list;
	int	type = AD_ANY_TYPE;

	for (*pnum=0; list != NULL; list = list -> next, (*pnum)++) 
		adr_add (&ret, adr_new (list->name, type, adrno++));

	return ret;
}
#endif


static int attemptExpansion (key, type, padr, complain)
char	*key;
int	type;
ADDR	**padr;
int	complain;
{
#ifndef	DIRLIST	
	dl	*list;
#endif
	char	*local;
	int	count;
	*padr = NULLADDR;

	if ((local = ad_getlocal (key,type)) != NULLCP) {
		PP_NOTICE(("Attempting to expand list '%s'", local));
#ifdef	DIRLIST
		switch (dir_getdl(local, &this_list)) {
		    case OK:
			*padr = dl2addr(this_list, &count);
			PP_NOTICE(("Expanded list '%s' to %d recipient%s", 
				   local, count,
				   (count == 1) ? "" : "s"));
			free(local);
			return OK;
		    case DONE:
			PP_NOTICE (("Temporary failure to expand list '%s'",
				    local));
			free (local);
			return DONE;
		    default:
			PP_NOTICE (("Failed to find list '%s' in the directory", local));
			free (local);
			break;
		}
#else
		switch (tb_getdl (local, &list,complain)) {
		    case OK:
			*padr = dl2addr (list, &count);
			dl_free (list);
			PP_NOTICE(("Expanded list '%s' to %d recipient%s", 
				   local, count,
				   (count == 1) ? "" : "s"));
			free (local);
			return OK;
		    case DONE:
			if (list != NULL)
				dl_free(list);
			PP_NOTICE (("Temporary failure to expand list '%s'", 
				    local));
			free(local);
			return DONE;
		    default:
			if (list != NULL)
				dl_free(list);
			PP_NOTICE (("Failed to expand list '%s'", local));
			free (local);
			break;
		} 
#endif
	}

#ifdef DONT_WANT_THIS_ANYMORE

	PP_NOTICE(("Attempting to expand list '%s'", key));

#ifdef	DIRLIST
	switch (dir_getdl(key, &this_list)) {
	    case OK:
		*padr = dl2addr(this_list, &count);
		PP_NOTICE(("Expanded list '%s' to %d recipient%s", 
			   key, count,
			   (count == 1) ? "" : "s"));
		return OK;
	    case DONE:
		PP_NOTICE (("Temporary failure to expand list '%s'",
			    key));
		return DONE;
	    default:
		PP_NOTICE (("Failed to expand list '%s'", key));
		break;
	}
#else

	switch (tb_getdl (key,&list,complain)) {
	    case OK:
		*padr = dl2addr (list, &count);
		dl_free (list);
		PP_NOTICE(("Expanded list '%s' to %d recipients", 
			   key, count));
		return OK;
	    case DONE:
		if (list != NULL)
			dl_free(list);
		PP_NOTICE (("Temporary failure to expand list '%s'", 
			    key));
		return DONE;
	    default:
		if (list != NULL)
			dl_free(list);
		PP_NOTICE (("Failed to expand list '%s'", key));
		break;
	} 
#endif

#endif
	return NOTOK;
}



/* --- rm this from the list --- */
static adr_rm (this, list)
ADDR	*this,
	**list;
{
	ADDR	*ix;

	/* --- bullet proofing --- */
	if (this == NULLADDR || list == NULL)
		return;

	if (this == *list) 
		/* --- first in list (easy) --- */
		*list = (*list) -> ad_next;
	else {
		ix = *list;
		while ( ix != NULLADDR
		       && ix -> ad_next != this)
			ix = ix -> ad_next;
		if (ix != NULLADDR)
			ix -> ad_next = this -> ad_next;
	}
}



static int expandList (orig, plist, pdlh)
ADDR		*orig,
		**plist;
DLHistory	**pdlh;
{
	ADDR	*new,
		*temp,
		*ix;
	RP_Buf	rp;
	int	do_next = TRUE;

#ifdef	DIRLIST
	Attr_Sequence	save_as;
#endif

	*plist = NULLADDR;

	if (expandedBefore (orig, *pdlh) == TRUE) {
		expansionLoop = TRUE;
		return TRUE;
	}
	switch(attemptExpansion (get_adrstr (orig),
				 get_adrtype (orig),
				 &new,
				 OK)) {
	    case OK:
		postExpansion (orig,pdlh);
		break;
	    case DONE:
		/* temporary failure */
		return DONE;
	    default:
		/* --- cannot expand starting list --- */
		return NOTOK;
	}
	
	adr_add (plist, new);
	ix = new;
#ifdef	DIRLIST
	save_as = this_list;
#endif

	while (expandSublists == TRUE && ix != NULLADDR) {
#ifdef UKORDER
		if (!rp_isbad(ad_parse(ix, &rp, CH_UK_PREF))
#else
		if (!rp_isbad(ad_parse(ix, &rp, CH_USA_PREF))
#endif
		    && ix->ad_outchan
		    && ix->ad_outchan->li_chan
		    && lexequ(ix->ad_outchan->li_chan->ch_name, 
			      mychan->ch_name) == 0) {
			/* attempt to expand sublist */
			if (expandedBefore (ix, *pdlh) == TRUE) {
				temp = ix;
				ix = ix -> ad_next;
				do_next = FALSE;
				expansionLoop = TRUE;
				adr_rm (temp, plist);
				adr_free (temp);
			} 
			else if (attemptExpansion (get_adrstr(ix),
						   get_adrtype(ix),
						   &new,
						   NOTOK) == OK) {
				postExpansion (ix, pdlh);
				temp = ix;
				adr_rm (temp, plist);
				adr_free (temp);
				adr_add (plist, new);
				ix = new;
				do_next = FALSE;
			}
		}
		if (do_next == TRUE)
			ix = ix -> ad_next;
		else
			do_next = TRUE;
	}
#ifdef	DIRLIST
	this_list = save_as;
#endif
	return OK;
}


static char  *get_adrstr (adr)
ADDR	*adr;
{
	char	*key;
	
	switch (mychan->ch_out_ad_type) {
	    case AD_X400_TYPE:
		key = adr->ad_r400adr;
		break;
	    case AD_822_TYPE:
		key = adr->ad_r822adr;
		break;
	    default:
		switch (adr->ad_type) {
		    case AD_X400_TYPE:
			key = adr->ad_r400adr;
			break;
		    case AD_822_TYPE:
			key = adr->ad_r822adr;
			break;
		    default:
			key = adr->ad_value;
			break;
		}
		break;
	}

	return key;
}

static int get_adrtype (adr)
ADDR	*adr;
{
	switch (mychan->ch_out_ad_type) {
	    case AD_X400_TYPE:
	    case AD_822_TYPE:
		return mychan->ch_out_ad_type;
	    default:
		return adr->ad_type;
	}
}

/*  */
/* DR to list maintainer */

static int processDR (msg, prm, que, recip, origsender)
char		*msg;
struct prm_vars	*prm;
Q_struct	*que;
ADDR		*recip;
ADDR		*origsender;
{
	DRmpdu drs, *dr = &drs;
	char	*msgdir, *key, *local, file[MAXPATHLENGTH],
		*strippedname, buf[BUFSIZ];
	ADDR	*moderator = NULLADDR;
	RP_Buf	reply;
	int	retval, size, dirlen, fd_in, n;
	struct stat st;
	struct timeval data_time;
	
	/* read in DR */
	dr_init(dr);
	if (rp_isbad(get_dr(recip->ad_no,
			    this_msg,
			    dr))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("report.%d not found for msg '%s'",
			 recip->ad_no, msg));
		delivery_setstate (recip->ad_no, 
				   int_Qmgr_status_messageFailure,
				   "DR not found");
		return 0;
	}
	/* get Moderator of list (copied from above) */
	key = (origsender->ad_type == AD_X400_TYPE) ? origsender->ad_r400adr : 
		origsender->ad_r822adr;
	if ((local = ad_getlocal(key, origsender->ad_type, YES)) == NULLCP)
		local = strdup(key);
	
#ifdef DIRLIST
	switch (dir_getdl(local, &this_list)) {
	    case OK:
		break;

	    case DONE:
		/* temp failure */
		PP_LOG (LLOG_EXCEPTIONS,
			("Tempory failure to access list '%s'",
			 local));
		delivery_setstate (recip->ad_no, 
				   int_Qmgr_status_messageFailure,
				   "temporary X.500 problem");
		return 0;
		
	    default:
		/* perm failure */
		/* hack up moderator and send to postmaster */
		moderator = adr_new(getpostmaster(AD_822_TYPE), 
				    AD_822_TYPE, 0);
	}
	if (moderator == NULLADDR
	    && (moderator = construct_sender (this_list)) == NULLADDR) 
		return submit_error(recip,"construct sender",&reply);
#else
	moderator = tb_getModerator(local);
#endif
	free(local);

	timer_start(&data_time);
	if (rp_isbad (io_wprm (prm, &reply))) 
		return submit_error (recip,"io_wprm",&reply);

	if (rp_isbad (io_wrq (que, &reply))) 
		return submit_error (recip,"io_wrq",&reply);

	if (moderator->ad_redirection_history != (Redirection *) NULL) {
		/* free of existing redirection history */
		redirect_free(moderator->ad_redirection_history);
		moderator->ad_redirection_history = (Redirection *) NULL;
	}
	if (rp_isbad (io_wadr (moderator, AD_ORIGINATOR, &reply))) {
		char	nbuf[BUFSIZ];
		(void) sprintf(nbuf, "io_wadr(%s)",
			       moderator -> ad_value);
		return submit_error (recip,nbuf, &reply);
	}

	if (rp_isbad (io_wadr (recip, AD_RECIPIENT, &reply))) {
		char	nbuf[BUFSIZ];
		(void) sprintf(nbuf, "io_wadr(%s)", recip->ad_value);
		return submit_error (recip,nbuf,&reply);
	}

	if (rp_isbad (io_adend (&reply)))
		return submit_error (recip,"io_adend", &reply);

	if (rp_isbad (io_wdr (dr, &reply)))
		return submit_error(recip, "io_wdr", &reply);

	if (rp_isbad (io_tinit (&reply)))
		return submit_error (recip, "io_tinit", &reply);

	if (qid2dir (msg, recip, TRUE, &msgdir) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("msg dir not found for recip %d of msg '%s'",
			 recip->ad_no, msg));
		delivery_setstate (recip->ad_no, 
				   int_Qmgr_status_messageFailure,
				   "source directory not found");
		return 0;
	}

	dirlen = strlen (msgdir) +1;

	if (msg_rinit(msgdir) != NOTOK) {
		size = 0;
		while (msg_rfile (file) != RP_DONE) {

			/* --- transmit file --- */
			strippedname = file + dirlen;
			if (stat (file, &st) != NOTOK)
				size += st.st_size;
			if (linked == TRUE) {
				(void) sprintf(buf, "%s %s",strippedname, file);
				if (rp_isbad (io_tpart (buf, TRUE, &reply))) 
					return submit_error (recip,"io_tpart",&reply);
			} else {
				if (rp_isbad (io_tpart (strippedname, FALSE, &reply))) 
					return submit_error (recip,"io_tpart",&reply);

				if ((fd_in = open (file, O_RDONLY)) == -1) {
					(void) strcpy (reply.rp_line,file);
					return submit_error (recip,"open",&reply);
				}
				while ((n = read (fd_in, buf, BUFSIZ)) > 0) {
					if (rp_isbad (io_tdata (buf, n))) {
						(void) strcpy (reply.rp_line,"???");
						return submit_error (recip,"io_tdata",&reply);
					}
				}

				close (fd_in);
				if (rp_isbad (io_tdend (&reply)))
					return submit_error (recip,"io_tdend", &reply);
			}
		}
		msg_rend();
	}
		
	if (rp_isbad (io_tend (&reply)))
		return submit_error (recip, "io_tend", &reply);
	set_success (recip, que, size);
	timer_end(&data_time, size, "Data submitted");
	return 0;
}
	
#ifndef lint

void    advise (va_alist)
va_dcl
{
    int     code;
    va_list ap;

    va_start (ap);

    code = va_arg (ap, int);

    (void) _ll_log (log_dsap, code, ap);

    va_end (ap);
}

#else
/* VARARGS */

void    advise (code, what, fmt)
char    *what,
        *fmt;
int      code;
{
    advise (code, what, fmt);
}
#endif
