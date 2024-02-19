/* dl_util.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl_util.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl_util.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: dl_util.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include "retcode.h"
#include "adr.h"
#include "or.h"
#include <isode/quipu/attrvalue.h>

extern AttributeType at_Member;
extern AttributeType at_Owner;
extern AttributeType at_Permit;
extern AttributeType at_Policy;
extern AttributeType at_ORAddress;
extern AttributeType at_RFC822;
extern AttributeType at_RoleOccupant;
extern OID dl_oc;

extern OR_ptr orAddr_parse ();

Attr_Sequence current_print = NULLATTR;

extern LLog * log_dsap;

ORName * addr2orname (addr)
ADDR * addr;
{
ORName * or;
OR_ptr por = NULLOR;

	or = (ORName *) smalloc (sizeof (ORName));
	or->on_dn = NULLDN;
	or->on_or = NULLOR;

	if (addr->ad_dn)
		or->on_dn = str2dn (addr->ad_dn);

	if (addr->ad_value) {
		
		if (addr->ad_r822adr)
			or_rfc2or(addr->ad_r822adr,&por);
		else
			por = or_std2or(addr->ad_value);
		
		if (por)
			or->on_or = or_default(por);
	}

	return (or);
}


OR_ptr as2or (as)
Attr_Sequence as;
{
Attr_Sequence tmp;
OR_ptr or_cpy ();

	if ((tmp = as_find_type(as,at_ORAddress)) != NULLATTR) 
		if (tmp->attr_value)
			return (or_cpy ((OR_ptr) tmp->attr_value->avseq_av.av_struct));
	if ((tmp = as_find_type(as,at_RFC822)) != NULLATTR) {
		OR_ptr or;
		if (tmp->attr_value)
			if (or_rfc2or((char *)tmp->attr_value->avseq_av.av_struct,&or) == OK)
				return (or);
	}

	return NULLOR;
}

avs_seq_print (ps,avs,format) 
PS   ps;
AV_Sequence  avs;
int  format;
{
AV_Sequence eptr;

	for(eptr = avs; eptr != NULLAV; eptr = eptr->avseq_next) {
		ps_print (ps,"  ");
		AttrV_print (ps,&eptr->avseq_av,format);
		ps_print (ps,"\n");
	}
}

static int member_compar (a, b)
AV_Sequence *a, *b;
{
    ORName  * aor = (ORName *)((*a)->avseq_av.av_struct);
    ORName  * bor = (ORName *)((*b)->avseq_av.av_struct);

    return orName_cmp (aor,bor);

}

static avs_seq_print_members (ps,avs) 
PS   ps;
AV_Sequence  *avs;
{
AV_Sequence eptr;
int i=0;
DN tdn, marker, next, trail = NULLDN;
extern char * local_dit;
ORName * or;

	marker = str2dn (local_dit);

	for(eptr = *avs; eptr != NULLAV; eptr = eptr->avseq_next) {
		i++;
		or = (ORName *)eptr->avseq_av.av_struct;
		if ((tdn = or->on_dn) != NULLDN) {
			for (next=marker; 
				(tdn!= NULLDN) && (next != NULLDN); 
				next=next->dn_parent, tdn=tdn->dn_parent) {
				if (dn_comp_cmp(tdn,next) != 0) {
					if (trail == NULLDN) 
						marker = NULLDN;
					else 
						trail->dn_parent = NULLDN;
					dn_free (next);
					break;
				}
				trail = next;
			}
			if (tdn == NULLDN) {
				if (trail == NULLDN) 
					marker = NULLDN;
				else 
					trail->dn_parent = NULLDN;
				dn_free (next);
			}
		}
	}

	if (i>1) {

		AV_Sequence *base, *bp, *ep;

		base = (AV_Sequence *) smalloc ((int)i * sizeof *base);
		ep = base;
		for(eptr = *avs; eptr != NULLAV; eptr = eptr->avseq_next) 
			*ep++ = eptr;
		
		qsort ((char *) base, i, sizeof *base, (IFP) member_compar);

		bp = base;
		eptr = *avs = *bp++;
		while (bp < ep) {
			eptr -> avseq_next = *bp;
			eptr = *bp++;
		    }
		eptr -> avseq_next = NULL;
		free ((char *) base);
	}

	i = 1;

	for(eptr = *avs; eptr != NULLAV; eptr = eptr->avseq_next) {
		ps_printf (ps,"%2d: ",i++);
		or = (ORName *)eptr->avseq_av.av_struct;
		if (or->on_dn != NULLDN) 
#ifdef MULTILINE_OUTPUT
			(void) ufn_dn_print_aux (ps,or->on_dn,marker,TRUE);
#else
			(void) ufn_dn_print_aux (ps,or->on_dn,marker,FALSE);
#endif
		else {
			ps_print (ps,"X.400 Address: ");
			orAddr_print (ps,or->on_or,READOUT);
		}
		ps_print (ps,"\n");
	}
}

check_ORName (ps,or,dncheck,orcheck,update,listname,quiet)
PS ps;
ORName * or;
char dncheck;
char orcheck;
char update;
DN listname;
char quiet;
{
OR_ptr newor = NULLOR; 
char buf [LINESIZE];
Attr_Sequence as;
char ret = TRUE;
char res;

	if (update)
		dncheck = TRUE;

	if (dncheck && (or->on_dn != NULLDN)) {
		if (!quiet) {
		  ps_print (ps,"Checking '");
		  ufn_dn_print (ps,or->on_dn,FALSE);
		  ps_print (ps,"'... ");
		  (void) ps_flush (ps);
	        }

		if ((res = dn2addr (or->on_dn, &as)) == OK)
			newor = as2or (as);

		if (newor == NULLOR) {
			if (quiet) {
				if (res == NOTOK) {
				   ps_print (ps,"\n ORAddress lookup failed '");
				   ufn_dn_print (ps,or->on_dn,FALSE);
				   ps_print (ps,"'");
				   ret = FALSE;
			        }
			} else if (res == DONE) 
				ps_print (ps,"\n *** Temporary ORAddress lookup failed ***\n");
			else {
				ps_print (ps,"\n *** ORAddress lookup failed ***\n");
			        ret = FALSE;
		        }
		} else if (orAddr_cmp (newor, or->on_or) == 0) {
			if (!quiet)
			  ps_print (ps,"OK\n");
		} else if (update) {	

			if (orAddr_check (newor,&buf[0]) == OK) {
				if (or->on_or == NULLOR) {
					ps_print (ps,"\n   Adding ORaddress ");
				} else {
					ps_print (ps,"\n   Changing ");
					orAddr_print (ps,or->on_or,READOUT);
					ps_print (ps," to ");
				}
				orAddr_print (ps,newor,READOUT);
				ps_print (ps,"... ");
				(void) ps_flush (ps);

				if (or_modify (ps,or,newor,at_Member,listname)) {
					ps_print (ps,"OK\n");
					or_free (or->on_or);
					or->on_or = newor;
				}
				(void) ps_flush (ps);
				return ret;
			} else {
				char buf2[BUFSIZ];
				or_or2std (newor,buf2,0);
				ps_printf (ps,"\n *** BAD ORAddress in DIT (%s): %s *** \n",buf,buf2);
				ret = FALSE;
			}
		} else if (orcheck && or->on_or != NULLOR) {
			if (orAddr_check (newor,&buf[0]) == OK) {
				if (quiet) {
					ps_printf (ps,"\n Error: '%s'\n   X.500: ",dn2ufn(or->on_dn,FALSE));
				} else
					ps_print (ps,"\n *** Error ***\n   X.500: ");
				orAddr_print (ps,newor,EDBOUT);
				ps_print (ps,"\n   X.400: ");
				orAddr_print (ps,or->on_or,EDBOUT);
				ps_print (ps,"\n");
				ret = FALSE;
			} else {
				char buf2[BUFSIZ];
				or_or2std (newor,buf2,0);
				ps_printf (ps,"\n *** BAD ORAddress in DIT (%s): %s ***\n",buf,buf2);
				ret = FALSE;
			}
		} else {
			if (!quiet)
			   ps_print (ps,"X.500 Name OK\n");
			if (or->on_or == NULLOR)
				or->on_or = newor;
		}
	}

	if ((orcheck) && (or->on_or != NULLOR)) {
		if (!quiet) {
			ps_print (ps,"Checking ");
			orAddr_print (ps,or->on_or,READOUT);
			ps_print (ps,"... ");
		}

		if (orAddr_check (or->on_or,&buf[0]) == OK) {
			if (!quiet)
				ps_printf (ps,"OK\n");
		} else {
			if (quiet) {
				ps_printf (ps,"\n Error: %s \n",buf);
				orAddr_print (ps,or->on_or,READOUT);
				ps_print (ps,"\n");
			} else
				ps_printf (ps,"\n *** Error: %s *** \n",buf);
			ret = FALSE;
		}
	}
	(void) ps_flush (ps);
	return ret;
}

check_dl_members (ps,dn,as,dncheck,orcheck,update,quiet)
PS ps;
DN dn;
Attr_Sequence as;
char dncheck;
char orcheck;
char update;
char quiet;
{
Attr_Sequence tmp;
AV_Sequence avs;
int res;
int result = OK;

	if ((tmp = as_find_type (as,at_Member)) == NULLATTR) {
		ps_print (ps,"Can't find dl-members attribute!!!\n");
		return NOTOK;
	}

	for (avs=tmp->attr_value; avs!= NULLAV; avs=avs->avseq_next) {
		res = check_ORName (ps,(ORName *)avs->avseq_av.av_struct,
					dncheck,orcheck,update,dn,quiet);
		if (res == FALSE)
			result = NOTOK;
	}

	return result;
}

dl_print (ps,dn)
PS ps;
DN dn;
{
Attr_Sequence tmp;
Attr_Sequence as;
DN dnm, get_manager_dn();

	switch (dir_getdl_aux (dn, &as)) {
	    case OK:
		break;
	    case DONE:
		ps_print (ps, "Temporary failure to read the entry");
		return;
	    default:
		ps_print (ps,"Can't read the entry");		
		return;
	}

	current_print = as;

	if (( dnm = get_manager_dn(as,TRUE)) == NULLDN) 
		ps_print (ps,"\nEntry has no Owner !!!\n");
	else {
		ps_print (ps,"\nOwner: ");
		ufn_dn_print (ps,dnm,TRUE);
	}

	if (((tmp = as_find_type (as,at_Permit)) == NULLATTR) 
	    || (tmp->attr_value == NULLAV))
		ps_print (ps,"\n\nEntry has no mhsDLSubmitPermissions !!!\n");
	else {
		ps_print (ps,"\n\nSubmit Permissions, ");
		avs_seq_print (ps,tmp->attr_value, UFNOUT);
	}

	if (((tmp = as_find_type (as,at_Policy)) == NULLATTR) 
	    || (tmp->attr_value == NULLAV))
		ps_print (ps,"\nEntry has no dl-policy\n");
	else {
		ps_print (ps,"\nList Policy, ");
		avs_seq_print (ps,tmp->attr_value, UFNOUT);
	}

	if ((tmp = as_find_type (as,at_Member)) == NULLATTR) {
		ps_print (ps,"\nEntry has no dl-members !!!\n");
		return;
	}
	ps_print (ps,"\nMembers:\n");
	avs_seq_print_members (ps,&tmp->attr_value);

}


ADDR * get_postmaster()
{
static ADDR *res = NULLADDR;
ADDR *dn2ADDR();
DN dn, search_postmaster();
static DN localdn = NULLDN;
extern char * local_dit;
extern char * getpostmaster();

	if (res != NULLADDR)
		return res;

	if ((localdn == NULLDN) && local_dit)
		localdn = str2dn (local_dit);

	if ((dn = search_postmaster(localdn)) == NULLDN) 
		return res = adr_new(getpostmaster(AD_822_TYPE), 
				     AD_822_TYPE, 0);

	if ((res = dn2ADDR (dn,TRUE)) == NULLADDR)
		return res = adr_new(getpostmaster(AD_822_TYPE), 
				     AD_822_TYPE, 0);

	return res;
}

char * get_spostmaster()
{
ADDR *res;

	res = get_postmaster();
	return res->ad_value;
}


ADDR * get_manager(as)
Attr_Sequence as;
{
Attr_Sequence tmp;
AV_Sequence avs;
ADDR * dn2ADDR();
ADDR 	*ret = NULLADDR;
ADDR    *next;
int num = 1;
void dn_print ();

	if ((tmp = as_find_type(as,at_Owner)) == NULLATTR)
		return get_postmaster();

	if ((avs = tmp->attr_value) == NULLAV)
		return get_postmaster();

	for (; avs!= NULLAV; avs=avs->avseq_next) {
		if ((next = dn2ADDR ((DN)(avs->avseq_av.av_struct),TRUE)) == NULLADDR) {
			pslog (log_dsap,LLOG_NOTICE,"Bad address in DN",dn_print,avs->avseq_av.av_struct);
			continue;
		}
		next->ad_extension = num;
		next->ad_no = num++;
		adr_add(&ret, next);
	}
	return ret;
}

DN get_manager_dn (as,unwrap)
Attr_Sequence as;
char unwrap;		/* IF TRUE, and entry is a role, unwrap it */
{
Attr_Sequence tmp;
AV_Sequence avs;

	if ((tmp = as_find_type(as,at_Owner)) == NULLATTR)
		return NULLDN;

	if ((avs = tmp->attr_value) == NULLAV)
		return NULLDN;

	if (unwrap) {
		if (dn2addr((DN)avs->avseq_av.av_struct, &tmp) != OK)
			return (DN)avs->avseq_av.av_struct;

		if ((tmp = as_find_type (tmp,at_RoleOccupant)) == NULLATTR) 
			return (DN)avs->avseq_av.av_struct;
	
		if ((tmp->attr_value) == NULLAV)
			return (DN)avs->avseq_av.av_struct;
		avs = tmp->attr_value;
	}

	return (DN)avs->avseq_av.av_struct;
}


