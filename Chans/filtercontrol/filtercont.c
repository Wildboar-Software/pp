/* filtercont.c: filter control channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/filtercontrol/RCS/filtercont.c,v 6.0 1991/12/18 20:10:08 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/filtercontrol/RCS/filtercont.c,v 6.0 1991/12/18 20:10:08 jpo Rel $
 *
 * $Log: filtercont.c,v $
 * Revision 6.0  1991/12/18  20:10:08  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"
#include "expand.h"
#include "qmgr.h"
#include "Qmgr-types.h"
#include "q.h"
#include "prm.h"
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<sys/wait.h>
#include	<signal.h>
#include 	<errno.h>

extern char 	*quedfldir;
extern char	*formdfldir;
extern char	*logdfldir;
extern CHAN 	*ch_nm2struct();
extern char	*expand_dyn ();
extern void	err_abrt(), rd_end(), sys_init(), arg2vstr();
extern int	errno;
CHAN 		*mychan;
char		*this_msg = NULL, *this_chan = NULL;
char		*none_str = "none", *yes_str = "yes", *no_str = "no";

static struct type_Qmgr_DeliveryStatus *process ();
static int processMsg ();
static int doFormat ();
static int is_mychan_file ();
static int in_BPT ();
static int file_create ();
static int do_child_logging ();
static int file_link ();
static int initialise ();
static int security_check ();
static void dirinit ();
static ADDR *getnthrecip ();
int	first_failureDR;
int	value;
char	*error, buf[BUFSIZ];

#ifndef WEXITSTATUS
#define WEXITSTATUS(x) (((union wait *)&(x)) -> w_retcode)
#endif

/*  */
/* main routine */
 
