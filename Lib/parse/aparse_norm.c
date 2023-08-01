/* aparse_norm.c: normalise an address folloing local synonyms */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/parse/RCS/aparse_norm.c,v 6.0 1991/12/18 20:23:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/parse/RCS/aparse_norm.c,v 6.0 1991/12/18 20:23:41 jpo Rel $
 *
 * $Log: aparse_norm.c,v $
 * Revision 6.0  1991/12/18  20:23:41  jpo
 * Release 6.0
 *
 */
#include "util.h"
#include "adr.h"
#include "aparse.h"
#include "alias.h"

extern char *loc_dom_site;

static int x400_norm(), x400_local_norm(), 
	rfc822_norm(), rfc822_local_norm(),
	convert_to_x400(), convert_to_rfc822();

int aparse_norm (ap)
register Aparse_ptr ap;
{
	int	out_adr_type = ap->ad_type;

	if (ap->aliases) {
		aparse_freeAliasList (ap->aliases);
		ap->aliases = NULL;
	}

	switch (ap->ad_type) {
	    case AD_X400_TYPE:
		if (x400_norm(ap) != OK) 
			return NOTOK;
		if (ap->ad_type != out_adr_type
		    && convert_to_x400 (ap) != OK)
			return NOTOK;
		return OK;
		
	    case AD_822_TYPE:
	    default:
		if (rfc822_norm(ap) != OK)
			return NOTOK;
		if (ap->ad_type != out_adr_type
		    && convert_to_rfc822 (ap) != OK)
			return NOTOK;
		return OK;
	}
}

/*  */

static int x400_norm (ap)
register Aparse_ptr ap;
{
	int	type;
	char	*replace;
	char	buf[BUFSIZ];

	if (ap->x400_str == NULLCP
	    && ap->orname->on_or == NULLOR)
		return NOTOK;

	if (ap->orname->on_or == NULLOR) {
		ap->orname->on_or = 
			or_std2or(ap->x400_str);
		ap->orname->on_or = or_default(ap->orname->on_or);
	}
		
	
	if (ap->x400_local) {
		free(ap->x400_local);
		ap->x400_local = NULLCP;
	}
	if (or_check (&(ap->orname->on_or),
		      buf, &type,
		      &(ap->local_hub_name),
		      &replace,
		      &(ap->lastMatch)) == NOTOK)
		return NOTOK;
	if (replace) free(replace);

	if (type == OR_ISLOCAL) {
		ap->x400_local = strdup(buf);
		return x400_local_norm (ap);
	}
	ap->ad_type = AD_X400_TYPE;
	return OK;
}

/*  */

static int x400_local_norm (ap)
register Aparse_ptr ap;
{
	ALIAS alias_struct, *alp = &alias_struct;
	OR_ptr	syn, tmp;
	char	buf[BUFSIZ];

	if (ap->x400_local == NULLCP)
		return NOTOK; /* ? */
	if (ap->dont_use_aliases == TRUE) {
		ap->ad_type = AD_X400_TYPE;
		return OK;
	}
	if (tb_getalias (ap->x400_local,
			 alp, ap->local_hub_name) != OK
	    || alp->alias_type != ALIAS_SYNONYM) {
		/* no synonym */
		ap->ad_type = AD_X400_TYPE;
		return OK;
	}

	if (alp->alias_external == ALIAS_EXTERNAL) {
		if (ap->internal == TRUE) {
			/* don't follow */
			ap->ad_type = AD_X400_TYPE;
			return OK;
		}
		/* follow but don't use aliases table */
		/* may still need to normalise */
		ap->dont_use_aliases = TRUE;
	}

	alias2adr (buf, alp, ap);

	if (aparse_inAliasList (buf, ap->aliases) == OK) {
		if (alp->alias_external != ALIAS_EXTERNAL)
			/* looping synonym */
			return NOTOK;
		/* ignore loops if external */
	}else
		aparse_addToAliasList (buf, &(ap->aliases));
	
	switch (alp->alias_ad_type) {
	    case AD_ANY_TYPE:
		/* replace from *ptree to lastMatch -> or_prev */
		/* with alp->alias_user */
		
		if ((syn = or_std2or(alp->alias_user)) == NULLOR)
			return NOTOK;

		/* rmoove old stuff */
		tmp = ap->lastMatch->or_next;
		tmp->or_prev = NULLOR;
		ap->lastMatch->or_next = NULLOR;
		or_free(tmp);
		
		while (syn != NULLOR) {
			tmp = syn -> or_next;
			if (tmp != NULLOR) {
				syn -> or_next = NULLOR;
				tmp -> or_prev = NULLOR;
			}

			if ((ap->orname->on_or = or_add (ap->orname->on_or,
							 syn, TRUE)) == NULLOR)
				return NOTOK;
			syn = tmp;
		}
		break;

	    case AD_X400_TYPE:
		/* completely replace old tree */
		or_free (ap->orname->on_or);
		if ((ap->orname->on_or = or_std2or(alp->alias_user)) == NULLOR)
			return NOTOK;
		ap->orname->on_or = or_default(ap->orname->on_or);
		break;

	    default:
		/* 822 synonym */
		aparse_rewindr822(ap);
		ap->r822_str = strdup(alp->alias_user);
		return rfc822_norm (ap);
	}

	return x400_norm(ap);
}

