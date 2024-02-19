/* misc.c: misc routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/misc.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/misc.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: misc.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */
#include "Qmgr-types.h"
#include <isode/acsap.h>
#include "console.h"
#include	<pwd.h>
#include 	<sys/time.h>

char	*host = NULLCP, tailorfile[MAXPATHLENGTH];
char	*actual_host = NULLCP;
char	*username = NULLCP;
int	authorised = FALSE;
extern char	*cmd_argv[];
extern int	cmd_argc, bypassHostQuestions, connected;

init_host()
{
	char	buf[BUFSIZ];
	int	uid;
	struct passwd *pwd;
	uid = getuid();
	pwd = getpwuid(uid);
	username = strdup(pwd->pw_name);
	strcpy(tailorfile, ".MTAconsole");
	gethostname(buf, BUFSIZ);
	host = strdup(buf);

}

set_host()
{
	int	cont = TRUE;
	char	buf[BUFSIZ];
	char	*password = NULLCP;
	
	if (bypassHostQuestions == TRUE) {
		bypassHostQuestions = FALSE;
		cont = FALSE;
	} else if (cmd_argc > 1) {
		if (host) free(host);
		host = strdup(cmd_argv[1]);
		cont = FALSE;
	}

	while (cont == TRUE) {
		fprintf(stdout, "host (%s) = ",
			(host == NULLCP) ? "" : host);
		fflush(stdout);
		if (gets(buf) == NULL)
			exit (OK);
		compress(buf, buf);
		if (emptyStr(buf)) {
			if (host == NULL)
				fprintf(stdout, "no host set\n");
			else 
				/* happy with the one we've got */
				cont = FALSE;
		} else {
			if (host) free(host);
			host = strdup(buf);
			cont = FALSE;
		}
	}

	if (authorised == TRUE) {
		if (password) free(password);
		password = strdup(getpassword("password = "));
	}
	fillin_passwdpep(username, password, authorised);
	if (password) free(password);
}

extern char	*Qversion, *Qinformation;

set_authmode (acc)
struct AcSAPconnect   	*acc;
{
	struct type_Qmgr_BindResult	*br;
	
	br = NULL;
	authorised = FALSE;
	if (Qversion != NULLCP) {
		free(Qversion);
		Qversion = NULLCP;
	}
	if (Qinformation != NULLCP) {
		free (Qinformation);
		Qinformation = NULLCP;
	}

	if (acc->acc_ninfo >= 1) {
		if (decode_Qmgr_BindResult (acc->acc_info[0], 1,
					    NULLVP, NULLIP, &br) == NOTOK) 
			PP_LOG(LLOG_EXCEPTIONS, ("failed to parse connect data [%s]",
						 PY_pepy));
		else {
			switch (br->result) {
			    case int_Qmgr_result_acceptedFullAccess:
				authorised = TRUE;
				break;
			    case int_Qmgr_result_acceptedLimitedAccess:
			    default:
				authorised = FALSE;
				break;
			}
			if (br -> information != NULL) 
				Qinformation = qb2str (br -> information);

			if (br -> version != NULL) 
				Qversion = qb2str (br -> version);

			free_Qmgr_BindResult(br);
		}
	}
}

/*  */

int	uk_order = 0;

char	*
reverse_adr(adr)
char	*adr;
{
	extern char	*rindex();

	char	*ret = malloc(strlen(adr) + 1), savech;
	char	*ix = adr, *iy = ret, *dom_end, *cix, *last, *dot;
	int	cont;
	while (*ix != '\0') {
		switch (*ix) {
		    case '@':
		    case '%':
			/* start of domain so reverse */
			*iy++ = *ix++;
			dom_end = ix;
			cont = TRUE;
			while (*dom_end != '\0'
			       && cont == TRUE) {
				switch(*dom_end) {
				    case ',':
				    case ':':
					cont = FALSE;
					break;
				    default:
					dom_end++;
					break;
				}
			}
			/* domain stretches from ix -> dom_end */

			last = dom_end;
			while (last != ix) {
				savech = *last;
				*last = '\0';
				if ((dot = rindex(ix, '.')) == NULLCP) {
					dot = ix;
					cix =  dot;
				} else 
					cix = dot + 1;
				*last = savech;
				do 
				{
					*iy++ = *cix++;
				} while (cix != last);
				
				if (dot != ix) {
					*iy++ = '.';
					last = dot;
				} else
					last = ix;
			}
			ix = dom_end;
			break;
				
		    default:			
			*iy++ = *ix++;
			break;
		}
	}
	*iy = '\0';
	return ret;
}

