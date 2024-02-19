/* sm_wtmail:  mail-commands for smtp mail */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/smtp/RCS/sm_wtmail.c,v 6.0 1991/12/18 20:12:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/smtp/RCS/sm_wtmail.c,v 6.0 1991/12/18 20:12:19 jpo Rel $
 *
 * $Log: sm_wtmail.c,v $
 * Revision 6.0  1991/12/18  20:12:19  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"
#include "chan.h"
#include <signal.h>
#include <isode/internet.h>
#include "ap.h"

extern CHAN	*chanptr;
extern char	*loc_dom_mta;
extern char	*strdup();

extern	char	*open_host;

struct sm_rstruct		/* save last reply obtained */
{				  /* for holding a net reply */
    int	    sm_rval;		  /* rp.h value for reply */
    int	    sm_rlen;		  /* current lengh of reply string */
    char    sm_rgot;		  /* true, if have a reply */
    char    sm_rstr[LINESIZE];	  /* human-oriented reply text */
} sm_rp;
#define smtp_error(def) (sm_rp.sm_rgot ? sm_rp.sm_rstr : (def))

static CHAN	*sm_chptr;	/* structure for channel that we are */
FILE	*sm_rfp, *sm_wfp;
static char	sm_rnotext[] = "No reply text given";
static int sm_rrec ();

#define SM_HTIME	180	/* Time allowed for HELO command */
#define SM_OTIME	300	/* Time allowed for a netopen */
#define SM_ATIME	120	/* Time allowed for answer after open */
#define SM_STIME	300	/* Time allowed for MAIL command */
#define SM_TTIME	300	/* Time allowed for RCPT command */
#define SM_DTIME        120	/* Time allowed for a DATA command */
#define SM_PTIME        600	/* Time allowed for the "." command */

#define SM_QTIME	60	/* Time allowed for QUIT command */

int	data_bytes = 0;

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

void sm_nclose();

sm_wfrom (sender)
char	*sender;
{
	char	linebuf[LINESIZE];

	(void) sprintf (linebuf, "MAIL FROM:<%s>", sender);
	if (rp_isbad (sm_cmd (linebuf, SM_STIME)))
	    return (RP_DHST);

	switch( sm_rp.sm_rval ) {
	    case 250:
		break;		/* We're off and running! */

	    case 500:
	    case 501:
	    case 552:
		return RP_PARM;

	    case 421:
	    case 450:
	    case 451:
	    case 452:
		return RP_AGN;

	    default:
		return RP_BHST;
	}
	return RP_OK;
}

sm_wto (adr)	     /* send one address spec to local */
char	adr[];			  /* rest of address */
{
	char linebuf[LINESIZE];

	if( isstr(adr))
		(void) sprintf (linebuf, "RCPT TO:<%s>", adr);
	else
		(void) strcpy(linebuf, "RCPT TO:<>");

	if (rp_isbad (sm_cmd (linebuf, SM_TTIME)))
		return (RP_DHST);

	switch (sm_rp.sm_rval)
	{
	    case 250:
	    case 251:
		return RP_AOK;

	    case 421:
	    case 450:
	    case 451:
	    case 452:
		return RP_AGN;

	    case 550:
	    case 551:
	    case 552:
	    case 553:
	    case 554:		/* BOGUS: sendmail is out of spec! */
		return RP_USER;

	    case 500:
	    case 501:
		return RP_PARM;

	}
	return RP_RPLY;

}

sm_wrdata ()
{
	if (rp_isbad (sm_cmd ("DATA", SM_DTIME)))
		return RP_DHST;

	switch (sm_rp.sm_rval) {
	    case 354:
		return RP_OK;

	    case 500:
	    case 501:
	    case 503:
	    case 554:
		return RP_NDEL;

	    case 421:
	    case 451:
		return RP_AGN;

	}
	return RP_RPLY;
}

sm_wrdot (naddr)
int naddr;
{
	if (rp_isbad (sm_cmd (".", SM_PTIME + (3 * naddr))))
		return RP_DHST;

	switch (sm_rp.sm_rval) {
	    case 250:
	    case 251:
		return RP_OK;

	    case 552:
	    case 554:
		return RP_NDEL;

	    case 421:
	    case 451:
	    case 452:
		return RP_AGN;
	}	
	return RP_AGN;
}
		

