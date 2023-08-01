/* imagesbr.c - image subroutines */

#ifndef	lint
static char *Rcsid = "$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/dirsbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
#endif

/* 
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/dirsbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 *
 * $Log: dirsbr.c,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 * 
 * 
 */


#include <ctype.h>
#include <stdio.h>
#include <varargs.h>
#include <isode/logger.h>
#include <isode/psap.h>
#include <isode/quipu/bind.h>
#include <isode/quipu/ds_search.h>
#include <isode/quipu/entry.h>
#include "image.h"

#ifndef NOPHOTOS
/* GENERAL */
#if ISODE < 69
#define s_filter filter
#endif

static char *myname = "image";
extern int debug;
int errsw = OK;

static LLog _pgm_log = {
    "-", NULLCP, NULLCP, LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE,
    LLOG_FATAL, -1, LLOGCLS | LLOGCRT | LLOGZER | LLOGTTY, NOTOK
};
LLog *pgm_log = &_pgm_log;
void adios ();

#define DEF_TIMELIMIT 20
/*    IMAGE */


struct image *fetch_image ();

/*    AKA */

struct aka {
    struct aka *ak_forw;
    struct aka *ak_back;

    time_t ak_notfound;
    char   *ak_domain;
    char   *ak_local;

    struct dn_seq *ak_bases;
    struct image *ak_image;
};
static struct aka akas;
static struct aka *AHead = &akas;

void init_aka ();

static int  stay_bound = 0;

DN	local_dn;


struct dn_seq *dm2dn_seq ();


extern char *local_dit;
struct dn_seq *dn_seq_push ();
int	dn_seq_print ();
PE	grab_pe ();

#ifdef TESTVERSION
main (argc, argv)
int argc;
char **argv;
{
	struct image *myim;

	if (argc != 3) {
		fprintf (stderr, "Usdage: %s host name\n", argv[0]);
		exit (1);
	}

	init_aka (myname = argv[0], 1, NULLCP); 
	myim = fetch_image (argv[1], argv[2]);

	if (myim) {
		printf ("Retrieved photo of size %dx%d\n", myim -> width,
			myim -> height);
	}
	else
		printf ("No photo found\n");
	exit (0);
}
#endif

/*  */

void	init_aka (pgm, stayopen, dit)
char   *pgm,
       *dit;
int	stayopen;
{
	char   *cp;
	register struct aka *ak;
	static int once_only = 0;
	static  char	*username, *password;

	if (once_only == 0) {
		int	argp;
		char   *arg[2],
		**argptr;

		quipu_syntaxes ();

		argp = 0;
		arg[argp++] = myname;
		arg[argp] = NULLCP;
	
		dsap_init (&argp, (argptr = arg, &argptr));
		quipurc (&username, &password);
		once_only = 1;
	}
	stay_bound = stayopen;
	myname = pgm;

	AHead -> ak_forw = AHead -> ak_back = AHead;
	if ((ak = (struct aka *) calloc (1, sizeof *ak)) == NULL)
		adios (NULLCP, "out of memory");

	ak -> ak_domain = ak -> ak_local = "";

	insque (ak, AHead -> ak_back);

	if ((local_dn = str2dn (cp = (dit ? dit
				      : *local_dit != '@' ? local_dit
				      : local_dit + 1)))
	    == NULLDN)
		adios (NULLCP, "local_dit invalid: \"%s\"", cp);
	do_bind (username, password);
}

