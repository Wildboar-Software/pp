/* dl_x500.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl_x500.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl_x500.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: dl_x500.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include <isode/quipu/dua.h>
#include <isode/quipu/read.h>
#include <isode/quipu/modify.h>
#include <isode/quipu/add.h>
#include <isode/quipu/entry.h>
#include <isode/quipu/ds_search.h>
#include <isode/quipu/connection.h>	/* ds_search uses di_block - include this for lint !!! */
#include <isode/quipu/bind.h>

extern AttributeType at_Member;
extern AttributeType at_RoleOccupant;
extern AttributeType at_CommonName;
extern AttributeType at_Owner;
extern AttributeType at_Permit;
extern AttributeType at_Policy;
extern AttributeType at_ORAddress;
extern AttributeType at_RFC822;
extern AttributeType at_GroupMember;
extern AttributeType at_ObjectClass;
extern OID role_oc;
extern char * local_dit;

extern LLog * log_dsap;

extern Filter strfilter(), ocfilter(), joinfilter();
extern Attr_Sequence as_combine();
extern DN dn_append();
extern char * dn2str();

static Attr_Sequence mailas = NULLATTR;

extern int str2dl ();

static int isPermanentError (error)
struct DSError	*error;
{
/* DONE == temporary, NOTOK == permanent */

	switch (error -> dse_type) {
	    case DSE_REMOTEERROR:
	    case DSE_REFERRAL:
	    case DSE_ABANDONED:
		return DONE;
	    case DSE_SERVICEERROR:
		switch (error -> ERR_SERVICE.DSE_sv_problem) {
		    case DSE_SV_BUSY:
		    case DSE_SV_TIMELIMITEXCEEDED:
		    case DSE_SV_ADMINLIMITEXCEEDED:
			return DONE;
		    default:
			return NOTOK;
		}
	    default:
		return NOTOK;
	}
}

static int ds_cache_read (dn,as, pres) 
DN dn;
Attr_Sequence as;
Attr_Sequence *pres;
{
struct ds_read_arg read_arg;
struct ds_read_result result;
struct DSError  error;
static CommonArgs ca = default_common_args;
Entry ptr;

	if ((ptr = local_find_entry (dn,TRUE)) != NULLENTRY) {
		if (ptr->e_complete) {
			*pres = ptr->e_attributes;
			return OK;
		}
		if (as != NULLATTR) {
			Attr_Sequence tmp;
			for (tmp = as; tmp!= NULLATTR; tmp = tmp->attr_link) 
				if (as_find_type(ptr->e_attributes,tmp->attr_type) == NULLATTR) 
					goto do_read;
			*pres = ptr->e_attributes;
			return OK;
		}
	}

do_read:;

	read_arg.rda_common = ca; /* struct copy */

	read_arg.rda_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	read_arg.rda_eis.eis_allattributes = FALSE;
	read_arg.rda_eis.eis_select = as;
	read_arg.rda_object = dn;
	
	if (ds_read (&read_arg, &error, &result) != DS_OK) {
		int	resultint = isPermanentError(&error);
		log_ds_error (&error);
		ds_error_free (&error);
		*pres = NULLATTR;
		return resultint;
	}

	result.rdr_entry.ent_attr = as_merge (result.rdr_entry.ent_attr,as_cpy(as));

	cache_entry (&result.rdr_entry,FALSE,TRUE);
        *pres = result.rdr_entry.ent_attr;
	return OK;
}


int dn2addr (dn, pres) 
DN dn;
Attr_Sequence	*pres;
{
Attr_Sequence found;
int	result;

	if (mailas == NULLATTR) {
		Attr_Sequence as;
		mailas = as_comp_new (at_ORAddress,  NULLAV, NULLACL_INFO);
		as = as_comp_new (at_RFC822,  NULLAV, NULLACL_INFO);
		mailas = as_merge (mailas,as);
		as = as_comp_new (at_ObjectClass,  NULLAV, NULLACL_INFO);
		mailas = as_merge (mailas,as);
		as = as_comp_new (at_RoleOccupant,  NULLAV, NULLACL_INFO);
		mailas = as_merge (mailas,as);
	}

	if ((result = ds_cache_read (dn,mailas, pres)) != OK)
		return result;

	/* Have we got an address ? */
	if ((found = as_find_type (*pres,at_ORAddress)) != NULLATTR) 
		return OK;

	if ((found = as_find_type (*pres,at_RFC822)) != NULLATTR) 
		return OK;

	/* NO address, is it a role ? */
	if ((found = as_find_type (*pres,at_ObjectClass)) == NULLATTR) 
		return NOTOK;

	if (check_in_oc(role_oc,found->attr_value)) {
		/* Its a role :- follow member... */
		if ((found = as_find_type (*pres,at_RoleOccupant)) == NULLATTR) 
			return NOTOK;

		return dn2addr ((DN)&found->attr_value->avseq_av, pres);
	}

	return OK;
	
}


