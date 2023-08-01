/* slocal.c: structured message local delivery channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/slocal/RCS/slocal.c,v 6.0 1991/12/18 20:12:09 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/slocal/RCS/slocal.c,v 6.0 1991/12/18 20:12:09 jpo Rel $
 *
 * $Log: slocal.c,v $
 * Revision 6.0  1991/12/18  20:12:09  jpo
 * Release 6.0
 *
 */



/*********************************************************************\
*                                                                     *
* local channel - deliver as structured messages                      *
* (based on local.c 10/01/89)                                        *
*                                                                     *
*                                                                     *
\*********************************************************************/


#include "head.h"
#include "util.h"
#include "prm.h"
#include "q.h"
#include "qmgr.h"
#include "dr.h"
#include <isode/logger.h>
#include <sys/stat.h>
#include "loc_user.h"
#include "sys.file.h"
#include <isode/usr.dirent.h>

#define NULLDIR ((DIR*)0)
#define NULLDCT ((struct dirent *)0)

extern  char    *ad_getlocal();
extern  char    *quedfldir;
extern  CHAN    *ch_nm2struct();
extern void	err_abrt(), getfpath(), rd_end(), chan_init();
static Q_struct Qstruct;
static char     *basename();
static CHAN     *mychan;
static char     *this_msg;
static char     *formatdir=NULLCP;
static int      initproc();
static void     dirinit();
static struct   type_Qmgr_DeliveryStatus *process();

static char mboxdir[] =         "PPMail";       /* main PP mail directory */
static char inboxdir[] =        "inbox";        /* inbox */
static char tempdir[] =         "slocal";       /* temporary directory name */
static char alert_file[] =      "/alert";       /* alert script in PPMail */
static char alert_message[] =   "You have new mail from PP"; /* system call */
static char base_string[] =     "base";         /* basename target in file */
						/* from msg_rfile */

int   firstSuccessDR = TRUE;


/* -----------------------  Begin  Routines  ------------------------------ */



main (argc, argv)
int     argc;
char    **argv;
{
	char    *p;

	p = argv[0];
	if (*p == '/') {
		p = rindex (p, '/');
		p++;
	}

	/* Init the channel - and find out who we are */
	chan_init (p);

	dirinit();

#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0)
		debug_channel_control (argc,argv, initproc, process, NULLIFP);
	else
#endif

	channel_control (argc, argv, initproc, process, NULLIFP);

	exit (0);
}




static int initproc (arg)
struct type_Qmgr_Channel *arg;
{
	char    *p;

	p = qb2str (arg);

	PP_TRACE (("initproc (%s)", p));
	if ((mychan = ch_nm2struct (p)) == NULLCHAN)
		err_abrt (RP_PARM, "Channel '%s' not known", p);

	free (p);
	return OK;
}




static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct type_Qmgr_UserList *up;
	struct prm_vars         prm;
	Q_struct                *qp = &Qstruct;
	ADDR                    *ad_sendr = NULL,
				*ad_recip = NULL;
	int                     ad_count,
				retval;

	bzero ((char *) qp, sizeof (*qp));
	if (this_msg)   free (this_msg);

	this_msg = qb2str (arg -> qid);

	PP_TRACE (("slocal/processing msg %s", this_msg));


	delivery_init (arg -> users);

	retval = rd_msg (this_msg, &prm, qp, &ad_sendr, &ad_recip, &ad_count);

	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("slocal/rd_msg err: %s", this_msg));
		return delivery_setall (int_Qmgr_status_messageFailure);
	}

	for (up = arg -> users; up; up = up -> next)
		douser (up -> RecipientId -> parm, ad_recip, this_msg);

	rd_end();

	return deliverystate;
}



static void dirinit()  /* Change into pp queue space */
{
	PP_TRACE (("dirinit()"));
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, "Uanble to change directory to '%s'",
		quedfldir);
}



/* process one extension-id for this msg */

static int douser (rno, ad_recip, msgd) 
int     rno;
ADDR    *ad_recip;
char    *msgd;
{
	ADDR    *ap;

	PP_TRACE (("douser (%d, ad_recip, %s)", rno, msgd));

	for (ap = ad_recip; ap ; ap = ap->ad_next) {
		if (rno != ap -> ad_no)
			continue;
		if (lchan_acheck (ap, mychan, 1, NULLVP) == NOTOK)
			return RP_MECH;
		else {
			deliver (ap, msgd);
			return RP_OK;
		}
	}

	PP_LOG (LLOG_EXCEPTIONS, ("slocal/user not recipient of %s",
		this_msg));

	delivery_set (rno, int_Qmgr_status_messageFailure);
	return RP_MECH;
}



