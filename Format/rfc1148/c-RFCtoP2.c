/* c-P2toRfc.c: rfc987(P2toRFC) filter channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/c-RFCtoP2.c,v 6.0 1991/12/18 20:20:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/c-RFCtoP2.c,v 6.0 1991/12/18 20:20:34 jpo Rel $
 *
 * $Log: c-RFCtoP2.c,v $
 * Revision 6.0  1991/12/18  20:20:34  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "head.h"
#include "qmgr.h"
#include "q.h"
#include "prm.h"
#include "chan.h"
#include <isode/cmd_srch.h>
#include <sys/stat.h>
#include "sys.file.h"
#include <isode/usr.dirent.h>


extern char	*quedfldir;
extern char 	*chndfldir;
extern char	*cont_p2, *hdr_822_bp, *hdr_p2_bp, *hdr_p22_bp;
extern void	rd_end(), sys_init(), or_myinit(), err_abrt();

CHAN 		*mychan;
char		*this_msg, *this_chan;
int		err_fatal;

static struct type_Qmgr_DeliveryStatus *process ();
static int initialise ();
static int security_check ();
static int filterMsg();
static int link_rest();
static int x40084;
static int doFilter();
static void dirinit ();
static ADDR *getnthrecip ();

/*  */
/* main routine */
 
main (argc, argv)
int 	argc;
char	**argv;
{
	sys_init(argv[0]);
	or_myinit();
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control(argc,argv,initialise,process,NULLIFP);
	else
#endif
		channel_control (argc, argv, initialise, process,NULLIFP);
}

/*  */
/* routine to move to correct place in file system */

static void dirinit ()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, " Unable to change directory to '%s'",
			  quedfldir);
}

/*  */
/* channel initialise routine */

extern int real987;

static void do_ch_info_flags(info)
char	*info;
{
	char	*info_copy, *margv[20];
	int	margc, ix;

	if (info == NULLCP) return;

	info_copy = strdup(info);
	if ((margc = sstr2arg(info_copy, 20, margv, " \t")) > 0) {
		for (ix = 0; ix < margc; ix++) {
			if (lexequ(margv[ix], "rfc987") == 0)
				real987 = OK;
			else
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown ch_out_info flag '%s'",
					margv[ix]));
		}
	}
	free(info_copy);
}

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char *name;

	name = qb2str(arg);

	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP,
			("Chans/rfc987 : Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}

	if (mychan->ch_out_info) 
		do_ch_info_flags(mychan->ch_out_info);

	/* check if a rfc987 channel */
	if (name != NULL) free(name);

	ap_use_percent();
	ap_norm_all_domains();

	return OK;
}

/*  */
/* routine to check if allowed to rfc987 filter this message */
static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
	char *msg_file = NULL, *msg_chan = NULL;
	int result;

	result = TRUE;
	msg_file = qb2str (msg->qid);
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp(msg_chan,mychan->ch_name) !=0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/rfc987 channel err: '%s'",msg_chan));
		result = FALSE;
	}
	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	return result;
}

