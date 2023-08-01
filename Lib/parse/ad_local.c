/* ad_local.c: process a local address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/ad_local.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/ad_local.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: ad_local.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "adr.h"
#include "alias.h"

extern char	*loc_dom_mta;
extern LIST_RCHAN	*tb_getuser();
extern Redirection *redirect_new();
extern void redirect_add();
extern void redirect_free ();

static int do_alias_check(), do_user_check(), reparse_local();

int ad_local (locname, ad, rp)
char	*locname;
register ADDR	*ad;
RP_Buf	*rp;
{
	ALIAS	alias_struct,
	*alp = &alias_struct;
	int	retval;
	char	tmp[BUFSIZ];

	PP_TRACE (("ad_local (%s, ad=%s, %s)",
		   locname, ad->ad_value, 
		   (ad->aparse->local_hub_name) ? ad->aparse->local_hub_name : "main hub"));

	if (ad->aparse->dont_use_aliases != TRUE) {
		switch (tb_getalias (locname, alp, 
				     ad->aparse->local_hub_name)) {
		    case NOTOK:
			/* -- routine or table error -- */
			PP_TRACE (("ad_local/tb_getalias failed on %s", 
				   locname));	
			(void) sprintf(tmp, "Alias error for '%s'", locname);
			set_error(ad, tmp);
			return parselose (rp, RP_USER, 
					  "Alias error for %s",
					  locname);
		    case OK:
			/* -- found, do alias check -- */
			if (alp->alias_external != ALIAS_EXTERNAL
			    || ad->ad_resp == NO)
				return do_alias_check (locname,
						       alp, ad, rp);

		    default:
			break;
		}
	}
	/* -- otherwise do the user checks -- */
	if (rp_isbad(retval = do_user_check(locname,
						ad,
						rp))
	    && !rp_isbad(reparse_local(locname, ad, rp)))
		retval = RP_AOK;
	return retval;


	/* NOTREACHED */
}

/*  */

static int redirect_copy (pto, from, ad, rp)
Redirection	**pto, *from;
ADDR	*ad;
RP_Buf	*rp;
{
	Redirection	*tmp;
	char	buf[BUFSIZ];
	
	while (from != (Redirection *) NULL) {
		if (redirect_before(*pto, from) == TRUE) {
			(void) sprintf(buf,
				       "Redirection loop detected for '%s'",
				       from->rd_addr);
			set_error(ad, buf);
			return parselose (rp, RP_USER,
					  "Redirection loop detected for '%s'",
					  from->rd_addr);
		}
		tmp = from->rd_next;
		from->rd_next = (Redirection *) NULL;
		redirect_add(pto, from);
		from = tmp;
	}
	return RP_AOK;
}
	       
static void ad_copy (ad, new, resp)
register ADDR	*ad;
ADDR	*new;
int	resp;
{
	PP_DBG (("ad_copy()"));

	if (ad->ad_outchan)
		list_rchan_free (ad->ad_outchan);
	ad->ad_outchan = new->ad_outchan;
	new->ad_outchan = NULLIST_RCHAN;
	ad->ad_parse_stat = new->ad_parse_stat;
	
	if (resp == YES) {
		Aparse_ptr tmp = ad->aparse;
		ad->aparse = new->aparse;
		ad->aparse->recurse_count = tmp->recurse_count;
		ad->aparse->dont_use_aliases = tmp->dont_use_aliases;
		new->aparse = tmp;
	}
}
		

int alias2adr (buf, alp, ap)
char	*buf;
ALIAS	*alp;
register Aparse_ptr	ap;
{
	char	tmp[BUFSIZ];
	OR_ptr	holder;

	if (alp->alias_ad_type == AD_ANY_TYPE) {
		/* need to keep domain as may be local subdom */
		switch (ap->ad_type) {
		    case AD_822_TYPE:
			if (ap->ap_domain) 
				(void) sprintf(buf, "%s@%s",
					       alp->alias_user, 
					       ap->ap_domain->ap_obvalue);
			else
				(void) strcpy(buf, alp->alias_user);
			break;
		    case AD_X400_TYPE:
			if (ap->lastMatch
			    && ap->orname->on_or) {
				holder = ap->lastMatch->or_next;
				ap->lastMatch->or_next = NULLOR;
				
				or_or2std(ap->lastMatch, tmp, FALSE);
				ap->lastMatch->or_next = holder;
				(void) sprintf(buf, "%s%s",
					       alp->alias_user,
					       tmp);
			} else
				(void) strcpy(buf, alp->alias_user);
			break;
		    default:
			(void) strcpy (buf, alp->alias_user);
			break;
		}
	} else
		(void) strcpy (buf, alp->alias_user);
}

