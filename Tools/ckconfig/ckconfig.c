/* ckconfig.c: management tool to check configuration of PP system */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ckconfig/RCS/ckconfig.c,v 6.0 1991/12/18 20:29:03 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ckconfig/RCS/ckconfig.c,v 6.0 1991/12/18 20:29:03 jpo Rel $
 *
 * $Log: ckconfig.c,v $
 * Revision 6.0  1991/12/18  20:29:03  jpo
 * Release 6.0
 *
 */



#include	"qmgr.h"
#include 	"util.h"
#include	"chan.h"
#include	"table.h"
#include	"list_bpt.h"
#include	"adr.h"
#include	"retcode.h"
#include	<sys/stat.h>
#include	<pwd.h>
#include	<signal.h>
#include <stdarg.h>
#include 	<sys/wait.h>
#include    <isode/isoaddrs.h>

#ifndef WEXITSTATUS
#define WEXITSTATUS(x) (((union wait *)&(x)) -> w_retcode)
#endif

extern struct passwd	*getpwnam(), *getpwuid();
extern void	getfpath(), dsap_init();

typedef enum {
	ok,
	notExist,
	highMode,
	lowMode,
	wrongOwner,
	wrongGroup
	} Retvals;

static Retvals 	check_file();
static void 	check_main_vars();
static void	check_dirs();
static void	check_chans();
static void	check_channel();
static void	check_filter();
static void	check_tables();
static void	check_table_file();
static void	check_table();
static void	check_bps(), check_hdrs();
static void	check_822address();
static int	yesorno();
static void check_hardwired();
static void makedirectory();
static void changemode();
static void changeowner();
static void	check_dsap ();
static void perrstr(char *fmt, ...);
static void	pok();
static void	ptabss();
static void	ptabsd();
static void 	ckerror ();
#define OPEN_MODE	0777
#define CLOSED_MODE	0700

#define	DIR_MODE	0775
#define EXEC_MODE	0755
#define RSEXEC_MODE	0700
#define TABLE_MODE	0644
#define RSTABLE_MODE	0600

int	interactive,
	monitoring,
	verbose;
extern char	*ppversion, *pptailor;
extern void 	sys_init();

void main(argc, argv)
int	argc;
char	**argv;
{
	int	opt;
	Retvals	retval;
	unsigned int	fileMode = 0;
	char	*myname = argv[0], prompt[BUFSIZ], owner[MAXPATHLENGTH];
	extern int optind;
	extern char *optarg;

	interactive = TRUE;
	monitoring = FALSE;
	verbose = FALSE;

	while ((opt = getopt (argc, argv, "fnvt:")) != EOF) {
		switch (opt) {
		    case 'f':
			interactive = FALSE;
			break;

		    case 'n':
			monitoring = TRUE;
			break;

		    case 'v':
			verbose = TRUE;
			break;
		    case 't':
			pptailor = optarg;
			break;

		    default:
			printf("Usage: %s [-n] [-f] [-v] [-t tailorfile]\n",myname);
			exit(1);
			break;
		}
	}
	retval = check_file(pptailor, 0644, 0600, &fileMode, owner);
	switch (retval) {
	    case notExist:
		perrstr("Tailor file does not exist\n\tShould be in %s\n\n",pptailor);
		exit(1);
		break;

	    case lowMode:
		perrstr("The tailor file '%s' has too low a permission mode.\n",
		       pptailor);
		sprintf(prompt, "Change mode of %s from %o to %o",
			pptailor,
			fileMode,
			0600);
		if (yesorno(prompt) == TRUE)
			changemode(pptailor, 0600);
		break;

	    case highMode:
		perrstr("The tailor file '%s' has too high a permission mode.\n",
		       pptailor);
		sprintf(prompt, "Change mode of %s from %o to %o",
			pptailor,
			fileMode,
			0644);
		if (yesorno(prompt) == TRUE)
			changemode(pptailor, 0644);
		break;

	    default:
		break;
	}
	pok("Tailor file",pptailor);
	tai_seterrorhandler (ckerror);
	sys_init (myname);
	(void) signal (SIGPIPE, SIG_DFL);

	check_dsap ();
	check_main_vars();
	check_dirs();
	check_chans();
	check_tables();  
	check_hardwired();
}

static void ckerror (str)
char *str;
{
	fprintf (stderr, "%s", str);
}

/*  */
extern	char	*loc_dom_mta,
		*loc_dom_site,
		*loc_or,
		*x400_mta,
		*postmaster,
		*pplogin,
		*pptsapd_addr,
		*mboxname,
		*qmgr_hostname;
extern int	max_hops, max_loops;

extern LIST_BPT	*bodies_all, *headers_all;
int		ppuid = 0, ppgid = 0;

