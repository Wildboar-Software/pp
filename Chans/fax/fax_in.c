/* faxd.c: PP skelton for inbound fax deamon */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/RCS/fax_in.c,v 6.0 1991/12/18 20:07:09 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/RCS/fax_in.c,v 6.0 1991/12/18 20:07:09 jpo Rel $
 *
 * $Log: fax_in.c,v $
 * Revision 6.0  1991/12/18  20:07:09  jpo
 * Release 6.0
 *
 */
#include	"util.h"
#include	"IOB-types.h"
#include	"prm.h"
#include	"q.h"
#include	"retcode.h"
#include	"sys.file.h"
#include	"ap.h"
#include	"or.h"
#include <stdarg.h>
#include 	"faxgeneric.h"

void    advise (int, char *, char *, ...);
void    adios (char *, char *, ...);
#define ps_advise(ps, f) \
        advise (LLOG_EXCEPTIONS, NULLCP, "%s: %s",\
                (f), ps_error ((ps) -> ps_errno))

extern FaxCtlr	*init_faxctrlr();
extern struct type_IOB_ORName	*orn2orname();
extern char	*quedfldir;

static void dirinit();
static int init_PP_stuff(), decode_ch_in_info();

CHAN	*mychan;
struct	type_IOB_InformationObject	*infoobj = NULL;
struct	prm_vars	params;
Q_struct		Qstruct;
FaxCtlr	*faxctl = NULL;
char msgid [LINESIZE];

/* not actually used */
int table_verified = FALSE;

main (argc, argv)
int	argc;
char	**argv;
{   
	struct type_IOB_G3FacsimileBodyPart     *g3fax;
	sys_init (argv[0]);
	dirinit();

	if ((faxctl = init_faxctrlr(FAX_INBOUND,
				    argc, 
				    argv)) == (FaxCtlr *) NULL) {
		err_abrt(RP_MECH,
			 "Unable to initialise fax driver specific elements");
	}

	if (init_PP_stuff(faxctl) != OK)
		err_abrt(RP_MECH,
			 "Unable to initialise PP specfics");
	fillin_infoobj(&infoobj, faxctl);

	/* daemon so continually loop */
	while (1) {
		if (faxctl->receiveG3Fax == NULLIFP) {
			err_abrt(RP_MECH,
				 "No receiveG3Fax procedure given for fax driver");
		}

		if ((*faxctl->receiveG3Fax) (faxctl, &g3fax) != OK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("fax receiveG3Fax failed ['%s']",
				 faxctl->errBuf));
		} else if (deal_with_fax (faxctl, g3fax) != OK) {
			/* have accepted fax from remote */
			/* ought to dump somewhere */
			/* so as not to lose it */
			PP_LOG(LLOG_EXCEPTIONS,
			       ("failed to deal with incoming fax"));
		}

		if (g3fax)
			free_IOB_G3FacsimileBodyPart (g3fax);
	}
}

/* --- routine to move to correct place in file system --- */
static void dirinit()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO,
			  "Unable to change directory to '%s'", quedfldir);
}

/*  */

/* wrap up fax and submit */
deal_with_fax(faxctl, fax)
FaxCtlr	*faxctl;
struct type_IOB_G3FacsimileBodyPart	*fax;
{
	PE	pe = NULLPE;
	
	if (infoobj -> un.ipm -> heading -> this__IPM != 
	    (struct type_IOB_IPMIdentifier *) NULL)
		free_IOB_IPMIdentifier (infoobj -> un.ipm -> heading -> this__IPM);
	gen_mid (&(infoobj -> un.ipm -> heading -> this__IPM));
	infoobj -> un.ipm -> body ->BodyPart->un.g3__facsimile = fax;
	if (encode_IOB_InformationObject (&pe, 1, 0, NULLCP, infoobj) != OK) {
		PP_OPER(NULLCP,
			("deal_with_fax failed to encode info object [%s]", PY_pepy));
/*		send_to_printer(fax)*/
		return NOTOK;
	}
	
