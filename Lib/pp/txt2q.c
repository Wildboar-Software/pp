/* txt2q.c: text encoding into Q struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2q.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/txt2q.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: txt2q.c,v $
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

#define txt2crit(n)	cmd_srch ((n), tbl_crit)
#define	txt2int(n)	atoi (n)

extern CMD_TABLE
		tbl_bool [],
		tbl_crit [],
		qtbl_que [/* message-envelope-parameters */],
		qtbl_mt_type [/* message-type */],
		qtbl_con_type [/* content-type */],
		qtbl_priority [/* message-priority */];

extern int n_qtbl_que;
extern struct qbuf *hex2qb ();
extern X400_Extension *txt2extension ();
static int Q_txt2msgtype ();
static int Q_txt2priority ();

/* -------------------  Text File -> Memory  ------------------------------ */


int txt2q (qp, startlineoffset, argv, argc)  /* Txt -> MessageEnvelopePrarmeters */
Q_struct        *qp;
long		startlineoffset;
char            **argv;
int             argc;
{
	int     key;

	PP_DBG (("Lib/pp/txt2q('%s' %d)", argv[0], argc));

	key = cmdbsrch (argv[0], qtbl_que, n_qtbl_que);

	switch (key) {
	    case Q_MSGTYPE:
		return (Q_txt2msgtype (&qp->msgtype, argv[1]));

	    case Q_MSGSIZE:
		if (!isstr (argv[1]))  return OK;
		qp->msgsize = atoi (argv[1]);
		return OK;

	    case Q_DEFERREDTIME:
		if (!isstr (argv[1]))  return OK;
		return (txt2time (argv[1], &qp->defertime));

	    case Q_LATESTTIME:
		if (!isstr (argv[1])) return OK;
		if (txt2time (argv[1], &qp ->latest_time) == NOTOK)
			return NOTOK;
		if (!isstr (argv[2])) return OK;
		qp -> latest_time_crit = txt2crit (argv[2]);
		return OK;

	    case Q_NWARNS:
		if (!isstr (argv[1]))  return OK;
		qp->nwarns = atoi (argv[1]);
		qp->nwarns_offset = startlineoffset + strlen(argv[0]) + 1;
		return OK;

	    case Q_WARNINTERVAL:
		if (!isstr (argv[1])) return OK;
		qp->warninterval = atoi (argv[1]);
		return OK;

	    case Q_RETINTERVAL:
		if (!isstr (argv[1]))  return OK;
		qp->retinterval = atoi (argv[1]);
		return OK;

	    case Q_CONTENT_TYPE:
		if (!isstr (argv[1])) return OK;
		qp->cont_type = strdup(argv[1]);
		return OK;

	    case Q_ENCODED_INFO:
		if (!isstr (argv[1]))  return OK;
		if (txt2encodedinfo (&qp->encodedinfo, &argv[1], argc - 1)
		    == NOTOK)  return NOTOK;
		return OK;

	    case Q_ORIG_ENCODED_INFO:
		if (!isstr (argv[1]))  return OK;
		if (txt2encodedinfo (&qp->orig_encodedinfo, &argv[1], argc - 1)
		    == NOTOK)  return NOTOK;
		return OK;

	    case Q_PRIORITY:
		if (!isstr (argv[1]))  return OK;
		return (Q_txt2priority (&qp->priority, argv[1]));
	    
	    case Q_DISCLOSE_RECIPS:
		if (!isstr (argv[1]))  return OK;
		qp -> disclose_recips = cmd_srch (argv[1], tbl_bool);
		return OK;

	    case Q_IMPLICIT_CONVERSION:
		if (!isstr(argv[1])) return OK;
		qp -> implicit_conversion_prohibited = cmd_srch (argv[1], tbl_bool);
		return OK;

	    case Q_ALTERNATE_RECIP_ALLOWED:
		if (!isstr(argv[1])) return OK;
		qp -> alternate_recip_allowed = cmd_srch (argv[1], tbl_bool);
		return OK;

	    case Q_CONTENT_RETURN_REQUEST:
		if (!isstr(argv[1])) return OK;
		qp -> content_return_request = cmd_srch (argv[1], tbl_bool);
		return OK;

	    case Q_RECIP_REASSIGN_PROHIBITED:
		if (!isstr (argv[1])) return OK;
		qp -> recip_reassign_prohibited = cmd_srch (argv[1], tbl_bool);
		if (!isstr (argv[2])) return OK;
		qp -> recip_reassign_crit = txt2crit (argv[2]);
		return OK;
		
	    case Q_DL_EXP_PROHIBITIED:
		if (!isstr (argv[1])) return OK;
		qp -> dl_expansion_prohibited = cmd_srch (argv[1], tbl_bool);
		if (!isstr (argv[2])) return OK;
		qp -> dl_expansion_crit = txt2crit (argv[2]);
		return OK;

	    case Q_CONV_WITH_LOSS:
		if (!isstr (argv[1])) return OK;
		qp -> conversion_with_loss_prohibited =
			cmd_srch (argv[1], tbl_bool);
		if (!isstr (argv[2])) return OK;
		qp -> conversion_with_loss_crit =
			txt2crit (argv[2]);
		return OK;

	    case Q_UA_ID:
		if (!isstr (argv[1])) return OK;
		qp->ua_id = strdup (argv[1]);
		return OK;
		
	    case Q_PP_CONT_CORR:
		if (!isstr (argv[1])) return OK;
		qp -> pp_content_correlator = strdup (argv[1]);
		return OK;

	    case Q_GEN_CONT_CORR:
		if (!isstr(argv[1])) return OK;
		qp -> general_content_correlator = hex2qb (argv[1]);
		if (!isstr (argv[2])) return OK;
		qp -> content_correlator_crit = txt2crit (argv[2]);
		return OK;

	    case Q_ORIG_RETURN_ADDRESS:
		return txt2fname (&qp -> originator_return_address,
				  &qp -> originator_return_address_crit,
				  argv + 1, argc -1);

	    case Q_FORWARDING_REQUEST:
		if (!isstr (argv[0])) return OK;
		qp -> forwarding_request = txt2int (argv[1]);
		if (!isstr (argv[2])) return OK;
		qp -> forwarding_request_crit = txt2crit (argv[2]);
		return OK;

	    case Q_ORIGINATOR_CERT:
		if (!isstr (argv[1])) return OK;
		qp -> originator_certificate = hex2qb (argv[1]);
		if (!isstr (argv[2])) return OK;
		qp -> originator_certificate_crit = txt2crit (argv[2]);
		return OK;
			
	    case Q_ALGORITHM_ID:
		if (!isstr (argv[1])) return OK;
		qp -> algorithm_identifier = hex2qb (argv[1]);
		if (!isstr (argv[2])) return OK;
		qp -> algorithm_identifier_crit = txt2crit (argv[2]);
		return OK;

	    case Q_MESSAGE_ORIGIN_AUTH_CHECK:
		if (!isstr (argv[1])) return OK;
		qp -> message_origin_auth_check = hex2qb (argv[1]);
		if (!isstr (argv[2])) return OK;
		qp -> message_origin_auth_check_crit = txt2crit (argv[2]);
		return OK;

	    case Q_SECURITY_LABEL:
		if (!isstr (argv[1])) return OK;
		qp -> security_label = hex2qb (argv[1]);
		if (!isstr (argv[2])) return OK;
		qp -> security_label_crit = txt2crit (argv[2]);
		return OK;

	    case Q_PROOF_OF_SUB:
		if (!isstr (argv[1])) return OK;
		qp -> proof_of_submission_request = txt2int (argv[1]);
		if (!isstr (argv[2])) return OK;
		qp -> proof_of_submission_crit = txt2crit (argv[2]);
		return OK;

	    case Q_MESSAGE_EXTENSIONS:
		{
			X400_Extension *ext, **extp;

			ext = txt2extension (&argv[1], argc - 1);

			for (extp = &qp-> per_message_extensions;
			     *extp; extp = &(*extp) -> ext_next)
				continue;
			*extp = ext;
		}
		return OK;

	    case Q_INCHANNEL:
		if (!isstr (argv[1]))  return NOTOK;
		if (qp -> inbound == NULL)
			qp->inbound = list_rchan_new (NULLCP, argv[1]);
		else	return list_rchan_schan (qp->inbound, argv[1]);
		return OK;

	    case Q_INHOST:
		if (!isstr (argv[1]))  return NOTOK;
		if (qp -> inbound == NULL)
			qp -> inbound = list_rchan_new (argv[1], NULLCP);
		else	return list_rchan_ssite (qp -> inbound, argv[1]);
		return OK;

	    case Q_MSGID:
		if (txt2mpduid (&qp->msgid, &argv[1], argc - 1) == NOTOK)
			return NOTOK;
		return OK;
	    case Q_TRACE:
		if (txt2trace (&qp->trace, &argv[1], argc - 1) == NOTOK)
			return NOTOK;
		return OK;

	    case Q_DL_EXP_HISTORY:
		return txt2dlexp (&qp -> dl_expansion_history,
				  argv + 1, argc - 1);

	    case Q_DL_EXP_HIST_CRIT:
		if (!isstr (argv[1])) return OK;
		qp -> dl_expansion_history_crit = txt2int (argv[1]);
		return OK;

	    case Q_QUEUETIME:
		if (!isstr (argv[1]))  return OK;
		return (txt2time (argv[1], &qp->queuetime));

	    case Q_DEPARTIME:
		if (!isstr (argv[1]))  return OK;
		return (txt2time (argv[1], &qp->departime));

	    case Q_END:
		return Q_END;
	}


	PP_LOG (LLOG_EXCEPTIONS,
		("Lib/pp/txt2q Unable to parse '%s'", argv[0]));
	return NOTOK;
}


