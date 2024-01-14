/* fax_out.c: PP skelton for outbound fax channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/RCS/fax_out.c,v 6.0 1991/12/18 20:07:09 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/RCS/fax_out.c,v 6.0 1991/12/18 20:07:09 jpo Rel $
 *
 * $Log: fax_out.c,v $
 * Revision 6.0  1991/12/18  20:07:09  jpo
 * Release 6.0
 *
 */


#include	"util.h"
#include	"retcode.h"
#include 	"prm.h"
#include	"q.h"
#include	"qmgr.h"
#include	"dr.h"
#include 	<isode/cmd_srch.h>
#include 	"tb_bpt88.h"
#include        "IOB-types.h"
#include        "MTA-types.h"
#include 	<sys/stat.h>
#include 	<sys/time.h>
#include <stdarg.h>
#include	"faxgeneric.h"

extern char	*quedfldir;
extern FaxCtlr	*init_faxctrlr();

static int initialise(), endfunc(), faxit(), decode_ch_out_info();
static struct type_Qmgr_DeliveryStatus	*process();
static void dirinit(), douser();
static char *get_fax_number();
static char	*this_msg;
static int firstSuccessDR, firstFailureDR;

void    advise ();
void    adios ();
#define ps_advise(ps, f) \
        advise (LLOG_EXCEPTIONS, NULLCP, "%s: %s",\
                (f), ps_error ((ps) -> ps_errno))

int 		alwaysConfirm, table_verified, fax_debug;
CHAN		*mychan;
Q_struct	Qstruct;
FaxCtlr		*faxctl = NULL;

main (argc, argv)
int	argc;
char	**argv;
{
	sys_init (argv[0]);
	dirinit ();
	fax_debug = 0;

	if ((faxctl = init_faxctrlr(FAX_OUTBOUND,
				    argc,
				    argv)) == (FaxCtlr *) NULL) {
		err_abrt(RP_MECH, 
			 "Unable to initialise fax driver specific elements");
	}

#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1], "debug") == 0)) {
		fax_debug = 1;
		debug_channel_control (argc, argv, initialise, process, endfunc);
	} else
#endif
		channel_control (argc, argv, initialise, process, endfunc);
}


/* --- routine to move to correct place in file system --- */
static void dirinit()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO,
			  "Unable to change directory to '%s'", quedfldir);
}
	
/* ARGSUSED */
static int endfunc (arg)
struct type_Qmgr_Channel	*arg;
{
	if (faxctl->close != NULLIFP)
		(*faxctl->close)(faxctl);
}

/*  --- channel initialise routines --- */

static int initialise (arg)
struct type_Qmgr_Channel	*arg;
{
	char	*name;

	name = qb2str (arg);

	if ((mychan = ch_nm2struct (name)) == NULLCHAN) {
		PP_OPER (NULLCP,
			 ("Channel '%s' not known", name));
		if (name != NULLCP)
			free(name);
		return NOTOK;
	}

	if (name != NULLCP)
		free(name);
	
 	if (decode_ch_out_info(mychan -> ch_out_info, faxctl) != OK)
		return NOTOK;

	if (faxctl->open != NULLIFP
	    &&	(*faxctl->open) (faxctl) != OK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("fax open failed ['%s']",
			 faxctl->errBuf));
		return NOTOK;
	}
	
	return OK;
}

/*  --- main processing routine --- */

static struct type_Qmgr_DeliveryStatus	*process (arg)
struct type_Qmgr_ProcMsg	*arg;
{
	struct type_Qmgr_UserList	*up;
	struct prm_vars			prm;
	Q_struct			*qp = &Qstruct;
	ADDR				*ad_sendr = NULLADDR,
					*ad_recip = NULLADDR;
	int				retval, ad_count;
	
	bzero ((char *) &prm, sizeof prm);
	bzero ((char *) qp, sizeof *qp);
	firstSuccessDR = firstFailureDR = TRUE;

