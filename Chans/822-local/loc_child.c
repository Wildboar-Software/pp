/* loc_child.c: do the actual delivery */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/822-local/RCS/loc_child.c,v 6.0 1991/12/18 20:04:31 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/822-local/RCS/loc_child.c,v 6.0 1991/12/18 20:04:31 jpo Rel $
 *
 * $Log: loc_child.c,v $
 * Revision 6.0  1991/12/18  20:04:31  jpo
 * Release 6.0
 *
 */


#include "util.h"
#include "qmgr.h"
#include <sys/wait.h>
#include <signal.h>
#include "sys.file.h"
#include "retcode.h"
#include <sys/stat.h>
#include <varargs.h>
#include "expand.h"
#include "q.h"
#include "loc_user.h"
#include "ap.h"
#ifdef SYS5
#ifdef vfork
#undef vfork
#endif
#include <unistd.h>
#endif


#ifndef L_INCR
#define L_INCR SEEK_CUR
#endif
#ifndef L_SET
#define L_SET SEEK_SET
#endif

#ifndef WCOREDUMP
#define WCOREDUMP(x) (((union wait *)&(x))->w_coredump)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(x) (((union wait *)&(x)) -> w_retcode)
#endif
#ifndef WTERMSIG
#define WTERMSIG(x) (((union wait *)&(x)) -> w_termsig)
#endif

#define PROTMODE (0022)
#ifdef notdef
#define TESTVERSION	/* do setuid'ing */
#endif

extern	char	*mailfilter, *sysmailfilter;
extern	char	*delim1, *delim2, *mboxname;
extern	char	*envp[];
extern	char	env_path[];
extern	char	*local_user;
extern	long	message_size;
extern Q_struct Qstruct;
extern CHAN	*mychan;
extern void	create_var ();
void	adios (), advise ();

static	char msg_size[20];

static	int hdr_fd, body_fd;

static	int	restrict;	/* >0 -> restricted access */

static	void setstatus (), cleanup (), suckuperrors ();

static	int	copy (), copynofrom (), readinfile ();
static	int	do_mailfilter ();
static char *returnpath, *localhost;
static char	*lowerfy ();
static char	*return_path ();
static LocUser	*this_user_opts;

#define	MAXVARS	100
static	Expand	variables[MAXVARS];
static int nvars;

