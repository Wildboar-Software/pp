/* rfc822norm: program to 822norm stdin to stdout */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc822norm/RCS/rfc822norm.c,v 6.0 1991/12/18 20:20:54 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc822norm/RCS/rfc822norm.c,v 6.0 1991/12/18 20:20:54 jpo Rel $
 *
 * $Log: rfc822norm.c,v $
 * Revision 6.0  1991/12/18  20:20:54  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	<isode/general.h>
#include	<isode/cmd_srch.h>
#include	"ap.h"
#include	"alias.h"
#include	"chan.h"
#include	"retcode.h"
#include	"adr.h"

#define	OPT_822		1
#define OPT_733		2	
#define	OPT_JNT		3
#define	OPT_BIGEND	4
#define OPT_LITTLEEND	5
#define	OPT_STRIPROUTES	6
#define OPT_STRIPDOMAIN	7
#define	OPT_STRIPTRACE	8
#define OPT_JNTSENDER	9
#define OPT_FOLD	10
#define OPT_MSGID	11
#define OPT_PERCENT	12
#define OPT_HIDELOCAL	13
#define OPT_STRIPACK	14
#define OPT_GENACK	15
#define OPT_CHANGEDOMAIN 16
#define OPT_FULL	17
#define OPT_EXORDOM	18
#define	OPT_VALDOM	19
#define OPT_EXORCISE	20
#define OPT_EXTERNAL	21
#define OPT_INTERNAL	22

CMD_TABLE	tbl_options [] = { /* rfc822norm commandline options */
	"-822",		OPT_822,
	"-733",		OPT_733,
	"-jnt",		OPT_JNT,
	"-bigend",	OPT_BIGEND,
	"-littleend",	OPT_LITTLEEND,
	"-striproutes",	OPT_STRIPROUTES,
	"-stripdomain",	OPT_STRIPDOMAIN,
	"-changedomain", OPT_CHANGEDOMAIN,
	"-striptrace",	OPT_STRIPTRACE,
	"-jntsender",	OPT_JNTSENDER,
	"-fold",	OPT_FOLD,
	"-msgid",	OPT_MSGID,
	"-percent",	OPT_PERCENT,
	"-full",	OPT_FULL,
	"-hidelocal",	OPT_HIDELOCAL,
	"-stripack",	OPT_STRIPACK,
	"-acks",	OPT_GENACK,
	"-exorcise",		OPT_EXORCISE,
	"-exorcise-domain", 	OPT_EXORDOM,
	"-valid-domains",	OPT_VALDOM,
	"-external",	OPT_EXTERNAL,
	"-internal",	OPT_INTERNAL,
	0,		-1
	};

#define	FLD_TO			1
#define	FLD_CC			2
#define	FLD_BCC			3
#define	FLD_FROM		4
#define	FLD_SENDER		5
#define FLD_REPLY_TO		6
#define	FLD_RESENT_FROM		7
#define FLD_RESENT_SENDER	8
#define	FLD_RESENT_TO		9
#define	FLD_RESENT_CC		10
#define	FLD_RESENT_BCC		11
#define	FLD_RESENT_BY		12
#define	FLD_REMAILED_FROM	13
#define FLD_REMAILED_TO		14
#define FLD_REMAILED_BY		15
#define FLD_REDISTRIBUTED_FROM	16
#define FLD_REDISTRIBUTED_TO	17
#define FLD_REDISTRIBUTED_BY	18
#define FLD_ORIG_SENDER		19
/* RFC 987 extensions */
#define FLD_RFC987		20
/* RFC 987 (88) extensions - proposed */
#define FLD_RFC987_88		21
#define FLD_JNT_ACK		22
/* misc */
#define FLD_ERRORS_TO		23
#define FLD_DATE		24

CMD_TABLE	tbl_fields [] = {/* address field names */
/* REAL RFC 822 */
	"Date",			FLD_DATE,
	"To",			FLD_TO,
	"CC",			FLD_CC,
	"bcc",			FLD_BCC,
	"From",			FLD_FROM,
	"Sender",		FLD_SENDER,
	"Reply-to",		FLD_REPLY_TO,
	"Resent-From",		FLD_RESENT_FROM,
	"Resent-Sender",	FLD_RESENT_SENDER,
	"Resent-To",		FLD_RESENT_TO,
	"Resent-Cc",		FLD_RESENT_CC,
	"Resent-Bcc",		FLD_RESENT_BCC,
	"Resent-By",		FLD_RESENT_BY,
/* JNT stuff */
	"Original-Sender",	FLD_ORIG_SENDER,
	"Acknowledge-To",	FLD_JNT_ACK,
/* RFC 733 & other misc stuff */
	"Remailed-From",	FLD_REMAILED_FROM,
	"Remailed-To",		FLD_REMAILED_TO,
	"Remailed-By",		FLD_REMAILED_BY,
	"Redistributed-From",	FLD_REDISTRIBUTED_FROM,
	"Redistributed-To",	FLD_REDISTRIBUTED_TO,
	"Redistributed-By",	FLD_REDISTRIBUTED_BY,
	"Errors-To",		FLD_ERRORS_TO,
/* RFC 987 fields */
	"P1-Recipient",		FLD_RFC987,
/* RFC 987 (88) fields */
	"X400-Originator",			FLD_RFC987_88,
	"X400-Recipients",			FLD_RFC987_88,
	"Notification-IPM-Originator",		FLD_RFC987_88,
	"Notification-Preferred-Recipients",	FLD_RFC987_88,
	"MTS-Originator",			FLD_RFC987_88,
	"MTS-Recipient",			FLD_RFC987_88,
	"Originally-Intended-Recipient",	FLD_RFC987_88,
	"Originator-Return-Address",		FLD_RFC987_88,
	"Report-Reporting-DL-Name",		FLD_RFC987_88,
	"Report-Originator-and-DL-Expansion-History",
						FLD_RFC987_88,
	0,			-1
	};

