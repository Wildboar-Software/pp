/* rd_rfchdr.c: parse 822 headers */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/rd_rfchdr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/rd_rfchdr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: rd_rfchdr.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include <isode/cmd_srch.h>
#include "q.h"
#include "tb_q.h"
#include "or.h"
#include "ap.h"
#include "x400_ub.h"

extern void err_abrt();
extern char *compress();
static int hdr_parse ();
static int hdr_type (), fillin_DomId();
static void hdr_rcv ();
static void hdr_via ();
static void hdr_check ();
static void hdr_subject ();
static void hdr_date ();
static void hdr_msgid ();
static void hdr_remcomm ();
static void do_correlators ();
static int getline ();
static int got_x400_mts_id = FALSE;
static int ua_id_set_by_x400 = FALSE;
static void 	hdr_x400_content_id(),
		hdr_priority(),
		hdr_x400_mts_id(),
		hdr_dl_history(),
		hdr_conversion(),
		hdr_convert_with_loss(),
		hdr_x400_received(),
		hdr_def_deliv(),
		hdr_latest_time(),
		hdr_orig_eit (),
		hdr_prohibition (),
		hdr_gen_del_report(),
		hdr_prev_non_del_report();

static char cor_to[BUFSIZ], *cor_subject = NULLCP,
	*cor_date = NULLCP, *cor_mid = NULLCP;
static int cor_to_len;

#define ub_content_id_length	16
#define ub_content_correlator_length 512

/* -- basic states for the state machine -- */
#define HDV_EOH			0
#define HDV_NEW			1
#define HDV_MORE		2

/* -- munge the header lines of the message -- */
#define HDR_FROM		1
#define HDR_SENDER		2
#define HDR_REPLYTO		3
#define HDR_RECEIVED		4
#define HDR_VIA			5
#define HDR_DATE		6
#define HDR_MESSAGEID		7
#define HDR_SUBJECT		8

/* rfc 1138 fields */
/* mts */
#define HDR_X400_MTS_ID		9
#define	HDR_ORIG_EIT		10
#define HDR_CONTENT_IDENTIFIER	11
#define HDR_PRIORITY		12
#define HDR_DL_HISTORY		13
#define HDR_CONVERSION		18
#define HDR_CONVERT_WITH_LOSS	14
/* mta */
#define HDR_X400_RECEIVED	15
#define HDR_DEF_DELIV		16
#define HDR_LAST_DELIV		17

/* needed for content correlator */
#define HDR_TO			19

/* appendix H of rfc 1148 bis */
#define HDR_GEN_DEL_REPORT	20
#define HDR_PREV_NON_DEL_REPORT	21
#define HDR_ALT_RECIP		22
#define HDR_DISCLOSE_RECIPS	23
#define HDR_CONT_RETURN		24

/* appendix B of rfc 1148 bis */
#define HDR_ACK_TO		25
/* greybook specific so done in greybook */

static	CMD_TABLE  htbl_rfc [] = {
	"from",			HDR_FROM,
	"sender",		HDR_SENDER,
	"reply-to",		HDR_REPLYTO,
	"received",		HDR_RECEIVED,
	"via",			HDR_VIA,
	"date",			HDR_DATE,
	"message-id",		HDR_MESSAGEID,
	"subject",		HDR_SUBJECT,
/* rfc 1138 */
	"x400-mts-identifier",	HDR_X400_MTS_ID,
	"original-encoded-information-types",	HDR_ORIG_EIT,
	"content-identifier",	HDR_CONTENT_IDENTIFIER,
	"priority",		HDR_PRIORITY,
	"dl-expansion-history",	HDR_DL_HISTORY,
	"conversion",		HDR_CONVERSION,
	"conversion-with-loss",	HDR_CONVERT_WITH_LOSS,
	"x400-received",	HDR_X400_RECEIVED,
	"deferred-delivery",	HDR_DEF_DELIV,
	"latest-delivery-time",	HDR_LAST_DELIV,
	"to",			HDR_TO,
	"generate-delivery-report",	HDR_GEN_DEL_REPORT,
	"prevent-nondelivery-report",	HDR_PREV_NON_DEL_REPORT,
	"alternate-recipient",	HDR_ALT_RECIP,
	"disclose-recipients",	HDR_DISCLOSE_RECIPS,
	"content-return",	HDR_CONT_RETURN,
	0,			0
	};


