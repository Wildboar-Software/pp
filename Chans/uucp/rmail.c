/* rmail.c: */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/uucp/RCS/rmail.c,v 6.0 1991/12/18 20:13:06 jpo Rel $";
static char sccsid[] = "@(#)rmail.c	1.4 (UKNET Altered by pb/pc) 7/23/91";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/uucp/RCS/rmail.c,v 6.0 1991/12/18 20:13:06 jpo Rel $
 *
 * $Log: rmail.c,v $
 * Revision 6.0  1991/12/18  20:13:06  jpo
 * Release 6.0
 *
 */



/*
 *                              R M A I L . C
 *
 *      Developed from the Berkeley mail program of the same name
 *      by Mike Obrien at RAND to run with the MMDF mail system.
 *      Rewritten by Doug Kingston, US Army Ballistics Research Laboratory
 *      Hacked a lot by Steve Bellovin (smb@unc)
 *
 *      This program runs SETUID to Opr so that it can set effective and
 *      real [ug]ids to mmdflogin.
 *
 *      Steve Kille -  Aug 84
 *              Take machete to it.  Use protocol mode,
 *              and always go thru submit - phew
 *      Lee McLoughlin Oct 84.
 *              Address munges some header lines into 822 format.
 *	Peter Collinson Dec 85.
 *		Lee's version was dependent on
 *		 (a) the uucp channel
 *		 (b) the domain tables associated with the uucp channel
 *		This version costs more cycles (but there is generally
 *		only one of these running on a machine at a time) and
 *		uses mmdf's tables to dis-assemble bang routes converting
 *		them into a domain address.
 *	Peter Cowen Sept 89
 *		PP'ised
 *	Julian Onions July 90
 *		Choked to death on it!
 *	Piete Brooks June 90
 *		converted ! recipient addresses to % and @
 *	Peter Collinson July 1991
 *		put things back to where they were when the mmdf version
 *		was done. Make a number of comments true again.
 */
#undef  RUNALON

#include "util.h"
#include "head.h"
#include "q.h"
#include "prm.h"
#include "ap.h"
#include "adr.h"
#include "chan.h"
#include "table.h"
#include "retcode.h"
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdarg.h>

#define VERSION "3.0"
#define NAMESZ  256		/* Limit on component name size.
				 * LMCL was 64 */
#define ungetline(s)	((void) strcpy (tmpline,s),usetmp=1)
#define MAXADRS 6		/* Max no. of adrs to output on a
				 * single line. */

#define NEED_QUOTE	": "	/* chars in mailbox name that need `"'ing */

char    debug = 0;

extern char *loc_dom_site, *pplogin, *supportaddr, *hdr_822_bp, *ia5_bp, *cont_822;


extern char *index ();
extern char *rindex ();
extern char *getenv ();		/* get the Accounting system from
				 * the environment */
extern char *strdup ();
extern char *malloc ();
extern char *compress ();

static char * canon_name();

FILE   *rm_msgf;		/* temporary out for message text */
CHAN   *chanptr;
int     Tmpmode = 0600;
char    Msgtmp[LINESIZE];
char    rm_date[LINESIZE];	/* date of origination from uucp
				 * header */
char    rm_from[LINESIZE];	/* accumulated path of sender */
char    origsys[NAMESZ];	/* originating system */
char    origpath[LINESIZE];	/* path from us to originating
				 * system */
char    Mailsys[LINESIZE];	/* Mail system signature */
char    tmpline[LINESIZE];	/* Temporary buffer for
				 * continuations and such */
char    usetmp = 0;		/* Wether tmpline should be used */
char    linebuf[LINESIZE];	/* scratchpad */
char    nextdoor[LINESIZE];	/* The site nextdoor who sent the
				 * mail */
char    uchan[LINESIZE];	/* Channel to use for submission */