int dir_getdl_aux (dn, pres)
DN dn;
Attr_Sequence	*pres;
{
static Attr_Sequence astop = NULLATTR;
int	result;

	if (astop == NULLATTR) {
		Attr_Sequence as;
		astop = as_comp_new (at_Member,  NULLAV, NULLACL_INFO);
		as = as_comp_new (at_Owner,  NULLAV, NULLACL_INFO);
		astop = as_merge (astop,as);
		as = as_comp_new (at_Permit,  NULLAV, NULLACL_INFO);
		astop = as_merge (astop,as);
		as = as_comp_new (at_Policy,  NULLAV, NULLACL_INFO);
		astop = as_merge (astop,as);
/*
		as = as_comp_new (acl_at,  NULLAV, NULLACL_INFO);
		astop = as_merge (astop,as);
*/
	}
	result = ds_cache_read (dn,astop,pres);
	return result;
}

int dir_getdl (list, pas)
char * list;
Attr_Sequence *pas;
{
	DN dn;
	int	result;
	static  DN localdn = NULLDN;

	if ((localdn == NULLDN) && local_dit)
		localdn = str2dn (local_dit);

	if ((result = str2dl(list,localdn,&dn)) != OK) 
		return result;

	result = dir_getdl_aux (dn, pas);
	dn_free (dn);

	return result;
} 

int read_group_entry (dn, pres)
DN dn;
Attr_Sequence	*pres;
{
static Attr_Sequence astop = NULLATTR;

	if (astop == NULLATTR) 
		astop = as_comp_new (at_GroupMember,  NULLAV, NULLACL_INFO);

	return (ds_cache_read (dn,astop, pres));
}


or_modify (ps,old,newname,at,dn) 
PS ps;
ORName * old;
OR_ptr newname;
AttributeType at;
DN dn;
{
struct ds_modifyentry_arg mod_arg;
struct DSError  error;
static CommonArgs ca = default_common_args;
struct entrymod *emnew;
AttributeValue av;
AV_Sequence avs;
ORName * newor;
ORName * orName_cpy();
OR_ptr or_cpy();

	mod_arg.mea_common = ca;
	mod_arg.mea_object = dn;

	emnew = em_alloc ();
	emnew->em_type = EM_ADDVALUES;
	av = AttrV_alloc();
	av->av_syntax = str2syntax("ORName");
	newor = orName_cpy (old);
	if (newor->on_or != NULLOR)
		or_free (newor->on_or);
	newor->on_or = or_cpy(newname);
	av->av_struct = (caddr_t) newor;
	avs = avs_comp_new(av);
	emnew->em_what = as_comp_new (AttrT_cpy(at),avs,NULLACL_INFO);

	mod_arg.mea_changes = emnew;

 	emnew = em_alloc ();
	emnew->em_type = EM_REMOVEVALUES;
	av = AttrV_alloc();
	av->av_syntax = str2syntax("ORName");
	av->av_struct = (caddr_t) orName_cpy(old);
	avs = avs_comp_new(av);
	emnew->em_what = as_comp_new (AttrT_cpy(at),avs,NULLACL_INFO);
	emnew->em_next = NULLMOD;

	mod_arg.mea_changes->em_next = emnew;

	if (ds_modifyentry (&mod_arg, &error) != DS_OK) {
		ds_error (ps, &error);
		return FALSE;
	}
	delete_cache (old->on_dn);

	ems_free (mod_arg.mea_changes);

	return TRUE;
}