/*  */

AP_ptr merge_new (ap, ntree, nloc, ndom)
AP_ptr	ap, ntree, nloc, ndom;
{
	AP_ptr	hdr, ix, nix = NULLAP, tmp;
	int	cont = TRUE;
	
	hdr = ap_new(AP_COMMENT, "header");
	hdr->ap_next = ap;
	hdr->ap_ptrtype = AP_PTR_MORE;
	ix = hdr;

	/* delete up to first mailbox */
	while (ix->ap_next != NULL
	       && cont == TRUE) {
		switch (ix -> ap_next -> ap_obtype) {
		    case AP_DOMAIN:
		    case AP_DOMAIN_LITERAL:
			/* part of route */
			ap_delete (ix);
			break;
				
		    case AP_MAILBOX:
		    case AP_GENERIC_WORD:
			/* reached mailbox */
			cont = FALSE;
			break;
				
		    case AP_COMMENT:
		    default:
			/* preserve */
			ix = ix->ap_next;
			break;
		}
	}
		
	/* merge in ntree up to inclusive */
	if (ntree) {
		for (nix = ntree;
		     nix -> ap_next != NULLAP
		     && nix -> ap_next != nloc -> ap_next;
		     nix = nix -> ap_next)
			continue;
		if (nix) {
			tmp = nix -> ap_next;
			nix -> ap_next = NULLAP;
		} else
			tmp = NULLAP;
			
		ix = ap_sqinsert(ix,
				 AP_PTR_MORE,
				 ntree);
		nix = tmp;
	}

	cont = TRUE;
	/* delete up to next domain */
	while (ix->ap_next != NULL
	       && cont == TRUE) {
		switch (ix -> ap_next -> ap_obtype) {
		    case AP_DOMAIN:
		    case AP_DOMAIN_LITERAL:
			cont = FALSE;
			break;
				
		    case AP_MAILBOX:
		    case AP_GENERIC_WORD:
			ap_delete (ix);
			break;
				
		    case AP_COMMENT:
		    default:
			/* preserve */
			ix = ix->ap_next;
			break;
		}
	}
		
	/* merge in up to ndom inclusive */
	if (nix) {
		ntree = nix;
		for (;
		     nix -> ap_next != NULLAP
		     && nix -> ap_next != ndom -> ap_next;
		     nix = nix -> ap_next)
			continue;
		if (nix) {
			tmp = nix -> ap_next;
			nix -> ap_next = NULLAP;
		} else
			tmp = NULLAP;
		ix = ap_sqinsert(ix,
				 AP_PTR_MORE,
				 ntree);
		nix = tmp;
	} else
		/* no local */
		return NULLAP;
		
	/* delete remaining domains */
	cont = TRUE;
	while (ix->ap_next != NULLAP && cont == TRUE) {
		switch (ix -> ap_next -> ap_obtype) {
		    case AP_DOMAIN:
		    case AP_DOMAIN_LITERAL:
		    case AP_MAILBOX:
		    case AP_GENERIC_WORD:
			ap_delete(ix);
			break;
		    case AP_COMMENT:
			/* preserve */
			ix = ix -> ap_next;
			break;
		    default:
			/* save end of names etc */
			cont = FALSE;
			break;
		}
	}
		
	if (nix)
		ix = ap_sqinsert(ix,
				 AP_PTR_MORE,
				 nix);
	ap = hdr -> ap_next;
	ap_free(hdr);
	return ap;
}

