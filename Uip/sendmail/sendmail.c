/* sendmail.c: A fake sendmail for those that really need it */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/sendmail/RCS/sendmail.c,v 6.0 1991/12/18 20:42:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/sendmail/RCS/sendmail.c,v 6.0 1991/12/18 20:42:15 jpo Rel $
 *
 * $Log: sendmail.c,v $
 * Revision 6.0  1991/12/18  20:42:15  jpo
 * Release 6.0
 *
 */



#include <signal.h>
#include <pwd.h>
#include "util.h"
#include "prm.h"
#include "q.h"
#include "adr.h"
#include "retcode.h"
#include "ap.h"
#include "io.h"
#include <stdarg.h>

extern  char	*loc_dom_site;
extern	char	*chndfldir;
extern	char 	*hdr_822_bp;
extern 	char	*ia5_bp;
extern 	char	*local_822_chan;
extern UTC	utclocalise();

extern struct passwd *getpwuid ();
extern char *getenv();
extern char *dupfpath();

static char	*SMTPSRVR = "smtpsrvr";

static char	*fullname;	/* sender's full name */
static char	*from;		/* sender's mail address */
static int	verify;
static int	badaddrs;
static int	rewritefrom;
static int	watch;
static int	extract_mode;
static char	*eaddr[1024];
static int	neaddr;
static char 	firstline[BUFSIZ];

static struct prm_vars prm;
static Q_struct qs;
static RP_Buf rps;
static RP_Buf *rp = &rps;

static SFD	die(int sig);
static void adios (char *what, char* fmt, ...);

static char	tbuf[BUFSIZ];

int getqbchar();
void read_header();
void send_address(char *addr);
void doheader(char **oobto);
int isheader(char *line);
void dofrom();
int dobody();
void smtp();

void uip_init (char *pname);

/*ARGSUSED*/
int main(int argc, char** argv)
{
	struct passwd  *pwdptr;
	register char *p;
	register char **av;
	char **oobto = NULL;
	ADDR	*adr;

	uip_init(argv[0]);
	pp_log_norm -> ll_stat |= LLOGTTY;

	if ((pwdptr = getpwuid (getuid())) == (struct passwd *) NULL)
		adios (NULLCP, "Unable to locate user's name");

	(void) sprintf (tbuf, "%s@%s", pwdptr -> pw_name, loc_dom_site);
	from = strdup (tbuf);

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT, die);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		(void) signal(SIGHUP, die);
	(void) signal(SIGTERM, die);
	(void) signal(SIGPIPE, die);

	fullname = getenv("NAME");
       /* nobody is going to use this anyway */
       if (p = index(pwdptr->pw_gecos,','))
               *p = 0;
       if (!fullname)
               fullname = strdup(pwdptr->pw_gecos);


	av = argv;
	while ((p = *++av) != NULL && p[0] == '-') {
		switch (p[1]) {
		case 'b':	/* operations mode */
			switch (p[2]) {
			case 'a':	/* smtp on stdin */
			case 's':	/* smtp on stdin */
				smtp();
				exit(98);	/* should never happen */
			case 'm':	/* just send mail */
				continue;
			case 'v':	/* verify mode */
				verify++;
				continue;
			default:
				adios (NULLCP, "Invalid operation mode %c", p[2]);
			}
			continue;

		case 'f':	/* from address */
		case 'r':	/* obsolete -f flag */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || *p == '-'))
			{
				p = *++av;
				if (p == NULL || *p == '-') {
					adios (NULLCP, "No \"from\" person");
					av--;
					continue;
				}
			}
			if (rewritefrom) {
				adios (NULLCP, "More than one \"from\" person");
				continue;
			}
			from = p;
			rewritefrom++;
			continue;

		case 'F':	/* set full name */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || *p == '-'))
			{
				adios (NULLCP, "Bad -F flag");
				av--;
				continue;
			}
			fullname = p;
			continue;

		case 'h':	/* hop count */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || !isdigit(*p)))
			{
				adios (NULLCP, "Bad hop count (%s)", p);
				av--;
				continue;
			}
			continue;	/* Ignore */

		case 't':	/* read recipients from message */
			extract_mode = 1;
			continue;


		case 'v':	/* give blow-by-blow description */
			watch++;
			continue;

		case 'T':	/* set timeout interval */
		case 'C':	/* select configuration file (already done) */
		case 'c':	/* connect to non-local mailers */
		case 'd':	/* debug */
		case 'e':	/* error message disposition */
		case 'i':	/* don't let dot stop me */
		case 'm':	/* send to me too */
		case 'n':	/* don't alias */
		case 'o':	/* set option, handled in line */
		case 's':	/* save From lines in headers */
			continue;
		}	
}

	setuid(getuid());

	prm_init (&prm);
	prm.prm_opts = PRM_ACCEPTALL;
	q_init (&qs);
	qs.inbound = list_rchan_new (loc_dom_site, local_822_chan);
	qs.encodedinfo.eit_types = list_bpt_new (hdr_822_bp);
	list_bpt_add (&qs.encodedinfo.eit_types, list_bpt_new (ia5_bp));

	if (rp_isbad (io_init (rp)) ||
	    rp_isbad (io_wprm (&prm, rp)) ||
	    rp_isbad (io_wrq (&qs, rp)))
		adios (NULLCP, "Unable to submit mail at this time: %s",
		       rp -> rp_line);

	adr = adr_new (from, AD_822_TYPE, 0);
	adr -> ad_status = AD_STAT_DONE;
	adr -> ad_resp = NO;
	if (rp_isbad (io_wadr (adr, AD_ORIGINATOR, rp)))
		adios (NULLCP, "Unable to submit mail from %s: %s",
			from, rp -> rp_line);
	adr_tfree (adr);

	if (*av == NULL && !extract_mode) {
		adios (NULLCP, "Usage: /usr/lib/sendmail [flags] addr...");
	}
	read_header ();
	oobto = av;
	while (*av)
		send_address(*av++);
	if (extract_mode) {
		int i;

		for (i = 0; i < neaddr; i++)
			send_address (eaddr[i]);
	}
	if (rp_isbad (io_adend (rp)))
		adios (NULLCP, "Problem with address list.");
   	if (verify) {
		io_end(NOTOK);
		exit(badaddrs ? 1 : 0);
	}
	if (rp_isbad (io_tinit (rp)) || rp_isbad (io_tpart (hdr_822_bp, 0, rp)))
		adios (NULLCP, "Problem initialising for text: %s", rp -> rp_line);

	doheader(oobto);

	exit(dobody());
}