main (argc, argv)
int 	argc;
char	**argv;
{
	sys_init(argv[0]);
	dirinit ();
#ifdef PP_DEBUG
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control(argc,argv,initialise,process, NULLIFP);
	else
#endif
		channel_control (argc, argv, initialise, process, NULLIFP);
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

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char *name;
	
	name = qb2str(arg);

	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP, ("Chan/filtercont : Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}

	(void) signal (SIGCHLD, SIG_DFL);
	/* check if a filter channel */
	if (name != NULL) free(name);
	return OK;
}

/*  */
/* routine to check if allowed to filter this message through this channel */
static int security_check (msg)
struct type_Qmgr_ProcMsg *msg;
{
 	char 		*msg_file = NULL, *msg_chan = NULL;
	int		result;

	result = TRUE;
	msg_file = qb2str (msg->qid);	
	msg_chan = qb2str (msg->channel);

	if ((mychan == NULLCHAN) || (strcmp(msg_chan,mychan->ch_name) != 0)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/fcontrol channel err: '%s'",msg_chan));
		result = FALSE;
	}

	/* free all storage used */
	if (msg_file != NULL) free(msg_file);
	if (msg_chan != NULL) free(msg_chan);
	return result;
}

/*  */
/* routine called to do filter */

Expand expand_macros[] = {
	"sender", NULLCP,
	"822sender", NULLCP,
	"400sender", NULLCP,
	"qid", NULLCP,
	"ua-id", NULLCP,
	"p1-id", NULLCP,
	"recip", NULLCP,
	"822recip", NULLCP,
	"400recip", NULLCP,
	"outtable", NULLCP,
	"outmta", NULLCP,
	"confirm", NULLCP,
	"intable", NULLCP,
	NULLCP, NULLCP};

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

	bzero((char *) &que,sizeof(que));
	bzero((char *) &prm,sizeof(prm));

	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_messageFailure);
	first_failureDR = TRUE;

	if (security_check(arg) != TRUE)
		return deliverystate;

	if (this_msg != NULL) free(this_msg);
	if (this_chan != NULL) free(this_chan);

	this_msg = qb2str(arg->qid);
	this_chan = qb2str(arg->channel);

	PP_LOG(LLOG_NOTICE,
	       ("%s formatting msg '%s'",mychan->ch_name,this_msg));

	if (rp_isbad(rd_msg(this_msg,&prm,&que,&sender,&recips,&rcount))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s rd_msg err: '%s'",mychan->ch_name,this_msg));
		rd_end();
		return delivery_setallstate(int_Qmgr_status_messageFailure,
					    "Can't read message");
	}

	expand_macros[0].expansion = que.Oaddress->ad_value;
	expand_macros[1].expansion = que.Oaddress->ad_r822adr;
	expand_macros[2].expansion = que.Oaddress->ad_r400adr;
	expand_macros[3].expansion = this_msg;
	expand_macros[4].expansion = que.ua_id;
	expand_macros[5].expansion = que.msgid.mpduid_string;
	if (mychan->ch_table != NULLTBL)
		expand_macros[9].expansion = mychan->ch_table->tb_name;
	else
		expand_macros[9].expansion = none_str;

	expand_macros[11].expansion = no_str;
	for (ix = arg->users; ix; ix = ix->next) {
		if ((adr = getnthrecip(&que, ix->RecipientId->parm)) != NULL
		    && adr -> ad_usrreq == AD_USR_CONFIRM) {
			expand_macros[11].expansion = yes_str;
			break;
		}
	}
	if (mychan->ch_in_table != NULLTBL)
		expand_macros[12].expansion = mychan->ch_in_table->tb_name;
	else
		expand_macros[12].expansion = none_str;

			
	if (arg->users == NULL)
		PP_LOG(LLOG_NOTICE,
			("%s : passed a NULL user list for message '%s'",
			mychan->ch_name,
			this_msg));

	/* check each recipient for processing */
	for (ix = arg->users; ix; ix = ix->next) {
		error = NULLCP;
		if ((adr = getnthrecip(&que,ix->RecipientId->parm)) == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s : failed to find recipient %d of msg '%s'",mychan->ch_name,ix->RecipientId->parm, this_msg));
			delivery_setstate(ix->RecipientId->parm, 
					  int_Qmgr_status_messageFailure,
					  "Unable to find specified recipient");
			continue;
		}

		switch(chan_acheck (adr, mychan, 1, (char **)NULL)) {
		    case OK:
			expand_macros[6].expansion = adr -> ad_value;
			expand_macros[7].expansion = adr -> ad_r822adr;
			expand_macros[8].expansion = adr -> ad_r400adr;
			if (adr -> ad_outchan
			    && adr -> ad_outchan -> li_mta)
				expand_macros[10].expansion = adr -> ad_outchan -> li_mta;
			else 
				expand_macros[10].expansion = none_str;
			    
			value = int_Qmgr_status_messageFailure;
			if (processMsg(this_msg,adr) == NOTOK) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("%s : failed to process msg '%s' for recip '%d' on channel '%s'",mychan->ch_name,this_msg, adr->ad_no, this_chan));
				delivery_setstate(adr->ad_no, 
						  value,
						  (error == NULLCP) ? "Problems with filtering" : error);
			} else {
				/* CHANGE update adr->ad_rcnt in struct and in file */
				PP_LOG(LLOG_NOTICE,
					("%s : processed '%s' for recipient %d",mychan->ch_name,this_msg,adr->ad_no));
				adr->ad_rcnt++; 
				wr_ad_rcntno(adr,adr->ad_rcnt);
				delivery_set(adr->ad_no,int_Qmgr_status_success);
			}
			break;
		    default:
			break;
		}
		if (error != NULLCP)
			free(error);
	}
	rd_end();
	q_free(&que);
	prm_free(&prm);
	return deliverystate;
}

