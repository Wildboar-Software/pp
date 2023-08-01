/* smtpsrvr.c: server for smtp requests */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/smtp/RCS/smtpsrvr.c,v 6.0 1991/12/18 20:12:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/smtp/RCS/smtpsrvr.c,v 6.0 1991/12/18 20:12:19 jpo Rel $
 *
 * $Log: smtpsrvr.c,v $
 * Revision 6.0  1991/12/18  20:12:19  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "prm.h"
#include "q.h"
#include "dl.h"
#include <isode/cmd_srch.h>
#include <signal.h>
#include <sys/stat.h>
#include <isode/internet.h>
#include <errno.h>



/* -- external variables -- */

extern char *hdr_822_bp, *ia5_bp;
extern char		*ctime(),
			*quedfldir,
			*postmaster,
			*pptailor,
			*getpostmaster();
extern time_t		time();
extern	ADDR		*adr_new ();

#if sparc && defined(__GNUC__)	/* work around bug in gcc 1.37 sparc version */
#define inet_ntoa myinet_ntoa

static char *myinet_ntoa (in)
struct in_addr in;
{
	static char buf[80];

	(void) sprintf (buf, "%d.%d.%d.%d",
			(in.s_addr >> 24) & 0xff,
			(in.s_addr >> 16) & 0xff,
			(in.s_addr >> 8 ) & 0xff,
			(in.s_addr	) & 0xff);
	return buf;
}
#else
extern char	*inet_ntoa ();
#endif



/* -- internal definitions -- */

#define BUFL		8096	/* -- length of buf -- */
#define CNULL		'\0'	/* -- null -- */
#define CR		'\r'	/* -- carriage return -- */
#define LF		'\n'	/* -- line feed -- */
#define NTIMEOUT	300	/* -- 5 minute timeout on net I/O -- */



/* -- internal variables -- */

static	char
	buf [BUFL],		/* -- for general usage -- */
	*getline(),
	netbuf [BUFL],		/* -- contains the valid characters -- */
	*adrfix(),
	*arg,			/* -- 0 if no arg - pts to comm param -- */
	*channel = "smtp",
	*netptr = netbuf,	/* -- next char to come out of netbuf -- */
	*progname,
	*sender = NULLCP,
	*them,
	*realthem,
	*us;

static	int
	hellod = 0,
	nstimeout = 30,
	dont_mung = 0,		/* -- used by getline() to limit munging -- */
	net_count = 0,		/* -- no of valid characters in netbuf -- */
	no_recip = 0;		/* -- no of valid recipients accepted -- */

CHAN	*chanptr,
	*ch_nm2struct();

extern CMD_TABLE	qtbl_con_type[];

static void byebye (), pprestart (), ppbegin (), ppstart (),
	netreply (), rset ();

static void initialise (), dispatch ();
static char *official ();



/* --------------------------------------------------------------
 .								.
 .	C o m m a n d	D i s p a t c h	  T a b l e		.
 .								.
 * ------------------------------------------------------------ */

static void data (), helo (), help (), mail (), confirm (),
	quit (), rcpt (), vrfy (), expn ();

static struct comarr   /* -- the command table -- */
{
	char		*cmdname;		/* -- ascii name -- */
	void		(*cmdfunc)();		/* -- cmd func to call -- */
} commands [] = {
	"data",		data,
	"helo",		helo,
	"help",		help,
	"mail",		mail,
	"noop",		confirm,
	"quit",		quit,
	"rcpt",		rcpt,
	"rset",		rset,
	"vrfy",		vrfy,
	"expn",		expn,
	NULLCP,		NULL
};

static char usage[] = "Usage: smtp [-t timeout] [-T tailor] channel";

