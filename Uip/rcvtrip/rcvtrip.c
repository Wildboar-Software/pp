/* rcvtrip.c: trip reporting program */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvtrip/RCS/rcvtrip.c,v 6.0 1991/12/18 20:42:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvtrip/RCS/rcvtrip.c,v 6.0 1991/12/18 20:42:02 jpo Rel $
 *
 * $Log: rcvtrip.c,v $
 * Revision 6.0  1991/12/18  20:42:02  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"retcode.h"
#include	"adr.h"
#include	"ap.h"
#include	"chan.h"
#include 	"alias.h"
#include	<pwd.h>

#define		NONFATAL	0	/* Nonfatal reports		 */
#define		FATAL		1	/* Fatal reports (exit)		 */
#define		TRACE		2	/* General info for debug	 */

#define		EX_SENT		1
#define		EX_DUP		2
#define		EX_NOT_DIR	3
#define		EX_PARSE_ERR	4
#define		EX_ERR		99

#define		SENT		0	/* Sent out a reply		 */
#define		UNSENT		1	/* No reply sent - just
					 * note	 */

#define		REPLIED		'+'	/* A reply was sent		 */
#define		NOT_REPLIED	'-'	/* A reply wasn't sent		 */

#define		HDR_NAM		1
#define 	HDR_EOH		2
#define		HDR_MOR		3
#define		HDR_NEW		4

#define		DAYS	* (60 * 60 * 24)

#ifndef	MAX_HEAD_LINES
#define	MAX_HEAD_LINES	10
#endif	/* MAX_HEAD_LINES */

extern int ap_outtype;

extern char *compress ();

int     debug = FALSE;		/* output trace messages?	   */
int	repeat_interval = 14 DAYS; /* repeat after this interval   */
int     found_1 = FALSE;	/* true if message was sent to	   */
int	head_lines = 0;
int	exit_alg = -1;		/* algorithm to deiced on the exit code */
int	exit_code = 1;

 /* directly to recipient	   */
char    subj_field[LINESIZE];	/* contents of subject field	   */
char    _username[LINESIZE];	/* space for the users username	   */
char    *username = (char *) 0;	/* the users username		   */
char    *msg_note = "tripnote";/* where to get reply message from */
char    *wholoc = "triplog";	/* where to note incoming messages */
char    *sigloc = ".signature";/* where to find user's name	   */
char    *tripsig = "MESSAGE AGENT";

/* signature of the returned message */

AP_ptr  from_adr = NULLAP;	/* address tree for from address   */
AP_ptr  sndr_adr = NULLAP;	/* - " -	    sender	   */
AP_ptr  repl_adr = NULLAP;	/* - " -	    reply-to	   */

ADDR	*user_adr;

char   *find_sig ();
static int exit_rc();

struct passwd *getpwuid ();

/*
 *	This message is output before the users file message
 */
char   *text_message[MAX_HEAD_LINES+1] = {
	"\tThis is an automatic reply.  Feel free to send additional",
	"mail, as only this one notice will be generated.  The following",
	"is a prerecorded message, sent for ", 0
};

/*
 *	This message is output if the users file is not readable or doesn't
 *	exist. (This is where you can have some fun!)
 */
char   *error_txt[] = {
	"\tIt appears that the person in question hasn't left a\n",
	"message file, so the purpose of this message is unknown.\n",
	"However it is assumed that as I, a daemon process,\n",
	"have been called into existance that the person in question\n",
	"is probably away for some time and won't be replying to your\n",
	"message straight away.\n\nCheers,\n\tThe Mail System\n", 0
};

main (argc, argv)
int     argc;
char  **argv;
{
	init (argc, argv);
	parsemsg ();
	doreplies ();
#ifdef  PP_DEBUG
	report (TRACE, 0, "exit with %d -> %d", exit_code, exit_rc(exit_code));
#endif
	exit (exit_rc(exit_code));
}

/*
 *	Initialisation - get user, setup flags etc.
 */

