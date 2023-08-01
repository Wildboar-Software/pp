/* submit_que.c: store messages in the queue */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_que.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/submit_que.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: submit_que.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include        "head.h"
#include        "prm.h"
#include        "q.h"
#include 	"adr.h"
#include        <fcntl.h>
#if defined(sun) || defined(SYS5)
#ifdef vfork
#undef vfork
#endif
#include	<unistd.h>
#endif
#include 	"sys.file.h"
#include        <sys/time.h>


extern UTC      time_t2utc();
extern void	err_abrt();
extern long     msg_size;
extern char 	*quedfldir;
extern char     *aquefile;              /* addr file */
extern char	*bquedir;               /* Q/msg/msg.XXX/base dir */
extern int	queue_depth;
extern int	queue_fanout;
extern int	errno;
extern int	disk_blocks, disk_percent;

/* -- globals -- */
char            *msg_basedir;           /* full pathname to bquedir */
char		*msg_unique;            /* unique-part of Q/msg file name */
char		msg_fullpath[MAXPATHLENGTH]; /* full pathname to msg */
static char	*ad_file;	        /* the (temp) addr file */


/* -- static variables -- */
static FILE     *ad_ffp;                /* file pointer to address file */

/* -- local routines -- */
void		clear_q();
int		move_q();
int		winit_q();
int		write_q();
static int	make_temp();
static void make_hier_queue ();
static void make_missing_dirs ();



/* ---------------------  Begin  Routines  -------------------------------- */




/* -- initialise system to write a message into the queue. -- */
int winit_q(rp)
RP_Buf *rp;
{
	char            buf[MAXPATHLENGTH];
	char		addition[BUFSIZ];

	PP_TRACE (("submit/winit_q()"));

	(void) strcpy (msg_fullpath, quedfldir);
	(void) strcat (msg_fullpath, "/");
	msg_unique = msg_fullpath + strlen (msg_fullpath);
	if (queue_depth > 0)
		make_hier_queue (addition);
	if (make_temp (msg_unique, queue_depth > 0 ? addition : NULLCP,
		       msg_fullpath) == NOTOK)
		return rplose (rp, RP_FCRT, "Can't create a unique Q file");

	if (ad_file)
		free (ad_file);
	(void) sprintf (buf, "%s/%s.X", msg_fullpath, aquefile);
	ad_file = strdup (buf);

	if ((ad_ffp = fopen (ad_file, "w")) == NULL)
		rplose (rp, RP_FOPN, "Can't access text Q file %s", buf);

	(void) sprintf (buf, "%s/%s", msg_fullpath, bquedir);
	msg_basedir = strdup (buf);

	if (fdiskfull (fileno(ad_ffp), disk_blocks, disk_percent) == NOTOK) {
		(void) fclose (ad_ffp);
		return rplose (rp, RP_FIO, "Disk usgae limit exceeded");
	}
	return RP_OK;
}

int move_q (rp)
RP_Buf *rp;
{
	char    tfile[MAXPATHLENGTH];

	(void) sprintf  (tfile, "%s/%s", msg_fullpath, aquefile);
	PP_TRACE (("Renames %s to %s", ad_file, tfile));
	if (rename (ad_file, tfile) == NOTOK)
		return rplose (rp, RP_FCRT, "Can't rename %s to %s",
			       ad_file, tfile);
	free (ad_file);
	ad_file = NULLCP;
	return RP_OK;
}




int write_q (qp, pr, rp)
register Q_struct               *qp;
register struct prm_vars        *pr;
RP_Buf *rp;
{
	int                     retval;
	ADDR                    *ap;

	PP_TRACE (("submit/write_q (qp,pr)"));

	if (qp->msgsize == NULL)
		qp->msgsize = msg_size;

	if (qp->queuetime == 0)
		qp->queuetime = utcnow ();

	if (pr -> prm_opts)
		pr -> prm_opts = PRM_NONE;
	if (pr -> prm_passwd) {
		free (pr -> prm_passwd);
		pr -> prm_passwd = NULLCP;
	}

	retval = wr_prm (pr, ad_ffp);
	if (rp_isbad (retval))
		goto bad;

	retval = wr_q (qp, ad_ffp);
	if (rp_isbad (retval))
		goto bad;

	retval = wr_adr (qp->Oaddress, ad_ffp, AD_ORIGINATOR);
	if (rp_isbad (retval))
		goto bad;

	for (ap = qp -> Raddress; ap; ap = ap -> ad_next) {
		retval = wr_adr (ap, ad_ffp, AD_RECIPIENT);
		if (rp_isbad (retval))
			goto bad;
	}

	if (check_close (ad_ffp) == NOTOK)
		err_abrt (RP_FIO, "Error writing address file");
	ad_ffp = NULLFILE;
	return RP_OK;

bad:
	return rplose (rp, RP_FIO, "Error writing to q file");
}