#define FOLD_SPACE	0
#define FOLD_COMMA	1
#define FOLD_SEMICOLON	2
#define	FOLD_RECIEVED	3
#define FOLD_NONE	4

CMD_TABLE	tbl_nonfields [] = {/* feilds dont deal with */
	"Received",		FOLD_RECIEVED,
	"X400-Received",	FOLD_SEMICOLON,
	"Via",			FOLD_SEMICOLON,
	"References",		FOLD_COMMA,
	"Keywords",		FOLD_COMMA,
	"X400-MTS-Identifier",	FOLD_NONE,
	"Message-ID",		FOLD_NONE,
	"In-Reply-To",		FOLD_NONE,
	"Obsoletes",		FOLD_COMMA,
	0,			-1
	};

#define TRACE_RECIEVED	1
#define TRACE_VIA	2
#define TRACE_X400	3

CMD_TABLE	tbl_tracefields [] = {/* fields viable for strip trace */
	"Received",		TRACE_RECIEVED,
	"Via",			TRACE_VIA,
	"X400-Received",	TRACE_X400,
	0,			-1
};

typedef struct dom_pair {
	AP_ptr	from;
	AP_ptr	to;
} DomPair;

typedef enum {maj_none, rfc822, rfc733, jnt} Major_options;
typedef enum {min_none, bigend, littleend} Minor_options;

extern void sys_init(), err_abrt();
static void norm_sender();
static int equalWithJntsender();
#ifdef VAT
static int tidy_up();
#endif
static getitm(), out_adr();

char		*myname;
int		nadrs;
int		pcol;
int		nonempty;
int		fold_width;
int		order_pref, percents;
int		normalised = APARSE_NORM_NONE;
int		striptrace = FALSE,
		striproutes = FALSE,
		stripacks = FALSE,
		genacks = FALSE;
char		**stripdomains = NULL;
char		**hidedomains = NULL;
struct dom_pair	*changedomains = NULL;
int		num_domains = 0,
		num_pairs = 0,
		num_to_hide = 0,
		message_id = 0,
		acks = 0,
		msgid_req = FALSE,
		internal = FALSE;
char		*jntsender = NULL,
		*jntsendernorm = NULL;
AP_ptr		jnttree, jntgroup, jntname, jntloc, jntdom, jntroute;
static int	getach();
static int	getbufchar();
static char	*next_fold();
static char	*fold_recieved();
extern AP_ptr	ap_pinit();
extern char	*compress();
extern char	*rcmd_srch();
extern int	ap_outtype;
extern int	ap_perlev;
extern char	*loc_dom_site,
		*loc_dom_mta;
char		*rloc_dom_site,
		*rloc_dom_mta;
static char	*reverse();
#ifndef BSD42
#define random rand
#define srandom srand
#endif

#define	DEFAULT_FOLD_WIDTH	79

static char	*fieldbuf = NULL,
		*contbuf = NULL;
static int	fieldsize = 0,
		contsize = 0;
static int	fieldlen = 0,
		contlen = 0;
static char	*contix = NULL;

static int	exorcise = FALSE;
static Table	*exorciseTable;
static char	*exorciseDomain;