#ifdef VAT
static void db_check(name, loc, dom, route)
AP_ptr	name,
	loc,
	dom, 
	route;
{
	AP_ptr	ix = name;
	
	while (ix != NULLAP &&
	       ix != loc &&
	       ix != dom &&
	       ix != route) {
		if ((ix -> ap_obtype == AP_PERSON_END
		     || ix -> ap_obtype == AP_PERSON_NAME
		     || ix -> ap_obtype == AP_PERSON_START)
		    && ix->ap_obvalue != NULLCP
		    && lexequ("Hardcastle-Kille", ix->ap_obvalue) == 0) {
			AP_ptr	ix2, tmp = ap_s2t("Hardcastle(Hyphen)Kille");
			
			for (ix2 = tmp; ix2 != NULLAP; ix2 = ix2->ap_next) {
				if (ix2->ap_obtype != AP_COMMENT)
					ix2->ap_obtype = ix->ap_obtype;
			}
			(void) ap_sqinsert (ix, AP_PTR_MORE, tmp);
			/* copy up and delete */
			tmp = ix -> ap_next;
			ix -> ap_obtype = tmp -> ap_obtype;
			ix -> ap_ptrtype = tmp -> ap_ptrtype;
			if (ix -> ap_obvalue) 
				free (ix->ap_obvalue);
			ix -> ap_obvalue = tmp -> ap_obvalue;
			tmp -> ap_obvalue = NULLCP;
			ix -> ap_next = tmp -> ap_next;
			tmp -> ap_next = NULLAP;
			ap_free(tmp);
		}
		ix = ix -> ap_next;
	}
}
#endif

static AP_ptr	get_next_route (ap, startfrom)
register Aparse_ptr ap;
AP_ptr		startfrom;
{
	AP_ptr	ret;
	
	if (startfrom == NULLAP)
		return NULLAP;
	
	if (startfrom->ap_obtype == AP_DOMAIN
	    || startfrom->ap_obtype == AP_DOMAIN_LITERAL)
		return startfrom;

	ret = startfrom;
	for (;;) {
		switch (ret->ap_ptrtype) {
		    case AP_PTR_MORE:
			/* -- next is part of this address -- */
			ret = ret -> ap_next;
			if (ret == ap->ap_group ||
			    ret == ap->ap_name ||
			    ret == ap->ap_local ||
			    ret == ap->ap_domain)
				return NULLAP;

			switch (ret -> ap_obtype) {
			    case AP_DOMAIN_LITERAL:
			    case AP_DOMAIN:
				return ret;
			    default:
				break;
			}
			break;
		    default:
			return NULLAP;
		}
	}
}

static int rfc822_norm (ap)
register Aparse_ptr ap;
{
	AP_ptr rout;

	if (ap->r822_str == NULLCP
	    && ap->ap_tree == NULLAP)
		return NOTOK;

	if (ap->ap_tree == NULLAP) 
		ap->ap_tree = ap_s2t(ap->r822_str);
	
	(void) ap_t2p (ap->ap_tree, &(ap->ap_group), 
		       &(ap->ap_name), &(ap->ap_local), 
		       &(ap->ap_domain), &(ap->ap_route));

#ifdef VAT
	if ((getpid() % 10) == 2)
		db_check (ap->ap_name, ap->ap_local,
			  ap->ap_domain, ap->ap_route);
#endif

	if (ap->normalised == APARSE_NORM_ALL) {
		/* normalise all domains */
		rout = ap->ap_route;
		while (rout &&
		       (rout = get_next_route(ap, rout)) != NULLAP) {
			(void) rfc822_norm_dmn(rout, ap->dmnorder);
			rout = rout->ap_next;
		}
		if (ap->ap_domain)
			(void) rfc822_norm_dmn (ap->ap_domain, ap->dmnorder);
		rout = get_next_route (ap, ap->ap_route);
	} else {
		/* normalise first domain */
		if ((rout = get_next_route(ap, ap->ap_route)) != NULLAP)
			(void) rfc822_norm_dmn (rout, ap->dmnorder);
		else if (ap->ap_domain)
			(void) rfc822_norm_dmn(ap->ap_domain, ap->dmnorder);
			
	}

	while (rout && rout->ap_islocal == TRUE) {
		/* local so remove from route */
		ap->ap_route = rout = get_next_route(ap, rout->ap_next);
		if (rout && rout->ap_normalised != TRUE)
			(void) rfc822_norm_dmn(rout, ap->dmnorder);
	}

	if (rout == NULLAP) {
		/* no route */
		if (ap->ap_domain == NULLAP) {
			/* no domain either so add default */
			if (ap->ap_local == NULLAP)
				/* no local either so fail */
				return NOTOK;
			for (rout = ap->ap_local;
			     rout -> ap_next != NULLAP;
			     rout = rout -> ap_next)
				continue;
			(void) ap_append(rout, AP_DOMAIN, loc_dom_site);
			ap->ap_domain = rout -> ap_next;
			(void) rfc822_norm_dmn(ap->ap_domain, ap->dmnorder);
		} else if (ap->ap_domain->ap_normalised != TRUE)
			(void) rfc822_norm_dmn(ap->ap_domain, ap->dmnorder);
		
		if (ap->ap_domain->ap_islocal == TRUE) 
			return rfc822_local_norm(ap);
	}
	ap->ad_type = AD_822_TYPE;
	return OK;
}