dl_modify (ps,name,dn,delete) 
PS ps;
ORName * name;
DN dn;
char delete;
{
struct ds_modifyentry_arg mod_arg;
struct DSError  error;
static CommonArgs ca = default_common_args;
struct entrymod *emnew;
AttributeValue av;
AV_Sequence avs;

	mod_arg.mea_common = ca;
	mod_arg.mea_object = dn;

	emnew = em_alloc ();
	if (delete)
		emnew->em_type = EM_REMOVEVALUES;
	else
		emnew->em_type = EM_ADDVALUES;
	av = AttrV_alloc();
	av->av_syntax = str2syntax("ORName");
	av->av_struct = (caddr_t) name;
	avs = avs_comp_new(av);
	emnew->em_what = as_comp_new (AttrT_cpy(at_Member),avs,NULLACL_INFO);
	emnew->em_next = NULLMOD;

	mod_arg.mea_changes = emnew;

	if (ds_modifyentry (&mod_arg, &error) != DS_OK) {
		ds_error (ps, &error);
		return FALSE;
	}
	delete_cache (dn);
	ems_free (mod_arg.mea_changes);

	return TRUE;
}

dl_modify_attr (ps,oldas,newas,dl,quiet) 
PS ps;
Attr_Sequence oldas,newas;
DN dl;
char quiet;
{
struct ds_modifyentry_arg mod_arg;
struct DSError  error;
static CommonArgs ca = default_common_args;
struct entrymod *emnew;

	mod_arg.mea_common = ca;
	mod_arg.mea_object = dl;

	if ((! oldas) || (oldas->attr_value == NULLAV)) {
		emnew = em_alloc ();
		emnew->em_type = EM_ADDATTRIBUTE;
		emnew->em_what = as_comp_cpy (newas);
      	 } else {
		emnew = em_alloc ();
		emnew->em_type = EM_REMOVEATTRIBUTE;
		emnew->em_what = as_comp_cpy (oldas);
		emnew->em_next = em_alloc();
		emnew->em_next->em_type = EM_ADDATTRIBUTE;
		emnew->em_next->em_what = as_comp_cpy (newas);
       	}

	emnew->em_next->em_next = NULLMOD;

	mod_arg.mea_changes = emnew;

	if (!quiet) {
		ps_print (ps, "modifying...");
		ps_flush (ps);
	}

	if (ds_modifyentry (&mod_arg, &error) != DS_OK) {
		ds_error (ps, &error);
		return FALSE;
	}

	if (!quiet) {
		ps_print (ps, "OK\n");
		ps_flush (ps);
	}

	delete_cache (dl);
	ems_free (mod_arg.mea_changes);

	return TRUE;
}

dl_modify_owner (ps,olddn,newdn,dl,quiet) 
PS ps;
DN olddn,newdn;
DN dl;
char quiet;
{
struct ds_modifyentry_arg mod_arg;
struct DSError  error;
static CommonArgs ca = default_common_args;
struct entrymod *emnew;
AttributeValue av;
AV_Sequence avs;

	mod_arg.mea_common = ca;
	mod_arg.mea_object = dl;

	emnew = em_alloc ();
	emnew->em_type = EM_REMOVEATTRIBUTE;
	av = AttrV_alloc();
	av->av_syntax = str2syntax("DN");
	av->av_struct = (caddr_t) dn_cpy(olddn);
	avs = avs_comp_new(av);
	emnew->em_what = as_comp_new (AttrT_cpy(at_Owner),avs,NULLACL_INFO);

	emnew->em_next = em_alloc();
	emnew->em_next->em_type = EM_ADDATTRIBUTE;
	av = AttrV_alloc();
	av->av_syntax = str2syntax("DN");
	av->av_struct = (caddr_t) dn_cpy(newdn);
	avs = avs_comp_new(av);
	emnew->em_next->em_what = as_comp_new (AttrT_cpy(at_Owner),avs,NULLACL_INFO);
	emnew->em_next->em_next = NULLMOD;

	mod_arg.mea_changes = emnew;

	if (!quiet) {
		ps_print (ps, "modifying...");
		ps_flush (ps);
	}

	if (ds_modifyentry (&mod_arg, &error) != DS_OK) {
		ds_error (ps, &error);
		return FALSE;
	}

	if (!quiet) {
		ps_print (ps, "OK\n");
		ps_flush (ps);
	}

	delete_cache (dl);
	ems_free (mod_arg.mea_changes);

	return TRUE;
}