main (argc, argv)
int	argc;
char	**argv;
{
	char	replybuf [BUFSIZ];
	char	*cp;
        int	isnumeric;
	int	opt;
	extern char *optarg;
	extern int optind;

	progname = argv [0];

	(void) signal (SIGQUIT,SIG_IGN);
	(void) signal (SIGTERM,quit);

	while ((opt = getopt (argc, argv, "t:T:")) != EOF) {
		switch (opt) {
		    case 't':
			nstimeout = atoi (optarg);
			break;

		    case 'T':
			pptailor = optarg;
			break;

		    default:
			sys_init (progname);
			fprintf (stderr, "bad switch -%c: %s\n", opt, usage);
			PP_OPER (NULLCP, ("Bad argument -%c: %s", opt, usage));
			exit (1);
		}
	}
	
	argc -= optind;
	argv += optind;

#ifdef  NAMESERVER
        ns_settimeo (nstimeout); /* don't wait forever! */
#endif

	if (argc != 1) {
		sys_init (progname);
		PP_OPER (NULLCP, ("smtpsrvr - no channel given!"));
		exit (NOTOK);
	}
	channel = *argv;
	sys_init (channel);
	(void) chdir (quedfldir);


	initialise ();		/* fill in us & them */
	/* SEK - force validation of IP address */

        for (isnumeric = TRUE, cp=them; *cp++ != '\0';) 
		if (!isdigit(*cp) && *cp != '.' && *cp != '\0') {
			isnumeric = FALSE;
			break;
		}

	/* -- find out who you are I might even believe you. -- */

	if ((chanptr = ch_mta2struct (channel, them)) == NULLCHAN) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("unknown channel: %s", channel));
		(void) sprintf (replybuf,
				"421 %s: channel '%s' is unknown to PP. (Complain to:  %s)\r\n",
				us, channel, postmaster);
		netreply (replybuf);
		exit (NOTOK);
	}

	if (isnumeric && 
	     lexequ (chanptr -> ch_in_info, "sloppy") != 0) {
		PP_OPER (NULLCP, ("can't resolve '%s'", them));
		(void) sprintf (replybuf,
			"421 %s: PP cannot resolve your address. '%s' (Help from:  %s)\r\n",
			us, them, postmaster);
		netreply (replybuf);
		exit (NOTOK);
	}
	  
	
	channel = chanptr -> ch_name;
	rename_log (channel);

	PP_NOTICE (("Connection from %s channel %s", realthem, channel));

	ppbegin ();

	/* -- say we're listening -- */

	(void) sprintf (replybuf,
			"220 %s PP Here - Pleased to meet you (Complaints/bugs to:  %s)\r\n",
			us, postmaster);
	netreply (replybuf);

	while (cp = getline()) {
		PP_LOG (LLOG_PDUS, ("<- %s", buf));

		dispatch (buf);
	}
	byebye (net_count < 0 ? 1 : 0);
}


/* ---------------------  Static  Routines  ------------------------------- */

static void dispatch (str)
char	*str;
{
	register struct comarr	*comp;

	for (comp = commands;comp->cmdname != NULL; comp ++) {
		if (strcmp (str, comp->cmdname) == 0) {
			(*comp->cmdfunc)(); /* call comm proc */
			return;
		}
	}
	PP_LOG (LLOG_EXCEPTIONS, ("Unknown command: %s", buf));
	netreply ("500 Unknown or unimplemented command\r\n");
}

static void initialise ()
{
#ifdef NAMESERVER
	int i;
#endif
	char	workarea[BUFSIZ];
	struct sockaddr_in	rmtaddr;
	int len;
	struct hostent *hp;

	if (gethostname (workarea,  sizeof workarea) == -1)
		PP_SLOG (LLOG_EXCEPTIONS, "gethostname",
			 ("Can't find out who I am"));

	us = official (workarea);
        if(us == NULLCP) {
		PP_OPER (NULLCP, 
			 ("Cannot find 'official' name of host '%s'\n",
			  workarea));
                exit(-1);
        }

	len = sizeof rmtaddr;
	if (getpeername (0, (struct sockaddr *)&rmtaddr, &len) != 0) {
		PP_SLOG (LLOG_EXCEPTIONS, "getpeername", ("Can't figure out who called us"));
		exit (-1);
	}
#ifdef NAMESERVER
	for (i = 0 ; i < 4 ; i++) {
		hp = gethostbyaddr((char *)&rmtaddr.sin_addr,
				   sizeof(rmtaddr.sin_addr), AF_INET);
		if(hp != NULL)
			break;
	}
#else
	hp = gethostbyaddr((char *)&rmtaddr.sin_addr,
			   sizeof(rmtaddr.sin_addr), AF_INET);
#endif
	if (hp == NULL) {
		(void) strcpy(workarea, inet_ntoa(rmtaddr.sin_addr));
		them = strdup (workarea);

		PP_LOG (LLOG_EXCEPTIONS,
			("lookup failed for address '%s'", workarea));
	} else
		them = strdup (hp->h_name);
	realthem = them;
	(void) signal (SIGPIPE, SIG_IGN);
}	