/*  */
/* routine called to do rfc987 filter */

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars	prm;
	Q_struct	que;
	ADDR		*sender = NULL;
	ADDR		*recips = NULL;
	int 		rcount;
	struct type_Qmgr_UserList 	*ix;
	ADDR				*adr;
	char		*error = NULLCP;

	bzero((char *) &que,sizeof(que));
	
	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_messageFailure);
	
	if (security_check(arg) != TRUE)
		return deliverystate;

	if (this_msg != NULL) free(this_msg);
	if (this_chan != NULL) free(this_chan);

	this_msg = qb2str(arg->qid);
	this_chan = qb2str(arg->channel);

	PP_LOG(LLOG_NOTICE,
	       ("Chans/rfc987 filtering msg '%s' through '%s'",this_msg, this_chan));

	if (rp_isbad(rd_msg(this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Format/RFCtoP2 rd_msg err: '%s'",this_msg));
		rd_end();
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Can't read message");
	}

	/* check each recipient for processing */
	for (ix = arg->users; ix; ix = ix->next) {
		if ((adr = getnthrecip(&que,ix->RecipientId->parm)) == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
				("Format/RFCtoP2 : failed to find recipient %d of msg '%s'",ix->RecipientId->parm, this_msg));

 			delivery_setstate(ix->RecipientId->parm,
					  int_Qmgr_status_messageFailure,
					  "Unable to find specified recipient");
			continue;
		}
	
		switch (chan_acheck(adr, mychan, 1, (char **) NULL)) {
		    case OK:
			if (filterMsg(this_msg,adr,&que, &error) == NOTOK) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Format/RFCtoP2 : failed to filter msg '%s' for recip '%d' on channel '%s'",this_msg, adr->ad_no, this_chan));
				delivery_setstate(adr->ad_no,
						  int_Qmgr_status_messageFailure,
						  (error == NULLCP) ? "Failed to do 1138 conversion (RFC -> P2)" : error);
			} else {
				/* CHANGE update adr->ad_rcnt in struct and in file */
				adr->ad_rcnt++;
				wr_ad_rcntno(adr,adr->ad_rcnt);
				delivery_set(adr->ad_no,int_Qmgr_status_success);
			}
		    default:
			break;
		}
		if (error != NULLCP) {
			free(error);
			error = NULLCP;
		}
	}
	rd_end();
	q_free (&que);
	prm_free(&prm);
	return deliverystate;
}

/*  */
static int filterMsg (msg,recip,qp,ep)
/* return OK if managed to filter msg through mychan for recip */
char		*msg;
ADDR		*recip;
Q_struct	*qp;
char		**ep;
{
	char	*origdir = NULL,
	        *newdir = NULL;
	int	result = OK;
	struct stat statbuf;

	if (qid2dir(msg, recip, TRUE, &origdir) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Format/RFCtoP2 original directory not found for recipient %d of message '%s'",recip->ad_no, msg));
		*ep = strdup("source directory not found");
		result = NOTOK;
	}

	/* temporary change to get new directory name */
	recip->ad_rcnt++;
	if ((result == OK) 
	    && (qid2dir(msg, recip, FALSE, &newdir) != OK)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Format/RFCtoP2 couldn't construct new directory for recipient '%d' of message '%s'",recip->ad_no, msg));
		*ep = strdup("Unable to construct destination directory");
		result = NOTOK;
	}
	recip->ad_rcnt--;
	
	if ((result == OK)
	    && (stat(newdir, &statbuf) == OK)
	    && ((statbuf.st_mode & S_IFMT) == S_IFDIR)) {
		/* directory already exists and so filter already done */

		if (origdir != NULL) free(origdir);
		if (newdir != NULL) free(newdir);
		return OK;
	}

	{
		/* content not set go on hdr we're creating */
		LIST_BPT	*ix = mychan -> ch_hdr_out;
		x40084 = FALSE;
		while (ix != NULLIST_BPT) {
			if (lexequ(ix -> li_name, hdr_p2_bp) == 0) {
				x40084 = TRUE;
				ix = NULLIST_BPT;
			} else
				ix = ix->li_next;
		}
	}
	if (result == OK && doFilter(origdir,
				     newdir,
				     msg, 
				     qp,
				     ep,
				     x40084) != OK)
		result = NOTOK;

	if (origdir != NULL) free(origdir);
	if (newdir != NULL) free(newdir);
	return result;
}

/*  */
char	olddir[MAXPATHLENGTH],
	newdir[MAXPATHLENGTH];

int	*dir_flags = NULL,
	num_dir_flags = 0,
	dirlevel = 0;

#define	INC	5

static void resize_dir_flags()
{
	num_dir_flags += INC;
	if (dir_flags == NULL)
		dir_flags = (int *) calloc((unsigned int) num_dir_flags, sizeof(int));
	else
		dir_flags = (int *) realloc( (char *) dir_flags, 
					    (unsigned) (num_dir_flags * sizeof(int)));
}

static int	is_822_hdr(entry)
struct dirent	*entry;
{
	if (strncmp(entry->d_name,hdr_822_bp, strlen(hdr_822_bp)) == 0)
		return 1;
	else		
		return 0;
}

