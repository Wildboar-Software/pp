/* P2toRFC.c: p2 msg + optional p1 struct + optional bodypart -> RFC 822 hdr */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/P2toRFC.c,v 6.0 1991/12/18 20:20:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/P2toRFC.c,v 6.0 1991/12/18 20:20:34 jpo Rel $
 *
 * $Log: P2toRFC.c,v $
 * Revision 6.0  1991/12/18  20:20:34  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "util.h"
#include "IOB-types.h"
#include "Ext-types.h"
#include "q.h"
#include <isode/cmd_srch.h>
#include "tb_bpt88.h"
#include "or.h"
#include "ap.h"
#include "oids.h"
#include "charset.h"

static int load_time(), load_addr();
static FILE *fp_out;
static struct type_IOB_Heading *head;
static struct type_IOB_IPN *ipn;
static char	ipn_body[MAXPATHLENGTH];
static char abuf [BUFSIZ];
static int decode_extension();
static int decode_ext_char (), decode_ext_rdm(),
	decode_ext_pdm(), decode_ext_int(), decode_ext_rnfa(),
	decode_ext_pra(), decode_ext_redir(), ext_decode_dlexph();
static Q_struct	del_info_q;
static ADDR del_info_recip;

extern CMD_TABLE bptbl_body_parts88[/* x400 88 body_parts */];
extern CMD_TABLE atbl_rdm[], atbl_pd_modes[], atbl_reg_mail[];
extern char	*cont_p2, *cont_p22;
extern char	*hdr_822_bp, *hdr_p2_bp, *ia5_bp, *hdr_p22_bp, *hdr_ipn_bp;
extern char *dn2str ();
extern int	msgid2rfc(), encinfo2rfc();
extern int	order_pref;
extern UTC	utclocalise();
int		convertresult;
extern char	*dn2ufn(), *oid2lstr();
static int	firstLine;

#define bit_ison(x,y)        (bit_test(x,y) == 1)

P2toRFC (p2_in, ext_in, qp, del_info, rfc_out, ipn_body_out, ep)
char		*p2_in;
char		*ext_in;
Q_struct   	*qp;
char		*del_info;
char		*rfc_out;
char		*ipn_body_out;
char		**ep;
{
	FILE	*fp_in;
	PE	pe = NULLPE;
	PS	ps = NULLPS;
	int	retval, ishead;
	char	*ix;
	char	buf[BUFSIZ];

	head = NULL;
	ipn = NULL;
	firstLine = TRUE;
	convertresult = OK;

	if ((ix = rindex(p2_in, '/')) == NULL)
		ix = p2_in;
	else
		ix++;

	if (lexnequ(ix, hdr_p2_bp, strlen(hdr_p2_bp)) == 0
	    || lexnequ(ix, hdr_p22_bp, strlen(hdr_p22_bp)) == 0)
		ishead = OK;
	else if (lexnequ(ix, hdr_ipn_bp, strlen(hdr_ipn_bp)) == 0)
		ishead = NOTOK;
	else {
		(void) sprintf(buf,
			       "Unknown x400 hdr type '%s'",
			       ix);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", buf));
		*ep = strdup(buf);
		return (NOTOK);
	}
	
	if (rfc_out == NULLCP)
		fp_out = stdout;
	else if ((fp_out = fopen (rfc_out, "w")) == NULL)
	{
		PP_SLOG (LLOG_EXCEPTIONS, rfc_out,
			 ("Can't open file"));
		(void) sprintf (buf,
				"Failed to open output file '%s'", 
				rfc_out);
		*ep = strdup(buf);
		return (NOTOK);
	}

	if (ipn_body_out)
		strcpy(ipn_body, ipn_body_out);
	else
		ipn_body[0] = '\0';

	if (p2_in == NULLCP)
		fp_in = stdin;
	else if ((fp_in = fopen (p2_in, "r")) == NULL)
	{
		PP_SLOG (LLOG_EXCEPTIONS, p2_in,
			 ("Can't open file"));
		fclose (fp_out);
		(void) sprintf (buf,
				"Failed to open input file '%s'",
				p2_in);
		*ep = strdup(buf);
		return (NOTOK);
	}

	if (((ps = ps_alloc (std_open)) == NULLPS) ||
	    (std_setup (ps, fp_in) == NOTOK))
	{
		PP_LOG (LLOG_EXCEPTIONS, ("p2to822() failed to setup PS"));
		retval = NOTOK;
		*ep = strdup("Failed to set up PS");
		goto cleanup;
	}


	if ((pe = ps2pe (ps)) == NULLPE)
	{
		PP_LOG (LLOG_EXCEPTIONS, ("ps2pe error on file '%s'", p2_in));
		retval = NOTOK;
		goto cleanup;
	}

	PY_pepy[0] = 0;
	if (ishead == OK) {
		if (decode_IOB_Heading (pe, 0, NULLIP, NULLVP, &head) != OK)
		{
			PP_OPER(NULLCP,
				("decode_IOB_heading() failed : [%s]", PY_pepy));
			retval = NOTOK;
			convertresult = NOTOK;
			abort();
			*ep = strdup("Illegal ASN.1 in IOB Heading");
			goto cleanup;
		}
       
		if (PY_pepy[0] != 0)
			PP_LOG(LLOG_EXCEPTIONS,
			       ("decode_IOB_heading non-fatal failure [%s]", PY_pepy));


		if (qp != (Q_struct *) NULL
		    && do_p1 (qp, ishead, ep) != OK)
		{
			PP_TRACE (("P1 output failure"));
			retval = NOTOK;
			goto cleanup;
		}

		if (NULLCP != del_info
		    && do_delinfo (del_info, ep) != OK)
		{
			PP_TRACE (("Delivery Info output failure"));
			retval = NOTOK;
			goto cleanup;
		}

		if (do_p2_head (ep) != OK)
		{
			PP_TRACE (( "P2 output failure"));
			retval = NOTOK;
			goto cleanup;
		}

	} else {
		if (decode_IOB_IPN (pe, 0, NULLIP, NULLVP, &ipn) != OK)
		{
			PP_OPER(NULLCP,
				("decode_IOB_IPN() failed : [%s]", PY_pepy));
			retval = NOTOK;
/*			convertresult = NOTOK;*/
			*ep = strdup("Illegal ASN.1 in IPN");
			goto cleanup;
		}
		
		if (PY_pepy[0] != 0)
			PP_LOG(LLOG_EXCEPTIONS,
			       ("decode_IOB_IPN non-fatal failure [%s]",
				PY_pepy));

		if ((Q_struct *) NULL != qp
		    && do_p1_ipn (qp, ishead, ep) != OK)
		{
			PP_TRACE (("P1 output failure"));
			retval = NOTOK;
			goto cleanup;
		}

		if (NULLCP != del_info
		    && do_delinfo (del_info, ep) != OK)
		{
			PP_TRACE(("Delivery Info output failure"));
			retval = NOTOK;
			goto cleanup;
		}

		if (do_p2_ipn(qp, ep) != OK)
		{
			PP_TRACE (("P2 output failure"));
			retval = NOTOK;
			goto cleanup;
		}

	}
	do_same_line ("\n");	/* Terminate header */

	if (do_extra (ext_in, ep) != OK)
	{
		PP_TRACE (( "Extra bits output failure"));
		retval = NOTOK;
		goto cleanup;
	}
	
	if (ishead != OK
	    && do_p2_ipn_body(qp, ep) != OK)
	{
		PP_TRACE((" IPN body output failure"));
		retval = NOTOK;
		goto cleanup;
	}

	retval = OK;
    cleanup:
	fclose (fp_out);
	fclose (fp_in);
	
	if (pe != NULLPE) pe_free(pe);
	if (ps != NULLPS) ps_free(ps);
	if (head != NULL) free_IOB_Heading (head);
	if (ipn != NULL) free_IOB_IPN (ipn);
	return (retval);
}

/*  */