main (argc, argv)
char  **argv;
int     argc;
{
	char    fromwhom[NAMESZ];	/* user on remote system */
	char   *fromptr;
	char    sys[NAMESZ];	/* an element in the uucp path */
	char   *cp, *d;
	int     fd;

	sys_init (argv[0]);

	while (argc > 2 && strcmp (argv[1], "-d") == 0) {
		debug ++;
		argc--;
		argv++;
	}

	if (argc < 2) {
		fprintf (stderr, "Usage: rmail user [user ...]\n");
		exit (1);
	}
	(void) umask (0);

	pp_setuserid ();
	(void) sprintf (Mailsys, "<%s@%s>", pplogin, loc_dom_site);

	/* Create temp file for body of message */
	(void) strcpy (Msgtmp, "/tmp/rmail.XXXXXX");
	(void) mktemp (Msgtmp);
	if ((fd = creat (Msgtmp, Tmpmode)) < 0)
		bomb ("Can't create %s\n", Msgtmp);
	(void) close (fd);

	if ((rm_msgf = fopen (Msgtmp, "r+")) == NULL)
		bomb ("Can't reopen %s\n", Msgtmp);

	(void) unlink (Msgtmp);

	/*
	 * Unravell the From and >From lines at the head of the
	 * message to work who sent it, the path it took to get
	 * here and when it was sent. Some or all of this info may
	 * be ignore depending on the 822 header info given.
	 */
	/* Zero fromwhom to stop channel crashing later on if no
	 * From or >From
	 */
	(void) strcpy(fromwhom, "");
	for (;;) {
		if (fgets (linebuf, sizeof linebuf, stdin) == NULL)
			break;
		if (strncmp (linebuf, "From ", 5)
		    && strncmp (linebuf, ">From ", 6))
			break;

		cp = index (linebuf, ' ');	/* start of name */
		fromptr = ++cp;
		cp = index (cp, ' ');	/* cp at end of name */
		*cp++ = 0;	/* term. name, cp at date */
		(void) strcpy (fromwhom, fromptr);
		while (isspace (*cp))
			cp++;	/* Skip any ws */

		/*
		 * The date is the rest of the line ending at \0 or
		 * remote
		 */
		d = rm_date;
		while (*cp && strncmp (cp, " remote from ", 13))
			*d++ = *cp++;
		*d = '\0';

		for (;;) {
			cp = index (cp + 1, 'r');
			if (cp == NULL) {
				cp = rindex (fromwhom, '!');
				if (cp != NULL) {
					char   *p;

					*cp = '\0';
					p = rindex (fromwhom, '!');
					if (p != NULL)
						(void) strcpy (origsys, p + 1);
					else
						(void) strcpy (origsys, fromwhom);
					(void) strcat (rm_from, fromwhom);
					(void) strcat (rm_from, "!");
					(void) strcpy (fromwhom, cp + 1);
					goto out;
				}

				/*
				 * Nothing coherent found - so look
				 * in environment for ACCTSYS
				 */
				if ((cp = getenv ("ACCTSYS")) && *cp) {
					(void) strcpy (origsys, cp);
					(void) strcat (rm_from, cp);
					(void) strcat (rm_from, "!");
					goto out;
				}
				(void) strcpy(cp = sys, "remote from somewhere");
			}
			if (strncmp (cp, "remote from ", 12) == 0)
				break;
		}

		(void) sscanf (cp, "remote from %s", sys);
		(void) strcat (rm_from, sys);
		(void) strcpy (origsys, sys);	/* Save for quick ref. */
		(void) strcat (rm_from, "!");
out:		;
	}
	if (fromwhom[0] == '\0')/* No from line, illegal */
		bomb ("%s", "No 'from' lines in message\n");

	ungetline (linebuf);

	/* Complete the from field */
	(void) strcat (rm_from, fromwhom);

	/*
	 * A uk special - see if the first two components of the
	 * constructed bang address are in fact the same site.
	 * If so replace by their official name */
	domain_cross (rm_from);

	/*
	 * Save a copy of the path to the original site. This is
	 * all the the path we were given - the user name after the
	 * last !.  NOTE: We keep the trailing !, hence the +1.
	 */
	(void) strcpy (origpath, rm_from);
	*(rindex (origpath, '!') + 1) = '\0';

	/* Savepath is given a copy of the immediate neighbour */
	if ((d = index (rm_from, '!')) != NULL) {
		*d = '\0';
		(void) strcpy (nextdoor, rm_from);
		*d = '!';
	}
	else
		(void) strcpy (nextdoor, rm_from);

	/* find the channel depending in the nextdoor site */
	set_channel (nextdoor);

	/* Convert the from to Arpa format and leave it in from */
	fromcvt (rm_from, fromwhom);
	(void) strcpy (rm_from, fromwhom);

	if (debug)
		printf ("from=%s, origpath=%s, date=%s\n",
			rm_from, origpath, rm_date);

	msgfix (rm_from, rm_date);

	exit (xsubmit (rm_from, argv, argc));
	/* NOTREACHED */
}

/*
 * Munge the message header.
 * All header lines with addresses in are munged into 822 format.
 * If no "From:" line is given supply one based on the UUCP Froms, do the
 * same if no "Date:".
 */
typedef struct {
	char   *hname;
	int     hfound;
}       Hdrselect;