int	child_process (loc, h_fd, b_fd)
LocUser	*loc;
int	h_fd, b_fd;
{
	int	childpid, pid;
#ifdef SVR4
	int status;
#else
	union wait status;
#endif

	PP_TRACE (("child_process (loc, %d, %d)", h_fd, b_fd));

	if ((childpid = tryfork ()) == NOTOK)
		return NOTOK;

	if (childpid) {		/* parent */
#ifdef SVR4
		while ((pid = wait(&status)) != childpid && pid != -1)
#else
		while ((pid = wait(&status.w_status)) != childpid && pid != -1)
#endif
			continue;
		if (pid == -1) {
			PP_LOG (LLOG_EXCEPTIONS, ("Pid == -1 something bad"));
			return NOTOK;
		}
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		PP_LOG (LLOG_EXCEPTIONS, ("processed killed %s",
					  WCOREDUMP(status) ? "core dumped" :
					  ""));
		return NOTOK;
	}

	/* child */
	hdr_fd = h_fd;
	body_fd = b_fd;
	this_user_opts = loc;

#ifndef TESTVERSION
	if (loc -> username &&
	    initgroups (loc -> username, loc -> gid) == NOTOK)
		PP_SLOG (LLOG_EXCEPTIONS, loc -> username,
			("Cant initilize groups list for"));
#ifdef SYS5
        if (setgid (loc->gid) == -1 ||
            setuid (loc->uid) == -1) {
#else
	if (setregid (loc -> gid, loc -> gid) == NOTOK ||
	    setreuid (loc -> uid, loc -> uid) == NOTOK) {
#endif
		PP_SLOG (LLOG_EXCEPTIONS,
			 (loc -> username ? loc -> username : "<none>"),
			 ("Can't set uid (channel not setuid?) for uid-=%d id",
			  loc-> uid ));
		_exit (int_Qmgr_status_mtaFailure);
	}
#endif
	PP_TRACE (("Changing directory to %s", loc -> directory));
	if (chdir (loc -> directory) == NOTOK) {
		PP_SLOG(LLOG_EXCEPTIONS, loc -> directory,
			("Can't change to directory"));
		_exit (int_Qmgr_status_mtaFailure);
	}

	returnpath = return_path (Qstruct.Oaddress -> ad_r822adr);
	localhost = getlocalhost ();

	if (loc -> mailfilter)
		setstatus (do_mailfilter (loc -> mailfilter, 0, loc));

	if (loc -> sysmailfilter)
		setstatus (do_mailfilter (loc -> sysmailfilter, 1, loc));

	switch (loc -> mailformat) {
	    case MF_PP:
		setstatus (putfile (loc -> mailbox));
		break;
	    case MF_UNIX:
		setstatus (putunixfile (loc -> mailbox));
		break;
	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown default mbox format %d",
					  loc -> mailformat));
		break;
	}
	_exit (int_Qmgr_status_messageFailure);
	return NOTOK;
}

		
static void setstatus (retval)
int	retval;
{
	PP_TRACE (("setstatus (%s)", rp_valstr(retval)));
	if (rp_isgood (retval)) {
		PP_TRACE (("Delivery done"));
		_exit (int_Qmgr_status_success);
	}
	else if (retval == RP_NOOP) {
		PP_TRACE (("Trying next method"));
		return;
	}
	else if (retval == RP_AGN) {
		PP_TRACE (("Retry later"));
		_exit (int_Qmgr_status_messageFailure);
	}
	PP_TRACE (("Failed"));
	_exit (int_Qmgr_status_negativeDR);
}

static char *return_path (str)
char	*str;
{
	extern int ap_outtype;
        char     *newaddr = NULLCP;
        AP_ptr  tree = NULLAP,
		group = NULLAP,
		name = NULLAP,
		local = NULLAP,
		domain = NULLAP,
		route = NULLAP;

	switch (mychan -> ch_ad_order) {
	    case CH_UK_ONLY:
	    case CH_UK_PREF:
		ap_outtype = AP_PARSE_822 | AP_PARSE_BIG;
		break;

	    default:
		return strdup (str);
	}

	if (ap_s2p  (str, &tree, &group, &name, &local, &domain, &route)
            == (char *)NOTOK) {
                PP_LOG (LLOG_EXCEPTIONS,
                        ("Unable to parse (s2p) Recipient addr %s", str));
                return NULLCP;
        }


        if ((newaddr = ap_p2s (group, name, local, domain, route))
            == (char *)NOTOK) {
                PP_LOG (LLOG_EXCEPTIONS,
                        ("Unable to parse (p2s) Recipient addr %s", str));
                newaddr = NULLCP;
        }

        ap_free (tree);

        return lowerfy(newaddr);
}

static jmp_buf jbuf;