static struct qbuf *Header;
static struct qbuf *qbase;
static int nleft;
static char *qbptr;

static void qb_read_init (qp)
struct qbuf *qp;
{
	char *cp;
	qbase = qp;

	cp = index (qp -> qb_data, ':');
	if (cp == NULL)
		adios (NULLCP, "Internal error - missing ':'");
	cp ++;
	nleft = qp -> qb_len - (cp - qp -> qb_data);
	qbptr = cp;
}

int getqbchar ()
{
	if (nleft > 0) {
		nleft --;
		return *qbptr++;
	}
	qbase = qbase -> qb_forw;
	if (qbase == Header)
		return EOF;
	if (qbase -> qb_data[0] != ' ' && qbase -> qb_data[0] != '\t')
		return EOF;
	nleft = qbase -> qb_len;
	qbptr = qbase -> qb_data;
	return getqbchar ();
}

void read_header ()
{
	struct qbuf *qp;
	char buffer[BUFSIZ];
	AP_ptr ap;

	Header = (struct qbuf *)malloc (sizeof *Header);
	Header -> qb_forw = Header -> qb_back = Header;

	while (fgets (buffer, sizeof buffer, stdin) != NULL) {
		if (isheader(buffer)) { /* end of header ? */
			qp = str2qb (buffer, strlen (buffer), 0);
			insque (qp, Header -> qb_back);
		}
		else {
			strcpy (firstline, buffer);
			break;
		}
	}

	if (!extract_mode)
		return;
	
	for (qp = Header -> qb_forw; qp != Header; qp = qp -> qb_forw) {
		if (lexnequ (qp -> qb_data, "to:", 3) == 0 ||
		    lexnequ (qp -> qb_data, "cc:", 3) == 0 ||
		    lexnequ (qp -> qb_data, "bcc:", 4) == 0) {
			qb_read_init (qp);
			for (;;) {
				ap_pinit (getqbchar);
				switch (ap_1adr ()) {
				    case DONE:
					break;

				    case NOTOK:
					adios (NULLCP, "error in address");
					break;

				    default:
					ap = ap_t2s (ap_pstrt, &eaddr[neaddr++]);
					ap_free (ap);
					continue;
				}
				break;
			}
		}
	}
}


void send_address (addr)
char    *addr;
{
	int     retval;
	ADDR	*adr;

	if (watch) {
		printf ("%s:  ", addr);
		(void) fflush (stdout);
	}

	adr = adr_new (addr, AD_822_TYPE, 0);
	if (rp_isbad (retval = io_wadr (adr, AD_RECIPIENT, rp)))
		adios (NULLCP, "Problem in send_address: [%s] %s.",
		       rp_valstr (retval),
		       rps.rp_line);

	adr_tfree (adr);

	switch (rp_gval (rp -> rp_val)) {
	case RP_AOK: 
		if(watch) printf ("address ok\n");
		break;

	case RP_NO: 
		if(watch) printf ("not deliverable; unknown problem\n");
		badaddrs = TRUE;
		break;

	case RP_USER: 
		if(watch) printf ("not deliverable; unknown address.\n");
		badaddrs = TRUE;
		break;

	case RP_NDEL: 
		if(watch) printf ("not deliverable; permanent error.\n");
		badaddrs = TRUE;
		break;

	case RP_AGN: 
		if(watch) printf ("failed, this attempt; try later\n");
		badaddrs = TRUE;
		break;

	case RP_NOOP: 
		if(watch) printf ("not attempted, this time; perhaps try later.\n");
		badaddrs = TRUE;
		break;

	default: 
		adios (NULLCP, "Unexpected address response:  %s", rp -> rp_line);
	}
	(void) fflush (stdout);
}