Hdrselect hdrselect[] = {
	"date", 0,		/* this is a little naughty - we
				 * need */
	/* to register that we have had this */
	/* but not address munge it */
	/* so it has token 0 */
	"from", 0,
	"to", 0,
	"cc", 0,
	"bcc", 0,
	"sender", 0,
	"reply-to", 0,
	"resent-from", 0,
	"resent-sender", 0,
	"resent-to", 0,
	0, 0

#define HDATE	0
#define HFROM	1
#define HTO	2
#define HCC	3
#define HBCC	4
#define HSENDER	5
#define HREPLY	6
#define HRFROM	7
#define HRSENDER	8
#define HRTO	9
};

/* Is this header in the list of those to have their addresses munged? */
/* return header value offset value */
/* notice that it returns 0 for the date header */
shouldmunge (name)
char   *name;
{
	register Hdrselect *h;

	if (debug > 1)
		printf ("in shouldmunge with %s\n", name);

	for (h = hdrselect; h->hname != NULL; h++)
		if (!lexequ (h->hname, name)) {
			h->hfound++;
			return (h - hdrselect);
		}
	return (0);
}

/* If this is a header line then grab the name of the header and stuff it
 * into name then return a pointer to the actually body of the line.
 * Otherwise return NULL.
 * NOTE: A header is a line that begins with a word formed of alphanums and
 * dashes (-) then possibly some whitespace then a colon (:).
 */
char   *
        grabheader (s, name)
register char *s, *name;
{
	char    *sanitise();
	char    tmpbuf[LINESIZE];

	/* Copy the name into name */
	while (isalpha (*s) || isdigit (*s) || *s == '-')
		*name++ = *s++;
	*name = '\0';

	/* Skip any whitespace */
	while (isspace (*s))
		s++;

	/* This is a header if the next char is colon */
	if (*s == ':') {
		s++;

		/*
		 * This is probably illegal but we fail horribly if
		 * it happens - so we will guard against it
		 */
		if (*s != ' ' && *s != '\t' && *s != '\0') {	/* we need to add a
								 * space */
			(void) strcpy (tmpbuf, s);
			(void) sprintf (s, " %s", tmpbuf);
		}
		return (sanitise(s));	/* Return a pointer to the rest of
				 * the line */
	}
	else
		return (NULL);
}

msgfix (from, date)
char   *from, *date;
{
	register char *s;
	char   *rest;
	char   *lkp;
	char    name[LINESIZE];
	char    tmpbuf[LINESIZE];
	int     haveheader = 0;	/* Do I have a header? */
	int     headertoken;
	char   *grabline ();

	/* Loop through all the headers */
	while (1) {
		if (debug > 1)
			printf ("in msgfix about to grabline\n");

		s = grabline ();

		if (debug > 2)
			printf ("got %s", s);

		/* Is this the end of the header? */
		if (*s == '\0' || *s == '\n' || feof (stdin))
			break;


		/* Is this a continuation line? */
		if (haveheader && isspace (*s)) {

			/*
			 * Note: Address munged headers handled
			 * specially
			 */
			fputs (s, rm_msgf);
		}
		else {

			/* Grab the header name from the line */
			if ((rest = grabheader (s, name)) == NULL)

				/*
				 * Not a header therefore all
				 * headers done
				 */
				break;

			haveheader = 1;

			/* Should I address munge this? */
			if (headertoken = shouldmunge (name)) {
				char   *finalstr;

				finalstr = "\n";
				fprintf (rm_msgf, "%s: ", name);

				/*
				 * deal specially with From lines
				 * the parser loses comments so we
				 * retain them specially here
				 */
				if (headertoken == HFROM
				    || headertoken == HRFROM) {
					if (lkp = index (rest, '<')) {
						*lkp = '\0';
						if (*rest == ' ')
							fprintf (rm_msgf, "%s<", rest + 1);
						else
							fprintf (rm_msgf, "%s<", rest);
						*lkp = '<';
						finalstr = ">\n";
						(void) strcpy (tmpbuf, lkp);

						/*
						 * notice that we
						 * copy from the <
						 * to start with
						 * a space
						 */
						tmpbuf[0] = ' ';
						rest = tmpbuf;
						lkp = rindex (rest, '>');
						if (lkp)
							*lkp = '\0';

					}
					if (lkp = index (rest, '(')) {
						*lkp = '\0';
						(void) sprintf (tmpbuf, " (%s", lkp + 1);
						finalstr = tmpbuf;
					}
				}
				hadr_munge (rest);
				fputs (finalstr, rm_msgf);
			}
			else
				fputs (s, rm_msgf);
		}
	}

	/* No From: line was given, create one based on the From's */
	if (hdrselect[HFROM].hfound == 0) {
		fprintf (rm_msgf, "From: %s\n", from);
	}

	/* No Date: line was given, create one based on the From's */
	if (hdrselect[HDATE].hfound == 0) {
		datecvt (date, tmpbuf);	/* Convert from uucp ->
					 * Arpa */
		fprintf (rm_msgf, "Date: %s\n", tmpbuf);
	}

	/* Copy the rest of the file, if there is any more */
	if (!feof (stdin)) {

		/*
		 * If the first line of the message isn't blank
		 * output the blank separator line.
		 */
		if (*s && *s != '\n')
			(void) putc ('\n', rm_msgf);
		do {
			(void) fputs (s, rm_msgf);
			s = grabline ();
		} while (!feof (stdin));
	}
}

