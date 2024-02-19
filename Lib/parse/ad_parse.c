/* ad_parse.c: parse an address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/ad_parse.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/ad_parse.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: ad_parse.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"
#include <stdarg.h>

static void ad_copy_addresses(), 
	ad_copy_errors();

static int rfc822thenx40090, x400thenrfc822(), parselose_exception();
static int rfc822thenx400 ();

int ad_fullparse (ad, rp, order_pref)
register ADDR	*ad;
RP_Buf		*rp;
int		order_pref;
{
	ad->aparse->full = OK;
	ad->aparse->dmnorder = order_pref;
	
	return ad_parse_aux (ad, rp);
}

int ad_parse (ad, rp, order_pref)
register ADDR	*ad;
RP_Buf		*rp;
int		order_pref;
{
	ad->aparse->full = NOTOK;
	ad->aparse->dmnorder = order_pref;

	return ad_parse_aux (ad, rp);
}

/*  */

int	ad_parse_aux (ad, rp)
register ADDR	*ad;
RP_Buf		*rp;
{
	int	is822, retval;
	PP_DBG(("ad_parse ('%s', '%d')",
		ad->ad_value,
		ad->ad_type));

	or_myinit();

	switch (ad->ad_type) {
	    case AD_X400_TYPE:
		return x400thenrfc822 (ad, rp);
	    case AD_822_TYPE:
		return rfc822thenx400 (ad, rp);
	    default:

		/* guess */
		/* shouldn't ever be call */
		is822 = TRUE;
		if (index(ad->ad_value, '/') != NULLCP)
			is822 = FALSE;
		if (index(ad->ad_value, '@') != NULLCP)
			is822 = TRUE;
		
		if (is822 == TRUE) {
			if (rp_isbad(retval =
				     rfc822thenx400(ad, rp))) {
				/* may have guessed wrong */
				if (ad->ad_r400adr) {
					free(ad->ad_r400adr);
					ad->ad_r400adr = NULLCP;
				}

				if (ad->ad_r822adr) {
					free (ad->ad_r822adr);
					ad->ad_r822adr = NULLCP;
				}
				if (ad->ad_parse_message) {
					free(ad->ad_parse_message);
					ad->ad_parse_message = NULLCP;
				}

				return x400thenrfc822(ad, rp);
			}
		} else {
			if (rp_isbad(retval = 
				     x400thenrfc822(ad, rp))) {
				/* wrong guess */
				if (ad->ad_r400adr) {
					free(ad->ad_r400adr);
					ad->ad_r400adr = NULLCP;
				}

				if (ad->ad_r822adr) {
					free (ad->ad_r822adr);
					ad->ad_r822adr = NULLCP;
				}
				if (ad->ad_parse_message) {
					free(ad->ad_parse_message);
					ad->ad_parse_message = NULLCP;
				}
				return rfc822thenx400(ad, rp);
			}
		}
		return retval;
	}
}

/*  */
extern char	or_error[];
#define MAX_LOOP	10

static int rfc822thenx400 (ad, rp)
register ADDR	*ad;
RP_Buf		*rp;
{
	int	retval, cont, count;
	char	*prev822adr, logerr[BUFSIZ];

	PP_DBG (("rfc822thenx400 ('%s', '%d')",
		 ad->ad_value, ad->ad_type));

	cont = TRUE;
	count = 0;
	/* parse 822 */
	if (rp_isbad (retval =
		      rfc822_parse (ad))) {
		ad_copy_errors(ad, logerr, TRUE);
		return parselose_exception (rp,
					    retval,
					    logerr,
					    ad->ad_parse_message);
	}
	do {
		/* validate 822 */
		ad->aparse->ad_type = AD_822_TYPE;
		retval = rfc822_validate(ad, rp);

		/* convert 822 to x400 */
		or_error[0] = '\0';
		if (rp_isbad(rfc822_x400 (ad))) {
			if (ad->ad_parse_message == NULLCP) 
				ad_copy_errors (ad, logerr, FALSE);
			ad_copy_addresses (ad, FALSE);
			return parselose_exception(rp,
						   retval,
						   logerr,
						   ad->ad_parse_message);
		}

		if (!rp_isbad(retval)) {
			/* 822 is okay */
			ad_copy_addresses (ad, TRUE);
			return (ad->ad_parse_stat = RP_AOK);
		}

		/* validate x400 */
		or_error[0] = '\0';
		ad->aparse->ad_type = AD_X400_TYPE;
		retval = x400_validate(ad, rp);

		/* save first forms of address in main adr struct */
		if (ad->ad_r822adr == NULLCP
		    || ad->ad_r400adr == NULLCP)
			ad_copy_addresses(ad, FALSE);

		/* save first error messages in main adr struct */
		if (ad->ad_parse_message == NULLCP)
			ad_copy_errors(ad, logerr, FALSE);
		

		/* reconvert back to 822 and see if different */
		if (isstr(ad->aparse->r822_str))
			prev822adr = strdup(ad->aparse->r822_str);
		else
			prev822adr = NULLCP;
		
		if (rp_isbad(x400_rfc822(ad))
		    || !isstr(ad->aparse->r822_str)
		    || prev822adr == NULLCP
		    || lexequ(ad->aparse->r822_str, prev822adr) == 0
		    || lexequ(ad->aparse->r822_str, ad->ad_value) == 0)
			cont = FALSE;
		else 
			count++;
		if (prev822adr) free(prev822adr);

		if (!rp_isbad(retval)) {
			/* x400 is okay */
			ad_copy_addresses(ad, TRUE);
			return (ad->ad_parse_stat = RP_AOK);
		}

	} while (count < MAX_LOOP && cont == TRUE);

	ad->aparse->ad_type = AD_822_TYPE;
	return parselose_exception(rp,
				   retval,
				   logerr,
				   ad->ad_parse_message);
}

