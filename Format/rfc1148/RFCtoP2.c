/* RFCtoP2.c : 1138 encoding 822 -> P2 for headers */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/RFCtoP2.c,v 6.0 1991/12/18 20:20:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/RFCtoP2.c,v 6.0 1991/12/18 20:20:34 jpo Rel $
 *
 * $Log: RFCtoP2.c,v $
 * Revision 6.0  1991/12/18  20:20:34  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "util.h"
#include "IOB-types.h"
#include "rfc-hdr.h"
#include "ap.h"
#include "or.h"
#include "oids.h"
#include "charset.h"
#include "chan.h"

static FILE *fp_P2_out;
static FILE *fp_rfc_in;
static FILE *fp_P2_ext;
static char P2_ext [MAXPATHLENGTH];
static CHAR8U	*asctot61();
static struct type_IOB_Heading *head;
static char first_hdr = TRUE;
static int comments_started = FALSE;

extern time_t time();
extern struct type_IOB_ORName	*orn2orname();

RFCtoP2 (rfc_in, P2_out, P2_ext_out, ep, x40084)
char *rfc_in;
char *P2_out;
char *P2_ext_out;
char	**ep;
int	x40084;
{
	PE      pe;
	PS      ps;
	int     retval;
	char	buf[BUFSIZ];

	first_hdr = TRUE;
	comments_started = FALSE;
	if (P2_ext_out)
		strcpy (P2_ext, P2_ext_out);
	else
		P2_ext[0] = '\0';

	head = (struct type_IOB_Heading *)
		calloc (1, sizeof (struct type_IOB_Heading));

	if ( head ==  (struct type_IOB_Heading *) NOTOK)

	{
		PP_LOG (LLOG_EXCEPTIONS, ("RFCtoP2 - malloc failed"));
		return NOTOK;
	}                       /* allocate h, so that we can free */
				/* cleanly at the end of all this */

	if (P2_out == NULLCP)
		fp_P2_out = stdout;

	else if ((fp_P2_out = fopen (P2_out, "w")) == NULL )
	{
		PP_SLOG (LLOG_EXCEPTIONS, P2_out,
			 ("Can't open file"));
		(void) sprintf (buf,
				"Unable to open output file '%s'",
				P2_out);
		*ep = strdup(buf);
		return NOTOK;
	}

	if (rfc_in == NULLCP)
		fp_rfc_in = stdin;
	else if ((fp_rfc_in = fopen (rfc_in, "r")) == NULL )
	{
		PP_SLOG (LLOG_EXCEPTIONS, rfc_in,
			 ("Can't open file"));
		(void) sprintf (buf,
				"Unable to open input file '%s'",
				rfc_in);
		*ep = strdup(buf);
		fclose (fp_P2_out);
		return NOTOK;
	}



	if (((ps = ps_alloc (std_open)) == NULLPS) ||
		(std_setup (ps, fp_P2_out) == NOTOK))
	{
		PP_LOG (LLOG_EXCEPTIONS, ("RFC822toP2() failed to setup PS"));
		*ep = strdup("Failed to setup PS");
		retval = NOTOK;
		goto cleanup;
	}


			/* set defaults */

	head -> importance = (struct type_IOB_ImportanceField *) calloc (1, sizeof(struct type_IOB_ImportanceField));
	head -> importance -> parm = int_IOB_ImportanceField_normal;

	if (build_hdr (x40084, ep) == NOTOK)
	{
		PP_LOG (LLOG_EXCEPTIONS, ("build_hdr blew it"));
		retval = NOTOK;
		goto cleanup;
	}

	PY_pepy[0] = 0;

	if (encode_IOB_Heading (&pe, 0, 0, NULLCP, head) != OK)
	{
		PP_OPER (NULLCP, 
			 ("RFCtoP2() failed to encode header [%s]", PY_pepy));
		(void) sprintf (buf,
				"Unable to encode p2 hdr ['%s']", 
				PY_pepy);
		*ep = strdup(buf);
		retval = NOTOK;
		goto cleanup;
	}

	if (PY_pepy[0] != 0)
		PP_LOG(LLOG_EXCEPTIONS,
		       ("encode_IOB_Heading non-fatal failure [%s]",PY_pepy));

	if (pe2ps (ps, pe) == NOTOK)
	{
		PP_LOG (LLOG_EXCEPTIONS, ("RFCtoP2 failed to write heading"));
		*ep = strdup("Failed to write header");
		pe_free (pe);
		retval = NOTOK;
		goto cleanup;
	}

	retval = OK;

cleanup:
	ps_free (ps);
	fclose (fp_P2_out);
	fclose (fp_rfc_in);
	if (fp_P2_ext != NULL)
		fclose (fp_P2_ext);
	free_IOB_Heading (head);
	return retval;
}

/*  */