/* ARGSUSED */
main(argc,argv)
int	argc;
char	**argv;
{
	/* parse flags */
	Major_options maj = maj_none;
	Minor_options mino = min_none;

	myname = *argv++;
	sys_init(myname);
	or_myinit();
/*	malloc_debug(2);*/
	ap_outtype = AP_PARSE_822;
	fold_width = DEFAULT_FOLD_WIDTH;
	order_pref = CH_USA_PREF;
	normalised = APARSE_NORM_NONE;
	jntsendernorm = NULL;
	percents = FALSE;
	jntsender = NULL;
	message_id = 0;
	msgid_req = FALSE;
	srandom(getpid());

	exorcise = FALSE;
	exorciseDomain = strdup(loc_dom_site);
	exorciseTable = tb_nm2struct ("domain");

	rloc_dom_site = reverse(loc_dom_site);
	rloc_dom_mta = reverse(loc_dom_mta);
	while (*argv != NULL) {
		switch(cmd_srch(*argv,tbl_options)) {
		    case -1:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("unknown option '%s'",*argv));
			exit(1);
			
		    case OPT_MSGID:
			msgid_req = TRUE;
			break;

		    case OPT_PERCENT:
			percents = TRUE;
			ap_use_percent();
			break;
			
		    case OPT_FULL:
			normalised = APARSE_NORM_ALL;
			break;

		    case OPT_FOLD:
			if (*(argv+1) == NULL)
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no fold width given with %s",*argv));
			else {
				++argv;
				fold_width = atoi(*argv);
			}
			break;

		    case OPT_822:
			if ((maj == maj_none) || (maj == rfc822)) {
				ap_outtype &= AP_TYPE_MASK;
				ap_outtype |= AP_PARSE_822;
				maj = rfc822;
			}
			break;

		    case OPT_733:
			if ((maj == maj_none) || (maj == rfc733)) {
				ap_outtype &= AP_TYPE_MASK;
				ap_outtype |= AP_PARSE_733;
				maj = rfc733;
			} 
			break;

		    case OPT_JNT:
			if ((maj == maj_none || maj == jnt)
			    && (mino == min_none || mino == bigend)) {
				ap_outtype &= AP_TYPE_MASK;
				ap_outtype |= AP_PARSE_733;
				maj = jnt;
				ap_outtype |= AP_PARSE_BIG;
				mino = bigend;
				order_pref = CH_UK_PREF;
				break;
			}
			PP_LOG(LLOG_EXCEPTIONS,
			       ("multiple major parse options"));
			exit(1);

		    case OPT_BIGEND:
			if (mino == min_none || mino == bigend) {
				ap_outtype |= AP_PARSE_BIG;
				mino = bigend;
				order_pref = CH_UK_PREF;
			}
			break;

		    case OPT_LITTLEEND:
			if (mino == min_none || mino == littleend) {
				mino = littleend;
				break;
			}
			PP_LOG(LLOG_EXCEPTIONS,
			       ("multiple minor parse options"));
			exit(1);

		    case OPT_STRIPTRACE:
			striptrace = TRUE;
			break;

		    case OPT_STRIPROUTES:
			striproutes = TRUE;
			break;

		    case OPT_STRIPDOMAIN:
			if (*(argv+1) == NULL)
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no domain specified with %s", *argv));
			else {
				++argv;
				if (num_domains == 0)
					stripdomains = (char **) calloc(1, 
						(unsigned int) sizeof(char *));
				else
					stripdomains = (char **) realloc((char *) stripdomains,
									 (unsigned int) ((num_domains + 1) * sizeof(char *)));
				stripdomains[num_domains++] = *argv;
			}
			break;

		    case OPT_CHANGEDOMAIN:
			if (*(argv+1) == NULL
			    || *(argv+2) == NULL)
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no domain pair specified with %s",
					*argv));
			else {
				if (num_pairs == 0)
					changedomains = (struct dom_pair *)
						calloc (1, 
							(unsigned int) sizeof(struct dom_pair));
				else
					changedomains = (struct dom_pair *)
						realloc ((char *) changedomains,
							 (unsigned int) ((num_pairs + 1) * sizeof (char *)));
				argv++;
				changedomains[num_pairs].from = ap_new(AP_DOMAIN,
								       *argv++);
				changedomains[num_pairs++].to = ap_new(AP_DOMAIN,
								       *argv);
			}
			break;
				
		    case OPT_JNTSENDER:
			if (*(argv+1) == NULL)
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no sender specified with %s", *argv));
			else {
				argv++;
				jntsender = *argv;
			}
			break;

		    case OPT_STRIPACK:
			stripacks = TRUE;
			break;

		    case OPT_GENACK:
			if (*(argv+1) == NULL)
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no flag specified with %s", *argv));
			else {
				argv++;
				if (lexequ(*argv, "yes") == 0)
					genacks = TRUE;
			}
			break;

		    case OPT_HIDELOCAL:
			if (*(argv+1) == NULL)
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no domain specified with %s", *argv));
			else {
				++argv;
				if (num_to_hide == 0)
					hidedomains = (char **) calloc(1, 
						(unsigned int) sizeof(char *));
				else
					hidedomains = (char **) realloc((char *) hidedomains,
									 (unsigned int) ((num_to_hide + 1) * sizeof(char *)));
				hidedomains[num_to_hide++] = *argv;
			}
			break;

		    case OPT_EXORCISE:
			exorcise = TRUE;
			break;

		    case OPT_EXORDOM:
			if (*(argv+1) == NULL) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no domain given with %s", *argv));
				exit(1);
			}		
			++argv;
			free(exorciseDomain);
			exorciseDomain = strdup(*argv);
			exorcise = TRUE;
			break;

		    case OPT_VALDOM:
			if (*(argv+1) == NULL) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("no table given with %s", *argv));
				exit(1);
			} else if ((exorciseTable = tb_nm2struct(*(argv+1))) == NULLTBL) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("unknown table %s given with %s",
					*(argv+1), *argv));
				exit(1);
			}
			++argv;
			exorcise = TRUE;
			break;

		    case OPT_EXTERNAL:
			internal = FALSE;
			break;
			
		    case OPT_INTERNAL:
			internal = TRUE;
			break;

		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("unknown option '%s'"));
			exit(1);
		}
		argv++;
	}

	if (genacks == TRUE && !jntsender)
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Acknowledge-To required but no sender specified"));
	if (jntsender)
		norm_sender();
	/* ap_outtype set so now process */
	if (proc() != OK)
		err_abrt( RP_LIO, "couldn't reformat it");
	free(rloc_dom_site);
	free(rloc_dom_mta);
	/* copy out rest of file */
	exit(0);
}

/*  */

static void	norm_sender()
{
	Aparse_ptr	aparse = aparse_new();
	aparse -> r822_str = strdup(jntsender);
	aparse -> ad_type = AD_822_TYPE;
	aparse -> dmnorder = order_pref;
	aparse -> normalised = normalised;
	aparse -> percents = percents;
	aparse -> internal = internal;

	if (aparse_norm(aparse) == NOTOK) {
		jnttree = ap_s2t(jntsender);

		jnttree = ap_t2p (jnttree, &jntgroup, &jntname,
				  &jntloc, &jntdom, &jntroute);
	} else {
		jnttree = aparse->ap_tree;
		jntgroup = aparse->ap_group;
		jntname = aparse->ap_name;
		jntloc = aparse->ap_local;
		jntdom = aparse->ap_domain;
		jntroute = aparse->ap_route;
		aparse -> ap_tree = NULLAP;
	}
	aparse_free(aparse);
	free ((char *) aparse);
	jntsendernorm = ap_p2s(jntgroup, jntname, jntloc, jntdom, jntroute);
}

static int equalWithJntsender(tree)
AP_ptr	tree;
{
	AP_ptr	jix = tree, aix = jntloc;

	while (jix != NULLAP && 
	       aix != NULLAP &&
	       aix -> ap_obvalue != NULLCP &&
	       jix -> ap_obvalue != NULLCP) {
		if (jix -> ap_obtype == AP_COMMENT)
			jix = jix -> ap_next;
		else if (aix -> ap_obtype == AP_COMMENT)
			aix = aix -> ap_next;
		else if (strcmp(aix -> ap_obvalue, jix -> ap_obvalue) != 0)
			return 0;
		else {
			aix = aix -> ap_next;
			jix = jix -> ap_next;
		}
	}
	while (jix != NULLAP && jix->ap_obvalue == NULLCP)
		jix = jix -> ap_next;

	while (aix != NULLAP && aix->ap_obvalue == NULLCP)
		aix = aix -> ap_next;

	if (aix == NULLAP && jix == NULLAP)
		return 1;
	return 0;
}

/*  */

