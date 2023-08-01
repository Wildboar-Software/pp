/* io.c: This file contains the entire io library */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/io_lib.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/io_lib.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: io_lib.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "adr.h"
#include "prm.h"
#include "dr.h"
#include <signal.h>
#include <isode/internet.h>
#include <sys/wait.h>


extern char     *cmddfldir;
extern char     *submit_addr;
extern int	errno;

static  int     mm_cid;       /* process id of child mail process */
static FILE     *io_wfp,
		*io_rfp;

extern char	*submit_prog;
int             remote_submit = 0;
char		*submit_port = "4001";
extern  int     protocol_mode;
static enum {
	is_init,
	is_parm,
	is_queue,
	is_addrs,
	is_adend,
	is_txtinit,
	is_data,
	is_txtend,
	} is_state = is_init;

#define check_proto(x)	if (is_state != (x)) \
	return io_lose (rp, RP_MECH, "Protocol state mismatch")

static int io_init_remote ();
static int io_init_local ();
static int io_lose ();
static void set_protocol_mode ();
static void unset_protocol_mode ();
static int io_rrply ();
static int io_wrec ();
static int io_rrec ();
static int io_tdata_aux ();
static char *io_data_vis ();
SFP	sigpipe;

extern void getfpath ();
/* ---------------------  Begin  Routines  -------------------------------- */




/*
Initialise for submission
*/

int io_init (rp)
RP_Buf  *rp;
{
	char *remote_info;
	char _remote_info[BUFSIZ];

	is_state = is_init;
	if (io_wfp != NULLFILE) {
		(void) fclose (io_wfp);
		io_wfp = NULLFILE;
	}
	if (io_rfp != NULLFILE) {
		(void) fclose (io_rfp);
		io_rfp = NULLFILE;
	}

	if (remote_info = getenv("PP_HOST_INFO"))
	{
		(void) strncpy(_remote_info, remote_info, sizeof _remote_info);
		_remote_info[sizeof _remote_info -1] = '\0';	
		remote_info = _remote_info;
	}
	else if (remote_info = submit_addr)
	{
		(void) strncpy(_remote_info, remote_info, sizeof _remote_info);
		_remote_info[sizeof _remote_info -1] = '\0';
		remote_info = _remote_info;
	}
	else	remote_info = NULLCP;

	if (remote_info && *remote_info == '\0')
		remote_info = NULLCP;

	PP_TRACE(("remote info %s", remote_info ? remote_info : "<none>"));

	if (remote_info) {
		char	*argv[50];
		int	argc, i;
		char	*port_name, *p;

		argc = sstr2arg (remote_info, 50, argv, ",");

		for (i = 0; i < argc; i++) {
			port_name = submit_port;

			if (p = index (argv[i], ':')) {
				*p++ = '\0';
				port_name = p;
			}
			if (argv[i][0] == '/') {
				if (io_init_local (rp, argv[i]) == RP_OK)
					return RP_OK;
			}
			else {
				if (io_init_remote (rp, argv[i],
						   port_name) == RP_OK)
					return RP_OK;
			}
		}
	}
	return (io_init_local (rp, NULLCP));
}




/*
Parameter handling
*/