quipurc (username, password)
char	**username, **password;
{
	char *home;
        FILE           *file;
        extern char    *local_dit;
	char *p;
	char Dish_Home[BUFSIZ];
	char	Read_in_Stuff[BUFSIZ];
	extern char *TidyString ();

	*username = *password = NULLCP;

        if (home = getenv ("QUIPURC"))
		(void) strcpy (Dish_Home, home);
        else if (home = getenv ("HOME"))
		(void) sprintf (Dish_Home, "%s/.quipurc", home);
	else
		(void) strcpy (Dish_Home, "./.quipurc");

        if ((file = fopen (Dish_Home, "r")) == 0)
                return;

        while (fgets (Read_in_Stuff, sizeof Read_in_Stuff, file) != 0) {
		char *part1, *part2;

                p = SkipSpace (Read_in_Stuff);
                if (( *p == '#') || (*p == '\0'))
                        continue; /* ignore comments and blanks */

                part1 = p;
                if ((part2 = index (p,':')) == NULLCP)
			continue;

                *part2++ = '\0';
                part2 = TidyString (part2);

                if (lexequ (part1, "username") == 0)
                        *username = strdup(*part2 != '@' ? part2 : part2 + 1);

                else if (lexequ (part1, "password") == 0) {
			*password = strdup(part2);
                } else if (lexequ (part1, "dsap") == 0)
                        (void) tai_string (part2);
                else if (lexequ (part1, "isode") == 0) {
                        char * split;
                        if ((split = index (part2,' ')) != NULLCP) {
                                *split++ = 0;
                                (void)isodesetvar (part2,strdup(split),0);
                        }
		}
        }
        (void) fclose (file);

}
/*  */

static struct aka *mbox2ak (local, domain)
char   *local,
       *domain;
{
	register struct aka *ak, *am, *adm= NULL;

	if (domain == NULL)
		domain = "";
	if (local == NULL)
		local = "";

	for (ak = AHead -> ak_forw; ak != AHead; ak = ak -> ak_forw) {
		if (lexequ (domain, ak -> ak_domain) != 0)
			continue;
		adm = ak;
		if (lexequ (local, ak -> ak_local) == 0) {
			if (debug)
				LLOG (pgm_log, LLOG_NOTICE,
				      ("hit \"%s\" \"%s\"", domain, local));

			return ak;
		}
	}
	if (adm) {
		if (debug)
			LLOG (pgm_log, LLOG_NOTICE,
			      ("partial hit \"%s\"", domain));
		if ((am = (struct aka *) calloc (1, sizeof *am)) == NULL
		    || (am -> ak_domain = strdup (domain)) == NULL
		    || (am -> ak_local = strdup (local)) == NULL)
			adios (NULLCP, "out of memory");
		am -> ak_bases = adm -> ak_bases;
		insque (ak = am, AHead);

		return ak;
	}
	
	if ((am = (struct aka *) calloc (1, sizeof *am)) == NULL
	    || (am -> ak_domain = strdup (domain)) == NULL
	    || (am -> ak_local = strdup (local)) == NULL)
		adios (NULLCP, "out of memory");

	if (debug)
		LLOG (pgm_log, LLOG_NOTICE,
		      ("miss \"%s\" \"%s\"", domain, local));

	if (index (domain, '.'))
		am -> ak_bases = dm2dn_seq (domain);
	else
		am -> ak_bases = dn_seq_push (local_dn, NULLDNSEQ);

	insque (ak = am, AHead);

	return ak;
}

/*    DIRECTORY */

#define	ADOMAIN		"associatedDomain"
#define	PHOTO		"photo"
#define	USERID		"userid"
#define MAILBOX		"rfc822Mailbox"
#define OTHERMAILBOX	"otherMailbox"
#define COMMONNAME 	"commonname"
#define SURNAME		"surname"

static int bound = 0;
static int dlevel = 0;
static AttributeType adomain_t;

struct dn_seq *dm2dn_seq_aux ();


static struct dn_seq *dm2dn_seq (dm)
char   *dm;
{
	register char *dp;
	char *av[100];
	int count;

	for (dp = dm, av[count = 0] = dm; *dp; dp++) {
		if (isupper (*dp))
			*dp = tolower (*dp);
		if (*dp == '.')
			av[++count] = dp+1;
	}
	av[++count] = NULL;

	if ((adomain_t = AttrT_new (ADOMAIN)) == NULL)
		adios ("invalid attribute type \"%s\"", ADOMAIN);

	dlevel = 0;

	if (!bound && do_bind (NULLCP, NULLCP) == NOTOK)
		return NULLDNSEQ;

	return dm2dn_seq_aux (dm, count, 0, av, NULLDN, NULLDNSEQ);
}

