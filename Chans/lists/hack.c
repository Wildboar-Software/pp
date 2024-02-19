/* hack.c - taken from quipu/dish/fred.c */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/hack.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/hack.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: hack.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


#include <ctype.h>
#include <stdio.h>
#include <isode/quipu/ds_search.h>
#include <isode/quipu/list.h>
#include <isode/quipu/read.h>
#include <isode/quipu/entry.h>
#include <isode/quipu/ufn.h>
#include <isode/quipu/dua.h>
#include <isode/tailor.h>

#if ISODE < 69
#define s_filter filter
#endif

struct dn_seq *dl_interact ();

Filter	joinfilter (), ocfilter ();

struct dn_seq *dn_seq_push ();

/*    DM2DN SUPPORT */

static	int	dlevel = 0;
static	int	dsa_status;

static struct dn_seq *dm2dn_seq ();
static struct dn_seq *dm2dn_seq_aux ();
Filter	make_filter ();

/*  */

DN str2ufn_here (ufn,dn)
char * ufn;
DN dn;
{
struct ds_search_arg search_arg;
static struct ds_search_result result;
struct DSError err;
static CommonArgs ca = default_common_args;
Filter filtcn, filtoc, filtand, filtor, filtuid, strfilter();

	filtcn = strfilter (AttrT_new("cn"),ufn,FILTERITEM_APPROX);
	filtuid = strfilter (AttrT_new("uid"),ufn,FILTERITEM_EQUALITY);
	filtoc = ocfilter ("person");

	filtuid->flt_next = filtcn;
	filtor = joinfilter (filtuid, FILTER_OR);

	filtoc->flt_next = filtor;
	filtand = joinfilter (filtoc,FILTER_AND);

	search_arg.sra_baseobject = dn;
	search_arg.sra_filter = filtand;
	search_arg.sra_subset = SRA_ONELEVEL;
	search_arg.sra_searchaliases = FALSE;
	search_arg.sra_common = ca; /* struct copy */
	search_arg.sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	search_arg.sra_eis.eis_allattributes = FALSE;
	search_arg.sra_eis.eis_select = NULLATTR;

	if (ds_search (&search_arg, &err, &result) != DS_OK) {
		filter_free (filtoc);
		log_ds_error (&err);
		ds_error_free (&err);
		return NULLDN;
	}
	filter_free (filtoc);

	if (result.CSR_entries) {
		if (result.CSR_entries->ent_next)
			/* could ask user to select here - leave that for UCL */
			return NULLDN;
		return result.CSR_entries->ent_dn ;
	}

	return NULLDN;
	
}

DN	do_dm_match (vec)
char  *vec;
{
    int	    seqno;
    char   *cp,
	    mbox[BUFSIZ]; 
    register struct dn_seq *dlist,
			   *dp;
    DN dn;
    extern char * local_dit;
    DN localdn;
    
    localdn = str2dn (local_dit);

    if ((cp = index (vec, '@')) && cp != vec) {
	*cp++ = NULL;
	(void) strcpy (mbox, vec);
	if (*cp == NULL) 
	    return NULLDN; 
    }
    else
	    return str2ufn_here(vec,localdn);

    if ((dlist = dm2dn_seq (cp)) == NULLDNSEQ) {
/*
	    ps_printf (OPT, "Unable to resolve domain.\n");
*/
	return NULLDN;
    }

    if ((! dlist) || dlist->dns_next)
	    NULLDN;

    if ((dn = str2ufn_here(mbox,dlist->dns_dn)) != NULLDN)
		return dn;

   return NULLDN;
}

/*  */

static struct dn_seq *dm2dn_seq (dm)
char   *dm;
{
    register char *dp;

    for (dp = dm; *dp; dp++)
	if (isupper (*dp))
	    *dp = tolower (*dp);

    dlevel = 0;
    dsa_status = OK;

    return dm2dn_seq_aux (dm, NULLDN, NULLDNSEQ);
}

/*  */

static struct dn_seq *dm2dn_seq_aux (dm, dn, dlist)
char   *dm;
DN	dn;
struct dn_seq *dlist;
{
    register char   *dp;
    struct ds_search_arg search_arg;
    register struct ds_search_arg *sa = &search_arg;
    struct ds_search_result search_result;
    register struct ds_search_result *sr = &search_result;
    struct DSError error;
    register struct DSError *se = &error;

    bzero ((char *) sa, sizeof *sa);

    sa -> sra_common.ca_servicecontrol.svc_options = SVC_OPT_PREFERCHAIN;
    sa -> sra_common.ca_servicecontrol.svc_timelimit = SVC_NOTIMELIMIT;
    sa -> sra_common.ca_servicecontrol.svc_sizelimit = SVC_NOSIZELIMIT;

    sa -> sra_baseobject = dn;
    sa -> sra_subset = SRA_ONELEVEL;
    sa -> sra_searchaliases = FALSE;

    sa -> sra_eis.eis_allattributes = FALSE;
    sa -> sra_eis.eis_select = NULLATTR;
    sa -> sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;

    dp = dm;
    for (;;) {
	int	    i;
	EntryInfo  *ptr;
	register struct s_filter *fi;

	if ((i = strlen (dp)) < dlevel)
	    break;

	sa -> sra_filter = fi = filter_alloc ();

	bzero ((char *) fi, sizeof *fi);
	fi -> flt_type = FILTER_ITEM;
	fi -> FUITEM.fi_type = FILTERITEM_EQUALITY;
	if ((fi -> FUITEM.UNAVA.ava_type = AttrT_new ("associateddomain")) == NULL)
	    fatal (-100, "associatedDomain: invalid attribute type");
	fi -> FUITEM.UNAVA.ava_value = str2AttrV (dp, str2syntax("IA5string"));

	if (ds_search (sa, se, sr) != DS_OK) 
		goto free_filter;

	if (sr -> srr_correlated != TRUE)
	    correlate_search_results (sr);

	if (sr -> CSR_entries == NULLENTRYINFO) {
	    filter_free (sa -> sra_filter);
	    if (dp = index (dp, '.'))
		dp++;
	    if (dp == NULL)
		break;
	    continue;
	}

	for (ptr = sr -> CSR_entries; ptr; ptr = ptr -> ent_next)
	    cache_entry (ptr, sa -> sra_eis.eis_allattributes,
			 sa -> sra_eis.eis_infotypes);

	if (i > dlevel) {
	    dlevel = i;
	    if (dlist)
		dn_seq_free (dlist), dlist = NULLDNSEQ;
	}

	if (i == dlevel)
	    for (ptr = sr -> CSR_entries; ptr; ptr = ptr -> ent_next) {
		struct dn_seq *dprev = dlist;

		dlist = dm2dn_seq_aux (dm, ptr -> ent_dn, dlist);

		if (dprev == dlist)
		    dlist = dn_seq_push (ptr -> ent_dn, dlist);
		else
		    if (i < dlevel)
			break;
	    }

	dn_free (sr -> CSR_object);
	entryinfo_free (sr -> CSR_entries, 0);
	crefs_free (sr -> CSR_cr);

free_filter: ;
	filter_free (sa -> sra_filter);
	break;
    }

    return dlist;
}

