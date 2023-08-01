/* submit_txt.c: handles the bodies of the messages */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_txt.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_txt.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: submit_txt.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "list_bpt.h"
#include "q.h"
#include "chan.h"
#include <isode/internet.h>
#include <fcntl.h>
#include <sys/stat.h>


extern LIST_BPT		*bodies_all, *headers_all;
extern int		privileged;
extern char		*loc_dom_mta;
extern char		*mgt_inhost;
extern char		*msg_basedir;
extern char		*msg_unique;
extern char		*cont_p2;
extern char		*cont_p22;
extern int		submit_debug;
extern char		msg_fullpath[];
extern void		pro_reply ();
extern void txt_mgt (), err_abrt();


/* -- globals  -- */
char			io_fpart[MAXPATHLENGTH];
long			msg_size;
int			numBps;
int			forwMsg;

/* -- static variables -- */
static FILE		*io_ofp;	/* -- output -- */
static int		io_linkpart;	/* -- linked part so no txt copy -- */
static char		*io_ostrend;
static char		io_tfname[MAXPATHLENGTH];


/* -- local routines -- */
int			txt_tend();
int			txt_init();
void			txt_input();
static int		txt_bptfind();
static int		txt_pend();
static int		txt_pinit();
static int		txt_read();
static void		txt_bp_check();
static void		txt_in();
static void		txt_via();
static void		txt_in_debug();



/* ---------------------  Begin	 Routines  -------------------------------- */




void txt_input (qp, addvia)   /* -- control the text -- */
Q_struct *qp;
int	addvia;
{
	int		retval,
			nparts = 0,
			first = TRUE;
	char lbuf[BUFSIZ];

	PP_TRACE (("txt_input()"));

	/* -- set up to recieve text -- */
	if (rp_isbad (retval = txt_read (lbuf)))
		err_abrt (retval, "Protocol failure");

	if ((retval = txt_init(qp)) != RP_OK)
		err_abrt (retval, "Text initialisation");

	pro_reply (RP_OK, "base %s", io_tfname);

	while ((retval = txt_pinit(qp)) == RP_OK) {
		PP_TRACE (("text loop"));
		/* -- add 822 trace -- */
		if (first && !io_linkpart && addvia)
			txt_via (qp, io_ofp);
		first = FALSE;
		nparts++;
		if (io_linkpart)
			continue;
		if (submit_debug)
			txt_in_debug ();
		else
			txt_in();
		if ((retval = txt_pend()) != RP_OK)
			err_abrt (retval, "Text ending");
	}

	if (rp_isbad (txt_tend()))
		retval = RP_MECH;

	if (retval != RP_DONE)
		err_abrt (RP_MECH, "Unknown error in txt_input");

	txt_mgt(qp);
}




int txt_tend()	/* -- finally finish -- */
{
	PP_TRACE (("txt_tend()"));

	if (io_ofp != NULLFILE) {
		if(check_close (io_ofp) == NOTOK)
			return RP_MECH;
		io_ofp = NULLFILE;
	}

	if (isstr (io_ostrend))
		* (io_ostrend - 1) = '\0';
	return RP_OK;
}




/* ----------------------  Static  Routines  -------------------------------- */




int txt_init(qp)
Q_struct *qp;
{

	PP_TRACE (("txt_init ('%s', '%s')", msg_fullpath, msg_basedir));

	io_linkpart	= FALSE;
	*io_fpart	= '\0';
	msg_size	= 0;
	qp -> msgsize = 0;
	numBps	= 0;
	forwMsg = 0;

	if (mkdir (msg_basedir, 0777) < 0)
		err_abrt (RP_FIO, "Cannot create base dir %s", msg_basedir);

	(void) strcpy (io_tfname, msg_basedir);
	(void) strcat (io_tfname, "/");

	/* -- make io_ostrend point to the end of the string. -- */
	io_ostrend = io_tfname + strlen (io_tfname);
	io_ofp = NULLFILE;

	return (RP_OK);
}




