/* mail: very dumb mail interface */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/mail/RCS/mail.c,v 6.0 1991/12/18 20:35:57 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/mail/RCS/mail.c,v 6.0 1991/12/18 20:35:57 jpo Rel $
 *
 * $Log: mail.c,v $
 * Revision 6.0  1991/12/18  20:35:57  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"
#include <stdarg.h>

#define		TO		1
#define		CC		2
#define		SUBJECT		3

#define		MAX_TO		100
#define		MAX_CC		100



static char	*to[MAX_TO],
		*cc[MAX_CC],
		subject[BUFSIZ],
		*invo_name;

static int	param = TO,     /* initial state */
		lastparam = TO,
		toptr,
		ccptr;

static RP_Buf	rps, *rp = &rps;

static void do_text ();
static void mail_init ();
static void do_addrs ();
void uip_init (char *pname);

/* ---------------------  Start	 Routines  -------------------------------- */


void main (argc, argv)
int	argc;
char	**argv;
{
	uip_init (invo_name = argv[0]);

	if (argc <= 1 )
		user_err (1,
			 "Usage: %s to [-c cc] [-s subject] [-t to]\n",
			  argv[0]);

	while (--argc > 0) {
		argv ++;
		if (**argv == '-')
			switch (argv[0][1]) {
			case 't':
				lastparam = param = TO;
				break;
			case 'c':
				lastparam = param = CC;
				break;
			case 's':
				param = SUBJECT;
				break;
			default:
				user_err (1, "Unknown switch %s\n", *argv);
				break;

			}   /* -- end of switch -- */

		else {
			switch (param) {
			case TO:
				if (toptr >= MAX_TO)
					user_err (1,
						 "Too many 'to' addresses\n");
				to[toptr++] = *argv;
				break;
			case CC:
				if (ccptr >= MAX_TO)
					user_err (1,
						 "Too many 'cc' addresses\n");
				cc[ccptr++] = *argv;
				break;
			case SUBJECT:
				(void) strcat (subject, *argv);
				(void) strcat (subject, " ");
				param = lastparam;
				break;

			}  /* -- end of switch -- */

		}   /* -- end of else -- */

	}  /* -- end of while -- */


	if (toptr == 0 && ccptr == 0)
		user_err (1, "No recipients given\n");

	mail_init();
	do_addrs();
	do_text();
	exit (0);
}




/* ---------------------  Start	 Routines  -------------------------------- */

static void mail_init()
{
	if (pps_init (subject, rp) == NOTOK)
		sys_err (1, "pps_init", rp, "Initialisation failed");
}




static void do_addrs()
{
	int	i;

	for (i = 0; i < toptr; i++)
		if (pps_adr (to[i], rp) == NOTOK)
			sys_err (1, "pps_adr", rp, "Address %s failed", to[i]);

	if (pps_cc(rp) == NOTOK)
		sys_err (1, "pps_cc", rp, "CC list failed");

	for (i = 0; i < ccptr; i++)
		if (pps_adr (cc[i], rp) == NOTOK)
			sys_err (1, "pps_adr", rp, "Address %s failed", cc[i]);

	if (pps_aend(rp) == NOTOK)
		sys_err (1, "pps_adend", rp, "Address end failed");
}




static void do_text()
{
	if (pps_tinit(rp) == NOTOK)
		sys_err (1, "pps_tinit", rp, "Can't initalise for text");

	if (pps_file (stdin, rp) == NOTOK)
		sys_err (1, "pps_file", rp, "Text copy failed");

	if (pps_end (OK, rp) == NOTOK)
		sys_err (1, "pps_end", rp, "Termination phase failed");

}




/* ---------------------  Error	 Routines  -------------------------------- */

void user_err (int code, char *fmt, ...)
{
	char	buf[BUFSIZ];
	va_list ap;

	va_start (ap, fmt);

	_asprintf (buf, NULLCP, fmt, ap);
	fprintf (stderr, "%s: %s\n", invo_name, buf);
	va_end (ap);

	exit (code);
}

void sys_err (int code, char *fmt, ...)
{
	char	buf[BUFSIZ];
	RP_Buf	*rp;
	char	*proc;
	va_list ap;

	va_start (ap, fmt);
	proc = va_arg (ap, char *);
	rp = va_arg (ap, RP_Buf *);

	_asprintf (buf, NULLCP, fmt, ap);
	fprintf (stderr, "%s: %s %s [%s]\n", invo_name, proc,
		 buf, rp -> rp_line);
	va_end (ap);
	exit (code);
}