char   *
        grabline ()
{
	if (debug > 2)
		printf ("in grabline ");

	/*
	 * Grab the next line. Remembering this might be the
	 * tmpline
	 */
	if (usetmp) {
		if (debug > 2)
			printf ("using tmpline ");
		(void) strcpy (linebuf, tmpline);
		usetmp = 0;
	}
	else
		fgets (linebuf, sizeof linebuf, stdin);

	/* Anything wrong? */
	if (ferror (stdin))
		fputs ("\n  *** Problem during receipt from UUCP ***\n", rm_msgf);

	if (debug > 2)
		printf ("returning %s\n ", linebuf);

	return (linebuf);

}

char   *next = NULL;		/* Where nextchar gets the next
				 * char from */
char    adrs[LINESIZE];		/* Partly munged addresses go here */

nextchar ()
{
	register char *s;	/* scratch pointer. */

	if (next != NULL && *next != '\0') {
		if (debug > 5)
			printf ("<%c>", *next);
		return (*next++);
	}

	/* The last buffer is now empty, fill 'er up */
	next = grabline ();

	if (feof (stdin) || !isspace (*next) || 
	    *next == '\n' || *next == '\0') {

		/*
		 * Yipee! We've reached the end of the address
		 * list.
		 */
		ungetline (next);
		next = NULL;	/* Force it to read buffer */
		return (-1);
	}

	/* Zap excess whitespace */
	(void) compress (next, adrs);

	/*
	 * This may be a slightly duff list generated by some of
	 * the UNIX mailers eg: "x y z" rather than "x,y,z" If it
	 * is in this ^^^^^^^ convert to  ^^^^^^^ NOTE: This won't
	 * handle list: "x<x@y> y<y@z>" conversions!
	 */
	if (index (adrs, ' ') != NULL &&
	    index (adrs, ',') == NULL &&
	    index (adrs, '<') == NULL &&
	    index (adrs, '(') == NULL &&
	    index (adrs, '"') == NULL) {
		for (s = adrs; *s; s++)
			if (*s == ' ')
				*s = ',';
	}
	next = adrs;

	/*
	 * This a continuation line so return in a seperator just
	 * in case
	 */
	return (',');
}


/* Munge and address list thats part of a header line.
 * Try and get the ap_* (MMDF) routines to do as much as possible for
 * us.
 */
hadr_munge (list)
char   *list;
{
	AP_ptr  the_addr;	/* ---> THE ADDRESS <--- */
	int     adrsout = 0;	/* How many address have been
				 * output */

	ungetline (list);

	while (1) {
		AP_ptr  ap_pinit ();

		the_addr = ap_pinit (nextchar);

		if (the_addr == (AP_ptr) NOTOK)
			bomb ("%s", "cannot initialise address parser!");

		ap_clear ();

		switch (ap_1adr ()) {
		    case NOTOK:
			/* Something went wrong!! */
			ap_sqdelete (the_addr, (AP_ptr) 0);
			ap_free (the_addr);

			/*
			 * Emergency action in case of parser break
			 * down: print out the rest of the line (I
			 * hope).
			 */
			fprintf (rm_msgf, "%s", tmpline);
			continue;
		    case OK:
			/* I've got an address to output! */

			/*
			 * output a comma seperator between the
			 * addresses and a newline every once in a
			 * while to make sure the lines aren't too
			 * long
			 */
			if (adrsout++)
				fprintf (rm_msgf, ",%s ", adrsout % MAXADRS ? "" : "\n");

			/* Munge will do all the work to output it */
			munge (the_addr);

			/* Reclaim the space */
			ap_sqdelete (the_addr, (AP_ptr) 0);
			ap_free (the_addr);

			if (debug > 4)
				printf ("munged and space freed\n");
			break;
		    case DONE:
			if (debug > 2)
				printf ("hadr_munge all done\n");

			/* Reclaim the space */
			ap_sqdelete (the_addr, (AP_ptr) 0);
			ap_free (the_addr);

			return;

		}
	}
}