init (argc, argv)
int     argc;
char  **argv;
{
	register struct passwd 	*pwdptr;
	int			opt;
	extern int		optind;
	extern char		*optarg;
	RP_Buf			rp;

	uip_init (argv[0]);

	ap_outtype = AP_PARSE_822;	/* standardise on 822 */

	while ((opt = getopt (argc, argv, "de:Ff:l:n:s:t:u:")) != EOF)
		switch (opt) {
		case 'd': debug = 1;			break;
		case 'e': exit_alg = atoi(optarg);	break;
		case 'F': found_1 = 1;			break;
		case 'f': tripsig = optarg;		break;
		case 'l': wholoc = optarg;		break;
		case 'n': msg_note = optarg;		break;
		/* This is disapbled for the momet ... */
		case 'r': opt = atoi(optarg);
			  if (!opt && *optarg != '0')	break;
			  repeat_interval = opt DAYS;	break;
		case 's': sigloc = optarg;		break;
		case 't': if (head_lines < MAX_HEAD_LINES)
				text_message[head_lines++] = optarg; break;
		case 'u': username = optarg;		break;
		}
#ifdef  PP_DEBUG
	report (TRACE, 0, "argc=%d, optind=%d, argv='%s' ... '%s'",
			argc, optind, argv[0], argv[argc-1]);
#endif
	argv += optind;
	argc -= optind;

	if (argc > 0)		/* we have a command line sender
				 * address */
	{
#ifdef  PP_DEBUG
		report (TRACE, 0, "argc=%d, *argv=%x=`%s'",
			argc, *argv, *argv);
#endif
		if (*argv) parse_addr (*argv, &sndr_adr);
		else report (TRACE, 0, "*argv was null !");
#ifdef  PP_DEBUG
		report (TRACE, 0, "*argv parsed");
#endif
	}

	if (username)
		;
	else if ((pwdptr = getpwuid (getuid ())) == (struct passwd *) 0)
		report (FATAL, EX_ERR, "Can't Identify user");
	else
		strcpy (username = _username, pwdptr->pw_name);

	if (head_lines) error_txt[head_lines] = (char *) 0;

	user_adr = adr_new (username, AD_ANY_TYPE, 0);
#ifdef UKORDER
	if (rp_isbad (ad_parse (user_adr, &rp, CH_UK_PREF)))
#else
	if (rp_isbad (ad_parse (user_adr, &rp, CH_USA_PREF)))
#endif
		report (FATAL, EX_PARSE_ERR, "Can't parse you as a mail user");
}

/*
 *	parse message - just a front end to the real parser
 */

parsemsg ()
{
	if (rp_isbad (hdr_fil (stdin)))
		report (FATAL, EX_PARSE_ERR, "parse of message failed");
}

/*************** Routines to despatch messages ***************/

/*
 *	Send replies to reply-to address if specified else
 *		from and sender address.
 */

doreplies ()
{
	if (found_1 != TRUE)
		report (FATAL, EX_NOT_DIR, "Not sent directly to %s", username);

	if (repl_adr == NULLAP && from_adr == NULLAP && sndr_adr == NULLAP)
		report (FATAL, EX_PARSE_ERR, "No parsable from or sender lines");

/*
 *	If there were any reply-to lines, use those in preference.
 */
	if (repl_adr != NULLAP)
		replyto (repl_adr);
	else if (from_adr != NULLAP)
		replyto (from_adr);
	else if (sndr_adr != NULLAP)
		replyto (sndr_adr);
}

/*
 *	Actually do the work of sending the reply.
 */