/* ARGSUSED */
do_p2_head(ep)
char	**ep;
{
	int	haveTo = FALSE;

	if (head -> authorizing__users != NULL) {
		do_key ("From");
		do_ORD_seq (head -> authorizing__users);
	}

	if (head -> originator != NULL) {
		if (head -> authorizing__users == NULL)
			do_key	("From");
		else
			do_key	("Sender");
		
		do_ORD (head -> originator);
	}

	if (head -> this__IPM != NULL)
	{
		do_key ("Message-ID");
		if (do_mid (head -> this__IPM, TRUE) != OK) {
			*ep = strdup("Invalid message id");
			return NOTOK;
		}
	}

	if (head -> primary__recipients != NULL)
	{
		do_key ("To");
		do_recip_seq (head -> primary__recipients);
		haveTo = TRUE;
	}

	if (head -> copy__recipients != NULL)
	{
		do_key ("Cc");
		do_recip_seq (head -> copy__recipients);
		haveTo = TRUE;
	}
	
	if (head -> blind__copy__recipients != NULL)
	{
		do_key ("Bcc");
		do_recip_seq (head -> blind__copy__recipients);
		haveTo = TRUE;
	}

	if (haveTo != TRUE) {
		do_key ("To");
		do_token ("list:;");
	}

	if (head -> replied__to__IPM != NULL)
	{
		do_key ("In-Reply-To");
		if (do_mid (head -> replied__to__IPM, FALSE) != OK) {
			*ep = strdup("Invalid In-Reply-To message id");
			return NOTOK;
		}
	}
	
	if (head -> obsoleted__IPMs != NULL)
	{
		do_key ("Obsoletes");
		if (do_mid_seq (head -> obsoleted__IPMs, TRUE) != OK) {
			*ep = strdup("Invalid Obsoletes message id");
			return NOTOK;
		}
	}
	
	if (head -> related__IPMs != NULL)
	{
		do_key ("References");
		if (do_mid_seq (head -> related__IPMs, FALSE) != OK) {
			*ep = strdup("Invalid References message id");
			return NOTOK;
		}
	}

	if (head -> subject != NULL)
	{
		do_key ("Subject");
		do_subject (head -> subject);
	}

	if (head -> expiry__time != NULL)
	{
		do_key ("Expiry-Date");
		do_utc (head -> expiry__time);
	}

	if (head -> reply__time != NULL)
	{
		do_key ("Reply-By");
		do_utc (head -> reply__time);
	}

	if (head -> reply__recipients != NULL)
	{
		do_key ("Reply-To");
		do_ORD_seq (head -> reply__recipients);
	}

	if (head -> importance != NULL
	    && head -> importance -> parm != int_IOB_ImportanceField_normal)
	{
		do_key ("Importance");
		do_importance (head -> importance -> parm);
	}

	if (head -> sensitivity != NULL)
	{
		do_key ("Sensitivity");
		do_sensitivity (head -> sensitivity -> parm);
	}

	if (head -> auto__forwarded != NULL
	    && head -> auto__forwarded -> parm)
	{
		do_key ("Autoforwarded");
		do_boolean (head -> auto__forwarded -> parm);
	}

	if (head -> extensions != NULL)
		do_extensions (head -> extensions);

	return (OK);
}

/* ARGSUSED */
do_p2_ipn(qp, ep)
Q_struct *qp;
char	**ep;
{
	char	buf[BUFSIZ];
	extern char	*postmaster;
	ADDR	*ix;

	do_key ("From");
	if (ipn->ipn__originator)
		do_ORD (ipn->ipn__originator);
	else if ((Q_struct *) NULL != qp)
		do_p1_addrs (qp -> Oaddress);
	else {
		sprintf(buf, "%s", postmaster);
		do_token(buf);
	}
	
	
	do_key ("To");
	if ((Q_struct *) NULL != qp) {
		for (ix = qp->Raddress;
		     ix != NULLADDR && ix -> ad_resp == NO;
		     ix = ix -> ad_next)
			continue;
		if (ix == NULLADDR)
			ix = qp->Raddress;
	
		if (ix != NULLADDR) {
			if (ix -> ad_redirection_history != (Redirection *) NULL) {
				if (ix->ad_redirection_history->rd_addr != NULLCP) {
					OR_ptr	or = or_std2or(ix->ad_redirection_history->rd_addr);
					or_or2rfc(or, buf);
					or_free(or);
				} else
					sprintf(buf,"list:;");
				
				if (ix -> ad_redirection_history -> rd_dn != NULLCP)
					(void) sprintf(buf, "%s (DN=%s)",
						       buf,
						       ix -> ad_redirection_history -> rd_dn);
			} else
				(void) strcpy(buf, ix -> ad_r822adr);
			do_token(buf);
		} else
			do_token("list:;");		
	} else
		do_token("list:;");

	do_key ("Subject");
	do_token ("X.400 Inter-Personal Receipt Notification");

	do_key ("Message-Type");
	do_token ("InterPersonal Notification");

	if (ipn -> subject__ipm != NULL)
	{
		do_key ("References");
		if (do_mid (ipn -> subject__ipm, FALSE) != OK) {
			*ep = strdup("Invalid References message id");
			return NOTOK;
		}
	}

	return OK;
}

do_p2_ipn_body (qp, ep)
Q_struct	*qp;
char	**ep;
{
	FILE	*fp;
	char	buf[BUFSIZ];

	if (ipn_body[0] != '\0') {
		if ((fp = fopen (ipn_body, "w")) == NULL)
		{
			PP_SLOG (LLOG_EXCEPTIONS, ipn_body,
				 ("Can't open file"));
			(void) sprintf(buf,
				       "Unable to open output file '%s' for IPN body",
				       ipn_body);
			*ep = strdup(buf);
			return (NOTOK);
		}
	} else 
		fp = stdout;

	fputs("Your message to: ", fp);

	if (ipn -> ipn__preferred__recipient != NULL) {
		get_ORD (ipn -> ipn__preferred__recipient, abuf);
		fputs(abuf, fp);
		if (ipn -> ipn__originator != NULL) {
                       fputs(" forwarded to: ", fp);
                       get_ORD (ipn -> ipn__originator, abuf);
                       fputs(abuf, fp);
	       }
	} else if (ipn -> ipn__originator != NULL) {
               get_ORD (ipn -> ipn__originator, abuf);
               fputs(abuf, fp);
       } else if ((Q_struct *) NULL != qp
		  && adr2rfc (qp->Oaddress, abuf, order_pref) == OK)
	       fputs(abuf, fp);
	else
		fputs("an unknown recipient", fp);

	fputs("\n", fp);
	if (ipn -> choice != NULL) {
		if (ipn -> choice -> offset == choice_IOB_0_non__receipt__fields)
			do_p2_ipn_body_non__receipt(ipn -> choice -> un.non__receipt__fields, fp);
		else 
			do_p2_ipn_body_receipt (ipn -> choice -> un.receipt__fields, fp);
	}		
	if (ipn -> conversion__eits != NULL) 
		do_p2_ipn_body_eits (ipn -> conversion__eits, fp);
		
	
	if (ipn -> choice != NULL &&
	    ipn -> choice -> offset == choice_IOB_0_non__receipt__fields) {
		if (ipn -> choice -> un.non__receipt__fields -> returned__ipm != NULL)
			fputs("\nThe Original Message follows: \n\n", fp);
		else
			fputs("\nThe Original Message is not available\n", fp);
	} else
		fputs("\nThe Original Message is not returned with positive notifications\n", fp);

	fclose (fp);
	return (OK);
}	

do_p2_ipn_body_non__receipt (non_receipt, fp)
struct type_IOB_NonReceiptFields	*non_receipt;
FILE				*fp;
{
	if (non_receipt -> non__receipt__reason -> parm == int_IOB_NonReceiptReasonField_ipm__discarded)
		do_p2_ipn_discard (non_receipt, fp);
	else
		do_p2_ipn_auto_forward (non_receipt, fp);
}

do_p2_ipn_discard (non_receipt, fp)
struct type_IOB_NonReceiptFields	*non_receipt;
FILE				*fp;
{
	fputs("was discarded for the following reason: ", fp);
	switch (non_receipt -> discard__reason -> parm) {
	    case int_IOB_DiscardReasonField_ipm__expired:
		fputs ("Expired.\n", fp);
		break;
	    case int_IOB_DiscardReasonField_ipm__obsoleted:
		fputs ("Obsoleted.\n", fp);
		break;
	    case int_IOB_DiscardReasonField_user__subscription__terminated:
		fputs ("User Subscription Terminated.\n", fp);
		break;
	}
}

do_p2_ipn_auto_forward (non_receipt, fp)
struct type_IOB_NonReceiptFields	*non_receipt;
FILE				*fp;
{
	char	*cp;
	fputs ("was automatically forwarded.\n", fp);
	if (non_receipt -> auto__forward__comment != NULL) {
		cp = qb2str(non_receipt -> auto__forward__comment);
		fputs ("The following comment was made: ", fp);
		fputs (cp, fp);
		fputs (".\n", fp);
		free (cp);
	}
}
	
do_p2_ipn_body_receipt (receipt, fp)
struct type_IOB_ReceiptFields	*receipt;
FILE				*fp;
{
	do_p2_utc (receipt -> receipt__time, fp);
	fputs("\n", fp);
	if (receipt -> acknowledgment__mode != NULL)
		do_p2_ack_mode (receipt -> acknowledgment__mode, fp);
	if (receipt -> suppl__receipt__info != NULL)
		do_p2_suppl_info (receipt -> suppl__receipt__info, fp);
}

do_p2_ack_mode (mode, fp)
struct type_IOB_AcknowledgmentModeField	*mode;
FILE	*fp;
{
	fputs ("This notification was generated ", fp);
	if (mode -> parm == int_IOB_AcknowledgmentModeField_manual)
		fputs("Manually.\n", fp);
	else
		fputs ("Automatically.\n", fp);
}