/* We now have a single address in the_addr to output.
 */
munge (the_addr)
AP_ptr  the_addr;
{
	char    adr[LINESIZE];
	char   *s, *frees;
	AP_ptr  local, domain, route;

	if (debug > 1)
		printf ("in munge with: ");

	/* Find where the important bits begin in the tree */
	ap_t2p (the_addr, (AP_ptr *) 0, (AP_ptr *) 0,
		&local, &domain, &route);

	/* Convert from the tree back into a string */
	frees = s = ap_p2s_nc ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);

	if (debug > 1)
		printf ("%s\n", s);

	/* Is it a uucp style address? */
	if (domain == (AP_ptr) 0 && index (s, '!') != (char *) 0) {
		char    adr2[LINESIZE];

		(void) strcpy (adr, s);

		/*
		 * Stick the path it took to get here at the start.
		 * But try to avoid any duplicates by overlapping
		 * the matching parts of the address. Eg: adr =
		 * 'x!y!z'  path = 'a!b!x!y!' adr2 = 'a!b!x!y!z'
		 */
		for (s = origpath; *s; s++) {
			/* Does it match? */
			if (strncmp (adr, s, strlen (s)) == 0) {
				char    c = *s;

				*s = '\0';
				(void) strcpy (adr2, origpath);
				(void) strcat (adr2, adr);
				*s = c;
				break;
			}
		}

		/*
		 * Did I just scan the whole path without finding a
		 * match!
		 */
		if (*s == '\0') {
			/* Append the adr to the path */
			(void) strcpy (adr2, origpath);
			(void) strcat (adr2, adr);
		}

		/*
		 * Munge the address into its shortest form and
		 * print it
		 */
		fromcvt (adr2, adr);
		fprintf (rm_msgf, "%s", adr);

		if (debug > 1)
			printf ("uucp munge gives %s\n", adr);
	}
	else {
		char   *p;
		char   *at;
		char   *official = NULL;
		char   *fmt;
		char   *brace;
		AP_ptr  norm;
		extern int ap_outtype;

		/* Normalise the address */
		ap_outtype = AP_PARSE_733;	/* Hmm. Maybe should be
						 * from channel */
		norm = ap_normalize (the_addr, CH_USA_PREF);

		/* Convert it back in to a string and output it */
		ap_t2s (norm, &p);
		if (debug > 1)
			printf ("Normalised address: %s\n", p);

		/*
		 * Look for the last address component and re-write
		 * to the
		 */

		/*
		 * official form. There are probably better ways of
		 * doing this
		 */
		at = rindex (p, '@');
		fmt = "%s";
		if (at) {
			brace = index (at + 1, '>');
			if (brace)
				*brace = '\0';
			official = canon_name (at + 1);
			if (official) {
				*at = '\0';
				fmt = brace ? "%s@%s>" : "%s@%s";
			}
			else if (brace)
				*brace = '>';
		}
		fprintf (rm_msgf, fmt, p, official);

		if (debug > 1) {
			printf ("arpa munge gives ");
			printf (fmt, p, official);
			printf ("\n");
		}
		if (official) free (official);
		free (p);
	}
	free (frees);
}



/*
 *      datecvt()  --  convert UNIX ctime() style date to ARPA style
 *                      (change done in place)
 */
datecvt (date, newdate)
char   *date;
char   *newdate;
{

	/*
	 * LMCL: Changed the default timezone, when none given.
	 */

	if (isdigit (date[0]) || date[3] == ',' || isdigit (date[5]))
		(void) strcpy (newdate, date); /* Probably already ARPA */
	else if (isdigit (date[20]))
		(void) sprintf (newdate,
				"%.3s, %.2s %.3s %.2s %.2s:%.2s:%.2s GMT",
				date, date + 8, date + 4, date + 22,
				date + 11, date + 14, date + 17);
	else if (isalpha (date[17])) /* LMCL. Bum Unix format */
		(void) sprintf (newdate,
				"%.3s, %.2s %.3s %.2s %.2s:%.2s:00 GMT",
				date, date + 8, date + 4, date + 23,
				date + 11, date + 14);
	else
		(void) sprintf (newdate,
				"%.3s, %.2s %.3s %.2s %.2s:%.2s:%.2s %.3s",
				date, date + 8, date + 4, date + 26,
				date + 11, date + 14, date + 17, date + 20);
}


