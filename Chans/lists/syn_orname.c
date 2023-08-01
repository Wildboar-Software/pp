/* syn_orname.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_orname.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/syn_orname.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: syn_orname.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



/* LINTLIBRARY */

#include "util.h"
#include <varargs.h>
#include "retcode.h"
#include "adr.h"
#include "ap.h"
#include "dlist.h"

extern OR_ptr orAddr_parse();
extern OR_ptr or_cpy();
extern ORName *pe2orn();
extern PE orn2pe();

ORName * orName_parse (str)
char * str;
{
ORName * or;
char * ptr;

	or = (ORName *) smalloc (sizeof (ORName));

	if ( (ptr=index (str,'$')) == NULLCP) {
		if ((or->on_dn = str2dn (str)) == NULLDN)
			return (NULLORName);
		or->on_or = NULLOR;
		return or;
	}

	*ptr--= 0;
	if (isspace (*ptr)) {
		*ptr = 0;
	}
	ptr++;
	ptr++;

	if (*str == 0)
		or->on_dn = NULLDN;
	else
		if ((or->on_dn = str2dn (str)) == NULLDN)
			return (NULLORName);

	ptr = SkipSpace(ptr);	
	if (*ptr == 0)
		or->on_or = NULLOR;
	else if ((or->on_or = orAddr_parse (ptr)) == NULLOR)
		return (NULLORName);
	
	return (or);
}

orName_print (ps,or,format)
PS ps;
ORName * or;
int format;
{
	if (or == NULLORName)
		return;
	if (or->on_dn != NULLDN) {

		dn_print (ps,or->on_dn,format);

		if ((format == UFNOUT) || format == READOUT) 
			return;
	}

	if ((format != READOUT) && (format != UFNOUT))
		ps_print (ps," $ ");
	else if (or->on_dn == NULLDN)
		ps_print (ps,"X.400 Address: ");

	if (or->on_or)
		orAddr_print (ps,or->on_or,format);

}

ORName * orName_cpy (a)
ORName * a;
{
ORName * top;

	top = (ORName *) smalloc (sizeof (ORName));
	top->on_or = or_cpy (a->on_or);
	top->on_dn = dn_cpy (a->on_dn);

	return (top);
}

orName_cmp (a,b)
ORName *a, *b;
{
int res;
	if ( a == NULLORName)
		return (b == NULLORName ? 0 : -1);
	if ( b == NULLORName)
		return 	1;

	if ((res = dn_cmp (a->on_dn,b->on_dn)) == 0) 
 		return orAddr_cmp (a->on_or,b->on_or);
	else
		return res;
}

orName_cmp_user (a,b)
ORName *a, *b;
{
	/* IF DN's equal -> match OK */

	if ((a->on_dn == NULLDN) || (b->on_dn == NULLDN))
 		return orAddr_cmp (a->on_or,b->on_or);

	return dn_cmp (a->on_dn,b->on_dn);
}

extern IFP orName_free();

orName_syntax ()
{
	(void) add_attribute_syntax ("ORName",
		(IFP) orn2pe,	(IFP) pe2orn,
		(IFP) orName_parse,	orName_print,
		(IFP) orName_cpy,	orName_cmp,
		ORName_free,		NULLCP,
		NULLIFP,		TRUE);
}