replyto (aptr)
AP_ptr  aptr;
{
	register char *cp;
	register char **cpp;
	AP_ptr  user, local, domain;
	AP_ptr  norm;
	char   *addr;
	char   *ref;
	char   *person;
	char    buffer[LINESIZE];
	FILE   *fp;
	RP_Buf	rps, *rp = &rps;

#ifdef	PP_DEBUG
	report (TRACE, 0, "replyto()");
#endif

#ifdef UKORDER
	norm = ap_normalize (aptr, CH_UK_PREF);
#else
	norm = ap_normalize (aptr, CH_USA_PREF);
#endif
	ap_t2p (norm, (AP_ptr *) 0, &user, &local, &domain, (AP_ptr *) 0);
	ref = ap_p2s_nc (NULLAP, NULLAP, local, domain, NULLAP);

#ifdef	PP_DEBUG
	report (TRACE, 0, "normed addr = '%s'", ref);
#endif

	if (user != NULLAP)
		person = user->ap_obvalue;
	else
		person = NULLCP;

	if (checkuser (ref)) {

#ifdef	PP_DEBUG
		report (TRACE, 0, "Seen %s before", ref);
#endif

		noteuser (ref, UNSENT);
		free (ref);
		return;
	}
	if (ap_t2s (norm, &addr) == (AP_ptr) NOTOK) {
		report (NONFATAL, 0, "Parse error for %s", addr);
		noteuser (ref, UNSENT);
		free (ref);
		return;
	}
	buffer[0] = '\0';
	if (!prefix ("re:", subj_field))
		strcpy (buffer, "Re: ");

	strcat (buffer, subj_field);

#ifdef	PP_DEBUG
	report (TRACE, 0, "pps_1adr ('%s' %s)",
		buffer, addr);
#endif

	pps_1adr (buffer, addr, rp);

	if (person != NULLCP) {
		compress (person, person);
		sprintf (buffer, "\nDear %s,\n", person);

#ifdef	PP_DEBUG
		report (TRACE, 0, "1st line %s", buffer);
#endif

		pps_txt (buffer, rp);
	}

#ifdef	PP_DEBUG
	report (TRACE, 0, "start builtin message");
#endif

	for (cpp = text_message; *cpp != NULLCP; cpp++)
	{	if (cpp != text_message) pps_txt ("\n", rp);
		pps_txt (*cpp, rp);
	}
	if ((cp = find_sig ()) != NULLCP)
		pps_txt (cp, rp);
	else if (user_adr && user_adr -> ad_r822adr)
		pps_txt (user_adr -> ad_r822adr);
	else
		pps_txt (username, rp);
	pps_txt ("\n\n\n", rp);

	if ((fp = fopen (msg_note, "r")) != (FILE *) 0) {

#ifdef	PP_DEBUG
		report (TRACE, 0, "processing file %s", msg_note);
#endif

		pps_file (fp, rp);
		fclose (fp);
	}
	else {

#ifdef	PP_DEBUG
		report (TRACE, 0, "Sending built in error message");
#endif

		for (cpp = error_txt; *cpp != NULLCP; cpp++)
			pps_txt (*cpp, rp);
	}

#ifdef	PP_DEBUG
	report (TRACE, 0, "ml_end(OK)");
#endif

	if (pps_end (OK, rp) == OK)
		noteuser (ref, SENT);
	else
		noteuser (ref, UNSENT);
	free (ref);
	free (addr);
}

/*************** Routines to pick apart the message ***************/

/*
 *	The actual picking out of headers
 */

hdr_fil (msg)
FILE   *msg;			/* The message file			 */
{
	char    line[LINESIZE];	/* temp buffer			 */
	char    name[LINESIZE];	/* Name of header field		 */
	char    contents[LINESIZE];	/* Contents of header field	 */

#ifdef	PP_DEBUG
	report (TRACE, 0, "hdr_fil()");
#endif

	if (msg == (FILE *) NULL) {
		report (FATAL, EX_PARSE_ERR, "NULL file pointer");
		return (RP_NO);	/* not much point doing anything
				 * else */
	}
/* process the file    */

	for (;;) {
		if (fgets (line, sizeof line, msg) == NULL) {
			report (NONFATAL, 0, "read error on message");
			return (RP_NO);	/* skip and go home */
		}

		switch (hdr_parse (line, name, contents)) {
		    case HDR_NAM:	/* No name so contine
					 * through header */
			continue;

		    case HDR_EOH:	/* End of header - lets go
					 * home */
			return (RP_OK);

		    case HDR_NEW:
		    case HDR_MOR:	/* We have a valid field	    */
			if (lexequ (name, "to") == 0  ||
			    lexequ (name, "cc") == 0||
			    lexequ (name, "resent-to") == 0 ||
			    lexequ (name, "resent-cc") == 0) {
				find_user (contents);
				break;
			}
			else if (lexequ (name, "resent-from") == 0) {
				if (from_adr)
					ap_free (from_adr);
				from_adr = AP_NIL;
				parse_addr (contents, &from_adr);
				break;
			} else if (lexequ (name, "from") == 0) {
				parse_addr (contents, &from_adr);
				break;
			}
			else if (lexequ (name, "reply-to") == 0) {
				parse_addr (contents, &repl_adr);
				break;
			}
			else if (lexequ (name, "sender") == 0) {
				parse_addr (contents, &sndr_adr);
				break;
			}
			else if (lexequ (name, "subject") == 0) {
				strncpy (subj_field, contents,
					 sizeof contents - 1);
				break;
			}
			else
				continue;
		}
	}
}