int io_wprm (prm, rp)
struct prm_vars         *prm;
RP_Buf                  *rp;
{
	int             retval;

	PP_DBG (("io_wprm (prm, rp)"));

	check_proto (is_init);

	if (io_wfp == NULLFILE)
		return (io_lose (rp, RP_MECH, "Not connected to sumbit"));

	sigpipe = signal (SIGPIPE, SIG_IGN);
	set_protocol_mode();
	retval = wr_prm (prm, io_wfp);
	(void) fflush (io_wfp);
	unset_protocol_mode();
	(void) signal (SIGPIPE, sigpipe);

	if (rp_isbad (retval) || ferror(io_wfp))
		return (io_lose (rp, retval, "Can't write parameters"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad handshake"));

	is_state = is_parm;
	return (rp -> rp_val);
}




/*
Q structure handling
*/

int io_wrq (qp, rp)
Q_struct        *qp;
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_wq (q, rp)"));

	if (io_wfp == NULLFILE)
		return (io_lose (rp, RP_MECH, "Not connected to submit"));
	check_proto (is_parm);

	sigpipe = signal (SIGPIPE, SIG_IGN);
	set_protocol_mode();
	retval = wr_q (qp, io_wfp);
	(void) fflush (io_wfp);
	unset_protocol_mode();
	(void) signal (SIGPIPE, sigpipe);

	if (rp_isbad (retval) || ferror(io_wfp))
		return (io_lose (rp, retval, "Can't write Q structure"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad handshake"));

	is_state = is_addrs;
	return (rp -> rp_val);
}




/*
DR structure handling
*/

int io_wdr (dp, rp)
DRmpdu          *dp;
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_wdr (dp, rp)"));

	if (io_wfp == NULLFILE)
		return (io_lose (rp, RP_MECH, "Not connected to submit"));
	check_proto (is_adend);

	sigpipe = signal (SIGPIPE, SIG_IGN);
	set_protocol_mode();
	retval = wr_dr (dp, io_wfp);
	(void) fflush (io_wfp);
	unset_protocol_mode();
	(void) signal (SIGPIPE, sigpipe);

	if (rp_isbad (retval) || ferror(io_wfp))
		return (io_lose (rp, retval, "Can't write DR structure"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad Handshake"));

	is_state = is_adend;
	return (rp -> rp_val);
}




/*
Address handling
*/

int io_wadr (ap, type, rp)
ADDR            *ap;
int             type;
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_wadr (adr, %d, rp)", type));

	if (io_wfp == NULLFILE)
		return (io_lose (rp, RP_MECH, "Not connected to submit"));
	check_proto (is_addrs);

	sigpipe = signal (SIGPIPE, SIG_IGN);
	set_protocol_mode();
	retval = wr_adr (ap, io_wfp, type);
	(void) fflush (io_wfp);
	unset_protocol_mode();
	(void) signal (SIGPIPE, sigpipe);

	if (rp_isbad (retval) || ferror(io_wfp))
		return (io_lose (rp, retval, "Can't write address"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad handshake"));
	if (rp -> rp_val != RP_AOK)
		return RP_BAD;
	return (rp -> rp_val);
}


/*
Address phase termination
*/

int io_adend (rp)
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_adend (rp)"));

	if (io_wfp == NULLFILE)
		return (io_lose (rp, RP_MECH, "Not connected to sumbit"));
	check_proto (is_addrs);

	if (rp_isbad (retval = io_wrec ("", 0)))
		return (io_lose (rp, retval, "Problem ending addreses"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad Handshake"));

	is_state = is_adend;
	return (rp -> rp_val);
}




/*
Text phase - initialisation
*/

int io_tinit (rp)
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_tinit()"));

	if (io_wfp == NULLFILE)
		return (io_lose (rp, RP_MECH, "Not connected to submit"));
	check_proto (is_adend);

	if ( rp_isbad (retval = io_wrec ("", 0)))
		return (io_lose (rp, retval, "Problem in text init"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad handshake"));

	PP_DBG (("Returned %s\n", rp->rp_line));
	is_state = is_txtinit;
	return (rp -> rp_val);
}




/*
Initialise to pass a single body part to submit
*/

int io_tpart (section, links, rp)
char            *section;
int             links;
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_tpart (%s, %d)", section, links));

	check_proto (is_txtinit);
	if (rp_isbad (retval = io_wrec (section, strlen (section))))
		return (io_lose (rp, retval, "Error writing to submit"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Handshake failure"));

	if (links != TRUE)
		is_state = is_data;
	return (rp -> rp_val);
}




/*
Write data to submit.
*/

int io_tdata (buf, len)
char            *buf;
int             len;
{
	if (is_state != is_data)
		return NOTOK;
	PP_LOG (LLOG_PDUS, ("Lib/io_tdata(%d): %s", len,
			    io_data_vis (buf, len)));
	return (io_tdata_aux (buf, len, TRUE));
}




/*
End a section sent to submit
*/

int io_tdend (rp)
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_tdend (ret)"));

	check_proto (is_data);
	if (rp_isbad (retval = io_tdata_aux ("", 0, FALSE)))
		return (io_lose (rp, retval, "I/O error"));

	(void) fflush (io_wfp);
	if (ferror (io_wfp))
		return (io_lose (rp, RP_FIO, "Write error"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad Handshake"));

	is_state = is_txtinit;
	return (rp -> rp_val);
}




/*
Finally finish the data section
*/

int io_tend (rp)
RP_Buf          *rp;
{
	int     retval;

	PP_DBG (("io_tend (ret)"));

	check_proto (is_txtinit);
	if (rp_isbad (retval = io_wrec ("", 0)))
		return (io_lose (rp, retval, "I/O error"));

	if (rp_isbad (retval = io_rrply (rp)))
		return (io_lose (rp, retval, "Bad Handshake"));

	if (rp_isbad (rp -> rp_val))
		return rp -> rp_val;

	if (rp_isbad (retval = io_rrply (rp)))
		return io_lose (rp, retval, "Bad Handshake");

	is_state = is_init;
	return (rp -> rp_val);
}




int io_end (type)
int     type;
{
	register int   status = OK;
	int pid;
#ifdef SVR4
	int w;
#else
	union wait w;
#endif

	PP_DBG (("io_end (%d)", type));

	(void) fclose (io_wfp);
	(void) fclose (io_rfp);
	io_rfp = NULLFILE;
	io_wfp = NULLFILE;

	if (!remote_submit)
#ifdef SVR4
		while ((pid = wait(&w)) != mm_cid && pid != -1)
#else
		while ((pid = wait(&w.w_status)) != mm_cid && pid != -1)
#endif
			continue;

	mm_cid = 0;
	is_state = is_init;
	return (status);
}




/* ---------------------  Static  Routines  ------------------------------- */




/*
Helper routine if local connection is required
*/

static int io_init_local (rp, command)
RP_Buf  *rp;
char	*command;
{
	char            temppath[LINESIZE];
	int             sv[2],
			osv[2],
			child,
			i;

	PP_DBG (("<pipe> io_init()"));

	if (pipe (sv) < 0 || pipe (osv) < 0)
		return (io_lose (rp, RP_LIO,
				"Can't create connection to child"));

	if ((child = tryfork()) < 0) {
		(void) close (sv[0]);
		(void) close (sv[1]);
		(void) close (osv[0]);
		(void) close (osv[1]);
		return (io_lose (rp, RP_LIO, "Try again (fork)"));
	}


	if (child == 0) {
		/*
		I am the child (I will use sv[1])
		*/

		if (command && *command)
			(void) strncpy (temppath, command, sizeof temppath);
		else
			getfpath (cmddfldir, submit_prog, temppath);

		if (osv[0] != 0) {
			(void) dup2 (osv[0], 0);
			(void) close (osv[0]);
		}
		if (sv[1] != 1) {
			(void) dup2 (sv[1], 1);
			(void) close (sv[1]);
		}
		(void) close (osv[1]);
		(void) close (sv[0]);
		for (i = getdtablesize() ; i >= 2 ; i--)
			(void) close (i);

		(void) execl (temppath, submit_prog, 0);

		PP_OPER (temppath,  ("Cannot exec"));

		_exit (1);
	}

	/*
	I am the parent - I will use sv[0]
	*/
	mm_cid = child;
	(void) close (sv[1]);
	io_rfp = fdopen (sv[0], "r");
	(void) close (osv[0]);
	io_wfp = fdopen (osv[1], "w");
	return (RP_OK);
}




/*
Helper routine for remote connect (submit server)
*/

static int  io_init_remote (rp, submit_host, submit_port_name)
RP_Buf  *rp;
char	*submit_host;
char	*submit_port_name;
{
	struct servent          *sp;
	u_short                 port;
	struct hostent          *host;
	struct sockaddr_in      s_in;
	int                     sd;

	PP_DBG (("<socket> io_init()"));

	if (isdigit (*submit_port_name))
	    port = htons ((u_short)atoi(submit_port_name));
	else if ((sp = getservbyname (submit_port_name, "tcp")) != NULL)
	    port = sp -> s_port;
	else
	    port = htons ((u_short)atoi(submit_port));

	sd = socket (AF_INET, SOCK_STREAM, 0);
	if (sd < 0)
		return (io_lose (rp, RP_LIO, "Can't get a socket"));

	bzero ((char *)&s_in, sizeof (s_in));
	s_in.sin_family = AF_INET;
	s_in.sin_port = port;

	if ((host = gethostbyname (submit_host)) == NULL) {
		(void) close (sd);
		PP_LOG (LLOG_EXCEPTIONS,
			("Remote server %s not found", submit_host));
		return (io_lose (rp, RP_MECH, "Can't locate host"));
	}

	bcopy ((char *)host -> h_addr, (char *)&s_in.sin_addr,
	       host -> h_length);

	if (connect (sd, (struct sockaddr *)&s_in, sizeof s_in) < 0) {
		(void) close (sd);
		return (io_lose (rp, RP_LIO, "Can't connect to submit"));
	}

	io_rfp = fdopen (sd, "r");
	io_wfp = fdopen (sd, "w");

	if (io_wfp == NULLFILE || io_rfp == NULLFILE)
	{	int was = errno;
		if (io_wfp == NULLFILE)
			(void) fclose(io_wfp);
		if (io_rfp == NULLFILE)
			(void) fclose(io_rfp);
		(void) close(sd);
		errno = was;
		return (io_lose (rp, RP_FIO, "File pointers not set"));
	}
	return (RP_OK);
}


/*
Helper routine to ensure correct protocol
*/

static int io_tdata_aux (buf, len, flag)
char            *buf;
int             len;
int             flag;
{
	long    datalen = htonl ((u_long)len);

	if (flag && len == 0)
		return (RP_MECH);

	(void) fwrite ((char *)&datalen, sizeof (datalen), 1, io_wfp);
	(void) fwrite (buf, sizeof (char), len, io_wfp);

	return ((ferror (io_wfp) ? RP_FIO : RP_OK));
}


/*
Get a reply from remote process
*/

static int io_rrply (rp)
RP_Buf          *rp;
{
	int     len;
	char    *rplystr;
	int	c;

	PP_DBG (("io_rrply (rp)"));

	if ((c = getc(io_rfp)) == EOF)
		return RP_EOF;
	rp -> rp_val = c;
	for (rplystr = rp -> rp_line, len = sizeof rp -> rp_line - 2;
	     len > 0; len --) {
		if ((c = getc(io_rfp)) == EOF)
			return RP_EOF;
		if (c == '\n')
			break;
		*rplystr++ = c;
	}
	if (c != '\n') {	/* no newline - find it */
		while ((c = getc(io_rfp)) != EOF && c != '\n')
			continue;
	}
	*rplystr = '\0';

	rplystr = rp_valstr ((int)rp -> rp_val);

	if (*rplystr == '*') {
		/*
		Replyer did a no-no
		*/
		PP_LOG (LLOG_EXCEPTIONS, ("ILLEGAL REPLY: (%s)", rplystr));
		rp -> rp_val = RP_RPLY;
	}

	return (RP_OK);
}






/*
Write a record
*/

static int io_wrec (linebuf, len)   /* write a record/packet */
register char   *linebuf;       /* chars to write */
register int    len;            /* number of chars to write */
{
	static char *io_data_vis ();
	PP_LOG (LLOG_PDUS, ("io_wrec ('%s', %d)",
			    io_data_vis (linebuf, len), len));


	sigpipe = signal (SIGPIPE, SIG_IGN);
	(void) fwrite (linebuf, sizeof (char), len, io_wfp);
	(void) putc ('\n', io_wfp);
	(void) fflush (io_wfp);   /* force it out */
	(void) signal (SIGPIPE, sigpipe);

	return (ferror (io_wfp) ? RP_LIO : RP_OK);
}




static int io_lose (rp, state, str)
RP_Buf          *rp;
int             state;
char            *str;
{
	rp -> rp_val = rp_gval (state);

	(void) strcpy (rp -> rp_line, str);

	PP_LOG (LLOG_NOTICE,
		("io_lose -> %s [%s]", rp_valstr (state), str));

	is_state = is_init;
	return (state);
}

static  char *io_data_vis (str, n)
char    *str;
int     n;
{
	static char    buffer[128];
	char *cp, *ep;

	for(cp = buffer, ep = buffer + sizeof(buffer) -1;
	    cp < ep && n-- > 0; str ++) {
		if (isprint (*str))
			*cp++ = *str;
		else if (cp >= ep - 3)
			break;
		else {
			*cp ++ = '\\';
			*cp ++ = '0' + ((*str >> 6) & 03);
			*cp ++ = '0' + ((*str >> 3) & 07);
			*cp ++ = '0' + ((*str) & 07);
		}
	}
	*cp = 0;
	return buffer;
}

static void set_protocol_mode ()
{
	unset_protocol_mode();
	protocol_mode = 1;
	return;
}


static void unset_protocol_mode ()
{
	static int      save = -1;

	if (save == -1)
		save = protocol_mode;
	else {
		protocol_mode = save;
		save = -1;
	}
}