/*
name:		getline()

function:
		- get commands from the standard input terminated by <cr><lf>.
		- afix a pointer (arg) to any arguments passed.
		- ignore carriage returns.
		- map UPPER case to lower case.
		- manage the netptr and net_count variables.

algorithm:
		while we havent received a line feed and buffer not full

			if net_count is zero or less
				get more data from net
					error: return 0
			check for delimiter character
				null terminate first string
				set arg pointer to next character
			check for carriage return
				ignore it
			if looking at command name
				convert upper case to lower case

		if command line (not mail)
			null terminate last token
		manage netptr

returns:
		0 for EOF
		-1 when an error occurs on network connection
		ptr to last character (null) in command line


variables:
		dont_mung
		net_count
		netptr
		buf


--------------------------------------------------------------------------- */

static char *getline()
{
	register char	*inp;		/* -- input pointer in netbuf -- */
	static char *outp;   /* -- output pointer in buf -- */
	register int	c;		/* -- temporary char -- */


	inp	= netptr;
	outp	= buf;
	arg	= NULLCP;

	do {
		if (--net_count <= 0) {
			if (timeout (NTIMEOUT)) {
				PP_SLOG (LLOG_EXCEPTIONS, "read",
					 ("%s net input", realthem));
				return (NULLCP);
			}

			net_count = read (0, netbuf, sizeof netbuf);
			timeout (0);

			if (net_count == 0)	 /* -- EOF -- */
				return NULLCP;
			if (net_count < 0) {	 /* -- error -- */
				PP_SLOG (LLOG_EXCEPTIONS, "read",
					 ("%s net input", realthem));
				return NULLCP;
			}
			inp = netbuf;
		}

		c = *inp++ & 0377;

		if (c == '\r')		/* -- ignore CR -- */
			continue;	/* -- try to sneak through -- */
#ifdef TELNET_SMTP
		if (c >= 0200)		/* skip telnet codes */
			continue;
#endif


		if (dont_mung == 0 && arg == NULL) {
			/* -- if char is a delim afix token -- */

			if (c == ' ' || c == ',') {
				c = CNULL;	/* -- make null term'ed -- */
				arg = outp + 1; /* -- set arg ptr -- */
			}
			else if (isupper (c))
				/* -- do case mapping (UPPER -> lower) -- */
				c = tolower(c);
		}

		*outp++ = c;

	}  while (c != '\n' && outp < &buf [BUFL]);


	if (dont_mung == 0)
		*--outp = 0;	/* -- null term the last token -- */

	/* -- scan off blanks in argument -- */
	if (arg) {
		while (*arg == ' ')
			arg++;
		if (*arg == '\0')
			arg = 0;	/* -- if all blanks, no argument -- */
	}

	if (dont_mung == 0)
		PP_DBG (("'%s', '%s'",
			buf, arg == 0 ? "<noarg>" : arg));

	/* -- reset netptr for next trip in -- */
	netptr = inp;

	/* -- return success -- */
	return (outp);
}



/*
The helo command
*/

