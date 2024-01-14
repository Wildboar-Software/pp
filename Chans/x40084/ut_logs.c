/* ut_logs.c - X400 logging and error routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40084/RCS/ut_logs.c,v 6.0 1991/12/18 20:13:50 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/ut_logs.c,v 6.0 1991/12/18 20:13:50 jpo Rel $
 *
 * $Log: ut_logs.c,v $
 * Revision 6.0  1991/12/18  20:13:50  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/rtsap.h>
#include <stdarg.h>


#define RC_BASE         0x80



/* -- externals -- */
extern char             *sys_errlist[];
extern int              sys_nerr;


/* -- globals -- */
char 			reason[BUFSIZ];


/* -- static -- */
static char *reason_err0[] = {
		"no specific reason stated",
		"user receiving ability jeopardized",
		"reserved(1)",
		"user sequence error",
		"reserved(2)",
		"local SS-user error",
		"unreceoverable procedural error"
		};
static int reason_err0_cnt = sizeof reason_err0 / sizeof reason_err0[0];


static char *reason_err8[] = {
		"demand data token"
		};
static int reason_err8_cnt = sizeof reason_err8 / sizeof reason_err8[0];



/* -- local routines -- */
char		*SReportString();
void		adios();
void		advise();
void		rts_adios();
void		rts_advise();
void		rts_exceptions();
void		do_reason();





/* ---------------------  Begin  Routines  -------------------------------- */




char   *SReportString (code)
int     code;
{
	register int    fcode;
	static char     buffer[BUFSIZ];

	if (code == SP_PROTOCOL)
		return "SS-provider protocol error";

	code &= 0xff;
	if (code & RC_BASE)
		if ((fcode = code & ~RC_BASE) < reason_err8_cnt)
			return reason_err8[fcode];
	else
		if (code < reason_err0_cnt)
			return reason_err0[code];

	(void) sprintf (buffer, "unknown reason code 0x%x", code);
	return buffer;
}




/* ---------------------  Logging Routines  ------------------------------- */



#ifdef lint
/*VARARGS2*/
void do_reason (str)
char   *str;
{
	do_reason (str);
}

#else
void do_reason (va_alist)
va_dcl
{
	va_list		ap;

	bzero (reason, BUFSIZ);
	va_start (ap);
	_asprintf (reason, NULLCP, ap);
	va_end (ap);
	PP_OPER (NULLCP, ("%s", reason));
}
#endif





/*ERRORS*/

void rts_adios (rta, event)
register struct RtSAPabort      *rta;
char                            *event;
{
	rts_advise (rta, event);
	_exit (1);
}




void rts_advise (rta, event)    /* -- updates x40084 logs + OPER log -- */
register struct RtSAPabort      *rta;
char                            *event;
{
	char                    buffer[BUFSIZ];

	if (rta->rta_cc > 0)
		(void) sprintf (buffer, "[%s] %*.*s",
			RtErrString (rta->rta_reason),
			rta->rta_cc, rta->rta_cc, rta->rta_data);
	else
		(void) sprintf (buffer, "[%s]",
			RtErrString (rta->rta_reason));

	do_reason ("%s: %s", event, buffer);
}

void rts_exceptions (rta, message)     /* -- only updates x400 logs -- */
register struct RtSAPabort      *rta;
char                            *message;
{
	char                    buffer[BUFSIZ];

	if (rta->rta_cc > 0)
		(void) sprintf (buffer, "[%s] %*.*s",
			RtErrString (rta->rta_reason),
			rta->rta_cc, rta->rta_cc, rta->rta_data);
	else
		(void) sprintf (buffer, "[%s]",
			RtErrString (rta->rta_reason));

	PP_LOG (LLOG_EXCEPTIONS, ("%s, %s", message, buffer));
}

void adios (char *what, char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    _ll_log (pp_log_oper, LLOG_FATAL, ap);
    _ll_log (pp_log_norm, LLOG_FATAL, ap);
    va_end (ap);
    _exit (1);
}

void advise (char *what, char *fmt, ...)
{
    int code;
    va_list ap;
    va_start (ap, fmt);
    code = va_arg (ap, int);
    _ll_log (pp_log_norm, code, ap);
    va_end (ap);
}
