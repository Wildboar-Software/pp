/* c-P2toRfc.c: rfc987(P2toRFC) filter channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/c-P2toRFC.c,v 6.0 1991/12/18 20:20:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/c-P2toRFC.c,v 6.0 1991/12/18 20:20:34 jpo Rel $
 *
 * $Log: c-P2toRFC.c,v $
 * Revision 6.0  1991/12/18  20:20:34  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"
#include "qmgr.h"
#include "q.h"
#include "prm.h"
#include "chan.h"
#include "ap.h"
#include "dr.h"
#include <isode/cmd_srch.h>
#include <sys/stat.h>
#include "sys.file.h"
#include <isode/usr.dirent.h>
#include "tb_bpt88.h"

extern char	*quedfldir;
extern char 	*chndfldir;
extern char	*cont_p2, *hdr_p2_bp, *hdr_p22_bp, *hdr_822_bp, *hdr_ipn_bp;
extern void	rd_end(), sys_init(), or_myinit(), err_abrt(), dsap_init();
extern int	convertresult;
extern CMD_TABLE bptbl_body_parts88[/* x400 88 body_parts */];

CHAN 		*mychan;
char		*this_msg = NULL, *this_chan = NULL;

static struct type_Qmgr_DeliveryStatus *process ();
static int initialise ();
static int filterMsg();
static int doFilter();
static int link_rest();
static int security_check ();
static void dirinit ();
static ADDR *getnthrecip ();
int err_fatal;
int order_pref;
extern int ap_outtype;
extern char	*local_dit;

/*  */
/* main routine */
 
main (argc, argv)
int 	argc;
char	**argv;
{
	sys_init(argv[0]);
	or_myinit();
	quipu_syntaxes();
	dsap_init((char ***) NULL, (int *) NULL);
	local_dit = NULLCP; /* hack to get full DNs */

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

static void do_ch_info_flags(info)
char	*info;
{
	char	*info_copy, *margv[20];
	int	margc, ix;

	if (info == NULLCP) return;

	info_copy = strdup(info);
	if ((margc = sstr2arg(info_copy, 20, margv, " \t")) > 0) {
		for (ix = 0; ix < margc; ix++) {
			if (lexequ (margv[ix], "uk") == 0) {
				order_pref = CH_UK_PREF;
				ap_outtype |= AP_PARSE_BIG;
			} else
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown ch_info flag '%s'",
					margv[ix]));
		}
	}
	free(info_copy);
}

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char *name = NULL;

	name = qb2str(arg);

	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP,
		       ("Chans/rfc987 : Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}
	
	order_pref = CH_USA_PREF;
	ap_outtype = AP_PARSE_SAME;
	if (mychan->ch_out_info != NULLCP) 
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
	int 		rcount, first_failureDR, retval;
	struct type_Qmgr_UserList 	*ix;
	ADDR				*adr;
	char		*error = NULLCP;

	bzero((char *) &que,sizeof(que));
	first_failureDR = TRUE;
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
			("Chans/rfc987 rd_msg err: '%s'",this_msg));
		rd_end();
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Can't read message");
	}

	/* check each recipient for processing */
	for (ix = arg->users; ix; ix = ix->next) {
		if ((adr = getnthrecip(&que,ix->RecipientId->parm)) == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
				("Chans/rfc987 : failed to find recipient %d of msg '%s'",ix->RecipientId->parm, this_msg));

 			delivery_setstate(ix->RecipientId->parm,
					  int_Qmgr_status_messageFailure,
					  "Unable to find specified recipient");
			continue;
		}
	
		switch (chan_acheck(adr, mychan, 1, (char **)NULL)) {
		    case OK:
			if (filterMsg(this_msg,adr,&que, &error) == NOTOK) {
				if (convertresult == NOTOK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Chans/rfc1138 : completely failed to filter msg '%s' for recip '%d' on channel '%s'. Sending a DR",
						this_msg, adr->ad_no, this_chan));
					set_1dr(&que, adr->ad_no, this_msg,
						DRR_CONVERSION_NOT_PERFORMED,
						DRD_CONTENT_SYNTAX_ERROR,
						(error == NULLCP) ? "Unable to parse the p2 header" : error);
					delivery_set(adr->ad_no,
						     (first_failureDR == TRUE) ? int_Qmgr_status_negativeDR : int_Qmgr_status_failureSharedDR);
					first_failureDR = FALSE;
				} else {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Chans/rfc987 : failed to filter msg '%s' for recip '%d' on channel '%s'",this_msg, adr->ad_no, this_chan));
					delivery_setstate(adr->ad_no,
							  int_Qmgr_status_messageFailure,
							  (error == NULLCP) ? "Problems doing 1138 conversion" : error);
				}
			} else {
				/* CHANGE update adr->ad_rcnt in struct and in file */
				adr->ad_rcnt++;
				wr_ad_rcntno(adr,adr->ad_rcnt);
				delivery_set(adr->ad_no,
					     int_Qmgr_status_success);
			}
			break;
		    default:
			break;
		}
		if (error != NULLCP) {
			free(error);
			error = NULLCP;
		}
	}
	if (rp_isbad(retval = wr_q2dr(&que, this_msg))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s wr_q2dr failure '%d'",mychan->ch_name,retval));
		delivery_resetDRs(int_Qmgr_status_messageFailure);
	}
	rd_end();
	q_free (&que);
	prm_free(&prm);
	return deliverystate;
}