static void helo()
{
	char	replybuf [BUFSIZ];
	char	*cp;

	if (arg == NULLCP || *arg == 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("No argument to HELO"));
		netreply ("501 No argument to HELO\r\n");
		return;
	}
	(void) compress (arg, arg);
	for (cp = arg; *cp; cp++) {
		switch (*cp) {
		    case '\\':
			cp++;
			break;
		    case '[':
			while (*cp && *cp != ']')
				cp ++;
			break;

		    default:
			if (isascii(*cp) && (!iscntrl(*cp) && !isspace (*cp)))
				break;
			/* fall */
		    case '(':
		    case ')':
		    case '<':
		    case '>':
		    case '@':
		    case ',':
		    case ';':
		    case ':':
		    case '"':
			PP_LOG (LLOG_EXCEPTIONS,
				("Bad character '%c' in domain name '%s'",
				 *cp, arg));
			netreply ("501 Bad hostname in HELO.\r\n");
			return;
		}
	}

	hellod ++;
	if (lexequ (arg, them) != 0) {
		(void) sprintf (replybuf,
				"250 %s: You are bluffing - `%s` expected\r\n", 
					us, them);
		PP_NOTICE (("%s claims to be %s", realthem, arg));
	}
	else
		(void) sprintf (replybuf, "250 %s: Looks good to me\r\n", us);

	netreply (replybuf);
}



/*
handle the MAIL command	 ("MAIL from:<user@host>")
*/

static void mail()
{
	static Q_struct qstruct;
	Q_struct	*qp = &qstruct;
	ADDR		*ap;
	char		replybuf [BUFSIZ];
	RP_Buf		thereply;
	LIST_BPT	*new;	

	if (hellod == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Expecting a HELO command"));
		hellod ++; /* MH < 6.7 breaks this! */
	}

	if (arg == 0 || *arg == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("No argument to MAIL supplied"));
		netreply ("501 No argument supplied\r\n");
		return;
	}
	else if (sender) {
		PP_LOG (LLOG_EXCEPTIONS,
			("MAIL command already supplied"));
		netreply ("503 MAIL command already accepted\r\n");
		return;
	}
	else if (!prefix ("from:", arg)) {
		PP_LOG (LLOG_EXCEPTIONS,
			("No sender given in MAIL"));
		netreply ("501 No sender named\r\n");
		return;
	}

	/* -- Scan FROM: parts of arg -- */

	sender = index (arg, ':') + 1;
	sender = adrfix (sender);

	q_init (qp);
	new = list_bpt_new (hdr_822_bp); 
	list_bpt_add (&qp -> encodedinfo.eit_types, new); 
	new = list_bpt_new (ia5_bp); 
	list_bpt_add (&qp -> encodedinfo.eit_types, new);
	qp -> inbound = list_rchan_new (them, channel);

	ppstart ();

	if (rp_isbad (io_wrq (qp, &thereply))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't write Q struct %s", thereply.rp_line));	
		netreply ("451 Temporary problem initialising\r\n");
		pprestart ();
		return;
	}

	if (sender == NULLCP || *sender == NULL)
		sender = getpostmaster(AD_822_TYPE);

	PP_NOTICE (("Sender %s", sender));

	ap = adr_new (sender, AD_822_TYPE, 0);

	(void) io_wadr (ap, AD_ORIGINATOR, &thereply);
	adr_tfree (ap);

	switch (thereply.rp_val) {
		case RP_BHST:
			thereply.rp_val = RP_NDEL;
			break;	
	}
	if (rp_gbval (thereply.rp_val) == RP_BNO) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't write sender address %s [PERM]",
			 thereply.rp_line));
		(void) sprintf (replybuf, "501 %s\r\n", thereply.rp_line);
		netreply (replybuf);
		pprestart();
	}
	else if (rp_gbval (thereply.rp_val) == RP_BTNO) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't write sender address %s [TEMP]",
			 thereply.rp_line));
		(void) sprintf (replybuf, "451 %s\r\n", thereply.rp_line);
		netreply (replybuf);
		pprestart();
	}
	else
		netreply ("250 OK\r\n");

	no_recip = 0;
}



/*
The RCPT command  ("RCPT TO:<forward-path>")
*/

