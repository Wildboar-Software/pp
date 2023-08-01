/* tai_sys.c: system tailoring routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/tai/RCS/tai_sys.c,v 6.0 1991/12/18 20:24:59 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/tai/RCS/tai_sys.c,v 6.0 1991/12/18 20:24:59 jpo Rel $
 *
 * $Log: tai_sys.c,v $
 * Revision 6.0  1991/12/18  20:24:59  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include <isode/cmd_srch.h>
#include "list_bpt.h"
#include <isode/tailor.h>

extern	void		err_abrt ();
extern	void		pp_setlog ();
extern	void		tai_error ();
extern	int		arg2vstr ();

extern char
	*chndfldir,
	*cmddfldir,
	*dap_user,
	*dap_passwd,
	*formdfldir,
	*lckdfldir,
	*lockdir,
	*loc_dom_mta,
	*loc_dom_site,
	*loc_or,
	*logdfldir,
	*mboxname,
	*niftpcpf,
	*niftpquedir,
	*postmaster,
	*adminstration_req_alt,
	*pp_isodetailor,
	*ppdbm,
	*pplogin,
	*pptsapd_addr,
	*qmgr_hostname,
	*quedfldir,
	*tbldfldir,
	*delim1,
	*delim2,
	*authchannel,
	*authloglevel,
	*submit_addr,
	*sysmailfilter,
	*mailfilter,
	*x400_mta,
	*wrndfldir;

extern	LLog
	*pp_log_norm,
	*pp_log_oper,
	*pp_log_stat;

extern	LIST_BPT
	*bodies_all,
	*headers_all;

extern	int
	lockstyle,
	warn_interval,
	warn_number,
	max_hops,
	max_loops,
	queue_depth,
	queue_fanout,
	use_fsync,
	disk_percent,
	disk_blocks,
	return_interval_low,
	return_interval_norm,
	return_interval_high;



extern CMD_TABLE tbl_bool[];

#define MAUTHLOG		1
#define MBODY_TYPES		2
#define MBOXNAME		3
#define MCHANDIR		4
#define MCMDDIR			5
#define MDBM			6
#define MFORMDIR		7
#define MISODELOG		8
#define MISOTAILOR		9
#define MLOC_DOM_MTA		10
#define MLOC_DOM_SITE		11 
#define MLOC_OR			12	
#define MLOGDIR			13 
#define MNORMLOG		14 
#define MOPERLOG		15  
#define MPOSTMASTER		16
#define MPPLOGIN		17 
#define MPPTSAPD_ADDR		18 
#define MQMGR_HOST		19 
#define MQUEDIR			20 
#define MTBLDIR			21 
#define MDELIM1			22 
#define MDELIM2			23 
#define MAUTHCHANNEL		24 
#define MAUTHLOGLEVEL		25 
#define MWRNDFLDIR		26 
#define LOCKSTYLE		27	
#define LOCKDIR			28
#define WARN_INTERVAL		29
#define	WARN_NUMBER		30
#define RETURN_INTERVAL		31
#define MAILFILTER		32
#define SYSMAILFILTER		33
#define MAXLOOPS		34
#define MAXHOPS			35
#define X400MTA			36
#define MHDR_TYPES		37
#define SUBMIT_ADDR		38
#define DAPUSER			39
#define DAPPASSWD		40
#define	QUEUE_STRUCTURE		41
#define USE_FSYNC		42
#define DISK_USE		43
#define MADMINASIGALT		44

/* this array is sorted - be careful if you change it! */
static	CMD_TABLE  cmdtab[] = {
	"adminstration_assigned_alternate_recipient", MADMINASIGALT,
	"authchannel",		MAUTHCHANNEL,
	"authlog",		MAUTHLOG,
	"authloglevel",		MAUTHLOGLEVEL,
	"bodypart",		MBODY_TYPES,
	"chandir",		MCHANDIR,
	"cmddir",		MCMDDIR,
	"dap_passwd",		DAPPASSWD,
	"dap_user",		DAPUSER,
	"dbm",			MDBM,
	"delim1",		MDELIM1,
	"delim2",		MDELIM2,
	"diskuse",		DISK_USE,
	"formdir",		MFORMDIR,
	"fsync",		USE_FSYNC,
	"headertype",		MHDR_TYPES,
	"isode",		MISOTAILOR,
	"isodelog",		MISODELOG,
	"lockdir",		LOCKDIR,
	"lockstyle",		LOCKSTYLE,
	"loc_dom_mta",		MLOC_DOM_MTA,
	"loc_dom_site",		MLOC_DOM_SITE,
	"loc_or",		MLOC_OR,
	"logdir",		MLOGDIR,
	"mailfilter",		MAILFILTER,
	"maxhops",		MAXHOPS,
	"maxloops",		MAXLOOPS,
	"mboxname",		MBOXNAME,
	"normlog",		MNORMLOG,
	"nwarnings",		WARN_NUMBER,
	"operlog",		MOPERLOG,
	"postmaster",		MPOSTMASTER,
	"pplogin",		MPPLOGIN,
	"pptsapd_addr",		MPPTSAPD_ADDR,
	"qmgrhost",		MQMGR_HOST,
	"quedir",		MQUEDIR,
	"queuestruct",		QUEUE_STRUCTURE,
	"returntime",		RETURN_INTERVAL,
	"submit_addr",		SUBMIT_ADDR,
	"sysmailfilter",	SYSMAILFILTER,
	"tbldir",		MTBLDIR,
	"warninterval",		WARN_INTERVAL,
	"wrndfldir",		MWRNDFLDIR,
	"x400mta",		X400MTA,
	0,			-1
	};