build_hdr(x40084, ep)
int	x40084;
char	**ep;
{
	char	*field, *key;
	struct type_IOB_RecipientSequence	*from = NULL;
	struct type_IOB_RecipientSequence	*temp;
	struct type_IOB_ORDescriptorSequence	*ix, *tail = (struct type_IOB_ORDescriptorSequence *) 0;
	struct type_IOB_RFC822FieldList		*hdr_extns = NULL;
	char	buf[BUFSIZ];

	while (TRUE) {
		switch (get_hdr (&key, &field)) {
		    case HDR_EOH:
			PP_DBG (("HDR_EOH"));
			break;

		    case HDR_ERROR:
			PP_LOG (LLOG_EXCEPTIONS,
				("Error reading header"));
			*ep = strdup("Error reading header");
			return NOTOK;

		    case HDR_ILLEGAL:
			PP_LOG (LLOG_EXCEPTIONS,
				("Erroneous header '%s' '%s'", key, field));
			(void) sprintf (buf,
				       "Erroneous header '%s' '%s'",
				       key, field);
			*ep = strdup(buf);
			return NOTOK;

		    case HDR_IGNORE:
			PP_DBG (("Ignoring header'%s'", key));
			continue;

		    case HDR_COMMENT:
			if (hdr_comment (key, field, ep, x40084) != OK)
				return NOTOK;
			continue;
			
		    case HDR_MID:
			if (hdr_mid (field, &(head -> this__IPM)) != OK)
				PP_DBG (("Bad message id '%s'", field));
			continue;

		    case HDR_FROM:
			if (hdr_recip_set (field, &from, x40084) != OK)
				PP_DBG (("Bad From '%s'", field));
			continue;

		    case HDR_SENDER:
			if (hdr_ordesc (field, &head -> originator, x40084) != OK)
				PP_DBG (("Bad Sender '%s'", field));
			continue;

		    case HDR_REPLY_BY:
			if (hdr_reply_by (field, &(head -> reply__time)) != OK)
				PP_DBG (("Bad reply date '%s'", field));
			continue;

		    case HDR_REPLY_TO:
			if (hdr_ordesc_seq (field, 
					    &(head -> reply__recipients),
					    x40084) != OK)
				PP_DBG (("Bad replied to '%s'", field));
			continue;
			
		    case HDR_IMPORTANCE:
			if (hdr_importance (field,
					    &(head->importance->parm)) != OK)
				PP_DBG (("Bad importance '%s'", field));
			continue;

		    case HDR_SENSITIVITY:
			if (hdr_sensitivity (field,
					     &(head->sensitivity)) != OK)
				PP_DBG (("Bad sensitivity '%s'", field));
			continue;

		    case HDR_AUTOFORWARDED:
			if (hdr_autoforwarded (field,
					       &(head->auto__forwarded)) != OK)
				PP_DBG(("Bad autoforwarded '%s'", field));
			continue;

		    case HDR_TO:
			if (hdr_recip_set (field, 
					   &(head -> primary__recipients),
					   x40084) != OK)
				PP_DBG (("Bad to '%s'", field));
			continue;

		    case HDR_CC:
			if (hdr_recip_set (field, 
					   &(head -> copy__recipients),
					   x40084) != OK)
				PP_DBG (("Bad cc '%s'", field));
			continue;

		    case HDR_BCC:
			if (hdr_recip_set (field, 
					   &(head -> blind__copy__recipients),
					   x40084) != OK)
				PP_DBG (("Bad bcc '%s'", field));
			continue;

		    case HDR_IN_REPLY_TO:
			if (hdr_reply_to (field, 
					  &(head -> replied__to__IPM),
					  &(head -> related__IPMs)) != OK)
				PP_DBG (("Bad in reply to '%s'", field));
			continue;

		    case HDR_OBSOLETES:
			if (hdr_mid_seq (field, &(head -> obsoleted__IPMs)) != OK)
				PP_DBG (("Bad obsoletes '%s'", field));
			continue;

		    case HDR_REFERENCES:
			if (hdr_mid_seq (field, &(head -> related__IPMs)) != OK)
				PP_DBG (("Bad references '%s'", field));
			continue;

		    case HDR_EXTENSIONS:
			if (x40084 == TRUE) {
				if (hdr_comment (key, field, ep, x40084) != OK)
					return NOTOK;
			} else if (hdr_extension (key, field, 
						  &(hdr_extns)) != OK)
				PP_DBG (("Bad extension '%s'", field));
			continue;

		    case HDR_INCOMPLETE_COPY:
			if (x40084 == TRUE) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unable to convert to p2 (84) '%s %s'",
					key, field));
				(void) sprintf (buf,
					       "Unable to convert to p2 (84) '%s %s'",
					       key, field);
				*ep = strdup(buf);
				return NOTOK;
			}
			if (hdr_incomplete_copy (&(head -> extensions)) != OK)
				PP_DBG (("Bad incomplete_copy '%s'", field));
			continue;

		    case HDR_LANGUAGE:
			if (x40084 == TRUE) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unable to convert to p2 (84) '%s %s'",
					key, field));
				(void) sprintf (buf,
					       "Unable to convert to p2 (84) '%s %s'",
					       key, field);
				*ep = strdup(buf);
				return NOTOK;
			}
			if (hdr_language (field, &(head -> extensions)) != OK)
				PP_DBG (("Bad language '%s'", field));
			continue;

		    case HDR_SUBJECT:
			if (hdr_subject (field, &(head -> subject)) != OK)
				PP_DBG (("Bad subject '%s'", field));
			continue;

		    case HDR_EXPIRY_DATE:
			if (hdr_expiry_date (field, &(head-> expiry__time)) != OK)
				PP_DBG(("Bad expiry date '%s'", field));
			continue;

		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unexpected Header Type"));
			*ep = strdup("Unexpected Header Type");
			return NOTOK;
		}
		break;
	}

	if (hdr_extns != (struct type_IOB_RFC822FieldList *) NULL) {
		/* header extensions */
		struct type_IOB_ExtensionsField *temp = (struct type_IOB_ExtensionsField *)
			calloc (1, sizeof (struct type_IOB_ExtensionsField));
		temp -> HeadingExtension = (struct type_IOB_HeadingExtension *)
			calloc (1, sizeof (struct type_IOB_HeadingExtension));
		temp -> HeadingExtension -> type = oid_cpy(str2oid(id_rfc_822_field_list));
		PY_pepy[0] = 0;
		if (encode_IOB_RFC822FieldList(&(temp -> HeadingExtension -> value),
					       0, 0, NULLCP, hdr_extns)) {
			PP_OPER (NULLCP,
				 ("failed to encode rfc822 FieldList extension [%s]",
				  PY_pepy));
			oid_free ((char *) temp -> HeadingExtension -> type);
			free ((char *) temp -> HeadingExtension);
			free ((char *) temp);
			return NOTOK;
		} else
			free_IOB_RFC822FieldList(hdr_extns);
		temp -> next = head->extensions;
		head->extensions = temp;
	}

		
	if (head -> originator == (struct type_IOB_ORDescriptor *) 0) {
		if (from != (struct type_IOB_RecipientSequence *) 0) {
			head -> originator = from -> RecipientSpecifier -> recipient;
			free ((char *) from -> RecipientSpecifier);
			temp = from;
			from = from -> next;
			free ((char *) temp);
		}
	}
	
	/* convert RecipientSequence to ORDescriptorSequence */
	while (from != (struct type_IOB_RecipientSequence *) 0) {
		ix = (struct type_IOB_ORDescriptorSequence *) calloc (1,
								     sizeof(struct type_IOB_ORDescriptorSequence));
		ix -> ORDescriptor = from -> RecipientSpecifier -> recipient;
		free((char *) from -> RecipientSpecifier);
		temp=from;
		from=from->next;
		free((char *) temp);
		if (head -> authorizing__users == (struct type_IOB_ORDescriptorSequence *) 0) 
			head -> authorizing__users = tail = ix;
		else {
			tail -> next = ix;
			tail = ix;
		}
	}
	if (head -> this__IPM == NULL)
		gen_mid (&(head -> this__IPM));
	
	return OK;
}