static void check_main_vars()
/* really just print them out */
{
	struct passwd	*password = NULL;
	LIST_BPT	*ix;
	RP_Buf		rp;
	ADDR		*ad;
	AEI		aei;
	register struct PSAPaddr *pa;
	
	if (str2paddr (pptsapd_addr) == NULLPA) 
		perrstr ("Invalid pptsapd address '%s'\n\n",
			  pptsapd_addr);
	else
		pok("pptsapd address", pptsapd_addr);

	if ((aei = _str2aei (qmgr_hostname, "pp qmgr", 
			     QMGR_CTX_OID, 0, dap_user,
			     dap_passwd)) == NULLAEI)
		if ((aei = _str2aei (qmgr_hostname, "pp-qmgr",
				     QMGR_CTX_OID, 
				     0, dap_user, dap_passwd)) != NULLAEI)
			perrstr ("Old form of AEI \"pp-qmgr\" %s\n",
				  "change to \"pp qmgr\" in isoentites");

	if (aei == NULLAEI)
		perrstr ("Invalid qmgr hostname: %s-pp qmgr unknown application-entity\n\n",
			  qmgr_hostname);
	else if ((pa = aei2addr(aei)) == NULLPA)
		perrstr ("Invalid qmgr hostname '%s': address translation failed\n\n",
			  qmgr_hostname);
	else
		pok ("qmgr hostname", qmgr_hostname);


	if ((password = getpwnam(pplogin)) == NULL)
		perrstr("Cannot access password entry for PP login '%s'\n\n",pplogin);
	else if (verbose == TRUE) {
		ptabss("PP login", pplogin);
		ptabsd("uid",	password->pw_uid);
		ptabsd("guid",password->pw_gid);
		pnewline();
	}
	if (password != NULL) {
		ppuid = password->pw_uid;
		ppgid = password->pw_gid;
	}

	pok("Local Mta",loc_dom_mta);
	pok("Local Site", loc_dom_site);
	pok("Local O/R address",loc_or);
	pok("X400 MTA name", x400_mta);

	if (max_hops <= 0 || max_hops >= 100)
		perrstr("WARNING: Maximum number of hops (trace fields) is set to %d",
			 (char *) max_hops);
	else if (verbose == TRUE) {
		char	buf[BUFSIZ];
		(void) sprintf(buf, "%d", max_hops);
		pok("Maximum hops", buf);
	}

	if (max_loops <= 0 || max_loops >= 100)
		perrstr("WARNING: Maximum nmber of local loops is set to %d",
			 (char *) max_loops);
	else if (verbose == TRUE) {
		char	buf[BUFSIZ];
		(void) sprintf(buf, "%d", max_loops);
		pok("Maximum loops", buf);
	}
		
	pok("Mailbox name", mboxname);

	ad = adr_new(postmaster, AD_822_TYPE, 0);
#ifdef UKORDER
	if (rp_isbad(ad_parse(ad, &rp, CH_UK_PREF))) 
#else
	if (rp_isbad(ad_parse(ad, &rp, CH_USA_PREF))) 
#endif
		perrstr("Postmaster '%s' is an invalid address (reason = '%s').\n",
			 postmaster,
			 ad->ad_parse_message);
	else
		pok("Post Master", postmaster);
	adr_free(ad);

	check_822address("postmaster");
	if (verbose == TRUE) check_822address("support");
	if (verbose == TRUE) check_822address("operator");
	if (verbose == TRUE) check_822address("admin");

	/* print list of body types */
	if (bodies_all == NULLIST_BPT)
		perrstr("System does not recognise any body parts\n\n","");
	else if (verbose == TRUE) {

		ix = bodies_all;
		printf("The system reconises the following body parts :-\n");
		while (ix != NULLIST_BPT) {
			ptabss("",ix->li_name);
			ix = ix->li_next;
		}
		pnewline();
	}
}
	
/*  */

typedef struct direntry {
	char		**fldirname;
	char		*name;
	int		maxmode, minmode;
} Direntry;

extern char	*quedfldir,
		*cmddfldir,
		*chndfldir,
		*formdfldir,
		*tbldfldir,
		*logdfldir,
		*ppdbm,
		*wrndfldir;

/*	0700 = rwx------
	0777 = rwxrwxrwx
	0711 = rwx--x--x
	0775 = rwxrwxr-x */

struct direntry dirs[] = {
	&quedfldir, "queue", 0700, 0700,
	&cmddfldir, "commands", 0775, 0711,
	&chndfldir, "channels", 0775, 0711,
	&formdfldir, "format", 0775, 0711,
	&tbldfldir, "table", 0775, 0711,
	&logdfldir, "logging", 0775, 0711,
	&wrndfldir, "warnings", 0775, 0711,
	0, 0
	};