static int Q_txt2msgtype (msgtype, keywd)  /* Txt -> Message-type */
int     *msgtype;
char    *keywd;
{
	PP_DBG (("Lib/pp/Q_txt2msgtype(%s)", keywd));

	switch (*msgtype = cmd_srch (keywd, qtbl_mt_type)) {
	case MT_UMPDU:
	case MT_PMPDU:
	case MT_DMPDU:
		return OK;
	}
	return NOTOK;
}



static int Q_txt2priority  (priority, keywd)  /* Txt -> Priority */
int     *priority;
char    *keywd;
{
	PP_DBG (("Lib/pp/Q_txt2priority(%s)", keywd));

	switch (*priority = cmd_srch (keywd, qtbl_priority)) {
	case PRIO_NORMAL:
	case PRIO_NONURGENT:
	case PRIO_URGENT:
		return OK;
	}
	return NOTOK;
}

struct qbuf *hex2qb (hexstr)
char	*hexstr;
{
	char	*str;
	int	n;
	struct qbuf *qb;

	str = smalloc ((n = strlen (hexstr)) / 2);
	n = implode ((unsigned char *)str, hexstr, n);

	qb = str2qb (str, n, 1);
	free (str);
	return qb;
}

int txt2fname (fname, crit, argv, argc)
FullName	**fname;
char	*crit;
char	**argv;
int	argc;
{
	FullName *fn;

	*fname = fn = (FullName *) smalloc (sizeof *fn);
	if (argc < 1) return OK;
	if (*argv)
		fn -> fn_addr = strdup (*argv);
	argv ++; argc --;

	if (argc < 1) return OK;
	if (*argv)
		fn -> fn_dn = strdup (*argv);
	argv ++; argc --;

	if (argc < 1) return OK;
	if (*argv)
		*crit = txt2crit (*argv);
	return OK;
}

int txt2dlexp (dlp, argv, argc)
DLHistory **dlp;
char	**argv;
int	argc;
{
	if (argc < 1)
		return OK;
	for (;*dlp; dlp = &(*dlp) -> dlh_next)
		continue;

	*dlp = (DLHistory *) smalloc (sizeof **dlp);
	bzero ((char *)*dlp , sizeof **dlp);

	if (argc < 1) return OK;
	if (*argv)
		(*dlp) -> dlh_addr = strdup (*argv);
	argv ++; argc --;

	if (argc < 1) return OK;
	if (*argv)
		(*dlp) -> dlh_dn = strdup (*argv);
	argv ++; argc --;

	if (argc < 1) return OK;

	return txt2time (*argv, &(*dlp) -> dlh_time);
}