/*  */

hdr_comment (key, field, ep, x40084)
char	*key,
	*field,
	**ep;
int	x40084;
{
	char	buf[BUFSIZ];
	PP_DBG (("hdr_comments '%s'", field));
	if (P2_ext == NULLCP || P2_ext[0] == '\0')
		return OK;
	
	if (!comments_started) {
		comments_started = TRUE;
		if ((fp_P2_ext = fopen (P2_ext, "w")) == NULL) {
			PP_SLOG (LLOG_EXCEPTIONS, P2_ext,
				 ("Can't open file"));
			(void) sprintf (buf,
					"Failed to open extension file '%s'",
					P2_ext);
			*ep = strdup(buf);
			return NOTOK;
		}
		if (TRUE == x40084)
			fputs("RFC-822-HEADERS:\n", fp_P2_ext);
		else
			fputs("Comments:\n", fp_P2_ext);
	}
	field_tidy(key);
	fputs (key, fp_P2_ext);
	fputs (": ", fp_P2_ext);
	field_tidy(field);
	fputs (field, fp_P2_ext);
	fputs ("\n", fp_P2_ext);
	return OK;
}

hdr_mid_seq (field, seq_ptr)
char	*field;
struct type_IOB_IPMIdentifierSequence	**seq_ptr;
{
	char	*p, *q;
	struct type_IOB_IPMIdentifierSequence	*temp, *ix;
	
	field_tidy (field);
	p = q = field;
	while (q != NULLCP)
	{
		if ((q = index (p, ',')) != NULLCP)
			*q++ = '\0';
		temp = (struct type_IOB_IPMIdentifierSequence *)
			calloc (1, sizeof (struct type_IOB_IPMIdentifierSequence));
		if (hdr_mid (p, &(temp -> IPMIdentifier)) != OK) {
			free ((char *) temp);
			return NOTOK;
		}
		p = q;
		if (*seq_ptr == NULL)
			*seq_ptr = temp;
		else {
			ix = *seq_ptr;
			while (ix -> next != NULL) ix = ix -> next;
			ix -> next = temp;
		}
	}
	return OK;
}
		
hdr_mid (field, mid_ptr)
char	*field;
struct type_IOB_IPMIdentifier	**mid_ptr;
{
	/* see rfc 1138 4.7.3.3 */
	char *midstring, *cp;
	char	ps[BUFSIZ];
	ORName	orn;
	AP_ptr	ap, local, domain;

	if ((midstring = index (field, '<')) == NULLCP
	    || (cp = rindex (field, '>')) == NULLCP)
		return NOTOK;
	*cp = '\0';
	midstring++;

	if (ap_s2p (midstring, &ap, (AP_ptr *) 0, (AP_ptr *) 0, &local,
		    &domain, (AP_ptr *) 0) == (char *) NOTOK)
	{
		PP_LOG (LLOG_EXCEPTIONS,
			( "Illegal Message ID '%s'", midstring));
		*mid_ptr = NULL;
		return NOTOK;
	}