/*  */
/* file and directory checking routines */
static Retvals check_file(name, himode, lomode, pfileMode, owner)
char		*name;
unsigned int	himode, lomode, *pfileMode;
char		*owner;
{
	struct stat 	statbuf;
	struct passwd	*temp;

	if (stat(name, &statbuf) != OK) 
		return notExist;
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
		*pfileMode = statbuf.st_mode & ~(S_IFMT|S_ISUID|S_ISGID|S_ISVTX);
	else
		*pfileMode = statbuf.st_mode & ~S_IFMT;
	if (*pfileMode & lomode != lomode)
		return lowMode;
	if (((*pfileMode) & (~himode)) != 0)
		return highMode;
	if (statbuf.st_uid != ppuid) {
		if ((temp = getpwuid(statbuf.st_uid)) == NULL) {
			if (verbose == TRUE)
				printf("**** ");
			printf("Unknown uid '%o' owns %s.\n",
			       statbuf.st_uid,
			       name);
			owner[0] = '\0';
		} else {
			(void) sprintf(owner, "%s", temp->pw_name);	
		}
		return wrongOwner;
	}
	return ok;
}

static void check_directory(dir)
struct direntry	*dir;
{
	char	prompt[BUFSIZ], owner[MAXPATHLENGTH];
	Retvals	retval;
	unsigned int	fileMode = 0;

	while ((retval = check_file(*(dir->fldirname), 
				    (unsigned int) dir->maxmode,
				    (unsigned int) dir->minmode,
				    &fileMode, 
				    owner)) != ok) {
		switch	(retval) {

		    case notExist:
			perrstr("The %s directory does not exist.\n", 
			       dir->name);
			sprintf(prompt, "Make directory %s with mode %o",
				*(dir->fldirname), dir->maxmode);
			if (yesorno(prompt) == TRUE) 
				makedirectory(*(dir->fldirname), dir->maxmode);
			else 
				return;
			break;
		    
		    case highMode:
			if (dir -> maxmode == dir -> minmode)
				perrstr("The directory '%s' has the wrong permission modes.\n",
					 *(dir->fldirname));
			else
				perrstr("The directory '%s' has too high a permission mode.\n",
					 *(dir->fldirname));
			sprintf(prompt, "Change mode of %s from %o to %o",
				*(dir->fldirname),
				fileMode,
				dir->maxmode);
			if (yesorno(prompt) == TRUE)
				changemode(*(dir->fldirname), dir->maxmode);
			else
				return;
			break;

		    case lowMode:
			if (dir -> maxmode == dir -> minmode)
				perrstr("The directory '%s' has the wrong permission modes.\n",
					 *(dir->fldirname));
			else
				perrstr("The directory '%s' has too low a permission mode.\n",
					 *(dir->fldirname));

			sprintf(prompt, "Change mode of %s from %o to %o",
				*(dir->fldirname),
				fileMode,
				dir->minmode);
			if (yesorno(prompt) == TRUE)
				changemode(*(dir->fldirname), dir->minmode);
			else
				return;
			break;

		    case wrongOwner:
			perrstr("The directory '%s' is not owned by the PP uid.\n",
			       *(dir->fldirname));
			if (owner[0] == '\0')
				(void) sprintf(prompt,
					       "Change owner of %s to %s",
					       *(dir->fldirname),
					       pplogin);
			else
				(void) sprintf(prompt,
					       "Change owner of %s from %s to %s",
					       *(dir->fldirname),
					       owner,
					       pplogin);
			if (yesorno(prompt) == TRUE)
				changeowner(*(dir->fldirname), 
					    ppuid,
					    ppgid);
			else
				return;
			break;

		    default:
			break;
		}
	}
	pok(dir->name, *(dir->fldirname));
}

static void check_dirs()
{
	int i = 0;
	char	*ix;
	struct direntry temp;
	char	fullpath[MAXPATHLENGTH];
	if (verbose == TRUE)
		printf("Checking the various directories\n\n");

	while (dirs[i].name != NULL)
		check_directory(&(dirs[i++]));
	/* special cases */
	/* ppdbm */
	if ((ix = rindex(ppdbm, '/')) != NULL) {
		*ix = '\0';
		if (verbose == TRUE)
			printf("Checking the directory for ppdbm file exist\n");
		temp.fldirname = &ppdbm;
		temp.name = "dbm";
		temp.maxmode = 0775;
		temp.minmode = 0711;
		check_directory(&temp);
		*ix = '/';
	}
}

/*  */
/* check the executables for the channels */
/*   and the filter executables for the filtercontrol channels */
/*   then print out some nice info */

static int	warning,
		delete,
		qmgr_load,
		debris,
		time_out;

#define	spec_chan_error	"There is no %s channel.\n"
static void check_special_channels()
{
	if (warning == 0) 
		perrstr(spec_chan_error, "warning");
	if (delete == 0)
		perrstr(spec_chan_error, "delete");
	if (qmgr_load == 0)
		perrstr(spec_chan_error, "qmgr-load");
	if (debris == 0)
		perrstr(spec_chan_error, "debris");
	if (time_out == 0)
		perrstr(spec_chan_error, "timeout");
}