/* -- in case of error, zap all the work thats been done. -- */
void clear_q()
{
	PP_TRACE (("clear_q ()"));
	(void) txt_tend();
	if (ad_ffp != NULLFILE) { /* address file */
		(void) fclose (ad_ffp);
		ad_ffp = NULLFILE;
	}
	if (ad_file) {
		(void) recrm (msg_fullpath);
		if(rmdir (msg_fullpath))
			PP_SLOG (LLOG_EXCEPTIONS, msg_fullpath,
				 ("failed to remove directory"));
		else
			PP_NOTICE(("Cleanup - zapped %s", msg_fullpath));
	}
}




/* ---------------------  Static  Routines  ------------------------------- */

static int	mypid = 0;

static void make_missing_dirs (base, seper)
char	*base;
char	*seper;
{
	char	buf[BUFSIZ];
	char	*p;

	(void) strcpy (buf, base);
	p = buf + strlen (buf);
	(void) strcpy (p, seper);

	while ((p = index(p, '/')) != NULL) {
		*p = '\0';
		if (mkdir(buf, 0777) == NOTOK && errno != EEXIST) {
			*p = '/';
			err_abrt (RP_FIO, "Can't create directory %s",
				  buf);
		}
		*p = '/';
		p++;
	}
}
#ifndef BSD42
#define random rand
#define srandom srand
#endif

static void make_hier_queue (buf)
char *buf;
{
	char *p;
	static int init = 0;
	int i;

	if (init == 0) {
		if (mypid == 0)
			mypid = getpid ();
		srandom (mypid);
		init = 1;
	}

	p = buf;
	for (i = 0; i < queue_depth; i++) {
		(void) sprintf (p, "%d/", random() % queue_fanout);
		p += strlen(p);
	}
}

/*
 * mktemp gives up too easily - this ones goes on forever
 * I think we'll run out of inodes long before this fails...
 */


static int make_temp (unique, seper, base)
char    *base;
char	*seper;
char    *unique;
{
	static long     tries = 0;
	int 	made_dirs = 0;

	if (mypid == 0) mypid = getpid ();

	for (; tries < 32000; tries ++) {
		if (seper)
			(void) sprintf (unique, "%smsg.%05d-%d",
					seper, mypid, tries);
		else
			(void) sprintf (unique, "msg.%05d-%d",
					mypid, tries);
		
		if (mkdir (base, 0777) == OK)
			return OK;
		else if (seper && !made_dirs) {
			tries --; /* try again after we created missing dirs */
			made_dirs = 1;
			unique[0] = '\0';
			make_missing_dirs (base, seper);
		}
			
	}
	return NOTOK;
}

static struct matrix_tbl {
	char	*name;
	char	*key;
	char	*directory;
	int	no;
	struct matrix_tbl *m_next;
} *MTbl;

static char *do_table (str, name)
char *str, *name;
{
	struct matrix_tbl **mp, *m;
	int maxkeyno = 1;
	char buf[BUFSIZ];

	for (mp = &MTbl; 
	     (m = *mp) != (struct matrix_tbl *) NULL; 
	     mp = &(*mp) -> m_next) {
		if (strcmp(m -> name, str) == 0)
			return strdup(m -> directory);
		else if (strcmp (m -> key, name) == 0) {
			if (m -> no >= maxkeyno)
				maxkeyno = m -> no + 1;
		}
	}
	m = *mp = (struct matrix_tbl *) smalloc(sizeof **mp);
	m -> name = strdup (str);
	m -> key = strdup(name);
	m -> no = maxkeyno;
	(void) sprintf (buf, "%s.%d", name, maxkeyno);
	m -> directory = strdup (buf);
	m -> m_next = NULL;
	return strdup (m -> directory);
}

void free_table (mp)
struct matrix_tbl *mp;
{
	if (mp == (struct matrix_tbl *) NULL)
		return;
	if (mp -> name) free (mp -> name);
	if (mp -> key) free (mp -> key);
	if (mp -> directory) free (mp -> directory);
	if (mp -> m_next) free_table(mp -> m_next);
	free ((char *)mp);
}

do_dirs (ap)
ADDR *ap;
{
	char	buf[BUFSIZ];
	LIST_RCHAN *fmt;

	buf[0] = 0;
	for (fmt = ap -> ad_fmtchan; fmt; fmt = fmt -> li_next) {
		(void) strcat (buf, "/");
		(void) strcat (buf, fmt -> li_chan -> ch_name);
		fmt -> li_dir = do_table (buf, fmt -> li_chan -> ch_name);
	}
}

setup_directories (qp)
Q_struct *qp;
{
	ADDR *ap;
	do_dirs (qp -> Oaddress);
	for (ap = qp -> Raddress; ap; ap = ap -> ad_next)
		do_dirs (ap);

	free_table (MTbl);
	MTbl = NULL;
}