	*mid_ptr = (struct type_IOB_IPMIdentifier *) 
		calloc (1, sizeof (struct type_IOB_IPMIdentifier));

	if (domain != NULLAP && local != NULLAP
	    && lexequ (domain -> ap_obvalue, "MHS") == 0
	    && (cp = index (local -> ap_obvalue, '*')) != NULLCP) {
		/* x.400 generated */

		if (cp != local -> ap_obvalue) {
			*cp++ = '\0';
			(*mid_ptr)-> user__relative__identifier = 
				str2qb (local -> ap_obvalue,
					strlen (local -> ap_obvalue),
					1);
		}
		if (*cp == '\0')
			return OK;

		orn.on_or = or_std2or(cp);
		orn.on_dn = NULL;

		ap_1delete(ap);
		ap_free(ap);

		if (orn.on_or != NULLOR) {
			(*mid_ptr) -> user = orn2orname(&orn);
			or_free(orn.on_or);
		}

		return OK;
	} else {
		or_asc2ps(midstring, ps);
		(*mid_ptr) -> user__relative__identifier = str2qb (ps,	
								   strlen(ps),
								   1);
		return OK;
	}
}

get_comments (buf, addr)
char *buf;
AP_ptr addr;
{
	AP_ptr ap;

	buf [0] = '\0';
	ap = addr;
	while (TRUE)
	{
		if (ap -> ap_obtype == AP_COMMENT)
		{
			strcat (buf, " (");
			strcat (buf, ap -> ap_obvalue);
			strcat (buf, ")");
		}
		if (ap -> ap_next == NULLAP
			|| ap -> ap_next -> ap_ptrtype == AP_PTR_NXT)
			break;
		ap = ap -> ap_next;
	}
	PP_DBG (("Comment '%s'", buf));
	return OK;
}
PE or2pe();

#ifdef notdef

static int ORstring2pe (orstring, pe)
char *orstring;
PE *pe;
{
	OR_ptr or;
	*pe = NULLPE;

	if (or_rfc2or (orstring, &or) == NOTOK)
	{
		PP_DBG (("Failed to parse '%s'", orstring));
		return NOTOK;
	}

	or = or_default (or);

	if ((*pe = or2pe (or)) == NULLPE)
	{
		PP_DBG (("Failed to decode OR_Ptr '%s'", orstring));
		return NOTOK;
	}
	or_free (or);
	return OK;
}
#endif

get_ordesc (cpp, or_desc_ptr, group_ptr, x40084)
char	**cpp;
struct type_IOB_ORDescriptor	**or_desc_ptr, **group_ptr;
int	x40084;
{
	/* see rfc 1138 4.7.1 */
	AP_ptr	ap, group, name, local, domain, route, ix;
	char	comment[BUFSIZ], freeform[BUFSIZ], buf[BUFSIZ];
	char	*addr;
	ORName	orn;

	if (*cpp == NULLCP || **cpp == '\0')
		return OK;

	PP_DBG (("get_ordesc ('%s')", *cpp));

	if ((*cpp = ap_s2p (*cpp, &ap,
			    &group, &name, &local,
			    &domain, &route)) == (char *) NOTOK
	    || *cpp == (char *) DONE)
		return NOTOK;

	*or_desc_ptr = (struct type_IOB_ORDescriptor *) 
		calloc (1, sizeof (struct type_IOB_ORDescriptor));

	addr = ap_p2s_nc (NULLAP, NULLAP, local, domain, route);
	
	if (addr == NULLCP || *addr == '\0') {
		orn.on_or = NULLOR;
		orn.on_dn = NULL;
	} else {
		if (or_rfc2or(addr, &(orn.on_or)) == NOTOK) {
			PP_DBG (("Failed to encode '%s'", addr));
			free (addr);
			return NOTOK;
		} else 
			/* maybe try to regain from comments ? */
			orn.on_dn = NULL;

		if (x40084 == TRUE) 
			or_downgrade(&(orn.on_or));
	}

	if (addr)
		free(addr);

	if (orn.on_or != NULLOR) {
		(*or_desc_ptr) -> formal__name = (struct type_IOB_ORName *)
			calloc (1, sizeof (struct type_IOB_ORName));
		(*or_desc_ptr) -> formal__name = orn2orname(&orn);
		or_free (orn.on_or);
	}
	
	get_comments (comment, ap);

	freeform [0] = '\0';
	if (name != NULLAP) {
		ix = name;
		while (ix != NULLAP &&
		       (ix -> ap_obtype == AP_COMMENT ||
			ix -> ap_obtype == AP_PERSON_END ||
			ix -> ap_obtype == AP_PERSON_NAME ||
			ix -> ap_obtype == AP_PERSON_START)) {
			if (ix -> ap_obtype != AP_COMMENT &&
			    ix -> ap_obvalue != NULLCP) {
				if (freeform[0] != '\0')
					strcat (freeform, " ");
				strcat (freeform, ix -> ap_obvalue);
			}
			ix = ix -> ap_next;
		}
	}

	strcat (freeform, comment);
	
	if (freeform[0] != '\0') {
		/* do ascii -> t.61 */
		CHAR8U	*t61str;
		int	len;

		strcpy (buf, freeform);
		
		if ((t61str = asctot61 ((CHAR8U *) buf, &len)) 
		    != (CHAR8U *) NULL) {
			(*or_desc_ptr) -> free__form__name = 
				str2qb (t61str, strlen((char *) t61str), 1);
			free(t61str);
		} else
			(*or_desc_ptr) -> free__form__name = 
				str2qb (buf, strlen(buf), 1);
	}

	if (group != NULLAP) {
		freeform[0] = '\0';
		ix = group;
		while (ix != NULLAP &&
		       (ix -> ap_obtype == AP_COMMENT ||
			ix -> ap_obtype == AP_GROUP_END ||
			ix -> ap_obtype == AP_GROUP_NAME ||
			ix -> ap_obtype == AP_GROUP_START)) {
			if (ix -> ap_obtype != AP_COMMENT
			    && ix -> ap_obvalue != NULLCP) {
				if (freeform[0] != '\0')
					strcat (freeform, " ");
				strcat (freeform, ix -> ap_obvalue);
			}
			ix = ix -> ap_next;
		}
		if (freeform[0] != '\0') {
			CHAR8U	*t61str;
			int	len;

			*group_ptr = (struct type_IOB_ORDescriptor *)
				calloc (1, sizeof (struct type_IOB_ORDescriptor));
			/* do ascii -> t.61 conversion */
			strcpy (buf, freeform);
			if ((t61str = asctot61 ((CHAR8U *) buf, &len)) 
			    != (CHAR8U *) NULL) {
				(*group_ptr) -> free__form__name = 
					str2qb (t61str, strlen((char *)t61str), 1);
				free(t61str);
			} else
				(*group_ptr) -> free__form__name = 
					str2qb (buf, strlen(buf), 1);
		}
	} else
		*group_ptr = NULL;
	ap_1delete(ap);
	ap_free (ap);
	return OK;
}