do_p2_suppl_info (info, fp)
struct type_IOB_SupplReceiptInfoField	*info;
FILE 	*fp;
{
	char	*cp;
	fputs ("The following extra information was given:\n", fp);
	cp = qb2str (info);
	fputs(cp, fp);
	fputs ("\n", fp);
	free(cp);
}

do_p2_ipn_body_eits (eits, fp)
struct type_IOB_ConversionEITsField	*eits;
FILE	*fp;
{
	int	first = OK;

	fputs ("The following information types were converted:", fp);
	
	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_undefined) == 1) {
		if (first == OK) {
			fputs ("Undefined", fp);
			first = NOTOK;
		} else
			fputs (", Undefined", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_telex) == 1) {
		if (first == OK) {
			fputs ("Telex", fp);
			first = NOTOK;
		} else
			fputs (", Telex", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_ia5__text) == 1) {
		if (first == OK) {
			fputs ("IA5-Text", fp);
			first = NOTOK;
		} else
			fputs (", IA5-Text", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_g3__facsimile) == 1) {
		if (first == OK) {
			fputs ("G3-Fax", fp);
			first = NOTOK;
		} else
			fputs (", G3-Fax", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_g4__class__1) == 1) {
		if (first == OK) {
			fputs ("TIF0", fp);
			first = NOTOK;
		} else
			fputs (", TIF0", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_teletex) == 1) {
		if (first == OK) {
			fputs ("Teletex", fp);
			first = NOTOK;
		} else
			fputs (", Teletex", fp);
	}
	
	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_videotex) == 1) {
		if (first == OK) {
			fputs ("Videotex", fp);
			first = NOTOK;
		} else
			fputs (", Videotex", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_voice) == 1) {
		if (first == OK) {
			fputs ("Voice", fp);
			first = NOTOK;
		} else
			fputs (", Voice", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_sfd) == 1) {
		if (first == OK) {
			fputs ("SFD", fp);
			first = NOTOK;
		} else
			fputs (", SFD", fp);
	}

	if (bit_test (eits -> built__in__encoded__information__types,
		      bit_MTA_BuiltInEncodedInformationTypes_mixed__mode) == 1) {
		if (first == OK) {
			fputs ("TIF1", fp);
			first = NOTOK;
		} else
			fputs (", TIF1", fp);
	}

	if (eits -> external__encoded__information__types != NULL) {
		if (first == OK) {
			fputs (", ", fp);
			first = NOTOK;
		}
		fputs (oid2lstr (eits -> external__encoded__information__types), fp);
	}
	fputs(".\n", fp);
}

do_p2_utc (utc_qb, fp)
struct qbuf	*utc_qb;
FILE	*fp;
{
	char 	*cp;
	UTC	ut, lut;
	fputs ("was received at ", fp);
	cp = qb2str (utc_qb);
	if ((ut = str2utct (cp, strlen(cp))) == NULL)
		ut = str2gent (cp, strlen(cp));
	lut = utclocalise(ut);
	UTC2rfc (lut, abuf);
	fputs(abuf, fp);
	fputs(".\n", fp);
	free(cp);
	free ((char *)ut);
	free ((char *)lut);
}

/*  */

static int moreThanHdrAndIa5(eit)
EncodedIT	*eit;
{
	LIST_BPT	*ix = eit->eit_types;
	int		found = 0;
static char		*p2_txt = NULLCP;
	if (p2_txt == NULLCP)
		p2_txt = rcmd_srch(BPT_P2_DLIV_TXT, bptbl_body_parts88);
	while (found == 0 && ix != NULL) {
		if (strncmp(hdr_822_bp, ix->li_name, strlen(hdr_822_bp)) == 0
		    || strncmp(p2_txt, ix->li_name, strlen(p2_txt)) == 0
		    || strncmp(hdr_p2_bp, ix->li_name, strlen(hdr_p2_bp)) == 0
		    || strncmp(ia5_bp, ix->li_name, strlen(ia5_bp)) == 0)
			ix = ix->li_next;
		else
			/* not ia5 or hdr */
			found = 1;
	}
	return found;
}

/* ARGSUSED */
do_p1 (qp, ishead, ep)
Q_struct *qp;
int	 ishead;
char	**ep;
{
	if (qp == (Q_struct *) 0)
		return (OK);

	do_p1_trace (qp -> trace);
	
	do_key ("Date");

	do_p1_utc (qp -> trace->trace_DomSinfo.dsi_time);

	if (ishead == OK && head -> originator == NULL)
	{
		if (head -> authorizing__users == NULL)
			do_key("From");
		else
			do_key("Sender");

		do_p1_addrs (qp -> Oaddress);
	}

	do_key ("X400-Originator");
	do_p1_addrs (qp -> Oaddress);

	if (ishead == OK &&
	    head -> primary__recipients == NULL &&
	    head -> copy__recipients == NULL &&
	    head -> blind__copy__recipients == NULL) 
	{
		do_key ("To");
		if (qp -> disclose_recips
		    || qp -> Raddress -> ad_next == NULLADDR)
			do_p1_addrs (qp -> Raddress);
		else
			do_token ("non-disclosure:;");
	}

	do_key ("X400-Recipients");
	if (qp -> disclose_recips 
	    || qp -> Raddress -> ad_next == NULLADDR)
		do_p1_addrs (qp -> Raddress);
	else
		do_token ("non-disclosure:;");

	do_key ("X400-MTS-Identifier");
	do_p1_mid (&(qp -> msgid));

	if (qp -> orig_encodedinfo.eit_types != NULL
	    && moreThanHdrAndIa5 (&qp -> orig_encodedinfo)) {
		do_key ("Original-Encoded-Information-Types");
		do_p1_eit (&(qp -> orig_encodedinfo));
	} else if (qp -> encodedinfo.eit_types != NULL
	    && moreThanHdrAndIa5 (&qp -> encodedinfo)) {
		/* not just ia5 bodypart */
		do_key ("Original-Encoded-Information-Types");
		do_p1_eit (&(qp -> encodedinfo));
	}

	if (qp -> cont_type != NULLCP) {
		do_key ("X400-Content-Type");
		if (lexequ (qp -> cont_type, cont_p2) == 0)
			do_token ("P2-1984 (2)");
		else if (lexequ (qp -> cont_type, cont_p22) == 0)
			do_token ("P2-1988 (22)");
		else
			do_token (qp -> cont_type);
	}

	if (qp -> ua_id != NULLCP)
	{
		do_key ("Content-Identifier");
		do_token (qp -> ua_id);
	}

	if (qp -> priority != PRIO_NORMAL)
	{
		do_key ("Priority");
		do_p1_priority (qp -> priority);
	}
	
	if (qp -> originator_return_address != NULL)
	{
		do_key ("Originator-Return-Address");
		do_p1_fullname (qp -> originator_return_address);
	}
	
	if (qp -> dl_expansion_history != NULL)
		do_p1_DLHistory (qp -> dl_expansion_history);

	if (qp -> implicit_conversion_prohibited == TRUE) 
	{
		do_key("Conversion");
		do_token("Prohibited");
	}
		
	if (qp -> conversion_with_loss_prohibited)
	{
		do_key ("Conversion-With-Loss");
		do_token ("Prohibited");
	}
	
	
       if (qp -> defertime != 0)
       {
	       do_key ("Deferred-Delivery");
	       do_p1_utc (qp -> defertime);
       }
	
	if (qp -> latest_time != 0
	    && qp -> latest_time_crit)
	{
		do_key ("Latest-Delivery-Time");
		do_p1_utc (qp -> latest_time);
	}

	return (OK);
}

/* ARGSUSED */
do_p1_ipn (qp, ishead, ep)
Q_struct *qp;
int	 ishead;
char	**ep;
{
	if (qp == (Q_struct *) 0)
		return (OK);

	do_p1_trace (qp -> trace);
	
	do_key ("Date");

	do_p1_utc (qp -> trace->trace_DomSinfo.dsi_time);

	do_key ("X400-Originator");
	do_p1_addrs (qp -> Oaddress);

	do_key ("X400-MTS-Identifier");
	do_p1_mid (&(qp -> msgid));

	if (qp -> cont_type != NULLCP) {
		do_key ("X400-Content-Type");
		if (lexequ (qp -> cont_type, cont_p2) == 0)
			do_token ("P2-1984 (2)");
		else if (lexequ (qp -> cont_type, cont_p22) == 0)
			do_token ("P2-1988 (22)");
		else
			do_token (qp -> cont_type);
	}

	if (qp -> orig_encodedinfo.eit_types != NULL
	    && moreThanHdrAndIa5 (&qp -> orig_encodedinfo)) {
		do_key ("Original-Encoded-Information-Types");
		do_p1_eit (&(qp -> orig_encodedinfo));
	} else if (qp -> encodedinfo.eit_types != NULL
	    && moreThanHdrAndIa5 (&qp -> encodedinfo)) {
		/* not just ia5 bodypart */
		do_key ("Original-Encoded-Information-Types");
		do_p1_eit (&(qp -> encodedinfo));
	}

	if (qp -> priority != PRIO_NORMAL)
	{
		do_key ("Priority");
		do_p1_priority (qp -> priority);
	}

	if (qp -> ua_id != NULLCP)
	{
		do_key ("Content-Identifier");
		do_token (qp -> ua_id);
	}

	return (OK);
}

