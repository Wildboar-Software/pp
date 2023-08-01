/* syn_permit.c: */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_permit.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_permit.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: syn_permit.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include <isode/cmd_srch.h>

extern ORName * orName_cpy();
extern ORName * orName_parse();

static permit_free (ptr)
struct dl_permit * ptr;
{
	if (ptr->dp_type == DP_GROUP)
		dn_free (ptr->dp_dn);
	else
		ORName_free (ptr->dp_or);
}


static struct dl_permit * permit_cpy (a)
struct dl_permit * a;
{
struct dl_permit * result;

	result = (struct dl_permit *) smalloc (sizeof (struct dl_permit));
	if ( (result->dp_type = a->dp_type) == DP_GROUP)
		result->dp_dn = dn_cpy (a->dp_dn);
	else
		result->dp_or = orName_cpy (a->dp_or);

	return (result);
}

static permit_cmp (a,b)
struct dl_permit * a;
struct dl_permit * b;
{	
	if (a->dp_type != b->dp_type)
		return (a->dp_type > b->dp_type ? 1 : -1);

	if (a->dp_type == DP_GROUP) 
		return (dn_cmp (a->dp_dn,b->dp_dn));
	else
		return (orName_cmp (a->dp_or,b->dp_or));
}


static permit_print (ps,p,format)
register PS ps;
struct   dl_permit* p;
int format;
{
	if ((format == READOUT) || (format == UFNOUT)) {
		if (p->dp_type == DP_GROUP) {
			ps_print (ps,"Members of the X.500 Group:\n   ");
			dn_print (ps,p->dp_dn,EDBOUT);
			return;
		}
		switch (p->dp_type) {
		case DP_INDIVIDUAL:
			ps_print (ps,"The individual:\n   ");
			break;
		case DP_MEMBER:
			ps_print (ps,"The members of the mail list:\n   ");
			break;
		case DP_PATTERN:
			if ((p->dp_or != NULLORName) && (p->dp_or->on_or == NULLOR)) {
				ps_print (ps,"Anybody");
				return;
			}
			ps_print (ps,"Entities matching the OR pattern:\n   ");
			break;
		}
		orName_print (ps, p->dp_or,UFNOUT);
	} else {
		switch (p->dp_type) {
		case DP_GROUP:
			ps_print (ps,"GROUP#");
			dn_print (ps,p->dp_dn,format);
			return;
		case DP_INDIVIDUAL:
			ps_print (ps,"INDIVIDUAL#");
			break;
		case DP_MEMBER:
			ps_print (ps,"MEMBER#");
			break;
		case DP_PATTERN:
			ps_print (ps,"PATTERN#");
			break;
		}
		orName_print (ps, p->dp_or,format);		
	}
}


static struct dl_permit* str2permit (str)
char * str;
{
struct dl_permit * result;
char * ptr;
static CMD_TABLE permit_table [] = {
	"GROUP",	DP_GROUP,
	"INDIVIDUAL",	DP_INDIVIDUAL,
	"MEMBER",	DP_MEMBER,
	"PATTERN",	DP_PATTERN,
	0,		-1,
};

	if ( (ptr=index (str,'#')) == NULLCP) {
		if (lexequ (str,"ALL") == 0) {
			result = (struct dl_permit *) smalloc (sizeof (struct dl_permit));
			result->dp_type = DP_PATTERN;
			result->dp_or = (ORName *) smalloc (sizeof (ORName));
			result->dp_or->on_dn = NULLDN;
			result->dp_or->on_or = NULLOR;
			return result;			
		}	
		parse_error ("seperator missing in permit '%s'",str);
		return ((struct dl_permit *) NULL);
	}

	result = (struct dl_permit *) smalloc (sizeof (struct dl_permit));
	*ptr-- = 0;
	if (isspace (*ptr)) 
		*ptr = 0;
	ptr++,ptr++;

	if ((result->dp_type = cmd_srch (str,permit_table)) == -1) {
		parse_error ("unknown permit choice '%s'",str);
 		return ((struct dl_permit *) NULL);
	}

	if (result->dp_type == DP_GROUP) {
		if ((result->dp_dn = str2dn(ptr)) == NULLDN) 
	 		return ((struct dl_permit *) NULL);
	} else {
		if ((result->dp_or = orName_parse(ptr)) == NULLORName)
	 		return ((struct dl_permit *) NULL);
	}

	if (result->dp_type == DP_MEMBER) {
		if (result->dp_or->on_dn == NULLDN) {
			parse_error ("Must specify X.500 name for member permission",NULLCP);
			return ((struct dl_permit *) NULL);
		}
	}

	return (result);
}

static PE permit_enc (m)
struct dl_permit * m;
{
PE ret_pe;

        (void) encode_MD_DLSubmitPermission (&ret_pe,0,0,NULLCP,m);

	return (ret_pe);
}

static struct dl_permit * permit_dec (pe)
PE pe;
{
	struct dl_permit * m;

	if (decode_MD_DLSubmitPermission (pe,1,NULLIP,NULLVP,&m) == NOTOK) {
		free ((char *)m);
		return ((struct dl_permit *) NULL);
	}

	return (m);
}

permit_syntax ()
{
	(void) add_attribute_syntax ("DLSubmitPermission",
		(IFP) permit_enc,	(IFP) permit_dec,
		(IFP) str2permit,	permit_print,
		(IFP) permit_cpy,	permit_cmp,
		permit_free,		NULLCP,
		NULLIFP,		TRUE);
}