hdr_ordesc (field, or_desc_ptr, x40084)
char	*field;
struct type_IOB_ORDescriptor	**or_desc_ptr;
int	x40084;
{
	struct type_IOB_ORDescriptor	*gptr;
	char	*cp;
	
	field_tidy (field);
	cp = field;
	if (cp == NULLCP || *cp == '\0')
		return OK;

	if (get_ordesc (&cp, or_desc_ptr, &gptr, x40084) != OK) {
		PP_DBG(("Bad single OR desc '%s'", field));
		return NOTOK;
	}
	return OK;
}

hdr_ordesc_seq (field, or_desc_ptr, x40084)
char	*field;
struct type_IOB_ORDescriptorSequence	**or_desc_ptr;
int	x40084;
{
	char	*cp;
	struct type_IOB_ORDescriptorSequence	*temp, *ix;
	struct type_IOB_ORDescriptor	*dptr, *gptr;

	field_tidy (field);
	cp = field;
	
	if (cp == NULLCP || *cp == '\0')
		return OK;
	
	while (get_ordesc (&cp, &dptr, &gptr, x40084) == OK) {
		if (gptr != (struct type_IOB_ORDescriptor *) NULL) {
			temp = (struct type_IOB_ORDescriptorSequence *)
				calloc (1, sizeof (struct type_IOB_ORDescriptorSequence));
			temp -> ORDescriptor = gptr;
			if (*or_desc_ptr == NULL)
				*or_desc_ptr = temp;
			else {
				ix = *or_desc_ptr;
				while (ix -> next != NULL) ix = ix -> next;
				ix -> next = temp;
			}
		}
		temp = (struct type_IOB_ORDescriptorSequence *)
			calloc (1, sizeof (struct type_IOB_ORDescriptorSequence));
		temp -> ORDescriptor = dptr;
		if (*or_desc_ptr == NULL)
			*or_desc_ptr = temp;
		else {
			ix = *or_desc_ptr;
			while (ix -> next != NULL) ix = ix -> next;
			ix -> next = temp;
		}
		if (cp == NULLCP || *cp == '\0')
			return OK;
	}
	PP_DBG (("Bad address field '%s'", field));
	return NOTOK;
}

hdr_recip_set (field, or_recip_ptr, x40084)
char	*field;
struct type_IOB_RecipientSequence	**or_recip_ptr;
int	x40084;
{
	/* see rfc 1138 4.7.1 */
	struct type_IOB_RecipientSequence	*temp, *ix;
	char	*cp;
	struct type_IOB_ORDescriptor	*dptr, *gptr;

	field_tidy (field);
	cp = field;

	if (cp == NULLCP || *cp == '\0')
		return OK;
	
	while (get_ordesc (&cp, &dptr, &gptr, x40084) == OK) {
		if (gptr != (struct type_IOB_ORDescriptor *) NULL) {
			temp = (struct type_IOB_RecipientSequence *)
				calloc (1, sizeof (struct type_IOB_RecipientSequence));
			temp -> RecipientSpecifier = (struct type_IOB_RecipientSpecifier *)
				calloc (1, sizeof(struct type_IOB_RecipientSpecifier));
			temp -> RecipientSpecifier -> recipient = gptr;
			/* defaults for other fields == 0 */
			if (*or_recip_ptr == NULL)
				*or_recip_ptr = temp;
			else {
				ix = *or_recip_ptr;
				while (ix -> next != NULL) ix = ix -> next;
				ix -> next = temp;
			}
		}
		if (dptr != (struct type_IOB_ORDescriptor *) NULL
		    && (dptr->formal__name != (struct type_IOB_ORName *) NULL
			|| dptr->free__form__name != (struct type_IOB_FreeFormName *) NULL)) {
			temp = (struct type_IOB_RecipientSequence *)
				calloc (1, sizeof (struct type_IOB_RecipientSequence));
			temp -> RecipientSpecifier = (struct type_IOB_RecipientSpecifier *)
				calloc (1, sizeof(struct type_IOB_RecipientSpecifier));
			temp -> RecipientSpecifier -> recipient = dptr;
			/* defaults for other fields == 0 */
			if (*or_recip_ptr == NULL)
				*or_recip_ptr = temp;
			else {
				ix = *or_recip_ptr;
				while (ix -> next != NULL) ix = ix -> next;
				ix -> next = temp;
			}
		}
		if (cp == NULLCP || *cp == '\0')
			return OK;
	}
	PP_DBG (("Bad Address field '%s'", field));
	return NOTOK;
}