char	*
reverse_mta(mta)
char	*mta;
{
	extern	char	*index();
	char *adr, *ret, *ix;
	char	buf[BUFSIZ];
	(void) sprintf(buf, "foo@%s", mta);
	ret = reverse_adr(buf);
	ix = index (ret, '@');
	adr = strdup(++ix);
	free(ret);
	return adr;
}

resize_info_str(pold, poldlen, inc)
char		**pold;
int		*poldlen,
		inc;
{
	if ((int)strlen(*pold) + inc >= *poldlen) {
		*poldlen += BUFSIZ;
		*pold = realloc(*pold, (unsigned) *poldlen);
	}
}

char	*time_t2RFC(in)
time_t	in;
{
	char	buf[BUFSIZ];
	struct tm *tm;
	struct UTCtime uts;
#ifdef SVR4
	extern long timezone;
#else
	struct timeval dummy;
	struct timezone time_zone;
#endif
	
	tm = localtime (&in);
	tm2ut (tm, &uts);
	uts.ut_flags |= UT_ZONE;
#ifdef SVR4
	uts.ut_zone = timezone / 60;
#else
	gettimeofday(&dummy, &time_zone);
	uts.ut_zone = time_zone.tz_minuteswest;
#endif
	UTC2rfc(&uts, buf);
	return strdup(buf);
}

/* convert number to volume */

char	*vol2str(vol)
int	vol;
{
	char	buf[BUFSIZ];

	if (vol < 1024)
		sprintf(buf, "%d bytes", vol);
	else if (vol < 1048576)
		sprintf(buf, "%d Kbytes", (vol/1024));
	else 
		sprintf(buf, "%d Mbytes", (vol/1048576));
	return strdup(buf);
}

char	*time_t2str(in)
time_t  in;
{
	char	buf[BUFSIZ];
	time_t	result;
	buf[0] = '\0';
	
	if (in < 0)
		return strdup("still in the womb");

	if ((result = in / (60 * 60 * 24)) != 0) {
		sprintf(buf, "%d day%s",
			result,
			(result == 1) ? "" : "s");
		in = in % (60 * 60 * 24);
	}

	if ((result = in / (60 * 60)) != 0) {
		sprintf(buf, 
			(buf[0] == '\0') ? "%s%d hr%s" : "%s %d hr%s",
			buf, 
			result,
			(result == 1) ? "" : "s");
		in = in % (60 * 60);
	}
	if ((result = in / 60) != 0) {
		sprintf(buf, 
			(buf[0] == '\0') ? "%s%d min%s" : "%s %d min%s",
			buf, 
			result,
			(result == 1) ? "" : "s");
		in = in % 60;
	}

	if (buf[0] == '\0' && in != 0)
		sprintf(buf, "%d sec%s", 
			in,
			(in == 1) ? "" : "s");
	if (buf[0] == '\0')
		sprintf(buf, "just born");
	return strdup(buf);
}

#define MaxCharPerInt 16

/* convert integer to string */
char	*itoa(i)
int 	i;
{
	char 	buf[MaxCharPerInt];

	sprintf(buf,"%d",i);
	
	return strdup(buf);
	
}

num2unit(num, buf)
int	num;
char	*buf;
{
	if (num > 1000000)
		(void) sprintf(buf, "%.2f M", 
			       (double) num / 1000000.0);
	else if (num > 1000)
		(void) sprintf(buf, "%.2f k", 
			       (double) num / 1000.0);
	else
		(void) sprintf(buf, "%d", num);
}

time_t parsetime(s)
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

char *mystrtotime(str)
char	*str;
{
	UTC	utc;
	time_t	newsecs;
	time_t	 current;
	char	*retval;

	newsecs = parsetime(str);
	time(&current);
	
	current += newsecs;
		
	utc = time_t2utc(current);

	retval = utct2str(utc);
	free((char *) utc);
	return strdup(retval);
}