static int do_alias_check (name, alp, ad, rp)
char	*name;
ALIAS	*alp;
register ADDR	*ad;
RP_Buf	*rp;
{
	char	tmp[BUFSIZ], buf[BUFSIZ];
	register ADDR	*new;
	int	copy_results, retval, type;

	if ((type = alp->alias_ad_type) == AD_ANY_TYPE)
		type = ad->aparse->ad_type;

	alias2adr(buf, alp, ad->aparse);
	if (aparse_inAliasList (buf, ad->aparse->aliases) == OK) {
		if (alp->alias_external == ALIAS_EXTERNAL)
			PP_NOTICE(("circular aliases found '%s' ['%s' -> '%s'] - BUT external so ignore !",
				   alp->alias_user, name, 
				   buf));
		else {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("do_alias_check circular aliases '%s' ['%s' -> '%s']",
				alp->alias_user, name, 
				buf));
			(void) sprintf(tmp,
				       "local circular alias/synonym detected");
			set_error(ad, tmp);
			(void) strcpy(rp->rp_line, tmp);
			return RP_PARSE;
		}
	} else
		aparse_addToAliasList (buf, 
				       &(ad->aparse->aliases));

	new = adr_new(buf,
		      type,
		      ad->ad_extension);
	aparse_copy_for_recursion(new->aparse, ad->aparse);
	if (alp->alias_external == ALIAS_EXTERNAL)
		new->aparse->dont_use_aliases = TRUE;

	switch (alp->alias_type) {
	    case ALIAS_SYNONYM:
		new->ad_resp = ad->ad_resp;
		copy_results = YES;
		break;
		
	    case ALIAS_PROPER:
		new->ad_resp = YES;
		copy_results = ad->ad_resp;
		break;
	}


	retval = ad_parse_aux (new, rp);
	if (!rp_isbad(retval) &&
	    alp->alias_type == ALIAS_PROPER) {
		/* add redirect */
		char	tbuf[BUFSIZ], *dn = NULLCP;
		OR_ptr	or;

		switch (ad->aparse->ad_type) {
		    case AD_X400_TYPE:
			if (isstr(ad->aparse->x400_str))
				(void) strcpy(tbuf, ad->aparse->x400_str);
			else if (ad->aparse->orname->on_or != NULLOR)
				or_or2std(ad->aparse->orname->on_or, 
					  tbuf, FALSE);
			else if (isstr(ad->ad_r400adr))
				(void) strcpy(tbuf, ad->ad_r400adr);
			else
				tbuf[0] = '\0';
			
			if (isstr(ad->aparse->x400_dn))
				dn = ad->aparse->x400_dn;
			break;
		    case AD_822_TYPE:
		    default:
			if (isstr(ad->aparse->r822_str))
				(void) or_rfc2or_aux(ad->aparse->r822_str,
						     &or,
						     ad->ad_no);
			else if (isstr(ad->ad_r822adr))
				(void) or_rfc2or_aux(ad->ad_r822adr,
						     &or,
						     ad->ad_no);
			else
				or = NULLOR;

			if (or != NULLOR) {
				or_or2std(or, tbuf, FALSE);
				or_free (or);
			} else
				tbuf[0] = '\0';
		}
		if (tbuf[0] != '\0') {
			Redirection	*rtmp ;

			rtmp = redirect_new(tbuf,
					    (dn == NULLCP) ? "" : dn,
					    NULLUTC,
					    RDR_MD_ASSIGNED);
			
			/* add in redirect */
			if (rp_isbad(retval = redirect_copy(&(ad->ad_redirection_history),
					       rtmp,
					       ad, rp))) {
				/* redirect loop so don't copy results */
				copy_results = NO;
				redirect_free(rtmp);
			}
		}

	}

	if (!rp_isbad(retval)
	    && new->ad_redirection_history != (Redirection *) NULL) {
		/* add in redirection history from recursion */
		if (rp_isbad(retval = redirect_copy(&(ad->ad_redirection_history),
				       new->ad_redirection_history,
				       ad, rp))) {
			/* redirect loop so don't copy results */
			copy_results = NO;
		} else
			new->ad_redirection_history = (Redirection *) NULL;
	}

	ad_copy (ad, new, copy_results);

	if (new)
		adr_free(new);

	return retval;
}