/*  */

do_extra (extra, ep)
char *extra;
char	**ep;
{
	char buf [BUFSIZ], tmp[BUFSIZ];
	FILE *fp_extra;

	if (extra == NULLCP || extra [0] == '\0')
		return (OK);

	if ((fp_extra = fopen (extra, "r")) == NULL)
	{
		PP_SLOG (LLOG_EXCEPTIONS, extra,
			 ("Can't open file"));
		(void) sprintf (tmp,
				"Unable to open extra input file '%s'", 
				extra); 
		*ep = strdup(tmp);
		return (NOTOK);
	}

	if (fgets (buf, BUFSIZ - 1, fp_extra) == NULL)
	{
		PP_DBG (( "Null file '%s'", extra));
		(void) sprintf (tmp,
				"Empty extra file '%s'", 
				extra);
		*ep = strdup(tmp);
		fclose (fp_extra);
		return (NOTOK);
	}

	buf [strlen(buf) - 1] = '\0';  /* remvove \n */
	if (lexnequ (buf, "RFC-822-HEADERS", 
		     strlen("RFC-822-HEADERS")) != 0
	    && lexnequ (buf, "Comments", 
			strlen("Comments")) != 0)
	{
		PP_DBG (( "Illegal first line '%s' in file '%s'",
				buf, extra));
		(void) sprintf (tmp,
				"Illegal first line '%s' in file '%s'",
				buf, extra);
		fclose (fp_extra);
		*ep = strdup(tmp);
		return (NOTOK);
	}

	while (TRUE)
	{
		if ((fgets (buf, BUFSIZ -1, fp_extra) == NULL) 
			&& !feof (fp_extra))
		{
			PP_DBG (( "File read error"));
			fclose (fp_extra);
			return (NOTOK);
		}

		if (!feof (fp_extra))
			fputs (buf, fp_out);
		else
		{
			fclose (fp_extra);
			return (OK);
		}
	}

}

/*  */
/* P2 structure bits */
do_822_extension_88 (pe)
PE	pe;
{
	char	*hdr_txt, *cix;

	struct type_IOB_RFC822FieldList 	*list, *ix;
	PY_pepy[0] = 0;
	if (decode_IOB_RFC822FieldList (pe, 0, NULLIP, NULLVP, &list) != OK)
		PP_OPER(NULLCP,
			("decode_IOB_RFC822Field () Failed : [%s]", PY_pepy));
	else {
		for (ix = list; ix != NULL; ix = ix -> next) {
			hdr_txt = qb2str(ix->RFC822Field);
			if ((cix = index(hdr_txt, ':')) == NULLCP) {
				PP_LOG(LLOG_EXCEPTIONS,
			       ("Unable to parse extra 822 hdr '%s'", hdr_txt));
				free(hdr_txt);
				continue;
			}
			*cix++ = '\0';
			while (isspace(*cix)) cix++;
			do_key(hdr_txt);
			do_token (cix);
			free(hdr_txt);
		}
		free_IOB_RFC822FieldList(list);
	}
}

do_822_extension_84 (pe)
PE      pe;
{
        char    *hdr_txt, *ix;

        struct type_IOB_RFC822Field      *hdr;
        PY_pepy[0] = 0;
        if (decode_IOB_RFC822Field (pe, 0, NULLIP, NULLVP, &hdr) != OK)
                PP_OPER(NULLCP,
                        ("decode_IOB_RFC822Field () Failed : [%s]", PY_pepy));
        else {
                hdr_txt = qb2str(hdr);
                if ((ix = index(hdr_txt, ':')) == NULLCP) {
                        PP_LOG(LLOG_EXCEPTIONS,
                               ("Unable to parse extra 822 hdr '%s'", hdr_txt));
                        free(hdr_txt);
                        return;
                }
                *ix++ = '\0';
                while (isspace(*ix)) ix++;
                do_key(hdr_txt);
                do_token (ix);
                free(hdr_txt);
                free_IOB_RFC822Field(hdr);
        }
}

static char	discard_extn[BUFSIZ];

do_discarded_extensions	()
{
	do_key ("Discarded-X400-IPMS-Extensions");
	do_token(discard_extn);
}

do_incomplete_copy()
{
	do_key ("Incomplete-Copy");
	do_token ("");
}

do_languages (pe)
PE	pe;
{
	char	*lang_txt;

	struct type_IOB_Languages	*lang, *ix;
	PY_pepy[0] = 0;
	if (decode_IOB_Languages (pe, 0, NULLIP, NULLVP, &lang) != OK)
		PP_OPER(NULLCP,
			("decode_IOB_Languages () failed : [%s]", PY_pepy));
	else {
		ix = lang;
		while (ix != NULL) {
			lang_txt = qb2str(ix -> Language);
			do_key ("Language");
			do_token (lang_txt);
			free(lang_txt);
			ix = ix -> next;
		}
		free_IOB_Languages(lang);
	}
}
	
		
do_extension (extension)
struct type_IOB_HeadingExtension	*extension;
{
	OID	oid;

	if (extension -> type == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("NULL oid in header extension"));
		return;
	}
	if ((oid = str2oid(id_rfc_822_field_list)) == NULLOID) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unable to convert '%s' to an OID",
			id_rfc_822_field_list));
	}
		
	if (oid != NULLOID
	    && oid_cmp (extension -> type, oid) == 0)
		return do_822_extension_88 (extension -> value);

	if ((oid = str2oid(id_rfc_822_field)) == NULLOID) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unable to convert '%s' to an OID",
			id_rfc_822_field));
	}
	
	if (oid != NULLOID
	    && oid_cmp (extension -> type, oid) == 0)
		return do_822_extension_84 (extension -> value);

	if ((oid = str2oid(id_hex_incomplete_copy)) == NULLOID) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unable to convert '%s' to an OID",
			id_hex_incomplete_copy));
	}
	
	if (oid != NULLOID
	    && oid_cmp (extension -> type, oid) == 0)
		return do_incomplete_copy();

	if ((oid = str2oid(id_hex_languages)) == NULLOID) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unable to convert '%s' to an OID",
			id_hex_languages));
	}
	
	if (oid != NULLOID
	    && oid_cmp (extension -> type, oid) == 0)
		return do_languages (extension -> value);

	PP_LOG(LLOG_EXCEPTIONS,
	       ("Unrecognised oid '%s' in header extension", 
		oid2lstr(extension -> type)));
	if ('\0' != discard_extn[0])
		strcat(discard_extn, ", ");
	strcat(discard_extn, oid2lstr(extension -> type));
}

do_extensions (extensions)
struct type_IOB_ExtensionsField	*extensions;
{
	discard_extn[0] = '\0';
	while (extensions != NULL) {
		do_extension (extensions -> HeadingExtension);
		extensions = extensions -> next;
	}
	if ('\0' != discard_extn[0])
		do_discarded_extensions();
}

static CHARSET	*ia5 = (CHARSET *) NULL;
static CHARSET	*t61 = (CHARSET *) NULL;

static CHAR8U *t61toasc (t61str)
CHAR8U	*t61str;
{
	CHAR8U	*ia5str;
	
	if (ia5 == (CHARSET *) NULL)
		ia5 = getchset("CCITT_T.50.irv:1988", (INT16S) 29);
	if (t61 == (CHARSET *) NULL)
		t61 = getchset("T.61-8bit", (INT16S) 29);

	if (ia5 != (CHARSET *) NULL
	    && t61 != (CHARSET *) NULL) {
		ia5str = (CHAR8U *) malloc(sizeof(CHAR8U) * strlen((char *) t61str)+1);
		if (strncnv (ia5, t61, 
			     ia5str, t61str, 
			     strlen((char *) t61str),FALSE) >= 0)
			return ia5str;
	} 
	return (CHAR8U *) NULL;
}

do_subject (qb)
struct qbuf *qb;
{
	CHAR8U *t61str, *ia5str;
	t61str = (CHAR8U *) qb2str (qb);
	if ((ia5str = t61toasc((CHAR8U *) t61str)) != (CHAR8U *) NULL) {
		do_string (ia5str);
		free(ia5str);
	} else
		do_string (t61str);
	free (t61str);
}


do_recip_seq (recip_seq)
struct type_IOB_RecipientSequence *recip_seq;
{
	struct type_IOB_RecipientSpecifier *recip;

	recip = recip_seq -> RecipientSpecifier;
	get_ORD (recip -> recipient, abuf);

