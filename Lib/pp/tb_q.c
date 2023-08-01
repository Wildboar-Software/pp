/* tb_q.c: Queue tables - of the keywds found in ADDR Control Files */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_q.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/tb_q.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: tb_q.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"q.h"
#include        "tb_q.h"
#include        <isode/cmd_srch.h>


/* ------------------------------------------------------------------------ */



CMD_TABLE  qtbl_que[] = { /* message-envelope-parameters */
	"algorithm-id",			Q_ALGORITHM_ID,
	"alt-recip-allowed",		Q_ALTERNATE_RECIP_ALLOWED,
	"cont-return",			Q_CONTENT_RETURN_REQUEST,
	"content-corr",			Q_GEN_CONT_CORR,
	"Content-type",                 Q_CONTENT_TYPE,
	"conv-with-loss",		Q_CONV_WITH_LOSS,
	"Deferred-time",                Q_DEFERREDTIME,
	"Departure-time",               Q_DEPARTIME,
	"disclose-recips",		Q_DISCLOSE_RECIPS,
	"dl-exp-hist-crit",		Q_DL_EXP_HIST_CRIT,
	"dl-exp-history",		Q_DL_EXP_HISTORY,
	"dl-exp-prohibited",		Q_DL_EXP_PROHIBITIED,
	"Encoded-info",                 Q_ENCODED_INFO,
	"forward-req",			Q_FORWARDING_REQUEST,
	"implicit-conv",		Q_IMPLICIT_CONVERSION,
	"Inbound-channel",              Q_INCHANNEL,
	"Inbound-mta",                 	Q_INHOST,
	"latest-time",			Q_LATESTTIME,
	"message-extension",		Q_MESSAGE_EXTENSIONS,
	"message-origin-auth-check",	Q_MESSAGE_ORIGIN_AUTH_CHECK,
	"Message-size",                 Q_MSGSIZE,
	"Message-type",                 Q_MSGTYPE,
	"MsgId",                        Q_MSGID,
	"Number-warnings",              Q_NWARNS,
	"orig-return-addr",		Q_ORIG_RETURN_ADDRESS,
	"Original-encoded-info",	Q_ORIG_ENCODED_INFO,
	"originator-certificate",	Q_ORIGINATOR_CERT,
	"pp-cont-corr",			Q_PP_CONT_CORR,
	"Priority",                     Q_PRIORITY,
	"proof-of-sub",			Q_PROOF_OF_SUB,
	"Queued-time",                  Q_QUEUETIME,
	"recip-reassign-prohib",	Q_RECIP_REASSIGN_PROHIBITED,
	"Return-interval",              Q_RETINTERVAL,
	"security-label",		Q_SECURITY_LABEL,
	"Start-of-MsgEnvAddr",          Q_END,
	"Trace",                        Q_TRACE,
	"Ua-id",                        Q_UA_ID,
	"Warning-interval",             Q_WARNINTERVAL,
	0,                              -1
	};
int n_qtbl_que = ((sizeof(qtbl_que)/sizeof(CMD_TABLE)) -1);


CMD_TABLE  qtbl_mt_type[] = {  /* message-type */
	"User-Mpdu",     MT_UMPDU,
	"Probe-Mpdu",    MT_PMPDU,
	"Deliv-Mpdu",    MT_DMPDU,
	0,               -1
	};


CMD_TABLE  qtbl_priority[] = { /* message-priority */
	"normal",       PRIO_NORMAL,
	"non-urgent",   PRIO_NONURGENT,
	"urgent",       PRIO_URGENT,
	0,              -1
	};
