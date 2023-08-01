/* warnings.c: send warning messages back to originator */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/warnings/RCS/warnings.c,v 6.0 1991/12/18 20:13:32 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/warnings/RCS/warnings.c,v 6.0 1991/12/18 20:13:32 jpo Rel $
 *
 * $Log: warnings.c,v $
 * Revision 6.0  1991/12/18  20:13:32  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "retcode.h"
#include "expand.h"
#include "qmgr.h"
#include "q.h"
#include "prm.h"
#include <sys/stat.h>

static int initialise();
static struct type_Qmgr_DeliveryStatus *process(), *submit_warning();
static int endfunc();
static void dirinit();
static ADDR *getnthrecip();

extern char *quedfldir, *loc_dom_site, *loc_dom_mta,
	*getpostmaster(), *hdr_822_bp, *ia5_bp;

int 	start_submit;
CHAN	*mychan;
#ifdef COPYTO_ADDRESS
char	*copyAddr = NULLCP;
#endif

/*  */
/* main routine */

main (argc, argv)
int 	argc;
char	**argv;
{
	sys_init(argv[0]);
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && strcmp(argv[1], "debug") == 0)
		debug_channel_control(argc,argv,initialise,process,endfunc);
	else
#endif
		channel_control(argc,argv,initialise,process,endfunc);
}

/*  */
/* move to correct place in file system */

static void dirinit()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO,
			  "Unable to change directory to '%s'",
			  quedfldir);
}

/*  */
/* initialise */

static void decode_ch_info (info)
char	*info;
{
	char	*info_copy;
	int	margc, i;
	char	*margv[100];
	
#ifdef COPYTO_ADDRESS
	copyAddr = NULLCP;
#endif
	info_copy = strdup(info);
	margc = sstr2arg(info_copy, 100, margv, ",=");
	
	for (i = 0; i < margc; i++) {
		if (!isstr(margv[i]))
			continue;
#ifdef COPYTO_ADDRESS
		if (lexequ(margv[i], "copy-to") == 0) {
			i++;
			if (i >= margc)
				copyAddr = getpostmaster(mychan->ch_out_ad_type);
			else
				copyAddr = strdup(margv[i]);
		} else
#endif
			PP_LOG (LLOG_EXCEPTIONS,
				("Unrecognised ch_out_info element '%s'",
				 margv[i]));
	}
}

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char	*name = qb2str(arg);
	
	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP,
			("warnings: Channel '%s' not knpown",
			 name));
		if (name) free(name);
		return NOTOK;
	}
	
	/* check is a warning channel */
	if (mychan->ch_chan_type != CH_WARNING) {
		PP_OPER(NULLCP,
			("warnings: Channel '%s' is not specified as type warning",
			 name));
		if (name) free(name);
	}
	if (name) free(name);

	if (mychan->ch_out_info)
		decode_ch_info(mychan->ch_out_info);

	start_submit = TRUE;

	return OK;
}

/*  */

static void stop_io()
{
	if (start_submit == FALSE)
		io_end(OK);
	start_submit = TRUE;
}

/* ARGSUSED */
static int endfunc(arg)
struct type_Qmgr_Channel	*arg;
{
	stop_io();
}

/*  */
/* routine to check if allowed to filter this message through this channel */
static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
 	char 		*msg_file = NULL, *msg_chan = NULL;
	int		result;

	result = TRUE;
	msg_file = qb2str (msg->qid);	
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp(msg_chan,mychan->ch_name) != 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/fcontrol channel err: '%s'",msg_chan));
		result = FALSE;
	}

	/* free all storage used */
	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	return result;
}

/*  */
/* routine called to send warning */

static struct type_Qmgr_DeliveryStatus *process(arg)
struct type_Qmgr_ProcMsg	*arg;
{
	struct prm_vars	prm;
	Q_struct	que;
	ADDR		*sender = NULLADDR;
	ADDR		*recips = NULLADDR;
	char		*this_msg;
	int		rcounts;
	struct type_Qmgr_DeliveryStatus	*retval;


	prm_init (&prm);
	q_init(&que);

	sender = recips = NULLADDR;