static int do_mailfilter (file, sysflg, loc)
char	*file;
int	sysflg;
LocUser	*loc;
{
	struct stat statbuf;
	static int	n;
	char	gbuf[30];

	PP_TRACE (("do_mailfilter (%s, %d)", file, sysflg));

	if (stat (file, &statbuf) == NOTOK) {
		PP_TRACE (("No file %s...", file));
		return RP_NOOP;
	}

	if (statbuf.st_mode & PROTMODE) {
		PP_NOTICE (("File %s has wrong modes", file));
		return RP_NOOP;
	}
	if (statbuf.st_uid != 0 &&
	    (sysflg == 0 && statbuf.st_uid != loc -> uid)) {
		PP_NOTICE (("File %s not owned by owner or root", file));
		return RP_NOOP;
	}

	if (setjmp (jbuf) == DONE)
		return RP_AGN;

	reset_syms ();
	initialise ();
	zapvars (variables, MAXVARS);

	if (readinfile (file) != OK)
		return RP_AGN;

	n = 0;
	(void) sprintf (msg_size, "%d", message_size);
	variables[n].macro = strdup("size");
	variables[n].expansion = strdup (msg_size);
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("return-path");
	variables[n].expansion =
		strdup (returnpath);
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("mailbox");
	variables[n].expansion = strdup (loc -> mailbox);
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("recipient");
	variables[n].expansion = strdup (local_user);
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("userid");
	if (loc -> username)
		variables[n].expansion = strdup (loc -> username);
	else {
		(void) sprintf (gbuf, "%d", loc -> uid);
		variables[n].expansion = strdup (gbuf);
	}
	create_var (&variables[n]);
	n ++;

	variables[n].macro = strdup ("groupid");
	(void) sprintf (gbuf, "%d", loc -> gid);
	variables[n].expansion = strdup (gbuf);
	create_var (&variables[n]);
	n ++;

	variables[n].macro = strdup ("channelname");
	variables[n].expansion = strdup (mychan -> ch_name);
	create_var (&variables[n]);
	n++;

	variables[n].macro = strdup ("hostname");
	variables[n].expansion = strdup (localhost);
	create_var (&variables[n]);
	n ++;

	nvars = parse_hdr (hdr_fd, variables + n, MAXVARS - n) + n;

	return run ();
}

static int sigpiped, sigalarmed;

int putfile (file)
char	*file;
{
	FILE	*fp;
	off_t	oldlen;
	int	n;
	int exists;
	char buffer[BUFSIZ];

	install_vars (variables, nvars, MAXVARS);
	expand (buffer, file, variables);
	PP_TRACE (("putfile %s", buffer));

	sigalarmed = sigpiped = 0;

	if (lseek (hdr_fd, 0L, L_SET) == NOTOK ||
	    lseek (body_fd, 0L, L_SET) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, "failed", ("lseek"));
		return RP_AGN;
	}

	exists = access (buffer, 0) == 0;

	if ((fp = flckopen (buffer, "a")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, buffer, ("Can't open file"));
		return RP_AGN;
	}	
	if (!exists)
		(void) fchmod (fileno (fp), 0600);

	oldlen = lseek (fileno (fp), 0L, L_INCR);
	if (oldlen < 0) {
		(void) flckclose (fp);
		return RP_AGN;
	}

	n = strlen (delim1);
	if (fwrite (delim1, 1, n, fp) != n) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}

	fprintf (fp, "Return-Path: <%s>\n",
		 returnpath);
	if (copy (hdr_fd, fp) == NOTOK) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}

	putc ('\n', fp);

	if (copy (body_fd, fp) == NOTOK) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}

	n = strlen (delim2);
	if (fwrite (delim2, 1, n, fp) != n) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}

	if (flckclose (fp) != OK) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}

	PP_TRACE (("Message written to file %s", buffer));
	return RP_OK;
}