	this_msg = qb2str (arg -> qid);
	
	PP_NOTICE (("Processing msg %s", this_msg));

	delivery_init (arg -> users);

	retval = rd_msg (this_msg, &prm, qp, &ad_sendr,
			 &ad_recip, &ad_count);

	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("rd_msg err: %s", this_msg));
		return delivery_setallstate (int_Qmgr_status_mtaFailure,
					     "Can't read message");
	}

	PP_NOTICE(("Fax sender: %s", ad_sendr->ad_value));

	for (up = arg -> users; up; up = up -> next)
		douser (up -> RecipientId -> parm, ad_recip, this_msg);
	if (rp_isbad(wr_q2dr (&Qstruct, this_msg))) {
		PP_LOG (LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure",mychan->ch_name));
		(void) delivery_resetDRs (int_Qmgr_status_mtaFailure);
	}
	rd_end();
	prm_free(&prm);
	q_free(qp);

	return deliverystate;
}

/* 
 * process one extension-id for this message
 */

static void douser (rno, ad_recip, msgd)
int	rno;
ADDR	*ad_recip;
char	*msgd;
{
	ADDR	*ap;

	PP_TRACE (("douser (%d, ad_recip, %s)", rno, msgd));

	for (ap = ad_recip; ap; ap = ap -> ad_next) {
		if (rno != ap -> ad_no) 
			continue;
		
		if (lchan_acheck (ap, mychan, 1, NULLVP) == NOTOK) {
			return;
		} else {
		 	faxit (ap);
			return;
		}
	}

	PP_LOG (LLOG_EXCEPTIONS,
		("user not recipient of %s", this_msg));

	delivery_setstate (rno, int_Qmgr_status_mtaFailure,
			   "user is not recipient of message");
}

/* 
 *	If status is not related to a failure DR, then just return status.
 *	If it is a negativeDR, then check if this is the first failure. If
 *	so, return negativeDR, otherwise return failure shared DR.
 */
static 
filterQmgrStatus(status)
int	status;
{
	int ret = status;

	if (status == int_Qmgr_status_negativeDR) {
		if (!firstFailureDR)
			ret = int_Qmgr_status_failureSharedDR;
		firstFailureDR = FALSE;
	} 

	return(ret);
}

/*
 * 
 */

static int	faxit (ap)
ADDR	*ap;
{
	int	retval = NOTOK;
	char	file[MAXPATHLENGTH], next[MAXPATHLENGTH];
	struct stat st;
	int size;
	struct timeval start, stop;
	char	*formatdir = NULLCP;


	size = 0;
	if (qid2dir (this_msg, ap, TRUE, &formatdir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't locate message %s", this_msg));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_mtaFailure,
				   "Can't find message");
		return RP_MECH;
	}

	if (rp_isbad (retval = msg_rinit (formatdir))) {
		PP_LOG (LLOG_EXCEPTIONS, ("msg_rinit can't init %s", formatdir));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_mtaFailure,
				   "can't read the body");
		return RP_MECH;
	}

#ifdef DONE_IN_INITIALISE	        
	if ((*faxctl->open)(faxctl) != OK) {
		PP_LOG (LLOG_EXCEPTIONS, ("%s", faxctl->errBuf));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_mtaFailure, 
				   faxctl->errBuf);
		return RP_MECH;
	}
#endif

	/* get phone number */
{
	char	*telno;
	faxctl->errBuf[0] = '\0';
	if (ap && (telno = get_fax_number(ap, faxctl)) != NULLCP) {
		PP_NOTICE(("faxing to %s for recipient %s", telno,ap->ad_value));
		strcpy(faxctl->telno, telno);
		free(telno);
	} else {
		if (faxctl->errBuf[0] != '\0')
			sprintf(faxctl->errBuf,
				"Can't determine phone number for address '%s'",
				ap -> ad_r400adr);
		PP_LOG (LLOG_EXCEPTIONS, ("%s", faxctl->errBuf));
		if (faxctl->abort != NULLIFP)
			(*faxctl->abort)(faxctl);
		delivery_setstate(ap -> ad_no, 
				  filterQmgrStatus(int_Qmgr_status_negativeDR), 
				  faxctl->errBuf);
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER, -1, faxctl->errBuf);
		return RP_MECH;
	}
}