/*  */
static int processMsg (msg,recip)
/* returns OK if managed to process msg for recip on mychan */
char	*msg;
ADDR 	*recip;
{
	char 	*origdir = NULL,
	        *newdir = NULL;
	int result = OK;
	struct stat statbuf;

	if (qid2dir(msg, recip, TRUE, &origdir) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s original directory not found for recipient %d of message '%s'",mychan->ch_name,recip->ad_no, msg));
		error = strdup("Unable to find original directory");
		result = NOTOK;
	}
	
	/* temp change so as to get new directory name */
	recip->ad_rcnt++;
	if (result == OK) {
		LIST_RCHAN	*lix;
		int	count;
		for (lix = recip->ad_fmtchan, count = 0;
		     lix != NULLIST_RCHAN;
		     lix = lix->li_next,count++)
			continue;
		if (count < recip->ad_rcnt) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s ran out of reformatter channels !"));
			error = strdup("Ran out of reformatter channels !");
			result = NOTOK;
		}
	}
	if ((result == OK) 
	    && (qid2dir(msg, recip, FALSE, &newdir) != OK)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s couldn't construct new directory for recipient '%d' of message '%s'",mychan->ch_name,recip->ad_no, msg));
		error = strdup ("Unable to construct new directory");
		result = NOTOK;
	}
	recip->ad_rcnt--;

	if ((result == OK) 
	    && (stat(newdir, &statbuf) == OK) 
	    && ((statbuf.st_mode & S_IFMT) == S_IFDIR)) {
		/* directory already exists and so reformatting already done */
		PP_LOG(LLOG_NOTICE,
			("%s : work already done for recipient %d of message '%s'",mychan->ch_name,recip->ad_no, msg));
		if (origdir != NULL) free(origdir);
		if (newdir != NULL) free(newdir);
		return OK;
	}

	if ((result == OK) && (doFormat(origdir,newdir,msg) != OK))
		result = NOTOK;

	if (origdir != NULL) free(origdir);
	if (newdir != NULL) free(newdir);
	return result;
}

/*  */
static int doFormat (orig, new, msg)
/* reformats orig directory contents to new directory through mychan */
char	*orig,	/* original directory */
	*new,	/* new directory */
	*msg;	/* message */
{
	char	tmpdir[MAXPATHLENGTH],
		newfile[MAXPATHLENGTH],
		*stripped_file,
		*origfile,
		*subdir,
	        file[MAXPATHLENGTH];
	int	result = OK;	
	struct stat statbuf;
	(void) sprintf(tmpdir, "%s/tmp.%s", 
		       msg, mychan->ch_name);
	
	if (stat(tmpdir, &statbuf) == OK) {
		(void) recrm (tmpdir);
		if (rmdir (tmpdir) == NOTOK) {
			PP_SLOG (LLOG_EXCEPTIONS, tmpdir,
				 (error = strdup("can't remove directory")));
			result = NOTOK;
		}
	}

	if (result == OK && mkdir(tmpdir, 0777) != OK) {
		PP_SLOG(LLOG_EXCEPTIONS, tmpdir,
		       ("Can't make directory"));
		if (errno == ENOSPC)
			value = int_Qmgr_status_mtaFailure;
		(void) sprintf (buf,
				"Unable to make temporary directory '%s'",
				tmpdir);
		error = strdup(buf);
		result = NOTOK;
	}

	if (result == OK) {
		msg_rinit(orig);	
		if (msg_rfile(file) != RP_OK) {
			PP_LOG(LLOG_EXCEPTIONS,
				("%s empty or non exsistant dir %s",mychan->ch_name,orig));
			(void) sprintf (buf,
					"Original directory '%s' is empty",
					orig);
			error = strdup (buf);
			result = NOTOK;
		} else {
			do {
				stripped_file = file + strlen(orig) + 1;
				(void) sprintf(newfile,"%s/%s",tmpdir, stripped_file);
				stripped_file = newfile + strlen(tmpdir) +1;
				/* stripped file now local to tmpdir */
				origfile = rindex(file,'/');
				*origfile = '\0';
				while ((result == OK) 
					&& ((subdir = index(stripped_file,'/')) != NULL)){
					/* check intervening directories */
					*subdir = '\0';
					if (!((stat(newfile, &statbuf) == OK)
						&& ((statbuf.st_mode & S_IFMT) == S_IFDIR))) {
						/* sub dir not there so mkdir */
						if (mkdir(newfile, 0777) != OK) {
							PP_SLOG(LLOG_EXCEPTIONS,
								newfile,
								("Can't make directory"));
							(void) sprintf (buf,
									"Failed to make directory '%s'",
									newfile);
							error = strdup(buf);
							result = NOTOK;
						} else
							PP_TRACE(("%s : made directory %s",mychan->ch_name,newfile));
							
						}
					*subdir = '/';
					stripped_file = subdir+1;
				}
				if ( result == OK) {	
					
					subdir = rindex(newfile,'/');
					*subdir = '\0';
					if (is_mychan_file(stripped_file) == TRUE)
						result = file_create(file,newfile,stripped_file);
					else
						result = file_link(file,newfile,stripped_file);
					*subdir = '/';
			}
			*origfile = '/';
			} while ((result == OK) && (msg_rfile(file) == RP_OK)) ;

		}
		msg_rend();

		if ((result == OK) && (rename(tmpdir, new) == -1)) {
			PP_SLOG(LLOG_EXCEPTIONS, "rename",
				("Can't rename directory '%s' to '%s'",
				tmpdir, new));
			(void) sprintf (buf,
					"Failed to rename %s to %s",
					tmpdir, new);
			error = strdup (buf);
			result = NOTOK;
		}
	}

	return result;
}