int putunixfile (file)
char	*file;
{
	FILE	*fp;
	time_t now;
	char	*tstr;
	off_t	oldlen;
	int	n;
	int exists;
	char	buf[BUFSIZ];
	char buffer[BUFSIZ];
	time_t	time ();

	install_vars (variables, nvars, MAXVARS);
	expand (buffer, file, variables);

	sigalarmed = sigpiped = 0;

	PP_TRACE (("putunixfile %s", buffer));

	if (lseek (hdr_fd, 0L, L_SET) == NOTOK ||
	    lseek (body_fd, 0L, L_SET) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, "failed", ("lseek"));
		return RP_AGN;
	}

	exists = access (buffer, 0) == 0;

	if ((fp = flckopen (buffer, "a")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, buffer, ("Can't open file"));
		return RP_AGN;
	}	
	if (!exists)
		(void) fchmod (fileno (fp), 0600);

	oldlen = lseek (fileno (fp), 0L, L_INCR);
	if (oldlen < 0) {
		(void) flckclose (fp);
		return RP_AGN;
	}

	(void) time (&now);
	tstr = ctime (&now);
	(void) sprintf (buf, "From %s %s",
			returnpath, tstr);

	n = strlen (buf);
	if (fwrite (buf, 1, n, fp) != n) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}

	fprintf (fp, "Return-Path: <%s>\n",
		 returnpath);
	if (copynofrom (hdr_fd, fp) == NOTOK) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}

	putc ('\n', fp);

	if (copynofrom (body_fd, fp) == NOTOK) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}
	putc ('\n', fp);

	if (fflush (fp) != OK) {
		cleanup (fp, oldlen);
		return RP_AGN;
	}
	(void) flckclose (fp);

	PP_TRACE (("Message written to file %s", buffer));
	return RP_OK;
}

static void cleanup (fp, len)
FILE	*fp;
off_t	len;
{
	int	fd;

	fd = dup (fileno (fp));
	(void) flckclose (fp);

	(void) ftruncate (fd, len); /* attempt to undo the damage */

	(void) close (fd);
}

static SFD alarmed (), piped();
static jmp_buf pipe_arlm_jmp;

putpipe (proc)
char	*proc;
{
	int	pid, cpid;
	int	retval;
	SFP	oldalarm, oldpipe;
	int	tofds[2], fromfds[2];
	char	buffer[BUFSIZ];
	int	killed = 0;
	FILE	*fp;
#ifdef SVR4
	int status;
#else
	union wait status;
#endif

	PP_TRACE(("putpipe (%s)", proc));

	if (lseek (hdr_fd, 0L, L_SET) == NOTOK ||
	    lseek (body_fd, 0L, L_SET) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, "failed", ("lseek"));
		return RP_AGN;
	}

	if (pipe (tofds) == NOTOK)
		return RP_AGN;
	if (pipe (fromfds) == NOTOK) {
		(void) close (tofds[0]);
		(void) close (tofds[1]);
		return RP_AGN;
	}

	if ((pid = tryfork ()) == NOTOK) { /* argggh! So near */
		(void) close (tofds[0]);
		(void) close (tofds[1]);
		(void) close (fromfds[0]);
		(void) close (fromfds[1]);
		return RP_AGN;
	}

	if (pid == 0) {		/* kid */
		int	i;

		install_vars (variables, nvars, MAXVARS);
		(void) expand (buffer, proc, variables);
		PP_TRACE (("exec %s", buffer));
		if (restrict <= 0)
			fillin_var ("PATH", env_path);

		(void) close (tofds[1]);
		(void) dup2 (tofds[0], 0);
		(void) close (tofds[0]);

		(void) close (fromfds[0]);
		(void) dup2 (fromfds[1], 1);
		(void) dup2 (fromfds[1], 2);
		(void) close (fromfds[1]);
		for (i = getdtablesize (); i > 2; i--)
			(void) close (i);
#ifndef SYS5
		(void) setpgrp (0, getpid ());
#endif
#ifdef TIOCNOTTY
		if ((i = open ("/dev/tty", O_RDWR, 0)) >= 0) {
			(void) ioctl (i, TIOCNOTTY, 0);
			(void) close (i);
		}
#endif	
		if (restrict > 0) {
			char	pathname[BUFSIZ];
			char	*argv[50];
			sstr2arg (buffer, 50, argv, " \t\n");
			if (index (argv[0], '/')) {
				PP_LOG (LLOG_EXCEPTIONS,
					("argv[0] contains a '/' '%s'",
					 argv[0]));
				_exit (1);
			}
			(void) sprintf (pathname, "%s/%s",
					this_user_opts -> searchpath, argv[0]);
			(void) execve (pathname, argv, envp);
			_exit (1);
		}
		(void) execle ("/bin/sh", "sh", "-c", buffer, NULLCP, envp);
		_exit (1);
	}

	(void) close (tofds[0]);
	(void) close (fromfds[1]);
	fp = fdopen (tofds[1], "w");

	oldalarm = signal (SIGALRM, alarmed);
	oldpipe = signal (SIGPIPE, piped);
	sigpiped = 0;
	sigalarmed = 0;

	(void) alarm ((unsigned)(300 + message_size/20));
	PP_TRACE (("alarm set for %d secs", 300 + message_size/20));

	if (setjmp (pipe_arlm_jmp) != DONE) {
		fprintf (fp, "Return-Path: <%s>\n",
			 returnpath);
		if (copy (hdr_fd, fp) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS, ("copy error"));
			sigpiped = 1; /* pretend it failed - it probably did */
		}
		if (!sigpiped) {
			fputs ("\n", fp);

			if (copy (body_fd, fp) == NOTOK) {
				PP_LOG (LLOG_EXCEPTIONS, ("copy error"));
				sigpiped = 1;
			}
		}
	}
	else {
		if (sigalarmed)
			PP_TRACE (("alarm went off"));
		if (sigpiped)
			PP_TRACE (("pipe went off"));
	}

	if (sigpiped) { /* pipe died - reset for timeout */
		sigpiped = 0;
		if (setjmp (pipe_arlm_jmp) == DONE)
			PP_TRACE (("Timeout"));
	}

	if (sigalarmed) {	/* alarm went off - shoot it dead! */
		PP_NOTICE (("Process taking too long ... killing"));
		killed = 1;
		(void) killpg (pid, SIGTERM);
		sleep (2);
		(void) killpg (pid, SIGKILL);
	}
	if (fp) {
		(void) fclose (fp);
		fp = NULL;
	}

	suckuperrors (fromfds[0]);