	delivery_init (arg->users);
	delivery_setall (int_Qmgr_status_success); /* always lie! */

	if (security_check(arg) != TRUE)
		return deliverystate;
	
	/* ok - send a warning */
	this_msg = qb2str (arg->qid);
	
	
	if (rp_isbad(rd_msg(this_msg, &prm, &que, 
			    &sender, &recips, &rcounts))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("warnings: rd_msg error '%s'", this_msg));
		rd_end();
		if (this_msg) free(this_msg);
		q_free(&que);
		
		return deliverystate;
	}
	
	if (que.nwarns == NOTOK) /* old message */
		return delivery_setall(int_Qmgr_status_success); /* lie */
	fillin_expansions(&que, arg->users, this_msg);
	
	PP_NOTICE(("sending warning for msg %s", this_msg));
	if ((retval = submit_warning(&que)) != 
	    (struct type_Qmgr_DeliveryStatus *) OK) {
		rd_end();
		return retval;
	}

	inc_warnings(&que);
	
	rd_end();

	if (this_msg) free(this_msg);
	q_free(&que);
	prm_free(&prm);
	
	return delivery_setall (int_Qmgr_status_success);
}

/*  */

inc_warnings(q)
Q_struct	*q;
{
	q->nwarns++;
	wr_q_nwarns(q);
}

/*  */
/* fillin the various things that can be expanded in the warning message */

Expand expand_macros[] = {
	"sender",	NULLCP,	/* 0 */
	"822sender",	NULLCP,	/* 1 */
	"400sender",	NULLCP,	/* 2 */
	"qid",		NULLCP,	/* 3 */
	"ua-id",	NULLCP,	/* 4 */
	"p1-id",	NULLCP,	/* 5 */
	"recips",	NULLCP,	/* 6 */
	"822recips",	NULLCP,	/* 7 */
	"400recips",	NULLCP,	/* 8 */
	"locmta",	NULLCP,	/* 9 */
	"locsite",	NULLCP, /* 10 */
	"hours-left",	NULLCP,	/* 11 */
	NULLCP, NULLCP};

static char	*add_string(porig, add)
char	**porig;
char	*add;
{
	char	*new;

	if (*porig == NULLCP) 
		new = strdup(add);
	else {
		new = malloc (strlen (*porig)
			      + strlen ("\n")
			      + strlen (add)
			      + 1);
		(void) sprintf(new, "%s\n%s",
			       *porig,
			       add);
		free(*porig);
	}
	return new;
}

fillin_expansions(q, users, this_msg)
Q_struct	*q;
struct type_Qmgr_UserList	*users;
char		*this_msg;
{
	struct type_Qmgr_UserList	*ix;
	ADDR	*adr;
	char	*recips, *recips822, *recips400;
	char	hbuf[64];
	time_t now;

	recips = recips822 = recips400 = NULLCP;

	expand_macros[0].expansion = q->Oaddress->ad_value;
	expand_macros[1].expansion = q->Oaddress->ad_r822adr;
	expand_macros[2].expansion = q->Oaddress->ad_r400adr;
	expand_macros[3].expansion = this_msg;
	expand_macros[4].expansion = q->ua_id;
	expand_macros[5].expansion = q->msgid.mpduid_string;
	
	for (adr = q -> Raddress; adr; adr = adr -> ad_next) {
		if (adr -> ad_status == AD_STAT_PEND) {
			recips = add_string(&recips, adr->ad_value);
			recips822 = add_string(&recips822, adr->ad_r822adr);
			recips400 = add_string(&recips400, adr->ad_r400adr);
		}
	}
	if (expand_macros[6].expansion)
		free(expand_macros[6].expansion);
	expand_macros[6].expansion = recips;

	if (expand_macros[7].expansion)
		free(expand_macros[7].expansion);
	expand_macros[7].expansion = recips822;

	if (expand_macros[8].expansion)
		free(expand_macros[8].expansion);
	expand_macros[8].expansion = recips400;
	expand_macros[9].expansion = loc_dom_mta;
	expand_macros[10].expansion = loc_dom_site;
	(void) time (&now);
	(void) sprintf (hbuf, "%d", 
			q -> retinterval - 
				((now - utc2time_t(q -> queuetime)) / 3600));
	expand_macros[11].expansion = strdup(hbuf);
}