/*  */

static int do_user_check(name, ad, rp)
char	*name;
register ADDR	*ad;
RP_Buf	*rp;
{
	char	buf[BUFSIZ], tmp[BUFSIZ];
	int	next;
	AP_ptr	dmn = NULLAP;

	LIST_RCHAN	*ix, *ix2, *newchan = NULLIST_RCHAN, *end;
	
	PP_TRACE (("ad_local/do_user_check (%s, %s)",
		   name, 
		   (ad->aparse->local_hub_name) ? ad->aparse->local_hub_name : "main hub"));
	
	if ((ad->ad_outchan = 
	     tb_getuser (name, ad->aparse->local_hub_name)) == NULLIST_RCHAN) {
		(void) sprintf(buf, 
			       "Unknown local user '%s'",
			       name);
		set_error(ad, buf);
		return parselose (rp, ad->ad_parse_stat = RP_USER,
				  "Unknown local user '%s'", name);
	}

	next = TRUE;
	for (ix = ad->ad_outchan;
	     ix != NULLIST_RCHAN;
	     ix = (next == TRUE) ? ix -> li_next : ix) {
		/* -- check this is the machine on --*/
		/* -- which local delivery is done -- */
		next = TRUE;
		
		if (ix -> li_mta != NULLCP
		    && lexequ (ix->li_mta, loc_dom_mta) != 0) {
			/* -- not this machine, update ix -- */
			
			/* lookup given mta */
			if (tb_getchan (ix -> li_mta, &newchan) == NOTOK)
				newchan = NULLIST_RCHAN;

			if (newchan == NULLIST_RCHAN) {
				/* try normalising given mta */
				dmn = ap_new(AP_DOMAIN, ix->li_mta);
				if (rfc822_norm_dmn (dmn, 
						     ad->aparse->dmnorder) != OK
				    || dmn -> ap_recognised == FALSE
				    || dmn -> ap_chankey == NULLCP) {
					if (dmn -> ap_recognised == FALSE)
						PP_LOG(LLOG_EXCEPTIONS,
						       ("Internal routing (users table) error for mta '%s' [unable to normalise domain]",
							ix->li_mta));
					else if (dmn->ap_chankey == NULLCP)
						PP_LOG(LLOG_EXCEPTIONS,
						       ("Internal routing (users table) error for mta '%s' [no index to routing info for '%s']",
							ix->li_mta,
							dmn->ap_obvalue));
					
					(void) sprintf(tmp,
						       "Unknown internal domain '%s'",
						       ix->li_mta);
					set_error(ad, tmp);
					ap_free(dmn);
					return ad->ad_parse_stat =
						parselose (rp,
							   RP_USER,
							   "Unknown internal domain '%s'",
							   ix -> li_mta);
				}
				if (dmn -> ap_islocal == FALSE)
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Mta '%s' is used for internal routing (users table) but is not marked as local in domain table",
						dmn -> ap_obvalue));

			}
			if (newchan == NULLIST_RCHAN
			    && dmn != NULLAP
			    && tb_getchan (dmn->ap_chankey, &newchan) == NOTOK) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Internal routing (users table) error for mta '%s' [no routing info for '%s']",
					ix -> li_mta, dmn->ap_chankey));
				(void) sprintf(tmp,
					       "Unknown internal domain '%s'",
					       ix -> li_mta);
				set_error(ad, tmp);
				return ad->ad_parse_stat =
					parselose (rp,
						   RP_USER,
						   "Domain '%s' not registered in channel table",
						   ix -> li_mta);
			}
			
			if (dmn != NULLAP) {
				ap_free(dmn);
				dmn = NULLAP;
			}
			/* insert new list_rchan into ad->ad_outchan */
			end = newchan;
			while (end->li_next != NULLIST_RCHAN)
				end = end->li_next;
			if (ix == ad->ad_outchan) {
				end -> li_next = ad->ad_outchan->li_next;
				ad->ad_outchan = newchan;
			} else {
				ix2 = ad->ad_outchan;
				while (ix2 != NULLIST_RCHAN
				       && ix2 -> li_next != ix)
					ix2 = ix2->li_next;
				if (ix2 != NULLIST_RCHAN) {
					end -> li_next = ix -> li_next;
					ix2->li_next = newchan;
				}
			}
			ix -> li_next = NULLIST_RCHAN;
			list_rchan_free(ix);
			ix = end;
		} else if (ix -> li_chan == NULLCHAN) {
			/* invalid local channel */
			if (ix == ad->ad_outchan) {
				ix2 = ix;
				ix = ad->ad_outchan = ad->ad_outchan->li_next;
				next = FALSE;
				/* yuch */
			} else {
				for (end = ad->ad_outchan;
				     end -> li_next != ix;
				     end = end->li_next)
					continue;
				end -> li_next = ix -> li_next;
				ix2 = ix;
				ix = end;
			}
			ix2->li_next = NULLIST_RCHAN;
			list_rchan_free(ix2);
		}
	}

	if (ad->ad_outchan == NULLIST_RCHAN) {
		set_error(ad, "No valid local delivery channels");
		return ad->ad_parse_stat =
			parselose(rp, RP_USER, "%s",
				  "No valid local delivery channels");
	}

	for (ix = ad->ad_outchan; ix; ix = ix -> li_next)
		if (ix->li_mta == NULLCP)
			ix->li_mta = strdup(loc_dom_mta);

	return (ad->ad_parse_stat = RP_AOK);
}