char *lowerfy (s)
char	*s;
{
	static char	buf[BUFSIZ];
	char	*cp;

	for (cp = buf; *s; s++)
		if (isascii (*s) && isupper (*s))
			*cp++ = tolower (*s);
		else
			*cp ++ = *s;
	*cp = NULL;
	return buf;
}

emptyStr(buf)
char	*buf;
{
	return (isspace(buf[0]) || buf[0] == '\0');
}

/*  */

/* ARGSUSED */
int do_quecontrol (ad, ds, args, arg)
int			ad;
struct client_dispatch	*ds;
int			args;
struct type_Qmgr_QMGROp **arg;
{
	*arg = (struct type_Qmgr_QMGROp *) malloc(sizeof(**arg));
	(*arg)->parm = args;
	return OK;
}

/* ARGSUSED */
int quecontrol_result (ad, id, dummy, result, roi)
int	ad,
	id,
	dummy;
struct type_Qmgr_Pseudo__qmgrControl	*result;
struct RoSAPindication			*roi;
{
	return OK;
}

static CMD_TABLE	tb_controls [] = { /* quecontrol options */
	"abort",		int_Qmgr_QMGROp_abort,
	"abor",			int_Qmgr_QMGROp_abort,
	"abo",			int_Qmgr_QMGROp_abort,
	"ab",			int_Qmgr_QMGROp_abort,
	"a",			int_Qmgr_QMGROp_abort,

	"decreasemaxchans",	int_Qmgr_QMGROp_decreasemaxchans,
	"decreasemaxchan",	int_Qmgr_QMGROp_decreasemaxchans,
	"decreasemaxcha",	int_Qmgr_QMGROp_decreasemaxchans,
	"decreasemaxch",	int_Qmgr_QMGROp_decreasemaxchans,
	"decreasemaxc",		int_Qmgr_QMGROp_decreasemaxchans,
	"decreasemax",		int_Qmgr_QMGROp_decreasemaxchans,
	"decreasema",		int_Qmgr_QMGROp_decreasemaxchans,
	"decreasem",		int_Qmgr_QMGROp_decreasemaxchans,
	"decrease",		int_Qmgr_QMGROp_decreasemaxchans,
	"decreas",		int_Qmgr_QMGROp_decreasemaxchans,
	"decrea",		int_Qmgr_QMGROp_decreasemaxchans,
	"decre",		int_Qmgr_QMGROp_decreasemaxchans,
	"decr",			int_Qmgr_QMGROp_decreasemaxchans,
	"dec",			int_Qmgr_QMGROp_decreasemaxchans,
	"de",			int_Qmgr_QMGROp_decreasemaxchans,

	"disableAll",		int_Qmgr_QMGROp_disableAll,
	"disableAl",		int_Qmgr_QMGROp_disableAll,
	"disableA",		int_Qmgr_QMGROp_disableAll,

	"disableSubmission", 	int_Qmgr_QMGROp_disableSubmission,
	"disableSubmissio", 	int_Qmgr_QMGROp_disableSubmission,
	"disableSubmissi", 	int_Qmgr_QMGROp_disableSubmission,
	"disableSubmiss", 	int_Qmgr_QMGROp_disableSubmission,
	"disableSubmis", 	int_Qmgr_QMGROp_disableSubmission,
	"disableSubmi", 	int_Qmgr_QMGROp_disableSubmission,
	"disableSubm", 		int_Qmgr_QMGROp_disableSubmission,
	"disableSub", 		int_Qmgr_QMGROp_disableSubmission,
	"disableSu", 		int_Qmgr_QMGROp_disableSubmission,
	"disableS", 		int_Qmgr_QMGROp_disableSubmission,

	"enableAll",		int_Qmgr_QMGROp_enableAll,
	"enableAl",		int_Qmgr_QMGROp_enableAll,
	"enableA",		int_Qmgr_QMGROp_enableAll,

	"enableSubmission",	int_Qmgr_QMGROp_enableSubmission,
	"enableSubmissio",	int_Qmgr_QMGROp_enableSubmission,
	"enableSubmissi",	int_Qmgr_QMGROp_enableSubmission,
	"enableSubmiss",	int_Qmgr_QMGROp_enableSubmission,
	"enableSubmis",		int_Qmgr_QMGROp_enableSubmission,
	"enableSubmi",		int_Qmgr_QMGROp_enableSubmission,
	"enableSubm",		int_Qmgr_QMGROp_enableSubmission,
	"enableSub",		int_Qmgr_QMGROp_enableSubmission,
	"enableSu",		int_Qmgr_QMGROp_enableSubmission,
	"enableS",		int_Qmgr_QMGROp_enableSubmission,

	"increasemaxchans",	int_Qmgr_QMGROp_increasemaxchans,
	"increasemaxchan",	int_Qmgr_QMGROp_increasemaxchans,
	"increasemaxcha",	int_Qmgr_QMGROp_increasemaxchans,
	"increasemaxch",	int_Qmgr_QMGROp_increasemaxchans,
	"increasemaxc",		int_Qmgr_QMGROp_increasemaxchans,
	"increasemax",		int_Qmgr_QMGROp_increasemaxchans,
	"increasema",		int_Qmgr_QMGROp_increasemaxchans,
	"increasem",		int_Qmgr_QMGROp_increasemaxchans,
	"increase",		int_Qmgr_QMGROp_increasemaxchans,
	"increas",		int_Qmgr_QMGROp_increasemaxchans,
	"increa",		int_Qmgr_QMGROp_increasemaxchans,
	"incre",		int_Qmgr_QMGROp_increasemaxchans,
	"incr",			int_Qmgr_QMGROp_increasemaxchans,
	"inc",			int_Qmgr_QMGROp_increasemaxchans,
	"in",			int_Qmgr_QMGROp_increasemaxchans,
	"i",			int_Qmgr_QMGROp_increasemaxchans,

	"rereadQueue",		int_Qmgr_QMGROp_rereadQueue,
	"rereadQueu",		int_Qmgr_QMGROp_rereadQueue,
	"rereadQue",		int_Qmgr_QMGROp_rereadQueue,
	"rereadQu",		int_Qmgr_QMGROp_rereadQueue,
	"rereadQ",		int_Qmgr_QMGROp_rereadQueue,
	"reread",		int_Qmgr_QMGROp_rereadQueue,
	"rerea",		int_Qmgr_QMGROp_rereadQueue,
	"rere",			int_Qmgr_QMGROp_rereadQueue,
	"rer",			int_Qmgr_QMGROp_rereadQueue,


	"restart",		int_Qmgr_QMGROp_restart,
	"restar",		int_Qmgr_QMGROp_restart,
	"resta",		int_Qmgr_QMGROp_restart,
	"rest",			int_Qmgr_QMGROp_restart,
	"res",			int_Qmgr_QMGROp_restart,

	"terminate",		int_Qmgr_QMGROp_gracefulTerminate,
	"terminat",		int_Qmgr_QMGROp_gracefulTerminate,
	"termina",		int_Qmgr_QMGROp_gracefulTerminate,
	"termin",		int_Qmgr_QMGROp_gracefulTerminate,
	"termi",		int_Qmgr_QMGROp_gracefulTerminate,
	"term",			int_Qmgr_QMGROp_gracefulTerminate,
	"ter",			int_Qmgr_QMGROp_gracefulTerminate,
	"te",			int_Qmgr_QMGROp_gracefulTerminate,
	"t",			int_Qmgr_QMGROp_gracefulTerminate,

	0,		-1
	};