/* -- initialise for reading a single part of a msg -- */
static int txt_pinit(qp)
Q_struct *qp;
{
	char	lbuf[LINESIZE],
		*splt;
	int	retval;

	PP_DBG (("txt_pinit (%s)", io_tfname));

	io_linkpart = FALSE;	/* -- reset -- */

	if (rp_isbad (retval = txt_read (lbuf)))
		return (retval);

	if (lbuf[0] == '\0')
		return (RP_DONE);

	if ((splt = index (lbuf, ' ')) != NULL) {
		*splt++ = '\0';
		io_linkpart = TRUE;
		if (!privileged)
			err_abrt (RP_USER, "Mortals cannot link");
	}

	if(rp_isbad (retval =  txt_psetup (qp, lbuf, splt)))
		return retval;

	if (io_linkpart)
		pro_reply (RP_OK, "linked the files");
	else
		pro_reply (RP_OK, "file is '%s'", io_tfname);

	return (RP_OK);
}

int txt_psetup (qp, bp, linkfile)
Q_struct *qp;
char	*bp, *linkfile;
{
	char *p;
	struct stat	statbuf;
	int fd;

	txt_bp_check (qp, bp);

	(void) strcpy (io_ostrend, bp);

	/* make relevant directories */
	for (p = io_ostrend; (p = index (p, '/')) != NULL; p++) {
		*p = '\0';
		(void) mkdir (io_tfname, 0777);
		*p = '/';
	}


	if (io_linkpart) {
		if (link (linkfile, io_tfname) < 0)
			err_abrt (RP_FIO, "Cannot link file '%s' to '%s'",
				  linkfile, io_tfname);

		if (*io_fpart == '\0')
			(void) strcpy (io_fpart, io_tfname);

		/* -- check message size -- */
		if (stat (io_tfname, &statbuf) >= 0)
			msg_size += statbuf.st_size;
		return (RP_OK);
	}


	if ((fd = open (io_tfname, O_CREAT | O_WRONLY | O_EXCL, 0644)) < 0)
		err_abrt (RP_FIO, "Can't create file '%s'", io_tfname);


	if ((io_ofp = fdopen (fd, "w")) == NULLFILE) {
		(void) close (fd);
		(void) unlink (io_tfname);
		*io_tfname = '\0';
		err_abrt (RP_FIO, "cannot open io_tfname");
	}


	if (*io_fpart == '\0')
		(void) strcpy (io_fpart, io_tfname);
	return RP_OK;
}




static int txt_pend()  /* -- end of the text input part -- */
{
	int retval;
	PP_TRACE (("txt_pend()"));

	if (rp_isbad (retval = txt_pfinish ()))
		return retval;
	pro_reply (RP_OK, "got this text part");

	return (RP_OK);
}

int txt_pfinish ()
{
	if (check_close (io_ofp) == NOTOK)
		err_abrt (RP_FIO, "Can't write text to disk");
	io_ofp = NULLFILE;
	return RP_OK;
}


static void txt_in()  /* -- input the text -- */
{
	long	datalen;
	long len;
	char	buf[BUFSIZ];

	PP_TRACE (("txt_in()"));

	while ((int)fread ((char *)&datalen, sizeof (datalen), 1, stdin) > 0) {
		datalen = ntohl (datalen);
		if ((len = datalen) == 0)
			return;
		while (len > 0) {
			int dlen, nc;

			dlen = len < sizeof (buf) ? len : sizeof (buf);
			nc = fread (buf, sizeof (buf[0]), dlen, stdin);
			if (nc <= 0)
				err_abrt (RP_FIO, "Read error");
			if (rp_isbad(txt_write (buf, nc)))
				err_abrt (RP_FIO, "Data copy failed");
			len -= nc;
		}
		PP_DBG (("submit/txt_in copied %d bytes",datalen));
	}
}

int txt_write (bp, nc)
char	*bp;
int	nc;
{
	msg_size += nc;
	if (fwrite (bp, sizeof (*bp), nc, io_ofp) != nc)
		return RP_FIO;
	return RP_OK;
}


static int txt_from_file (file)
char *file;
{
	char *cp;
	FILE *fp;
	char	buf[BUFSIZ];
	int n;

	file ++;
	if ((cp = index(file, '\n')) != NULLCP)
		*cp = 0;
	if ((fp = fopen (file, "r")) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("No such file %s", file));
		return NOTOK;
	}

	while ((n = fread (buf, 1, sizeof buf, fp)) > 0)
		if (rp_isbad (txt_write (buf, n)))
			err_abrt (RP_FIO, "Copy failed");
	return OK;
}