/*  */
/* submit warning message */

static char	*expand_line(line)
char	*line;
{
	char	*margv[100], *ret, *cp;
	int	margc, len, i;
	
	cp = strdup(line);
	compress(cp, cp);
	if (*cp == '\0') {
		free(cp);
		return strdup(line);
	}
	if (index (line, '$')) 
		ret = expand_dyn (line, expand_macros);
	else 
		ret = strdup (line);
	return ret;
}

/* for now */
extern char	*wrndfldir, *wrnfile;

static char	*q2warnfile(q)
Q_struct	*q;
{
	static char	filename[MAXPATHLENGTH];
	struct stat statbuf;
	
	(void) sprintf(filename,
		       "%s/%s.%d",
		       wrndfldir,
		       wrnfile,
		       q->nwarns + 1);

	if (stat(filename, &statbuf) == OK)
		return filename;

	/* try for default warning */
	(void) sprintf(filename,
		       "%s/%s",
		       wrndfldir,
		       wrnfile);
	if (stat(filename, &statbuf) == OK)
		return filename;
	return NULLCP;
}

static void submit_error(proc, rp)
char	*proc;
RP_Buf	*rp;
{
	PP_LOG(LLOG_EXCEPTIONS,
	       ("Chans/warnings %s failure [%s]",
		proc, rp->rp_line));
	start_submit = TRUE;
	io_end(NOTOK);
}

#define fold_and_submit_line submit_line

static struct type_Qmgr_DeliveryStatus *submit_line(line, rp)
char	*line;
RP_Buf	*rp;
{
	if (rp_isbad(io_tdata (line, strlen(line)))) {
		submit_error ("io_tdata", rp);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_tdata failed");
	}
	return (struct type_Qmgr_DeliveryStatus *) OK;
}