#ifdef	SVR4								  
	(void) gettimeofday(&start);
#else
	(void) gettimeofday(&start, (struct timezone *)NULL);
#endif
	faxctl->errBuf[0] = '\0';
	if (faxctl->initXmit != NULLIFP
	    && rp_isbad((*faxctl->initXmit)(faxctl))) {
		if (faxctl->abort != NULLIFP)
			(*faxctl->abort)(faxctl);
		PP_LOG (LLOG_EXCEPTIONS, ("initXmit failed [%s]", 
					  faxctl->errBuf));
		delivery_setstate (ap -> ad_no, 
				   filterQmgrStatus(faxctl->qmgrErrCode), 
				   faxctl->errBuf);
		if (faxctl->qmgrErrCode == int_Qmgr_status_negativeDR)
			set_1dr (&Qstruct, ap -> ad_no, this_msg,
				 DRR_UNABLE_TO_TRANSFER, -1, faxctl->errBuf);
		return RP_MECH;
	}

	if (msg_rfile (next) != RP_OK) {
		if (faxctl->abort != NULLIFP)
			(*faxctl->abort)(faxctl);
		PP_LOG(LLOG_EXCEPTIONS, ("Empty directory %s", formatdir));
		delivery_setstate (ap -> ad_no, int_Qmgr_status_mtaFailure,
				   "empty directory");
		return RP_MECH;
	} else {
		retval = OK;
		do {
			strcpy(file,next);
			if (msg_rfile(next) != RP_OK)
				next[0] = '\0';
			if (stat (file, &st) != NOTOK)
				size += st.st_size;
			if (rp_isbad(fax_bodypart (faxctl, 
						   file, 
						   (next[0] == '\0')))) {
				PP_LOG (LLOG_EXCEPTIONS, ("fax_bodypart failed [%s]", 
							  faxctl->errBuf));
				delivery_setstate (ap -> ad_no, 
						   filterQmgrStatus(faxctl->qmgrErrCode), 
						   faxctl->errBuf);
				if (faxctl->qmgrErrCode == int_Qmgr_status_negativeDR)
					set_1dr (&Qstruct, ap -> ad_no, 
						 this_msg,
						 DRR_UNABLE_TO_TRANSFER, -1, 
						 faxctl->errBuf);
				if (faxctl->abort != NULLIFP)
					(*faxctl->abort)(faxctl);
				retval = NOTOK;
			}
		} while (retval == OK && next[0] != '\0');

		msg_rend();

		if (retval == OK) {
			if ((*faxctl->termXmit)(faxctl) != OK) {
				PP_LOG(LLOG_EXCEPTIONS, ("termXmit failed [%s]", 
							 faxctl->errBuf));
				delivery_setstate (ap -> ad_no, 
						   filterQmgrStatus(faxctl->qmgrErrCode), 
						   faxctl->errBuf);
				if (faxctl->qmgrErrCode == int_Qmgr_status_negativeDR)
					set_1dr (&Qstruct, ap -> ad_no, 
						 this_msg,
						 DRR_UNABLE_TO_TRANSFER, -1, 
						 faxctl->errBuf);
				if (faxctl->abort != NULLIFP)
					(*faxctl->abort)(faxctl);
				retval = NOTOK;
			}
		}
	}

#ifdef	SVR4
	(void) gettimeofday (&stop);
#else
	(void) gettimeofday(&stop, (struct timezone *)NULL);