/* -- statics -- */
static	FILE			*hdr_fp;
static	int			from_count,
				trace_count,
				date_count,
				hdr_prefix,
				redistributed_mail,
				sender_count,
				x400_receiveds;
static Trace			*date_trace;

/* -- externals -- */
extern	Q_struct		Qstruct;
extern CHAN *ch_inbound;
extern char *loc_dom_mta;




/* ---------------------  Begin	 Routines  -------------------------------- */




void rd_rfchdr (file)  /* -- basic processing of incoming header lines -- */
char			*file;
{
	char	*bp,
		*name = NULLCP,	   /* hdr content location */
		*contents = NULLCP;
	int	retval = NOTOK,
		type;


	PP_DBG (("submit/rd_rfchdr (%s)", file));

	sender_count		= 0;
	from_count		= 0;
	trace_count		= 0;
	date_count		= 0;
	redistributed_mail	= FALSE;
	ua_id_set_by_x400 	= FALSE;
	got_x400_mts_id		= FALSE;
	x400_receiveds		= 0;
	date_trace		= (Trace *) NULL;
	cor_to[0] = '\0';
	cor_to_len = sizeof cor_to;
	cor_mid = NULLCP;
	cor_date = NULLCP;
	cor_subject = NULLCP;

	if ((hdr_fp = fopen (file, "r")) == NULL)
		err_abrt (RP_FIO,
			"Unable to open '%s'", file);


	while (getline (&bp, hdr_fp) == OK) {
		hdr_prefix = FALSE;

		switch (retval = hdr_parse (bp, &name, &contents)) {
		    case HDV_MORE:
			continue;
		    case HDV_NEW:
			if ((type = hdr_type (name)) > 0)
				hdr_check (type, contents);
			continue;
		    case HDV_EOH:
			break;
		    case NOTOK:
			err_abrt(RP_USER, "Unable to parse '%s' as key:field", bp);
			break;
		}
		break;
	}

	(void) fclose (hdr_fp);

	do_correlators();

	if (retval != HDV_EOH)
		PP_DBG (("submit/rd_rfchdr/retval (%d) != hdr_eoh", retval));

	if (ch_inbound -> ch_access == CH_MTS &&
	    sender_count == 0 && from_count == 0)
		err_abrt (RP_USER, "No sender given");
	if (ch_inbound -> ch_strict_mode == CH_STRICT_CHECK) {
		if (date_count == 0)
			err_abrt (RP_USER, "No date field given");
		else if (date_count != 1)
			err_abrt (RP_USER, "Too many date fields (%d)",
				  date_count);
	}
}

/* ---------------------  Static  Routines  ------------------------------- */

static int hdr_parse (txt, name, contents)    /* -- parse one header line -- */
register char		*txt;		  /* -- a line of header text -- */
char			**name;		  /* -- location of field's name -- */
char			**contents;    /* -- location of field's contents -- */
{
	char		linetype;


	PP_DBG (("submit/hdr_parse (%s)", txt));


	if (isspace (*txt)) {
		/* -- continuation text -- */
		if (*txt == '\n' || *txt == '\0')
			return (HDV_EOH);
		linetype = HDV_MORE;
	}
	else  {
		linetype = HDV_NEW;

		*name = txt;
		while (*txt && *txt != ':')
			txt ++;
		if (*txt == '\0' || *txt == '\n')
			return NOTOK;
		*txt ++ = '\0';

		(void) compress (*name, *name);
	}


	*contents = txt;
	(void) compress (*contents, *contents);

	return (linetype);
}

static int hdr_type (name)    /* -- return the type of the component  -- */
char	*name;
{
	PP_DBG (("submit/hdr_type (%s)",name));

	if (prefix ("Resent-", name)) {
		name += 7;
		goto doremail;
	}
	if (prefix ("Remailed-", name)) {
		name += 9;
		goto doremail;
	}
	if (prefix ("Redistributed-", name)) {
		name += 14;
doremail:
		hdr_prefix = TRUE;
		if (!redistributed_mail) {
			sender_count = from_count = 0;
			redistributed_mail = TRUE;
		}
	}

	return (cmd_srch (name, htbl_rfc));
}