	if (recip -> notification__requests != NULLPE &&
		bit_test (recip -> notification__requests,
				bit_IOB_NotificationRequests_rn) == 1) {
		strcat (abuf, " (Receipt Notification Requested)");
	}

	if (recip -> notification__requests != NULLPE &&
		bit_test (recip -> notification__requests,
				bit_IOB_NotificationRequests_nrn) == 1) {
		strcat (abuf, " (Non Receipt Notification Requested)");
	}

	if (recip -> notification__requests != NULLPE &&
		bit_test (recip -> notification__requests,
				bit_IOB_NotificationRequests_ipm__return) == 1) {
		strcat (abuf, " (IPM Return Requested)");
	}

	if (recip -> reply__requested)
		strcat (abuf, " (Reply Requested)");

	do_token (abuf);
	if (recip_seq -> next != NULL)
	{
		do_same_line (",");
		do_recip_seq (recip_seq -> next);
	}
}



do_ORD_seq (ord_seq)
struct type_IOB_ORDescriptorSequence *ord_seq;
{
	get_ORD (ord_seq -> ORDescriptor, abuf);
	do_token (abuf);
	if (ord_seq -> next != NULL)
	{
		do_same_line (",");
		do_ORD_seq (ord_seq -> next);
	}
}


do_ORD (ord)
struct type_IOB_ORDescriptor *ord;
{
	get_ORD (ord, abuf);
	do_token (abuf);
}

OR_ptr pe2or();

static void add_phrase(out, in)
char	*out, *in;
{
	AP_ptr	tree = NULLAP, group, name, local, domain, route;
	
	if (ap_s2p(in, &tree, &group, &name, 
		   &local, &domain, &route) != (char *) NOTOK
	    && route != NULLAP && group == NULLAP) {
		char	*str;
		group = ap_new(AP_PERSON_NAME, local->ap_obvalue);
		str = ap_p2s(group, name, local, domain, route);
		strcpy (out, str);
		free(str);
		ap_free(group);
	} else
		strcpy (out, in);

	if (tree != NULLAP) {
		ap_sqdelete (tree, NULLAP);
		ap_free (tree);
	}
}

get_ORD (ord, buf)
struct type_IOB_ORDescriptor *ord;
char *buf;
{
	/* see rfc1138 4.7.2 */
	ORName *orn;
	char tbuf [BUFSIZ], asc[BUFSIZ];
	char *cp, *str = NULLCP, *dn = NULLCP;

	if ((orn = orname2orn(ord -> formal__name)) == NULLORName) {
		PP_DBG (("Failed to convert IOB_orname to ORName"));
		tbuf[0] = '\0';
	} else {
		if (or_or2rfc (orn->on_or, tbuf) != OK) {
			PP_DBG (("or_or2rfc failed"));
			tbuf[0] = '\0';
		}
		if (orn->on_dn != NULL)
			dn = dn2ufn (orn->on_dn, FALSE);
		ORName_free(orn);
	}

	if (ord -> free__form__name == NULL) 
		if (tbuf[0] == '\0')
			strcpy (buf, "Empty Address :;");
		else
			if (tbuf[0] == '@')
				sprintf (buf, "FOO <%s>", tbuf);
			else 
				/* if route add phrase */
				add_phrase(buf, tbuf);

	else
	{
		CHAR8U	*t61str, *ia5str;
		
		t61str = (CHAR8U *) qb2str (ord -> free__form__name);

		/* do t.61 -> ascii conversion */
		if ((ia5str = t61toasc((CHAR8U *) t61str)) != (CHAR8U *) NULL) {
			strcpy (asc, (char *) ia5str);
			free((char *) ia5str);
		} else
			strcpy (asc, (char *) t61str);
		
		if (tbuf [0] == '\0')
			sprintf (buf, "\"%s\" :;", asc);
		else
			sprintf (buf, "\"%s\" <%s>", asc, tbuf);
		free ((char *) t61str);
	}

	if (dn != NULLCP)
	{
		strcat (buf, " (DN=");
		strcat (buf, dn);
		strcat (buf, ")");
		free(dn);
	}

	if (ord -> telephone__number != NULL)
	{
		cp = qb2str (ord -> telephone__number);
		if (*cp != '\0') {
			strcat (buf, " (Tel ");
			strcat (buf, cp);
			strcat (buf, ")");
			free(cp);
		}
	}
	ap_s2s (buf, &str, order_pref);
	if (str != NULLCP
	    && *str != '\0') {
		(void) strcpy(buf, str);
		free(str);
	}
}

do_mid_seq (mid_seq, addMHS)
struct type_IOB_IPMIdentifierSequence *mid_seq;
int	addMHS;
{
	if (do_mid (mid_seq -> IPMIdentifier, addMHS) != OK)
		return NOTOK;
	if (mid_seq -> next != NULL)
	{
		do_same_line (",");
		return do_mid_seq (mid_seq -> next, addMHS);
	}
	return OK;
}

extern char *loc_dom_site;

do_mid (mid, addMHS)
struct type_IOB_IPMIdentifier *mid;
int	addMHS;
{
	char *midstring;
	ORName	*orn;
	char tbuf [BUFSIZ], buf[BUFSIZ];
	char *cp;
	AP_ptr ap;
	AP_ptr local;
	AP_ptr domain;
	/* see rfc 1138 4.7.3.4 */

	if (mid -> user__relative__identifier != NULL) {
		midstring = qb2str (mid -> user__relative__identifier);
		or_ps2asc (midstring, abuf);
		free(midstring);

		if (lexequ(abuf, "RFC-822") == 0)
			return do_mid_987(mid);

		if (mid -> user == NULL) {
			(void) sprintf (buf, "<%s>", abuf);
			if ((index(buf, '@') != NULLCP)
			    && (ap_s2p (buf, &ap, (AP_ptr *) 0, (AP_ptr *) 0, 
				       &local, &domain, 
				       (AP_ptr *) 0) != (char *) NOTOK)) {
				/* buf parses as 822.msg-id */
				do_token (buf);
				ap_free(ap);
				return OK;
			}
		}
	} else
		abuf[0] = '\0';

	/* try as x400 generated */
	if (mid -> user) {
		if ((orn = orname2orn (mid -> user)) != NULLORName) {
			or_or2std (orn->on_or, tbuf, FALSE);
			if (abuf[0] != '\0')
				sprintf (buf, "%s*%s", abuf, tbuf);
			else
				sprintf (buf, "*%s", tbuf);

			local = ap_new (AP_MAILBOX, buf);
			domain = ap_new (AP_DOMAIN, "MHS");
			cp = ap_p2s_nc (NULLAP, NULLAP, local, domain, NULLAP);
			sprintf (abuf, "<%s>", cp);
			free (cp);
			ap_free (local);
			ap_free (domain); 
			ORName_free(orn);
			do_token (abuf);
			return OK;
		}
	}

	/* no user */
	if (abuf[0] != '\0') {
		if (addMHS == TRUE) {
			(void) sprintf(buf, "%s*",abuf);
			local = ap_new(AP_MAILBOX, buf);
			domain = ap_new(AP_DOMAIN, "MHS");
			cp = ap_p2s_nc (NULLAP, NULLAP, local, domain, NULLAP);
			sprintf (abuf, "<%s>", cp);
			ap_free (domain); 
		} else {
			/* generate phrase as 1148 4.7.3.5 says */
			local = ap_new (AP_GROUP_NAME, abuf);
			cp = ap_p2s_nc(local, NULLAP, NULLAP, NULLAP, NULLAP);
			strcpy(abuf, cp);
		}
		free (cp);
		ap_free (local);
		do_token (abuf);
		return OK;
	}

	PP_DBG (( "Highly dubious ID"));
	(void) sprintf (abuf, "<FOO@%s>", loc_dom_site);
	do_token(abuf);
	/* gen mid */
	return NOTOK;
}

do_mid_987(mid)
struct type_IOB_IPMIdentifier	*mid;
{
	char	buf[BUFSIZ], buf2[BUFSIZ];
	ORName	*orn;

	if (mid -> user) {
		if ((orn = orname2orn(mid->user)) != NULLORName
		    && orn->on_or != NULLOR
		    && or_or2rfc_aux(orn->on_or, buf, FALSE) == OK) {
			(void) sprintf(buf2, "<%s>",buf);
			do_token (buf2);
			ORName_free(orn);
			return OK;
		}
		if (orn) ORName_free(orn);
	}

	PP_DBG(("Highly dubious 987 ID"));
	(void) sprintf(buf, "<FOO@%s>", loc_dom_site);
	do_token(buf);
	return NOTOK;
}

do_utc (utc_qb)
struct qbuf *utc_qb;
{
	char *cp;
	UTC	ut;

	cp = qb2str (utc_qb);
	ut = str2utct (cp, strlen(cp));
	do_p1_utc (ut);
	free (cp);
}