	submitPE (pe);
	
	if (pe != NULLPE) pe_free(pe);

	return OK;
}

/*  */

int	io_running = FALSE;

stop_io()
{
	if (io_running == TRUE)
		io_end(NOTOK);
	io_running = FALSE;
}

submitPE (pe)
PE	pe;
{
	PS	ps;
        RP_Buf                  rps, *rp = &rps;
	
	if (io_running == FALSE) {
		if (rp_isbad (io_init(rp))) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("io_init err %s", rp -> rp_line));
			exit (1);
		}
		io_running = TRUE;
	}

	if ((ps = ps_alloc(str_open)) == NULLPS) {
		ps_advise (ps, "ps_alloc");
		exit(1);
	}

	if (str_setup (ps, NULLPE, 0, 0) == NOTOK) {
		advise (LLOG_EXCEPTIONS, NULLCP, "str_setup loses");
		exit (1);
	}

	if (pe2ps(ps, pe) == NOTOK) {
		ps_advise(ps, "pe2ps loses");
		exit(1);
	}

	if (rp_isbad (io_wprm (&params, rp))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("io_wprm err %s",
			rp -> rp_line));
		stop_io();
		exit (1);
	}
        if (rp_isbad (io_wrq (&Qstruct, rp))) {
                PP_LOG (LLOG_EXCEPTIONS, ("io_wrq err %s",
                                          rp -> rp_line));
		stop_io();
		exit(1);
        }

        if (rp_isbad (io_wadr (Qstruct.Oaddress, AD_ORIGINATOR, rp))) {
                PP_OPER (NULLCP, ("io_wadr originator/postmaster err %s",
                                          rp -> rp_line));
		stop_io();
		exit (1);
        }

        if (rp_isbad (io_wadr (Qstruct.Raddress, AD_RECIPIENT, rp))) {
                PP_LOG (LLOG_EXCEPTIONS, ("io_wadr recipient err %s",
                                          rp -> rp_line));
		stop_io();
		exit (1);
	}

        if (rp_isbad (io_adend (rp))) {
                PP_LOG (LLOG_EXCEPTIONS, ("io_adend err %s",
                                          rp -> rp_line));
		stop_io();
		exit (1);
        }

	if (rp_isbad (io_tinit (rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("tinit blows %s", rp -> rp_line));
		stop_io();
	}
	if (rp_isbad (io_tpart (Qstruct.cont_type, FALSE, rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("io_tpart blows %s", rp -> rp_line));
		stop_io();
		exit (1);
	}
	if (rp_isbad (io_tdata(ps->ps_base, 
			       ps -> ps_ptr - ps -> ps_base))) {
		PP_LOG(LLOG_EXCEPTIONS, ("io_tdata error"));
		stop_io();
		exit (1);
	}
	
	if (rp_isbad (io_tdend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("io_tdend fails"));
		stop_io();
		exit(1);
	}
	
	if (rp_isbad (io_tend (rp))) {
		PP_LOG (LLOG_EXCEPTIONS, ("io_tend fails"));
		stop_io();
		exit(1);
	}

	ps_free(ps);
}

/*  */
/* copied from dr2rfc code */

/* build up structures need to submit fax into PP */
extern char	*loc_dom_mta, *cont_p22, *hdr_p22_bp, *hdr_p2_bp;
extern char	*getpostmaster();

static init_PP_stuff(faxctl)
FaxCtlr	*faxctl;
{
        EncodedIT               *ep = &Qstruct.encodedinfo;

	if ((mychan = ch_nm2struct (faxctl->channel)) == NULLCHAN) {
		PP_OPER (NULLCP,
			 ("Channel '%s' not known", faxctl->channel));
		exit(1);
	}

	if (decode_ch_in_info(mychan -> ch_in_info, faxctl) != OK)
		return NOTOK;
	
	prm_init(&params);
	params.prm_opts = PRM_ACCEPTALL | PRM_NOTRACE;
	q_init (&Qstruct);
	Qstruct.msgtype = MT_UMPDU;
	Qstruct.inbound = list_rchan_new (loc_dom_mta,
					  faxctl->channel);
	Qstruct.cont_type = strdup(mychan -> ch_content_in);

	/* eits completely run by tailoring */
	ep->eit_types = NULL;
	if (mychan->ch_hdr_in != NULLIST_BPT)
		list_bpt_add(&ep->eit_types, 
			     list_bpt_dup(mychan -> ch_hdr_in));
	else
		list_bpt_add(&ep->eit_types, 
			     list_bpt_new((lexequ(mychan -> ch_content_in, 
						  cont_p22) ==  0) ? 
					  hdr_p22_bp : hdr_p2_bp));
	if (mychan->ch_bpt_in != NULLIST_BPT)
		list_bpt_add(&ep->eit_types, 
			     list_bpt_dup(mychan -> ch_bpt_in));
	else
		list_bpt_add(&ep->eit_types,
			     list_bpt_new("g3fax"));

        /* -- create the ADDR struct for orig and recipient -- */
        Qstruct.Oaddress            = adr_new (getpostmaster(mychan->ch_in_ad_type), 
					       mychan->ch_in_ad_type, 0);
        Qstruct.Raddress            = adr_new (faxctl->fax_recip, 
					       mychan->ch_in_ad_type, 1);
	return OK;
}

static int decode_ch_in_info (info, faxctl)
char	*info;
FaxCtlr	*faxctl;
{
	char  *info_copy;
	int   margc,i;
	char  *margv[100];

	if (info == NULLCP) 
		return OK;

	info_copy = strdup(info);
	margc = sstr2arg(info_copy, 100, margv, ",=");
  
	for (i = 0; i < margc; i++) {
		/* PP specific args */
		if (faxctl->arg_parse != NULLIFP) {
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

	
/*  */
/* copied from p2flatten code */

fillin_infoobj(pinfoobj, faxctl)
struct type_IOB_InformationObject	**pinfoobj;
FaxCtlr	*faxctl;
{
	*pinfoobj = (struct type_IOB_InformationObject *) calloc(1,
						  sizeof(struct type_IOB_InformationObject));
	(*pinfoobj)->offset = type_IOB_InformationObject_ipm;
	fillin_ipm(&((*pinfoobj)->un.ipm), faxctl);
}

int	fillin_ipm(pipm, faxctl)
struct type_IOB_IPM	**pipm;
FaxCtlr	*faxctl;
{
	*pipm = (struct type_IOB_IPM *)
		calloc(1, sizeof(struct type_IOB_IPM));
	fillin_heading(&((*pipm)->heading), faxctl);
	fillin_body(&((*pipm)->body), faxctl);
}

int	fillin_heading(pheading, faxctl)
struct type_IOB_Heading	**pheading;
FaxCtlr	*faxctl;
{
	/* stright from rfc1148 code */
	struct type_IOB_Heading *head;

	head = (struct type_IOB_Heading *)
		calloc (1, sizeof (struct type_IOB_Heading));

	fillin_sender (&(head->originator), faxctl);
	fillin_recip_seq(&(head->primary__recipients), faxctl);
	fillin_subject(&(head->subject), faxctl);
		
	*pheading = head;
}

int	fillin_sender (poriginator, faxctl)
struct type_IOB_OriginatorField	*poriginator;
FaxCtlr	*faxctl;
{
	fillin_ORdesc(poriginator, faxctl,
		      getpostmaster(mychan->ch_in_ad_type),
		      mychan->ch_in_ad_type);
}

int	fillin_recip_seq(or_recip_ptr, faxctl)
struct type_IOB_RecipientSequence	**or_recip_ptr;
FaxCtlr	*faxctl;
{
	*or_recip_ptr = (struct type_IOB_RecipientSequence *)
		calloc (1, sizeof(struct type_IOB_RecipientSequence));
	fillin_recip(&(*or_recip_ptr)->RecipientSpecifier, faxctl);
}

int	fillin_recip(precip, faxctl)
struct type_IOB_RecipientSpecifier	**precip;
FaxCtlr	*faxctl;
{
	*precip = (struct type_IOB_RecipientSpecifier *)
		calloc(1, sizeof(struct type_IOB_RecipientSpecifier));
	fillin_ORdesc(&(*precip)->recipient, faxctl, 
		      faxctl->fax_recip, mychan->ch_in_ad_type);
}

int	fillin_ORdesc(pdesc, faxctl, addr, type)
struct type_IOB_ORDescriptor	**pdesc;
FaxCtlr	*faxctl;
char	*addr;
int	type;
{
	char	buf[BUFSIZ];
	PE	pe;
	ORName	orn;
	int	retval;
	OR_ptr	or;
	*pdesc = (struct type_IOB_ORDescriptor *)
		calloc (1, sizeof(struct type_IOB_ORDescriptor));

	switch (type) {
	    case AD_X400_TYPE:
		or = or_std2or(addr);
		break;
	    default:
		sprintf(buf, "%s", addr);
		retval = or_rfc2or(buf, &or);
		break;
	}

	or = or_default(or);
	orn.on_or = or;
	orn.on_dn = (DN) NULL;
	(*pdesc)->formal__name	= orn2orname(&orn);
	or_free(or);
}		

int	fillin_subject(subject_ptr, faxctl)
struct type_IOB_SubjectField	**subject_ptr;
FaxCtlr	*faxctl;
{
	*subject_ptr = (struct type_IOB_SubjectField *) 
		calloc (1, sizeof(struct type_IOB_SubjectField));
	
	*subject_ptr = str2qb (faxctl->subject, 
			       strlen(faxctl->subject), 1);
}

int	fillin_body(pbody, faxctl)
struct type_IOB_Body 	**pbody;
FaxCtlr	*faxctl;
{
	struct type_IOB_Body	*temp;

	temp = (struct type_IOB_Body *) 
		calloc(1, sizeof(struct type_IOB_Body));
	temp -> BodyPart = (struct type_IOB_BodyPart *)
		calloc(1, sizeof(struct type_IOB_BodyPart));
	temp->BodyPart->offset = type_IOB_BodyPart_g3__facsimile;
	*pbody = temp;
}

gen_mid (mid_ptr)               /* generate  random mailid */
struct type_IOB_IPMIdentifier **mid_ptr;
{
	OR_ptr	or;
	ORName	orn;
	long	now;

	if (*mid_ptr)
		free_IOB_IPMIdentifier(*mid_ptr);
	*mid_ptr = (struct type_IOB_IPMIdentifier *) 
		calloc (1,sizeof (struct type_IOB_IPMIdentifier));
 
	(void) time (&now);
        sprintf (msgid, "%ld %s", getpid(), ctime (&now));
        msgid [strlen(msgid) - 1] = '\0';
	(*mid_ptr)->user__relative__identifier = str2qb(msgid,
							strlen(msgid),
							1);
	or = or_std2or(getpostmaster(AD_X400_TYPE));
	or = or_default(or);
	orn.on_or = or;
	orn.on_dn = (DN) NULL;
	(*mid_ptr)->user = orn2orname(orn);
	or_free(or);

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

#ifndef lint
void    adios (char *what, char *fmt, ...)
{
        va_list ap;

        va_start (ap, fmt);

        _ll_log (pp_log_norm, LLOG_FATAL, what, fmt, ap);

        va_end (ap);

        _exit (1);
}
#else
/* VARARGS2 */

void    adios (what, fmt)
char   *what,
       *fmt;
{
        adios (what, fmt);
}
#endif


#ifndef lint
void    advise (int code, char *what, char *fmt, ...)
{
        va_list ap;

        va_start (ap, fmt);

        _ll_log (pp_log_norm, code, what, fmt, ap);

        va_end (ap);
}
#else
/* VARARGS3 */

void    advise (code, what, fmt)
char   *what,
       *fmt;
int     code;
{
        advise (code, what, fmt);
}
#endif