msg_failure (addr, rp, reason)
char   *addr;
RP_Buf *rp;
char   *reason;
{
	extern char	*getpostmaster();
	char    buf[BUFSIZ];

	if (rp)
		(void) sprintf (buf, "%s: %s [%s]",
				reason, rp->rp_line,
				rp_valstr ((int)rp->rp_val));
	else
		(void) sprintf (buf, "%s", reason);

	PP_LOG (LLOG_EXCEPTIONS, ("Message failure: %s", buf));

	return msg_return (getpostmaster(AD_822_TYPE), addr, buf);
}


msg_return (addr, dest, reason)
char   *addr, *dest, *reason;
{
	RP_Buf  rp;

	io_end (NOTOK);

	if (rp_isbad (pps_1adr ("UUCP (rmail) Failed Message", addr, &rp)))
		bomb ("pps_1adr failed [%s]", rp.rp_line);
	if (rp_isbad (pps_txt ("Your message was not delivered to:\n    ", &rp)))
		bomb ("pps_txt failed [%s]", rp.rp_line);
	if (rp_isbad (pps_txt (dest, &rp)))
		bomb ("pps_txt failed [%s]", rp.rp_line);
	if (rp_isbad (pps_txt ("\nFor the following reason:\n    ", &rp)))
		bomb ("pps_txt failed [%s]", rp.rp_line);
	if (rp_isbad (pps_txt (reason, &rp)))
		bomb ("pps_txt failed [%s]", rp.rp_line);
	if (rp_isbad (pps_txt ("\n\n--------------- Returned Mail ---------------\n\n", &rp)))
		bomb ("pps_txt failed [%s]", rp.rp_line);
	rewind (rm_msgf);
	if (rp_isbad (pps_file (rm_msgf, &rp)))
		bomb ("pps_file failed [%s]", rp.rp_line);

	if (rp_isbad (pps_txt ("--------------- End of Returned Mail -------------\n", &rp)))
		bomb ("pps_txt failed [%s]", rp.rp_line);
	if (rp_isbad (pps_end (OK, &rp)))
		bomb ("pps_end failed [%s]", rp.rp_line);
	return 0;
}

/*  */

static char *
pling2norm(to, from)
char *to;
char *from;
{
	char temp[1024];
	char *pling;
	char *c;
	int quote_it;

	(void) strcpy(temp, from);
	*to = '\0';

	/* copy over all (?) the hosts on the path */
	while (pling = rindex(temp, '!'))
	{
		quote_it = 0;
		*(pling++) = '\0';
		if (*to)
			(void) strcat(to, "%");
		else if (*pling != '"')
			for ( c = NEED_QUOTE; *c; c++)
				if (index(pling, *c)) {
					quote_it = 1;
					break;
				}
		if (quote_it)
			(void) strcat (to, "\"");
		(void) strcat(to, pling);
		if (quote_it)
			(void) strcat (to, "\"");
	}

	quote_it = 0;
	if (*to)
		(void) strcat(to, "%");
	else if (*temp != '"')
			for ( c = NEED_QUOTE; *c; c++)
				if (index(temp, *c)) {
					quote_it = 1;
					break;
				}
	if (quote_it)
		(void) strcat (to, "\"");
	(void) strcat(to, temp);
	if (quote_it)
		(void) strcat (to, "\"");
	if (pling = rindex(to, '%')) *pling = '@';
	if (debug > 1) printf("%s -> %s\n", from, to);
	return to;
}
/*  */