/*  */

static int find_best_match (atv, maxdmn, av)
AV_Sequence atv;
int maxdmn;
char *av[];
{
	int best = maxdmn + 1;
	int i;
	char buffer[BUFSIZ];
	PS	ps;

	if ((ps = ps_alloc (str_open)) == NULLPS
	    || str_setup (ps, buffer, sizeof buffer, 1) == NOTOK)
		adios ("can't set up PS stream");

	for (; atv; atv = atv -> avseq_next) {
		AttrV_print (ps, &atv -> avseq_av, EDBOUT);
		*ps->ps_ptr = NULL;
		for (i = 0; i < maxdmn; i++)
			if (lexequ (buffer, av[i]) == 0)
				if (i < best) {
					best = i;
					break;
				}
		if (str_setup (ps, buffer, sizeof buffer, 1) == NOTOK)
			adios ("can't reset PS stream");
	}
	ps_free (ps);
	if (best > maxdmn)
		return best;
	return maxdmn - best;
}

static struct dn_seq *dm2dn_seq_aux (dm, maxdmn, ndmn, av, dn, dlist)
char   *dm;
DN	dn;
int maxdmn, ndmn;
char *av[];
struct dn_seq *dlist;
{
	char   buffer[BUFSIZ];
	struct ds_search_arg search_arg;
	register struct ds_search_arg *sa = &search_arg;
	struct ds_search_result search_result;
	register CommonArgs *ca;
	register struct ds_search_result *sr = &search_result;
	struct DSError error;
	register struct DSError *se = &error;
	PS	    ps;

	if ((ps = ps_alloc (str_open))
	    && str_setup (ps, buffer, sizeof buffer, 1) != NOTOK) {
		dn_print (ps, dn, EDBOUT);
		ps_print (ps, " ");
		*--ps -> ps_ptr = NULL, ps -> ps_cnt ++;
	}
	else
		buffer[0] = NULL;
	if (ps)
		(void) ps_free (ps);

	if (debug)
		fprintf (stderr, "dlevel=%d maxlevel=%d dm=%s dn=%s\n",
			 ndmn, maxdmn, dm, buffer);

	bzero ((char *) sa, sizeof *sa);

	ca = &sa -> sra_common;
	ca -> ca_servicecontrol.svc_options = SVC_OPT_PREFERCHAIN;
	ca -> ca_servicecontrol.svc_timelimit = DEF_TIMELIMIT;
	ca -> ca_servicecontrol.svc_sizelimit = SVC_NOSIZELIMIT;

	sa -> sra_baseobject = dn;
	sa -> sra_subset = SRA_ONELEVEL;
	sa -> sra_searchaliases = FALSE;

	sa -> sra_eis.eis_allattributes = FALSE;
	sa -> sra_eis.eis_select = as_comp_new (str2AttrT (ADOMAIN), NULLAV,
						NULLACL_INFO);
	sa -> sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;

