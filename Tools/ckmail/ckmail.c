/* ckmail.c: tool to allow user to query status of users msgs */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ckmail/RCS/ckmail.c,v 6.0 1991/12/18 20:29:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ckmail/RCS/ckmail.c,v 6.0 1991/12/18 20:29:15 jpo Rel $
 *
 * $Log: ckmail.c,v $
 * Revision 6.0  1991/12/18  20:29:15  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "Qmgr-ops.h"           /* operation definitions */
#include "Qmgr-types.h"         /* type definitions */
#include "retcode.h"
#include <isode/cmd_srch.h>
#include "adr.h"
#include "alias.h"
#include "ap.h"
#include "or.h"
#include <pwd.h>
#include "ryinitiator.h"        /* for generic interactive initiators */

/*  */
extern CMD_TABLE
	atbl_ctrl_addrs[/* Env-crl-address */];
extern char	*loc_dom_site, *pplogin, *postmaster;
extern UTC	utclocalise();
/* DATA */
static char *myservice = 	"pp qmgr";

/* OPERATIONS */
static int 	do_readmsginfo(),
		do_quit(),
		p_msg(),
		p_msginfo(),
		p_recip(),
		isPP();
     

/* RESULTS */
int 	readmsginfo_result();
	
/* ERRORS */
int 	general_error ();

#define readmsginfo_error 	general_error

static struct client_dispatch dispatches[] = {
{
	"readmsginfo", operation_Qmgr_readmsginfo,
	do_readmsginfo,
#ifdef PEPSY_VERSION
	&_ZQmgr_mod, _ZReadMessageArgumentQmgr,
#else
	free_Qmgr_readmsginfo_argument,
#endif
	readmsginfo_result, readmsginfo_error,
	"Read set of messages"
},
{
    "quit",     0,      do_quit,
#ifdef	PEPSY_VERSION
    NULL, 0,
#else
    NULLIFP, 
#endif
    NULLIFP, NULLIFP,
    "terminate the association and exit",
},
{
    NULL
}
};

char	*qhost;		/* host to query */
int	first;
int	all = FALSE;
int 	verbose = FALSE;
int	time_out, first, do_loop = FALSE;
int	haveUid, userId;
int debug = 0;
char	*userAlias;
ADDR	*ad;
char	*name;
extern char	*qmgr_hostname;
#define MAX_NUM_IGNORES	30
static char	*ignores[MAX_NUM_IGNORES];
static int	numIgnores;

/*  */
main(argc, argv)
int	argc;
char	**argv;
{	
	char	*myname;
	int	opt;
	extern char	*optarg;
	extern int	optind;
	if (myname = rindex (argv[0], '/'))
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = argv[0];

	sys_init(myname);
	or_myinit();
	
	qhost = strdup(qmgr_hostname);
	all = FALSE;
	numIgnores = 0;
	ignores[numIgnores++] = "msg-clean";
	verbose = FALSE;
	do_loop = FALSE;
	first = 0;
	haveUid = NOTOK;
	userAlias = NULLCP;
	while ((opt = getopt(argc, argv, "dvVaAh:H:l:L:u:U:i:I:c:C:")) != EOF) {
		switch (opt) {
		    case 'a':
		    case 'A':
			all = TRUE;
			break;
		    case 'd':
			debug = 1;
			break;
		    case 'v':
		    case 'V':
			verbose = TRUE;
			break;
		    case 'h':
		    case 'H':
			if (optarg[0] == '-') {
				printf("Illegal host '%s' (starts with a '-')\n",
				       optarg);
				exit(0);
			}
			qhost = optarg;
			break;
		    case 'l':
		    case 'L':
			if (optarg[0] =='-') {
				printf ("Expecting time in minutes got '%s'", optarg);
				exit(0);
			}
			time_out = atoi(optarg);
			do_loop = TRUE;
			break;

		    case 'u':
		    case 'U':
			if (isPP() == NOTOK) {
				printf("You are not a mail superuser and so cannot use the '-u' flag\n");
				exit(0);
			}
			userAlias = optarg;
			break;

		    case 'i':
		    case 'I':
			if (isPP() == NOTOK) {
				printf("You are not a mail superuser and so cannot use the '-i' flag\n");
				exit(0);
			}
			userId = atoi(optarg);
			haveUid = OK;
			break; 
			
		    case 'c':
		    case 'C':
			if (numIgnores >= MAX_NUM_IGNORES) {
				printf("can only ignore %d channels\n", MAX_NUM_IGNORES);
				exit (0);
			}
			ignores[numIgnores++] = optarg;
			break;
		    default:
			printf("usage: %s [-v] [-a] [-c channel] [-h host] [-l time] [-u user] [-i uid]\n",argv[0]);
			exit(0);
		}
	}
	fillinaddr();
	if (verbose == TRUE) {
		printf("Please wait while attempt to connect to %s....",qhost);
		fflush(stdout);
	}
	if (do_loop == TRUE) {
		while (TRUE) {
			(void) ryinitiator (myname, qhost,
					    argc, argv, myservice,
					    table_Qmgr_Operations,
					    dispatches, do_quit);
			first++;
			sleep (time_out*60);
		}
	} else
		(void) ryinitiator (myname, qhost,
				    argc, argv, myservice,
				    table_Qmgr_Operations,
				    dispatches, do_quit);

	exit(0);	
}