static int is_mychan_file(name)
char	*name;
{
	char	*suffix;
	if (in_BPT(name, mychan->ch_hdr_in) == TRUE)
		return TRUE;
	if ((suffix = index(name,'.')) == NULL)
		return FALSE;
	return in_BPT(++suffix,mychan->ch_bpt_in);
}

static int in_BPT(suffix, list)
char 		*suffix;
LIST_BPT	*list;
{
	LIST_BPT *ix = list;

	while ((ix != NULL) 
	       && (strcmp(suffix,ix->li_name) != 0))
		ix = ix->li_next;
	return (ix == NULL) ? FALSE : TRUE;
}

/*  */
static int file_create (orig, tmp, file)
char	*orig,	/* original message directory */
	*tmp, /* where to put new file */
	*file;	/* file to format */
{
	char	filein[MAXPATHLENGTH], /* input file */
	fileout[MAXPATHLENGTH],	/* output file */
	*name = NULL,		/* base name of file */
	*program = NULL,	/* full name of exec'd program */
	*ix,
	*info_copy,
	*expanded = NULL,
	*margv[100];
	int	fd_in,		/* input file id */
	fd_out,			/* output file id */
	fd_sterr[2],		/* stderr pipe */
	pid,			/* child process id */
	pgmresult,
	margc,
	i, total_len,
	result = OK;
	struct stat statbuf;
	
	(void) sprintf(filein, "%s/%s",orig,file);

	if (mychan->ch_hdr_out && mychan->ch_hdr_out->li_name)
		(void) sprintf(fileout, "%s/%s",tmp,
			       mychan->ch_hdr_out->li_name);
	else {
		name = strdup(file);
		ix = index(name,'.');
		*ix = '\0';

		(void) sprintf(fileout, "%s/%s.%s",
			       tmp,name,mychan->ch_bpt_out->li_name);
	}

	if ((fd_in = open(filein, O_RDONLY, 0666)) == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, filein,
		       ("Can't open input file"));
		(void) sprintf(buf,
			       "Unable to open input file '%s'",
			       filein);
		error = strdup(buf);
		return NOTOK;
	}
	if ((fd_out = open(fileout, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, fileout,
		       ("Can't open output file"));
		(void) sprintf(buf,
			       "Unable to open output file '%s'",
			       fileout);
		error = strdup(buf);
		return NOTOK;
	}
	if (pipe(fd_sterr) != 0) {
		PP_SLOG(LLOG_EXCEPTIONS, "pipe",
		       ("Can't pipe standard err"));
		error = strdup("Failed to pipe standard error");
		return NOTOK;
	}
		       
	/* get program name */
	if (mychan->ch_out_info == NULL) {
		PP_OPER(NULLCP,
			("%s cannot find channel info",mychan->ch_name));
		value = int_Qmgr_status_mtaFailure;
		error = strdup("No channel info associated with channel");
		return NOTOK;
	}
	info_copy = strdup (mychan -> ch_out_info);
	if ((margc = sstr2arg(info_copy, 100, margv, " \t")) < 1) {
		PP_OPER(NULLCP,
			("%s : no arguments in info string",
			 mychan->ch_name));
		value = int_Qmgr_status_mtaFailure;
		(void) sprintf(buf,
			       "No arguments in info string '%s'",
			       mychan -> ch_out_info);
		error = strdup(buf);
		return NOTOK;
	}
	if (margv[0][0] == '/')
		/* given full path name */
		program = strdup(margv[0]);
	else {
		program = (char *) malloc((unsigned int)
					  (strlen(formdfldir) + 1 +
					   strlen(margv[0]) + 1));
		(void) sprintf(program,"%s/%s",formdfldir,margv[0]);
	}
	
	if (stat(program, &statbuf) != OK) {
		PP_OPER(NULLCP,
			("%s : missing filter '%s'",mychan -> ch_name, program));
		value = int_Qmgr_status_mtaFailure;
		(void) sprintf (buf,
				"Missing filter '%s'",
				program);
		error = strdup(buf);
		return NOTOK;
	}

	total_len = 0;
	for (i = 0; i < margc; i++) {
		if (index (margv[i], '$')) 
			margv[i] = expand_dyn (margv[i],
					   expand_macros);
		else
			margv[i] = strdup (margv[i]);
		total_len += strlen(margv[i]) + 3;
		/* +3 for intervening space and possible quotes */
	}
	total_len += 10;
	/* just in case */

	expanded = malloc((unsigned)(total_len * sizeof(char)));
	arg2vstr (0, total_len, expanded, margv);
	if ((ix = index (expanded, ' ')) == NULLCP)
		ix = expanded;
	PP_NOTICE (("%s %s < %s > %s", program, ix,
		    filein, fileout));
		
#ifdef BSD42
	if ((pid = vfork ()) == 0) {
#else
	if ((pid = tryfork()) == 0) {
#endif
		/* in child so redirect in- and out-put */
		(void) dup2 (fd_in, 0);
		(void) dup2 (fd_out, 1);
		(void) dup2 (fd_sterr[1], 2);
		(void) close(fd_in);
		(void) close(fd_out);
		(void) close(fd_sterr[0]);
		(void) close(fd_sterr[1]);

		(void) execv(program,margv);
		_exit (1);
	} else if (pid == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, this_msg,
		       ("tryfork failed"));
		error = strdup("Tryfork tailed");
		result = NOTOK;
	} else {
// #ifdef SVR4
		int wst;
// #else
// 		union wait wst;
// #endif

		while ((pgmresult = wait(&wst)) != pid && pgmresult != -1)
			PP_TRACE(("process %d returned", pgmresult));

		if ((pgmresult == pid) && !WIFSIGNALED(wst) &&
		     (WEXITSTATUS(wst) == 0))
			result = OK;
		else 
			result = NOTOK;
	}
	(void) close(fd_sterr[1]);
	do_child_logging(fd_sterr[0]);
 	(void) close(fd_in);
	(void) close(fd_out);
	if (program != NULL) free(program);

	for (i = 0; i < margc; i++) 
		free(margv[i]);
	if (expanded) free(expanded);
	free (info_copy);

	return result;
}

