/* init_tai.c: tailoring initialisation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/tai/RCS/init_tai.c,v 6.0 1991/12/18 20:24:59 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/tai/RCS/init_tai.c,v 6.0 1991/12/18 20:24:59 jpo Rel $
 *
 * $Log: init_tai.c,v $
 * Revision 6.0  1991/12/18  20:24:59  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include <sys/stat.h>
#include <isode/tailor.h>
#include <fcntl.h>
#include <varargs.h>

#define MAXTAIARGS	100
#define COMMENTCHAR	'#'


extern char		*pptailor,
			*isodelogpath,
			*tbldfldir,
			*logdfldir;

static char		*getlines();
static void pp_tailor_export ();
static void pp_log_init ();
extern void err_abrt ();
static void pp_log_export ();
static int  tai_read();
extern void siginit ();
extern char chrcnv[];

/* ---------------------  Begin	 Routines  -------------------------------- */

#define LEXEQU(a,b)	(chrcnv[(a[0])] == chrcnv[(b[0])] && lexequ((a),(b))== 0)


int sys_init (name)
char	*name;
{
	return pp_initialise (name, TAI_ALL);
}

pp_initialise (name, flags)
char	*name;
int	flags;
{
	char		buf[BUFSIZ],
			*args[MAXTAIARGS],
			*lineptr,
			*rname;
	static char onceonly = 0;
	int		ac;


	if ((rname = rindex (name, '/')) != NULLCP)
		rname ++;
	if (rname == NULLCP || *rname == 0)
		rname = name;


	ll_hdinit (pp_log_oper, rname);
	ll_hdinit (pp_log_stat, rname);
	ll_hdinit (pp_log_norm, rname);
	isodetailor (rname, 0);

	if (onceonly ++)
		return OK;

	if (flags & TAI_SIGNALS)
		siginit();

	pp_log_init (flags);

	(void) sprintf (buf, "%s/", logdfldir);
	isodelogpath = strdup (buf);


	if (tai_read (pptailor) == NOTOK)
		err_abrt (RP_FIO,
			  "Lib/init_tai.c: Cannot open tailor file '%s'",
			  pptailor);

	while ((lineptr = getlines ()) != NULLCP) {
		if (*lineptr == '\0')
			continue;
		PP_DBG (("tailor line='%s'", buf));
		if ((ac = sstr2arg (lineptr, MAXTAIARGS,
				    args, " \t,")) == NOTOK)
			err_abrt (RP_MECH,
				  "Lib/init_tai.c: too many tailor params");

		if (ac <= 1)
			continue;

		
		if (LEXEQU (args[0], "chan") &&
		    chan_tai (ac, args) == OK)
			continue;

		if (LEXEQU (args[0], "tbl") &&
		    tbl_tai (ac, args) == OK)
			continue;
			
		if (sys_tai (ac, args) == OK)
			continue;

		if (LEXEQU (rname, args[0])) {
			PP_DBG (("Adding tailor %s for %s",
				 args[1], rname));
			(void) sys_tai (ac - 1, &args[1]);
		}
	}

	pp_tailor_export (flags);
	if (flags & TAI_LOGS)
		pp_log_export ();
	return OK;
}




/* ---------------------  Static  Routines  ------------------------------- */




static void pp_tailor_export (flags)
int	flags;
{
	extern char	*cmddfldir, *chndfldir, *formdfldir,
			*quedfldir,
			*ppdbm, *wrndfldir, *x400_mta, *loc_dom_mta;
	extern char	*dupfpath();
	char		buf[BUFSIZ];


	if (flags & TAI_LOGS) {
		if (isodelogpath)
			free (isodelogpath);

		(void) sprintf (buf, "%s/", logdfldir);
		isodelogpath = strdup (buf);
	}
	isodexport (NULLCP);

	chndfldir		= dupfpath (cmddfldir, chndfldir);
	formdfldir		= dupfpath (cmddfldir, formdfldir);
	ppdbm			= dupfpath (tbldfldir, ppdbm);
	wrndfldir		= dupfpath (tbldfldir, wrndfldir);
	if (x400_mta == NULLCP)
		x400_mta = strdup (loc_dom_mta);
}

static char *pp_tailor_str, *ppt_ptr;
static int tlineno;

static int tai_read (file)
char *file;
{
	int fd;
	struct stat st;

	tlineno = 1;
	if ((fd = open (file, O_RDONLY, 0)) == NOTOK)
		return NOTOK;
	if (fstat (fd, &st) == NOTOK) {
		(void) close (fd);
		return NOTOK;
	}
	ppt_ptr = pp_tailor_str = smalloc ((int)st.st_size + 1);
	if (read (fd, pp_tailor_str, (int)st.st_size) != st.st_size) {
		(void) close (fd);
		return NOTOK;
	}
	pp_tailor_str[st.st_size] = 0;
	(void) close (fd);
	return OK;
}