static int str2control(str)
char	*str;
{
	Command	ret;
	char	*ix = str, savech, *start;
	
	while (*ix != '\0' && isspace(*ix)) ix++;
	start = ix;
	while (*ix != '\0' && !isspace(*ix)) ix++;
	savech = *ix;
	*ix = '\0';
	
	ret = (Command) cmd_srch(start, tb_controls);
	*ix = savech;
	return ret;
}

static int control_options()
{
	fprintf(stdout,
		"Qmgr Controls available are:");
	fprintf(stdout,
		"abort, terminate, restart, rereadQueue,\n\tdisableSubmission, enableSubmission, disableAll, enableAll, increasemaxchans, and decreasemaxchans\n");
}

static int get_control(pcontrol)
int	*pcontrol;
{
	int	cont = TRUE, first = TRUE;
	int	ret = NOTOK, control;
	char	buf[BUFSIZ];

	while (cont == TRUE) {
		if (cmd_argc > 1 && first == TRUE) {
			strcpy(buf, cmd_argv[1]);
			first = FALSE;
		} else {
			fprintf(stdout, "quecontrol (q=quit)> ");
			fflush (stdout);
		
			if (gets(buf) == NULL)
				exit(OK);
		}
		compress(buf, buf);
		
		if (buf[0] == 'q') {
			/* use same command as before */
			cont = FALSE;
		} else if ((control = str2control(buf)) < 0) {
			fprintf(stdout,
				"Unknown control '%s'\n", buf);
			control_options();
		} else {
			*pcontrol = control;
			cont = FALSE;
			ret = OK;
		}
	}

	return ret;
}