static struct type_Qmgr_DeliveryStatus *submit_warning(q)
Q_struct	*q;
{
	struct prm_vars	prm;
	Q_struct	qs;
	char		*warningfile;
	RP_Buf		reply;
	char		buffer[BUFSIZ], timestr[BUFSIZ], *expanded;
	struct type_Qmgr_DeliveryStatus *retval;
	FILE		*fp;
	UTC	now, start_time, utclocalise ();


	now = utcnow ();
	start_time = utclocalise (now);
	free ((char *)now);

	prm_init(&prm);
	q_init(&qs);
	
	qs.inbound = list_rchan_new (loc_dom_site, 
				     mychan->ch_name);
	qs.encodedinfo.eit_types = list_bpt_new (hdr_822_bp);
	list_bpt_add(&qs.encodedinfo.eit_types,
		     list_bpt_new (ia5_bp));
	qs.Oaddress = adr_new (getpostmaster(AD_822_TYPE), AD_822_TYPE, 0);
	qs.Raddress = adr_new (q->Oaddress->ad_r822adr, AD_822_TYPE, 1);
#ifdef COPYTO_ADDRESS
	if (copyAddr != NULLCP)
		qs.Raddress->ad_next = adr_new(copyAddr, 
					       mychan->ch_out_ad_type, 1);
#endif
	
	/* select which template warning message to use */
	if ((warningfile = q2warnfile(q)) == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("unable to find warnings template warning"));
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "unable to find warnings template file");
	}

	/* check submission started */
	if (start_submit == TRUE
	    && rp_isbad (io_init (&reply))) {
		submit_error ("io_init", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "unable to start submit");
	}

	if (rp_isbad (io_wprm(&prm, &reply))) {
		submit_error ("io_wprm", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_wprm failed");
	}

	if (rp_isbad (io_wrq(&qs, &reply))) {
		submit_error ("io_wrq", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_wrq failed");
	}

	/* from postmaster@loc_dom_site */
	if (rp_isbad(io_wadr (qs.Oaddress, AD_ORIGINATOR, &reply))) {
		submit_error ("io_wadr(postmaster)", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_wadr(postamster) failed");
	}
	
	/* to originator of message */

	if (rp_isbad(io_wadr (qs.Raddress, AD_RECIPIENT, &reply))) {
		submit_error ("io_wadr(recipient)", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_wadr(recipient) failed");
	}

#ifdef COPYTO_ADDRESS
	if (copyAddr != NULLCP &&
	    qs.Raddress->ad_next != NULLADDR) {
		if (rp_isbad(io_wadr (qs.Raddress->ad_next, AD_RECIPIENT, &reply))) {
			submit_error ("io_wadr(recipient)", &reply);
			return delivery_setallstate (int_Qmgr_status_messageFailure,
						     "io_wadr(recipient) failed");
		}
	}
#endif

	if (rp_isbad (io_adend (&reply))) {
		submit_error ("io_adend", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_adend failed");
	}

	if (rp_isbad (io_tinit(&reply))) {
		submit_error ("io_tinit", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_tinit failed");
	}

	/* submit header */
	
	if (rp_isbad (io_tpart (hdr_822_bp, FALSE, &reply))) {
		submit_error ("io_tpart", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_tpart failed");
	}

	(void) sprintf(buffer, "From: %s\n", qs.Oaddress->ad_value);
	
	if ((retval = submit_line(buffer, &reply)) != 
	    (struct type_Qmgr_DeliveryStatus *) OK) {
		return retval;
	}

	(void) sprintf(buffer, "To: %s\n", qs.Raddress->ad_value);
	
	if ((retval = submit_line(buffer, &reply)) != 
	    (struct type_Qmgr_DeliveryStatus *) OK) {
		return retval;
	}

#ifdef COPYTO_ADDRESS
	if (copyAddr != NULLCP) {
		(void) sprintf(buffer, "Cc: %s\n", 
			       copyAddr);
		if ((retval = submit_line(buffer, &reply)) != 
		    (struct type_Qmgr_DeliveryStatus *) OK) {
			return retval;
		}
	}
#endif
	(void) UTC2rfc (start_time, timestr);
	free ((char *)start_time);
	start_time = NULLUTC;
	
	(void) sprintf(buffer,
		       "Date: %s\n",
		       timestr);

	if ((retval = submit_line(buffer, &reply)) != 
	    (struct type_Qmgr_DeliveryStatus *) OK) {
		return retval;
	}


	(void) sprintf(buffer, 
		       "Subject: WARNING: message delayed at \"%s\"\n",
		       loc_dom_site);

	if ((retval = submit_line(buffer, &reply)) != 
	    (struct type_Qmgr_DeliveryStatus *) OK) {
		return retval;
	}
	
	if (rp_isbad(io_tdend (&reply))) {
		submit_error("io_tdend", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_tdend failed");
	}

	(void) sprintf(buffer, "1.%s", ia5_bp);
	if (rp_isbad(io_tpart(buffer, FALSE, &reply))) {
		submit_error ("io_tpart", &reply);
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "io_tpart failed");
	}

	/* submit text of message */

	if ((fp = fopen (warningfile, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS,
			 warningfile,
			 ("Can't open file"));
		stop_io();
		return delivery_setallstate(int_Qmgr_status_messageFailure,
					    "failed to open warnings file");
	}

	while (fgets (buffer, sizeof buffer, fp) != NULL) {
		/* expand macros in that line */

		if ((expanded = expand_line(buffer)) == NULLCP) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unable to expand line in warning file '%s'",
			       warningfile));
			fclose(fp);
			stop_io();
			return delivery_setallstate 
				(int_Qmgr_status_messageFailure,
				 "expansion of line failed");
		}
       
		if ((retval = fold_and_submit_line(expanded, &reply)) != 
		    (struct type_Qmgr_DeliveryStatus *) OK) {
			fclose (fp);
			return retval;
		}
	
		free(expanded);

	}	

	fclose(fp);
	if (rp_isbad(io_tdend (&reply))
	    || rp_isbad(io_tend (&reply))) {
		submit_error("io_tend failed", &reply);
		return delivery_setallstate(int_Qmgr_status_messageFailure,
					    "io_tend failed");
	}
	
	return (struct type_Qmgr_DeliveryStatus *) OK;
}
    
static ADDR *getnthrecip(que, num)
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