xsubmit (from, argv, argc)
char   *from;
char  **argv;
int     argc;
{
	char    buf[LINESIZE];
	RP_Buf  rp;
	struct prm_vars prm;
	LIST_BPT *new;
	ADDR   *orig = NULL;
	Q_struct que;
	char   *name = NULL;
	int     i;
	int     first_bp = TRUE;

	PP_NOTICE((">>> UUCP incoming message from %s", from));
	prm_init (&prm);
	q_init (&que);
	/* inbound channel, inbound host, contenttype */

	prm.prm_opts = PRM_ACCEPTALL;
	que.msgtype = MT_UMPDU;
	que.encodedinfo.eit_types = NULLIST_BPT;
	new = list_bpt_new (hdr_822_bp);
	list_bpt_add (&que.encodedinfo.eit_types, new);
	new = list_bpt_new (ia5_bp);
	list_bpt_add (&que.encodedinfo.eit_types, new);

	que.inbound = list_rchan_new (origsys, chanptr->ch_name);

	rewind (rm_msgf);

	if (debug > 0) {
		char buff[1024];

		printf("From %s, ", from);
		for (argv++; --argc > 0; argv++)
			printf ("arg = %s (%s)\n",
				pling2norm(buff, *argv), *argv);
		while (fgets (buf, LINESIZE - 1, rm_msgf) != NULL)
			printf ("T=%s", buf);
		if (ferror (rm_msgf))
			bomb ("%s", "Error reading messge file");

		exit (0);
	}

	if (rp_isbad (io_init (&rp)))
		return msg_failure (from, &rp, "io_init failed");

	if (rp_isbad (io_wprm (&prm, &rp)))
		return msg_failure (from, &rp, "io_wprm failed");
	if (rp_isbad (io_wrq (&que, &rp)))
		return msg_failure (from, &rp, "io_wrq failed");

	orig = adr_new (from, AD_822_TYPE, 0);
	if (rp_isbad (io_wadr (orig, AD_ORIGINATOR, &rp)))
		return msg_failure (from, &rp,
				    "Sender address unacceptable");

	adr_free (orig);

	for (i = 1; i < argc; i++) {
		ADDR   *adr;
		char buff[1024];

		name = pling2norm(buff, argv[i]);
		adr = adr_new (name, AD_822_TYPE, i);
		if (rp_isbad (io_wadr (adr, AD_RECIPIENT, &rp)))
			return msg_failure (name, &rp,
					 "Recipient address bad");
		PP_NOTICE(("valid recipient %s", argv[i]));
		adr_free (adr);
	}

	if (rp_isbad (io_adend (&rp)))
		return msg_failure (name, &rp, "Address end failed");

	if (rp_isbad (io_tinit (&rp)))
		return msg_failure (name, &rp, "body initialisation failed");
	if (rp_isbad (io_tpart (hdr_822_bp, FALSE, &rp)))
		return msg_failure (name, &rp,
				    "822 header initialisation failed");

	rewind (rm_msgf);
	first_bp = TRUE;
	while (fgets (buf, LINESIZE - 1, rm_msgf) != NULL) {
		if (first_bp == TRUE
		    && buf[0] == '\n') {
			first_bp = FALSE;
			if (rp_isbad (io_tdend (&rp)))
				return msg_failure (name, &rp,
						    "header end failed");
			if (rp_isbad (io_tpart ("1.ia5", FALSE, &rp)))
				return msg_failure (name, &rp,
						    "body initialisation failed");
		}

		if (rp_isbad (io_tdata (buf, strlen (buf))))
			return msg_failure (name, (RP_Buf *)0,
					    "data copied failed");
	}
	if (ferror (rm_msgf))
		return msg_failure (name, (RP_Buf *)0, "Data copy failed");

	if (rp_isbad (io_tdend (&rp)))
		return msg_failure (name, &rp, "Text termination problem");

	if (rp_isbad (io_tend (&rp)))
		return msg_failure (name, &rp, "Termination problem");
	(void) io_end (OK);
	PP_NOTICE(("Message successfully submitted"));
	return 0;
}


/*VARARGS1*/
#ifdef lint
bomb (fmt)
char   *fmt;
{
	bomb (fmt);
}

#else
bomb (va_alist)
va_dcl
{
	va_list ap;
	char    buf[BUFSIZ];

	va_start (ap);

	_asprintf (buf, NULLCP, ap);
	PP_LOG (LLOG_EXCEPTIONS, ("uucp bombing: %s", buf));
	va_end (ap);

	exit (99);
}
#endif

/*
 *      fromcvt()  --  trys to convert the UUCP style path into
 *                      an ARPA style name.
 */
fromcvt (from, newfrom)
char   *from, *newfrom;
{
	register char *cp;
	register char *sp;
	register char *off;
	char   *at;
	char   *atoff;
	char    buf[LINESIZE];

	if (debug > 1)
		printf ("fromcvt on %s\n", from);

	(void) strcpy (buf, from);
	cp = rindex (buf, '!');
	if (cp == 0) {
		(void) strcpy (newfrom, from);
		return;
	}

	/*
	 * look for @site at the end of the name
	 */
	atoff = NULL;
	if ((at = index (cp, '@')) != NULL) {
		/* got one - is it followed by a ! ? */
		if (index (at + 1, '!') != NULL)
			at = NULL;
		else {
			/* look up the official name of the at site */
			atoff = canon_name (at + 1);
		}
	}
	*cp = 0;
	while (sp = rindex (buf, '!')) {

		/*
		 * scan the path backwards looking for hosts that
		 * we know about
		 */
		if (off = canon_name (sp + 1)) {
			if (atoff && !lexequ (atoff, off))
				(void) strcpy (newfrom, cp + 1);
			else
				(void) sprintf (newfrom, "%s@%s", cp + 1, off);
			if (atoff) free (atoff);
			return;
		}
		else if (at && !lexequ (at + 1, sp + 1)) {
			(void) strcpy (newfrom, cp + 1);
			if (atoff) free(atoff);
			return;
		}
		*cp = '!';
		cp = sp;
		*cp = 0;
	}
	if (atoff) free(atoff);
	if (off = canon_name (buf)) {
		(void) sprintf (newfrom, "%s@%s", cp + 1, off);
		free (off);
	}
	else
		(void) sprintf (newfrom, "%s@%s", cp + 1, buf);
}