/*  */

static int x400thenrfc822 (ad, rp)
register ADDR	*ad;
RP_Buf		*rp;
{
	int	retval, cont, count;
	char	*prevx400adr, logerr[BUFSIZ];
	
	cont = TRUE;
	count = 0;

	PP_DBG (("x400thenrfc822 ('%s', '%d')",
		 ad->ad_value, ad->ad_type));
	
	/* parse x400 */
	or_error[0] = '\0';
	if (rp_isbad(retval = x400_parse(ad))) {
		ad_copy_errors(ad, logerr, TRUE);
		return parselose_exception (rp,
					    retval,
					    logerr,
					    ad->ad_parse_message);
	}
	do {

		/* validate x400 */
		or_error[0] = '\0';
		ad->aparse->ad_type = AD_X400_TYPE;
		retval = x400_validate(ad, rp);

		/* convert x400 to rfc */
		
		if (rp_isbad(x400_rfc822 (ad))) {
			if (ad->ad_parse_message == NULLCP)
				ad_copy_errors(ad, logerr, FALSE);
			ad_copy_addresses(ad, FALSE);
			return parselose_exception(rp,
						   retval,
						   logerr,
						   ad->ad_parse_message);
		}

		if (!rp_isbad(retval)) {
			/* x400 is okay */
			ad_copy_addresses(ad, TRUE);
			return (ad->ad_parse_stat = RP_AOK);
		}
		
		/* validate 822 */
		ad->aparse->ad_type = AD_822_TYPE;
		retval = rfc822_validate(ad, rp);
		
		/* save first form of address in main adr struct */
		if (ad->ad_r822adr == NULLCP
		    || ad->ad_r400adr == NULLCP)
			ad_copy_addresses(ad, FALSE);
		
		/* save first form of address in main adr struct */
		if (ad->ad_parse_message == NULLCP)
			ad_copy_errors(ad, logerr, FALSE);
		
		/* reconvert back to x400 and see if different */
		if (isstr(ad->aparse->x400_str))
			prevx400adr = strdup(ad->aparse->x400_str);
		else
			prevx400adr = NULLCP;
		
		or_error[0] = '\0';
		if (rp_isbad (rfc822_x400 (ad))
		    || !isstr(ad->aparse->x400_str)
		    || prevx400adr == NULLCP
		    || lexequ (ad->aparse->x400_str, prevx400adr) == 0
		    || lexequ (ad->aparse->x400_str, ad->ad_value) == 0)
			cont = FALSE;
		else
			/* lets go round again */
			count++;
			
		if (prevx400adr)
			free(prevx400adr);

		if (!rp_isbad(retval)) {
			/* 822 is okay */
			ad_copy_addresses(ad, TRUE);
			return (ad->ad_parse_stat = RP_AOK);
		}
	} while (count < MAX_LOOP && cont == TRUE);

	ad->aparse->ad_type = AD_X400_TYPE;

	return parselose_exception(rp,
				   retval,
				   logerr,
				   ad->ad_parse_message);
}

