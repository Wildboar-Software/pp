/* q2txt.c: converts qstruct into text encoding */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/q2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/q2txt.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: q2txt.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include        "q.h"
#include        "tb_com.h"
#include        "tb_q.h"
#include 	"mta.h"

#define crit2txt(n)	rcmd_srch ((n), tbl_crit)
#define txt2int(n)	atoi (n)

extern	void	genreset ();
extern	int	argv2fp ();
extern	void	encodedinfo2txt ();
extern	int	extension2txt ();
extern	void	mpduid2txt ();
extern	void	trace2txt ();
extern	char	*no2txt3();

extern CMD_TABLE
		tbl_crit [],
		tbl_bool [],
		qtbl_que [/* message-envelope-parameters */],
		qtbl_mt_type [/* message-type */],
		qtbl_con_type [/* content-type */],
		qtbl_priority [/* message-priority */];



/* -------------------  Memory -> Text File  ------------------------------ */

char	*qb2hex ();

static char *Q_msgtype2txt (), *Q_priority2txt ();
void dlexp2txt ();
void fname2txt ();
extern char     *int2txt ();
extern char     *time2txt ();

int q2txt (fp, qp)
FILE                    *fp;
register Q_struct       *qp;
{
	char    *argv[100], buf[5];
	int     argc;
	Trace   *trace;
	DLHistory *dlexp;

	genreset ();

	PP_DBG (("Lib/pp/q2txt(type=%d, size=%ld)",
		qp->msgtype, qp->msgsize));

	argv[0] = rcmd_srch (Q_MSGTYPE, qtbl_que);
	argv[1] = Q_msgtype2txt (qp->msgtype);
	argv[2] = NULLCP;
	if (argv2fp (fp, argv) == NOTOK)
		return NOTOK;

	if (qp -> msgsize != 0) {
		argv[0] = rcmd_srch (Q_MSGSIZE, qtbl_que);
		argv[1] = int2txt ((int)qp->msgsize);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp->defertime != 0) {
		argv[0] = rcmd_srch (Q_DEFERREDTIME, qtbl_que);
		argv[1] = time2txt (qp->defertime);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> latest_time) {
		argv[0] = rcmd_srch (Q_LATESTTIME, qtbl_que);
		argv[1] = time2txt (qp -> latest_time);
		argv[2] = crit2txt (qp -> latest_time_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (Q_NWARNS, qtbl_que);
	argv[1] = no2txt3((int)qp->nwarns, buf);
	argv[2] = NULLCP;
	if (argv2fp (fp, argv) == NOTOK)
		return NOTOK;

	if (qp -> warninterval != 0) {
		argv[0] = rcmd_srch (Q_WARNINTERVAL, qtbl_que);
		argv[1] = int2txt (qp->warninterval);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> retinterval != 0) {
		argv[0] = rcmd_srch (Q_RETINTERVAL, qtbl_que);
		argv[1] = int2txt (qp->retinterval);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> cont_type != NULLCP) {
		argv[0] = rcmd_srch (Q_CONTENT_TYPE, qtbl_que);
		argv[1] = qp -> cont_type;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (Q_ENCODED_INFO, qtbl_que);
	argc = 1;
	encodedinfo2txt (&qp->encodedinfo, argv, &argc);
	argv[argc] = NULLCP;
	if (argc != 1)
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;

	argv[0] = rcmd_srch (Q_ORIG_ENCODED_INFO, qtbl_que);
	argc = 1;
	encodedinfo2txt (&qp->orig_encodedinfo, argv, &argc);
	argv[argc] = NULLCP;
	if (argc != 1)
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	
	if (qp -> priority != PRIO_NORMAL) {
		argv[0] = rcmd_srch (Q_PRIORITY, qtbl_que);
		argv[1] = Q_priority2txt (qp->priority);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> disclose_recips) {
		argv[0] = rcmd_srch (Q_DISCLOSE_RECIPS, qtbl_que);
		argv[1] = rcmd_srch (TRUE, tbl_bool);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (qp -> implicit_conversion_prohibited) {
		argv[0] = rcmd_srch (Q_IMPLICIT_CONVERSION, qtbl_que);
		argv[1] = rcmd_srch (TRUE, tbl_bool);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (qp -> alternate_recip_allowed) {
		argv[0] = rcmd_srch (Q_ALTERNATE_RECIP_ALLOWED, qtbl_que);
		argv[1] = rcmd_srch (TRUE, tbl_bool);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (qp -> content_return_request) {
		argv[0] = rcmd_srch (Q_CONTENT_RETURN_REQUEST, qtbl_que);
		argv[1] = rcmd_srch (TRUE, tbl_bool);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> recip_reassign_prohibited) {
		argv[0] = rcmd_srch (Q_RECIP_REASSIGN_PROHIBITED, qtbl_que);
		argv[1] = rcmd_srch (TRUE, tbl_bool);
		argv[2] = crit2txt (qp -> recip_reassign_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> dl_expansion_prohibited) {
		argv[0] = rcmd_srch (Q_DL_EXP_PROHIBITIED, qtbl_que);
		argv[1] = rcmd_srch (TRUE, tbl_bool);
		argv[2] = crit2txt (qp -> dl_expansion_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> conversion_with_loss_prohibited) {
		argv[0] = rcmd_srch (Q_CONV_WITH_LOSS, qtbl_que);
		argv[1] = rcmd_srch (TRUE, tbl_bool);
		argv[2] = crit2txt (qp -> conversion_with_loss_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp->ua_id != NULLCP) {
		argv[0] = rcmd_srch (Q_UA_ID, qtbl_que);
		argv[1] = qp -> ua_id;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> pp_content_correlator) {
		argv[0] = rcmd_srch (Q_PP_CONT_CORR, qtbl_que);
		argv[1] = qp -> pp_content_correlator;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> general_content_correlator) {
		argv[0] = rcmd_srch (Q_GEN_CONT_CORR, qtbl_que);
		argv[1] = qb2hex (qp -> general_content_correlator);
		argv[2] = crit2txt (qp -> content_correlator_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (qp -> originator_return_address) {
		argv[0] = rcmd_srch (Q_ORIG_RETURN_ADDRESS, qtbl_que);
		argc = 1;
		fname2txt (qp -> originator_return_address,
			   qp -> originator_return_address_crit, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> forwarding_request != NOTOK) {
		argv[0] = rcmd_srch (Q_FORWARDING_REQUEST, qtbl_que);
		argv[1] = int2txt (qp -> forwarding_request);
		argv[2] = crit2txt (qp -> forwarding_request_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> originator_certificate) {
		argv[0] = rcmd_srch (Q_ORIGINATOR_CERT, qtbl_que);
		argv[1] = qb2hex (qp -> originator_certificate);
		argv[2] = crit2txt (qp -> originator_certificate_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (qp -> algorithm_identifier) {
		argv[0] = rcmd_srch (Q_ALGORITHM_ID, qtbl_que);
		argv[1] = qb2hex (qp -> algorithm_identifier);
		argv[2] = crit2txt (qp -> algorithm_identifier_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (qp -> message_origin_auth_check) {
		argv[0] = rcmd_srch (Q_MESSAGE_ORIGIN_AUTH_CHECK, qtbl_que);
		argv[1] = qb2hex (qp -> message_origin_auth_check);
		argv[2] = crit2txt (qp -> message_origin_auth_check_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (qp -> security_label) {
		argv[0] = rcmd_srch (Q_SECURITY_LABEL, qtbl_que);
		argv[1] = qb2hex (qp -> security_label);
		argv[2] = crit2txt (qp -> security_label_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
		free (argv[1]);
	}

	if (qp -> proof_of_submission_request) {
		argv[0] = rcmd_srch (Q_PROOF_OF_SUB, qtbl_que);
		argv[1] = int2txt (qp -> proof_of_submission_request);
		argv[2] = crit2txt (qp -> proof_of_submission_crit);
		argv[3] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (Q_MESSAGE_EXTENSIONS, qtbl_que);
	if (qp -> per_message_extensions) {
		X400_Extension *ext;

		for (ext = qp -> per_message_extensions;
		     ext; ext = ext -> ext_next) {
			argc = 1;
			if (extension2txt (ext, fp, argv, &argc) == NOTOK)
				return NOTOK;
		}
	}

	if (qp -> inbound && qp -> inbound -> li_chan) {
		argv[0] = rcmd_srch (Q_INCHANNEL, qtbl_que);
		argv[1] = qp -> inbound -> li_chan -> ch_name;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if ( qp -> inbound && qp -> inbound -> li_mta) {
		argv[0] = rcmd_srch (Q_INHOST, qtbl_que);
		argv[1] = qp -> inbound -> li_mta;
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (Q_MSGID, qtbl_que);
	argc = 1;
	mpduid2txt (&qp->msgid, argv, &argc);
	if (argc > 1) {
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (Q_TRACE, qtbl_que);
	for (trace = qp->trace; trace; trace = trace -> trace_next) {
		argc = 1;
		trace2txt (trace, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	argv[0] = rcmd_srch (Q_DL_EXP_HISTORY, qtbl_que);
	for (dlexp = qp -> dl_expansion_history; dlexp;
	     dlexp = dlexp -> dlh_next) {
		argc = 1;
		dlexp2txt (dlexp, argv, &argc);
		argv[argc] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}
	if (qp -> dl_expansion_history_crit) {
		argv[0] = rcmd_srch (Q_DL_EXP_HIST_CRIT, qtbl_que);
		argv[1] = crit2txt (qp -> dl_expansion_history_crit);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp -> queuetime != 0) {
		argv[0] = rcmd_srch (Q_QUEUETIME, qtbl_que);
		argv[1] = time2txt (qp->queuetime);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	if (qp->departime != 0) {
		argv[0] = rcmd_srch (Q_DEPARTIME, qtbl_que);
		argv[1] = time2txt (qp->departime);
		argv[2] = NULLCP;
		if (argv2fp (fp, argv) == NOTOK)
			return NOTOK;
	}

	(void) fprintf (fp, "%s\n", rcmd_srch (Q_END, qtbl_que));

	return (ferror (fp) ? NOTOK : OK);
}


static char *Q_msgtype2txt (type)  /* Message-type -> Txt */
int     type;
{
	PP_DBG (("Lib/pp/Q_msgtype2txt (%d)", type));

	switch (type) {
	case MT_UMPDU:
	case MT_PMPDU:
	case MT_DMPDU:
		return rcmd_srch (type, qtbl_mt_type);
	}
	return NULLCP;
}

static char *Q_priority2txt (type)  /* Priority -> Txt */
int     type;
{
	PP_DBG (("Lib/pp/Q_priority2txt('%d')", type));

	switch (type) {
	case PRIO_NORMAL:
	case PRIO_NONURGENT:
	case PRIO_URGENT:
		return rcmd_srch (type, qtbl_priority);
	}
	return NULLCP;
}