#define N_CMDENT	((sizeof(cmdtab)/sizeof(CMD_TABLE)) - 1)



/* ---------------------  Begin	 Routines  -------------------------------- */


static	void	do_isode_tailor();
static	void	add_bodies(), add_hdrs();
static	char	*argbak2str();

static CMD_TABLE tbl_lock_styles[]  = {
	"flock",	LOCK_FLOCK,
	"fcntl",	LOCK_FCNTL,
	"file",		LOCK_FILE,
	"lockf",	LOCK_LOCKF,
	0,		-1
	};

int sys_tai (argc, argv)   /* do system wide initialisations  */
int	argc;
char	**argv;
{
	char	*arg;
	int val;

	if (argc < 2)
		return (NOTOK);
	arg = argv[1];

	PP_DBG (("tai/sys_tai %s %s", argv[0], arg));

	switch (cmdbsrch (argv[0], cmdtab, N_CMDENT)) {
	    case MAUTHLOG:
		log_tai (pp_log_stat, &argv[1], argc-1);
		break;
	    case MBODY_TYPES:
		add_bodies (&argv[1], argc - 1);
		break;
	    case MHDR_TYPES:
		add_hdrs (&argv[1], argc - 1);
		break;
	    case MBOXNAME:
		mboxname = arg;
		break;
	    case MCHANDIR:
		chndfldir = arg;
		break;
	    case MCMDDIR:
		cmddfldir = arg;
		break;
	    case MDBM:
		ppdbm = arg;
		break;
	    case MFORMDIR:
		formdfldir = arg;
		break;
	    case MISODELOG:
		pp_setlog (&argv[1], argc - 1);
		break;
	    case MISOTAILOR:
		do_isode_tailor (&argv[1], argc-1);
		break;
	    case MLOC_DOM_MTA:
		loc_dom_mta = arg;
		break;
	    case MLOC_DOM_SITE:
		loc_dom_site = arg;
		break;
	    case MLOC_OR:
		loc_or = arg;
		break;
	    case MLOGDIR:
		logdfldir = arg;
		break;
	    case MNORMLOG:
		log_tai (pp_log_norm, &argv[1], argc-1);
		break;
	    case MOPERLOG:
		log_tai (pp_log_oper, &argv[1], argc-1);
		break;
	    case MPOSTMASTER:
		postmaster = arg;
		break;
	    case MADMINASIGALT:
		adminstration_req_alt = arg;
		break;
	    case MPPLOGIN:
		pplogin = arg;
		break;
	    case MPPTSAPD_ADDR:
		pptsapd_addr = arg;
		break;
	    case MQMGR_HOST:
		qmgr_hostname = arg;
		break;
	    case MQUEDIR:
		quedfldir = arg;
		break;
	    case MTBLDIR:
		tbldfldir = arg;
		break;
	    case MDELIM1:
		delim1 = arg;
		break;
	    case MDELIM2:
		delim2 = arg;
		break;
	    case MAUTHCHANNEL:
		authchannel = argbak2str(&argv[1]);
		break;
	    case MAUTHLOGLEVEL:
		authloglevel = arg;
		break;
	    case MWRNDFLDIR:
		wrndfldir = arg;
		break;
	    case LOCKSTYLE:
		lockstyle = cmd_srch (arg, tbl_lock_styles);
		if (lockstyle == -1) {
			tai_error ("Unknown lockstyle setting %s",
				   arg);
			lockstyle = LOCK_FLOCK;
		}
		break;
	    case LOCKDIR:
		lockdir = arg;
		break;

	    case WARN_INTERVAL:
		if ((val = atoi(arg)) > 0)
			warn_interval = val;
		else
			tai_error ("Bad warning interval %d", val);
		break;
	    case WARN_NUMBER:
		if ((val = atoi(arg)) >= 0)
			warn_number = val;
		else
			tai_error ("Bad warning number %d", val);
		break;
	    case RETURN_INTERVAL:
		if ((val = atoi(arg)) > 0)
		    return_interval_norm = val;
		else
			tai_error ("Bad return interval time %d", val);
		return_interval_high = return_interval_norm / 2;
		return_interval_low = return_interval_norm * 2;
		if (argc >= 3 ) {
			if ((val = atoi (argv[2])) > 0)
				return_interval_high = val;
			else	tai_error ("Bad high priority return time %d",
					   val);
		}
		if (argc >= 4) {
			if ((val = atoi(argv[3])) > 0)
				return_interval_low = val;
			else
				tai_error ("Bad low priority return time %d",
					   val);
		}
		break;
	    case MAILFILTER:
		mailfilter = arg;
		break;
	    case SYSMAILFILTER:
		sysmailfilter = arg;
		break;
	    case MAXHOPS:
		if ((val = atoi(arg)) > 1)
			max_hops = val;
		else	tai_error ("Bad max hops value %d", val);
		break;
	    case MAXLOOPS:
		if ((val = atoi(arg)) > 1)
			max_loops = val;
		else	tai_error ("Bad max loops value %d", val);
		break;
	    case X400MTA:
		x400_mta = arg;
		break;
	    case SUBMIT_ADDR:
		submit_addr = arg;
		break;
	    case DAPUSER:
		dap_user = arg;
		break;
	    case DAPPASSWD:
		dap_passwd = arg;
		break;
	    case QUEUE_STRUCTURE:
		queue_depth = 1;
		if ((val = atoi(arg)) > 0)
			queue_fanout = val;
		else
			tai_error ("Bad queue fanout param %d", val);
		if (argc >= 3) {
			if ((val = atoi(arg)) > 0)
				queue_depth = val;
			else
				tai_error ("Bad queue depth value %d", val);
		}
		break;
	    case USE_FSYNC:
		if ((val = cmd_srch (arg, tbl_bool)) == NOTOK)
			tai_error ("Bad boolean value for fsync %s", arg);
		else
			use_fsync = val;
		break;
	    case DISK_USE:
		if ((val = atoi(arg)) >= 0)
			disk_blocks = val;
		else	tai_error ("Bad disk use parameter %d", val);
		if (argc >= 3) {
			if ((val = atoi(argv[2])) >= 0)
				disk_percent = val;
			else
				tai_error ("Bad disk percentage figure %d",
					   val);
		}
		break;
	    default:
		PP_DBG (("Lib/tai_sys.c/default=%s", arg));
		return (NOTOK);
	}
	return (OK);
}