/*  */
/* OPERATIONS */

/* ARGSUSED */
static int  do_quit (sd, ds, args, arg)
int     sd;
struct client_dispatch *ds;
char  **args;
char **arg;
{
	struct AcSAPrelease acrs;
	register struct AcSAPrelease   *acr = &acrs;
	struct AcSAPindication  acis;
	register struct AcSAPindication *aci = &acis;
	register struct AcSAPabort *aca = &aci -> aci_abort;

	if (AcRelRequest (sd, ACF_NORMAL, NULLPEP, 0, NOTOK, acr, aci) == NOTOK)
		acs_adios (aca, "A-RELEASE.REQUEST");

	if (!acr -> acr_affirmative) {
		(void) AcUAbortRequest (sd, NULLPEP, 0, aci);
		adios (NULLCP, "Release rejected by peer: %d", acr -> acr_reason);
	}

	ACRFREE (acr);

/*	exit (0);*/

}

static struct type_Qmgr_Filter		*fillin_1_to(name)
char	*name;
{
	struct type_Qmgr_Filter	*ret = (struct type_Qmgr_Filter *)
		calloc(1, sizeof(*ret));
	ret->recipient = str2qb(name, strlen(name), 1);
	return ret;
}

static struct type_Qmgr_Filter		*fillin_1_from(name)
char	*name;
{
	struct type_Qmgr_Filter	*ret = (struct type_Qmgr_Filter *)
		calloc(1, sizeof(*ret));
	ret->originator = str2qb(name, strlen(name), 1);
	return ret;
}

static struct type_Qmgr_FilterList	*fillinfilters(args)
char	**args;
{
	struct type_Qmgr_FilterList	*head = NULL,
					*tail = NULL,
					*ix;
	
	ix = (struct type_Qmgr_FilterList *) calloc(1, sizeof(*ix));
	ix->Filter = fillin_1_from(ad->ad_r822adr);
	if (head == NULL)
		head = tail = ix;
	else {
		tail->next = ix;
		tail = ix;
	}

	ix = (struct type_Qmgr_FilterList *) calloc(1, sizeof(*ix));
	ix->Filter = fillin_1_from(ad->ad_r400adr);
	if (head == NULL)
		head = tail = ix;
	else {
		tail->next = ix;
		tail =ix;
	}
	if (all == FALSE) 
		return head;
		
	ix = (struct type_Qmgr_FilterList *) calloc(1, sizeof(*ix));
	ix->Filter = fillin_1_to(ad->ad_r822adr);
	if (head == NULL)
		head = tail = ix;
	else {
		tail->next = ix;
		tail = ix;
	}

	ix = (struct type_Qmgr_FilterList *) calloc(1, sizeof(*ix));
	ix->Filter = fillin_1_to(ad->ad_r400adr);
	if (head == NULL)
		head = tail = ix;
	else {
		tail->next = ix;
		tail =ix;
	}
	return head;
}

/* ARGSUSED */
static do_readmsginfo (sd, ds, args, arg)
int				sd;
struct client_dispatch		*ds;
char				**args; /* contains various filters */
struct type_Qmgr_ReadMessageArgument	**arg;
{
	char	*str;
	UTC	utc;

	*arg = (struct type_Qmgr_ReadMessageArgument *) malloc(sizeof(**arg));
	
	/* fillin time */
	utc = (UTC) malloc (sizeof(struct UTCtime));