dl_bind (name,passwd)
DN name;
char * passwd;
{
  struct ds_bind_arg bindarg;
  struct ds_bind_arg bindresult;
  struct ds_bind_error binderr;

  bindarg.dba_version = DBA_VERSION_V1988;

  bindarg.dba_passwd_len = 0;
  bindarg.dba_passwd [0] = '\0';

  if ((bindarg.dba_dn = name) != NULLDN) {
	if (passwd) {
	    bindarg.dba_passwd_len = strlen (passwd);
	    (void) strcpy (bindarg.dba_passwd,passwd);
        }
  }

  if (ds_bind (&bindarg,&binderr,&bindresult) != DS_OK)
    	return NOTOK;
  else
	return OK;
}


dl_unbind ()
{
	(void) ds_unbind ();
}


str2dl (str,localdn,res)
char * str;
DN localdn;
DN * res;
{
extern int print_parse_errors;
int old;
struct ds_search_arg search_arg;
static struct ds_search_result result;
struct DSError err;
static CommonArgs ca = default_common_args;
Filter filtcn, filtoc, filtand;

	old = print_parse_errors;
	print_parse_errors = FALSE;
	*res = str2dn (str);
	print_parse_errors = old;

	if (*res != NULLDN)
		return OK;

	filtcn = strfilter (at_CommonName,str,FILTERITEM_EQUALITY);
	if ((filtoc = ocfilter ("ppDistributionList")) == NULLFILTER)
		return NOTOK;

	filtoc->flt_next = filtcn;
	filtand = joinfilter (filtoc,FILTER_AND);

	search_arg.sra_baseobject = localdn;
	search_arg.sra_filter = filtand;
	search_arg.sra_subset = SRA_ONELEVEL;
	search_arg.sra_searchaliases = FALSE;
	search_arg.sra_common = ca; /* struct copy */
	search_arg.sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	search_arg.sra_eis.eis_allattributes = FALSE;
	search_arg.sra_eis.eis_select = NULLATTR;

	if (ds_search (&search_arg, &err, &result) != DS_OK) {
		int	resultint = isPermanentError(&err);
		filter_free (filtoc);
		log_ds_error (&err);
		ds_error_free (&err);
		return resultint;
	}
	filter_free (filtoc);

	if (result.CSR_entries) {
		if (*res = result.CSR_entries->ent_dn) /* assign */
			return OK;
	} 

	return NOTOK;
	
}


DN search_postmaster (localdn)
DN localdn;
{
struct ds_search_arg search_arg;
static struct ds_search_result result;
struct DSError err;
static CommonArgs ca = default_common_args;
Filter filtcn;

	if (mailas == NULLATTR) {
		Attr_Sequence as;
		mailas = as_comp_new (at_ORAddress,  NULLAV, NULLACL_INFO);
		as = as_comp_new (at_RFC822,  NULLAV, NULLACL_INFO);
		mailas = as_merge (mailas,as);
		as = as_comp_new (at_ObjectClass,  NULLAV, NULLACL_INFO);
		mailas = as_merge (mailas,as);
		as = as_comp_new (at_RoleOccupant,  NULLAV, NULLACL_INFO);
		mailas = as_merge (mailas,as);
	}

	filtcn = strfilter (at_CommonName,"PostMaster",FILTERITEM_EQUALITY);

	search_arg.sra_baseobject = localdn;
	search_arg.sra_filter = filtcn;
	search_arg.sra_subset = SRA_ONELEVEL;
	search_arg.sra_searchaliases = TRUE;
	search_arg.sra_common = ca; /* struct copy */
	search_arg.sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	search_arg.sra_eis.eis_allattributes = FALSE;
	search_arg.sra_eis.eis_select = mailas;

	if (ds_search (&search_arg, &err, &result) != DS_OK) {
		filter_free (filtcn);
		log_ds_error (&err);
		ds_error_free (&err);
		return NULLDN;
	}
	filter_free (filtcn);

	if (result.CSR_entries) {
		cache_entry (result.CSR_entries,FALSE,TRUE);
		return (result.CSR_entries->ent_dn);
	} 

	return NULLDN;
}
	