#endif

	if (retval == OK) {
		PP_NOTICE((">>> Fax: msg sent to %s in %d seconds", 
			   ap -> ad_value,
			   stop.tv_sec - start.tv_sec));

		if (alwaysConfirm == TRUE ||
		    ap -> ad_usrreq == AD_USR_CONFIRM ||
		    ap -> ad_mtarreq == AD_MTA_CONFIRM ||
		    ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
			char	buf[BUFSIZ];
			if (lexequ(faxctl->telno, "0") == 0)
				sprintf(buf, "message locally printed");
			else
				sprintf(buf, "message fax'd to '%s'", 
					faxctl->telno);

			set_1dr (&Qstruct, ap -> ad_no, 
				 this_msg, DRR_NO_REASON, -1, buf);

			delivery_set (ap -> ad_no, 
				      (firstSuccessDR == TRUE) ?
				      int_Qmgr_status_positiveDR :
				      int_Qmgr_status_successSharedDR);
			firstSuccessDR = FALSE;
		}
		else {
			(void) wr_ad_status (ap, AD_STAT_DONE);
			(void) wr_stat (ap, &Qstruct, this_msg, size);
			delivery_set (ap -> ad_no, int_Qmgr_status_success);
		}
	}
	return retval;
}

/* 
 * send a bodypart to fax machine
 */

extern CMD_TABLE bptbl_body_parts88[/* x400 84 body_parts */];

static int	type_of_file(file)
char	*file;
{
	char	*dot;
	if ((dot = rindex(file, '.')) == NULLCP)
		return NOTOK;
	dot++;
	if (lexequ(dot, rcmd_srch(BPT_G3FAX, bptbl_body_parts88)) == 0)
		return BPT_G3FAX;
	if (lexequ(dot, rcmd_srch(BPT_IA5, bptbl_body_parts88)) == 0)
		return BPT_IA5;
	return NOTOK;
}

int	num_sent;

static int fax_bodypart (faxctl, file, last)
FaxCtlr	*faxctl;
char	*file;
int	last;
{
	int	type;
	num_sent = 0;
	switch ((type = type_of_file(file))) {
	    case BPT_G3FAX:
		if (faxctl->sendPage != NULLIFP) 
			return fax_g3_bodypart (faxctl, file, last);
		break;
	    case BPT_IA5:
		if (faxctl->sendIA5File != NULLIFP)
			return fax_ia5_bodypart (faxctl, file, last);
		break;
	    default:
		break;
	}

	sprintf(faxctl->errBuf, "Unable to fax '%s' bodyparts",
		(type == NOTOK) ? file : rcmd_srch(type, bptbl_body_parts88));
	PP_LOG(LLOG_EXCEPTIONS, ("%s", faxctl->errBuf));
	return RP_MECH;
}

static int fax_ia5_bodypart (faxctl, file, last)
FaxCtlr *faxctl;
char	*file;
int	last;
{
	int	retcode;

	PP_DBG(("hello fax_ia5_bodypart"));
	if (faxctl->setIA5Params != NULLIFP
	    && (*faxctl->setIA5Params) (faxctl) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("setIA5Params failed [%s]",
			faxctl->errBuf));
		return RP_MECH;
	}
	
	retcode = (*faxctl->sendIA5File)(faxctl, file, last);
	if (retcode == OK)
		return(OK);
	else 
		return(RP_MECH);
}