static void check_chans()
{
	CHAN	**ix = ch_all;
	
	warning = delete = qmgr_load = debris = time_out = 0;

	if (verbose == TRUE)
		printf("\nChecking the various channels\n\n");

	while (*ix != NULL)
		check_channel(*ix++);
	check_special_channels();
}

static char *chan_type(chan)
CHAN	*chan;
{
	char	*str = NULLCP;
	switch(chan->ch_chan_type) {
	    case CH_SHAPER:
		str = "formatter channel";
		break;
	    case CH_IN:
		str = "listener channel";
		break;
	    case CH_OUT:
		str = "transmitter channel";
		break;
	    case CH_BOTH:
		str = "listener/transmitter channel";
		break;
	    case CH_WARNING:
		str = "warning channel";
		warning = 1;
		break;
	    case CH_DELETE:
		str = "deletion channel";
		delete = 1;
		break;
	    case CH_QMGR_LOAD:
		str = "loader channel";
		qmgr_load = 1;
		break;
	    case CH_DEBRIS:
		str = "debris cleaning channel";
		debris = 1;
		break;
	    case CH_TIMEOUT:
		str = "timeout channel";
		time_out = 1;
		break;
	    case CH_SPLITTER:
		str = "splitter";
		break;
	    default:
		str = "Unknown type";
		break;
	}
	return str;
}

static void check_channel(chan)
CHAN	*chan;
{
	struct stat statbuf;
	char	fullname[MAXPATHLENGTH],
		*ch_type,
		*margv[100];
	int	margc;
	
	ch_type = chan_type(chan);

	/* do checking */
	if (chan->ch_progname != NULLCP) {
		if ((margc = sstr2arg(chan->ch_progname, 100, margv, " \t")) < 1) {
			perrstr("Unable to parse program '%s' for channel '%s'\n",
			       chan->ch_progname,
			       chan->ch_name);
			return;
		}
		if (margv[0][0] == '/')
			sprintf(fullname,"%s",margv[0]);
		else
			sprintf(fullname,"%s/%s",chndfldir,margv[0]);

		if (stat(fullname, &statbuf) != OK)
			perrstr("The program %s for channel %s does not exist\n",
				 fullname, chan->ch_name);
		else if (!(statbuf.st_mode & S_IEXEC)) 
			perrstr("The program %s for the channel %s is not executable\n",
				 fullname, chan->ch_name);
		else if (statbuf.st_mode & S_ISUID
			 && chan->ch_access != CH_MTS 
			 && (chan->ch_chan_type != CH_OUT 
			     || chan->ch_chan_type != CH_BOTH)) {
			perrstr("WARNING The program %s for the channel %s is set uid.\n",
				 fullname, chan->ch_name);
			if (verbose == TRUE)
				printf("**** ");
			printf("Only local channels should be set uid\n");

		}
		
		if (chan->ch_access == CH_MTS
		    && (chan->ch_chan_type == CH_OUT 
			|| chan->ch_chan_type == CH_BOTH)) {
			/* local delivery */
			if (statbuf.st_uid != 0)
				perrstr("WARNING Channel '%s' is local delivery\n\tbut program '%s' isn't owned by root.\n",
					 chan->ch_name,
					 fullname);
		} else if (statbuf.st_uid != ppuid) 
			perrstr("WARNING program '%s' for channel '%s' is not owned by '%s'\n",
				 fullname,
				 chan->ch_name,
				 pplogin);
		
		if (chan->ch_access == CH_MTS 
		    && (chan->ch_chan_type == CH_OUT || chan->ch_chan_type == CH_BOTH)
		    && !(statbuf.st_mode & S_ISUID)) 
			perrstr("WARNING Channel '%s' is local delivery\n\tbut program '%s' isn't set uid.\n",
				 chan->ch_name,
				 fullname);
	} else if (chan->ch_chan_type != CH_IN)
		perrstr("WARNING No program given for channel %s\n",chan->ch_name);
	if (chan->ch_out_info != NULL
	    && chan->ch_chan_type == CH_SHAPER)
		/* assume is a filter control channel */
		check_filter(chan->ch_name, chan->ch_out_info);

	if (chan->ch_table != NULL) 
		check_table_file(chan->ch_table,0644, 0600);
	
	if (chan->ch_mta_table != NULL) 
		check_table_file(chan->ch_mta_table,0644, 0600);

	if (chan->ch_in_table != NULL) 
		check_table_file(chan->ch_in_table,0644, 0600);

	if (chan->ch_hdr_in != NULL) 
		check_hdrs(chan->ch_name, chan->ch_hdr_in);

	if (chan->ch_hdr_out != NULL)
		check_hdrs(chan->ch_name, chan->ch_hdr_out);

	if (chan->ch_bpt_in != NULL)
		check_bps(chan->ch_name, chan->ch_bpt_in);

	if (chan->ch_bpt_out != NULL)
		check_bps(chan->ch_name, chan->ch_bpt_out);

	if (chan->ch_drchan != NULLCP &&
	    chan->ch_chan_type != CH_BOTH &&
	    chan->ch_chan_type != CH_OUT)
		perrstr("drchan '%s' specified for channel '%s'\n\tOnly outbound channels should have drchan specified\n",
				 chan->ch_drchan, chan->ch_name);
	if (chan -> ch_drchan) {
		CHAN *drc = ch_nm2struct(chan -> ch_drchan);
		if (drc == NULLCHAN)
			perrstr ("drchan '%s' for channel %s does not exist\n",
				  chan -> ch_drchan, chan -> ch_name);
		else if (drc -> ch_chan_type != CH_BOTH &&
			 drc -> ch_chan_type != CH_OUT)
			perrstr ("drchan '%s' for channel %s is not outbound type\n",
				  chan -> ch_drchan, chan -> ch_name);
	}
	if (chan->ch_badSenderPolicy != CH_BADSENDER_STRICT
	    && chan->ch_chan_type != CH_BOTH
	    && chan->ch_chan_type != CH_IN)
		perrstr("bad-sender-policy specified for channel '%s'\n\tOnly inbound channels should have bad-sender-policy specified\n",
			 chan->ch_name);

	if (chan->ch_out_ad_type == AD_ANY_TYPE) 
		perrstr("WARNING address type set to any for channel %s.\n\tThis may cause address parsing problems.\n",chan->ch_name);
	if (chan->ch_in_ad_type == AD_ANY_TYPE) 
		perrstr("WARNING address type set to any for channel %s.\n\tThis may cause address parsing problems.\n",chan->ch_name);

	if (verbose == TRUE) {
		/* print info */
		ptabss(chan->ch_name,
		       (chan->ch_show == NULL) ? chan->ch_name : chan->ch_show);
		if (chan->ch_progname != NULLCP)
			ptabss("program", chan->ch_progname);

		if (chan->ch_drchan != NULLCP)
			ptabss("drchan", chan->ch_drchan);

		ptabss("type", ch_type);

		if (chan->ch_out_info != NULL)
			ptabss("filter run as", chan->ch_out_info);

		if (chan->ch_table == NULL)
			ptabss("channel outbound table", "none");
		else
			ptabss("channel outbound table",chan->ch_table->tb_name);

		if (chan->ch_in_table == NULL)
			ptabss("channel inbound table", "none");
		else
			ptabss("channel inbound table",chan->ch_in_table->tb_name);

		if (chan->ch_mta_table == NULL)
			ptabss("channel MTA table", "none");
		else
			ptabss("channel MTA table",chan->ch_mta_table->tb_name);
		pnewline();
	}
}