static int	get_822_hdr(dir,hdr)
char	*dir;
char	*hdr;
{
	int	num;
	struct dirent	**namelist;

	num = _scandir(dir, &namelist, is_822_hdr, NULL);
	if (num != 1) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Format/RFCtoP2 : cannot find unique 822 hdr file"));
		return NOTOK;
	} 
	sprintf(hdr,"%s/%s",dir,(*namelist)[0].d_name);
	free((char *) namelist);
	return OK;
}
		
static doFilter (orig, new, msg, qp, ep, x40084)
/* filters orig directory through mychan to new directory */
char		*orig,
		*new,
		*msg;
Q_struct	*qp;
char		**ep;
int		x40084;
{
	char		hdrp2[MAXPATHLENGTH],
			hdr822[MAXPATHLENGTH],
			hdr_xtra[MAXPATHLENGTH];
	int		result = OK;
	struct stat 	statbuf;
	char		buf[BUFSIZ];

	(void) sprintf(newdir, "%s/tmp.%s", 
		       msg, mychan->ch_name);
	
	if (stat(newdir, &statbuf) == OK) {
		char	*cmd_line;
		/* exists so remove it */
		cmd_line = malloc((unsigned) (strlen("rm -rf ") +
						   strlen(newdir) + 1));
		sprintf(cmd_line, "rm -rf %s", newdir);
		system(cmd_line);
		if (cmd_line != NULL) free(cmd_line);
	}

	if (mkdir(newdir, 0777) != OK) {
		PP_SLOG(LLOG_EXCEPTIONS, newdir,
			("Can't make directory"));
		(void) sprintf(buf,
			       "Failed to make temp directory '%s'",
			       newdir);
		*ep = strdup(buf);
		result = NOTOK;
	}

	if (result == OK) {
		/* filter from orig to newdir */
		sprintf(hdrp2,"%s/%s",newdir,
			(x40084 == TRUE) ? hdr_p2_bp : hdr_p22_bp);
		if (get_822_hdr(orig,hdr822) != OK) 
			return NOTOK;

		if (stat(hdr822, &statbuf) != OK) {
			PP_OPER(NULLCP,
			       ("Format/RFCtoP2 : cannot find 822 hdr '%s'",hdr822));
			(void) sprintf (buf,
					"Cannot find 822 hdr '%s'",
					hdr822);
			*ep = strdup(buf);
			return NOTOK;
		}
		sprintf(hdr_xtra,"%s/1.ia5",newdir);
		sprintf(olddir,"%s",orig);

		result = RFCtoP2(hdr822,hdrp2,hdr_xtra, ep, x40084);
		resize_dir_flags();
		dirlevel = 0;

		if (stat(hdr_xtra, &statbuf) == OK)
			/* something in header xtra */
			dir_flags[dirlevel] = TRUE;
		else
			dir_flags[dirlevel] = FALSE;

		err_fatal = FALSE;

		link_rest(orig, ep);

		if (err_fatal == TRUE)
			result = NOTOK;
		/* if success rename newdir to new */

		if ((result == OK) && (rename(newdir,new) == -1)) {
			PP_SLOG(LLOG_EXCEPTIONS, "rename",
				("Can't rename directory '%s' to '%s'",
				 newdir, new));
			(void) sprintf (buf,
					"Unable to rename temp dir from '%s' to '%s'",
					newdir, new);
			*ep = strdup(buf);
			result = NOTOK;
		}
	}
	return result;
}

/*  */
/* link rest across from orig to newdir */
/* may have to do some renumbering if 1.ia5 is extra headers */

/* origdir and newdir are set to current dirlevel of directories */
char	*linkerror = NULLCP;