static void hdr_check (type, body)
char	*body;
int	type;
{
	AP_ptr	ap;
	int hlen;

	PP_DBG (("submit/hdr_check (%d,%s)", type, body));

	if (type <= 0)
		return;		/* -- for all unknown types -- */

	switch (type) {
	    case HDR_SENDER:
		if (redistributed_mail && !hdr_prefix)
			break;
		if ((ap = ap_s2t (body)) == NULLAP)
			err_abrt (RP_USER, "Syntactically invalid address for sender '%s'", body);
		else
			ap_free(ap);
		sender_count++;
		break;
	    case HDR_TO:
		if (cor_to[0] != '\0' && cor_to_len > 4) {
			(void) strcat (cor_to, ", ");
			cor_to_len -= 2;
		}
		if ((hlen = strlen (body)) > cor_to_len)
			cor_to_len = 0;
		else {
			(void) strncat (cor_to, body, cor_to_len);
			cor_to_len -= hlen;
		}
		break;
	    case HDR_FROM:
		if (redistributed_mail && !hdr_prefix)
			break;
		if ((ap = ap_s2t (body)) == NULLAP)
			err_abrt (RP_USER, "Syntactically invalid address for from field '%s'", body);
		else
			ap_free(ap);
		from_count++;
		break;
	    case HDR_REPLYTO:
		if (redistributed_mail && !hdr_prefix)
			break;
		break;
	    case HDR_VIA:
		hdr_via (body);
		break;
	    case HDR_RECEIVED:
		hdr_rcv (body);
		break;
	    case HDR_MESSAGEID:
		hdr_msgid (body);
		break;
	    case HDR_DATE:
		if (hdr_prefix == FALSE) {
			hdr_date (body);
			date_count++;
		}
		break;
	    case HDR_SUBJECT:
		hdr_subject (body);
		break;
	    case HDR_X400_MTS_ID:
		hdr_x400_mts_id (body);
		break;
	    case HDR_ORIG_EIT:
		hdr_orig_eit (body);
		break;
	    case HDR_CONTENT_IDENTIFIER:
		hdr_x400_content_id (body);
		break;
	    case HDR_PRIORITY:
		hdr_priority(body);
		break;
	    case HDR_DL_HISTORY:
		hdr_dl_history(body);
		break;
	    case HDR_CONVERSION:
		hdr_conversion(body);
		break;
	    case HDR_CONVERT_WITH_LOSS:
		hdr_convert_with_loss(body);
		break;
	    case HDR_X400_RECEIVED:
		hdr_x400_received (body);
		break;
	    case HDR_DEF_DELIV:
		hdr_def_deliv (body);
		break;
	    case HDR_LAST_DELIV:
		hdr_latest_time (body);
		break;
	    case HDR_GEN_DEL_REPORT:
		hdr_gen_del_report ();
		break;
	    case HDR_PREV_NON_DEL_REPORT:
		hdr_prev_non_del_report ();
		break;
	    case HDR_ALT_RECIP:
		hdr_prohibition(body, &(Qstruct.alternate_recip_allowed));
		break;
	    case HDR_DISCLOSE_RECIPS:
		hdr_prohibition(body, &(Qstruct.disclose_recips));
		break;
	    case HDR_CONT_RETURN:
		hdr_prohibition(body, &(Qstruct.content_return_request));
		break;
	    default:
		break;
	}			/* -- switch  -- */
}

static void hdr_subject (str)
char	*str;
{
	char	buf[BUFSIZ];

	if (cor_subject == NULLCP)
		cor_subject = strdup(str);

	if (ua_id_set_by_x400 == TRUE)
		return;
	if (Qstruct.ua_id != NULLCP)
		return;
	bzero (buf, sizeof buf);
	if ((int)strlen(str) >= ub_content_id_length) {
		(void) strncat (buf, str, (ub_content_id_length-strlen("...")));
		(void) strcat (buf, "...");
	} else
		(void) strcat (buf, str);
	Qstruct.ua_id = strdup (buf);
}