void doheader(oobto)
char **oobto;
{
	int	gotfrom, gotsender, gotdate, gotrecipient;
	char	line[LINESIZE];
	char	buf[LINESIZE];
	struct qbuf *qp;

	gotfrom = gotsender = gotdate = gotrecipient = 0;
	for (qp = Header -> qb_forw; qp != Header; qp = qp -> qb_forw) {
		if (qp -> qb_data[0] == '\n')
			break;
		if (prefix ("Date:", qp -> qb_data))
			gotdate++;
		if (prefix ("From:", qp -> qb_data)) {
			gotfrom++;
			if (rewritefrom) {
				dofrom();
				continue;
			}
		}
		if (prefix ("Sender:", qp -> qb_data))
			gotsender++;
		if (prefix ("To:", qp -> qb_data)
		    || prefix ("Cc:", qp -> qb_data)
		    || prefix ("Bcc:", qp -> qb_data)
		    || prefix ("Apparently-To:", qp -> qb_data) )
			gotrecipient++;

		if (rp_isbad (io_tdata (qp -> qb_data, qp -> qb_len)))
			adios (NULLCP, "Data copy error");
	}

	if (!gotrecipient && oobto) {
		while (*oobto) {
			(void) sprintf (buf, "Apparently-To: %s\n", *oobto++);
			if (rp_isbad (io_tdata (buf, strlen(buf))))
				adios (NULLCP, "Data copy error");
		}
	}
	if (!gotdate) {
		UTC	now, lut;
		(void) strcpy (buf, "Date: ");
		now = utcnow();
		lut = utclocalise(now);
		UTC2rfc (lut, buf + 6);
		free ((char *) lut);
		(void) strcat (buf, "\n");
		if (rp_isbad (io_tdata (buf, strlen(buf))))
			adios (NULLCP, "Data copy error");
	}
	if (!gotfrom)
		dofrom();
	if (!gotsender) {
		(void) sprintf(buf, "Sender: %s\n", from);
		if (rp_isbad (io_tdata (buf, strlen(buf))))
			adios (NULLCP, "data copy error");
	}

	(void) sprintf (tbuf, "1.%s", ia5_bp);
	if (rp_isbad (io_tdend (rp)) || rp_isbad (io_tpart (tbuf, 0, rp)))
		adios (NULLCP, "Error seting up for body part: %s",
		       rp -> rp_line);
	if (firstline[0] != '\n')
		io_tdata (firstline, strlen(firstline));

}

/*
 * Could this be an RFC822 header line?
 */
int isheader(line)
char *line;
{
	register char *cp = line;

	while (*cp == ' ' || *cp == '\t')
		cp++;
	return *cp == '\n' ? 0 : 1;
}

void dofrom()
{
	char	line[128];

	if (isstr(fullname))
		(void) sprintf(line, "From: %s <%s>\n", fullname, from);
	else
		(void) sprintf(line, "From: %s\n", from);
	io_tdata (line, strlen(line));
}

int dobody()
{
	char    buffer[BUFSIZ];
	register int    i;

	while (!feof (stdin) && !ferror (stdin) &&
	    (i = fread (buffer, sizeof (char), sizeof (buffer), stdin)) > 0)
		if (rp_isbad (i = io_tdata (buffer, i)))
			adios (NULLCP, "Problem writing body");

	if (ferror (stdin))
		adios (NULLCP, "Problem reading body");

	if (rp_isbad (io_tdend (rp)) || rp_isbad (io_tend (rp)))
		adios (NULLCP, "problem ending submission: %s", rp -> rp_line);

	return(0);	/* eventually the program exit value */
}

void smtp()
{
	char	*smtpd = dupfpath(chndfldir, SMTPSRVR);

	setuid(geteuid());
	execl (smtpd, "sendmail-smtp", "smtp", (char *)0);
	adios ("failed", "execl of %s", smtpd);
}

static void adios (char *what, char* fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
	_ll_log (pp_log_norm, LLOG_FATAL, what, fmt, ap);
    va_end (ap);
    _exit (1);
}

static SFD die(int sig)
{
	io_end(NOTOK);
	adios (NULLCP, "sendmail: dying from signal %d", sig);
}