static int	do_link(entry, x40084)
struct dirent	*entry;
int	x40084;
{
	struct	stat statbuf;
	struct dirent	**namelist;
	int		num;
	char		oldfullname[MAXPATHLENGTH],
			newfullname[MAXPATHLENGTH],
			*ix;

	if ((strcmp(entry->d_name,".") == 0)
	    || (strcmp(entry->d_name,"..") == 0))
		return 0;

	if (strncmp(entry->d_name,"hdr.822",7) == 0)
		/* already dealt with */
		return 0;

	(void) sprintf(oldfullname, "%s/%s",olddir,entry->d_name);

	/* create new filename */

	if (isdigit(*(entry->d_name))
	    && (dir_flags[dirlevel] == TRUE)) {
		/* need to alter number (increment) */
		ix = index(entry->d_name,'.');
		*ix = '\0';
		num = atoi(entry->d_name);
		*ix = '.';
		sprintf(newfullname,"%s/%d%s",newdir,++num,ix);
	} else
		/* just copy */
		sprintf(newfullname,"%s/%s",newdir,entry->d_name);
	
			
	if ((stat(oldfullname,&statbuf) == OK)
	    && ((statbuf.st_mode & S_IFMT) == S_IFDIR)) {
		/* directory so do scandir on it */
		char		hdrp2[MAXPATHLENGTH],
				hdr822[MAXPATHLENGTH],
				hdr_xtra[MAXPATHLENGTH];
		struct stat 	statbuf;

		if (mkdir(newfullname, 0777) != OK) {
			PP_SLOG(LLOG_EXCEPTIONS, newfullname,
				("Can't make directory"));
			exit(-1);
		}
		dirlevel++;
		if (dirlevel >= num_dir_flags)
			resize_dir_flags();

		sprintf(olddir, "%s", oldfullname);
		sprintf(newdir, "%s", newfullname);

		/* deal with hdr.822 in this directory if there */
		if ((get_822_hdr(olddir,hdr822) == OK)
			&& (stat(hdr822, &statbuf) == OK)) {
			sprintf(hdrp2,"%s/%s",
				newdir,
				(x40084 == TRUE) ? hdr_p2_bp : hdr_p22_bp);
			sprintf(hdr_xtra,"%s/1.ia5",newdir);

			if (RFCtoP2(hdr822,hdrp2,
				    hdr_xtra,&linkerror,x40084) != OK) {
				err_fatal = TRUE;
				return 0;
			}

			if (stat(hdr_xtra, &statbuf) == OK)
				/* something in header xtra */
				dir_flags[dirlevel] = TRUE;
			else
				dir_flags[dirlevel] = FALSE;

		} else 
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Format/RFCtoP2 : cannot find 822 hdr '%s'",hdr822));
			
		num = _scandir(olddir,&namelist, do_link, NULL);
		/* rewind newdir and olddir */
		ix = rindex(olddir,'/');
		*ix = '\0';
		ix = rindex(newdir,'/');
		*ix = '\0';
		dirlevel--;

	} else
		/* just link */
		if (link(oldfullname, newfullname) == -1) {
			PP_SLOG(LLOG_EXCEPTIONS, "link",
			       ("Can't link from '%s' to '%s'",oldfullname,newfullname));
			exit(-1);
		}
	return 0;
}
	
static int link_rest(orig, ep)
char	*orig;
char	**ep;
{
	int num;
	struct dirent	**namelist;

	dirlevel = 0;
	num = _scandir(orig,&namelist, do_link, NULL);
	if (linkerror != NULLCP) {
		if (*ep == NULLCP) 
			*ep = linkerror;
		else
			free(linkerror);
		linkerror = NULLCP;
	}
}

/*  */
/* auxilary routines to extract from lists */
static ADDR *getnthrecip(que, num)
Q_struct	*que;
int		num;
{
	ADDR *ix = que->Raddress;

	if (num == 0)
		return que->Oaddress;
	while ((ix != NULL) && (ix->ad_no != num))
		ix = ix->ad_next;
	return ix;
}

void    advise (what, fmt, a, b, c, d, e, f, g, h, i, j)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f,
       *g,
       *h,
       *i,
       *j;
{
    (void) fflush (stdout);

    fprintf (stderr, "RFCtoP2 test");
    fprintf (stderr, fmt, a, b, c, d, e, f, g, h, i, j);
    if (what)
	(void) fputc (' ', stderr), perror (what);
    else
	(void) fputc ('\n', stderr);

    (void) fflush (stderr);
}


/* VARARGS 2 */
void    adios (what, fmt, a, b, c, d, e, f, g, h, i, j)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f,
       *g,
       *h,
       *i,
       *j;
{
    advise (what, fmt, a, b, c, d, e, f, g, h, i, j);
    _exit (1);
}