int	noFrom, from, gotDate;

/* 822norm stdin to stdout */
proc ()
{
	int	amp_fail;
	int	res;
	int	done;
	register char	*cp;
	PP_TRACE(("outtype %o",ap_outtype));
	noFrom = 1;
	gotDate = 0;
	from = 0;
	while ((getitm(&res) == OK) && (res == OK)) {

		ap_clear();
		nadrs = 0;
		amp_fail = FALSE;
		nonempty = FALSE;
		done = FALSE;

		if (ap_pinit(getbufchar) == BADAP) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("problem parsing message"));
			return NOTOK;
		}
			
		while (done == FALSE) {
			res = ap_1adr();	/* Parse one adr */
			switch(res) {
			    case DONE:	/* done */
				done = TRUE;
				break;
			    case NOTOK:
				/* pass the garbage and generate warning */
				amp_fail = TRUE;
				break;
			    default:	/* print it */
				ap_ppush (getbufchar);
				if ((res = out_adr(ap_pstrt)) != OK) 
					return res;
				ap_ppop();
				ap_pstrt = ap_pcur = ap_alloc ();
				break;
			}
		}
		putchar('\n');
		if (ap_perlev) {	/* if still nested */
			PP_TRACE(("nested level %d",ap_perlev));
			amp_fail++;
		}
		if (amp_fail == TRUE) {
			char	*fold, *ix;
			int	nonempty = FALSE;
			printf("PP-Warning: Parse error in original version of preceding line\n");
			printf("Original-%s: ", fieldbuf);
			ix = &(contbuf[0]);
			while (*ix != '\0') {
				fold = next_fold(ix+1, FOLD_COMMA);
				if ((fold_width != -1)
				    && (fold - ix + pcol > fold_width)
				    && nonempty == TRUE) {
					/* fold */
					pcol = strlen(fieldbuf) + strlen("Original-: ");
					printf("\n%*s",
					       strlen(fieldbuf) + strlen("Original-: "),
					       "");
					while (isspace(*ix))
						ix++;
				} else
					nonempty = TRUE;
				pcol += fold - ix;
				while (ix != fold)
					putchar (*ix++);
			}
			putchar('\n');				
		}
	}

	if (gotDate == 0) {
		/* no date so add one */
		UTC	ut;
		char	buf[BUFSIZ], *cp;
		strcpy (buf, "Date");
		cp = &buf[0];
		while (*cp != '\0')
			putchar (*cp++);
		putchar (':');
		putchar (' ');

		ut = utcnow();
		UTC2rfc(ut, buf);

		cp = &buf[0];
		while (*cp != '\0')
			putchar (*cp++);
		putchar('\n');
	}

	if (message_id == 0 && msgid_req == TRUE) {
		/* output message id */
		MPDUid	msgid;
		char	buf[BUFSIZ];
		strcpy(buf, "Message-ID");
		cp = &buf[0];
		while (*cp != '\0')
			putchar (*cp++);
		putchar (':');
		putchar (' ');

		MPDUid_new(&msgid);
		(void) sprintf(buf, "<\"%s\"@%s>", 
			       msgid.mpduid_string, loc_dom_site);
		cp = buf;
		while (*cp != '\0')
			putchar (*cp++);
		putchar ('\n');
	}
	if (jntsendernorm != NULL) {
		if (genacks == TRUE && acks == 0) {
			/* output Acknowledge-To field */
			cp = rcmd_srch(FLD_JNT_ACK, tbl_fields);
			while (*cp != '\0')
				putchar (*cp++);
			putchar(':');
			putchar(' ');
			cp = jntsendernorm;
			while (*cp != '\0')
				putchar(*cp++);
			putchar('\n');
		}

		if (noFrom) {
			/* output Sender */
			cp = rcmd_srch(FLD_SENDER, tbl_fields);
			while (*cp != '\0')
				putchar(*cp++);
			putchar(':');
			putchar(' ');

			cp = jntsendernorm;
			while (*cp != '\0')
				putchar(*cp++);
			putchar('\n');
		}

		ap_sqdelete(jnttree, NULLAP);
		free((char *)jnttree);
	}
	if (res == NOTOK) 
		return NOTOK;
	
#ifdef VAT
	tidy_up();
#endif

	return (ferror(stdout) ? NOTOK : OK);
}

/*  */
/* input routines */

#define	INC	2

/* returns current pointer = *orig + olength */
static char	*resize_buf(orig, olength, size)
char	**orig;
int	olength,
	*size;
{

	*size += INC;
	if (*orig == NULLCP)
		*orig = calloc (1, (unsigned int) (*size));
	else
		*orig = realloc (*orig, (unsigned int) (*size));
	return ((*orig)+olength);
}

