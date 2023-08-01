/* or_rfc2or.c: convert from rfc address to or address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_rfc2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_rfc2or.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_rfc2or.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"
#include "ap.h"

extern char     *loc_dom_site;
extern char     *loc_or;
extern char     or_error[];
static int	RFC_encode();
static void	get_loc_val ();
static int	my_or_str_isps();

int or_rfc2or_aux (rfc, or, adno)
char            *rfc;
OR_ptr          *or;
int             adno;
{
    AP_ptr      ap,
    		group,
    		name,
		local,
		route,
		domain;
    char        buf[LINESIZE];
    int		retval;
    OR_ptr	tmp;
    PP_DBG (("Lib/or_rfc2or ('%s')", rfc));

    /*
    An address can be specified as: 'xxxx: ;'
    */

    if (rfc_space (rfc))
	return (or_blank (or));


    local  = NULLAP;
    domain = NULLAP;
    route  = NULLAP;
    *or = NULLOR;

    if ((ap = ap_s2t (rfc)) != BADAP
	 && ap_t2p (ap, &group, &name, &local, &domain, &route)
		!= BADAP)
    {
	if (route != NULLAP) {
		ap_val2str(buf, route->ap_obvalue, route->ap_obtype);
		if (or_str_isps (buf))
			(void) or_domain2or (buf, or);
		goto i_was_forced_into_this;
	}

	if (domain != NULLAP) {
		ap_val2str(buf, domain->ap_obvalue, domain->ap_obtype);
		if (!or_str_isps (buf))
			goto i_was_forced_into_this;
	} else
	    (void) strcpy (buf, loc_dom_site);
	
	if (or_domain2or(buf, or) == NOTOK)
	       goto i_was_forced_into_this;

	if (local != NULLAP) {
		get_loc_val (local, buf);
		if(!my_or_str_isps (buf))
			goto i_was_forced_into_this;
	} else
	    buf[0] = '\0';

	tmp = or_tdup(*or);
	if (or_local2or(buf, or) == OK) {
		or_chk_admd(or);
		/* check to see if what we've created */
		/* actually matches with the standard */
		if (or_valid_or(*or) == OK) {
			or_free(tmp);
			return (OK);
		}
		/* not valid so put rhs generated tree back */
		or_free(*or);
		*or = tmp;
	} else {
		or_free(*or);
		*or = tmp;
	}
    } else {
	    if (ap != BADAP)
		    ap_free(ap);
	    return or_lose ("Can't parse address '%s'",rfc);
    }

i_was_forced_into_this:;
    retval = RFC_encode(rfc, or, adno, domain, route);
    or_chk_admd(or);
    if (ap != BADAP)
	    ap_free(ap);
    return retval;

}

/* check 1148 constraints on str */
static int my_or_str_isps(str)
char *str;
{
	int	cont = TRUE, seenspace = FALSE;
	if (isspace(*str) || isspace(str[strlen(str)-1]))
		/* leading or trailing spaces */
		return 0;

	while (*str && cont == TRUE) {

		if (isspace(*str)) {
			if (seenspace == TRUE) {
				/* adjacent spaces */
				cont = FALSE;
				continue;
			}
			seenspace = TRUE;
		} else
			seenspace = FALSE;

		if ((*str == '$' 
		    && (*(str+1) == '=' || *(str+1) == '/') || *(str+1) == ';')
		    || *str == ';' || *str == '/'
		    || (or_isps(*str)))
			str++;
		else
			cont = FALSE;
	}
	return *str == '\0';
}

#define UB_DDA_VALUE	128
#define UB_DDA_TYPE	8

static int RFC_encode(rfc, or, adno, domain, route)
char	*rfc;
OR_ptr	*or;
int	adno;
AP_ptr	domain,
	route;
{
    char	buf[LINESIZE];
    OR_ptr	ptr = NULLOR, new;
    char	*dmn = NULLCP;
    char	*start, *end, savech;
    int len;
    char	dda_type[UB_DDA_TYPE+1];
    int		ddacount = 0;
    /*
    Make explicit RFC encoding
    */
    (void) or_asc2ps (rfc, buf);
    len = strlen(buf);
    start = &(buf[0]);
    do {
	    if (len < UB_DDA_VALUE)
		    end = NULLCP;
	    else {
		    end = start + UB_DDA_VALUE;
		    savech = *end;
		    *end = '\0';
	    }
	    if (ddacount == 0)
		    (void) strcpy(dda_type, "RFC-822");
	    else
		    (void) sprintf(dda_type, "RFC822C%d", ddacount);
	    ddacount++;
	    new = or_new (OR_DD, dda_type, start);
	    if ((ptr = or_add (ptr, new, FALSE)) == NULLOR)
		    return NOTOK;
	    
	    len -= strlen(start);
	    start = end;
	    if (end != NULLCP)
		    *end = savech;
    } while (start != NULLCP && *start != '\0');


    if (*or == NULLOR) {
	    if (route != NULLAP)
		    dmn = route -> ap_obvalue;
	    else if (domain != NULLAP)
		    dmn = domain -> ap_obvalue;

	    if (adno == 0
		/* MTS originator */
		|| dmn == NULLCP
		|| tb_get1148gate (dmn, or) != OK)
		    /* RFC_encode with local or */
		    *or = or_std2or (loc_or);
    }
    for (new = ptr; new != NULLOR; new = ptr) {
	    ptr = new -> or_next;
	    new -> or_next = NULLOR;
	    *or = or_add( *or, new, TRUE);
	    if (*or == NULLOR)
		    return NOTOK;
    }
    return (OK);
}

static void get_loc_val (lap, buf)
AP_ptr lap;
char buf[];
{
	for (buf[0] = '\0'; lap != NULLAP; lap = lap -> ap_next) {
		switch (lap -> ap_obtype) {
		    default:
		    case AP_NIL:
			break;
		    case AP_COMMENT:
			continue;

		    case AP_GENERIC_WORD:
		    case AP_MAILBOX:
			if (buf[0])
				(void) strcat (buf, " ");
			(void) strcat (buf, lap -> ap_obvalue);
			continue;
		}
		break;
	}
}