/*  */

/* copy addresses from ad->aparse to ad->ad_r*adr */
/* overwriting depending on setting of force */

static void ad_copy_r822_address(ad, force)
register ADDR	*ad;
int		force;
{
	if (!ad->aparse->r822_str)
		return;
	if (ad->ad_r822adr) {
		if (force != TRUE)
			return;
		free(ad->ad_r822adr);
	}
	ad->ad_r822adr = strdup(ad->aparse->r822_str);
}

static void ad_copy_x400_address(ad, force)
register ADDR	*ad;
int		force;
{
	if (!ad->aparse->x400_str)
		return;
	if (ad->ad_r400adr) {
		if (force != TRUE)
			return;
		free(ad->ad_r400adr);
	}
	ad->ad_r400adr = strdup(ad->aparse->x400_str);
}

static void ad_copy_addresses(ad, force)
register ADDR	*ad;
int		force;
{
	ad_copy_r822_address(ad, force);
	ad_copy_x400_address(ad, force);
}

/* concat errors from ad->aparse to ad->ad_parse_message */
/* overwriting depending on setting of force */

static void ad_copy_errors(ad, old, force)
register ADDR	*ad;
char	*old;
int	force;
{
	char	*str;
	if (ad->aparse->x400_error == NULLCP
	    && ad->aparse->r822_error == NULLCP)
		return;

	if (ad->ad_type == AD_X400_TYPE) {
		if (ad->aparse->x400_error)
			str = ad->aparse->x400_error;
		else 
			str = ad->aparse->r822_error;
	} else {
		if (ad->aparse->r822_error)
			str = ad->aparse->r822_error;
		else
			str = ad->aparse->x400_error;
	}

	if (ad->ad_parse_message) {
		if (force != TRUE)
			return;
		if (str)
			free(ad->ad_parse_message);
	}
	
	ad->ad_parse_message = strdup(str);

	if (old == NULLCP) 
		return;

	if (ad->aparse->r822_error
	    && ad->aparse->x400_error) {
		/* both errors */
		if (lexequ(ad->aparse->x400_error,
			   ad->aparse->r822_error) == 0) {
			(void) sprintf(old, "%s", ad->aparse->r822_error);
			return;
		}
		
		if (ad->ad_type == AD_X400_TYPE) 
			(void) sprintf(old,
				       "%s (%s)",
				       ad->aparse->x400_error,
				       ad->aparse->r822_error);
		else
			(void) sprintf(old,
				       "%s (%s)",
				       ad->aparse->r822_error,
				       ad->aparse->x400_error);
		return;
	}

	if (ad->aparse->r822_error) 
		(void) sprintf(old, "%s", ad->aparse->r822_error);
	else if (ad->aparse->x400_error)
		(void) sprintf(old, "%s", ad->aparse->x400_error);
}

/*  */

set_error (ad, buf)
register ADDR	*ad;
char		*buf;
{
	char	**perr;
	if (ad->aparse->ad_type == AD_X400_TYPE) 
		perr = &(ad->aparse->x400_error);
	else
		perr = &(ad->aparse->r822_error);
	
	if (*perr)
		free(*perr);
	
	*perr = strdup(buf);
}

/*  */

static int parselose_exception (rp, val, logstr, str)
RP_Buf  *rp;
int     val;
char	*logstr;
char    *str;
{
	int	minlen;

	PP_LOG (LLOG_EXCEPTIONS, 
		("parselose (%s, '%s')", rp_valstr (val), logstr));

	rp -> rp_val = val;
	
	if (isstr(str)) {
		minlen = (strlen(str) < sizeof (rp -> rp_line) - 1) ?
			strlen(str) : sizeof (rp -> rp_line) - 1;
		(void) strncpy (rp -> rp_line,  str, minlen);
		rp -> rp_line[minlen] = '\0';
	} else
		(void) strcpy (rp -> rp_line,
			       "ERROR");
					

	return val;
}

int parselose (RP_Buf *rp, int val, char *str, ...)
{
	va_list ap;
	char    buf[BUFSIZ];

	va_start (ap, str);

	_asprintf (buf, NULLCP, str, ap);

	PP_LOG (LLOG_TRACE, 
		("parselose (%s, '%s')", rp_valstr (val), buf));

	rp -> rp_val = val;
	(void) strncpy (rp -> rp_line, buf, sizeof (rp -> rp_line) - 1);

	va_end (ap);

	return val;
}