dl_showentry(ps,dn)
PS ps;
DN dn;
{
struct ds_read_arg read_arg;
struct ds_read_result result;
struct DSError  error;
static CommonArgs ca = default_common_args;
Entry ptr;

	if ((ptr = local_find_entry (dn,TRUE)) != NULLENTRY) 
		if (ptr->e_complete) {
			as_print (ps,ptr->e_attributes,READOUT);
			return;
		}

	read_arg.rda_common = ca; /* struct copy */

	read_arg.rda_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	read_arg.rda_eis.eis_allattributes = TRUE;
	read_arg.rda_eis.eis_select = NULLATTR;
	read_arg.rda_object = dn;
	
	if (ds_read (&read_arg, &error, &result) != DS_OK) 
		ds_error(ps,&error);

	cache_entry (&result.rdr_entry,FALSE,TRUE);

	as_print (ps,result.rdr_entry.ent_attr,READOUT);
}

add_list_request (ps,where, listname, owner)
PS ps;
DN where;
char * listname;
DN owner;
{
struct ds_addentry_arg add_arg;
struct DSError  error;
Attr_Sequence as,nas;
char buffer [LINESIZE];
DN obj;
DN dnc;
static CommonArgs ca = default_common_args;

	add_arg.ada_common = ca; /* struct copy */

	(void) sprintf (buffer, "cn=%s-request",listname);
	if ((as = str2as (buffer)) == NULLATTR)
		return FALSE;
	if ((dnc = str2dn (buffer)) == NULLDN)
		return FALSE;

	(void) sprintf (buffer, "ObjectClass=ppRole&quipuobject");
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	(void) sprintf (buffer, "roleOccupant=%s",dn2str(owner));
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	(void) sprintf (buffer, "mhsORAddresses=%s-request",listname);
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	(void) sprintf (buffer, 
	     "acl=others # read # entry & others # read # default & group #");
	strcat (buffer, dn2str(owner));
	strcat (buffer, "# write # entry & group #");
	strcat (buffer, dn2str(owner));
	strcat (buffer, "# write # default");
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	obj = dn_cpy (where);
	dn_append (obj,dn_cpy(dnc));
	add_arg.ada_object = obj;
	add_arg.ada_entry = as;

	if (ds_addentry (&add_arg, &error) != DS_OK) {
		ds_error (ps,&error);
		dn_free (obj);
		as_free (as);
		return FALSE;
	}

	dn_free (obj);
	as_free (as);

	return TRUE;
}

add_new_list (ps, where,listname, owner, description,members)
PS ps;
DN where;
char * listname;
DN owner;
char *description;
Attr_Sequence members;
{
struct ds_addentry_arg add_arg;
struct DSError  error;
Attr_Sequence as,nas;
char buffer [LINESIZE];
DN obj;
DN dnc;
static CommonArgs ca = default_common_args;

	add_arg.ada_common = ca; /* struct copy */

	(void) sprintf (buffer, "cn=%s",listname);
	if ((as = str2as (buffer)) == NULLATTR)
		return FALSE;
	if ((dnc = str2dn (buffer)) == NULLDN)
		return FALSE;

	(void) sprintf (buffer, "ObjectClass=ppDistributionList&quipuobject");
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	(void) sprintf (buffer, "owner=%s@cn=%s-request",dn2str(where),listname);
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	if (members) 
		as = as_merge (as,members);
	else {
		(void) sprintf (buffer, "mhsDLmembers=%s",dn2str(owner));
		if ((nas = str2as (buffer)) == NULLATTR)
			return FALSE;
		as = as_merge (as,nas);
        }

	if (description && *description != 0) {
		(void) sprintf (buffer, "description=%s",description);
		if ((nas = str2as (buffer)) == NULLATTR)
			return FALSE;
		as = as_merge (as,nas);
	}

	(void) sprintf (buffer, "mhsDLSubmitPermissions=ALL");
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	(void) sprintf (buffer, 
	     "acl=others # read # entry & others # read # default & group #");
	strcat (buffer, dn2str(owner));
	strcat (buffer, "# write # entry & group #");
	strcat (buffer, dn2str(owner));
	strcat (buffer, "# write # default");
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	(void) sprintf (buffer, "mhsORAddresses=%s",listname);
	if ((nas = str2as (buffer)) == NULLATTR)
		return FALSE;
	as = as_merge (as,nas);

	obj = dn_cpy (where);
	dn_append (obj,dn_cpy(dnc));
	add_arg.ada_object = obj;
	add_arg.ada_entry = as;

	if (ds_addentry (&add_arg, &error) != DS_OK) {
		ds_error (ps,&error);
		dn_free (obj);
		as_free (as);
		return FALSE;
	}

	dn_free (obj);
	as_free (as);

	return TRUE;
}