/* returns OK while still got more input to read */
/* result == NOTOK if failed parsing */
static int getitm(result)
int 	*result;
{
	register int	c;
	register char	*cp,
			*ix;
	int		gotitm = FALSE;
	int		fld;

	fieldlen = 0;
	cp = fieldbuf;
	*result = OK;

	while (gotitm == FALSE) {
		if ((c = getach()) == EOF) 
			/* end of file */
			return NOTOK;
		
		switch (c) {
		    case '\n':
			if (fieldlen != 0)
				break;
		    case '\0':
			/* end of input */
			return NOTOK;
		}
		
		switch (c) {
		    case ':':
			/* Field name collected */
			if ((fieldlen+1) >= fieldsize) 
				cp = resize_buf(&fieldbuf,fieldlen,&fieldsize);
			*cp = '\0';
			PP_TRACE(("field '%s'",fieldbuf));
			ix = fieldbuf;
			if (isspace(*ix) && *ix != '\n') {
				while (isspace(*ix) && *ix != '\n')
					ix++;
				/* rewind back one to first nonspace */
				ix--;
			}

			/* got field now get contents */
			cp = contbuf;
			contlen = 0;

			while (((c = getach()) != 0) && (c != EOF)) {
				if (++contlen >= contsize) 
					cp = resize_buf(&contbuf,(contlen-1),&contsize);
				*cp++ = c;
			}
				
			/* terminate with a null char */
			if ((contlen + 1) >= contsize) 
				cp = resize_buf(&contbuf,contlen,&contsize);
			*cp ='\0';
			compress(fieldbuf, fieldbuf);
			compress(contbuf, contbuf);
			contix = contbuf;

			fld = cmd_srch(ix, tbl_tracefields);

			/* check if need to skip */
			if (striptrace == TRUE
			    && fld != -1) {
				/* skip it */
				cp = fieldbuf;
				fieldlen = 0;
				break;
			} 
			
			if (fld != -1
			    && valid_trace(fld, fieldbuf, contbuf) != OK) {
				/* already output with error */
				cp = fieldbuf;
				fieldlen = 0;
				break;
			}

			fld = cmd_srch(ix, tbl_fields);

			if (fld == FLD_JNT_ACK) {
				if (stripacks == TRUE) {
					/* skip it */
					cp = fieldbuf;
					fieldlen = 0;
					break;
				} else
					acks++;
			}
			       
			if (fld == FLD_FROM)
				from = 1;
			else
				from = 0;

			if (jntsender != NULL
			    && (fld == FLD_SENDER))
				/* convert to Original-Sender */
				cp = rcmd_srch(FLD_ORIG_SENDER, tbl_fields);
			else 
				cp = fieldbuf;

			if (fld == FLD_DATE)
				gotDate++;

			pcol = strlen(cp)+2;
			while (*cp != '\0')
				putchar(*cp++);
			putchar(':');
			/* put in jpo's space */
			putchar(' ');

			if (fld != -1 && fld != FLD_DATE) {
				gotitm = TRUE;

			} else {
				int	fold_num;
				/* copy rest of line out */
				nonempty = FALSE;
				pcol = strlen(fieldbuf) + 1;
				cp = contix;
				if (lexequ(fieldbuf, "Message-ID") == 0)
					message_id++;
					
				if ((fold_num = cmd_srch(fieldbuf,tbl_nonfields)) == -1) {
					PP_DBG(("unknown non field %s folding on spaces",fieldbuf));
					fold_num = FOLD_SPACE;
				}
				while (*contix != '\0') {
					/* go to next fold */
					cp = next_fold(contix+1,fold_num);
					
					if ((fold_width != -1)
					    && (cp - contix + pcol > fold_width) 
					    && nonempty) {
						/* new line */
						pcol = strlen(fieldbuf) + 2;
						printf("\n%*s", strlen(fieldbuf) + 2, "");
						/* strip out white space */
						while (isspace(*contix))
							contix++;
					} else
						nonempty = TRUE;
					pcol += cp - contix;
					/* output line */
					while (contix != cp)
						putchar(*contix++);
				}
				putchar('\n');				
				if (c == EOF)
					/* EOF */
					return NOTOK;
			}
			cp = fieldbuf;
			fieldlen = 0;
			break;

		    default:
			if (++fieldlen >= fieldsize) 
				cp = resize_buf(&fieldbuf,(fieldlen-1),&fieldsize);
			*cp++ = c;
		}
	}
	return OK;
}

static int isblank(ch)
char	ch;
{
	return (ch == ' ' || ch == '\t');
}

/* returns 0 when reach end of an item */
/* returns EOF when reach EOF */
static int	getach()
{
	static unsigned char	buf[MAXPATHLENGTH];
	static unsigned char	*bufp = buf;
	static int	noInput = 0;
	
	if (noInput == 0) { /* buffer is empty */
		noInput = read(0, buf, MAXPATHLENGTH);
		bufp = buf;
	}

	if (*bufp == '\n') {
		if (noInput == 1) {
			/* last char in buffer */
			noInput = read(0, buf, MAXPATHLENGTH);
			bufp = buf;
			if (isblank(*bufp)) {
				noInput--;
				return *bufp++;
			} else
				return 0;
		} else if (isblank(*(bufp+1))) {
			/* skip newline */
			bufp++;
			noInput--;
		}
	}
	if (*bufp == '\n') {
		bufp++;
		noInput--;
		return 0;
	}
	return ((--noInput >= 0) ? *bufp++ : EOF);
}

static int	getbufchar()
{
	char	ret = *contix;
	if (ret != 0) contix++;
	return (ret == 0) ? EOF : ret;
}

/*  */
/* output routine */

static char	*next_fold(ix,fold)
char	*ix;
int	fold;
{
	char   	fold_ch;

	switch (fold) {
	    case FOLD_NONE:
		fold_ch = '\0';
		break;
	    case FOLD_SPACE:
		fold_ch = ' ';
		break;
	    case FOLD_COMMA:
		fold_ch = ',';
		break;
	    case FOLD_SEMICOLON:
		fold_ch = ';';
		break;
	    case FOLD_RECIEVED:
		return  fold_recieved(ix);
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("unknown fold number %d",fold));
		fold_ch = ' ';
	}

	while (*ix != '\0' && *ix != fold_ch)
		ix++;
	if (*ix == '\0')
		return ix;
	else
		return ++ix;
}

static char	*fold_recieved(chs)
char	*chs;
{
	char	*ix;
	while (*chs != '\0' && *chs != ';' && *chs != ' ')
		chs++;
	if (*chs == '\0')
		return chs;
	if (*chs == ';')
		return ++chs;
	/* skip leading spaces */
	while (isspace(*chs))
		chs++;

	/* now check if have key words */
	ix = chs;
	while (*ix != '\0' &&
	       (ix - chs <= (int)strlen("from")+1)) {
		
		if (ix - chs == 3) {
			if (strncmp(chs,"by ",3) == 0)
				return chs;
			else if (strncmp(chs,"id ",3) == 0)
				return chs;
		}
		
		if (ix - chs == 4) {
			if (strncmp(chs,"via ",4) == 0)
				return chs;
			else if (strncmp(chs,"for ",4) == 0)
				return chs;
		}
		
		if (ix - chs == 5) {
			if (strncmp(chs,"from ",5) == 0)
				return chs;
			else if (strncmp(chs, "with ", 5) == 0)
				return chs;
				
		}
		ix++;
	}
	if (*ix == '\0')
		return ix;
	return fold_recieved(chs);
}