static int deliver (ap, msgd)
ADDR    *ap;
char    *msgd;
{
	static  LocUser *loc;

	char    umaildir[MAXPATHLENGTH];     /* usr structured mail dir - PPMail */
	char    uinboxdir[MAXPATHLENGTH];    /* usr structured inbox dir - PPMail*/
	char    utempdir[MAXPATHLENGTH];     /* temp dir - PPMail/inbox/.slocal */
	char    newname [MAXPATHLENGTH];     /* new name i.e. 12/, 13/ etc. */
	char    fname   [MAXPATHLENGTH];     /* filename */
	char    fnameout[MAXPATHLENGTH];     /* output name */
					/* PPMail/inbox/.slocal/filename */
	char    new_directory[MAXPATHLENGTH]; /* place for new dir names to */
					 /* be generated in message */
	char    ualert_file[MAXPATHLENGTH];  /* users alert file name */
	char    s_string[BUFSIZ];         /* system call string */
	char    *p,*q;
	char    *user;
	int	next_msgno;
	int     alert_exists;
	int     i;
	int     retval,sysreply;
	RP_Buf  rps, *rp = &rps;


	PP_TRACE (("slocal/deliver msgd=%s", msgd));

	if (ap->ad_outchan->li_chan != mychan) {
		PP_LOG (LLOG_EXCEPTIONS,
		     ("slocal/this extension-id (%d) is not for this channel",
		     ap->ad_extension));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_BAD;
	}

	if ((user = ad_getlocal (ap -> ad_r822adr, AD_822_TYPE)) == NULLCP) {
		char buffer[BUFSIZ];
		(void) sprintf (buffer, "User %s not local!", ap -> ad_r822adr);
		PP_LOG (LLOG_EXCEPTIONS, ("%s", buffer));
		delivery_set (ap -> ad_no, int_Qmgr_status_negativeDR);
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER, DRD_UA_UNAVAILABLE, buffer);
		wr_q2dr (&Qstruct, this_msg);
		return RP_USER;
	}

	if ((loc = tb_getlocal (user, mychan)) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS,
			 ("User %s not registered in chan %s table",
			user, mychan -> ch_name));
		free (user);
		delivery_set (ap -> ad_no, int_Qmgr_status_negativeDR);
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_UNABLE_TO_TRANSFER,
			 DRD_UA_UNAVAILABLE,
			 "User not registered in delivery table");
		wr_q2dr (&Qstruct, this_msg);
		return RP_USER;
	}


	PP_TRACE (("Found entry  for '%s' uid '%s' (%d) directory '%s'",
		   ap -> ad_r822adr,
		   loc -> username ? loc -> username : "<none>",
		   loc -> uid, loc -> directory));

	if (rp_isbad (loc_init (loc))) {
		PP_LOG (LLOG_EXCEPTIONS, ("loc_init error"));
		delivery_set (ap -> ad_no, int_Qmgr_status_mtaFailure);
		return RP_MECH;
	}

	/*
	Find the correctly formatted version of the message
	*/

	if (qid2dir (this_msg, ap, TRUE, &formatdir) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			 ("Can't locate message %s", this_msg));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_MECH;
	}

	/* --- *** --- 
	Try to cd into umaildir - if unable to do so then create 
	the directory and log it. Then try opening.  
	--- *** --- */

	(void) sprintf (umaildir, "%s/%s", loc -> directory, mboxdir);

	if (rp_isbad (loc_cd (umaildir, rp))) {
		if (rp_isbad (loc_mkdir (umaildir, 0700, rp))) {
			PP_LOG (LLOG_EXCEPTIONS,
				 ("slocal/Unable to create '%s'", umaildir));
			delivery_set (ap -> ad_no, 
				int_Qmgr_status_messageFailure);
			return RP_FOPN;
		}

		PP_LOG (LLOG_EXCEPTIONS,
			("slocal/Created directory '%s'", umaildir));
	}


	if (rp_isbad (loc_opendir (umaildir, rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("slocal/Unable to open directory '%s'", umaildir));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_FOPN;
	}


	if (rp_isbad (loc_closedir (rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			 ("slocal/Unable to close directory '%s'", umaildir));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_FOPN;
	}

	getfpath (mychan -> ch_out_info, umaildir, ualert_file);
	(void) strcat (ualert_file, alert_file);

	if (rp_isbad (loc_open (ualert_file, "r", 1, rp))) {
		alert_exists=FALSE;
		loc_close (rp);
	}
	else 
		alert_exists=TRUE;

	(void) sprintf (uinboxdir, "%s/%s", umaildir, inboxdir);

	/*
	Try to open uinboxdir - if unable then create in user's
	directory and log it. Retry open - fail if cannot open.
	*/

	if (rp_isbad (loc_cd (uinboxdir, rp))) {
		if (rp_isbad (loc_mkdir (uinboxdir, 0700, rp))) {

			PP_LOG (LLOG_EXCEPTIONS,
			 ("slocal/Unable to create user inbox directory '%s'", 
			uinboxdir));

			delivery_set (ap -> ad_no, 
				int_Qmgr_status_messageFailure);
			return RP_FOPN;
		}
		else {
			PP_LOG (LLOG_EXCEPTIONS,
				 ("slocal/Created user inbox directory '%s'", 
				uinboxdir));
		}
	}


	if (rp_isbad (loc_opendir (uinboxdir, rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			("slocal/can't open user inbox '%s'", uinboxdir));
			return RP_FOPN;
	}

	/* Generate new (temp) dir, then copy all body part files into it. */
	(void) sprintf (utempdir, "%s/%s", uinboxdir, tempdir);

	if (rp_isbad (loc_mkdir (utempdir, 0700, rp))) {

		PP_LOG (LLOG_EXCEPTIONS,
		 ("slocal/can't create user mail temporary directory '%s'", 
		utempdir));

		if (rp_isbad (loc_isdir (utempdir, rp))) {
			delivery_set (ap -> ad_no, 
				int_Qmgr_status_messageFailure);
			return RP_FOPN;
		}
		else {
			PP_LOG (LLOG_EXCEPTIONS, 
				 ("slocal/using existing temporary directory %s",
				utempdir));
		}
	}
	PP_TRACE (("delivering to mail temporary directory %s", utempdir));

	if (rp_isbad (retval = msg_rinit (formatdir))) {
		PP_LOG (LLOG_EXCEPTIONS, 
			 ("slocal/msg_rinit can't init %s", formatdir));

		loc_closedir (rp);      /* close directory */
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_MECH;
	}


	PP_TRACE (("slocal msg_rinit formatdir %s", formatdir));

	/* --- *** ---
	 *      Strategy for decoding filenames from queue :
	 *      look for first occurence of `base' in name - then skip 
	 *      to the next `/' - all that follows is the structure
	 *      of the message itself.
	 *      basename() returns the path/filename component
	 *      following the `/'
	--- *** --- */

	if (dump_mtsparams (&Qstruct, utempdir, ap) != OK) {
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_FIO;
	}

	while ((retval = msg_rfile (fname)) == RP_OK) {
		PP_TRACE (("slocal msg_rfile returned %s", fname));

		if ((p = basename (fname)) == NULLCP ) {

			PP_LOG (LLOG_EXCEPTIONS,
			 ("slocal/basename() cannot extract real message"));

			delivery_set (ap -> ad_no, 
				int_Qmgr_status_messageFailure);
			return RP_FOPN;
		}

		q = p;

		while (q = index (q,'/')) { /* keep making directories */
			strcpy (new_directory,utempdir);
			strcat (new_directory,"/");
			strncat (new_directory,p,q-p);

			if (rp_isbad (loc_mkdir (new_directory, 0700, rp))) {

				if (rp_isbad (loc_isdir (new_directory, rp))) {

					PP_LOG (LLOG_EXCEPTIONS,
					 ("slocal/'%s' is not a directory", 
					new_directory));

					delivery_set (ap -> ad_no, 
						int_Qmgr_status_messageFailure);
					return RP_FOPN;
				}
				else {
					PP_DBG (("slocal/using existing message directory %s",new_directory));
				}
			}
			else {
				PP_TRACE (("slocal/created message directory %s", new_directory));
			}
			q++;    /* step past current '/' */
		}

		(void) sprintf (fnameout,"%s/%s",utempdir,p);

		if ((i = slocal_copy (fname, fnameout)) != RP_OK) {
			PP_LOG (LLOG_EXCEPTIONS,
				 ("slocal/slocal_copy from %s to %s failed - error %d", fname, fnameout,i));
			delivery_set (ap -> ad_no, 
				int_Qmgr_status_messageFailure);
			return RP_MECH;
		}
	}

	retval = msg_rend();
	if (retval != RP_OK) {
		delivery_set (ap -> ad_no, int_Qmgr_status_mtaFailure);
		return retval;
	}

	if (rp_isbad (loc_nextmess (uinboxdir, rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			 ("slocal/cannot determine next message number for %s", uinboxdir));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_MECH;
	}

	PP_DBG (("slocal/next message number in %s = %s",
		uinboxdir,rp -> rp_line));

	next_msgno = atoi (rp ->rp_line);
	(void) sprintf (newname, "%s/%d\0", uinboxdir, next_msgno);

	if (rp_isbad (loc_closedir (rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			 ("slocal/can't close directory '%s'", uinboxdir));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_FOPN;
	}

	PP_TRACE (("slocal/renaming temp. dir. %s to %s", utempdir,newname));
	if (rp_isbad (loc_move (utempdir, newname, rp))) {
		PP_LOG (LLOG_EXCEPTIONS,
			 ("slocal/can't rename user mail temporary directory %s to %s", utempdir,newname));
		delivery_set (ap -> ad_no, int_Qmgr_status_messageFailure);
		return RP_MECH;
	}

	PP_NOTICE ((">>> slocal:  Message delivered to %s", user));

	if (ap -> ad_usrreq == AD_USR_CONFIRM ||
	    ap -> ad_mtarreq == AD_MTA_CONFIRM ||
	    ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
		set_1dr (&Qstruct, ap -> ad_no, this_msg,
			 DRR_NO_REASON,
			 -1, NULLCP);
		if (!rp_isbad (wr_q2dr (&Qstruct, this_msg))) {
			PP_NOTICE ((">>> slocal:  Setting positive DR"));
			delivery_set (ap -> ad_no,
				      (firstSuccessDR == TRUE) ?
				      int_Qmgr_status_positiveDR :
				      int_Qmgr_status_successSharedDR);
			firstSuccessDR = FALSE;
		}

	}
	else {
		retval = wr_ad_status (ap, AD_STAT_DONE);
		(void) wr_stat (ap, &Qstruct, this_msg, Qstruct.msgsize);
		delivery_set (ap -> ad_no,
			      rp_isbad(retval) ?
			      int_Qmgr_status_messageFailure :
			      int_Qmgr_status_success);
	}

	if (alert_exists) {
		(void) sprintf (s_string,"%s %s %d %s &\n",
			ualert_file, loc -> username, 
			next_msgno, alert_message);

		PP_TRACE (("slocal/calling system with '%s'", s_string));

		if (sysreply = loc_system (s_string, rp)) {
			PP_LOG (LLOG_EXCEPTIONS,
				 ("slocal/system() call returns value %d", 
				sysreply));
		}
	}


	/* --- free locations --- */
	if (loc) {
		(void) loc_end();
		free_loc_user (loc);
	}
	if (user)
		free (user);


	return retval;
}


/* --- open fname, copy to fnameout in smailbox --- */

static slocal_copy (fname, fnameout)   
char    *fname;
char    *fnameout;
{
	FILE    *ifp;
	char    buf[BUFSIZ];
	int     n, retval = RP_OK;
	RP_Buf  rps, *rp = &rps;

	PP_TRACE (("slocal_copy (%s, fp)", fname));

	if ((ifp = fopen (fname,"r")) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("slocal/can't read file '%s'", fname));
		return (RP_FOPN);
	}

	if (rp_isbad (loc_open (fnameout, "w", 1, rp))) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("slocal/can't open file '%s'", fnameout));
		return (RP_FOPN);
	}

	while ((n = fread (buf, sizeof (char), sizeof (buf), ifp)) > 0)
		if (loc_write (buf, n, rp) != RP_OK) {
			PP_LOG (LLOG_EXCEPTIONS, 
				("slocal/can't write to file %s [%s]", 
				fnameout, rp ->rp_line));
			retval = rp ->rp_val;
			break;
		}

	(void) fclose (ifp);
	(void) loc_close (rp);
	return (retval);
}


/* --- extract basename from full path --- */
static char  *basename (f)    
char    *f;
{
	char	*ret;
	
	ret = f + strlen(formatdir);
	while (isstr(ret) && *ret == '/')
		ret++;
	return ret;
}


static dump_mtsparams (qp, tmp, ap2)
Q_struct *qp;
char    *tmp;
ADDR    *ap2;
{
	char    buf[BUFSIZ], buf2[BUFSIZ];
	ADDR    *ap;
	FILE    *fp;

	(void) strcpy (buf, "/tmp/mtsXXXXXX");
	mktemp (buf);
	if ((fp = fopen (buf, "w+")) == NULL)
		return NOTOK;
	if (rp_isbad (wr_q (qp, fp))) {
		(void) fclose (fp);
		(void) unlink (buf);
		return NOTOK;
	}
	if (rp_isbad (wr_adr (qp -> Oaddress, fp, AD_ORIGINATOR))) {
		(void) fclose (fp);
		(void) unlink (buf);
		return NOTOK;
	}
	for (ap = qp -> Raddress; ap; ap = ap -> ad_next) {
		if (ap -> ad_no != ap2 -> ad_no)
			ap -> ad_resp = FALSE;
		if (rp_isbad (wr_adr (ap, fp, AD_RECIPIENT))) {
			(void) fclose (fp);
			(void) unlink (buf);
			return NOTOK;
		}
	}
	(void) fflush (fp);
	(void) fclose (fp);


	(void) sprintf (buf2, "%s/MTS-parameters", tmp);
	if (rp_isbad (slocal_copy (buf, buf2))) {
		(void) unlink (buf);
		return NOTOK;
	}
	(void) unlink (buf);
	return OK;
}