static void hdr_x400_content_id (str)
char	*str;
{
	char	buf[BUFSIZ];

	if (Qstruct.ua_id != NULLCP)
		free(Qstruct.ua_id);
	bzero (buf, sizeof buf);
	if ((int)strlen(str) >= ub_content_id_length) {
		(void) strncat (buf, str, (ub_content_id_length-strlen("...")));
		(void) strcat (buf, "...");
	} else
		(void) strncat (buf, str, strlen(str));
	Qstruct.ua_id = strdup (buf);
	ua_id_set_by_x400 = TRUE;
}

static void hdr_priority (str)
char	*str;
{
	(void) compress(str, str);
	if (lexequ(str, "normal") == 0)
		Qstruct.priority = PRIO_NORMAL;
	else if (lexequ(str, "urgent") == 0)
		Qstruct.priority = PRIO_URGENT;
	else if (lexequ(str, "non-urgent") == 0
		|| lexequ(str, "nonurgent") == 0)
		Qstruct.priority = PRIO_NONURGENT;
}

static void hdr_dl_history (str)
char	*str;
{
	register DLHistory	*dlp;
	char *cp;
	int n;
	OR_ptr or;
	UTC utc;
	char buf[BUFSIZ];

	if ((cp = index(str, ';')) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("Illegal DLHistory field %s", str));
		return;
	}
	if ((n = cp - str) >= BUFSIZ)
		n = BUFSIZ - 1;
	(void) strncpy (buf, str, n);
	buf[n] = 0;
	if (or_rfc2or_aux (buf, &or, 1) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Illegal rfc822 address in DLHistory %s", buf));
		return;
	}
	or_or2std (or, buf, FALSE);
	or_free (or);
	cp ++;
	if (index (cp, ';') == NULL)
		PP_LOG (LLOG_EXCEPTIONS,
			("DLhistory Missing trailing ';' in %s", str));
	if (rfc2UTC (cp, &utc) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("DLhistory: bad time in %s", str));
		return;
	}
	dlp = (DLHistory *)smalloc (sizeof *dlp);
	dlp -> dlh_addr = strdup (buf);
	dlp -> dlh_time = utc;
	dlp -> dlh_dn = NULLCP;
	dlp -> dlh_next = Qstruct.dl_expansion_history;
	Qstruct.dl_expansion_history = dlp;
}	

static void hdr_conversion (str)
char	*str;
{	
	(void) compress (str, str);
	if (lexequ (str, "prohibited") == 0)
		Qstruct.implicit_conversion_prohibited = 1;
}

static void hdr_convert_with_loss (str)
char	*str;
{	
	(void) compress (str, str);
	if (lexequ (str, "prohibited") == 0)
		Qstruct.conversion_with_loss_prohibited = 1;
}

static void hdr_x400_received (str)
char	*str;
{
	Trace	*trace;

	PP_DBG (("submit/hdr_x400_received (%s)", str));

	trace_count++;

	if (ch_inbound -> ch_access == CH_MTS)
		err_abrt (RP_MECH,
			  "No trace information allowed for local submission");

	x400_receiveds++;

	if (date_trace != (Trace *) NULL) {
		/* remove date generated trace */
		/* should never need as trace should be above */
		/* date fields */
		if (Qstruct.trace == date_trace)
			Qstruct.trace = Qstruct.trace->trace_next;
		else {
			for (trace = Qstruct.trace;
			     trace != (Trace *) NULL
			     && trace->trace_next != date_trace;
			     trace = trace->trace_next)
				continue;
			if (trace->trace_next == date_trace)
				trace->trace_next = date_trace->trace_next;
		}
		date_trace->trace_next = (Trace *) NULL;
		trace_free(date_trace);
		date_trace = (Trace *) NULL;
	}
			    
	trace = (Trace *) smalloc (sizeof(Trace));
	bzero ((char *) trace, sizeof(*trace));

	if (rfc2x400trace (trace, str) == OK) {
		PP_DBG (("rfc2x400trace OK"));
		trace -> trace_next = Qstruct.trace;
		Qstruct.trace = trace;
	} else
	       free((char *) trace);
}

static void hdr_def_deliv (str)
char	*str;
{
	if (Qstruct.defertime != 0)
		/* don't override */
		return;
	if (rfc2UTC (str, &Qstruct.defertime) != OK) 
		/* ignore */
		Qstruct.defertime = 0;
}