static int fax_g3_bodypart (faxctl, file, last)
FaxCtlr *faxctl;
char	*file;
int	last;
{
        register PS     psin;
        FILE            *fp;
	struct type_IOB_G3FacsimileBodyPart	*g3fax = NULL;
	int		retval = RP_MECH;

	PP_DBG (("hello fax_g3_bodypart"));

	/* read in and decode fax bodypart */

	if ((fp = fopen (file, "r")) == (FILE *)0) {
		PP_SLOG(LLOG_EXCEPTIONS, file,
			("Can't open file"));
		sprintf(faxctl->errBuf,
			"Can't open file '%s'", file);
		faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
		return RP_MECH;
	}

	if ((psin = ps_alloc (std_open)) == NULLPS)
        {
                (void) fclose (fp);
                ps_advise (psin, "ps_alloc");
		sprintf(faxctl->errBuf,
			"ps_alloc failed");
		faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
                return RP_MECH;
        }

	if (std_setup (psin, fp) == NOTOK)
        {
                ps_free (psin);
                (void) fclose (fp);
                advise (LLOG_EXCEPTIONS, NULLCP, "%s: std_setup loses", file);
		sprintf(faxctl->errBuf,
			"std_setup failed");
		faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
                return RP_MECH;
        }

	if (do_decode_fax(psin, &g3fax) == NOTOK) {
                (void) fclose (fp);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("failed to decode G3FacsimileBodyPart"));
                ps_done(psin, "ps2pe");
		sprintf(faxctl->errBuf,
			"unable to decode fax bodypart");
		faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
                return RP_MECH;
        }

        (void) fclose (fp);


	if (g3fax->parameters
	    && faxctl->setG3Params != NULLIFP
	    && (*faxctl->setG3Params)(faxctl, g3fax->parameters) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("setG3Params failed [%s]", faxctl->errBuf));
		return RP_MECH;
	}
			
	if (fax_bitstrings (faxctl, g3fax, last) == OK)
		retval = OK;
/*	if (g3fax != NULL)
		free_IOB_G3FacsimileBodyPart(g3fax);*/
        return retval;
}

/* 
 * send each page of g3fax to fax machine 
 */

static int fax_bitstrings (faxctl, g3fax, last)
FaxCtlr	*faxctl;
struct type_IOB_G3FacsimileBodyPart	*g3fax;
int last;
{
	struct type_IOB_G3FacsimileData	*ix;
	int retval;
	
	for (ix = g3fax -> data; ix != NULL; ix = ix -> next) {
		int		len;
		char	*str;

		if ((str = bitstr2strb (ix -> element_IOB_0, &len)) == NULLCP) {
			PP_LOG(LLOG_EXCEPTIONS, ("empty page"));
			return NOTOK;
		}
		len = (len + BITSPERBYTE -1)/BITSPERBYTE;

		retval = faxctl->sendPage(faxctl, str, len, 
					  (last && ix -> next == NULL));
		free (str);
		if (retval != OK)
			return retval;
	}
	return  OK;
}

/*
 * parse info field
 */

static int decode_ch_out_info(info, faxctl)
char	*info;
FaxCtlr	*faxctl;
{
  char	*info_copy;
  int	margc,i;
  char	*margv[100];

  if (info == NULLCP) 
    return OK;

  info_copy = strdup(info);
  margc = sstr2arg(info_copy, 100, margv, ",=");
  
  alwaysConfirm = FALSE;

  for (i = 0; i < margc; i++) {
	  /* PP specific args */
	  if (lexequ(margv[i], "confirm") == 0) {
		  i++;
		  if (lexequ(margv[i], "always") == 0)
			  alwaysConfirm = TRUE;
		  else if (lexequ(margv[i], "never") == 0)
			  alwaysConfirm = FALSE;
	  } else if (faxctl->arg_parse != NULLIFP) {
		  /* pass to driver specific parser */
		  if ((*faxctl->arg_parse) (faxctl, 
					    margv[i], 
					    margv[i+1]) == OK)
			  i++;
		  else {
			  PP_LOG(LLOG_EXCEPTIONS,
				 ("%s", faxctl->errBuf));
			  free(info_copy);
			  return NOTOK;
		  }
	  } else {
		  PP_LOG(LLOG_EXCEPTIONS,
			 ("No arg_parse routine registered, unrecognised ch_info element '%s'",
			  margv[i]));
		  free(info_copy);
		  return NOTOK;
	  }
  }
  free(info_copy);
  return OK;
}

/*
 *
 * address to telephone number mapping via table lookup
 */