static void rcpt()
{
	RP_Buf		thereply;
	register char	*p;
	char		replybuf [BUFSIZ];
	ADDR		*ap;

	/* -- parse destination arg -- */

	if (hellod == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Expecting a HELO command"));
		hellod ++;	/* MH bug */
	}

	if (sender == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, ("RCPT but no MAIL"));
		netreply ("503 You must give a MAIL command first\r\n");
		return;
	}
	else if (arg == NULLCP || !prefix ("to:", arg)) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipient named in RCPT"));
		netreply ("501 No recipient named.\r\n");
		return;
	}

	p = index (arg, ':') + 1;
	p = adrfix (p);
	if (p == NULLCP || *p == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipient in RCPT"));
		netreply ("501 No recipient named.\r\n");
		return;
	}

	PP_NOTICE (("Recipient Address '%s'", p));


	ap = adr_new (p, AD_822_TYPE, no_recip + 1);

	(void) io_wadr (ap, AD_RECIPIENT, &thereply);

	adr_tfree (ap);

	if (thereply.rp_val == RP_BHST || 
	    rp_gbval (thereply.rp_val) == RP_BNO) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Problems writing address %s [PERM]",
			 thereply.rp_line));
		(void) sprintf (replybuf, "550 %s\r\n",
				thereply.rp_line);
		netreply (replybuf);
	}
	else if (rp_gbval (thereply.rp_val) == RP_BTNO) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Problems writing address %s [TEMP]",
			 thereply.rp_line));
		(void) sprintf (replybuf, "451 %s\r\n",
				thereply.rp_line);
		netreply (replybuf);
	}
	else {
		netreply ("250 Recipient OK.\r\n");
		no_recip++;
	}

}



/*----------------------------------------------------------------------------


ADDRFIX()  --  This function takes the SMTP "path" and removes the leading
and trailing "<>"'s which would make the address illegal to RFC822 mailers.
Note that although the specification states that the angle brackets are
required, we will accept addresses  without them.   (DPK@BRL, 4 Jan 83)


----------------------------------------------------------------------------*/


static char *adrfix (addrp)
char			*addrp;
{
	register char	*cp;


	PP_TRACE (("adrfix %s", addrp));
	if (cp = index (addrp, '<')) {
		addrp = ++cp;

		if (cp = rindex (addrp, '>'))
			*cp = 0;
	}

	(void) compress (addrp, addrp);

	PP_TRACE (("adrfix(): '%s'", addrp));

	return (addrp);
}


/*
The DATA command.  Send text to Submit.
*/