static int specifiedDomain(ptr)
AP_ptr	ptr;
{
	int	specified = FALSE,
		i = 0;

	if (ptr->ap_obtype != AP_DOMAIN
	    && ptr -> ap_obtype != AP_DOMAIN_LITERAL)
		return FALSE;

	while (specified != TRUE && i < num_domains) {
		if ((ptr->ap_obvalue != NULL)
		    && (strcmp(stripdomains[i], ptr->ap_obvalue) == 0))
			specified = TRUE;
		i++;
	}
	return specified;
}


static int hiddenDomain(ptr)
AP_ptr	ptr;
{
	int	hidden = FALSE,
		i, nwild, nw;
	char   *bp, *cp;

	/* get to next domain */
	while ((ptr != NULL) && (ptr->ap_obtype != AP_DOMAIN))
		ptr = ptr->ap_next;
	if (ptr == NULL)
		return 0;

	if (ptr->ap_obvalue != NULL)
		for (i = 0; hidden != TRUE && i < num_to_hide; i++) {
			for (nwild = 0; hidedomains[i][nwild] == '.' ||
			    hidedomains[i][nwild] == '*'; nwild ++)
				continue;
			nw = nwild;
			nwild /= 2;
			bp = cp = ptr->ap_obvalue;
			if ( ! hidedomains[i][nw] ) {
				PP_LOG (LLOG_EXCEPTIONS,
				    ("Format/rfc822norm/hiddenDomain: Too wild - Reality error"));
				continue;
			}

			while ( (nwild -- > 0) && (bp = index(bp, '.')) != NULL)
				bp ++;
			if ( !bp )
				continue;
			while ((bp = index(bp,'.')) != NULL) {
				if (++bp =='\0')
					break;
				cp = index(cp,'.'); 
				cp++;
				if ((strcmp(&hidedomains[i][nw], bp) == 0)) {
					hidden = TRUE;
					strcpy(ptr->ap_obvalue, cp);
					break;
				}
			}
		}
	return hidden;
}

static int recognisedDomain(ptr)
AP_ptr	ptr;
{
	if (ptr->ap_obtype != AP_DOMAIN
	    && ptr->ap_obtype != AP_DOMAIN_LITERAL)
		return FALSE;
	if (ptr->ap_normalised != TRUE)
		rfc822_norm_dmn(ptr, order_pref);
	return ptr->ap_recognised;
}

static void changedomain (dom)
AP_ptr 	dom;
{
	int	i;
	for (i = 0; i < num_pairs; i++) {
		if (changedomains[i].from->ap_normalised != TRUE)
			rfc822_norm_dmn(changedomains[i].from,
					order_pref);
		if (lexequ(changedomains[i].from->ap_obvalue, 
			   dom->ap_obvalue) == 0)
			break;
	}
	if (i < num_pairs) {
		if (dom->ap_obvalue)
			free(dom->ap_obvalue);
		if (changedomains[i].to->ap_normalised != TRUE)
			rfc822_norm_dmn(changedomains[i].to,
					order_pref);
		dom->ap_obvalue = strdup(changedomains[i].to->ap_obvalue);
		if (dom->ap_normalised == TRUE) {
			if (dom->ap_localhub) {
				free (dom->ap_localhub);
				dom->ap_localhub = NULLCP;
			}
			if (dom->ap_chankey) {
				free (dom->ap_chankey);
				dom->ap_chankey = NULLCP;
			}
			if (dom->ap_error) {
				free (dom->ap_error);
				dom->ap_error = NULLCP;
			}
		}
	}
}

static void do_changedomains (tree)
AP_ptr	tree;
{
	while (tree != NULLAP) {
		if (tree -> ap_obtype == AP_DOMAIN
		    || tree -> ap_obtype == AP_DOMAIN_LITERAL)
			changedomain(tree);
		tree = tree->ap_next;
	}
}

static void	do_stripdomains(pap, pgroup, pname, ploc, 
				pdom, proute)
AP_ptr  *pap,
	*pgroup,
	*pname,
	*ploc,
	*pdom,
	*proute;
{
	AP_ptr	ix, hdr;
	int	cont = TRUE;

	if (*proute != NULLAP) {
		/* go down tree removing specified domains */
		hdr = ap_new(AP_DOMAIN, "header");
		hdr->ap_next = *pap;
		hdr->ap_ptrtype = AP_PTR_MORE;
		ix = hdr;

		while ((ix->ap_next != NULL)
		       && cont == TRUE) {
			switch (ix -> ap_next -> ap_obtype) {
			    case AP_DOMAIN:
			    case AP_DOMAIN_LITERAL:
				if (specifiedDomain (ix->ap_next) == TRUE) 
					ap_delete (ix);
				else
					ix = ix -> ap_next;
				break;

			    case AP_MAILBOX:
			    case AP_GENERIC_WORD:
				/* don't remove dom so stop here */
				cont = FALSE;
				break;
				
			    default:
				ix = ix -> ap_next;
			}
		}

		/* reset all pointers */
		*pap = hdr->ap_next;
		hdr->ap_next = NULLAP;
		ap_free(hdr);
		(void) ap_t2p(*pap, pgroup, pname, ploc, pdom, proute);
	}
}

static void	do_striproutes(pap, pgroup, pname, ploc, 
				pdom, proute)
