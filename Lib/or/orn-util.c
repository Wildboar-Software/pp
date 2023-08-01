/* orn-util.c: utility routines for ORNames */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/orn-util.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/orn-util.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: orn-util.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "or.h"
#include "IOB-types.h"
#include "MTA-types.h"
#include "Ext-types.h"
#include "extension.h"

#define STR2QB(s)	str2qb(s, strlen(s), 1)

void ORName_free (orn)
ORName *orn;
{
	if (orn == NULLORName)
		return;

	if (orn -> on_or)
		or_free (orn->on_or);
	if (orn -> on_dn)
		dn_free (orn -> on_dn);
	free ((char *)orn);
}

int or_append (tree, new)
OR_ptr	*tree;
OR_ptr	new;
{
	OR_ptr	tn;

	tn = or_add (*tree, new, 0);
	if (tn == NULLOR) {
		PP_LOG (LLOG_EXCEPTIONS, ("or_add failed on node %s",
					  new -> or_value));
		return NOTOK;
	}
	*tree = tn;
	return OK;
}


struct qbuf *or_getttx (val, enc)
char	*val;
int	enc;
{
	char *cp;

	switch (enc) {
	    case OR_ENC_TTX:
	    case OR_ENC_TTX_AND_OR_PS:
		if ((cp = index (val, '*')) == NULLCP) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Missing * in T.61 encoding for %s", val));
			return NULL;
		}
		return or_t61decode (cp + 1);
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Not a TTX encoding"));
		return NULL;
	}
}

struct qbuf *or_getps (val, enc)
char	*val;
int	enc;
{
	char *cp;

	switch (enc) {
	    case OR_ENC_PS:
		return STR2QB (val);
	    case OR_ENC_TTX_AND_OR_PS:
		if ((cp = index(val, '*')) == NULLCP)
			return STR2QB (val);
		else
			return str2qb (val, cp - val, 1);
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("getps called with bad encoding"));
		return NULL;
	}
}