sm_init (curchan)		  /* session initialization */
CHAN *curchan;			  /* name of channel */
{
    sm_chptr = curchan;
    /* phs_note (sm_chptr, PHS_CNSTRT); */
    return (RP_OK);		  /* generally, a no-op */
}


static
	sm_irdrply ()		  /* get net reply & stuff into sm_rp */
{
    static char sep[] = "; ";	  /* for sticking multi-lines together */
    short     len,
	    tmpreply,
	    retval;
    char    linebuf[LINESIZE];
    char    tmpmore;
    register char  *linestrt;	  /* to bypass bad initial chars in buf */
    register short    i;
    register char   more;	  /* are there continuation lines? */

newrply:
    for (more = FALSE, sm_rp.sm_rgot = FALSE, sm_rp.sm_rlen = 0;
	    rp_isgood (retval = sm_rrec (linebuf, sizeof linebuf, &len));)
    {
	    PP_LOG (LLOG_PDUS, ("<- %s", linebuf));
	    /* 1st col in linebuf gets reply code */
	    for (linestrt = linebuf; /* skip leading baddies, probably */
		 len > 0 &&	/*  from a lousy Multics */
		 (!isascii ((char) *linestrt) ||
		  !isdigit ((char) *linestrt));
		 linestrt++, len--)
		    continue;

	    tmpmore = FALSE;	/* start fresh */
	    tmpreply = atoi (linestrt);
	    if ((len -= 3) > 0)
	    {
		    linestrt += 3;
		    if (len > 0 && *linestrt == '-')
		    {
			    tmpmore = TRUE;
			    linestrt++;
			    if (--len > 0)
				    for (; len > 0 && isspace (*linestrt); linestrt++, len--)
					    continue;
		    }
	    }

	    if (more)		/* save reply value from 1st line */
	    {			/* we at end of continued reply? */
		    if (tmpreply != sm_rp.sm_rval || tmpmore)
			    continue;
		    more = FALSE; /* end of continuation */
	    }
	    else		/* not in continuation state */
	    {
		    sm_rp.sm_rval = tmpreply;

		    more = tmpmore; /* more lines to follow? */

		    if (len <= 0)
		    {		/* fake it, if no text given */
			    bcopy (sm_rnotext, linestrt = linebuf,
				   (sizeof sm_rnotext) - 1);
			    len = (sizeof sm_rnotext) - 1;
		    }
	    }

	    if ((i = MIN (len, (LINESIZE - 1) - sm_rp.sm_rlen)) > 0)
	    {			/* if room left, save the human text */
		    bcopy (linestrt, &sm_rp.sm_rstr[sm_rp.sm_rlen], i);
		    sm_rp.sm_rlen += i;
		    sm_rp.sm_rstr[sm_rp.sm_rlen] = 0;
		    if (more && sm_rp.sm_rlen < (LINESIZE - 4))
		    {		/* put a separator between lines */
			    bcopy (sep, &(sm_rp.sm_rstr[sm_rp.sm_rlen]), (sizeof sep));
			    sm_rp.sm_rlen += (sizeof sep) - 1;
		    }
	    }

	    if (!more)
	    {
		    if (sm_rp.sm_rval < 100)
			    goto newrply; /* skip info messages */

		    sm_rp.sm_rgot = TRUE;
		    return (RP_OK);
	    }
    }
    return (retval);		  /* error return */
}


static sm_rrec (linebuf, blen, len)	/* read a reply record from net */
char   *linebuf;		  /* where to stuff text */
int	blen;
short	 *len;			    /* where to stuff length */
{
    extern int errno;
    int c, n;
    char *cp;

    *len = 0;			  /* for clean logging if nothing read */
    linebuf[0] = '\0';

    for (n = blen, cp = linebuf; n > 0;) {
	    if ((c = getc (sm_rfp)) == EOF || c == '\n')
		    break;
	    *cp ++ = c;
	    n --;
    }
    *cp = '\0';
    *len = blen - n;

    if (ferror (sm_rfp) || feof (sm_rfp))
    {				  /* error or unexpected eof */
	PP_LOG (LLOG_EXCEPTIONS,
		("netread:  ret=%d, fd=%d", *len, fileno (sm_rfp)));
	sm_nclose (NOTOK);	   /* since it won't work anymore */
	return (RP_BHST);
    }
    if (n <= 0)
    {
	PP_LOG (LLOG_EXCEPTIONS, ("net input overflow"));
	while (getc (sm_rfp) != '\n'
		&& !ferror (sm_rfp) && !feof (sm_rfp))
		continue;
    }
    else
	if (linebuf[*len - 1] == '\r')
	    *len -= 1;		  /* get rid of crlf or just lf */

    return (RP_OK);
}