#ifdef SVR4
	while ((cpid = wait (&status)) != pid && pid != -1)
#else
	while ((cpid = wait (&status.w_status)) != pid && pid != -1)
#endif
		PP_TRACE (("proc %d", cpid));

	PP_TRACE (("pid %d returned term=%d, retcode=%d core=%d killed =%d",
		   pid, WTERMSIG(status), WEXITSTATUS(status),
		   WCOREDUMP(status), killed));
	(void) alarm (0);
	suckuperrors (fromfds[0]);

	if (!killed && cpid == pid && WIFEXITED (status)) {
		if (WEXITSTATUS(status) == 0)
			retval = RP_OK;
		else if (rp_gbval(WEXITSTATUS(status)) == RP_BNO)
			retval = RP_BAD;
		else	retval = RP_AGN;
	}
	else retval = RP_AGN;

	/* restore status */
	(void) close (fromfds[0]);
	(void) signal (SIGPIPE, oldpipe);
	(void) signal (SIGALRM, oldalarm);

	return retval;
}

static SFD alarmed ()
{
	sigalarmed = 1;
	longjmp (pipe_arlm_jmp, DONE);
}

static SFD piped ()
{
	sigpiped = 1;
	longjmp (pipe_arlm_jmp, DONE);
}

static int copy (fd, fp)
int	fd;
FILE	*fp;
{
	char	buf[BUFSIZ];
	int	n = 0;
	char lastc = '\n';
	struct stat st;
	int count;

	if (fstat (fd, &st) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, "", ("Can't stat file"));
		return NOTOK;
	}
	count = st.st_size;

	PP_TRACE (("copy (%d -> %d) %d bytes", fd, fileno (fp), count));
	while (!sigpiped && !sigalarmed &&
	       (n = read (fd, buf, sizeof buf)) > 0) {
		count -= n;
		lastc = buf[n-1];
		if (fwrite (buf, 1, n, fp) != n)
			return NOTOK;
	}
	if (n >= 0 && !sigalarmed && !sigpiped && count > 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("File copy not correct %d bytes left", count));
		return NOTOK;
	}
		

	if (n >= 0 && lastc != '\n')
		(void) putc ('\n', fp);
	return n < 0 || fflush(fp) == EOF || ferror(fp) ? NOTOK : OK;
}