AP_ptr  *pap,
	*pgroup,
	*pname,
	*ploc,
	*pdom,
	*proute;
{
	int	cont = TRUE;
	char	*tmp;
	AP_ptr	ix,
		hdr;

	if (recognisedDomain(*pdom) == TRUE) {
		/* recognised *pdom, remove all route */
		tmp = ap_p2s(*pgroup, *pname, *ploc, 
				*pdom, NULLAP);
		ix = *pap;
		ap_s2p(tmp, pap, pgroup, pname, ploc, pdom, proute);
		ap_free(ix);
		free(tmp);
	} else {	
		/* go down tree removing recognised domains */
		hdr = ap_new(AP_DOMAIN, "header");
		hdr->ap_next = *pap;
		hdr->ap_ptrtype = AP_PTR_MORE;
		ix = hdr;

		while ((ix->ap_next != NULL)
		       && cont == TRUE) {
			switch (ix -> ap_next -> ap_obtype) {
			    case AP_DOMAIN:
			    case AP_DOMAIN_LITERAL:
				if (recognisedDomain (ix->ap_next) == TRUE) 
					ap_delete (ix);
				else
					cont = FALSE;
				break;

			    case AP_MAILBOX:
			    case AP_GENERIC_WORD:
				cont = FALSE;
				break;
				
			    default:
				ix = ix -> ap_next;
			}
		}

		/* reset all pointers */
		*pap = hdr->ap_next;
		hdr->ap_next = NULLAP;
		ap_free(hdr);
		(void) ap_t2p(*pap, pgroup, pname, ploc, pdom, proute);
	}
}

static 
do_hidedomains(dom_ptr, route_ptr)
AP_ptr  dom_ptr,
route_ptr;
{
	int	retval = 0;
	AP_ptr	ptr;
	if ((ptr = route_ptr) != NULLAP )
		while ( ptr ) {
			retval += hiddenDomain(ptr);
			ptr = ptr->ap_next;
		}
	return retval += hiddenDomain(dom_ptr);
}

#ifdef VAT
static int 	tidy_up()
{
	if (random() % 10000 != 42)
		return;
	/* lucky person gets a message */
	switch (random() % 4) {
	    case 0:
		printf ("Checked-by: NSA, MI5, CIA, KGB\n");
		break;
	    case 1:
		printf ("Green-Message: This message is stored on recycled memory\n");
		break;
	    case 2:
		printf ("Best-Wishes-From: Steve, Julian, Pete, Alina et al\n");
		break;
	    default:
		printf ("Congratulations: You are the recipient of our %d message\n", random() % 1000000);
		break;
	}
}
#endif

static char *re_parse_ptr;
static int	get_rp_char()
{
	char	ret = *re_parse_ptr;
	if (ret != 0) re_parse_ptr++;
	return (ret == 0) ? EOF : ret;
}

/*  */

static int mustExorcise(domain)
char	*domain;
{
	int	retval = FALSE;
	char	chan[BUFSIZ], normalised[BUFSIZ];
	char	*subdom;

	if (exorciseTable == NULLTBL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("No table of valid addresses"));
		return FALSE;
	}
	if (tb_getdomain_table(exorciseTable, domain, chan, normalised, 
			       order_pref, &subdom) != OK)
		retval = TRUE;
	if (subdom) free(subdom);
	return retval;
}

AP_ptr	exorciseAp;

static void createExorciseAp ()
{	
	exorciseAp = ap_new(AP_DOMAIN, exorciseDomain);
	rfc822_norm_dmn (exorciseAp, order_pref);
}

static void do_exorcisedomain (pap, pgroup, pname, ploc,
			       pdom, proute)
AP_ptr	*pap, *pgroup, *pname, *ploc, *pdom, *proute;
{
	AP_ptr	ix;
	if (*proute != NULLAP) {
		for (ix = *proute;
		     ix != NULLAP && 
		     ix -> ap_obtype != AP_DOMAIN
		     && ix -> ap_obtype != AP_DOMAIN_LITERAL;
		     ix = ix -> ap_next);
		if (ix != NULLAP
		    && mustExorcise(ix->ap_obvalue) == TRUE) {
			if (exorciseAp == NULLAP)
				createExorciseAp();
			/* add exorciseAp immediately before ix */
			ap_insert(ix, AP_PTR_MORE,
				  ap_new(ix -> ap_obtype,
					 ix -> ap_obvalue));
			free(ix -> ap_obvalue);
			ap_fllnode(ix, exorciseAp -> ap_obtype,
				   exorciseAp -> ap_obvalue);
		}
	} else if (*pdom != NULLAP) {
		for (ix = *pdom;
		     ix != NULLAP && 
		     ix -> ap_obtype != AP_DOMAIN
		     && ix -> ap_obtype != AP_DOMAIN_LITERAL;
		     ix = ix -> ap_next);
		if (ix != NULLAP
		    && mustExorcise (ix->ap_obvalue) == TRUE) {
			char	*addrp;
			if (exorciseAp == NULLAP)
				createExorciseAp();
			/* add exorciseAp to route */
			addrp = ap_p2s(*pgroup, *pname, *ploc, 
				       *pdom, exorciseAp);
			ap_sqdelete (*pap, NULLAP);
			ap_free(*pap);
			ap_s2p(addrp, pap, pgroup, pname, ploc, pdom, proute);
			free(addrp);
		}
	}
}

/*  */
extern aliasList 	*aliases;