static int do_child_logging(ifd)
int	ifd;
{
	/* log stuff child puts out on stderr */
	char	line[LINESIZE];
	int	len;
	FILE	*fd = fdopen(ifd, "r");

	while (fgets(line, LINESIZE, fd) != NULL) {
		len = strlen(line);
		if (line[len-1] == '\n')
			line[len-1] = '\0';
		PP_LOG(LLOG_NOTICE,
		       ("%s : %s",mychan->ch_name,line));
	}
	(void) fclose(fd);
}

static int file_link(orig,tmp,file)
char	*orig,	/* original message directory */
	*tmp,	/* new temporary directory */
	*file;	/* file to link across */
{
	char	old[MAXPATHLENGTH],	/* old file */
	        new[MAXPATHLENGTH];	/* new link */
	struct stat statbuf;
	int	result = OK;

	(void) sprintf(old, "%s/%s",orig,file);

	(void) sprintf(new, "%s/%s",tmp,file);

	if ((stat(old, &statbuf) == OK)
	    && (stat(new, &statbuf) != OK)
	    && (link(old, new) != -1)) {
		result = OK;
	} else {
		(void) sprintf(buf,
			       "Failed to link '%s' to '%s'",
			       new, old);
		error = strdup(buf);
		result = NOTOK;
	}

	return result;
}
		
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