static void tblentry2num(buf, ptelno, porg)
char	*buf;
char	**ptelno;
char	**porg;
{
	/* parse as number,rest */
	char	*telno = NULLCP, *org = NULLCP, *ix;

	if ((ix = index(buf, ',')) == NULLCP) {
		compress(buf,buf);
		telno = strdup(buf);
	} else {
		*ix++ = '\0';
		compress(buf, buf);
		telno = strdup(buf);
		/* for now rest is company */
		if (*ix != '\0') 
			org = strdup(ix);
	}

	if (ptelno != NULL) 
		*ptelno = telno;
	else if (telno != NULLCP)
		free(telno);
	      
	if (porg != NULL)
		*porg = org;
	else if (org != NULLCP)
		free(org);
}

static char *get_fax_number(adr, faxctl)
ADDR	*adr;
FaxCtlr	*faxctl;
{
	OR_ptr	or, or2;
	char	*telno = NULLCP, buffer[BUFSIZ];
	int	using_outmta = FALSE;
	table_verified = FALSE;

	if (adr->ad_r400adr == NULLCP
	    || adr->ad_r400adr[0] != '/')
		return NULLCP;

	if ((or = or_std2or(adr->ad_r400adr)) == NULLOR) {
		sprintf(faxctl->errBuf,
			"Unable to parse X.400 address '%s'",
			adr->ad_r400adr);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s",faxctl->errBuf));
		return NULLCP;
	}
	or2 = or;
	while ((or2 = or_locate (or2, OR_DD)) != NULLOR) {
		if (cmd_srch(or2->or_ddname, 
			     ortbl_ddvalid) == OR_DDVALID_FAX) {
			telno = strdup (or2->or_value);
			break;
		} else if (or2 -> or_next != NULLOR)
			or2 = or2 -> or_next;
		else
			break;
		
	}
 	
	/* use outmta */
	if (telno == NULLCP 
	    && adr -> ad_outchan != NULLIST_RCHAN) {
		using_outmta = TRUE;
		telno = strdup(adr -> ad_outchan -> li_mta);
	} 
	if (telno != NULLCP
	    && mychan != NULLCHAN
	    && mychan -> ch_table != NULLTBL
	    && tb_k2val (mychan -> ch_table, telno, buffer, TRUE) != NOTOK) {
		table_verified = TRUE;
		PP_NOTICE(("fax key '%s' verfied in table '%s'",
			   telno, mychan -> ch_table -> tb_name));
		free (telno);
		tblentry2num(buffer, &telno, (char **) NULL);
	} else if (using_outmta == TRUE) {
		sprintf(faxctl->errBuf,
			"Unable to obtain remote fax info for address '%s'",
			adr->ad_r400adr);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		if (telno)
			free(telno);
		if (or)
			or_free(or);
		return NULLCP;
	}
	if (or)
		or_free(or);
	return telno;
}

/* 
 * decode from PE to C struct
 */

extern PE	ps2pe_aux ();

static int do_decode_fax (psin, pfax)
register PS	psin;
struct type_IOB_G3FacsimileBodyPart	**pfax;
{
	struct type_IOB_G3FacsimileData		*new, *tail = NULL;
	int					pages;
	register PE     			pe = NULLPE;
	
	/* get first chunk */
	if ((pe = ps2pe_aux (psin, 1, 0)) == NULLPE) {
		ps_done (psin, "ps2pe_aux");
		return NOTOK;
	}
	
	if ((PE_ID(pe -> pe_class, pe -> pe_id) !=  
	     PE_ID(PE_CLASS_UNIV, PE_CONS_SEQ))
	    && (PE_ID(pe -> pe_class, pe -> pe_id) !=
		PE_ID(PE_CLASS_CONT, (PElementID) 0x03))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unexpected pe in G3FacsimileBodyPart: expected sequence"));
		return NOTOK;
	}
	