static void hdr_latest_time (str)
char	*str;
{
	if (Qstruct.latest_time != 0)
		/* don't override */
		return;
	if (rfc2UTC (str, &Qstruct.latest_time) != OK)
		Qstruct.latest_time = 0;
	else
		/* assume if present then critical */
		Qstruct.latest_time_crit = Q_LATESTTIME;
}

static void hdr_date (str)
char	*str;
{
	Trace	*tp;
	char	*adr = NULL;
	OR_ptr	or;

	if (x400_receiveds > 0)
		/* have x400-received lines */
		/* so don't need to add date generated trace */
		return;

	if (cor_date != NULLCP)
		cor_date = strdup(str);

	tp = (Trace *) smalloc (sizeof (Trace));
	bzero ((char *)tp, sizeof(*tp));
	if (rfc2UTC (str, &(tp -> trace_DomSinfo.dsi_time)) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("submit/hdr_date unable to convert time to UTC '%s'",
			str));
		free((char *) tp);
		return;
	}
	tp->trace_DomSinfo.dsi_action = ACTION_RELAYED;
	
	if (Qstruct.Oaddress->ad_r400adr != NULL) {
		adr = strdup(Qstruct.Oaddress->ad_r400adr);
		if ((or = or_std2or(adr)) == NULLOR) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("submit/hdr_date unable to get OR tree for orig '%s'", Qstruct.Oaddress->ad_r400adr));
			free((char *)adr);
		}
	} 
	if (or == NULL && Qstruct.Oaddress->ad_r822adr != NULL) {
		adr = strdup(Qstruct.Oaddress->ad_r822adr);
		if (or_rfc2or_aux (adr, &or, 1) == NOTOK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("submit/hdr_date unable to get OR tree for orig '%s'", Qstruct.Oaddress->ad_r822adr));
			free ((char *) adr);
		}
	}

	if (or == NULL
	    || 	fillin_DomId (&tp -> trace_DomId, or, adr) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("submit/hdr_date unable to create gld for orig"));
		free ((char *) tp);
		return;
	}
	
	or_free(or);
	if (Qstruct.Oaddress->ad_r822adr != NULLCP) {
		AP_ptr	tree, group,
			name, local,
			domain, route;
		ap_s2p(Qstruct.Oaddress->ad_r822adr,
		       &tree, &group, &name,
		       &local, &domain, &route);
		rfc822_norm_dmn (domain, ch_inbound->ch_ad_order);
		tp -> trace_mta = strdup(domain -> ap_obvalue);
		ap_sqdelete (tree, NULLAP);
		ap_free(tree);
	}
	free(adr);
	if (tp -> trace_DomSinfo.dsi_time == NULL)
		tp -> trace_DomSinfo.dsi_time = utcnow ();
	tp -> trace_next = Qstruct.trace;
	date_trace = Qstruct.trace = tp;
}

/* ARGSUSED */
static void hdr_x400_mts_id (str)
char	*str;
{
	if (Qstruct.msgid.mpduid_string != NULL)
		MPDUid_free(&Qstruct.msgid);
	/* force random generation to avoid clashes */
	MPDUid_new(&Qstruct.msgid);
	got_x400_mts_id = TRUE;
#ifdef KEEPMTSID
	rfc2msgid(&Qstruct.msgid, str);
#endif
}

static void hdr_orig_eit (str)
char	*str;
{
	if (Qstruct.orig_encodedinfo.eit_types != NULL)
		/* override */
		encodedinfo_free (&Qstruct.orig_encodedinfo);
	(void) rfc2encinfo (&Qstruct.orig_encodedinfo, str);
}