/* ------------------------------------------------------------------ */

static void do_isode_tailor (av, ac)
char	**av;
int	ac;
{
	char buffer[BUFSIZ];
	int	i;

	buffer[0] = 0;
	for (i = 1; i < ac; i++) {
		(void) strcat (buffer, av[i]);
		(void) strcat (buffer, " ");
	}

	if (isodesetvar (av[0], strdup (buffer), 1) == NOTOK)
		tai_error ("Unknown isode variable %s", av[0]);
}

/* ------------------------------------------------------------------ */

static void add_bodies (av, ac)
char	**av;
int	ac;
{
	int i;
	LIST_BPT	*new;

	for (i = 0; i < ac; i++, av ++) {
		if (!isstr (*av))
			continue;
		new = list_bpt_new (*av);
		if (new == NULLIST_BPT)
			err_abrt (RP_MECH, "Out of memory");
		list_bpt_add (&bodies_all, new);
	}
}

extern char	*hdr_prefix;

static void add_hdrs (av, ac)
char	**av;
int	ac;
{
	int i;
	LIST_BPT	*new;
	char	buf[BUFSIZ];

	for (i = 0; i < ac; i++, av ++) {
		if (!isstr (*av))
			continue;
		(void) sprintf(buf, "%s%s", hdr_prefix, *av);
		new = list_bpt_new (buf);
		if (new == NULLIST_BPT)
			err_abrt (RP_MECH, "Out of memory");
		list_bpt_add (&headers_all, new);
	}
}

/* ------------------------------------------------------------------ */

static char *argbak2str(av)	/* convert back to string for now */
char	**av;

/*	The purpose of this is to allow a tailor line to be entered in the same
	format as a database entry, or to postpone parsing until a later
	time
*/

{
	char	buffer[BUFSIZ];

	(void) arg2vstr( 0, BUFSIZ, buffer, av );	/* 0 = no folding */
	PP_DBG (("tai_sys/argbak2str '%s'", buffer));
	return strdup( buffer );
}
