/* write_queue.c - Creates & submits a Q_struct for a DR Msg */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/dr2rfc/RCS/write_queue.c,v 6.0 1991/12/18 20:06:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/dr2rfc/RCS/write_queue.c,v 6.0 1991/12/18 20:06:58 jpo Rel $
 *
 * $Log: write_queue.c,v $
 * Revision 6.0  1991/12/18  20:06:58  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "q.h"
#include "tb_a.h"
#include "tb_q.h"
#include "tb_p1.h"
#include <isode/cmd_srch.h>

extern CHAN             *mychan;
extern char             *loc_dom_site;
extern char             *getpostmaster(),
			*hdr_822_bp,
			*ia5_bp, error[];
extern ADDR             *adr_new ();
extern CMD_TABLE	qtbl_con_type[];
extern LIST_BPT		*list_bpt_dup();

/* ---------------------  Begin Routines  --------------------------------- */

char 	*recip_err;


write_queue (to)
ADDR    *to;
{
        struct prm_vars         prm;
        Q_struct                QNewStruct,
                                *qp = &QNewStruct;
        EncodedIT               *ep = &qp -> encodedinfo;
        RP_Buf                  rps, *rp = &rps;


        PP_TRACE (("dr2rfc/write_queue()"));

        if (to->ad_r822adr == NULLCP) {
                PP_LOG (LLOG_EXCEPTIONS, ("%s%s",
                        "dr2rfc/write_queue - no rfc822 ",
                        "recipient address has been specified"));
		(void) sprintf(error,
			       "no rfc822 recipient address has been specified");
                return RP_BAD;
        }


        /* -- initialize prm and Q fields -- */
        prm_init (&prm);
        q_init (qp);
        qp -> msgtype           = MT_UMPDU;
	qp -> inbound  		= list_rchan_new (loc_dom_site,
						  mychan->ch_name);
	ep->eit_types = NULL;
	list_bpt_add(&ep->eit_types, list_bpt_new(hdr_822_bp));
	list_bpt_add(&ep->eit_types, list_bpt_new(ia5_bp));
	

        MPDUid_new (&qp -> msgid);

        /* -- create the ADDR struct for orig and recipient -- */
        qp->Oaddress            = adr_new (getpostmaster(AD_822_TYPE), 
					   AD_822_TYPE, 0);
        qp->Raddress            = adr_new ((isstr(to->ad_r822DR)) ?
					   to->ad_r822DR : to->ad_r822adr, 
					   AD_822_TYPE, 1);


        if (rp_isbad (io_wprm (&prm, rp))) {
                PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/io_wprm err %s",
                                          rp -> rp_line));
		stop_io();
		(void) sprintf (error,
				"io_wprm error [%s]",
				rp -> rp_line);
                return RP_BAD;
        }


        if (rp_isbad (io_wrq (qp, rp))) {
                PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/io_wrq err %s",
                                          rp -> rp_line));
		stop_io();
		(void) sprintf (error,
				"io_wrq error [%s]",
				rp -> rp_line);
                return RP_BAD;
        }


        if (rp_isbad (io_wadr (qp -> Oaddress, AD_ORIGINATOR, rp))) {
                PP_OPER (NULLCP, ("dr2rfc/io_wadr originator/postmaster err %s",
                                          rp -> rp_line));
		stop_io();
		(void) sprintf (error,
				"io_wadr originator/postmaster error [%s]",
				rp -> rp_line);
                return RP_BAD;
        }


        if (rp_isbad (io_wadr (qp -> Raddress, AD_RECIPIENT, rp))) {
                PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/io_wadr recipient err %s",
                                          rp -> rp_line));
		recip_err = strdup(rp->rp_line);
		if (rp_isbad(io_wadr(qp -> Oaddress, AD_RECIPIENT, rp))) {
			PP_OPER (NULLCP, ("dr2rfc/io_wadr originator/postmaster err %s",
                                          rp -> rp_line));
			(void) sprintf (error,
					"io_wadr originator/postmaster error [%s]",	
					rp -> rp_line);
			stop_io();
			return RP_BAD;
		}
        } else 
		recip_err = NULLCP;


        if (rp_isbad (io_adend (rp))) {
                PP_LOG (LLOG_EXCEPTIONS, ("dr2rfc/io_adend err %s",
                                          rp -> rp_line));
		stop_io();
		(void) sprintf (error,
				"io_adend error [%s]",
				rp -> rp_line);
                return RP_BAD;
        }

        /* -- frees the structure -- */
        q_free (qp);
        return RP_OK;

}