hdr_reply_to (field, replied_ptr, related_ptr)
char	*field;
struct type_IOB_IPMIdentifier	**replied_ptr;
struct type_IOB_IPMIdentifierSequence	**related_ptr;
{
	char	*start, *end, savech;
	struct type_IOB_IPMIdentifierSequence	*temp, *ix;
	struct type_IOB_IPMIdentifier	*msgid;
	
	if ((start = index(field, '<')) == NULLCP)
		/* no message id can't do anything */
		return OK;
	
	while (*(start+1) != '\0' && *(start+1) == '<') start++;

	if ((end = index(start, '>')) == NULLCP)
		/* incomplete msgid can't do anything */
		return OK;
	end++;
	savech = *end;
	*end = '\0';
	if (hdr_mid (start, &msgid) != OK) 
		return NOTOK;
	*end = savech;

	if (*end == '\0'
	    || (start = index(end, '<')) == NULLCP) 
		/* only one so put in replied_ptr */
		*replied_ptr = msgid;
	else {
		/* more than one put in related_ptr */
		temp = (struct type_IOB_IPMIdentifierSequence *)
			calloc (1, sizeof(struct type_IOB_IPMIdentifierSequence));
		temp -> IPMIdentifier = msgid;
		
		if (*related_ptr == NULL)
			*related_ptr = temp;
		else {
			ix = *related_ptr;
			while (ix -> next != NULL) ix = ix -> next;
			ix -> next = temp;
		}

		while (end != NULLCP && *end != '\0') {

			if ((start = index(end, '<')) == NULLCP)
				/* no message id can't do anything */
				return OK;
			while (*(start+1) != '\0' && *(start+1) == '<') start++;
			if ((end = index(start, '>')) == NULLCP)
				/* incomplete msgid can't do anything */
				return OK;
			temp = (struct type_IOB_IPMIdentifierSequence *)
				calloc (1, sizeof(struct type_IOB_IPMIdentifierSequence));
			end++;
			savech = *end;
			*end = '\0';
			if (hdr_mid (start, &(temp -> IPMIdentifier)) != OK) {
				free(temp);
				return OK;
			}
			*end = savech;
			if (*related_ptr == NULL)
				*related_ptr = temp;
			else {
				ix = *related_ptr;
				while (ix -> next != NULL) ix = ix -> next;
				ix -> next = temp;
			}
			
		}			
	}
	return OK;
}

hdr_extension (key, field, extensionptr)
char	*field, *key;
struct type_IOB_RFC822FieldList	**extensionptr;
{
	char	buf[BUFSIZ];
	struct type_IOB_RFC822FieldList *temp = (struct type_IOB_RFC822FieldList *)
		calloc(1, sizeof(*temp));
	struct type_IOB_RFC822Field 	*hdr;
	if (field[strlen(field)-1] == '\n')
		field[strlen(field)-1] = '\0';
	(void) sprintf (buf, "%s: %s", key, field);
	temp->RFC822Field = str2qb (buf, strlen(buf), 1);
	temp->next = *extensionptr;
	*extensionptr = temp;
	return OK;
}

hdr_incomplete_copy (extensionptr)
struct type_IOB_ExtensionsField	**extensionptr;
{
	struct type_IOB_ExtensionsField *temp = (struct type_IOB_ExtensionsField *)
		calloc (1, sizeof (struct type_IOB_ExtensionsField));
	temp -> HeadingExtension = (struct type_IOB_HeadingExtension *)
		calloc (1, sizeof (struct type_IOB_HeadingExtension));

	PY_pepy[0] = 0;

	if (encode_IOB_IncompleteCopy (&(temp -> HeadingExtension -> value),
				      0, 0, NULLCP, 
				      (struct type_IOB_IncompleteCopy *) NULL) != OK)
	{
		PP_OPER (NULLCP,
			 ("failed to encode incomplete copy value [%s]",
			  PY_pepy));
		free ((char *)temp -> HeadingExtension);
		free ((char *)temp);
		return NOTOK;
	}

	temp -> HeadingExtension -> type = oid_cpy(str2oid(id_hex_incomplete_copy));
	
	temp -> next = *extensionptr;
	*extensionptr = temp;
	return OK;
}
	
hdr_language (field, extensionptr)
char	*field;
struct type_IOB_ExtensionsField	**extensionptr;
{
	struct type_IOB_Languages	*lang = (struct type_IOB_Languages *)
		calloc (1, sizeof (struct type_IOB_Languages));