	utc->ut_flags = UT_SEC;
	utc->ut_sec = 0;
	str = utct2str(utc);
	(*arg)->interval = str2qb(str, strlen(str), 1);
	(*arg)->filters = fillinfilters(args);
	if (debug) print_filterlist ((*arg)->filters);
	return OK;
}

/*  */
/* RESULTS */

static int	isIgnored(chan)
char	*chan;
{
	int	ix;

	for (ix = 0;ix < numIgnores;ix++)
		if (lexequ(ignores[ix], chan) == 0)
			return 1;
	return 0;
}

static char	*getChan(recip)
struct type_Qmgr_RecipientInfo	*recip;
{
	struct type_Qmgr_ChannelList	*ic;
	int	ix;
	char	*chan = NULLCP;
	
	if (!recip) return NULLCP;

	for (ix = 0, ic = recip->channelList;
	     ix < recip->channelsDone && ic != NULL; 
	     ix++, ic = ic -> next);

	if (ic && ic -> Channel) {
		chan = qb2str(ic -> Channel);
		if (isIgnored(chan)) {
			free(chan);
			chan = NULLCP;
		}
	}
	return chan;
}

static int	isSender(msg)
struct type_Qmgr_MsgStruct	*msg;
{
	char	*orig = qb2str(msg->recipientlist->RecipientInfo->user);
	int	retval = 1;
	if (strcmp(orig, ad->ad_r822adr) != 0
	    && strcmp(orig, ad->ad_r400adr) != 0) 
		/* to user */
		retval = 0;
	free(orig);
	return retval;
}

static int isPrintable(msg, sender)
struct type_Qmgr_MsgStruct	*msg;
int				sender;
{
	char	*chan = NULLCP;
	struct type_Qmgr_RecipientList	*ix;

	if (!sender)
		return 1;
	/* go through recip and see if getChan */
	ix = msg->recipientlist;
	ix = ix -> next; /* skip past sender */
	while (ix != NULL && chan == NULLCP) {
		chan = getChan(ix -> RecipientInfo);
		ix = ix -> next;
	}
	if (chan == NULLCP)
		return 0;
	free(chan);
	return 1;
}
	
static int isPrintableMsg(msgs)
struct type_Qmgr_MsgStructList *msgs;
{
	struct type_Qmgr_MsgStructList *ix = msgs;
	int	printable = 0;
	
	while (ix != NULL && printable == 0) {
		printable = isPrintable(ix -> MsgStruct, 
					isSender(ix->MsgStruct));
		ix = ix -> next;
	}
	return printable;
}
		
	
/* ARGSUSED */
int readmsginfo_result (sd, id, dummy, result, roi)
int						sd,
						id,
						dummy;
register struct type_Qmgr_MsgList		*result;
struct RoSAPindication				*roi;
{
	struct type_Qmgr_MsgStructList	*ix;
	if (result == NULL || result->msgs == NULL
	    || !isPrintableMsg(result->msgs)) {
		if (do_loop == FALSE || first == 0)
			printf("There are no messages %s %s at %s\n",
			       (all == TRUE) ? "to or from" : "from",
			       name, qhost);
	} else {
		printf("The messages %s %s on %s are as follows:\n",
		       (all == TRUE) ? "to or from" : "from",
		       name, qhost);
		ix = result->msgs;
		while (ix != NULL) {
			p_msg(ix->MsgStruct, isSender(ix->MsgStruct));
			ix = ix->next;
		}
	}
	return OK;
}

static	int	p_msg(msg, sender)
struct type_Qmgr_MsgStruct	*msg;
{
	int	len, tlen;
	struct type_Qmgr_RecipientList	*ix;

	first = 1;
	
	if (!sender) {
		/* to user */
		if ((len = p_msginfo(msg->messageinfo, sender)) == 0)
			return;
		printf(" is from");
		tlen = len + strlen(" is from") + 1;
		p_recip(msg->recipientlist->RecipientInfo, tlen, 1);
		if (verbose) {
			printf ("\n%*s is pending delivery to", len, "");
			first = 1;
			for (ix = msg->recipientlist -> next;
			     ix;  ix = ix -> next)
				p_recip (ix -> RecipientInfo, len, 0);
		}
		printf("\n");

	} else if (isPrintable(msg, sender)) {
		ix = msg->recipientlist;
		if ((len = p_msginfo(msg->messageinfo, sender)) == 0)
			return;
		if (verbose) {
			printf (" is from");
			p_recip (ix -> RecipientInfo, len, 1);
			printf ("\n%*s", len, "");
			first = 1;
		}
		ix = ix -> next;
		if (ix != NULL) {
			printf(" is pending delivery to");
			len += strlen(" is pending delivery to") +1;
			while (ix != NULL) {
				p_recip(ix->RecipientInfo, len, 0);
				ix = ix -> next;
			}
		} else
			printf(" is waiting for notification/deletion");
		printf("\n");
	} 
}