/*  */
static int filterMsg (msg,recip,qp, ep)
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
		       ("Chans/rfc987 original directory not found for recipient %d of message '%s'",recip->ad_no, msg));
		*ep = strdup("source directory not found");
		result = NOTOK;
	}

	/* temporary change to get new directory name */
	recip->ad_rcnt++;
	if ((result == OK) 
	    && (qid2dir(msg, recip, FALSE, &newdir) != OK)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/rfc987 couldn't construct new directory for recipient '%d' of message '%s'",recip->ad_no, msg));
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

	if ((result == OK) && (doFilter(origdir,newdir,msg, 
					qp, ep)!= OK))

		result = NOTOK;

	if (origdir != NULL) free(origdir);
	if (newdir != NULL) free(newdir);
	return result;
}

/*  */
char	olddir[MAXPATHLENGTH],
	newdir[MAXPATHLENGTH],
	xtra_822hdrs[MAXPATHLENGTH];

int	*dir_flags = NULL,
	num_dir_flags = 0,
	dirlevel = 0;

#define	INC	5

static void resize_dir_flags()
{
	num_dir_flags += INC;
	if (dir_flags == NULL)
		dir_flags = (int *) calloc((unsigned int)num_dir_flags, sizeof(int));
	else
		dir_flags = (int *) realloc( (char *) dir_flags, 
					    (unsigned) (num_dir_flags * sizeof(int)));
}

static int	is822hdrfile(file)
char		*file;
{
	char	buf[LINESIZE];
	FILE	*fp;

	(void) sprintf(xtra_822hdrs, "%s/%s", olddir, file);
	if ((fp = fopen(xtra_822hdrs,"r")) == NULL)
	{
		PP_SLOG(LLOG_EXCEPTIONS, xtra_822hdrs,
			("Can't open file"));
		return FALSE;
	}

	if (fgets (buf, LINESIZE - 1, fp) == NULL)
	{
		PP_DBG (("Null file '%s'",xtra_822hdrs));
		fclose(fp);
		return FALSE;
	}

	buf [strlen(buf) - 1] = '\0'; /* remove \n */
	if (lexnequ (buf, "RFC-822-HEADERS:",
		     strlen("RFC-822-HEADERS:")) != 0
	    && lexnequ (buf, "Comments:",
			strlen("Comments:")) != 0)
	{
		fclose (fp);
		return FALSE;
	}

	fclose(fp);
	return TRUE;
}


static int	is_xtra_822hdrs(entry)
struct dirent	*entry;
{
	if ((strncmp(entry->d_name,"1.",2) == 0)
	    && (is822hdrfile(entry->d_name) == TRUE)) {
		(void) sprintf(xtra_822hdrs, "%s/%s", olddir, entry->d_name);
		return 1;
	}
	return 0;
}

static	char	*get_xtra_822hdrs(dir)
char	*dir;
{
	int	num;
	struct dirent	**namelist;

	num = _scandir(dir, &namelist, is_xtra_822hdrs, NULL);
	if (num == 0) {
		dir_flags[dirlevel] = FALSE;
		return NULLCP;
	} else {
		dir_flags[dirlevel] = TRUE;
		return xtra_822hdrs;
	}
}

extern char	*ia5_bp;

static doFilter (orig, new, msg, qp, ep)
/* filters orig directory through mychan to new directory */
char		*orig,
		*new,
		*msg;
