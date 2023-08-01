/* rfc822_dnorm.c: normalise an 822 domain */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_dnorm.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/rfc822_dnorm.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: rfc822_dnorm.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */
#include "head.h"
#include "ap.h"
#include "chan.h"

extern char	*bits2str();

rfc822_norm_dmn (dmn, dmnorder)
register AP_ptr		dmn;
int			dmnorder;
{
	int	retval;
	char	official[LINESIZE],
		mta_key[LINESIZE],
		tbuf[BUFSIZ],
		*one_ord[LINESIZE],
		*other_ord[LINESIZE];

	if (dmn == NULL)
		return NOTOK;
	

	/* hardwire in local mta and site */
	(void) strcpy (tbuf, dmn->ap_obvalue);
	(void) str2bits (tbuf, '.', one_ord, other_ord);

	retval = tb_getdomain(dmn->ap_obvalue,
			      mta_key,
			      official,
			      dmnorder,
			      &(dmn->ap_localhub));

	dmn->ap_normalised = TRUE;
	if (retval == OK) {

		dmn->ap_recognised = TRUE;
		if (official[0]) {
			free (dmn->ap_obvalue);
			dmn->ap_obvalue = strdup(official);
			PP_DBG(("rfc822_norm_dmn Official = '%s'",
				official));
		}
		if (mta_key[0]) {
			if (dmn->ap_chankey)
				free(dmn->ap_chankey);
			dmn->ap_chankey = strdup(mta_key);
			PP_DBG(("rfc822_norm_dmn channel key = '%s'",
				mta_key));
		}
		if (dmn->ap_localhub != NULLCP)
			dmn->ap_islocal = TRUE;
		else
			dmn->ap_islocal = FALSE;

	} else {

		dmn->ap_recognised = FALSE;
		(void) sprintf(official,
			       "Unknown domain '%s'",
			       dmn->ap_obvalue);
		dmn->ap_error = strdup(official);
		PP_LOG (LLOG_EXCEPTIONS,
			("%s is not a known domain reference",
			 dmn->ap_obvalue));

	}

	return retval;
}
