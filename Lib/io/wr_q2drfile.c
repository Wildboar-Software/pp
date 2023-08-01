/* wr_q2drfile.c: write out a Delivery Notification struct 
	from the info held in the queue and address structures
*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_q2drfile.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/wr_q2drfile.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: wr_q2drfile.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"q.h"
#include	"dr.h"
#include	"sys.file.h"
#include 	<sys/stat.h>
#include	"list_bpt.h"
#include	<sys/time.h>
#include	"or.h"

extern char	*dr_file;
extern char	*x400_mta;
static void set_succ_suppl ();
static int wr_dr_info ();

/* ---------------------  Begin	 Routines  -------------------------------- */

int wr_q2dr (qp, msg_id)
register Q_struct	*qp;
char	*msg_id;
{
	return wr_q2drfile (qp, msg_id, 1);
}

int wr_q2drfile (qp, path, update)
register Q_struct	*qp;
char	*path;
int	update;
{
	FILE	*fp;
	ADDR	*ap;
	char	 odir[MAXPATHLENGTH];
	int	retval=NOTOK,
		dr_file_no = -1,
		dr_required = FALSE;
	struct stat stbuf;

	PP_DBG (("Lib/io/wr_q2drfile (dir=%s)", path));

	for (ap=qp->Raddress; ap != NULLADDR; ap=ap->ad_next) {
		if (ap->ad_status != AD_STAT_DRREQUIRED)
			continue;

		dr_required = TRUE;

		dr_file_no = ap->ad_no;

		(void) sprintf (odir, "%s/%s%d", path,
				dr_file, dr_file_no);

		PP_DBG(("ap->ad_no = %d file = %s",ap->ad_no, odir));

		if (stat (odir, &stbuf) == NOTOK)
			break;


		if (stbuf.st_size == 0) {
			PP_NOTICE (("Removing empty file %s",
				    odir));
			(void) unlink(odir);
			break;
		}
		PP_OPER (NULLCP,
			 ("wr_q2drfile: File %s already exists of size %d",
			  odir, stbuf.st_size));
		return RP_LIO;
	}

	if (ap == NULLADDR && dr_required == FALSE)
		return (RP_OK);

	qp->Oaddress->ad_status = AD_STAT_PEND;
	if (update &&
	    rp_isbad(retval = wr_ad_status(qp->Oaddress, qp->Oaddress->ad_status)))
		return retval;

	PP_DBG (("Lib/io/wr_q2drfile (opening %s)", odir));

	if ((fp = fopen (odir, "w")) == NULLFILE) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/io/wr_q2drfile (unable to open %s)", odir));
		return (RP_FIO);
	}


	retval = wr_dr_info (fp, qp, dr_file_no, update);

	if (check_close (fp) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, odir, ("Error writing to file"));
		(void) unlink (odir);
		return RP_FIO;
	}

	if (chmod(odir, 0666) == NOTOK)
		PP_SLOG(LLOG_EXCEPTIONS, odir, ("chmod failed"));

	return (retval);
}




static int wr_dr_info (fp, qp, dr_file_no, update) /* Creates a DR  output */
FILE			*fp;
register Q_struct	*qp;
int			dr_file_no;
int			update;
{
	ADDR		*ap;
	time_t		time();
	int		retval;
	DRmpdu		DeliveryReport, *dr;
	Rrinfo		*nxt, *new;
	Report		*rep;
	char		buf[LINESIZE];


	PP_DBG (("Lib/io/wr_dr_info()"));

	/* -- Initialize -- */
	dr_init (dr = &DeliveryReport);

	or_myinit();   /* in case */

	/* -- set MPDUid -- */
	dr -> dr_mpduid = (MPDUid *) smalloc (sizeof (*dr->dr_mpduid));
	bzero ((char*) dr->dr_mpduid, sizeof (*dr->dr_mpduid));
	MPDUid_new (dr -> dr_mpduid);

	/* -- set Trace -- */
	trace_add (&dr -> dr_trace, trace_new());
	trace_add(&dr -> dr_subject_intermediate_trace, trace_dup(qp -> trace));
	for (ap = qp->Raddress; ap != NULLADDR; ap = ap->ad_next)
	   if (ap->ad_status == AD_STAT_DRREQUIRED) {

		   if (ap -> ad_no == dr_file_no)
			   ap->ad_status = AD_STAT_DRWRITTEN;
		   else	ap -> ad_status = AD_STAT_DONE;

		   new = (Rrinfo *) smalloc (sizeof (*new));
		   bzero ((char *)new, sizeof (*new));

		   if (dr -> dr_recip_list != NULL) {
			   for (nxt = dr->dr_recip_list; nxt -> rr_next;
				nxt = nxt->rr_next)
				   continue;
			   nxt -> rr_next = new;
		   }
		   else
			   dr -> dr_recip_list = new;


		   new -> rr_recip = ap -> ad_no;
		   new -> rr_arrival = utcdup (qp -> queuetime);
		   rep = &new -> rr_report;

		   switch (ap->ad_reason) {
		       case DRR_NO_REASON:
			   rep->rep_type = DR_REP_SUCCESS;
			   rep->rep.rep_dinfo.del_time = utcnow();
			   rep->rep.rep_dinfo.del_type = 1;

			   if (ap -> ad_add_info == NULLCP) {
				   set_succ_suppl (ap, qp->msgtype, buf);
				   new -> rr_supplementary = strdup (buf);
			   } else
				   new -> rr_supplementary = strdup(ap -> ad_add_info);
			   break;

		       default:
			   rep->rep_type = DR_REP_FAILURE;
			   rep->rep.rep_ndinfo.nd_rcode = ap->ad_reason;
			   rep->rep.rep_ndinfo.nd_dcode = ap->ad_diagnostic;
			   if (ap->ad_add_info != NULLCP)
				   new ->rr_supplementary =
					   strdup (ap->ad_add_info);

			   new -> rr_converted = (EncodedIT *)
				   calloc (1, sizeof *new -> rr_converted);
			   new -> rr_converted->eit_types =
				   list_bpt_dup(qp -> encodedinfo.eit_types);
			   break;
		   }

		   if (update &&
		       rp_isbad (retval = wr_ad_status(ap, ap -> ad_status)))
			   return retval;
	   }


	retval = dr2txt (fp, &DeliveryReport);

	dr_free (&DeliveryReport);

	return (retval == OK ? RP_OK : RP_FIO);
}




static void set_succ_suppl (ap, msgtype, msg_buf)
ADDR	*ap;
int	msgtype;
char	*msg_buf;
{

	char	*str;


	if (ap->ad_outchan && ap->ad_outchan->li_chan &&
	    ap->ad_outchan->li_chan->ch_access == CH_MTS) 
		str = "Delivered";
	else
		str = "Relayed";
		
	if (msgtype == MT_PMPDU) { /* probe */
		(void) sprintf (msg_buf, 
"%s to '%s' (Probe acknowledgement by PP gateway at '%s')",
				str, ap->ad_outchan->li_mta, x400_mta);
	}
	else 
		(void) sprintf (msg_buf, "%s to '%s'",
				str, ap->ad_outchan->li_mta);

	return;
}