	pe_free (pe);

	*pfax = (struct type_IOB_G3FacsimileBodyPart *) calloc (1,
							       sizeof (struct type_IOB_G3FacsimileBodyPart));
	
	/* get and decode parameters */
	if ((pe = ps2pe (psin)) == NULLPE) { /* EOF or error? */
		ps_done (psin, "ps2pe");
		return NOTOK;
        }
        PY_pepy[0] = 0;

	if (decode_IOB_G3FacsimileParameters(pe, 1, 
					    NULLIP, NULLVP, 
					    &((*pfax) -> parameters)) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("decode_IOB_G3FacsimileParameters failure [%s]",
			PY_pepy));
		pe_done(pe, "Parse failure IOB_G3FacsimileParameters");
		return NOTOK;
	}
	
	if (PY_pepy[0] != 0)
		PP_LOG(LLOG_EXCEPTIONS,
		       ("parse_IOB_G3FacsimileParameters non fatal failure [%s]",
                        PY_pepy));

	pe_free (pe);

	/* get next chunk */
	if ((pe = ps2pe_aux (psin, 1, 0)) == NULLPE) {
		ps_done (psin, "ps2pe_aux");
		return NOTOK;
	}
	
	if (PE_ID(pe -> pe_class, pe -> pe_id) != 
	    PE_ID(PE_CLASS_UNIV, PE_CONS_SEQ)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unexpected pe in G3FacsimileBodyPart: expected sequence"));
		return NOTOK;
	}

	pe_free (pe);
	
	pages = 0;
	while ((pe = ps2pe (psin)) != NULLPE) {
		switch (PE_ID(pe -> pe_class, pe -> pe_id)) {
		    case PE_ID (PE_CLASS_UNIV, PE_PRIM_BITS):
			break;
		    case PE_ID (PE_CLASS_UNIV, PE_UNIV_EOC) :
			pe_free(pe);
			continue;
		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unexpected pe in G3FacsimileBodyPart: expected bitstring"));
			return NOTOK;
		}
		new = (struct type_IOB_G3FacsimileData *) (calloc (1,
								  sizeof(struct type_IOB_G3FacsimileData)));
		new -> element_IOB_0 = prim2bit(pe);
		if ( (*pfax) -> data == NULL)
			(*pfax) -> data = tail = new;
		else {
			tail -> next = new;
			tail = tail -> next;
		}
		pages++;
	}
	
	if ((*pfax) -> parameters -> optionals == opt_IOB_G3FacsimileParameters_number__of__pages
	    && (*pfax) -> parameters -> number__of__pages != pages) {
		PP_LOG(LLOG_EXCEPTIONS, 
		       ("Incorrect number of pages in G3FacsimileBodyPart: expected %d, got %d",
			(*pfax) -> parameters -> number__of__pages, pages));
		return NOTOK;
	}
	return OK;
}

/* 
 * various isode-like routines
 */

/* pe_done: utility routine to do the right thing for pe errors */
int             pe_done (pe, msg)
PE              pe;
char            *msg;
{
        if (pe->pe_errno)
        {
                PP_OPER(NULLCP,
                        ("%s: [%s] %s",msg,PY_pepy,pe_error(pe->pe_errno)));
                pe_free (pe);
                return NOTOK;
        }
        else
        {
                pe_free (pe);
                return OK;
        }
}

/* ps_done: like pe_done */
int             ps_done (ps, msg)
PS              ps;
char           *msg;
{
        ps_advise (ps, msg);
        ps_free (ps);
        return NOTOK;
}

void adios (char *what, char* fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
	_ll_log (pp_log_norm, LLOG_FATAL, ap);
    va_end (ap);
    _exit (1);
}

void advise (char *what, char *fmt, ...)
{
	int code;
    va_list ap;
    va_start (ap, fmt);
	code = va_arg (ap, int);
    _ll_log (pp_log_norm, code, ap);
    va_end (ap);
}