static void check_filter(chan, info)
char	*chan,
	*info;
{
	char	fullname[MAXPATHLENGTH],
		*ix = info,
		saved;
	struct stat statbuf;

	while (*ix != '\0' && !isspace(*ix))
		ix++;
	saved = *ix;
	*ix = '\0';
	
	if (info[0] == '/')
		sprintf(fullname, "%s", info);
	else
		sprintf(fullname, "%s/%s",formdfldir,info);

	*ix = saved;

	if (stat(fullname, &statbuf) != OK) {
		if (verbose == TRUE)
			printf("**** ");
		printf("The filter %s for channel %s does not exist\n",
		       fullname, chan);
		return;
	}
	if (!(statbuf.st_mode & S_IEXEC)) {
		if (verbose == TRUE)
			printf("**** ");
		printf("The filter %s for channel %s is not executable\n",
		       fullname, chan);
		return;
	}
}

/*  */
/* check the tables */

static void check_tables()
{
	Table	**ix = tb_all;

	if (verbose == TRUE)
		printf("\nChecking the various tables\n\n");
	while (*ix != NULL) {
		check_table(*ix++);
		pnewline();
	}
}

static void check_table_file(tbl, maxmode, minmode)
Table		*tbl;
int		maxmode, minmode;
{
	char	fullname[MAXPATHLENGTH], owner[MAXPATHLENGTH], prompt[BUFSIZ];
	unsigned int	fileMode = 0;
	Retvals	retval;

	if (tbl->tb_file[0] == '/')
		sprintf(fullname,"%s",tbl->tb_file);
	else
		sprintf(fullname,"%s/%s",tbldfldir,tbl->tb_file);

	while ((retval = check_file(fullname, (unsigned int) maxmode,
				    (unsigned int) minmode,
				    &fileMode, owner)) != ok) {
		switch (retval) {
		    case notExist:
			perrstr("The file %s is not present for table %s\n",
				 fullname, tbl->tb_name);
			return;
		   
		    case highMode:
			if (maxmode == minmode)
				perrstr("The table file '%s' has the wrong permission modes.\n",
					 fullname);
			else
				perrstr("The table file '%s' has too high a permission mode.\n",
					 fullname);
			sprintf(prompt, "Change mode of %s from %o to %o",
				fullname,
				fileMode,
				maxmode);
			if (yesorno(prompt) == TRUE)
				changemode(fullname, maxmode);
			else
				return;
			break;

		    case lowMode:
			if (maxmode == minmode)
				perrstr("The table file '%s' has the wrong permission modes.\n",
					 fullname);
			else
				perrstr("The table file '%s' has too low a permission mode.\n");
			
			sprintf(prompt, "Change mode of %s from %o to %o",
				fullname,
				fileMode,
				minmode);
			if (yesorno(prompt) == TRUE)
				changemode(fullname, minmode);
			else
				return;
			break;

		    case wrongOwner:
			perrstr("The table file '%s' is not owned by the PP uid.\n",
			       fullname);
			if (owner[0] == '\0')
				(void) sprintf(prompt,
					       "Change owner of %s to %s",
					       fullname,
					       pplogin);
			else
				(void) sprintf(prompt,
					       "Change owner of %s from %s to %s",
					       fullname,
					       owner,
					       pplogin);
			if (yesorno(prompt) == TRUE)
				changeowner(fullname,
					    ppuid,
					    ppgid);
			else
				return;
			break;
			 
		    default:
			break;
		}
	}
}
	