sm_cmd (cmd, time)		/* Send a command */
char	*cmd;
int	time;			/* Max time for sending and getting reply */
{
    short     retval;

    PP_LOG (LLOG_PDUS, ("-> %s", cmd));

    if (sm_wfp == NULL)
	return sm_rp.sm_rval = RP_DHST;
    if (timeout((unsigned)time)) {
	PP_TRACE (("sm_cmd(): host died?"));
	sm_nclose (NOTOK);
	return (sm_rp.sm_rval = RP_DHST);
    }
    fprintf (sm_wfp, "%s\r\n", cmd);
    if (!ferror (sm_wfp))
	    (void) fflush (sm_wfp);

    if (ferror (sm_wfp))
    {
	PP_TRACE (("sm_cmd(): host died?"));
	sm_nclose (NOTOK);
	return (sm_rp.sm_rval = RP_DHST);
    }

    if (rp_isbad (retval = sm_irdrply ()))
	return( sm_rp.sm_rval = retval );

    timeout (0);
    return RP_OK;
}

sm_wstm (buf, len)	      /* write some message text out */
char	*buf;		      /* what to write */
register int	len;		  /* how long it is */
{
	static char lastchar = 0;
	short	  retval;
	register char  *bufptr;
	register char	newline;

	if (buf == 0 && len == 0) { /* end of text */
		if (lastchar != '\n') {	/* make sure it ends cleanly */
			fputs ("\r\n", sm_wfp);
			data_bytes += 2;
		}
		if (ferror (sm_wfp))
			return (RP_DHST);
		lastchar = 0;	/* reset for next message */
		retval = RP_OK;
	}
	else
	{
		newline = (lastchar == '\n') ? TRUE : FALSE;
		for (bufptr = buf; len--; bufptr++)
		{
			switch (*bufptr) /* cycle through the buffer */
			{
			    case '\n': /* Telnet requires crlf */
				newline = TRUE;
				putc ('\r', sm_wfp);
				data_bytes ++;
				break;

			    case '.': /* Insert extra period at beginning */
				if (newline) {
					putc ('.', sm_wfp);
					data_bytes ++;
				}
				/* DROP ON THROUGH */
			    default:
				newline = FALSE;
			}
			if (ferror (sm_wfp))
				break;
			/* finally send the data character */
			putc ((lastchar = *bufptr), sm_wfp);
			data_bytes ++;
			if (ferror (sm_wfp))
				break;
		}
		retval = ferror(sm_wfp) ? RP_DHST : RP_OK;
	}

	return (retval);
}

static int start_conn (sp, name, errstr)
struct sockaddr_in *sp;
char	*name;
char	*errstr;
{
	int s;

	s = socket (AF_INET, SOCK_STREAM, 0);
	if( s < 0 ) {
		(void) strcpy (errstr, "Can't get socket");
		PP_SLOG (LLOG_EXCEPTIONS, "socket",
			 ("%s", errstr));
		return NOTOK;
	}

	if (timeout(SM_OTIME)) {
		(void) sprintf (errstr, "[%s] open timeout", name,
				inet_ntoa (sp -> sin_addr));
		PP_LOG (LLOG_EXCEPTIONS,
			("%s", errstr));
		return NOTOK;
	}
	
	if( connect (s, (struct sockaddr *)sp, sizeof *sp) < 0 ) {
		(void) close (s);
		(void) sprintf (errstr, "Connection failed to %s [%s]",
				name,  inet_ntoa (sp -> sin_addr));
		PP_SLOG (LLOG_EXCEPTIONS, "connect",
			 ("%s", errstr));
		timeout (0);
		return NOTOK;
	}
	timeout (0);
	return  s;
}

static char	*official ();

