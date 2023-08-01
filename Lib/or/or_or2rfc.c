/* or_or2rfc.c: convert from or address to rfc address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2rfc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/or/RCS/or_or2rfc.c,v 6.0 1991/12/18 20:23:08 jpo Rel $
 *
 * $Log: or_or2rfc.c,v $
 * Revision 6.0  1991/12/18  20:23:08  jpo
 * Release 6.0
 *
 */



#include "or.h"
#include "util.h"
#include "ap.h"
#include <isode/cmd_srch.h>
extern char *loc_dom_site;




/* ---------------------  Begin  Routines  -------------------------------- */




/*
Maps OR struct into a RFC822 string
*/

#define UB_DDA_VALUE 128

#define	RFC822C1	100
#define	RFC822C2	101
#define	RFC822C3	102

CMD_TABLE	ortbl_rfc822ddas[] = { /* type strings for rfc 822 ddas */
	"RFC-822",	OR_DDVALID_RFC822,
	"RFC822C1",	RFC822C1,
	"RFC822C2",	RFC822C2,
	"RFC822C3",	RFC822C3,
	0,		-1
	};

int or_or2rfc_aux (or, buf, addr_check)
OR_ptr          or;
char            *buf;
int             addr_check;
{
    AP_ptr      local,
		domain;
    OR_ptr      current_ptr;
    char        lbuf[LINESIZE],
		dbuf[LINESIZE],
		*cp;

    PP_DBG (("Lib/or_or2rfc()" ));
    if (or == NULLOR)
	    return NOTOK;

    or = or_default (or);
    current_ptr = or;

    if ((current_ptr = or_find (current_ptr, OR_DD, NULLCP)) != NULLOR) {
	    if (current_ptr -> or_next == NULLOR) {
		    /* --- single DD attributes --- */
		    if (or_ddname_chk (current_ptr -> or_ddname)) {
			    /* --- Special DD type --- */
			    if (or_str_isps(current_ptr->or_value)) {
				    or_ps2asc (current_ptr -> or_value, buf);
				    PP_DBG (("Lib/or_or2rfc: Matched dd, '%s' = '%s'",
					     current_ptr -> or_ddname, buf));
				    return OK;
			    } else {
				    PP_LOG(LLOG_EXCEPTIONS,
					   ("dd '%s' value '%s' is not a printable string",
					    current_ptr->or_ddname, 
					    current_ptr -> or_value));
				    return NOTOK;
			    }
		    }
	    } else {
		    /* --- multiple DD attributes --- */
		    /* --- look to merge RFC-822s --- */
		    char rfc822[UB_DDA_VALUE+1],
		    	 rfc822c1[UB_DDA_VALUE+1],
		    	 rfc822c2[UB_DDA_VALUE+1],
		    	 rfc822c3[UB_DDA_VALUE+1];
		    int  allRFC822s = TRUE;

		    rfc822[0] = '\0';
		    rfc822c1[0] = '\0';
		    rfc822c2[0] = '\0';
		    rfc822c3[0] = '\0';
		    do {
			    if (current_ptr -> or_type == OR_DD) {
				    if (or_str_isps (current_ptr->or_value)) {
					    switch(cmd_srch(current_ptr -> or_ddname,
							    ortbl_rfc822ddas)) {
						case OR_DDVALID_RFC822:
						    or_ps2asc(current_ptr->or_value,
							      rfc822);
						    break;

						case RFC822C1:
						    or_ps2asc(current_ptr->or_value,
							      rfc822c1);
						    break;

						case RFC822C2:
						    or_ps2asc(current_ptr->or_value,
							      rfc822c2);
						    break;
						case RFC822C3:
						    or_ps2asc(current_ptr->or_value,
							      rfc822c3);
						    break;
						default:
						    allRFC822s = FALSE;
						    break;
					    }
					    PP_DBG(("Lib/or_or2rfc: Matched dd, '%s' = '%s'",
						    current_ptr->or_ddname,
						    current_ptr->or_value));
				    } else {
					    PP_LOG(LLOG_EXCEPTIONS,
						   ("dd '%s' value '%s' is not a printable string",
						    current_ptr->or_ddname,
						    current_ptr->or_value));
					    return NOTOK;
				    }
			    } else
				    allRFC822s = FALSE;

			    current_ptr = current_ptr -> or_next;
		    } while (current_ptr != NULLOR);

		    if (allRFC822s == TRUE) {
			    buf[0] = '\0';

			    if (rfc822[0])
				    (void) strcat(buf, rfc822);

			    if (rfc822c1[0])
				    (void) strcat(buf, rfc822c1);

			    if (rfc822c2[0])
				    (void) strcat(buf, rfc822c2);

			    if (rfc822c3[0])
				    (void) strcat(buf, rfc822c3);
			    if (buf[0] != '\0')
				    return OK;
		    }
	    }
    }
		    
    if (or_or2rbits (or, lbuf, dbuf) == NOTOK)
	return NOTOK;


    /* --- if NUL domain - explicit encoding --- */

    if (dbuf[0] == '\0') {
	(void) strcpy (buf, lbuf);
	PP_DBG (("Lib/or_or2rfc Returns '%s'", buf));
	return OK;
    }


    PP_TRACE (("Lib/or_or2rfc lbuf='%s' dbuf='%s'", lbuf, dbuf));


    if (lexequ (dbuf, loc_dom_site) == 0)
	if (addr_check)
	    return DONE;

    local  = ap_new (AP_MAILBOX, lbuf);
    domain = ap_new (AP_DOMAIN, dbuf);

    cp = ap_p2s (NULLAP, NULLAP, local, domain, NULLAP);

    (void) strcpy (buf, cp);

    free (cp);
    ap_free (local);
    ap_free (domain);

    PP_TRACE (("Lib/or_or2rfc returns '%s'", buf));

    return OK;
}
