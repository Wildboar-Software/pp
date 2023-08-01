/* ll_log.h: logging definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/ll_log.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: ll_log.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_LOG_
#define _H_LOG_



#include        <isode/logger.h>


extern  LLog    *pp_log_norm,
		*pp_log_stat,
		*pp_log_oper;


/* can't leave spaces in macro definitions... PP_LOG ( blows up! */

#define PP_LOG(level, args)	SLOG (pp_log_norm, level, NULLCP, args)
#define PP_SLOG(level, str, args) \
				SLOG (pp_log_norm, level, str, args)
#define PP_OPER(what, args)	if (pp_log_oper -> ll_events & (LLOG_FATAL)) { \
	(void) ll_log (pp_log_oper, LLOG_FATAL, what, "%s", ll_preset args); \
	(void) ll_log (pp_log_norm, LLOG_FATAL, what, "%s", ll_preset args); \
	} \
	else
#define PP_STAT(args)		SLOG (pp_log_stat, LLOG_NOTICE, NULLCP, args)
#define PP_NOTICE(args)		SLOG(pp_log_norm, LLOG_NOTICE, NULLCP, args)

/* Tracing of PP PDUS */
#define PDU_READ	(1)
#define PDU_WRITE	(0)

#if PP_DEBUG > 0 

#ifdef PEPSY_VERSION 
#ifdef __STDC__

#define PP_PDUP(fnx,pe,text,rw) \
	if (pp_log_norm -> ll_events & LLOG_PDUS) { \
		pvpdu (pp_log_norm, print_##fnx##_P, pe, text, rw); \
} \
else

#define PP_PDU(fnx,pe,text,rw) \
	if (pp_log_norm -> ll_events & LLOG_PDUS) { \
		pvpdu (pp_log_norm, fnx##_P, pe, text, rw); \
} \
else

#else /* !STDC */

#define PP_PDUP(fnx,pe,text,rw) \
	if (pp_log_norm -> ll_events & LLOG_PDUS) { \
		pvpdu (pp_log_norm, print_/**/fnx/**/_P, pe, text, rw); \
} \
else

#define PP_PDU(fnx,pe,text,rw) \
	if (pp_log_norm -> ll_events & LLOG_PDUS) { \
		pvpdu (pp_log_norm, fnx/**/_P, pe, text, rw); \
} \
else

#endif /* STDC */

#else /* !PEPSY_VERSION */

#ifdef __STDC__

#define PP_PDUP(fnx,pe,text,rw)\
	if (pp_log_norm -> ll_events & LLOG_PDUS) {\
               vpdu (pp_log_norm, print_##fnx, pe, text, rw);\
}\
else

#else /* !__STDC__ */

#define PP_PDUP(fnx,pe,text,rw)\
	if (pp_log_norm -> ll_events & LLOG_PDUS) {\
               vpdu (pp_log_norm, print_/**/fnx, pe, text, rw);\
}\
else

#endif /* __STDC__ */

#define PP_PDU(fnx,pe,text,rw) \
	if (pp_log_norm -> ll_events & LLOG_PDUS) { \
		vpdu (pp_log_norm, fnx, pe, text, rw); \
} \
else

#endif /* PEPSY_VERSION */

#else /* !DEBUG */

#define PP_PDU(fnx,pe,text,rw)
#define PP_PDUP(fnx,pe,text,rw)

#endif

/* general PP Tracing */
#if PP_DEBUG > 0
#define PP_TRACE(args)  SLOG (pp_log_norm, LLOG_TRACE, NULLCP, args)
#else
#define PP_TRACE(args)
#endif


/* Low level PP Tracing */
#if PP_DEBUG > 1
#define PP_DBG(args)    SLOG (pp_log_norm, LLOG_DEBUG, NULLCP, args)
#else
#define PP_DBG(args)
#endif

#endif
