/* tb_1148gate: look up domain in table of 1148 gateways */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_1148gate.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_1148gate.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_1148gate.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"
#include "table.h"

static Table	*tb_1148gateways = NULLTBL;
extern char	*rfc1148gateway_tbl, *loc_or, or_error[];

int tb_get1148gate (domain, or)
char	*domain;
OR_ptr	*or;
{
	char	*p, buf[LINESIZE];
	
	PP_DBG (("Lib/tb_get1148gate (%s)", domain));

	if (tb_1148gateways == NULLTBL) {
		if ((tb_1148gateways = tb_nm2struct (rfc1148gateway_tbl)) == NULLTBL) {
			PP_LOG (LLOG_EXCEPTIONS, ("Lib/tb_get1148gate: rfc1148gateway table NULL!"));
			return NOTOK;
		}
	}
    
	for (p = domain; *p != '\0';) {
		if (tb_k2val (tb_1148gateways, p, buf, TRUE) == OK)
			break;
		p = index (p, '.');
		if (p == NULLCP)
			break;
		else
			p++;
	}


	if (p == NULLCP || *p == '\0') {
		(void) sprintf (or_error,
				"Unable to find rfc1148 gateway for '%s'",
				domain);
		return NOTOK;
	}
	else {
		*or = or_dmn2or (buf);
		if (*or == NULLOR)
			return or_lose ("format error '%s':'%s'",
					p, buf);
	}

	return OK;
}