static void check_table(tbl)
Table	*tbl;
{
	char	fullname[MAXPATHLENGTH], owner[MAXPATHLENGTH], prompt[BUFSIZ];
	unsigned int 	fileMode = 0;
	Retvals	retval;

	if (tbl->tb_file[0] == '/')
		sprintf(fullname,"%s",tbl->tb_file);
	else
		sprintf(fullname,"%s/%s",tbldfldir,tbl->tb_file);

	while ((retval = check_file(fullname, 0644, 0600,
				    &fileMode, owner)) != ok) {
		switch (retval) {
		    case notExist:
			perrstr("The file %s is not present for table %s\n",
				 fullname, tbl->tb_name);
			return;
		    
		    case highMode:
			perrstr("The table file '%s' has too high a permission mode.\n",
			       fullname);
			sprintf(prompt, "Change mode of %s from %o to %o",
				fullname,
				fileMode,
				0644);
			if (yesorno(prompt) == TRUE)
				changemode(fullname, 0644);
			else
				break;
			continue;

		    case lowMode:
			perrstr("The table file '%s' has too low a permission mode.\n",
			       fullname);
			sprintf(prompt, "Change mode of %s from %o to %o",
				fullname,
				fileMode,
				0600);
			if (yesorno(prompt) == TRUE)
				changemode(fullname, 0600);
			else
				break;
			continue;

		    case wrongOwner:
			perrstr("The table file '%s' is not owned by the PP uid.\n",
			       fullname);
			if (owner[0] == '\0')
				(void) sprintf(prompt,
					       "Change owner of %s to %s",
					       fullname,
					       pplogin);
			else
				(void) sprintf(prompt,
					       "Change owner of %s from %s to %s",
					       fullname,
					       owner,
					       pplogin);
			if (yesorno(prompt) == TRUE)
				changeowner(fullname,
					    ppuid,
					    ppgid);
			else
				break;
			continue;
			 
		    default:
			continue;
		}
		break;
	}

	ptabss(tbl->tb_name,tbl->tb_show);
	ptabss("file",fullname);
	switch (tbl->tb_flags) {
	    case TB_DBM:
		ptabss("Storage","database");
		break;
	    case TB_LINEAR:
		ptabss("Storage", "linear");
		break;
	    default:
		ptabss("Storage","unknown option");
		break;
	}
}

static void check_bps(channame, list)
char		*channame;
LIST_BPT	*list;
{
	LIST_BPT	*ix;

	while (list != NULL) {
		ix = bodies_all;
		while (ix != NULL
		       && strcmp(ix->li_name, list->li_name) != 0)
			ix = ix->li_next;
		if (ix == NULLIST_BPT)
			perrstr("The channel '%s' has an unrecognised bodypart '%s'.\n",
				 channame,
				 list->li_name);
		list = list->li_next;
	}
}

static void check_hdrs(channame, list)
char		*channame;
LIST_BPT	*list;
{
	LIST_BPT	*ix;

	while (list != NULL) {
		ix = headers_all;
		while (ix != NULL
		       && strcmp(ix->li_name, list->li_name) != 0)
			ix = ix->li_next;
		if (ix == NULLIST_BPT)
			perrstr("The channel '%s' has an unrecognised header '%s'.\n",
				 channame,
				 list->li_name);
		list = list->li_next;
	}
}
	       