static void hdr_msgid (str)
char	*str;
{
	MPDUid	*mp;
	char	*midstring, *cp;
	AP_ptr	ap, local, domain;
	OR_ptr	or;

	if (cor_mid == NULLCP)
		cor_mid = strdup(str);

	if (got_x400_mts_id == TRUE)
		return;

	if (Qstruct.msgid.mpduid_string == NULL)
		MPDUid_new  (&Qstruct.msgid);

	mp = &Qstruct.msgid;
	if (mp -> mpduid_string != NULLCP)
		free (mp -> mpduid_string);
	(void) compress (str, str);
	if ((int) strlen(str) > UB_LOCAL_ID_LENGTH) {
		char tbuf[UB_LOCAL_ID_LENGTH+1];
		(void) strncpy (tbuf, str, UB_LOCAL_ID_LENGTH);
		tbuf[UB_LOCAL_ID_LENGTH] = 0;
		mp -> mpduid_string = strdup (tbuf);
	}
	else mp -> mpduid_string = strdup (str);

	if ((midstring = index (str, '<')) == NULLCP
	    || (cp = rindex(str, '>')) == NULLCP) 
		return;
	*cp ='\0';
	midstring++;

	if (ap_s2p(midstring, &ap, NULLAPP, NULLAPP, &local,
		   &domain, NULLAPP) == (char *) NOTOK)
		return;
	if (domain != NULLAP) {
		rfc822_norm_dmn (domain, 
				 Qstruct.inbound->li_chan->ch_ad_order);
		if (or_domain2or(domain->ap_obvalue, &or) != NOTOK) {
			OR_ptr	c, admd, prmd;
			
			or_chk_admd(&or);
			if ((c = or_find(or, OR_C, NULLCP)) != NULLOR
			    && (admd = or_find(or, OR_ADMD, NULLCP)) != NULLOR
			    && (prmd = or_find(or, OR_PRMD, NULLCP)) != NULLOR) {
				/* swap generated with found */
				if (mp->mpduid_DomId.global_Country)
					free(mp->mpduid_DomId.global_Country);
				mp->mpduid_DomId.global_Country = strdup(c->or_value);
				if (mp->mpduid_DomId.global_Admin)
					free(mp->mpduid_DomId.global_Admin);
				mp->mpduid_DomId.global_Admin = strdup(admd->or_value);
				if (mp->mpduid_DomId.global_Private)
					free(mp->mpduid_DomId.global_Private);
				mp->mpduid_DomId.global_Private = strdup(prmd->or_value);
			}
			if (or)
				or_free(or);
		}
	}
	
	ap_sqdelete(ap, NULLAP);
}	


/* -- take comments from 'from' & add to 'to' leaving 'from' without any -- */

static void hdr_remcomm (from, to)
char		*from;
char		*to;
{
	char	*start = from,
		*r = to,
		*s,
		*p,
		*q;


	if (start) {

		/* -- in case of NULL start ie missing date -- */

		PP_DBG (("submit/hdr_remcomm (%s,%s)", from, to));

		/* -- get to end -- */
		while (*r != '\0')
			r++;

		for (p = start ; (p = index (p, '(')) != NULLCP;) {
			if ((q = index (p, ')')) == NULLCP) {
				*p = '\0';
				break;
			}

			if (*to != '\0')
				*r++ = ' ';

			for (s = p; s <= q; *r++ = *s++) continue;

			(void) strcpy (p, q+1);

		}    /* -- for -- */

		*r = '\0';
		(void) compress (from, from);
	}  /* -- if -- */
	else
		PP_DBG (("submit/hdr_remcomm (NULL, %s)",to));
}

static void hdr_via (contents)
char	*contents;
{
	register Trace		*tp;
	char	*datestart = NULLCP,
		*argv[25], *domain, extraspace[LINESIZE];
	OR_ptr	or;

	PP_DBG (("submit/hdr_via (%s)", contents));

	trace_count++;
	
	if (trace_count == 1)
		return;/* we've added the first trace! */
	
	if (ch_inbound -> ch_access == CH_MTS)
		err_abrt (RP_MECH,
			  "No trace information allowed for local submission");

	/* -- rip out the comments -- */

	*extraspace = '\0';
	hdr_remcomm (contents, extraspace);

	if ((datestart = index (contents, ';')) != NULLCP)
		*datestart++ = '\0';

	if (sstr2arg (contents, 25, argv, ",\t ") < 1 
	    || argv[0] == NULLCP)
		return;

	domain = strdup(argv[0]);
	or = NULLOR;
	if (or_domain2or (domain, &or) == NOTOK) {
		PP_TRACE(("unable to parse domain '%s' to OR tree", argv[0]));
		return;
	}
	or_chk_admd (&or);
	tp = (Trace *) smalloc (sizeof (Trace));
	bzero ((char *)tp, sizeof(*tp));
	
	if (fillin_DomId (&tp -> trace_DomId, or, argv[0]) == NOTOK){
		free ((char *) tp);
		return;
	}

	or_free(or);
	free(domain);
	tp -> trace_mta = strdup(argv[0]);

	if (datestart == NULLCP || 
	    rfc2UTC(datestart,&(tp->trace_DomSinfo.dsi_time)) == NOTOK)
		tp -> trace_DomSinfo.dsi_time = utcnow();

	tp->trace_DomSinfo.dsi_action = ACTION_RELAYED;
	tp -> trace_next = Qstruct.trace;
	Qstruct.trace = tp;
}


	
	