static void data()
{
	char	tbuf[LINESIZE];
	struct rp_bufstruct	thereply;
	struct timeval data_time;
	register char	*p,
				*bufptr;
	int		errflg,
				werrflg,
				size = 0,
				doing_header = 1;

	errflg	= 0;
	werrflg = 0;

	timer_start (&data_time);

	if (no_recip == 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipients for DATA"));
		netreply ("503 No recipients have been specified.\r\n");
		return;
	}

	if (rp_isbad (io_adend(&thereply))) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("Bad address end %s", thereply.rp_line));
		netreply ("451 Unknown mail system trouble.\r\n");
		return;
	}

	if (rp_isbad (io_tinit (&thereply)) ||
	    rp_isbad (io_tpart (hdr_822_bp, FALSE, &thereply))) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("Bad text init %s", thereply.rp_line));
		netreply ("451 Unknown text initialisation failure.\r\n");
		return;
	}

	netreply ("354 Enter Mail, end by a line with only '.'\r\n");

	dont_mung = 1;	    /* -- tell getline only to drop cr -- */


	PP_TRACE (("body of message"));

	while (1) {  /* -- forever -- */

		if ((p = getline()) == 0) {
			p = "\n***Sender closed connection***\n";
			errflg++;
			break;
		}

		if (p == (char *)NOTOK) {
			p = "\n***Error on net connection***\n";
			if (!errflg++)
				PP_LOG (LLOG_EXCEPTIONS,
					("netread error from host %s",
					realthem));
			break;
		}

		/* -- are we done? -- */

		if (buf [0] == '.')
			if (buf [1] == '\n')
				break;	/* -- yep -- */
			else
				bufptr = &buf [1];  /* -- skip leading . -- */
		else
			bufptr = &buf [0];

		/* -- If write err occurs, stop writing but keep reading -- */

		if (!werrflg) {
			size += p-bufptr;

			if (doing_header && buf [0] == '\n') {
				/*
				  do we need to go convert into body parts ??
				  */

				doing_header = 0;

				if (rp_isbad (io_tdend (&thereply))) {
					PP_LOG (LLOG_EXCEPTIONS,
						("io_tdend - %s",
						 thereply.rp_line));	
					netreply ("451 Data input problem.\r\n");
					return;
				}

				(void) sprintf (tbuf, "1.%s", ia5_bp);
				if (rp_isbad (io_tpart (tbuf, FALSE, &thereply))) {
					PP_LOG (LLOG_EXCEPTIONS,
						("io_tpart - %s",
						 thereply.rp_line));	
					netreply ("451 Body input failure.\r\n");
					return;
				}
			}
			else if (rp_isbad (io_tdata (bufptr, (p-bufptr)))) {
				werrflg++;
				PP_LOG (LLOG_EXCEPTIONS, 
					("error from submit"));
			}

		}
	}
	if (doing_header) {
		(void) sprintf (tbuf, "1.%s", ia5_bp);
		if (rp_isbad (io_tdend (&thereply)) ||
		    rp_isbad (io_tpart (tbuf, FALSE, &thereply))) {
			netreply ("451 Data Input problem.\r\n");
			return;
		}
	}
	dont_mung = 0;	/* -- set getline to normal operation -- */

	PP_TRACE (("Finished receiving text."));
	timer_end (&data_time, size, "Data Received");

	if (werrflg) {
		netreply ("451-Mail trouble (write error to mailsystem)\r\n");
		netreply ("451 Please try again later.\r\n");
		byebye (1);
	}

	if (errflg)
	    byebye (1);

	if (rp_isbad (io_tdend (&thereply))) {
		PP_LOG (LLOG_EXCEPTIONS, ("io_tdend - %s", thereply.rp_line));
		netreply ("451 Data termination problems.\r\n");
		return;
	}

	(void) io_tend (&thereply);

	if (rp_isgood (rp_gval (thereply.rp_val))) {
		(void) sprintf (buf, "250 %s\r\n", thereply.rp_line);
		netreply (buf);
	}
	else if (rp_gbval (thereply.rp_val) == RP_BNO) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("io_tend failed %s", thereply.rp_line));
		(void) sprintf (buf, "554 %s\r\n", thereply.rp_line);
		netreply (buf);
		return;
	}
	else {
		PP_LOG (LLOG_EXCEPTIONS, 
			("io_tend failed %s", thereply.rp_line));
		(void) sprintf (buf, "451 %s\r\n", thereply.rp_line);
		netreply (buf);
		return;
	}

	PP_NOTICE (("<<< Message received from %s (%d recipients, %d bytes)",
		    realthem, no_recip, size));
	sender = NULLCP;
	no_recip = 0;
}



/*
The RSET command
*/

static void rset()
{
	pprestart ();
	confirm();
}

static void pprestart ()
{
	io_end (NOTOK);
	ppbegin ();
}

static void ppbegin ()
{
	RP_Buf reply;

	if (rp_isbad (io_init(&reply))) {
		PP_LOG (LLOG_EXCEPTIONS,
			 ("can't reinitialize mail system [%s]",
			 reply.rp_line));
		netreply ("421 Server can't initialize mail system (PP)\r\n");
		byebye (2);
	}
	sender = NULLCP;
	no_recip = 0;
}

static void ppstart()
{
	RP_Buf reply;
	struct prm_vars prm;

	prm_init (&prm);
	if (rp_isbad (io_wprm (&prm, &reply))) {
		PP_LOG (LLOG_EXCEPTIONS, ("can't set parameters [%s]",
			 reply.rp_line));
		netreply ("421 Server can't initialize mail system (PP)\r\n");
		byebye (2);
	}

	no_recip = 0;
}



/*
The QUIT command
*/

static void quit()
{
	time_t	timenow;

	PP_NOTICE (("smtp server quit"));

	time (&timenow);

	(void) sprintf (buf, "221 %s says goodbye to %s at %.19s.\r\n",
		us, them, ctime (&timenow));
	netreply (buf);

	byebye (0);
}


static void byebye (retval)
int retval;
{
	/*
	if (retval == OK)
		mm_sbend();
	*/
	if (rp_isbad (retval))
		PP_LOG (LLOG_EXCEPTIONS, ("Smtp server aborting"));

	io_end (retval == 0 ? OK : NOTOK);
	exit (retval);
}