/*  */

static int reparsing = 0;

static int reparse_local (inname, ad, rp)
char	*inname;
register ADDR	*ad;
RP_Buf	*rp;
{
	char	*bang = NULLCP,
		*percent = NULLCP,
		*at = NULLCP,
		*ix, *newadrstr, *name,
		*start = strdup(inname);
	int	retval = NOTOK;
	register ADDR	*newadr;

	name = start;
	if (reparsing > 0) 
		/* more than one level of reparsing wrong */
		return NOTOK;
	/* remove preceding and trailing quotes */
	while (*name == '"' && *name != '\0') name++;

	ix = &(name[strlen(name)]);
	while (*ix == '"' && ix != name) {
		*ix = '\0';
		ix --;
	}
	
	bang = index(name, '!');
	percent = index(name, '%');
	at = index(name, '@');

	/* only allow one type of addressing */
	if (!((bang && !percent && !at) ||
	      (percent && !bang && !at) ||
	      (at && !bang && !percent))) {
		free(start);
		return NOTOK;
	}

	if (bang) {
		int	numBangs = 0;
		char	*iix, *jx;

		bang = name;
		while ((bang = index(bang, '!')) != NULLCP) {
			numBangs++;
			bang++;
		}
		
		newadrstr = malloc((unsigned)strlen(name) + 2 * (numBangs - 1)
				   + 1);
		iix = name; 
		jx = newadrstr;
		while (numBangs > 1) {
			*jx++ = '@';
			while (*iix != '!' && *iix != '\0')
				*jx++ = *iix++;
			if (*iix == '!') {
				numBangs--;
				if (numBangs == 1) 
					*jx++ = ':';
				else
					*jx++ = ',';
				iix++;
			}
		}
		*jx = '\0';
		bang = rindex(iix, '!');
		*bang++ = '\0';
		(void) sprintf(newadrstr, "%s%s@%s",
			       newadrstr, bang, iix);
	} else
		newadrstr = strdup(name);

	/* with bangs, percents etc. got to be 822 */
	newadr = adr_new(newadrstr,
			     AD_822_TYPE,
			     ad->ad_no);
	aparse_copy_for_recursion(newadr->aparse, ad->aparse);
	
	if (percent
	    || ad->aparse->percents == TRUE)
		newadr->aparse->percents = TRUE;
		
	reparsing++;

	if (!(rp_isbad(retval = ad_parse_aux(newadr, rp)))) {
		/* parsed okay so copy relevant bits across */
		if (newadr->ad_redirection_history != (Redirection *) NULL) {
			/* add in redirection history from recursion */
			if (!rp_isbad(retval = redirect_copy(&(ad->ad_redirection_history),
							     newadr->ad_redirection_history,
							     ad, rp)))
				newadr->ad_redirection_history = (Redirection *) NULL;
		}
		if (!rp_isbad(retval))
			ad_copy(ad, newadr,  YES);
	} else {
		/* didn't work so ignore errors */
		;
	}

	reparsing--;

	free(start);
	free(newadrstr);
	adr_free(newadr);
	return retval;
}
