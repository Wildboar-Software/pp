/* trash_channel.c: message trash collection channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/trashman/RCS/trash.c,v 6.0 1991/12/18 20:12:57 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/trashman/RCS/trash.c,v 6.0 1991/12/18 20:12:57 jpo Rel $
 *
 * $Log: trash.c,v $
 * Revision 6.0  1991/12/18  20:12:57  jpo
 * Release 6.0
 *
 */



#include        <sys/param.h>
#include        "head.h"
#include        "qmgr.h"
#include        "Qmgr-types.h"
#include        "q.h"
#include        "prm.h"
#include        <sys/stat.h>
#include 	"sys.file.h"
#include        <isode/usr.dirent.h>


extern char     *quedfldir, *aquefile;
extern char     *chndfldir;
extern CHAN     *ch_nm2struct();
extern void	rd_end(), sys_init(), err_abrt();
extern time_t	time();
CHAN            *mychan;

static struct type_Qmgr_DeliveryStatus *process ();
static int initialise ();
static void dirinit ();
static int okayToTrash();
static void rec_rmdir();

int testing = FALSE;

#ifndef S_ISDIR
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif

/*  */
/* main routine */

main (argc, argv)
int     argc;
char    **argv;
{
	sys_init(argv[0]);
	dirinit ();
#ifdef PP_DEBUG
	if (argc > 2 && (strcmp(argv[2],"test") == 0))
		testing = TRUE;
	if (argc>1 && (strcmp(argv[1],"debug") == 0))
		debug_channel_control(argc,argv,initialise,process,NULLIFP);
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
/* routine to initialise channel */
/* also does all the work */

char    fullname[MAXPATHLEN];
time_t  interval, 
	current;

#define DEFAULT_INTERVAL        (60*60*24*3)	/* 3 days for normal files */

static int my_unlink(file)
char	*file;
{
	if (testing == TRUE) {
		fprintf(stdout, "Trash channel would unlink file '%s'", file);
		return 0;
	} else
		return unlink(file);
}

static void my_recrm(name)
char	*name;
{
	if (testing == TRUE) 
		fprintf(stdout, "Trash channel would recrm directory '%s'",
			name);
	else {
		if (chdir(name) == 0) {
			(void) recrm(".");
			dirinit();
			if (rmdir(name) == NOTOK) 
				PP_SLOG(LLOG_EXCEPTIONS,name,
					("unable to remove directory"));
		}
	}
}

static void rmNonMsg (name)
char	*name;
{
	struct stat statbuf;
	if (stat (name, &statbuf) == NOTOK) 
		PP_SLOG(LLOG_EXCEPTIONS, name,
			("unable to stat file"));
	else if (current - statbuf.st_mtime > interval) {
		PP_NOTICE(("Removing non message entry '%s'", name));
		if (S_ISDIR(statbuf.st_mode)) {
			my_recrm(name);
		} else
			if (my_unlink(name) == NOTOK)
				PP_SLOG(LLOG_EXCEPTIONS, name,
					("Can't unlink file"));
	}
}

static time_t parsetime(s)
char	*s;
{
	int	n;
	if (s == NULLCP || *s == NULL) return 0;
	while(*s != NULL && isspace(*s)) s++;
	n = 0;
	while (*s != NULL && isdigit(*s)) {
		n = n * 10 + *s - '0';
		s++;
	}
	while (*s != NULL && isspace(*s)) s++;
	if (*s != NULL && isalpha(*s)) {
		switch (*s) {
		    case 's':
		    case 'S': 
			break;
		    case 'm': 
		    case 'M':
			n *= 60; 
			break;
		    case 'h':
		    case 'H':
			n *= 3600; 
			break;
		    case 'd': 
		    case 'D':
			n *= 86400; 
			break;
		    case 'w':
		    case 'W':
			n *= 604800;
			break;
		    default:
			break;
		}
		return n + parsetime(s+1);
	}
	else return n + parsetime(s);
}

static int initialise (arg)
struct type_Qmgr_Channel *arg;
{
	char *name;
	int     i, noEntries, maxnumb;
	char	**msgs;
	struct stat statbuf;

	name = qb2str(arg);

	if ((mychan = ch_nm2struct(name)) == NULLCHAN) {
		PP_OPER(NULLCP,
			("Channel '%s' not known",name));
		if (name != NULL) free(name);
		return NOTOK;
	}

	if (name != NULL) free(name);
	if (mychan->ch_chan_type != CH_DEBRIS) {
		PP_OPER(NULLCP,
			("Channel '%s' incorrect type [expecting type=debris (%d)]",
			 mychan->ch_name, mychan->ch_chan_type));
		return NOTOK;
	}

	if (mychan->ch_out_info != NULLCP)
		interval = parsetime(mychan->ch_out_info);
	else if (mychan->ch_in_info != NULLCP)
		interval = parsetime(mychan->ch_in_info);
	else
		interval = DEFAULT_INTERVAL;

	(void) time(&current);

	/* scan the directory */
	
	msgs = NULLVP;
	noEntries = maxnumb = 0;

	hier_scanQ (quedfldir, NULLCP,
		    &noEntries, &msgs, 
		    &maxnumb, rmNonMsg);

	for (i = 0; i < noEntries; i++) {
		if (stat(msgs[i], &statbuf) != OK) {
			PP_SLOG (LLOG_EXCEPTIONS, msgs[i],
				 ("Can't stat "));
			continue; /* just have got zapped? */
		}
		if ((current - statbuf.st_mtime) < interval) {
			if (testing == TRUE)
				printf ("Ignore %s\n", msgs[i]);
			continue; /* not yet ... */
		}
		if (!S_ISDIR(statbuf.st_mode)) {
			PP_OPER(NULLCP,
				("Msg entry for '%s' is not a directory",
				msgs[i]));
			continue;
		}
		if (okayToTrash(msgs[i]) == OK) 
			/* remove */
			my_recrm(msgs[i]);
	}
		
	if (msgs)
		free_hier(msgs, noEntries);
	
	return OK;
}

static int okayToTrash(name) 
char    *name;
{
	struct stat statbuf;
	struct prm_vars prm;
	Q_struct        que;
	ADDR            *sender = NULL;
	ADDR            *recips = NULL;
	int             rcount;
	ADDR            *ix = NULL;

	bzero ((char *)&prm, sizeof(prm));
	bzero ((char *)&que, sizeof(que));

	/* test time for addr file */
	(void) sprintf(fullname, "%s/%s", name, aquefile);
	if (stat(fullname,&statbuf) == OK) {
		if ((current - statbuf.st_mtime) > interval) {
			/* check to see if all recipients are done */

			if (rp_isbad(rd_msg_file(name,&prm,&que,
						 &sender,&recips,&rcount,
						 RDMSG_RDONLY))) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("rd_msg failure: '%s'", name));
				/* don't delete */
				return NOTOK;
			}
			rd_end();

			/* now the recips */
			ix = que.Raddress;
			while ((ix != NULL) && (ix->ad_status == AD_STAT_DONE))
				ix = ix->ad_next;
			if (ix != NULL) 
				/* don't trash as some work needs to be done */
				return NOTOK;

			q_free(&que);
			prm_free(&prm);
		} else
			return NOTOK;
	}
	return OK;
}

/*  */
/* all work done in initialise */
/* process is just a dummy routine */

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	delivery_init(arg->users);
	delivery_setall(int_Qmgr_status_success);
	return deliverystate;
}