sm_nopen(hostnam, errstr)
char	*hostnam;
char	*errstr;
{
	int	fds[2], skt;
	short	retval;
	char	linebuf[LINESIZE];
	static char *us;
	extern int smtpport;
	struct hostent *hp, *gethostbyname ();
	struct sockaddr_in s_in;
	char **aptr;

	PP_TRACE (("[ %s ]", hostnam));

	*errstr = 0;

	if ( ! us ) {
		us = official (loc_dom_mta);
		if(us == NULLCP) {
			PP_OPER (NULLCP, 
				 ("Cannot find my own 'official' name '%s'\n",
				  loc_dom_mta));
			(void) sprintf (errstr, "Who am I %s ?", loc_dom_mta);
			return RP_BHST;
		}
	}
#ifdef NAMESERVER
	if (rp_isbad (retval = ns_gethost (hostnam, &hp, errstr)))
		return (retval);
#else NAMESERVER
	if ((hp = gethostbyname (hostnam)) == NULL) {
		(void) sprintf (errstr, "Host lookup of %s failed", hostnam);
		return RP_BHST;
	}
#endif NAMESERVER

	if (smtpport == 0) {
		struct servent *sp;

		if ((sp = getservbyname ("smtp", "tcp")) == NULL) {
			(void) sprintf (errstr, "Can't determine smtp port");
			return RP_AGN;
		}
		smtpport = (short)sp->s_port;
	}
	bzero ((char *)&s_in, sizeof s_in);
	s_in.sin_family = AF_INET;
	s_in.sin_port = smtpport;


			/* SEK - try all addresses */
	for (aptr = hp -> h_addr_list; *aptr; aptr++)
	{
		bcopy (*aptr, (char *)&s_in.sin_addr, hp -> h_length); 
		PP_NOTICE (("Attempting open to [%s]", 
				 inet_ntoa (s_in.sin_addr)));

		if ((skt = start_conn (&s_in, hp -> h_name, errstr)) == NOTOK)
			continue;

		fds[0] = skt;
		fds[1] = dup (skt);

		if ((sm_rfp = fdopen (fds[0], "r")) == NULL ||
			(sm_wfp = fdopen (fds[1], "w")) == NULL) {
			return (RP_LIO);
		}

		if (timeout(SM_ATIME)) {
			(void) sprintf (errstr, "Timeout opening %s", hostnam);
			sm_nclose (NOTOK);
			return( RP_BHST );
		}
		if (rp_isbad (retval = sm_irdrply ())) {
			(void) sprintf (errstr, "Bad response from %s: %s",
					hostnam, smtp_error ("bad reply"));
			timeout (0);
			sm_nclose (NOTOK);
			return(retval);
		}
		timeout (0);

		if( sm_rp.sm_rval != 220 ) {
			sm_nclose (NOTOK);
			return( RP_BHST );
		}

		/* HELO is part of protocol - all commands are 4 letters long */
		(void) sprintf (linebuf, "HELO %s", us);
		if (rp_isbad (sm_cmd( linebuf, SM_HTIME))
		    || sm_rp.sm_rval != 250 ) {
			(void) sprintf (errstr, "Bad response to helo: %s",
					smtp_error("bad helo"));
			sm_nclose (NOTOK);
			return (RP_RPLY);
		}
		PP_NOTICE (("Connected to %s [%s]", hostnam,
			    inet_ntoa (s_in.sin_addr)));
		return (RP_OK);
	}
	return (RP_BHST);  /* No addresses OK */

}

void sm_nclose (type)		/* end current connection */
short	  type;			/* clean or dirty ending */
{
	if (type == OK && sm_wfp)
		sm_cmd ("QUIT", SM_QTIME);

	if (sm_rfp == NULL && sm_wfp == NULL)
		return;
	PP_NOTICE (("Closing connection to %s", open_host));
	if (open_host)
		free(open_host);
	open_host = NULLCP;

	if (timeout(15)) {
		return;
	}
	if (sm_rfp != NULL)
		(void) fclose (sm_rfp);
	if (sm_wfp != NULL)
		(void) fclose (sm_wfp);
	timeout (0);
	sm_rfp = sm_wfp = NULL;
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
                        sleep((unsigned) i*2);
        }
#else
	hp = gethostbyname(name);
#endif
	return hp == NULL ? NULLCP : strdup (hp -> h_name);
}