	for (;;) {
		int	    i;
		EntryInfo  *ptr;
		register s_filter *fi;

		if (debug)
			fprintf (stderr,
				 "-- dlevel=%d maxlevel=%d, domain=%s dn=%s\n",
				 ndmn, maxdmn, dm, buffer);

		if (ndmn >= maxdmn)
			break;

		sa -> sra_filter = fi = filter_alloc ();

		bzero ((char *) fi, sizeof *fi);
		fi -> flt_type = FILTER_OR;

		for (i = 0; i < maxdmn - ndmn; i++) {
			s_filter *f;

			f = filter_alloc ();
			bzero ((char *)f, sizeof *f);
			f -> flt_type = FILTER_ITEM;
			f -> FUITEM.fi_type = FILTERITEM_EQUALITY;
			f -> FUITEM.UNAVA.ava_type = AttrT_cpy(adomain_t);
			f -> FUITEM.UNAVA.ava_value =
				str2AttrV (av[i], adomain_t -> oa_syntax);

			f -> flt_next = fi -> FUFILT;
			fi -> FUFILT = f;
		}

		if (ds_search (sa, se, sr) != DS_OK) {
			if (debug)
				LLOG (pgm_log, LLOG_EXCEPTIONS,
				      ("search operation failed (%d)",
				       se -> dse_type));

			if (se -> dse_type == DSE_REMOTEERROR) {
				(void) ds_unbind ();
				bound = 0;
			}

			goto free_filter;
		}

		if (sr -> srr_correlated != TRUE)
			correlate_search_results (sr);

		if (sr -> CSR_entries == NULLENTRYINFO) {
			if (debug)
				LLOG (pgm_log, LLOG_NOTICE,
				      ("search for %s \"%s\" at baseobject \"%s\" failed",
				       ADOMAIN, dm, buffer));
			goto free_filter;
		}

		if (debug)
			LLOG (pgm_log, LLOG_NOTICE,
			      ("search for %s \"%s\" at baseobject \"%s\" succeeded",
			       ADOMAIN, dm, buffer));

		for (ptr = sr -> CSR_entries; ptr; ptr = ptr -> ent_next) {
			struct dn_seq *dprev = dlist;
			Attr_Sequence as;
			int best = maxdmn + 1;

			for (as = ptr -> ent_attr; as; as = as -> attr_link) {
				if (AttrT_cmp (as -> attr_type,
					       adomain_t) == 0) {
					i = find_best_match (as -> attr_value,
							     maxdmn, av);
					if (i < best)
						best = i;
				}
			}
			if (best < maxdmn) /* ok match */
				ndmn = best;
			else if (best == maxdmn) { /* perfect match */
				dlist = dn_seq_push (ptr -> ent_dn, dlist);
				continue;
			}
			else
				continue;

			dlist = dm2dn_seq_aux (dm, maxdmn, ndmn, av,
					       ptr -> ent_dn, dlist);

			if (dprev == dlist) /* nothing better in recursion? */
				dlist = dn_seq_push (ptr -> ent_dn, dlist);
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

/*  */
typedef struct SearchKeys {
	char	*type;
	char	*value;
} SearchKeys;

static  int image_search (ak, imp)
register struct aka *ak;
struct image *imp;
{
	register struct dn_seq *dlist;
	SearchKeys sk[10];
	int	retval;
	char buf[BUFSIZ];
	char	*cp;
	int n;

	if (debug) {
		LLOG (pgm_log, LLOG_NOTICE, ("searching for %s %s",
					     ak -> ak_domain, ak -> ak_local));
		pslog (pgm_log, LLOG_NOTICE, "  using baseobjects ",
		       dn_seq_print, (caddr_t) ak -> ak_bases);
	}

	sk[0].type = MAILBOX;
	(void) sprintf (buf, "%s@%s", ak -> ak_local, ak -> ak_domain);
	sk[0].value = strdup(buf);
	sk[1].type = USERID;
	sk[1].value = strdup(ak -> ak_local);
	sk[2].type = OTHERMAILBOX;
	(void) sprintf (buf, "internet$%s@%s",
			ak -> ak_local, ak -> ak_domain);
	sk[2].value = strdup(buf);
	sk[3].type = sk[3].value = NULLCP;

	retval = NOTOK;
	for (dlist = ak -> ak_bases; dlist; dlist = dlist -> dns_next) {
		if ((retval = do_search (dlist -> dns_dn,
					 sk, imp)) != DONE)
			break;
	}
	for (n = 0; sk[n].type; n++)
		free (sk[n].value);

	if (retval != DONE)
		return retval;

	/* OK - try some more heuristics */
	n = 0;
	sk[n].type = COMMONNAME;
	sk[n++].value = strdup(ak -> ak_local);

	if ((cp = rindex(ak -> ak_local, '.')) != NULL) {
		(void) strcpy (buf, ak -> ak_local); /* try replacing '.'->' ' */
		for (cp = buf; *cp; cp++)
			if (*cp == '.')
				*cp = ' ';
		sk[n].type = COMMONNAME;
		sk[n++].value = strdup(buf);
	}
	sk[n].type = sk[n].value = NULLCP;

	retval = NOTOK;

	for (dlist = ak -> ak_bases; dlist; dlist = dlist -> dns_next) {
		if ((retval = do_search (dlist -> dns_dn,
					 sk, imp)) != DONE)
			break;
	}
	if (retval != DONE)
		return retval;

	n = 0;
	sk[n].type = SURNAME;
	sk[n++].value = strdup(ak -> ak_local);
	if ((cp = rindex(ak -> ak_local, '.')) != NULL) {
		sk[n].type = SURNAME; /* try last component as surname */
		sk[n++].value = cp + 1;
	}
	sk[n].type = sk[n].value = NULLCP;

	retval = NOTOK;

	for (dlist = ak -> ak_bases; dlist; dlist = dlist -> dns_next) {
		if ((retval = do_search (dlist -> dns_dn,
					 sk, imp)) != DONE)
			break;
	}
	return retval;
}

static AttributeType t_photo, t_phone, t_address, t_description, t_info;

struct pairs {
	char *p_name;
	AttributeType *p_at;
} pairs[] = {
	PHOTO,			&t_photo,
	"telephoneNumber",	&t_phone,
	"postalAddress",	&t_address,
	"description",		&t_description,
	"info",			&t_info,
	NULL,
};

static char *as2str (as)
Attr_Sequence as;
{
	static PS ps;
	char buffer[BUFSIZ];

	if (ps == NULLPS) {
		if ((ps = ps_alloc (str_open)) == NULLPS)
			adios (NULLCP, "Out of memory");
	}
	if (str_setup (ps, buffer, sizeof buffer, 1) == NOTOK)
		adios (NULLCP, "Can't setup PS");

	avs_print (ps, as -> attr_value, READOUT);
	ps_print (ps, " ");
	*--ps -> ps_ptr = NULL, ps -> ps_cnt ++;

	return strdup (buffer);
}

static Attr_Sequence get_select ()
{
	static Attr_Sequence as = NULL;

	if (!as) {
		struct pairs *p;
		AttributeType at;

		for (p = pairs; p -> p_name; p++) {
			at = *p -> p_at = AttrT_new (p -> p_name);

			if (at)
				as = as_merge (as,
					       as_comp_new (AttrT_cpy(at),
							    NULLAV,
							    NULLACL_INFO));
		}
	}

	return as;
}

decode_result (imp, eptr)
struct image *imp;
Attr_Sequence eptr;
{

	for (; eptr; eptr = eptr -> attr_link) {
		if (AttrT_cmp (eptr -> attr_type, t_photo) == 0)
			imp -> pe =
				pe_cpy (grab_pe (eptr -> attr_value
						 -> avseq_av));
		else if (AttrT_cmp (eptr -> attr_type, t_info) == 0)
			imp -> info = as2str (eptr);
		else if (AttrT_cmp (eptr -> attr_type, t_description) == 0)
			imp -> description = as2str (eptr);
		else if (AttrT_cmp (eptr -> attr_type, t_address) == 0)
			imp -> address = as2str (eptr);
		else if (AttrT_cmp (eptr -> attr_type, t_phone) == 0)
			imp -> phone = as2str (eptr);
	}
}

static int do_search (dn, sk, imp)
DN	dn;
SearchKeys sk[];
struct image *imp;
{
	struct ds_search_arg search_arg;
	register struct ds_search_arg *sa = &search_arg;
	struct ds_search_result search_result;
	register CommonArgs *ca;
	register struct ds_search_result *sr = &search_result;
	struct DSError error;
	register struct DSError *se = &error;
	extern int dn_print();
	extern char *dn2ufn ();
	SearchKeys *skp;
	s_filter *fi;

	bzero ((char *) sa, sizeof *sa);

	ca = &sa -> sra_common;
	ca -> ca_servicecontrol.svc_options = SVC_OPT_PREFERCHAIN;
	ca -> ca_servicecontrol.svc_timelimit = DEF_TIMELIMIT;
	ca -> ca_servicecontrol.svc_sizelimit = 1;

	sa -> sra_subset = SRA_WHOLESUBTREE;
	sa -> sra_searchaliases = TRUE;

	if ((sa -> sra_baseobject = dn) == NULL)
		return DONE;

	sa -> sra_filter = fi = filter_alloc ();

	bzero ((char *) fi, sizeof *fi);
	fi -> flt_type = FILTER_OR;

	for (skp = sk; skp -> type; skp ++) {
		s_filter *f;
		AttributeType at;

		f = filter_alloc ();
		bzero ((char *) f, sizeof *f);

		f -> flt_type = FILTER_ITEM;
		f -> FUITEM.fi_type = FILTERITEM_EQUALITY;
		if ((f -> FUITEM.UNAVA.ava_type = at =
		     AttrT_new (skp -> type)) == NULL)
			adios ("invalid attribute type \"%s\"", USERID);
		f -> FUITEM.UNAVA.ava_value =
			str2AttrV (skp -> value, at -> oa_syntax);
		f -> flt_next = fi -> FUFILT;
		fi -> FUFILT = f;
	}

	sa -> sra_eis.eis_allattributes = FALSE;
	sa -> sra_eis.eis_select = get_select ();
	sa -> sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;

	if (debug) {
		extern int fi_print ();

		pslog (pgm_log, LLOG_NOTICE, "Searching for ",
		       fi_print, (caddr_t)fi);
		pslog (pgm_log, LLOG_NOTICE, " from  ",
		       dn_print, (caddr_t) dn);
	}
	if (ds_search (sa, se, sr) != DS_OK) {
		if (debug)
			LLOG (pgm_log, LLOG_EXCEPTIONS,
			      ("search operation failed (%d)",
			       se -> dse_type));

		switch (se -> dse_type) {
		    case DSE_REMOTEERROR:
			(void) ds_unbind ();
			bound = 0;
			break;
		    default:
			break;
		}

		filter_free (sa -> sra_filter);
		return bound == 0 ? NOTOK : DONE;
	}

	if (sr -> srr_correlated != TRUE)
		correlate_search_results (sr);

	switch (sr -> CSR_limitproblem) {
	    case LSR_NOLIMITPROBLEM:
		break;
	    case LSR_TIMELIMITEXCEEDED:
		if (debug)
			LLOG (pgm_log, LLOG_NOTICE,
			      ("took tooo long"));
		/* fall */
	    case LSR_ADMINSIZEEXCEEDED:
		filter_free (sa -> sra_filter);
		return NOTOK;

	    case LSR_SIZELIMITEXCEEDED:
		if (debug)
			pslog (pgm_log, LLOG_NOTICE,
			       "search: hit too many targets at ",
			       dn_print, (caddr_t)dn);
		break; /* carry on anyway */
	}
		
	if (sr -> CSR_entries == NULLENTRYINFO) {
		if (debug)
			pslog (pgm_log, LLOG_NOTICE,
			       "search failed at baseobject ",
			       dn_print, (caddr_t) dn);
		filter_free (sa -> sra_filter);
		return DONE;
	}

	if (sr -> CSR_entries -> ent_attr == NULLATTR) {
		if (debug)
			pslog (pgm_log, LLOG_NOTICE,
			       "search succeeded (but no attribute) at ",
			       dn_print, (caddr_t) dn);
	}
	else {
		if (debug)
			pslog (pgm_log, LLOG_NOTICE,
			       "search succeeded at ",
			       dn_print, (caddr_t) dn);
		decode_result (imp, sr -> CSR_entries -> ent_attr);
	}

	imp -> ufnname = dn2ufn (sr ->CSR_entries -> ent_dn, -1);

	dn_free (sr -> CSR_object);
	entryinfo_free (sr -> CSR_entries, 0);
	crefs_free (sr -> CSR_cr);
	filter_free (sa -> sra_filter);
	return OK;

}
/*  */

static int do_bind (dn, passwd)
char	*dn;
char *passwd;
{
    struct ds_bind_arg bind_arg,
		       bind_result;
    register struct ds_bind_arg *ba = &bind_arg,
				*br = &bind_result;
    struct ds_bind_error bind_error;
    register struct ds_bind_error *be = &bind_error;
    static char *savedn, *savepasswd;

    bzero ((char *) ba, sizeof *ba);
    ba -> dba_version = DBA_VERSION_V1988;
    ba -> dba_auth_type = DBA_AUTH_NONE;
    if (dn || (dn = savedn) != NULLCP) {
	    if (savedn == NULLCP)
		    savedn = strdup (dn);
	    ba -> dba_dn = str2dn (dn);
	    if (passwd || (passwd=savepasswd) != NULLCP) {
		    if (savepasswd == NULLCP)
			    savepasswd = strdup (passwd);
		    (void) strcpy (ba -> dba_passwd, passwd);
		    ba -> dba_auth_type = DBA_AUTH_SIMPLE;
	    }
    }
		    

    if (ds_bind (ba, be, br) != DS_OK) {
	if (debug)
	    LLOG (pgm_log, LLOG_EXCEPTIONS,
		  ("directory bind failed: %s",
		   be -> dbe_type == DBE_TYPE_SECURITY ? "security error"
						       : "DSA unavailable"));

	return NOTOK;
    }

    bound = 1;

    return OK;
}

/*    IMAGE */

static int    passno;
static int    x, y, maxx;

static struct image *the_image = NULL;

/*  */
void image_free (imp)
struct image *imp;
{
	if (imp -> data)	qb_free(imp ->data);
	if (imp -> ufnname)	free (imp->ufnname);
	if (imp -> phone)	free (imp -> phone);
	if (imp -> description)	free (imp -> description);
	if (imp -> info)	free (imp -> info);
	if (imp -> address)	free (imp -> address);
	if (imp -> pe)		pe_free (imp->pe);
	free ((char *)imp);
}

struct image *fetch_image (local, domain)
char   *local,
       *domain;
{
	register struct aka *ak;
	time_t now;

	(void) time(&now);

	if ((ak = mbox2ak (local, domain)) == NULL)
		return NULL;

	if (ak -> ak_image)
		return ak -> ak_image;
	if (ak -> ak_notfound > now - 30*60) {
		if (debug)
			LLOG (pgm_log, LLOG_NOTICE, ("Hit negative cache!"));
		return NULL; /* neg cache */
	}
	if (!bound && do_bind (NULLCP, NULLCP) == NOTOK)
		return NULL;

	if ((the_image = (struct image *) calloc (1, sizeof *the_image)) == NULL) {
		if (debug)
			fprintf (stderr, "calloc fails");
		return NULL;
	}
	if (image_search (ak, the_image) == NOTOK) {
		ak -> ak_notfound = now;
		return NULL;
	}

	if (!stay_bound) {
		(void) ds_unbind ();
		bound = 0;
	}

	if (the_image -> pe)
		dec_photo (the_image);
	return (ak -> ak_image = the_image);
}


dec_photo (imp)
struct image *imp;
{
	PS ps;
	PE pe = imp -> pe;

	imp -> pe = NULLPE;

	if ((ps = ps_alloc (str_open)) == NULLPS) {
		if (debug)
			fprintf (stderr, "ps_alloc: failed\n");

		goto out;
	}
	if (str_setup (ps, NULLCP, 0, 0) == NOTOK) {
		if (debug)
			fprintf (stderr, "ps_alloc: %s\n",
				 ps_error (ps -> ps_errno));

		goto out;
	}
    
	if (pe2ps (ps, pe) == NOTOK) {
		if (debug)
			fprintf (stderr, "pe2ps: %s\n",
				 pe_error (pe -> pe_errno));

		goto out;
	}

	for (passno = 1; passno < 3; passno++)
		if (decode_t4 (ps -> ps_base, PHOTO,
			       ps -> ps_ptr - ps -> ps_base)
		    == NOTOK) {
			fprintf (stderr, "\n");
			if (imp -> data) {
				qb_free (imp -> data);
				imp -> data = NULL;
			}
			break;
		}

    out: ;
	if (ps)
		ps_free (ps);
	pe_free (pe);
}
/*  */

/* ARGSUSED */

photo_start (name)
char   *name;
{
    if (passno == 1)
	maxx = 0;
    x = y = 0;

    return OK;
}


/* ARGSUSED */

photo_end (name)
char   *name;
{
	int	    len;
	register struct qbuf *qb,
	*pb;
    
	if (passno == 1) {
		x = maxx, y--;

		if (debug)
			fprintf (stderr, "ending pass one for \"%s\", %dx%d\n",
				 name, x, y);

		the_image -> width = x, the_image -> height = y;

		len = ((the_image -> width + 7) / 8) * the_image -> height;
		if ((the_image -> data = qb = (struct qbuf *) malloc (sizeof *qb)) == NULL) {
			if (debug)
				fprintf (stderr, "unable to allocate qbuf");
			return NOTOK;
		}
		qb -> qb_forw = qb -> qb_back = qb;
		qb -> qb_data = NULL, qb -> qb_len = len;
	
		if ((pb = (struct qbuf *) calloc ((unsigned) (sizeof *pb + len), 1))
		    == NULL) {
			if (debug)
				fprintf (stderr, "unable to allocate qbuf (%d+%d)",
					 sizeof *qb, len);
			return NOTOK;
		}
		pb -> qb_data = pb -> qb_base, pb -> qb_len = len;
		insque (pb, qb);
		return OK;
	}
	return OK;
}


photo_black (length)
int	length;
{
    if (passno == 2) {
	register int    i,
			j;
	register unsigned char *cp;

	cp = (unsigned char *) the_image -> data -> qb_forw -> qb_data
				       + ((the_image -> width + 7) / 8) * y + x / 8;
	i = x % 8;
	for (j = length; j > 0; j--) {
	    *cp |= 1 << i;
	    if (++i == 8)
		cp++, i = 0;
	}

    }

    x += length;

    return OK;
}


photo_white (length)
int	length;
{
    x += length;

    return OK;
}


/* ARGSUSED */

photo_line_end (line)
caddr_t line;
{
    if (passno == 1 && x > maxx)
	maxx = x;
    x = 0, y++;

    return OK;
}

/*    ERRORS */

#ifndef	lint
void	_advise ();


void	adios (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _advise (ap);

    va_end (ap);

    _exit (1);
}
#else
/* VARARGS */

void	adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif


#ifndef	lint
void	advise (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _advise (ap);

    va_end (ap);
}


static void  _advise (ap)
va_list	ap;
{
    char    buffer[BUFSIZ];

    asprintf (buffer, ap);

    (void) fflush (stdout);

    if (errsw != NOTOK) {
	fprintf (stderr, "%s: ", myname);
	(void) fputs (buffer, stderr);
	(void) fputc ('\n', stderr);

	(void) fflush (stderr);
    }    
}
#else
/* VARARGS */

void	advise (what, fmt)
char   *what,
       *fmt;
{
    advise (what, fmt);
}
#endif

#endif