static int copynofrom (fd, fp)
int	fd;
FILE	*fp;
{
	char	buf[BUFSIZ];
	int	n;
	FILE	*fp2;
	char lastc = '\n';

	if ((fp2 = fdopen (dup(fd), "r")) == NULL)
		return NOTOK;

	PP_TRACE (("copy (%d -> %d)", fd, fileno (fp)));
	while (!sigpiped && !sigalarmed &&
	       fgets (buf, sizeof buf, fp2)) {
		if (strncmp (buf, "From ", 5) == 0)
			putc ('>', fp);
		if (fputs (buf, fp) == EOF) {
			(void) fclose (fp2);
			return NOTOK;
		}
		lastc = buf[strlen(buf) -1];
	}
	if (lastc != '\n')
		(void) putc ('\n', fp);
	n = ferror (fp2) || ferror (fp) ? NOTOK : OK;
	if (fclose (fp2) == NOTOK)
		return NOTOK;
	return n;
}

#ifndef	lint
void	adios (va_alist)
va_dcl
{
    va_list ap;
    char buffer[BUFSIZ];

    va_start (ap);
    
    asprintf (buffer, ap);
    ll_log (pp_log_norm, LLOG_EXCEPTIONS, NULLCP, "%s", buffer);

    va_end (ap);
    longjmp (jbuf, DONE);
}
#else
/* VARARGS2 */

void	adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif


#ifndef	lint
void	advise (va_alist)
va_dcl
{
    int	    code;
    va_list ap;

    va_start (ap);
    
    code = va_arg (ap, int);

    _ll_log (pp_log_norm, code, ap);

    va_end (ap);
}
#else
/* VARARGS3 */

void	advise (code, what, fmt)
char   *what,
       *fmt;
int	code;
{
    advise (code, what, fmt);
}
#endif

static int readinfile (file)
char	*file;
{
	extern FILE	*yyin;

	PP_TRACE (("readinfile (%s)", file));
	if ((yyin = fopen (file, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, file, ("Can't open file"));
		return NOTOK;
	}

	yyparse ();

	(void) fclose (yyin);
	return OK;
}

static void suckuperrors (fd)
int	fd;
{
	fd_set	rfds, ifds;
	char	buf[80];	/* smallish */
	int	n;
	struct timeval timer;

	timerclear(&timer);

	PP_TRACE (("suckuperrors (%d)", fd));
	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);

#define the_universe_exists	1

	while (the_universe_exists) {
		ifds = rfds;

		if (select (fd + 1, &ifds, NULLFD, NULLFD, &timer) <= 0)
			break;

		PP_TRACE (("select fired"));
		if (FD_ISSET (fd, &ifds)) {
			if((n = read (fd, buf, sizeof buf - 1)) > 0) {
				buf[n] = 0;
				PP_LOG (LLOG_EXCEPTIONS,
					("Output from process '%s'", buf));
			}
			else break;
		}
		else	break;
	}
	PP_TRACE (("suckuperrors - done"));
}
				
zapvars (vp, mvp)
Expand *vp;
int	mvp;
{
	while (mvp --) {
		if (vp -> macro) {
			free (vp -> macro);
			vp -> macro = NULL;
		}
		if (vp -> expansion) {
			free (vp -> expansion);
			vp -> expansion = NULL;
		}
		vp ++;
	}
}

/* ARGSUSED */
printit (s)
char	*s;
{
	return;
}

static char *lowerfy (s)
char	*s;
{
	register char *cp;

	for (cp = s;*cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	return s;
}