static int yesorno(prompt)
char	*prompt;
{
	if (monitoring == TRUE) {
		printf("%s\n",prompt);
		return FALSE;
	}

	if (interactive == TRUE) {
		char buf[BUFSIZ],
		     *ix;
		int  noAnswer = TRUE,
		     answer = FALSE;

		/* output a yes/no query and return result */
		while (noAnswer == TRUE) {
			printf("%s (y/n) ?",prompt);
			fgets(buf, BUFSIZ, stdin);
			ix = buf;
			while (*ix != NULL && isspace(*ix) != 0)
				ix++;
			switch (*ix) {
			    case 'y':
			    case 'Y':
				noAnswer = FALSE;
				answer = TRUE;
				break;
			    case 'n':
			    case 'N':
				noAnswer = FALSE;
				answer = FALSE;
				break;
			    default:
				break;
			}
		}
		return answer;
	} else {
		/* see if can do automatically and if so try it */
		pnewline();
		return TRUE;
	}
}

static int doneMsg;

static void check_htable(name)
char	*name;
{
	/* check table with hardwired name 'name' exists */
	if (tb_nm2struct(name) == NULLTBL) {
		if (doneMsg == 0) {
			printf("Note that the following messages may or may not be fatal\nThis depends on the local requirements\n");
			doneMsg = 1;
		}
		perrstr("A table with the hardwired name '%s' is not present\n",
		       name);
	} else if (verbose == TRUE)
		pok("Hardwired table", name);
		
}

static void check_hprog(name, defaultdir)
char	*name;
char	*defaultdir;
{
	struct stat statbuf;
	char	fullname[MAXPATHLENGTH];

	if (name[0] == '/')
		strcat(fullname, name);
	else
		sprintf(fullname, "%s/%s", defaultdir, name);

	if (stat(fullname, &statbuf) != OK)
		perrstr("The hardwired program %s does not exist\n",
				 fullname);
	else if (!(statbuf.st_mode & S_IEXEC)) 
		perrstr("The hardwired program %s is not executable\n",
			 fullname);
	else if (statbuf.st_uid != ppuid)
		perrstr("WARNING hardwired program %s is not owned by '%s'\n",
			 fullname,
			 pplogin);
	else if (verbose == TRUE) 
		/* print info */
		pok ("Hardwired program", name);
}

	
static void check_hchan(name)
char	*name;
{
	
	if (ch_nm2struct(name) == NULLCHAN) {
		if (doneMsg == 0) {
			printf("Note that the following messages may or may not be fatal\nThis depends on the local requirements\n");
			doneMsg = 1;
		}
		perrstr("A channel with the hardwired name '%s' is not present\n",
		       name);
	} else if (verbose == TRUE) 
		pok("Hardwired channel", name);
}

static void check_hbp(name)
char	*name;
{
	LIST_BPT	*ix = bodies_all;

	while (ix != NULL 
	       && strcmp(name, ix->li_name) != 0)
		ix = ix->li_next;
	if (ix == NULLIST_BPT) {
		if (doneMsg == 0) {
			printf("Note that the following messages may or may not be fatal\nThis depends on the local requirements\n");
			doneMsg = 1;
		}
		perrstr("A bodypart with the hardwired name '%s' is not present\n",
		       name);
	} else if (verbose == TRUE)
		pok("Hardwired body part", name);
}		

static void check_hhdr(name)
char	*name;
{
	LIST_BPT	*ix = headers_all;

	while (ix != NULL 
	       && strcmp(name, ix->li_name) != 0)
		ix = ix->li_next;
	if (ix == NULLIST_BPT) {
		if (doneMsg == 0) {
			printf("Note that the following messages may or may not be fatal\nThis depends on the local requirements\n");
			doneMsg = 1;
		}
		perrstr("A header with the hardwired name '%s' is not present\n",
		       name);
	} else if (verbose == TRUE)
		pok("Hardwired header", name);
}		