DN mail2dn (ps,addr)
PS ps;
char * addr;
{
DN dn, do_dm_match();
ADDR * ad;
char *local, *ad_getlocal();
RP_Buf rp;

	ps_printf (ps,"Looking for mail address '%s' in the directory...",addr);

	ad = adr_new (addr, (*addr == '/') ? AD_X400_TYPE : AD_822_TYPE, 1);
	ad->ad_resp = YES;

#ifdef UKORDER
	(void) ad_parse(ad, &rp, CH_UK_PREF);
#else
	(void) ad_parse(ad, &rp, CH_USA_PREF);
#endif
	if (ad->ad_r400adr == NULLCP) {
		adr_free (ad);
		ps_printf (ps,"Failed\n",addr);
		return NULLDN;
	}

	if ((local = ad_getlocal (ad->ad_r400adr, AD_X400_TYPE)) == NULLCP) {
		if (ad->ad_r822adr)
			dn = do_dm_match (ad->ad_r822adr);
		else
			dn = do_dm_match (addr);

		if (dn) {
			adr_free (ad);
			ps_printf (ps,"%s\n", dn2ufn(dn,FALSE));
			return dn;       
		} else {
			adr_free (ad);
			ps_printf (ps,"Failed\n",addr);
			return NULLDN;
		}
	}

	dn = do_dm_match (local);

	free (local);
	adr_free (ad);

	ps_printf (ps,"%s\n", dn2ufn(dn,FALSE));
	
	return dn;       
}

ORName * mail2orname (ps,addr)
PS ps;
char * addr;
{
ORName * or;
char * ptr;
Attr_Sequence as;
OR_ptr as2or (), orAddr_parse_user();

	or = (ORName *) smalloc (sizeof (ORName));

	if ((or->on_dn = mail2dn(ps,addr)) == NULLDN) {
		ptr = SkipSpace(addr);	
		if (*ptr == 0)
			or->on_or = NULLOR;
		else if ((or->on_or = orAddr_parse (ptr)) == NULLOR)
			return (NULLORName);
	} else {
		if (dn2addr (or->on_dn, &as) != OK) 
			or->on_or = orAddr_parse (addr);
		else
			or->on_or = as2or (as);
	}

	return (or);
}

Attr_Sequence mail2member (ps, addr)
PS ps;
char * addr;
{
AttributeValue av;
AV_Sequence avs;

	av = AttrV_alloc();
	av->av_syntax = str2syntax("ORName");
	if ((av->av_struct = (caddr_t) mail2orname(ps,addr)) == NULL)
		return NULLATTR;

	avs = avs_comp_new(av);
	return as_comp_new (AttrT_cpy(at_Member),avs,NULLACL_INFO);
}