do_importance (importance)
int importance;
{
	switch (importance)
	{
		case int_IOB_ImportanceField_low:
			do_token ("Low");
			break;
		case int_IOB_ImportanceField_normal:
			do_token ("Normal");
			break;
		case int_IOB_ImportanceField_high:
			do_token ("High");
			break;
		    default:
			sprintf(abuf, "Undefined (value=%d)", importance);
			do_token (abuf);
			break;
	}
}

do_sensitivity (sensitivity)
int sensitivity;
{
	switch (sensitivity)
	{
		case int_IOB_SensitivityField_personal:
			do_token ("Personal");
			break;
		case int_IOB_SensitivityField_private:
			do_token ("Private");
			break;
		case int_IOB_SensitivityField_company__confidential:
			do_token ("Company-Confidential");
			break;
		    default:
			sprintf(abuf, "Undefined (value=%d)", sensitivity);
			do_token (abuf);
			break;
	}
}

do_boolean (bool)
char bool;
{
	if (bool)
		do_token ("TRUE");
	else
		do_token ("FALSE");
}

/*  */
/* Stuff for P1 */

do_p1_utc (ut)
UTC	ut;
{
	UTC	lut;
	lut = utclocalise(ut);
	if (UTC2rfc (lut, abuf) == NOTOK)
		strcpy(abuf, ">>INVALID UTC TIME<<");
	do_token (abuf);
	free((char *) lut);
}

do_p1_addrs (addr)
ADDR *addr;
{
	if (adr2rfc (addr, abuf, order_pref) == OK)
		do_token (abuf);
	if (addr -> ad_next != NULL)
	{
		do_same_line (",");
		do_p1_addrs (addr -> ad_next);
	}
}

do_p1_mid (mid)
MPDUid *mid;
{
	msgid2rfc (mid, abuf); 
	do_token (abuf);
}

do_p1_eit (eit)
EncodedIT *eit;
{
	x400eits2rfc (eit, abuf); 
	do_token (abuf);
}

do_p1_priority (priority)
int priority;
{
	switch (priority)
	{
		case PRIO_NORMAL:
			do_token ("Normal");
			break;
		case PRIO_URGENT:
			do_token ("Urgent");
			break;
		case PRIO_NONURGENT:
			do_token ("Non-Urgent");
			break;
	}
}

do_p1_trace (trace)
Trace *trace;
{

	if (trace->trace_next != NULL)
		do_p1_trace(trace->trace_next);
	do_key ("X400-Received");
	x400trace2rfc (trace,abuf);
	do_token (abuf);
}

do_p1_fullname (fullname)
FullName	*fullname;
{
	do_token (fullname->fn_addr);
}

do_p1_DLHistory (history)
DLHistory	*history;
{
	char	temp[BUFSIZ];
	OR_ptr	or;
	UTC	lut;
	if (history -> dlh_next != NULL)
		do_p1_DLHistory(history -> dlh_next);
	do_key ("DL-Expansion-History");
	or = or_std2or(history -> dlh_addr);
	or_or2rfc_aux(or, temp, FALSE);
	(void) sprintf (abuf,
			"%s",
			temp);
	if (history -> dlh_time != 0) {
		lut = utclocalise(history -> dlh_time);
		UTC2rfc (lut, temp);
		free ((char *) lut);
		(void) sprintf (abuf,
				"%s ; %s;",
				abuf, temp);
	}
	do_token (abuf);
}
			

/*  */
/* General purpose header output */

do_key (key)
char *key;
{
	if (firstLine != TRUE) 
		fputs ("\n", fp_out);
	else
		firstLine = FALSE;
	fputs (key, fp_out);
	fputs (": ", fp_out);
}

do_string (string)              /* fold string on spaces for unstructured */
char *string;
{
	fputs (" ", fp_out);
	fputs (string, fp_out);
}

do_same_line (token)
char *token;
{
	fputs (token, fp_out);
}

do_token (token)
char *token;
{
	fputs (" ", fp_out);
	fputs (token, fp_out);
}

/*  */

do_delinfo (file, ep)
char	*file;
char	**ep;
{
	FILE	*fp_in;
	PE	pe = NULLPE;
	PS	ps = NULLPS;
	struct type_IOB_MessageParameters *params = (struct type_IOB_MessageParameters *) NULL;
	char	buf[BUFSIZ];

	if ((fp_in = fopen (file, "r")) == NULL)
	{
		PP_SLOG (LLOG_EXCEPTIONS, file,
			("Can't open file for reading"));
		fclose (fp_in);
		(void) sprintf (buf,
				"Failed to open input file '%s'",
				file);
		*ep = strdup(buf);
		return NOTOK;
	}

	if (((ps = ps_alloc (std_open)) == NULLPS) ||
	    (std_setup (ps, fp_in) == NOTOK))
	{
		PP_LOG (LLOG_EXCEPTIONS, ("p2to822() failed to setup PS"));
		*ep = strdup("Failed to set up PS");
		fclose(fp_in);
		if (ps) ps_free(ps);
		return NOTOK;
	}


	if ((pe = ps2pe (ps)) == NULLPE)
	{
		PP_LOG (LLOG_EXCEPTIONS, ("ps2pe error on file '%s'", file));
		fclose (fp_in);
		if (pe) pe_free(pe);
		if (ps) ps_free(ps);
	}

	PY_pepy[0] = 0;
	if (decode_IOB_MessageParameters(pe, 0, NULLIP, NULLVP, &params) != OK)
	{
		PP_OPER(NULLCP,
			("decode_IOB_MessageParameters() failed : [%s]", PY_pepy));
		convertresult = NOTOK;
		*ep = strdup("Illegal ASN.1 in forwarded message parameters");
		fclose (fp_in);
		if (pe) pe_free(pe);
		if (ps) ps_free(ps);
		if (params) free_IOB_MessageParameters(params);
		return NOTOK;
	}
	if (0 != PY_pepy[0])
		PP_LOG(LLOG_EXCEPTIONS,
		       ("decode_IOB_MessageParameters non-fatal failure [%s]",
			PY_pepy));
	if (params->delivery__time)
		do_delivery_time(params->delivery__time);
	if (params->delivery__envelope)
		do_delivery_envelope(params->delivery__envelope);
	fclose (fp_in);
	if (pe) pe_free(pe);
	if (ps) ps_free(ps);
	if (params) free_IOB_MessageParameters(params);
	return OK;
}

do_delivery_time(time)
struct type_MTA_MessageDeliveryTime *time;
{
	do_key("Delivery-Date");
	do_utc(time);
}

do_delivery_envelope(envelope)
struct type_MTA_OtherMessageDeliveryFields	*envelope;
{
	char	buf[BUFSIZ];

	do_key("X400-Content-Type");
	switch (envelope->member_MTA_8->offset) {
	    case choice_MTA_4_built__in:
		do_built_in_content(envelope->member_MTA_8->un.built__in);
		break;
	    case choice_MTA_4_external:
		do_external_content (envelope->member_MTA_8->un.external);
		break;
	}
	
	if (head -> originator == NULL
	    && head -> authorizing__users == NULL)
		do_key ("From");
	else
		do_key("X400-Originator");
	do_MTA_ORName(envelope->originator__name, buf);
	do_token(buf);

	if (envelope->original__encoded__information__types != NULL) {
		do_key ("Original-Encoded-Information-Types");
		do_MTA_eits (envelope->original__encoded__information__types);
	}

	if (envelope->priority 
	    && envelope->priority->parm != int_MTA_Priority_normal) {
		do_key ("Priority");
		do_MTA_priority (envelope->priority->parm);
	}

	if (envelope->delivery__flags
	    && bit_ison (envelope->delivery__flags,
			 bit_MTA_DeliveryFlags_implicit__conversion__prohibited)) {
		do_key("Conversion");
		do_token("Prohibited");
	}

	if (envelope->this__recipient__name) {
		do_key("X400-Recipients");
		do_MTA_this_recipient(envelope->this__recipient__name,
				      envelope->originally__intended__recipient__name);
	}

	if (envelope->converted__encoded__information__types != NULL) {
		do_key ("Converted-Encoded-Information-Types");
		do_MTA_eits (envelope->converted__encoded__information__types);
	}

	if (envelope->other__recipient__names) {
		do_key("X400-Recipients");
		do_MTA_other_recips(envelope->other__recipient__names);
	}
	
	do_key("Date");
	do_utc(envelope->message__submission__time);

	if (envelope->content__identifier) {
		char	*str = qb2str(envelope->content__identifier);
		do_key("Content-Identifier");
		do_token(str);
		free(str);
	}

	if (envelope->extensions)
		do_MTA_extensions(envelope->extensions);
	
}

do_MTS_discard_extensions ()
{
	do_key ("Discarded-X400-MTS-Extensions");
	do_token(discard_extn);
}

do_MTA_extensions(extns)
struct type_MTA_Extensions *extns;
{
	bzero((char *) &del_info_q, sizeof(del_info_q));
	bzero((char *) &del_info_recip, sizeof(del_info_recip));