/*
 *	The real parser
 */

hdr_parse (src, name, contents)	/* parse one header line		 */
register char *src;		/* a line of header text		 */
char   *name,			/* where to put field's name		 */
       *contents;		/* where to put field's contents	 */
{
	extern char *compress ();
	char    linetype;
	register char *dest;

#ifdef	PP_DEBUG
	report (TRACE, 0, "hdr_parse('%*.*s')", strlen(src)-1, strlen(src)-1, src);
#endif

	if (isspace (*src)) {	/* continuation text			 */

#ifdef	PP_DEBUG
		report (TRACE, 0, "hrd_parse -> cmpnt more");
#endif

		if (*src == '\n')
			return (HDR_EOH);
		linetype = HDR_MOR;
	}
	else {			/* copy the name			 */
		linetype = HDR_NEW;
		for (dest = name; *dest = *src++; dest++) {
			if (*dest == ':')
				break;	/* end of the name	 */
			if (*dest == '\n') {	/* oops, end of the
line		 */
				*dest = '\0';
				return (HDR_NAM);
			}
		}
		*dest = '\0';
		compress (name, name);	/* strip extra & trailing
					 * spaces	 */

#ifdef	PP_DEBUG
		report (TRACE, 0, "hdr_parse -> cmpnt name '%s'", name);
#endif
	}

	for (dest = contents; isspace (*src);)
		if (*src++ == '\n') {	/* skip leading white space
	 *//* u
					 * nfulfilled promise, no
					 * contents	 */
			*dest = '\0';
			return ((linetype == HDR_MOR) ? HDR_EOH : linetype);
		}		/* hack to fix up illegal spaces	 */

	while ((*dest = *src) != '\n' && *src != 0)
		src++, dest++;	/* copy contents and then, backup	 */
	while (isspace (*--dest));	/* to eliminate trailing
					 * spaces	 */
	*++dest = '\0';

	return (linetype);
}

/*************** Parsing of addresses got from message ***************/

/*
 *	See if user is in this field. parse address first and
 *		compare mbox's.
 */