/*
Reply that the current command has been logged and noted
*/

static void confirm()
{
	netreply ("250 OK\r\n");
}



/*
The help command gives a list of valid commands
*/
static void help()
{
	register int		i;
	register struct comarr	*p;
	char			replybuf [BUFSIZ];

	netreply ("214-The following commands are accepted:\r\n214-");

	for (p=commands, i=1;  p->cmdname;  p++, i++) {
		(void) sprintf (replybuf, "%s%s", p->cmdname,
				((i%10)	 ?  " "	 :  "\r\n214-" ));
		netreply (replybuf);
	}

	(void) sprintf (replybuf, "\r\n214 Send complaints/bugs to: %s\r\n",
			postmaster);
	netreply (replybuf);
}

/* vrfy - attempt to verify user is OK */

static void vrfy ()
{
	RP_Buf rp;
	ADDR	*adr;
	char buffer[BUFSIZ];
	
	if (hellod == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Expecting a HELO command"));
		hellod ++;		/* MH bug */
	}

	if (arg == NULLCP || *arg == 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("No argument to VRFY command"));
		netreply ("501 No argument supplied\r\n");
		return;
	}
	adr = adr_new (arg, AD_822_TYPE, 1);
	if (rp_isbad (ad_parse (adr, &rp, CH_USA_PREF))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Failed to VRFY '%s' [%s]", arg, rp.rp_line));
		(void) sprintf (buffer,
				"550 User unknown [%s]\r\n",
				rp.rp_line);
		netreply (buffer);
	}
	else {
		(void) sprintf (buffer, "250 <%s>\r\n",
				adr -> ad_type == AD_822_TYPE ?
				adr -> ad_r822adr : adr -> ad_r400adr);
		netreply (buffer);
	}
	adr_free (adr);
}

static void expn ()
{
	dl *list;
	Name	*np;
	struct stat statbuf;
	char	buffer[BUFSIZ];

	if (hellod == 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Expecting a HELO command"));
		hellod ++;	/* MH bug */
	}

	if (arg == NULLCP || *arg == 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("No argument to VRFY command"));
		netreply ("501 No argument supplied\r\n");
		return;
	}
	switch (tb_getdl (arg, &list, OK)) {
	    case DONE:
		if (list != NULL)
			dl_free(list);
		PP_NOTICE (("No such list %s", arg));
		netreply ("550 No such list\r\n");
		return;

	    case NOTOK:
		if (list != NULL)
			dl_free(list);
		PP_LOG (LLOG_EXCEPTIONS, ("Error locating list %s", arg));
		netreply ("550 List Error\r\n");
		return;
	}

	if (list -> dl_file == NULLCP ||
	    stat (list -> dl_file, &statbuf) == NOTOK ||
	    (statbuf.st_mode & (~S_IFMT)) == SECRMODE) {
		netreply ("550 Access Denied to you\r\n");
		dl_free (list);
		return;
	}

	for (np = list -> dl_list; np; np = np -> next) {
		(void) sprintf (buffer, "250%c%s\r\n",
				np -> next == NULL ? ' ' : '-',
				np -> name);
		netreply (buffer);
	}
	dl_free (list);
}
		
/*
Send appropriate ascii responses over the network connection.
*/

static void netreply (str)
char	*str;
{
	PP_LOG (LLOG_PDUS, ("-> %s", str));

	if (timeout (NTIMEOUT))
		byebye (1);

	if (write (1, str, strlen (str)) < 0) {
		timeout (0);
		PP_SLOG (LLOG_EXCEPTIONS, "write",
			("(netreply) in writing [%s]", str));
		byebye (1);
	}

	timeout (0);

	PP_TRACE (("%s", str));
}

static char	*official (name)
char	*name;
{
	struct hostent *hp;
#ifdef NAMESERVER
	int	i;

        for(i = 0 ; i < 10 ; i++){
                hp = gethostbyname(name);
                if(hp != NULL)
                        break;
		PP_TRACE (("Timeout looking up %s", name));
                if(i > 2)
                        sleep(i*2);
        }
#else
	hp = gethostbyname(name);
#endif
	return hp == NULL ? NULLCP : strdup (hp -> h_name);
}