static void hdr_rcv (contents)
char	*contents;
{
	register Trace		*tp;
	char	*bystart = NULLCP, *datestart = NULLCP,
		*argv[25], *domain, extraspace[LINESIZE];
	int	argc, i;
	OR_ptr	or;

	PP_DBG (("submit/hdr_rcv (%s)", contents));

	trace_count++;
	
	if (trace_count == 1)
		return;/* we've added the first trace! */
	
	if (ch_inbound -> ch_access == CH_MTS)
		err_abrt (RP_MECH,
			  "No trace information allowed for local submission");
	/* -- rip out the comments -- */

	*extraspace = '\0';
	hdr_remcomm (contents, extraspace);

	if ((datestart = index (contents, ';')) != NULLCP)
		*datestart++ = '\0';
	
	argc = sstr2arg (contents, 25, argv, ",\t ");
	
	for (i = 0; i < argc; i++) {
		if (lexequ (argv [i], "by") == 0) {
			if (bystart == NULLCP && i < argc -1) {
				bystart = argv [++i];
				continue;
			}
		}
	}
	
	if (bystart == NULLCP)
		return;
	domain = strdup(bystart);
	or = NULLOR;
	if (or_domain2or (domain, &or) == NOTOK) {
		PP_TRACE(("unable to parse domain '%s' to OR tree", bystart));
		return;
	}
	or_chk_admd (&or);

	tp = (Trace *) smalloc (sizeof (Trace));
	bzero ((char *)tp, sizeof(*tp));
	
	if (fillin_DomId (&tp -> trace_DomId, or, bystart) == NOTOK){
		free ((char *) tp);
		return;
	}

	or_free(or);
	free(domain);
	tp -> trace_mta = strdup(bystart);

	if (datestart == NULLCP || 
	    rfc2UTC(datestart,&(tp->trace_DomSinfo.dsi_time)) == NOTOK)
		tp -> trace_DomSinfo.dsi_time = utcnow();

	tp->trace_DomSinfo.dsi_action = ACTION_RELAYED;
	tp -> trace_next = Qstruct.trace;
	Qstruct.trace = tp;
}
		


static int getline (bp, fp)
char	**bp;
FILE	*fp;
{
	static char *buf;
	static int bufsiz = 0;
	int	count;
	char	*cp;
	int	c;

	if (buf == NULLCP)
		buf = smalloc (bufsiz = BUFSIZ);
	for (cp = buf, count = 0; ; count ++) {
		if (count >= bufsiz - 5) {
			int curlen = cp - buf;
			buf = realloc (buf, (unsigned) (bufsiz += BUFSIZ));
			if (buf == NULL)
				err_abrt (RP_LIO, "Out of memeory");
			cp = buf + curlen;
		}
		switch (c = getc (fp)) {
		    case '\n':
			*cp ++ = ' ';
			if ((c = getc(fp)) == ' ' || c == '\t')
				continue;
			ungetc (c, fp);
			break;
		    case EOF:
			if (cp == buf)
				return NOTOK;
			break;
		    default:
			*cp ++ = c;
			continue;
		}
		break;
	}
	*cp = '\0';
	*bp = buf;
	return OK;
}