	discard_extn[0] = '\0';
	while (extns != (struct type_MTA_Extensions *) NULL) {
		switch (extns->ExtensionField->type->offset) {
		    case type_MTA_ExtensionType_local:
			do_MTA_local_extension (extns->ExtensionField);
			break;
		    case type_MTA_ExtensionType_global:
			do_MTA_global_extension (extns->ExtensionField);
			break;
		}
		extns = extns -> next;
	}
	if ('\0' != discard_extn[0])
		do_MTS_discard_extensions();
}

do_MTA_local_extension (ext)
struct type_MTA_ExtensionField *ext;
{
	PP_LOG(LLOG_EXCEPTIONS,
	       ("Unknown local extension '%s'",
		oid2lstr(ext->type->un.local)));
	if ('\0' != discard_extn[0])
		strcat (discard_extn, ", ");
	strcat(discard_extn, oid2lstr(ext->type->un.local));
}

do_MTA_global_extension (ext)
struct type_MTA_ExtensionField *ext;
{
	char	buffer[BUFSIZ];
	switch (ext->type->un.global) {
	    case EXT_CONVERSION_WITH_LOSS_PROHIBITED:
		if (decode_extension
		    ((caddr_t) &(del_info_q.conversion_with_loss_prohibited),
		     _ZConversionWithLossProhibitedExt,
		     &_ZExt_mod,
		     "Extensions.ConversionWithLossProhibited",
		     decode_ext_char,
		     ext) == OK
		    && del_info_q.conversion_with_loss_prohibited) {
			do_key ("Conversion-With-Loss");
			do_token("Prohibited");
		}
		break;
	    case EXT_REQUESTED_DELIVERY_METHOD:
		if (decode_extension
			((caddr_t)del_info_recip.ad_req_del,
			 _ZRequestedDeliveryMethodExt,
			 &_ZExt_mod,
			 "Extensions.RequestedDeliveryMethod",
			 decode_ext_rdm,
			 ext) == OK) {
			if (del_info_recip.ad_req_del[0] != AD_RDM_NOTUSED
			    && del_info_recip.ad_req_del[0] != AD_RDM_ANY) {
				int 	ix = 0, first = OK;
				char	*type;

				do_key("Requested-Delivery-Methods");
				buffer[0] = '\0';
				while (ix < AD_RDM_MAX 
				       && del_info_recip.ad_req_del[ix] != AD_RDM_NOTUSED
				       && del_info_recip.ad_req_del[ix] != AD_RDM_ANY) {
					if ((type = rcmd_srch(del_info_recip.ad_req_del[ix], atbl_rdm)) != NULLCP) {
						if (OK != first )
							strcat(buffer, ", ");
						strcat(buffer,
						       type);
					}
					ix++;
					first = NOTOK;
				}
				do_token(buffer);
			}
		}
		break;
	    case EXT_ORIGINATOR_RETURN_ADDRESS:
		do_MTA_orig_ret_addr (ext);
		break;
	    case EXT_PHYSICAL_FORWARDING_ADDRESS_REQUEST:
		decode_extension 
			((caddr_t)&(del_info_recip.ad_phys_fw_ad_req),
			 _ZPhysicalForwardingAddressRequestExt,
			 &_ZExt_mod,
			 "Extensions.PhysicalForwardingAddressRequest",
			 decode_ext_char,
			 ext);
		break;
	    case EXT_PHYSICAL_DELIVERY_MODES:
		decode_extension
			((caddr_t)&(del_info_recip.ad_phys_modes),
			 _ZPhysicalDeliveryModesExt,
			 &_ZExt_mod,
			 "Extensions.PhysicalDeliveryModes",
			 decode_ext_pdm,
			 ext);
		break;
	    case EXT_REGISTERED_MAIL:
		decode_extension
			((caddr_t)&(del_info_recip.ad_reg_mail_type),
			 _ZRegisteredMailTypeExt,

			 &_ZExt_mod,
			 "Extensions.RegisteredMailType",
			 decode_ext_int,
			 ext);
		break;
	    case EXT_RECIPIENT_NUMBER_FOR_ADVICE:
		decode_extension
			((caddr_t)&(del_info_recip.ad_recip_number_for_advice),
				 _ZRecipientNumberForAdviceExt,
				 &_ZExt_mod,
				 "Extensions.RecipientNumberForAdvice",
				 decode_ext_rnfa,
				 ext);
		break;
	    case EXT_PHYSICAL_RENDITION_ATTRIBUTES:
		decode_extension
			((caddr_t)&(del_info_recip.ad_phys_rendition_attribs),
			 _ZPhysicalRenditionAttributesExt,
			 &_ZExt_mod,
			 "Extensions.PhysicalRenditionAttributes",
			 decode_ext_pra,
			 ext);
		break;
	    case EXT_PHYSICAL_DELIVERY_REPORT_REQUEST:
		decode_extension
			((caddr_t)&(del_info_recip.ad_pd_report_request),
			 _ZPhysicalDeliveryReportRequestExt,
			 &_ZExt_mod,
			 "Extensions.PhysicalDeliveryReportRequest",
			 decode_ext_int,
			 ext);
		break;
	    case EXT_PHYSICAL_FORWARDING_PROHIBITED:
		decode_extension
			((caddr_t)&(del_info_recip.ad_phys_forward),
			 _ZPhysicalForwardingProhibitedExt,
			 &_ZExt_mod,
			 "Extensions.PhysicalForwardingProhibited",
			 decode_ext_char,
			 ext);
		break;
	    case EXT_REDIRECTION_HISTORY:
		decode_extension
			((caddr_t)&(del_info_recip.ad_redirection_history),
			 _ZRedirectionHistoryExt,
			 &_ZExt_mod,
			 "Extensions.RedirectionHistory",
			 decode_ext_redir,
			 ext);

		break;
	    case EXT_DL_EXPANSION_HISTORY:
		if (decode_extension
			((caddr_t)&(del_info_q.dl_expansion_history),
			 _ZDLExpansionHistoryExt,
			 &_ZExt_mod,
			 "Extensions.DLExpansionHistory",
			 ext_decode_dlexph,
			 ext) == OK)
			do_p1_DLHistory (del_info_q.dl_expansion_history);
		break;
	    default:
		break;
	}
}

do_MTA_orig_ret_addr(ext)
struct type_MTA_ExtensionField *ext;
{
	struct type_Ext_OriginatorReturnAddress *ora;
	OR_ptr	or;
	char	buf[BUFSIZ];

	if (decode_Ext_OriginatorReturnAddress (ext -> value, 1,
						   NULLIP, NULLVP,
						   &ora) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't parse originator return address [%s]",
			 PY_pepy));
		return;
	}

	or = oradr2ora (ora);
	
	if (or_or2rfc(or, buf) == OK) {
		do_key("Originator-Return-Address");
		do_token(buf);
	}
	or_free(or);
	free_Ext_OriginatorReturnAddress (ora);
}

do_MTA_eits (eits)
struct type_MTA_EncodedInformationTypes *eits;
{
	int	some;

	some = do_MTA_buildin_eits(eits->built__in__encoded__information__types);
	if (eits->external__encoded__information__types) {
		if (TRUE == some)
			do_same_line(",");
		do_MTA_external_eits(eits->external__encoded__information__types);
	}
}

do_MTA_external_eits(eits)
struct type_MTA_ExternalEncodedInformationTypes	*eits;
{
	int some = FALSE;

	while (eits != (struct type_MTA_ExternalEncodedInformationTypes *) NULL) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token (oid2lstr(eits->ExternalEncodedInformationType));
		eits = eits -> next;
	}
}

do_MTA_buildin_eits (eits)
struct type_MTA_BuiltInEncodedInformationTypes	*eits;
{
	int	some = FALSE;

	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_undefined)) {
		do_token("undefined (0)");
		some = TRUE;
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_telex)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("telex (1)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_ia5__text)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("ia5 text (2)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_g3__facsimile)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("g3 facsimile (3)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_g4__class__1)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("g4 class 1 (4)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_teletex)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("teletex (5)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_videotex)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("videotex (6)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_voice)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("voice (7)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_sfd)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("sfd (8)");
	}
	if (bit_ison(eits,
		     bit_MTA_BuiltInEncodedInformationTypes_mixed__mode)) {
		if (TRUE == some)
			do_same_line(",");
		else
			some = TRUE;
		do_token("mixed mode (9)");
	}

	return some;
}


	
do_MTA_priority (priority)
int priority;
{
	switch (priority)
	{
	    case int_MTA_Priority_normal:
		do_token ("Normal");
		break;
	    case int_MTA_Priority_urgent:
		do_token ("Urgent");
		break;
	    case int_MTA_Priority_non__urgent:
		do_token ("Non-Urgent");
		break;
	}
}

