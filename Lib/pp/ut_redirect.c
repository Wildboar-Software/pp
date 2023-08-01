/* ut_redirect.c: utility routines for redirections */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_redirect.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_redirect.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_redirect.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "adr.h"

extern UTC utclocalise(), utcnow();

Redirection *redirect_new(addr, dn, utc, reason)
char	*addr;
char	*dn;
UTC	utc;
int	reason;
{
	Redirection	*ret;

	ret = (Redirection *) smalloc (sizeof *ret);
	ret->rd_addr = isstr(addr) ? strdup(addr) : NULLCP;
	ret->rd_dn = isstr(dn) ? strdup(dn) : NULLCP;
	if (utc == NULLUTC) {
		/* redirect now */
		UTC	utcn;
		utcn = utcnow();
		ret -> rd_time = utclocalise(utcn);
		free((char *) utcn);
	} else
		ret -> rd_time = utclocalise(utc);
	ret->rd_reason = reason;
	ret->rd_next = (Redirection *) NULL;
	return ret;
}

void redirect_add(base, redirect)
Redirection	**base, *redirect;
{
	Redirection	**ix;

	for (ix = base; *ix; ix = &(*ix) -> rd_next)
		continue;
	*ix = redirect;
}

void redirect_free(redirect)
Redirection	*redirect;
{
	if (redirect == (Redirection *) NULL)
		return;
	if (redirect -> rd_next)
		redirect_free(redirect -> rd_next);
	if (redirect -> rd_addr)
		free(redirect -> rd_addr);
	if (redirect -> rd_dn)
		free(redirect -> rd_dn);
	if (redirect -> rd_time)
		free((char *) redirect -> rd_time);
	free((char *) redirect);
}

void redirect_rewind(base, to)
Redirection	**base, *to;
{
	Redirection	**ix;
	
	for (ix = base; *ix && *ix != to; ix = &(*ix) -> rd_next)
		continue;
	if (*ix) {
		redirect_free(*ix);
		*ix = (Redirection *) NULL;
	}
}

/* check to see in newRedirect in list */
int redirect_before (base, newRedirect)
Redirection *base, *newRedirect;
{
	for (;base != (Redirection *) NULL; base = base->rd_next) {
		if (base->rd_addr && newRedirect -> rd_addr
		    && lexequ(base->rd_addr,newRedirect->rd_addr) == 0)
			return TRUE;
		if (base->rd_dn && newRedirect->rd_dn
		    && lexequ(base->rd_dn, newRedirect->rd_dn) == 0)
			return TRUE;
	}

	return FALSE;
}