static int	p_msginfo(info, sender)
struct type_Qmgr_PerMessageInfo	*info;
int				sender;
{
	char	*id, buf[BUFSIZ];
	int	retval;
	UTC	ut, lut;
	
	id = qb2str(info->queueid);
	retval = strlen(id);
	printf("%s",id);
	free(id);
	if (info -> uaContentId != NULL) {
		id = qb2str (info -> uaContentId);
		printf (" with content id '%s'\n%*s", id, retval, " ");
		free(id);
	}
	if (info -> age != NULL) {
		id = qb2str(info -> age);
		ut = str2utct(id, strlen(id));
		lut = utclocalise (ut);
		if (UTC2rfc (lut, buf) != NOTOK) 
			printf (" submitted at %s\n%*s", buf, retval, " ");
		if (lut) free ((char *) lut);
		free(id);
	}
	if (verbose != TRUE)
		return retval;
	if (info -> expiryTime != NULL) {
		id = qb2str (info -> expiryTime);
		ut = str2utct(id, strlen(id));
		lut = utclocalise(ut);
		if (UTC2rfc (lut, buf) != NOTOK) 
			printf (" will expire at %s\n%*s", buf, retval, " ");
		if (lut) free ((char *) lut);
		free(id);
	}
	if (info -> priority && info -> priority -> parm != 1)
		printf (" priority %d\n%*s", info -> priority -> parm,
			retval, "");
	if (info -> size)
		printf (" size %d\n%*s", info -> size, retval, "");
	if (info -> numberWarningsSent)
		printf (" %d warnings sent\n%*s", info -> numberWarningsSent,
			retval, "");
	if ((info -> optionals & opt_Qmgr_PerMessageInfo_errorCount) &&
	    info -> errorCount)
		printf (" %d errors accumulated\n%*s", info -> errorCount,
			retval, "");
	
	if (info -> deferredTime != NULL) {
		id = qb2str (info -> deferredTime);
		ut = str2utct(id, strlen(id));
		lut = utclocalise(ut);
		if (UTC2rfc (lut, buf) != NOTOK) 
			printf (" deferred until %s\n%*s", buf, retval, " ");
		if (lut) free ((char *) lut);
		free(id);
	}
	
	return retval;
}

static int 	p_recip(recip, tab, sender)
struct type_Qmgr_RecipientInfo	*recip;
int				tab;
int				sender;
{
	char	*usr, *chan = NULLCP;

	if (!sender && (chan = getChan(recip)) == NULLCP)
		return;

	if (first != 1)
		printf(",\n%*s", tab, " ");
	else
		printf(" ");
	first = 0;
	
	usr = qb2str(recip->user);
	
	if (sender  || chan == NULLCP)
		printf("%s", usr);
	else {
		printf("%s (channel = %s)",usr, chan);
		free(chan);
	}
	free(usr);
}

/*  */
/* ERRORS */

/* ARGSUSED */
general_error (sd, id, error, parameter, roi)
int     sd,
	id,
	error;
caddr_t parameter;
struct RoSAPindication *roi;
{
	register struct RyError *rye;

	if (error == RY_REJECT) {
		advise (NULLCP, "%s", RoErrString ((int) parameter));
		return OK;
	}

	if (rye = finderrbyerr (table_Qmgr_Errors, error))
		advise (NULLCP, "%s", rye -> rye_name);
	else
		advise (NULLCP, "Error %d", error);

	return OK;
}

/*  */
/* fillin an ADDR struct for person running this program */

fillinaddr()
{
	struct passwd *pwd;
	RP_Buf	rp;

	if (userAlias != NULLCP) {
		if ((pwd = getpwnam(userAlias)) == NULL)
			name = userAlias;
		else name = pwd -> pw_name;
	} else if (haveUid == OK) {
		if ((pwd = getpwuid(userId)) == NULL) 
			adios (NULLCP,
			       "Cannot get passwd entry for uid '%d'",
			       userId);
		name = pwd -> pw_name;
	} else {
		if ((pwd = getpwuid(getuid())) == NULL)
			adios(NULLCP,"Cannot get your password entry");
		name = pwd -> pw_name;
	}
	ad = adr_new(name, AD_ANY_TYPE, 0);
#ifdef UKORDER
	if (rp_isbad(ad_parse(ad, &rp, CH_UK_PREF))) 
#else
	if (rp_isbad(ad_parse(ad, &rp, CH_USA_PREF))) 
#endif
		adios(NULLCP, 
		      "Address parse failed\nReason : %s\n",rp.rp_line);
}