Q_struct	*qp;
char		**ep;
{
	char		hdrp2[MAXPATHLENGTH],
			hdr822[MAXPATHLENGTH], *ipn_body;
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
		/* find p2 header to filter */
		LIST_BPT	*ix = mychan -> ch_hdr_in;
		int	found = FALSE;
		ipn_body = NULLCP;
		
		while (found == FALSE
		       && ix != NULLIST_BPT) {
			sprintf(hdrp2,"%s/%s",orig, ix -> li_name);
			if (stat(hdrp2, &statbuf) == OK)
				found = TRUE;
			else 
				ix = ix -> li_next;
		}

		if (found == FALSE) {
			PP_OPER(NULLCP,
				("Format/P2toRFC : cannot find p2 hdr"));
			(void) sprintf (buf,
					"Cannot find p2 hdr");
			*ep = strdup(buf);
			return NOTOK;
		}

		sprintf(hdr822, "%s/1.%s", newdir, ia5_bp);
		ipn_body = strdup(hdr822);
		
		sprintf(hdr822,"%s/%s",newdir,hdr_822_bp);
		sprintf(olddir,"%s",orig);
		dirlevel = 0;
		resize_dir_flags();

		result = P2toRFC(hdrp2,get_xtra_822hdrs(orig),qp,NULLCP,
				 hdr822,ipn_body,ep);

		if (ipn_body) free(ipn_body);
		if (result != NOTOK) {
			err_fatal = FALSE;
			link_rest(orig, ep);
			if (err_fatal == TRUE)
				result = NOTOK;
		}
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

static int	do_link(entry)
struct dirent	*entry;
{
	struct	stat statbuf;
	struct dirent	**namelist;
	int		num;
	char		oldfullname[MAXPATHLENGTH],
			newfullname[MAXPATHLENGTH],
			buf[BUFSIZ],
			*ix;

	if ((strcmp(entry->d_name,".") == 0)
	    || (strcmp(entry->d_name,"..") == 0))
		return 0;

	if ((dir_flags[dirlevel] == TRUE)
	    && (strncmp(entry->d_name,"1.",2) == 0))
		/* already done so ignore */
		return 0;

	if (strcmp(entry->d_name, hdr_p2_bp) == 0
	    || strcmp(entry->d_name, hdr_p22_bp) == 0
	    || strcmp(entry->d_name, hdr_ipn_bp) == 0
	    || strcmp (entry->d_name, 
		       rcmd_srch (BPT_P2_DLIV_TXT, 
				  bptbl_body_parts88)) == 0)
		/* already dealt with */
		return 0;

	(void) sprintf(oldfullname, "%s/%s",olddir,entry->d_name);

	/* create new filename */

	if (isdigit(*(entry->d_name))
	    && (dir_flags[dirlevel] == TRUE)) {
		/* need to alter number (decrement) */
		ix = index(entry->d_name,'.');
		*ix = '\0';
		num = atoi(entry->d_name);
		*ix = '.';
		sprintf(newfullname,"%s/%d%s",newdir,--num,ix);
	} else
		/* just copy */
		sprintf(newfullname,"%s/%s",newdir,entry->d_name);
	
			
	if ((stat(oldfullname,&statbuf) == OK)
	    && ((statbuf.st_mode & S_IFMT) == S_IFDIR)) {
		/* directory so do scandir on it */
		char		hdrp2[MAXPATHLENGTH],
				hdr822[MAXPATHLENGTH],
				delivery_info[MAXPATHLENGTH],
				*del_info;
		struct stat 	statbuf;
		int		found;
		LIST_BPT	*bix = mychan -> ch_hdr_in;
		
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

		found = FALSE;
		/* deal with p2 hdr in this directory if there */
		while (found == FALSE
		       && bix != NULLIST_BPT) {
			sprintf(hdrp2,"%s/%s",olddir, bix -> li_name);
			if (stat(hdrp2, &statbuf) == OK)
				found = TRUE;
			else
				bix = bix -> li_next;
		}
		
		if (found == FALSE) {
			PP_OPER(NULLCP,
			       ("Format/P2toRFC : cannot find p2 hdr in forwarded msg"));
			linkerror = strdup("cannot find p2 hdr in forwarded msg");
			return 0;
		}

		sprintf(hdr822,"%s/%s",
			newdir, hdr_822_bp);
		sprintf(delivery_info, "%s/%s",
			olddir, 
			rcmd_srch (BPT_P2_DLIV_TXT,
				   bptbl_body_parts88));
		if (stat(delivery_info, &statbuf) == OK)
			del_info = delivery_info;
		else
			del_info = NULLCP;

		if (P2toRFC(hdrp2,get_xtra_822hdrs(olddir),
			    (Q_struct *) NULL, del_info,
			    hdr822, NULLCP, 
			    &linkerror) != OK) {
			err_fatal = TRUE;
			return 0;
		}

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
/* auxilary routines to extr<act from lists */
static ADDR *getnthrecip(que, num)
Q_struct	*que;
int		num;
{
	ADDR *ix = que->Raddress;

	if (num == 0)
		return que->Oaddress;
	while ((ix != NULL) && (ix->ad_no !=  num))
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