static char	*getlines ()
{
	register char *p = ppt_ptr;
	char *base;
	register char *lastp;

#define skipto(x)  while(*p && *p != (x)) p ++;

	while (*p == COMMENTCHAR || *p == '\n') {
		skipto ('\n');
		tlineno ++;
		if (p) p++;
	}

	if (*p == '\0') {
		ppt_ptr = p;
		return NULLCP;
	}

	base = p;
	lastp = NULLCP;
	while (1) {
		skipto ('\n');
		tlineno ++;
		if (*p == '\n')
			*p = ' ';
		else break;
		if (lastp == NULLCP) lastp = p;
		if (p[1] == COMMENTCHAR) {
			continue;
		} else if (p[1] == ' ' || p[1] == '\t') {
			if (lastp)
				while (lastp < p) *lastp++ = ' ';
			lastp = NULLCP;
			continue;
		}
		break;
	}
#undef skipto
	if (p && *p) *p++ = '\0';
	if (lastp) *lastp = '\0';
	ppt_ptr = p;
	return base;
}


static void tai_def_eh (str)
char *str;
{
	PP_LOG (LLOG_EXCEPTIONS, ("%s", str));
}

static VFP teh = tai_def_eh;

void tai_seterrorhandler (fnx)
VFP fnx;
{
	if (fnx == NULLVFP)
		teh = tai_def_eh;
	else
		teh = fnx;
}

#ifdef lint
/* VARARGS1 */
void tai_error (fmt)
char *fmt;
{
	tai_error (fmt);
}
#else
void tai_error (va_alist)
va_dcl
{
	char buf[BUFSIZ];
	char buf2[BUFSIZ];
	va_list ap;

	va_start (ap);
	_asprintf (buf, NULLCP, ap);
	(void) sprintf (buf2, "Tailor error in entry ending line %d: %s\n",
			tlineno, buf);
	(teh)(buf2);
	va_end (ap);
}
#endif
/* ------------------------------------------------------------------ */
struct logpairs {
	LLog	**isode_log;
	LLog	**isode_save;
	char	*name;
	char	*level;
};


#define NULLOG	((LLog **)0)

static LLog
#ifdef X25
	*log_x25,
#endif
	*log_acsap,
	*log_addr,
	*log_compat,
	*log_psap,
	*log_psap2,
	*log_rosap,
	*log_rtsap,
	*log_ssap,
	*log_tsap;

static struct logpairs logpairs[] = {
	&acsap_log,	&log_acsap,	"acsap",	"acsaplevel",
	&addr_log,	&log_addr,	"addr",		"addrlevel",
	&compat_log,	&log_compat,	"compat",	"compatlevel",
	&psap2_log,	&log_psap2,	"psap2",	"psap2level",
	&psap_log,	&log_psap,	"psap",		"psaplevel",
	&rosap_log,	&log_rosap,	"rosap",	"rosaplevel",
	&rtsap_log,	&log_rtsap,	"rtsap",	"rtsaplevel",
	&ssap_log,	&log_ssap,	"ssap",		"ssaplevel",
	&tsap_log,	&log_tsap,	"tsap",		"tsaplevel",
#ifdef X25
	&x25_log,	&log_x25,	"x25",		"x25level",
#endif
	0
};

static LLog zdummy = {
	"dummy.log", NULLCP, NULLCP,
	LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE, LLOG_FATAL, -1,
	LLOGCLS | LLOGCRT | LLOGZER, NOTOK
};
static LLog *dummy = &zdummy;


static void pp_log_init (flags)
int	flags;
{
	struct logpairs *lp;

	for (lp = logpairs; lp -> name; lp ++) {
		if (flags & TAI_LOGS)
			**(lp -> isode_log) = *pp_log_norm;
		*(lp -> isode_save) = *(lp -> isode_log);
		if (flags & TAI_LOGS)
			*(lp -> isode_log) = dummy;
	}
}

static void pp_log_export()
{
	struct logpairs *lp;

	for (lp = logpairs; lp -> name; lp++)
		*(lp -> isode_log) = *(lp -> isode_save);
}

void pp_setlog (argv, argc)
char	*argv[];
int	argc;
{
	struct logpairs *lp;

	if (argc < 1)
		return;

	for (lp = logpairs; lp -> name; lp ++) {
		if (lexequ (argv[0], lp -> level) == 0)
			break;
	}

	if (lp -> name == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, ("level %s not found",
					  argv[0]));
		return;
	}
	log_tai (*(lp -> isode_save), &argv[1], argc - 1);
}