static int	out_adr(ap)
AP_ptr	ap;
{
	AP_ptr  loc_ptr,        /* -- in case fake personal name needed -- */
	group_ptr,
	name_ptr,
	dom_ptr,
	route_ptr, tmp;
	char	*addrp;
	int	len;
	static  rd = 0;
	Aparse_ptr	aparse = aparse_new();

	if (ap->ap_obtype == AP_NIL)
		return OK;

	rd++;
	
	aparse -> ad_type = AD_822_TYPE;
	aparse -> dmnorder = order_pref;
	aparse -> normalised = normalised;
	aparse -> percents = percents;
	aparse -> internal = internal;

	ap_t2s(ap, &(aparse->r822_str));
	if (aparse_norm(aparse) == NOTOK) {
		ap = ap_t2p(ap, &group_ptr, &name_ptr, 
			    &loc_ptr, &dom_ptr, &route_ptr);
	} else {
		tmp = ap;
		ap = aparse->ap_tree;
		aparse->ap_tree = tmp;
		group_ptr = aparse->ap_group;
		name_ptr = aparse->ap_name;
		loc_ptr = aparse->ap_local;
		dom_ptr = aparse->ap_domain;
		route_ptr = aparse->ap_route;
	}
	aparse_free(aparse);
	free ((char *) aparse);

	if (from && equalWithJntsender(loc_ptr))
		noFrom = 0;
	
	if (num_pairs != 0
	    && changedomains != NULL)
		do_changedomains (ap);
	
	/* do all stripping then create addrp */
	if ((dom_ptr != NULL)
		&& (num_to_hide != 0)
		&& do_hidedomains(dom_ptr, route_ptr)) {

	/* This might be local now, so go through the whole expansion
	 * process again.  Remember though that we might be exremely
	 * silly, and cause `infinite' recursion.  Hence the rd variable.
	 *
	 * If we are being silly we'll take what was demanded, and log an
	 * exception later
	 */

		if (rd < 3) {
			re_parse_ptr = addrp = ap_p2s(group_ptr, name_ptr, loc_ptr, 
						      dom_ptr, route_ptr);
			ap_sqdelete (ap, NULLAP);
			ap_free (ap);
			ap = ap_pstrt = ap_pcur = ap_alloc ();
			ap_ppush(get_rp_char);
			ap_clear();
			if (ap_1adr() == OK)
				out_adr(ap);
			ap_ppop();
			free(addrp);
			rd--;
			return OK;
			/* NOTREACHED */
		}
	}
	if ((dom_ptr != NULL)
	    && (striproutes == TRUE))
		do_striproutes(&ap, &group_ptr, &name_ptr, &loc_ptr,
			       &dom_ptr, &route_ptr);
	else if ((dom_ptr != NULL) 
		 && (num_domains != 0))
		do_stripdomains(&ap, &group_ptr, &name_ptr, &loc_ptr,
				&dom_ptr, &route_ptr);

	if (exorcise == TRUE)
		do_exorcisedomain(&ap, &group_ptr, &name_ptr, &loc_ptr,
				  &dom_ptr, &route_ptr);

	addrp = ap_p2s(group_ptr, name_ptr, loc_ptr, 
		       dom_ptr, route_ptr);

				       
	if (addrp == (char *)NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/addr/ap_t2s: error from ap_p2s()"));
		addrp = strdup ("(PP Error!)");
	}
	if (rd == 3) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Format/rfc822norm: Excessive local hiding - lying about %s", addrp));
	}

	if (nadrs != 0) {
		printf(", ");
		pcol += 2;
	}

	PP_TRACE(("output '%s'",addrp));

	if ((len = strlen(addrp)) > 0) { /* print */
		pcol += len;
		if (fold_width != -1 && pcol > fold_width && nonempty) {
			pcol = strlen(fieldbuf) + 2 + len;
			printf("\n%*s", strlen(fieldbuf) + 2, "");
		} else
			nonempty = TRUE;
		
		printf("%s",addrp);
		nadrs++;
	}
	free(addrp);
/*	ap_sqdelete (ap, NULLAP);
	ap_free (ap);*/
	rd--;
	return OK;
}

/*  */
static char *reverse(str)
char	*str;
{
	char	*ret = malloc((unsigned) strlen(str)+1),
		*dup = strdup(str),
		*ix;
	ret[0] = '\0';
	while ((ix = rindex(dup,'.')) != NULL) {
		if (ret[0] == '\0')
			sprintf(ret, "%s", ix+1);
		else
			sprintf(ret,"%s.%s",ret,ix+1);
		*ix = '\0';
	}
	if (ret[0] == '\0')
		sprintf(ret, "%s", dup);
	else
		sprintf(ret,"%s.%s",ret,dup);
	free(dup);
	return ret;
}

/*  */

static int valid_received (cont)
char	*cont;
{
	if (index (cont, ';') == NULLCP)
		return NOTOK;
	return OK;
}

static int valid_via(cont)
char	*cont;
{
	char	*sep, *ix;
	if ((sep = index (cont, ';')) == NULLCP)
		return NOTOK;
	
	for (ix = &(cont[0]);
	     ix != sep && isspace(*ix);
	     ix++);
	
	if (ix == sep)
		return NOTOK;
	
	return OK;
}

static int valid_x400(cont)
char	*cont;
{
	char	*sep;
	/* at least two semi-colons */
	if ((sep = index(cont, ';')) == NULLCP
	    || index((sep+1), ';') == NULLCP)
		return NOTOK;
	return OK;
}

int valid_trace(type, field, cont)
int	type;
char	*field;
char	*cont;
{
	int	retval = OK;
	switch (type) {
	    case TRACE_RECIEVED:
		retval = valid_received(cont);
		break;
	    case TRACE_VIA:
		retval = valid_via(cont);
		break;
	    case TRACE_X400:
		retval = valid_x400(cont);
		break;
	    default:
		break;
	}

	if (retval == NOTOK) {
		char	*fold, *ix;
		int	nonempty = FALSE;
		
		printf("Original-%s: ", field);
		ix = &(cont[0]);
		while (*ix != '\0') {
			fold = next_fold(ix+1, FOLD_SPACE);
			if ((fold_width != -1)
			    && (fold - ix + pcol > fold_width)
			    && nonempty == TRUE) {
				/* fold */
				pcol = strlen(field) + strlen("Original-: ");
				printf("\n%*s",
				       strlen(field) + strlen("Original-: "),
				       "");
				while (isspace(*ix))
					ix++;
			} else
				nonempty = TRUE;
			pcol += fold - ix;
			while (ix != fold)
				putchar (*ix++);
		}
		putchar('\n');				

		printf("PP-warning: Illegal %s field on preceding line\n",
		       rcmd_srch(type, tbl_tracefields));
	}
	return retval;
}
