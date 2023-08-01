/* x400_validate.c: validate an x400 address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/x400_val.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/x400_val.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: x400_val.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"
#include "or.h"

extern char or_error[];

static int x400_val_mta(), x400_val_local();

int x400_validate(ad, rp)
register ADDR	*ad;
RP_Buf		*rp;
{
	char	tbuf[BUFSIZ], *replace;
	int	type;

#ifdef	NOTNEEDED
	/* should have already been done when on_or was being set up */
	if (or_checktypes (ad->aparse->orname->on_or,
			   tmp) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Bad OR address '%s'", tmp));
		if (ad->aparse->x400_error)
			free(ad->aparse->x400_error);
		ad->aparse->x400_error = strdup(tmp);
		return RP_BAD;
	}
#endif

	if (or_check (&(ad->aparse->orname->on_or),
		      tbuf, &type,
		      &(ad->aparse->local_hub_name),
		      &replace, 
		      &(ad->aparse->lastMatch)) == NOTOK) {
		PP_DBG (("x400_validate OR_check failed '%s'",
			 or_error));
		if (ad->aparse->x400_error)
			free(ad->aparse->x400_error);
		ad->aparse->x400_error = strdup(or_error);
		return RP_BAD;
	}

	PP_DBG (("x400_validate (tbuf=%s, type=%d)",
		 tbuf, type));

	if (replace != NULLCP) 
		free(replace);

	fillin_400_str(ad);

	switch (type) {
	    case OR_ISLOCAL:
		return x400_val_local(ad,tbuf,rp);
	    case OR_ISMTA:
		return x400_val_mta(ad,tbuf,rp);
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Illegal return from or_check '%d'", type));
		return RP_MECH;
	}
}

/*  */

static int x400_val_mta (ad, name, rp)
register ADDR	*ad;
char		*name;
RP_Buf		*rp;
{
	char	tmp[BUFSIZ];
	LIST_RCHAN	*rlp;

	/* not local so remove local if there */
	if (ad->aparse->x400_local) {
		free(ad->aparse->x400_local);
		ad->aparse->x400_local = NULLCP;
	}

	/* -- look up the channel table -- */
	if (ad->ad_outchan == NULLIST_RCHAN)
		if (tb_getchan (name,
				&(ad->ad_outchan)) != OK) {
			if (ad->aparse->x400_error)
				free(ad->aparse->x400_error);

			(void) sprintf(tmp,
				       "Unknown domain '%s'",
				       name);
			PP_NOTICE(("No x400 channel regisitered for '%s'",
				   name));
			ad->aparse->x400_error = strdup(tmp);
		}

	for (rlp=ad->ad_outchan;rlp;rlp=rlp->li_next)
		if (rlp->li_mta == NULLCP)
			rlp->li_mta = strdup(name);

	if (ad->ad_outchan == NULLIST_RCHAN) {
		(void) strcpy (rp -> rp_line, ad -> aparse->x400_error);
		return rp -> rp_val = RP_BAD;
	}
	return RP_AOK;
}

/*  */

static int x400_val_local (ad, name, rp)
register ADDR	*ad;
char		*name;
RP_Buf		*rp;
{
	char	tmp[BUFSIZ];
	OR_ptr	local;
	

	if (ad->aparse->lastMatch != NULLOR 
	    && (local = ad->aparse->lastMatch->or_next) != NULLOR) {
		/* try first looking up with all components */
		/* including DDAs */
		int	retval;
		char	*saveerror = ad->aparse->x400_error;
		ad->aparse->x400_error = NULLCP;
		local -> or_prev = NULLOR;
		or_or2std(local, tmp, FALSE);
		if (ad->aparse->x400_local) 
			free (ad->aparse->x400_local);
		ad->aparse->x400_local = strdup(tmp);
		local -> or_prev = ad->aparse->lastMatch;
		
		if (!rp_isbad(retval = ad_local(tmp, ad, rp)))
			return retval;
		if (ad->aparse->x400_error)
			free(ad->aparse->x400_error);
		ad->aparse->x400_error = saveerror;
	}

	if (ad->aparse->x400_local) 
		free (ad->aparse->x400_local);
	if (or_getpn(ad->aparse->orname->on_or,
		     tmp) == TRUE)
		ad->aparse->x400_local = strdup(tmp);
	else
		ad->aparse->x400_local = NULLCP;

	return ad_local(name, ad, rp);
}

/*  */

int fillin_400_str (ad)
register ADDR	*ad;
{
	char	tbuf[BUFSIZ];

	if (ad->aparse->x400_str) free(ad->aparse->x400_str);
	
	or_or2std(ad->aparse->orname->on_or, tbuf, FALSE);
	
	ad->aparse->x400_str = strdup(tbuf);
}