static void check_hardwired()
{
	extern char	*submit_prog, *uucpin_chan, *local_822_chan, 
			*alias_tbl, *channel_tbl, 
			*list_tbl, *user_tbl, *or_tbl, *or2rfc_tbl, 
			*rfc2or_tbl, *chan_auth_tbl, *rfc1148gateway_tbl, 
			*mta_auth_tbl, *user_auth_tbl, *qmgr_auth_tbl,
			*hdr_822_bp, *hdr_p2_bp, *hdr_ipn_bp, *ia5_bp;
	if (verbose == TRUE)
		printf("\nChecking the various hardwired names (static.c)\n\n");
	if (verbose == TRUE) {
		printf("Note that the following messages may or may not be fatal\nThis depends on the local requirements\n");
		doneMsg = 1;
	} else
		doneMsg = 0;

	check_hprog(submit_prog, cmddfldir);
	check_hchan(uucpin_chan);
	check_hchan(local_822_chan);
	check_htable(alias_tbl);
	check_htable(channel_tbl);
	check_htable(list_tbl);
	check_htable(user_tbl);
	check_htable(or_tbl);
	check_htable(or2rfc_tbl);
	check_htable(rfc2or_tbl);
	check_htable(chan_auth_tbl);
	check_htable(mta_auth_tbl);
	check_htable(user_auth_tbl);
	check_htable(rfc1148gateway_tbl);
	check_hhdr(hdr_822_bp);
	check_hhdr(hdr_p2_bp);
	check_hhdr(hdr_ipn_bp);
	check_hbp(ia5_bp);
	check_htable(qmgr_auth_tbl);
}

static void makedirectory(name, mode)
char		*name;
int	mode;
{
	char	*ix, *last;
	struct stat statbuf;

	last = ix = name;
	if (*ix == '/') ix++;
	while (*ix != '\0') {
		while (*ix != '\0' && *ix != '/') ix++;
		if (*ix != '\0' ) {
			*ix = '\0';
			if (stat(name, &statbuf) != OK) {
				if (mkdir(name, mode) != 0) {
					printf("!*!* Failed to make directory %s\n",name);
					*ix = '/';
					return;
				} else 
					printf("---> Made directory %s\n",name);
			}
			*ix = '/';
			last = ix;
			ix++;
		} else {
			if (ix != last + 1) {
				/* doesn't end with / so check last segment */
				if (stat(name, &statbuf) != OK) {
					if (mkdir(name, mode) != 0) 
						printf("!*!* Failed to make directory %s\n",name);
					else
						printf("---> Made directory %s\n",name);
				}
				/* reached end so return */
			}
			return;
		}
	}

/*	if (mkdir(name, mode) != 0)
		printf("!*!* Failed to make directory %s\n",name);
	else
		printf("---> Made directory %s\n",name);*/
}

static void changeowner(file, uid, gid)
char		*file;
int		uid, gid;
{
	if (chown(file, uid, gid) != 0)
		printf("!*!* Failed to change owner of %s to %o and group %o\n",
		       file, uid, gid);
	else
		printf("---> Changed owner of %s to %o\n",
		       file, uid);
}

static void changemode(file, mode)
char		*file;
int		mode;
{
	if (chmod(file,mode) != 0) 
		printf("!*!* Failed to change mode of %s to %o\n",
		       file, mode);
	else
		printf("---> Changed mode of %s to %o\n",
		       file,mode);
}

void pnewline()
{
	if (verbose == TRUE)
		printf("\n");
}

static void perrstr(char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list ap;
	va_start (ap, fmt);
	_asprintf (buf, NULLCP, fmt, ap);
	fputs (buf, stdout);
	va_end (ap);
}

#define tab	20
static void pok(one,two)
char	*one,
	*two;
{
	if (verbose == TRUE) {
		ptabss(one,two);
		pnewline();
	}
}

static void ptabss(one,two)
char	*one,
	*two;
{
	if (verbose == TRUE)
		printf("%-*s: %s\n",tab, one, two);
}

static void ptabsd(one,two)
char	*one;
int	two;
{
	if (verbose == TRUE)
		printf("%-*s: %d\n",tab,one,two);
}

static void check_822address(str)
char	*str;
{
	ADDR	*ad;
	RP_Buf	rp;
	ad = adr_new(str, AD_822_TYPE, 0);
#ifdef UKORDER
	if (rp_isbad(ad_parse(ad, &rp, CH_UK_PREF)))
#else
	if (rp_isbad(ad_parse(ad, &rp, CH_USA_PREF)))
#endif
		perrstr("rfc822 required address '%s' is an invalid address (reason = '%s').\n",
			 str,
			 ad->ad_parse_message);
	adr_free(ad);
}
		
static void check_dsap ()
{
	int pid, cpid;

	if ((pid = tryfork ()) == -1)
		return;
	if (pid == 0) {		/* child */
		dsap_init((char ***)NULL, (int *) NULL);
		_exit (0);
	}
	else {
// #ifdef SYSV
		int w;
		while ((cpid = wait (&w)) != pid && cpid != NOTOK)
			continue;
// #else
// 		union wait w;
// 		while ((cpid = wait (&w.w_status)) != pid && cpid != NOTOK)
// 			continue;
// #endif
		if (!WIFSIGNALED(w) && WEXITSTATUS(w) == 0) {
			dsap_init((char ***)NULL, (int *) NULL);
			return;
		}
		perrstr("dsap error: probably quipu oid tables not installed?\n");
	}
}