	struct type_IOB_ExtensionsField *temp = (struct type_IOB_ExtensionsField *)
		calloc (1, sizeof (struct type_IOB_ExtensionsField));

	temp -> HeadingExtension = (struct type_IOB_HeadingExtension *)
		calloc (1, sizeof (struct type_IOB_HeadingExtension));

	while (isspace (*field)) field++;

	lang -> Language = str2qb(field, strlen(field), 1);

	PY_pepy[0] = 0;

	if (encode_IOB_Languages (&(temp -> HeadingExtension -> value),
				 0,
				 0,
				 NULLCP,
				 lang) != OK)
	{
		PP_OPER (NULLCP,
			 ("failed to encode language heading extension [%s]",
			  PY_pepy));
		free ((char *)lang);
		free ((char *) temp -> HeadingExtension);
		free ((char *) temp);
		return NOTOK;
	}

	temp -> HeadingExtension -> type = oid_cpy(str2oid(id_hex_languages));

	temp -> next = *extensionptr;
	*extensionptr = temp;

	return OK;
}

static CHARSET	*ia5 = (CHARSET *) NULL;
static CHARSET	*t61 = (CHARSET *) NULL;

/* needed for odd behaviour of keld's code */
#define FUDGE 10

static CHAR8U *asctot61 (ia5str, plen)
CHAR8U	*ia5str;
int 	*plen;
{
	CHAR8U	*t61str;
	
	if (ia5 == (CHARSET *) NULL)
		ia5 = getchset("CCITT_T.50.irv:1988", (INT16S) 29);
	if (t61 == (CHARSET *) NULL)
		t61 = getchset("T.61-8bit", (INT16S) 29);

	if (ia5 != (CHARSET *) NULL
	    && t61 != (CHARSET *) NULL) {
		t61str = (CHAR8U *) malloc(sizeof(CHAR8U) * (strlen((char *)ia5str)+1));
		if ((*plen = strncnv (t61, ia5, 
				      t61str, ia5str, 
				      strlen((char *) ia5str), FALSE) >= 0))
			return t61str;
	} 
	return (CHAR8U *) NULL;
}
	
hdr_subject (field, subject_ptr)
char	*field;
struct type_IOB_SubjectField	**subject_ptr;
{
	CHAR8U	*t61str;
	int	len;
	while (isspace (*field)) field++;
	
	if (*field == '\0') {
		*subject_ptr = NULL;
		return OK;
	}
	if (*(field+strlen(field)-1) == '\n')
		*(field+strlen(field)-1) = '\0';

	if ((t61str = asctot61((CHAR8U *) field, &len)) != (CHAR8U *) NULL) {
		*subject_ptr = str2qb(t61str, strlen((char *)t61str), 1);
		free(t61str);
	} else
		*subject_ptr = str2qb (field, strlen(field), 1);
	return OK;
}

hdr_expiry_date(field, expiry)
char	*field;
struct qbuf **expiry;
{
	UTC	ut;
	char	*str;
	if (rfc2UTC (field, &ut) != OK)
		return NOTOK;
	str = utct2str(ut);
	*expiry = str2qb(str, strlen(str), 1);
	return OK;
}


hdr_reply_by(field, reply)
char	*field;
struct qbuf **reply;
{
	UTC	ut;
	char	*str;
	if (rfc2UTC (field, &ut) != OK)
		return NOTOK;
	str = utct2str(ut);
	*reply = str2qb(str, strlen(str), 1);
	return OK;
}
	

/*  */

static char *hdrbuf;
static int hdrbuflen;
static char linebuf [LINESIZE];

static CMD_TABLE hdr_list[] = {
"autoforwarded",	HDR_AUTOFORWARDED,
"bcc",          HDR_BCC,
"cc",           HDR_CC,
"comments",	HDR_COMMENT,
"content-identifier",	HDR_IGNORE,
"conversion",	HDR_IGNORE,
"conversion-with-loss",	HDR_IGNORE,
"date",         HDR_IGNORE,
"deferred-delivery",	HDR_IGNORE,
"discarded-x400-ipms-extensions", HDR_IGNORE,
"discarded-x400-mts-extensions", HDR_IGNORE,
"dl-expansion-history",	HDR_IGNORE,
"expiry-date",	HDR_EXPIRY_DATE,
"from",         HDR_FROM,
"generate-delivery-report",	HDR_IGNORE,
"importance",	HDR_IMPORTANCE,
"in-reply-to",  HDR_IN_REPLY_TO,
"incomplete-copy", HDR_INCOMPLETE_COPY,
"language",	HDR_LANGUAGE,
"latest-delivery-time",	HDR_IGNORE,
"message-id",   HDR_MID,
"message-type",	HDR_IGNORE,
"obsoletes",	HDR_OBSOLETES,
"original-encoded-information-types",	HDR_IGNORE,
"prevent-nondelivery-report",	HDR_IGNORE,
"priority",	HDR_IGNORE,
"received",     HDR_IGNORE,
"references",   HDR_REFERENCES,
"reply-by",	HDR_REPLY_BY,
"reply-to",     HDR_REPLY_TO,
"sender",       HDR_SENDER,
"sensitivity",	HDR_SENSITIVITY,
"subject",      HDR_SUBJECT,
"to",           HDR_TO,
"via",          HDR_IGNORE,
"x400-content-type", 	HDR_IGNORE,
"x400-mts-identifier",	HDR_IGNORE,
"x400-originator",	HDR_IGNORE,
"x400-received",	HDR_IGNORE,
(char *) 0,             HDR_EXTENSIONS};