find_user (str)
char   *str;
{
	AP_ptr  tree, local, domain;
	ADDR	*ad;
	RP_Buf	rp;
	char   *p;

#ifdef	PP_DEBUG
	report (TRACE, 0, "find_user('%s')", str);
#endif

	if (found_1 == TRUE) {

#ifdef	PP_DEBUG
		report (TRACE, 0, "find_user() name already found\n");
#endif

		return;
	}

	while ((str = ap_s2p (str, &tree, (AP_ptr *) 0, (AP_ptr *) 0, &local,
		     (AP_ptr *) &domain, (AP_ptr *) 0)) != (char *) DONE
	       && str != (char *) NOTOK) {
		p = ap_p2s_nc (NULLAP, NULLAP, local, domain, NULLAP);

		ad = adr_new (p, AD_822_TYPE, 0);
		ap_sqdelete (tree, NULLAP);
		ap_free (tree);

#ifdef UKORDER
		if (rp_isbad (ad_parse (ad, &rp, CH_UK_PREF))) {
#else
		if (rp_isbad (ad_parse (ad, &rp, CH_USA_PREF))) {
#endif
			adr_free (ad);
			continue;
		}
		found_1 = (lexequ (ad -> ad_r822adr, user_adr -> ad_r822adr) == 0 ||
			lexequ (p, username) == 0);
#ifdef	PP_DEBUG
		report (TRACE, 0, "find_user() -> found mbox %s (%s) ->%d",
			p, user_adr -> ad_r822adr, found_1);
#endif
		free (p);
		if (found_1 == TRUE)
			return;
	}
}

/*
 *	Attempt to parse a field into an address tree for later use.
 */

parse_addr (str, aptr)
char   *str;
AP_ptr *aptr;
{

#ifdef	PP_DEBUG
	report (TRACE, 0, "parse_addr('%s')", str);
#endif

	if (*aptr != NULLAP) {

#ifdef	PP_DEBUG
		report (TRACE, 0, "field '%s' rejected, already seen one");
#endif

		return;
	}

	if ((*aptr = ap_s2t (str)) == (AP_ptr) NOTOK)
	{	*aptr = NULLAP;
		report (NONFATAL, 0, "Can't parse address '%s'", str);
	}
}

/*************** User Data Base Routines ***************/

/*
 *	Note the fact we did/didnot reply to a given person
 *	and include the subject line for good measure.
 */

noteuser (addr, mode)
char   *addr;
int     mode;
{
	FILE   *fp;
	time_t  now;
	char   *ctime ();
	int     result = NOTOK;

#ifdef	PP_DEBUG
	report (TRACE, 0, "noteuser(%s,%d)", addr, mode);
#endif

	time (&now);

	if ((fp = fopen (wholoc, "a")) != NULL) {
		(void) chmod (wholoc, 0600);
		fprintf (fp, "%c %-30s %19.19s >> %.20s\n",
			 mode == SENT ? REPLIED : NOT_REPLIED,
			 addr, ctime (&now), subj_field);
		(void) fflush (fp);
		result = ferror (fp) ? NOTOK : OK;
		fclose (fp);
		if (result == OK && mode == SENT) exit_code = 0;
	}
	return result;
}

/*
 *	Have we replied to this person before?
 */
checkuser (addr)
char   *addr;
{
	FILE   *fp;
	char    buffer[LINESIZE];
	char    compaddr[LINESIZE];

	if (repeat_interval == 0)	return FALSE;

	if ((fp = fopen (wholoc, "r")) == NULL)
		return FALSE;
	while (fgets (buffer, sizeof buffer, fp) != NULL) {
		if (buffer[0] == NOT_REPLIED)
			continue;
		/* SHOULD parse the date to see whether to repeat */
		getaddress (buffer, compaddr);

#ifdef	PP_DEBUG
		report (TRACE, 0, "checkuser, = '%s' & '%s'?", compaddr, addr);
#endif

		if (lexequ (compaddr, addr) == 0) {
			fclose (fp);
			return TRUE;
		}
	}
	fclose (fp);
	return FALSE;
}


/*************** Some Utility Routines ***************/


/*
 *	Dig out a signature for the user
 */

char   *find_sig ()
{
	FILE   *fp;
	static char buf[LINESIZE];
	static int been_here = FALSE;
	char   *p;

#ifdef	PP_DEBUG
	report (TRACE, 0, "find_sig()");
#endif

	if (been_here == TRUE)	/* cuts off at least 1/4
				 * micro-second */
		return buf;

	if ((fp = fopen (sigloc, "r")) == NULL)
		return NULLCP;
	if (fgets (buf, sizeof (buf), fp) == NULLCP) {
		fclose (fp);
		return NULLCP;
	}
	if ((p = index (buf, '\n')) != NULLCP)
		*p = '\0';
	fclose (fp);
	return buf;
}

getaddress (source, dest)
char   *source, *dest;
{
	register char *p;
	int     depth = 0;

	p = source + 2;		/* skip over reply indicator */
	while (*p) {
		switch (*p) {
		    case '"':	/* in quoted part ? */
			depth = !depth;
			break;
		    case ' ':
		    case '\t':
			if (depth == 0) {
				*dest = '\0';
				return;
			}
			break;
		    case '\\':	/* gobble up next char */
			*dest++ = *p++;
			break;
		}
		*dest++ = *p++;
	}
	*dest = '\0';
}

/*VARARGS2*/
report (mode, rc, fmt, a1, a2, a3, a4)
int     mode;
int	rc;
char   *fmt, *a1, *a2, *a3, *a4;
{
	static FILE *log;

	if (debug == FALSE && mode == TRACE)
		return;

	if (debug == TRUE) {
		fprintf (stderr, "%s\t", mode == TRACE ? "TRACE" :
			 (mode == FATAL ? "FATAL" : "!FATAL"));
		fprintf (stderr, fmt, a1, a2, a3, a4);
		putc ('\n', stderr);
		fflush (stderr);
	}

	if (mode == FATAL)
		exit (exit_rc(rc));	/* die a horrible death */
}

static int exit_rc(num)
int num;
{
	switch (exit_alg)
	{
	case 0:	return RP_MECH;
	case 1:	return 0;
	case 2:	return num;
	default:return 0;
	}
}