Qcontrol()
{
	int	control;
	
	if (get_control(&control) == OK) 
		my_invoke(quecontrol, (char **) control);
}

extern char	*pager;
extern int	pageron;

#define PAGER	1
#define TAILORFILE 	2
#define AUTHORISED	3
#define USER		4

static CMD_TABLE	tb_variables [] = { /* line console variables */
	"pager",	PAGER,
	"tailorfile",	TAILORFILE,
	"authorised",	AUTHORISED,
	"user",		USER,
	0, 		-1
	};

print_variables()
{
	if (pager) {
		fprintf(stdout,
			"pager='%s' (%s)\n",
			pager,
			(pageron == TRUE) ? "on" : "off");
	} else
		fprintf(stdout,
			"no pager set\n");
	if (tailorfile[0] != '\0')
		fprintf(stdout,
			"tailorfile='%s'\n",
			tailorfile);
	else
		fprintf(stdout,
			"no tailorfile set\n");

	fprintf(stdout,
		"authorised=%s\n",
		(authorised == TRUE) ? "yes" : "no");
	fprintf(stdout,
		"user='%s'\n",
		username);
}

str2variable(str)
char	*str;
{
	return cmd_srch(str, tb_variables);
}

variables()
{
	if (cmd_argc > 1) {
		int i;
		char	*ix;
		for (i = 1; i < cmd_argc; i++) {
			if ((ix = index(cmd_argv[i], '=')) == NULLCP) 
				fprintf(stderr,
					"unable to parse '%s' as variable=value\n",
					cmd_argv[i]);
			else {
				*ix = '\0';
				switch (str2variable(cmd_argv[i])) {
				    case PAGER:
					*ix++ = '=';
					if (*ix == '\0'
					    || lexequ(ix, "-") == 0) {
						if (pager) free(pager);
						pager = NULLCP;
					} else if (lexequ(ix, "on") == 0)
						pageron = TRUE;
					else if (lexequ(ix, "off") == 0)
						pageron = FALSE;
					else {
						if (pager) free(pager);
						pager = strdup(ix);
					}
					break;

				    case TAILORFILE:
					if (connected == TRUE) 
						fprintf(stderr,
							"cannot change tailorfile while connected\n");
					else {
						*ix++ = '=';
						strcpy(tailorfile, ix);
					} 
					break;

				    case USER:
					if (connected == TRUE)
						fprintf(stderr,
							"cannot change username while connected\n");
					else {
						*ix++ = '=';
						if (username) free(username);
						username = strdup(ix);
					}
					break;

				    case AUTHORISED:
					*ix++ = '=';
					switch(*ix) {
					    case 'y':
					    case 'Y':
						if (connected == TRUE)
							fprintf(stderr,
								"cannot turn authorised while connected\n");
						else 
							authorised = TRUE;
						break;
					    case 'n':
					    case 'N':
						authorised = FALSE;
						break;
					    default:
						fprintf(stdout, 
							"Don't recognise boolean value '%s'\n",
							ix);
					}
					break;
							
				    default:
					fprintf(stderr,
						"unknown variable '%s' ",
						cmd_argv[i]);
					*ix = '=';
					fprintf(stderr,
						"in '%s'\n",
						cmd_argv[i]);
					break;
				}
			}
		}
	}
	print_variables();
}