static int rfc822_local_norm(ap)
register Aparse_ptr ap;
{
	ALIAS alias_struct, *alp = &alias_struct;
	char	buf[BUFSIZ], *str;
	AP_ptr	newtree, newgroup, newname, newloc, newdom, newroute;

	if (ap->r822_local) free(ap->r822_local);
	ap->r822_local = ap_p2s_nc(NULLAP, NULLAP,
				   ap->ap_local,
				   NULLAP, NULLAP);

	if (ap->dont_use_aliases == TRUE) {
		ap->ad_type = AD_822_TYPE;
		return OK;
	}

	if (tb_getalias(ap->r822_local, alp, ap->ap_domain->ap_localhub) != OK
	    || alp->alias_type != ALIAS_SYNONYM) {
		/* no synonym */
		ap->ad_type = AD_822_TYPE;
		return OK;
	}

	if (alp->alias_external == ALIAS_EXTERNAL) {
		if (ap->internal == TRUE) {
			/* don't follow */
			ap->ad_type = AD_822_TYPE;
			return OK;
		}
		/* follow but don't use aliases table */
		/* may still need to normalise */
		ap->dont_use_aliases = TRUE;
	}

	alias2adr (buf, alp, ap);
	
	if (aparse_inAliasList (buf, ap->aliases) == OK) {
		if (alp->alias_external != ALIAS_EXTERNAL)
			/* looping synonym */
			return NOTOK;
		/* ignore lopps if external */
	} else
		aparse_addToAliasList (buf, &(ap->aliases));

	switch (alp->alias_ad_type) {
	    case AD_ANY_TYPE:
		/* use existing domian but replace loc */
		newloc = ap_s2t(alp->alias_user);
		str = ap_p2s(NULLAP, NULLAP, newloc, ap->ap_domain, NULLAP);
		ap_free(newloc);
		break;

	    case AD_822_TYPE:
		/* replace all */
		str = strdup(alp->alias_user);
		break;
		
	    default:
		aparse_rewindx400(ap);
		ap->x400_str = strdup(alp->alias_user);
		return x400_norm(ap);
	}
	
	newtree= ap_s2t(str);
	(void) ap_t2p(newtree, &newgroup,
		      &newname, &newloc,
		      &newdom, &newroute);
	ap->ap_tree = merge_new(ap->ap_tree, newtree, newloc, newdom);
	
	free(str);
	(void) ap_t2p (ap->ap_tree, &(ap->ap_group), 
		       &(ap->ap_name), &(ap->ap_local), 
		       &(ap->ap_domain), &(ap->ap_route));

	if (ap->r822_str) free(ap->r822_str);
	ap->r822_str = ap_p2s(ap->ap_group, ap->ap_name,
			      ap->ap_local, ap->ap_domain,
			      ap->ap_route);
	return rfc822_norm (ap);
}

	
/*  */

extern char	or_error[];

static int convert_to_x400 (ap)
register Aparse_ptr ap;
{
	char	tbuf[BUFSIZ];
	OR_ptr or;

	if (ap -> r822_str == NULLCP)
		return NOTOK;

	aparse_rewindx400(ap);

	if (or_rfc2or_aux (ap->r822_str,
			   &(ap->orname->on_or), 1) == NOTOK) {
		or_free(ap->orname->on_or);
		ap->orname->on_or = NULLOR;
		PP_DBG(("convert_to_x400 failed (%s)", or_error));
		return NOTOK;
	}
	
	if ((or = or_default (ap->orname->on_or)) == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS, ("or_default failed"));
		return NOTOK;
	}
	ap -> orname -> on_or = or;

	or_or2std (ap->orname->on_or, tbuf, FALSE);
	ap->x400_str = strdup(tbuf);
	ap->ad_type = AD_X400_TYPE;

	return OK;
}

/*  */

static int convert_to_rfc822 (ap)
register Aparse_ptr ap;
{
	char	tbuf[BUFSIZ];

	if (ap->orname->on_or == NULLOR)
		return NOTOK;

	aparse_rewindr822(ap);

	if (or_or2rfc (ap->orname->on_or, tbuf) == NOTOK) {
		PP_DBG(("convert_to_rfc822 failed (%s)", or_error));
		return NOTOK;
	}

	ap->r822_str = strdup(tbuf);
	ap->ap_tree = ap_s2t(ap->r822_str);
	(void) ap_t2p (ap->ap_tree, &(ap->ap_group), 
		       &(ap->ap_name), &(ap->ap_local), 
		       &(ap->ap_domain), &(ap->ap_route));

	ap->ad_type = AD_822_TYPE;
	
	return OK;
}