/*
 *	This piece added by Peter Collinson (UKC)
 *	rather than just making rmail submit into the uucp channel which
 *	is hard coded. First look up in a table 'rmail.chans' for entries
 *	of the form
 *	nextdoor:channel
 *	If the name exists then use that channel otherwise just default to
 *	the uucp channel. This is for authorisation purposes really
 *
 *	It seems to me to be much less confusing if we use the canonical
 *	name of the site in the channel lookup. This means that aliasing
 *	at the uucp level should map onto the correct site without
 *	having to install redundant names into the rmail table
 *	
 *	If we do this, then there's a good chance that canon_name will
 *	be called twice with the same key. So add a modicum of caching into
 *	canon_name()
 */
extern char *uucpin_chan;

set_channel (nxt)
char   *nxt;
{
	char	*canon;
	char	*uucpc;

	uucpc = uucpin_chan;
	
	canon = canon_name(nxt);
	if (canon == NULL)
		canon = nxt;
	if ((chanptr = ch_mta2struct (uucpc, canon)) == NULLCHAN)
		bomb ("Cannot look up channel '%s'\n", uucpc);
}

/*
 *	Check if the from machine which we have constructed actually
 *	contains two references to the same machine of the form
 *	site.uucp!site.??.uk!something
 *	if so replace by the single official name
 */
domain_cross (route)
char   *route;
{
	char   *second;
	char   *endsecond;
	char   *official;
	char   *host_equal ();

	second = index (route, '!');
	if (second == (char *) 0)
		return;
	second++;
	endsecond = index (second, '!');
	if (endsecond == (char *) 0)
		return;
	second[-1] = '\0';
	*endsecond = '\0';
	if (official = host_equal (route, second)) {
		char buf[LINESIZE];

		/* Construct things in a seperate buffer to avoid problems
		 * caused by the overlap of route and endsecond.
		 * i.e. if len(official) > len(route!second)
		 */
		(void) strcpy (buf, official);
		free (official);
		*endsecond = '!';
		(void) strcat (buf, endsecond);
		(void) strcpy(route, buf);
	}
	else {
		second[-1] = '!';
		*endsecond = '!';
	}
}

/*
 *	host_equal
 *	a routine to see whether the strings which are input
 *	are in fact the same host
 *	Returns the official name if they are and a null pointer if not
 */
char   *
host_equal(n1, n2)
	char   *n1, *n2;
{
	char	*o1, *o2;
	char	*canon_name();

	if ((o1 = canon_name(n1)) == NULLCP)
		return NULLCP;
	if ((o2 = canon_name(n2)) == NULLCP) {
		free (o1);
		return(NULLCP);
	}
	if (lexequ (o1, o2) == 0) {
		free (o2);
		return o1;
	}
	free (o1);
	free (o2);
	return NULLCP;
}

/* This procedure takes a string and converts all control characters
 * to spaces. This is necessary to protect against the depradations
 * of various out of spec mailers (HP-UX for example) which allow
 * spurious control characters in headers.
 */
char    *
sanitise(s)
	char    *s;
{
	char	*t;

	/* Scan along string and convert all control characters except
	 * white space into space.
	 */
	for (t = s; *t; t++)
		*t = (iscntrl(*t) && !isspace(*t)) ? ' ' : *t;
	return (s);
}

/* UUCP MUST work on USA order but we will be charitable */
static int	dmnorder = CH_USA_PREF;

static char *
canon_name(host)
	char	*host;
{
	static char official[LINESIZE];
	static char lasthost[LINESIZE];
	char	*ptr;

	if (lasthost[0])
	{	if (lexequ(host, lasthost) == 0)
		{	if (debug > 0)
				printf("canon_name cache hit: %s -> %s\n",
					host, official);
			return strdup(official);
		}
	}
	if (rp_isbad(tb_getdomain(host, (char *)0, official, dmnorder, &ptr))) {
		official[0] = lasthost[0] = '\0';
		/* must clear the cache, as official is now "(ERROR)" */
		return (char *) 0;
	}
	(void) strcpy(lasthost, host);
	return strdup(official);
}