static void txt_in_debug ()  /* -- input the text -- */
{
	char buf[BUFSIZ];
	int n;
	int first = 1;
	PP_TRACE (("txt_in_debug ()"));

	while (fgets (buf, sizeof buf, stdin) && strcmp(buf, ".\n") != 0) {
		if (first && *buf == '<' && txt_from_file(buf) == OK)
			return;
		first = 0;
		msg_size += n = strlen(buf);
		if (rp_isbad(txt_write (buf, n)))
			err_abrt (RP_FIO, "data copy failed");
	}
}

static int txt_read (buf)  /* -- read the control flow -- */
char	*buf;
{
	register int	c;
	register char	*p;

	PP_TRACE (("txt_read()"));
	for (p = buf; (c = getc (stdin)) != EOF && c != '\n'; *p++ = c)
		continue;
	*p = '\0';
	PP_DBG (("text_read - read '%.10s'", buf));
	return ((c == EOF) ? RP_EOF : RP_OK);
}




static void txt_via (qp, fp)
Q_struct *qp;
FILE	*fp;
{
	char	buf[BUFSIZ];
	char	thedate[LINESIZE];
	char	showstr[BUFSIZ];
	UTC	ut, lt;
	extern UTC utclocalise();
	CHAN	*ch_in;

	ut = utcnow();
	lt = utclocalise(ut);
	(void) UTC2rfc(lt, thedate);
	free ((char *)ut);
	free ((char *)lt);

	ch_in = qp -> inbound -> li_chan;

	if (ch_in -> ch_trace_type == CH_TRACE_VIA)
		(void) sprintf (buf, "Via: %s; %s\n",
				loc_dom_mta, thedate);
	else {
		if ( ch_in -> ch_show && 
		     (strncmp (ch_in -> ch_show, "via", 3) == 0
		     || strncmp (ch_in -> ch_show, "with", 4) == 0)) {
			(void) strcpy (showstr, " ");
			(void) strcat (showstr, ch_in -> ch_show);
		}
		else showstr[0] = '\0';
			
		(void) sprintf (buf, "Received: from %s by %s%s id <%s@%s>; %s\n",
				mgt_inhost, loc_dom_mta,
				showstr,
				msg_unique + 4, loc_dom_mta,
				thedate);
	}
	fputs (buf, fp);
}




static void txt_bp_check (qp, file)
Q_struct *qp;
char	*file;
{
	char		*p;
	
	if (*file == '/')
		err_abrt (RP_PARM, "File has full pathname %s", file);

	if ((p = index (file, '/')) != NULLCP) {
		if (!isdigit (*file) || strncmp (p - 4, ".ipm", 4) != 0)
			err_abrt (RP_PARM, "Not a forwarded message %s", file);
		txt_bp_check (qp, ++p);
		forwMsg++;
		return;
	}


	/* -- check against content types -- */
	if (strcmp (file, cont_p2) == 0
	    || strcmp (file, cont_p22) == 0
	    || (isstr(qp->cont_type) && strcmp (file, qp->cont_type) == 0))
		return; 

	if (isdigit (*file)) {
		for (p = file + 1; isdigit (*p); p++)
			continue;
		if (*p != '.')
			err_abrt (RP_PARM, "Bad filename syntax %s", file);
		p++;
	}
	else	p = file;



	/* -- check if this body part is allowed -- */

	if (!txt_bptfind (bodies_all, p)
	    && !txt_bptfind(headers_all, p))
		err_abrt (RP_PARM, "Unknown body part '%s'", file);



	/* -- check the file against the list of eits in Q -- */

	if (!txt_bptfind (qp->encodedinfo.eit_types, p))
		err_abrt (RP_PARM, 
			"'%s' is not specified in the orig encoded info", file);

	numBps++;
	return;
}




static int txt_bptfind (list, item)
LIST_BPT	*list;
char		*item;
{

	if (list_bpt_nfind (list, item, strlen(item)))
		return TRUE;

	return FALSE;
}