static int fillin_DomId (domId, or, str)
GlobalDomId	*domId;
OR_ptr		or;
char		*str;
{
	OR_ptr	tmp_or;

	if ((tmp_or = or_find (or, OR_C, NULLCP)) == NULLOR) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("submit/fillin_DomId: no country in '%s'", str));
		return NOTOK;
	}
	domId->global_Country = strdup(tmp_or -> or_value);
	
	if ((tmp_or = or_find (or, OR_ADMD, NULLCP)) == NULLOR) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("submit/fillin_DomId: no ADMD in '%s'", str));
		free ((char *) domId -> global_Country);
		return NOTOK;
	}
	domId->global_Admin = strdup(tmp_or -> or_value);
	
	if ((tmp_or = or_find (or, OR_PRMD, NULLCP)) != NULLOR) 
		domId->global_Private = strdup(tmp_or -> or_value);
	return OK;
}

static int add_str (buf, s1, s2, len)
char	buf[];
char	*s1, *s2;
int len;
{
	int l1, l2;
	if (len <= 0)
		return len;
	l1 = strlen(s1);
	l2 = strlen(s2);
	if (l1 + 2>= len) return len;
	if (buf[0]) {
		(void) strcat(buf, ",\n");
		len -= 2;
	}
	(void) strcat (buf, s1);
	len -= l1;
	if (len > l2) {
		(void) strcat (buf, s2);
		len -= l2;
	}
	else {
		(void) strncat (buf, s2, len - 1);
		len = 0;
	}
	return len;
}

static void do_correlators()
{
	char	buf[2*BUFSIZ], realbuf[BUFSIZ+1];
	PS	ps;
	PE	pe;
	int	len;
	char	*cp;

	buf[0] = '\0';
	len = 512;
	buf[len] = 0;
	if (NULLCP != cor_subject) {
		len = add_str (buf, "Subject: ", cor_subject, len);
		free(cor_subject);
		cor_subject = NULLCP;
	}

	if (NULLCP != cor_mid) {
		len = add_str (buf, "Message-ID: ", cor_mid, len);
		free(cor_mid);
		cor_mid = NULLCP;
	}

	if (NULLCP != cor_date) {
		len = add_str (buf, "Date: ", cor_date, len);
		free(cor_date);
		cor_date = NULLCP;
	}

	if ('\0' != cor_to) {
		len = add_str (buf, "To: ", cor_to, len);
		cor_to[0] = '\0';
	}

	realbuf[0] = '\0';
	if ((int) strlen(buf) >= ub_content_correlator_length) {
		(void) strncat (realbuf, buf,
				(ub_content_correlator_length-strlen("...")));
		(void) strcat (realbuf, "...");
	} else
		(void) strcat (realbuf, buf);
	
	Qstruct.pp_content_correlator = strdup(realbuf);

	pe = oct2prim(realbuf, strlen(realbuf));

	if ((ps = ps_alloc (str_open)) == NULLPS) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't allocate PS"));
		return;
	}

	len = ps_get_abs (pe);
	cp = smalloc (len);

	if (str_setup (ps, cp, len, 1) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't setup stream [%s]",
					  ps_error (ps -> ps_errno)));
		return;
	}

	if (pe2ps (ps, pe) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("pe2ps failed [%s]", ps_error (ps -> ps_errno)));
		return;
	}
	
	ps_free (ps);

	if ((Qstruct.general_content_correlator = str2qb (cp, len, 1)) == NULL)
		return;
	
	free(cp);
	pe_free(pe);
}

static void hdr_prohibition(str, pchar)
char	*str, *pchar;
{
	compress(str, str);
	if (lexequ(str, "allowed") == 0)
		*pchar = TRUE;
	else if (lexequ(str, "prohibited") == 0)
		*pchar = FALSE;
	else
		PP_NOTICE(("Unknown string encoding for prohibition: '%s'",
			   str));
}

static void hdr_gen_del_report ()
{
	ADDR	*ix;

	for (ix = Qstruct.Raddress;
	     ix != NULLADDR;
	     ix = ix -> ad_next) {
		if (ix -> ad_resp == TRUE) 
			/* set for +ve drs */
			ix -> ad_usrreq = AD_USR_CONFIRM;
	}
}

static void hdr_prev_non_del_report ()
{
	ADDR	*ix;

	for (ix = Qstruct.Raddress;
	     ix != NULLADDR;
	     ix = ix -> ad_next) {
		if (ix -> ad_resp == TRUE)
			/* no drs at all */
			ix -> ad_usrreq = AD_USR_NOREPORT;
	}
}