static int isPP()
{
	RP_Buf	rp;
	int	uid;
	struct passwd	*tmp;
	ADDR	*tmpadr;

	if ((uid = getuid()) == 0)
		/* super user */
		return OK;
	if ((tmp = getpwuid(uid)) == NULL) {
		printf("Cannot get your passwd entry\n");
		exit(0);
	}
	tmpadr = adr_new(tmp->pw_name, AD_ANY_TYPE, 0);

#ifdef UKORDER
	if (rp_isbad(ad_parse(tmpadr, &rp, CH_UK_PREF))) {
#else
	if (rp_isbad(ad_parse(tmpadr, &rp, CH_USA_PREF))) {
#endif
		printf("Cannot parse your mail address\nReason : %s\n",rp.rp_line);
		exit(1);
	}

	if ((strcmp(tmp->pw_name, pplogin) == 0)
	    || (strcmp(tmpadr->ad_r822adr, postmaster) == 0)) {
		adr_free(tmpadr);
		return OK;
	}
	adr_free(tmpadr);
	return NOTOK;
}

static int indent = 0;

static void print_qb (str, qb)
char *str;
struct qbuf *qb;
{
	struct qbuf *qp;

	printf ("%*s", indent * 2, "");
	if (str)
		printf ("%s: ", str);
	for (qp = qb -> qb_forw; qp != qb; qp = qp -> qb_forw)
		printf ("%*.*s", qp -> qb_len, qp -> qb_len,
			qp -> qb_data);
	putchar ('\n');
}

static void print_eit (eit)
struct type_Qmgr_EncodedInformationTypes *eit;
{
	printf("%*sEncodedInformationTypes\n", indent * 2, "");
	indent ++;
	while (eit) {
		print_qb (NULLCP, eit -> PrintableString);
		eit = eit -> next;
	}
	indent --;
}

static void print_priority (prio)
struct type_Qmgr_Priority *prio;
{
	printf ("%*sPriority: ", indent * 2, "");
	switch (prio -> parm) {
	    case int_Qmgr_Priority_low:
		printf ("low\n");
		break;
	    case int_Qmgr_Priority_normal:
		printf ("normal\n");
		break;
	    case int_Qmgr_Priority_high:
		printf ("high\n");
		break;
	    default:
		printf ("%d\n", prio -> parm);
		break;
	}
}

static void print_mpdu (mpdu)
struct type_Qmgr_MPDUIdentifier *mpdu;
{
	printf ("%*smpduiden\n", indent * 2, "");
}

static void print_filter(f)
struct type_Qmgr_Filter *f;
{
	printf ("%*sFilter\n", indent * 2, "");
	indent ++;
	if (f -> contenttype)
		print_qb ("ContentType", f -> contenttype);
	if (f -> eit)
		print_eit (f->eit);
	if (f -> priority)
		print_priority (f->priority);
	if (f -> moreRecentThan)
		print_qb ("moreRecentThan", f -> moreRecentThan);
	if (f -> earlierThan)
		print_qb ("earlierThan", f -> earlierThan);
	if (f -> maxSize)
		printf ("%*smaxsize: %d\n", indent * 2, "", f -> maxSize);
	if (f -> originator)
		print_qb ("Originator", f -> originator);
	if (f -> recipient)
		print_qb ("Recipient", f -> recipient);
	if (f -> channel)
		print_qb ("Channel", f -> channel);
	if (f -> mta)
		print_qb ("Mta", f->mta);
	if (f -> queueid)
		print_qb ("Queueid", f -> queueid);
	if (f -> mpduiden)
		print_mpdu(f -> mpduiden);
	if (f -> uaContentId)
		print_qb ("UAContentId", f -> uaContentId);
	indent --;
}

print_filterlist (filterl)
struct type_Qmgr_FilterList *filterl;
{
	while(filterl) {
		print_filter (filterl -> Filter);
		filterl = filterl -> next;
	}
}