#define N_HDRS ((sizeof(hdr_list)/sizeof(CMD_TABLE)) - 1)

get_hdr (key, field)
char **key;
char **field;
{
	char *cp;
	int hval;

PP_DBG (("get_hdr()"));

	if (first_hdr)
	{
		first_hdr = FALSE;
		if (fgets (linebuf, LINESIZE - 1, fp_rfc_in) == NULL)
			return HDR_ERROR;
		hdrbuflen = LINESIZE;
		hdrbuf =  malloc (LINESIZE);
		if (hdrbuf == NULL)
			return HDR_ERROR;
	}

	if (linebuf [0] == '\0')
		return HDR_EOH;

	hdrbuf [0] = '\0';
	strcat (hdrbuf, linebuf);
	linebuf [0] = '\0';

	while (!feof (fp_rfc_in))
	{
		if ((fgets (linebuf, LINESIZE -1, fp_rfc_in) == NULL)
				&& !feof (fp_rfc_in))
			return HDR_ERROR;
		if (!isspace (linebuf[0]))
			break;
					/* continuation */
		if ((int)strlen (linebuf) <= 1)
		{
			PP_DBG (("Header terminated before EOF"));
			linebuf [0] = '\0';
			break;
		}

		if (hdrbuflen < (int)strlen (hdrbuf) + (int)strlen (linebuf))
		{       
			hdrbuflen = hdrbuflen * 2;
					/* linebuf must be shorter */
			hdrbuf = realloc (hdrbuf, (unsigned int) hdrbuflen);
			if (hdrbuf == NULL)
			{
				PP_LOG (LLOG_EXCEPTIONS, ("Malloc failed"));
				return NOTOK;
			}
		}
		strcat (hdrbuf, linebuf);
	}

	if (feof (fp_rfc_in))
		/* set end of file condition ready for next call */
		linebuf[0] = '\0';
	
	if ((cp = index (hdrbuf, ':')) == NULLCP)
	{
		PP_DBG (("Header without key '%s'", hdrbuf));
		*key = hdrbuf;
		*field = hdrbuf;
		return HDR_ERROR;
	}

	*cp++ = '\0';
	*key = hdrbuf;
	*field = cp;
	field_tidy (*key);


	hval = cmdbsrch(*key, hdr_list, N_HDRS);
	PP_DBG (("Header: '%d' '%s' '%s'", hval,
		*key,  *field));
	return hval;
}

field_tidy (field)
char *field;
{       /* strip newlines, leading/trailing space */
	register char *p, *q;

	p = field;
	q = field;


	while (isspace(*p))
		p++;

	while (*p != '\0')
		if (*p != '\n')
			*q++ = *p++;
		else
			p++;
	*q-- = '\0';

	while (isspace(*q))
		*q-- = '\0';
}       

extern char	*loc_dom_site;

gen_mid (mid_ptr)               /* generate  random mailid */
struct type_IOB_IPMIdentifier **mid_ptr;
{
        char buf [LINESIZE];
        long now;
        (void) time (&now);
        sprintf (buf, "<\"%ld %s", getpid(), ctime (&now));
        buf [strlen(buf) - 1] = '\0';
        strcat (buf, "\"@");
        strcat (buf, loc_dom_site);
        strcat (buf, ">");
        return (hdr_mid (buf, mid_ptr));
}

hdr_importance (field, pparm)
char	*field;
int	*pparm;
{
	compress(field, field);
	
	if (lexequ(field, "low") == 0)
		*pparm = int_IOB_ImportanceField_low;
	else if (lexequ(field, "normal") == 0)
		*pparm = int_IOB_ImportanceField_normal;
	else if (lexequ (field, "high") == 0)
		*pparm = int_IOB_ImportanceField_high;
	else
		return NOTOK;
	return OK;
}

hdr_sensitivity (field, psensitivity)
char	*field;
struct type_IOB_SensitivityField	**psensitivity;
{
	int	parm;

	compress(field, field);

	if (lexequ(field, "personal") == 0)
		parm = int_IOB_SensitivityField_personal;
	else if (lexequ(field, "private") == 0)
		parm = int_IOB_SensitivityField_private;
	else if (lexequ(field, "company-confidential") == 0)
		parm = int_IOB_SensitivityField_company__confidential;
	else
		return NOTOK;

	*psensitivity = (struct type_IOB_SensitivityField *) 
		calloc (1, sizeof(struct type_IOB_SensitivityField));
	(*psensitivity)->parm = parm;
	return OK;
}

hdr_autoforwarded (field, pauto)
char	*field;
struct type_IOB_AutoForwardedField	**pauto;
{
	int	parm;

	compress(field, field);

	if (lexequ(field, "true") == 0)
		parm = 1;
	else if (lexequ(field, "false") == 0)
		parm =0;
	else 
		return NOTOK;
	
	*pauto = (struct type_IOB_AutoForwardedField *)
		calloc(1, sizeof(struct type_IOB_AutoForwardedField));
	(*pauto)->parm = parm;
	
	return OK;
}
