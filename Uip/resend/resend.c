/* resend.c: resend a message onwards */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/resend/RCS/resend.c,v 6.0 1991/12/18 20:42:09 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/resend/RCS/resend.c,v 6.0 1991/12/18 20:42:09 jpo Rel $
 *
 * $Log: resend.c,v $
 * Revision 6.0  1991/12/18  20:42:09  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "prm.h"
#include "q.h"
#include "adr.h"
#include "retcode.h"
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>
#include <varargs.h>

extern	char *strdup();
extern UTC	utclocalise();

extern char *loc_dom_site;
extern char *local_822_chan;
extern char *hdr_822_bp;
extern char	*ia5_bp;

static char   *username = (char *) 0;

#define ADR_TO	1
#define ADR_CC	2
static int	list = ADR_TO;

#define MAXADRS 100
static char	*toadrs[MAXADRS];
static int	tocnt = 0;
static char	*ccadrs[MAXADRS];
static int	cccnt = 0;

static RP_Buf	rps;
static RP_Buf	*rp = &rps;
static struct prm_vars prm;
static Q_struct qs;

static void adios ();

#define prefix(a,b)	(lexnequ ((a), (b), strlen(a)) == 0)

main (argc, argv)
int     argc;
char   *argv[];
{
	char	*cp;

	uip_init (argv[0]);

	pgminit ();

	if (argc <= 1)
		adios (NULLCP, "No addresses given");

	while (--argc > 0) {
		cp = *++argv;
		if (*cp == '-')
			switch (cp[1]) {
			    case 't':
				list = ADR_TO;
				break;
			    case 'c':
				list = ADR_CC;
				break;
			    case 'u':
				username = cp+2;
				if (!*username) { username = *++argv; argc--; }
				username = strdup(username);
				fprintf(stderr, "Username = `%s'\n", username);
				break;
			    default:
				adios (NULLCP, "Bad switch %s", cp);
			}
		else if (list == ADR_TO)
			toadrs[tocnt++] = cp;
		else	ccadrs[cccnt++] = cp;
	}
	fprintf(stderr, "Now send mail\n");
	sendmail ();
	exit (0);
}

pgminit ()
{
	extern struct passwd *getpwuid ();
	extern char *getmailid ();
	struct passwd  *pwdptr;
	int     realid;

	if (!username)
	{	realid = getuid ();

		if ((pwdptr = getpwuid (realid)) == (struct passwd *) NULL)
			err_abrt (RP_PARM, "Unable to locate user's name");

		username = strdup (pwdptr -> pw_name);
	}
}

sendmail ()
{
	ADDR	*adr;
	int i;

	prm_init (&prm);
	q_init (&qs);

	qs.inbound = list_rchan_new (loc_dom_site, local_822_chan);
	qs.encodedinfo.eit_types = list_bpt_new (hdr_822_bp);
	list_bpt_add (&qs.encodedinfo.eit_types, list_bpt_new (ia5_bp));

	if (rp_isbad (io_init (rp)) ||
	    rp_isbad (io_wprm (&prm, rp)) ||
	    rp_isbad (io_wrq (&qs, rp)))
		adios (NULLCP, "Unable to start submit: %s", rp -> rp_line);

	adr = adr_new (username, AD_822_TYPE, 0);
	adr -> ad_status = AD_STAT_DONE;
	adr -> ad_resp = NO;
	if (rp_isbad (io_wadr (adr, AD_ORIGINATOR, rp)))
		adios (NULLCP, "Bad sender %s: %s",
		       username, rp -> rp_line);
	adr_tfree (adr);

	for ( i = 0; i < tocnt; i++) {
		adr = adr_new (toadrs[i], AD_822_TYPE, 0);
		if (rp_isbad (io_wadr (adr, AD_RECIPIENT, rp)))
			adios (NULLCP, "Bad recipeint %s: %s",
			       toadrs[i], rp -> rp_line);
		adr_tfree (adr);
	}
	for ( i = 0; i < cccnt; i++) {
		adr = adr_new (ccadrs[i], AD_822_TYPE, 0);
		if (rp_isbad (io_wadr (adr, AD_RECIPIENT, rp)))
			adios (NULLCP, "Bad recipeint %s: %s",
			       ccadrs[i], rp -> rp_line);
		adr_tfree (adr);
	}

	if (rp_isbad (io_adend (rp)) ||
	    rp_isbad (io_tinit (rp)) ||
	    rp_isbad (io_tpart (hdr_822_bp, 0, rp)))
		adios (NULLCP, "Can't initialise for text submission: %s",
		       rp -> rp_line);
	
	dumpheader();
	doresent();

	dobody ();
}

dumpheader()
{
	char	line[LINESIZE];

	while (fgets (line, LINESIZE, stdin) != NULL) {
		if (line[0] == '\n')
			break;
		if (prefix ("resent-", line) ||
		    prefix ("received:", line) ||
		    prefix ("x400-received", line) ||
		    prefix ("via:", line))
			if (rp_isbad (io_tdata ("Old-", 4)))
				adios (NULLCP, "Data Copy failed");
		if (rp_isbad (io_tdata (line, strlen (line))))
			adios (NULLCP, "Data copy failed");
	}
}

doresent()
{
	char    datbuf[64];
	UTC	now, lut;
	now = utcnow();
	lut = utclocalise(now);
	UTC2rfc(lut, datbuf);		/* rfc822 format date */
	free ((char *) lut);
	sndhdr ("Resent-Date:  ", datbuf);
	sndhdr ("Resent-From:", username);
	doto ();
	(void) sprintf (datbuf, "1.%s", ia5_bp);
	if ( rp_isbad (io_tdend (rp)) || rp_isbad (io_tpart (datbuf, 0, rp)))
		adios (NULLCP, "Can't setup for body part: %s", rp -> rp_line);
}

sndhdr (name, contents)
char    name[],
	contents[];
{
	char    linebuf[LINESIZE];

	(void) sprintf (linebuf, "%-10s%s\n", name, contents);
	if (rp_isbad (io_tdata (linebuf, strlen (linebuf))))
		adios (NULLCP, "Data Copy failed");
}

doto ()
{
	register int    i;

	for (i = 0; i < tocnt; i++)
		sndhdr ("Resent-To:", toadrs[i]);
	for (i = 0; i < cccnt; i++)
		sndhdr ("Resent-Cc:", ccadrs[i]);
}

dobody ()
{
	char    buffer[BUFSIZ];
	register int    i;

	while (!feof (stdin) && !ferror (stdin) &&
	    (i = fread (buffer, sizeof (char), sizeof (buffer), stdin)) > 0)
		if (rp_isbad (io_tdata (buffer, i)))
			adios (NULLCP, "Problem writing body");

	if (ferror (stdin))
		adios (NULLCP, "Problem reading body");
	if (rp_isbad (io_tdend (rp)) || rp_isbad (io_tend (rp)))
		adios (NULLCP, "Error terminating: %s", rp -> rp_line);
}

#ifndef lint
static void    adios (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    _ll_log (pp_log_norm, LLOG_FATAL, ap);
    
    va_end (ap);

    _exit (1);
}
#else
/* VARARGS2 */

static void    adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif
