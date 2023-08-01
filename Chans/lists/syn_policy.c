/* syn_policy.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_policy.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_policy.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: syn_policy.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include <isode/cmd_srch.h>

static CMD_TABLE expand_table [] = {
	"FALSE",	FALSE,
	"TRUE",		TRUE,
	0,		-1,
};

static CMD_TABLE convert_table [] = {
	"ORIGINAL",	DP_ORIGINAL,
	"FALSE",	DP_FALSE,
	"TRUE",		DP_TRUE,
	0,		-1,
};

static CMD_TABLE priority_table [] = {
	"ORIGINAL",	DP_ORIGINAL,
	"LOW",		DP_LOW,
	"NORMAL",	DP_NORMAL,
	"HIGH",		DP_HIGH,
	0,		-1,
};

static dl_policy_free (ptr)
struct dl_policy * ptr;
{
	free ((char *) ptr);
}


static struct dl_policy * dl_policy_cpy (a)
struct dl_policy * a;
{
struct dl_policy * result;

	result = (struct dl_policy *) smalloc (sizeof (struct dl_policy));
	*result = *a;		/* struct copy */
	return (result);
}

static dl_policy_cmp (a,b)
struct dl_policy * a;
struct dl_policy * b;
{
	if (a == (struct dl_policy *) NULL)
		if (b == (struct dl_policy *) NULL)
			return (0);
		else 
			return (-1);

	if (b == (struct dl_policy *) NULL)
		return 1;

	if (a->dp_expand != b->dp_expand)
		return ( a->dp_expand > b->dp_expand ? 1 : -1);

	if (a->dp_priority != b->dp_priority)
		return ( a->dp_priority > b->dp_priority ? 1 : -1);

	if (a->dp_convert != b->dp_convert)
		return ( a->dp_convert > b->dp_convert ? 1 : -1);

	return (0);
}


static dl_policy_print (ps,p,format)
register PS ps;
struct   dl_policy* p;
int format;
{

	if (format == READOUT)
		ps_print (ps,"Expansion: ");

	ps_printf (ps,"%s",rcmd_srch(p->dp_expand,expand_table));

	if (format == READOUT)
		ps_print (ps,", Conversion: ");
	else
		ps_print (ps,"$");

	ps_printf (ps,"%s",rcmd_srch(p->dp_convert,convert_table));

	if (format == READOUT)
		ps_print (ps,", Priority: ");
	else
		ps_print (ps,"$");

	ps_printf (ps,"%s",rcmd_srch(p->dp_priority,priority_table));
}

static struct dl_policy* str2dl_policy (str)
char * str;
{
struct dl_policy * result;
char * ptr;
char * mark = NULLCP;
char * prtparse ();



	if ( (ptr=index (str,'$')) == NULLCP) {
		parse_error ("seperator missing in dl_policy '%s'",str);
		return ((struct dl_policy *) NULL);
	}

	result = (struct dl_policy *) smalloc (sizeof (struct dl_policy));
	*ptr--= 0;
	if (isspace (*ptr)) {
		*ptr = 0;
		mark = ptr;
	}
	ptr++;

	if (*str == 0)
		result->dp_expand = TRUE;
	else if ((result->dp_expand = cmd_srch(str,expand_table)) == -1) {
		parse_error ("%s unrecognised",str);
                return ((struct dl_policy *) NULL);
	}
	*ptr++ = '$';

	if (mark != NULLCP)
		*mark = ' ';

	str = SkipSpace(ptr);	

	if ( (ptr=index (str,'$')) == NULLCP) {
		parse_error ("2nd seperator missing in dl_policy '%s'",str);
		return ((struct dl_policy *) NULL);
	}

	*ptr--= 0;
	if (isspace (*ptr)) {
		*ptr = 0;
		mark = ptr;
	} else
		mark = NULLCP;

	ptr++;

	if (*str == 0)
		result->dp_convert = DP_ORIGINAL;
	else if ((result->dp_convert = cmd_srch(str,convert_table)) == -1) {
		parse_error ("%s unrecognised (2)",str);
                return ((struct dl_policy *) NULL);
	}

	*ptr++ = '$';

	if (mark != NULLCP)
		*mark = ' ';

	ptr = SkipSpace (ptr);

	if (*ptr == 0)
		result->dp_priority = DP_LOW;
	else if ((result->dp_priority = cmd_srch(ptr,priority_table)) == -1) {
		parse_error ("%s unrecognised (3)",ptr);
                return ((struct dl_policy *) NULL);
	}

	return (result);
}

static PE dl_policy_enc (m)
struct dl_policy * m;
{
PE ret_pe;

        (void) encode_DL_DlPolicy (&ret_pe,0,0,NULLCP,m);

	return (ret_pe);
}

static struct dl_policy * dl_policy_dec (pe)
PE pe;
{
	struct dl_policy * m;

	if (decode_DL_DlPolicy (pe,1,NULLIP,NULLVP,&m) == NOTOK) {
		free ((char *)m);
		return ((struct dl_policy *) NULL);
	}
	return (m);
}

policy_syntax ()
{
	(void) add_attribute_syntax ("DlPolicy",
		(IFP) dl_policy_enc,	(IFP) dl_policy_dec,
		(IFP) str2dl_policy,	dl_policy_print,
		(IFP) dl_policy_cpy,	dl_policy_cmp,
		dl_policy_free,		NULLCP,
		NULLIFP,		TRUE);
}