do_MTA_ORName (orname, buf)
struct type_MTA_ORName *orname;
char	*buf;
{
	ORName	*orn;
	char	tbuf[BUFSIZ], *dn = NULLCP;

	if ((orn = orname2orn((struct type_IOB_ORName *) orname)) == NULLORName) {
		PP_DBG (("Failed to convert MTA_orname to ORName"));
		sprintf(buf, "");
		return;
	} else {
		if (or_or2rfc (orn->on_or, tbuf) != OK) {
			PP_DBG (("or_or2rfc failed"));
			tbuf[0] = '\0';
		}
		if (orn->on_dn != NULL)
			dn = dn2ufn (orn->on_dn, FALSE);
		ORName_free(orn);
	}

	if (dn != NULLCP) {
		sprintf(buf, "%s (DN=%s)",
			tbuf, dn);
		free(dn);
	} else
		strcpy(buf, tbuf);
}
	
do_MTA_other_recips (list)
struct type_MTA_OtherRecipientNames *list;
{
	char	buf[BUFSIZ];
	while (list != (struct type_MTA_OtherRecipientNames *) NULL) {
		do_MTA_ORName (list -> OtherRecipientName, buf);
		do_token(buf);
		list = list -> next;
		if (list != (struct type_MTA_OtherRecipientNames *) NULL) 
			do_same_line (",");
	}
}

do_MTA_this_recipient (recip, orig)
struct type_MTA_ORName	*recip, *orig;
{
	char	buf[BUFSIZ];
	do_MTA_ORName(recip, buf);
	if (orig != (struct type_MTA_ORName *) NULL) {
		char	tmp[BUFSIZ], tmp2[BUFSIZ];
		do_MTA_ORName(orig, tmp);
		sprintf(tmp2, "(Originally To: %s Redirected)",
			tmp);
		strcat(buf, tmp2);
	}
	adr2comments(&del_info_recip, buf);
	do_token(buf);
}

do_built_in_content(content)
struct type_MTA_BuiltInContentType *content;
{
	char	buf[BUFSIZ];

	switch (content->parm) {
	    case int_MTA_BuiltInContentType_unidentified:
		do_token("unidentified (0)");
		break;
	    case int_MTA_BuiltInContentType_external:
		do_token("external (1)");
		break;
	    case int_MTA_BuiltInContentType_interpersonal__messaging__1984:
		do_token("P2-1984 (2)");
		break;
	    case int_MTA_BuiltInContentType_interpersonal__messaging__1988:
		do_token("P2-1988 (22)");
		break;
	    default:
		sprintf(buf, "unknown (%d)",
			content->parm);
		do_token (buf);
		break;
	}
}

do_external_content (content)
OID	content;
{
	do_token (oid2lstr(content));
}


/* nicked from x40088 */
static int decode_extension (value, magic_num, mod,
			     label, decoder, ext)
caddr_t	value;
int	magic_num;
modtyp	*mod;
IFP	decoder;
char	*label;
struct type_MTA_ExtensionField *ext;
{
	caddr_t *genp;
	int retval;

#if PP_DEBUG > 0
	if (pp_log_norm -> ll_events & LLOG_PDUS)
		pvpdu (pp_log_norm, magic_num, mod, 
			ext -> value, label, PDU_READ);
#endif
	if (dec_f(magic_num, mod, ext -> value, 1,
		  NULLIP, NULLVP, &genp) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't parse %s value [%s]", label, PY_pepy));
		return DONE;
	}

	if ((retval = (*decoder) (value, genp)) != OK)
		return retval;

	fre_obj((char *) genp, mod->md_dtab[magic_num], mod, 1);
	return OK;
}

static int decode_ext_char (cp, ptr)
char *cp;
struct type_Ext_DLExpansionProhibited *ptr; /* any integer type will do */
{
	*cp = ptr -> parm;
	return OK;
}

static int decode_ext_rdm (value, genp)
int	value[AD_RDM_MAX];
struct type_Ext_RequestedDeliveryMethod *genp;
{
	int i;
	
	for (i = 0; i < AD_RDM_MAX && genp;
	     i++, genp = genp -> next)
		value[i] = genp -> element_Ext_0;
	return OK;
}

static int decode_ext_pdm (value, genp)
int	*value;
PE	genp;
{
#define setbit(t,v) \
	(*value) |= (bit_ison(genp, (t)) ? (v) : 0)

	setbit (bit_Ext_PhysicalDeliveryModes_ordinary__mail,
		AD_PM_ORD);
	setbit (bit_Ext_PhysicalDeliveryModes_special__delivery,
		AD_PM_SPEC);
	setbit (bit_Ext_PhysicalDeliveryModes_express__mail,
		AD_PM_EXPR);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection,
		AD_PM_CNT);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection__with__telephone__advice,
		AD_PM_CNT_PHONE);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection__with__telex__advice,
		AD_PM_CNT_TLX);
	setbit (bit_Ext_PhysicalDeliveryModes_counter__collection__with__teletex__advice,
		AD_PM_CNT_TTX);
	setbit (bit_Ext_PhysicalDeliveryModes_bureau__fax__delivery,
		AD_PM_CNT_BUREAU);
#undef setbit
	return OK;
}

static int decode_ext_int (ip, ptr)
int *ip;
struct type_Ext_DLExpansionProhibited *ptr; /* any integer type will do */
{
	*ip = ptr -> parm;
	return OK;
}

static int decode_ext_rnfa (value, genp)
char	**value;
struct qbuf *genp;
{
	if ((*value = qb2str (genp)) == NULL)
		return NOTOK;
	return OK;
}

static int decode_ext_pra (value, gen)
OID	*value;
OID	gen;
{
	if ((*value = oid_cpy (gen)) != NULLOID)
		return OK;
	return NOTOK;
}


static int decode_1redir (rpp, redir)
Redirection **rpp;
struct type_Ext_Redirection *redir;
{
	ADDR *ad;
	int retval;

	*rpp = (Redirection *) smalloc (sizeof **rpp);
	if ((retval =
	     load_addr (&ad, redir -> intended__recipient__name -> address))
	    != OK)
		return retval;
	(*rpp) -> rd_addr = ad -> ad_value;
	ad -> ad_value = NULLCP;
	(*rpp) -> rd_dn = ad -> ad_dn;
	ad -> ad_dn = NULLCP;
	adr_free (ad);

	if ((retval = load_time (&(*rpp) -> rd_time,
				 redir -> intended__recipient__name -> redirection__time)) != OK)
		return retval;
	(*rpp) -> rd_reason = redir -> redirection__reason -> parm;
	return OK;
}

static int decode_ext_redir (value, genp)
Redirection **value;
struct type_Ext_RedirectionHistory *genp;
{
	int retval;

	while (genp) {
		if ((retval = decode_1redir (value, genp->Redirection)) != OK)
			return retval;
		value = &(*value) -> rd_next;
		genp = genp -> next;
	}
	return OK;
}

static int ext_decode_dlh (dlp, dpp)
struct type_Ext_DLExpansion *dlp;
DLHistory **dpp;
{
	UTC	ut;
	ADDR	*ad;
	int retval;

	if ((retval = load_time (&ut, dlp -> dl__expansion__time)) != OK)
		return retval;
	if ((retval = load_addr (&ad, dlp -> address)) != OK)
		return retval;

	*dpp = dlh_new (ad -> ad_value, ad -> ad_dn , ut);
	adr_free (ad);

	free ((char *)ut);
	return OK;
}

static int ext_decode_dlexph (dpp, genp)
DLHistory **dpp;
struct type_Ext_DLExpansionHistory *genp;
{
	struct type_Ext_DLExpansionHistory *dlhp;
	DLHistory *dp;
	int retval;

	for (dlhp = genp; dlhp; dlhp = dlhp -> next) {
		if ((retval = ext_decode_dlh (dlhp -> DLExpansion, &dp)) != OK)
			return retval;
		dlh_add (dpp, dp);
	}
	return OK;
}


static int load_time (utc, utcstr)
UTC	*utc;
struct type_UNIV_UTCTime *utcstr;
{
	char *str = qb2str (utcstr);
	if (str == NULLCP)
		return NOTOK;
	*utc = str2utct (str, strlen(str));
	if (*utc == NULLUTC)
		return NOTOK;
	free (str);
	*utc = utcdup (*utc);
	return *utc ? OK : NOTOK;
}

static int load_addr (app, orname)
ADDR	**app;
struct type_MTA_ORName *orname;
{
	ORName *orn;
	char	buf[BUFSIZ];
	ADDR	*ap;

	if ((orn = orname2orn (orname)) == NULL)
		return NOTOK;

	or_or2std (orn -> on_or, buf, 0);
	*app = ap  = adr_new (buf, AD_X400_TYPE, 1);
	ap -> ad_subtype = AD_X400_88;

	if (orn -> on_dn)
		ap -> ad_dn = dn2str (orn -> on_dn);
	ORName_free (orn);
	return OK;
}
