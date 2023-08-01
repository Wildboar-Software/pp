/* testchan.c: as invoked by qmgr to do benchmarking stuff*/

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/testchan/RCS/testchan.c,v 6.0 1991/12/18 20:12:42 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/testchan/RCS/testchan.c,v 6.0 1991/12/18 20:12:42 jpo Rel $
 *
 * $Log: testchan.c,v $
 * Revision 6.0  1991/12/18  20:12:42  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "chan.h"
#include "prm.h"
#include "q.h"
#include "dr.h"
#include "qmgr.h"
#include <pwd.h>
#include "sys.file.h"
#include <signal.h>
#include <isode/internet.h>

extern  char    *quedfldir;
extern void	rd_end(), chan_init(), err_abrt(), timer_start(), timer_end();
FILE            *msg_fp;
CHAN            *mychan;
char            *this_msg;


static char *files[] = {
	"/usr/dict/words",
	"/etc/passwd",
	"/usr/adm/messages",
	"/etc/motd",
	"/etc/hosts",
	"/etc/printcap",
	"/etc/rc.local",
	NULLCP
	};
#define RANDFILE() (files[random() % (sizeof(files)/sizeof(*files) -1)])

static int data_bytes;
extern time_t time();


static struct type_Qmgr_DeliveryStatus *process();
static void dirinit();
static int chaninit();
static int endproc ();
static int dotext ();
static int copy ();
static struct hostent *find_host ();
static FILE *serv_open();
#define OPT_RANDOM 1
#define OPT_SLEEP 2
#define OPT_NETECHO 3
int option = OPT_RANDOM;
/* ---------------------  Begin  Routines  -------------------------------- */




main (argc, argv)
int     argc;
char    **argv;
{
	char    *p;
	int opt;

	if ((p = rindex (argv[0], '/')) != NULLCP)
		p ++;
	if (p == NULLCP || *p == NULL)
		p = argv[0];

	chan_init (p);   /* init the channel - and find out who we are */

	dirinit();              /* get to the right directory */
	(void) signal (SIGPIPE, SIG_IGN);

	while ((opt = getopt (argc, argv, "es")) != EOF) {
		switch (opt) {
		    case 's':
			option = OPT_SLEEP;
			break;
		    case 'e':
			option = OPT_NETECHO;
			break;
		}
	}

#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0)
		debug_channel_control (argc, argv, chaninit, process, endproc);
	else
#endif
		channel_control (argc, argv, chaninit, process, endproc);
	exit (0);
}



static int chaninit (arg)
struct type_Qmgr_Channel *arg;
{
	char    *p = qb2str (arg);

	if ((mychan = ch_nm2struct (p)) == (CHAN *)0)
		err_abrt (RP_PARM, "Channel '%s' not known", p);

	srandom (getpid());
	rename_log(p); 
	PP_NOTICE (("starting %s (%s)", mychan -> ch_name, mychan -> ch_show));
	free (p);
	return OK;
}

static int endproc ()
{
	PP_NOTICE (("Closing %s", mychan -> ch_name));
}

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars prm;
	struct type_Qmgr_UserList *up;
	Q_struct        Qstruct, *qp = &Qstruct;
	int     retval;
	ADDR    *ap,
		*ad_sendr = NULLADDR,
		*ad_recip = NULLADDR,
		*alp = NULLADDR,
		*ad_list = NULLADDR;
	int     ad_count;
	char *cur_host = NULLCP;

	if (this_msg) free (this_msg);

	this_msg = qb2str (arg -> qid);

	bzero ((char *)&prm, sizeof prm);
	bzero ((char *)qp, sizeof *qp);

	(void) delivery_init (arg -> users);

	retval = rd_msg (this_msg, &prm, qp, &ad_sendr, &ad_recip, &ad_count);

	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("rd_msg err: %s", this_msg));
		return delivery_setallstate (int_Qmgr_status_messageFailure,
					     "Can't read message");
	}

	for (ap = ad_recip; ap; ap = ap -> ad_next) {
		for (up = arg ->users; up; up = up -> next) {
			if (up -> RecipientId -> parm != ap -> ad_no)
				continue;

			switch (chan_acheck (ap, mychan,
					     ad_list == NULLADDR, &cur_host)) {
			    default:
			    case NOTOK:
				continue;

			    case OK:
				break;
			}
			break;
		}
		if (up == NULL)
			continue;

		if (ad_list == NULLADDR)
			ad_list = alp = (ADDR *) calloc (1, sizeof *alp);
		else {
			alp -> ad_next = (ADDR *) calloc (1, sizeof *alp);
			alp = alp -> ad_next;
		}
		*alp = *ap;
		alp -> ad_next = NULLADDR;
	}

	if (ad_list == NULLADDR) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipients in user list"));
		rd_end ();
		q_free (qp);
		return deliverystate;
	}

	PP_NOTICE (("processing msg %s to %s", this_msg, cur_host));

	deliver (ad_list, qp);

	q_free (qp);
	rd_end();

	return deliverystate;
}

static void dirinit()       /* Change into pp queue space */
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, "Unable to change directory to '%s'",
						quedfldir);
}

deliver (ad_list, qp)
ADDR    *ad_list;
Q_struct *qp;
{
	ADDR    *ap;

	PP_TRACE (("deliver ()"));

	switch (option) {
	    case OPT_NETECHO:
		do_echo ("/usr/dict/words");
		break;
	    case OPT_SLEEP:
		do_sleep (5);
		break;
	    case OPT_RANDOM:
		do_random ();
		break;
	}
	if (option == OPT_RANDOM) {
		if (random()% 100 == 1) {
			for (ap = ad_list; ap; ap = ap -> ad_next) {
				delivery_setall (int_Qmgr_status_negativeDR);
				PP_LOG (LLOG_EXCEPTIONS, ("problem with user"));
				set_1dr (qp, ap -> ad_no,
#ifndef PP52
					 this_msg,
#endif
					 DRR_UNABLE_TO_TRANSFER,
					 DRD_UNRECOGNISED_OR,
					 "problem with recipient");
				ap -> ad_resp = 0;
			}
			if (rp_isbad(wr_q2dr (qp, this_msg)))
				delivery_resetDRs (int_Qmgr_status_messageFailure);
			return;
		}
		else if (random () % 50 < 10) {
			PP_LOG (LLOG_EXCEPTIONS, ("problem with message"));
			delivery_setallstate (int_Qmgr_status_messageFailure,
					      "problem with message");
			return;
		}
		else if (random() % 50 < 10) {
			PP_LOG (LLOG_EXCEPTIONS, ("problem with MTA"));
			delivery_setallstate (int_Qmgr_status_mtaFailure, "MTA missing");
			return;
		}
		else if (random() % 50 < 10) {
			PP_LOG (LLOG_EXCEPTIONS, ("problem with message & MTA"));
			delivery_setallstate (int_Qmgr_status_mtaAndMessageFailure,
					      "MTA died");
			return;
		}
	}

	for (ap = ad_list; ap; ap = ap -> ad_next) {
		if (ap -> ad_resp) {
			if (ap -> ad_usrreq == AD_USR_CONFIRM ||
			    ap -> ad_mtarreq == AD_MTA_CONFIRM ||
			    ap -> ad_mtarreq == AD_MTA_AUDIT_CONFIRM) {
				set_1dr (qp, ap -> ad_no,
#ifndef PP52
					 this_msg,
#endif
					 DRR_NO_REASON, -1, NULLCP);
				delivery_set (ap -> ad_no,
					      int_Qmgr_status_positiveDR);
			}
			else {
				PP_NOTICE (("address OK"));
				(void) wr_ad_status (ap, AD_STAT_DONE);
#ifndef PP52
				(void) wr_stat (ap, qp, this_msg, data_bytes);
#endif
				delivery_set (ap -> ad_no,
					      int_Qmgr_status_success);
			}
		}
	}
	if (rp_isbad(wr_q2dr (qp, this_msg)))
		delivery_resetDRs(int_Qmgr_status_messageFailure);
	PP_NOTICE ((">>> Message %s transfered",
		    this_msg));
}       
			
do_random ()
{
	switch (random() % 5) {
	    case 0:
		do_sleep (random()%60);
		break;
	    case 2:
		mess_with_network ();
		break;
	    case 3:
		cpu_intensive ();
		break;
	    case 4:
		mem_intensize ();
		break;
	    default:
		mess_with_files (RANDFILE());
		break;
	}
}
do_sleep (secs)
int secs;
{
	PP_NOTICE (("sleeping %d", secs));
	sleep (secs);
}

mess_with_files (infile)
char *infile;
{
	char *outfile;
	FILE *fp, *fpo;
	int n;

	if ((fp = fopen (infile, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, infile, ("Can't open "));
		return;
	}
	if (random() % 2 == 0) {
		outfile = "/dev/null";
		if ((fpo = fopen (outfile, "w")) == NULL ) {
			PP_SLOG (LLOG_EXCEPTIONS, infile, ("Can't open "));
			fclose (fp);
			return;
		}	
		PP_NOTICE (("mess_with_files %s->/dev/null", infile));
	}
	else {
		PP_NOTICE (("mess_with_files %s -> temp", infile));

		fpo = tmpfile ();
	}
		
	n = copy (fp, fpo);
	PP_NOTICE (("copied %d bytes", n));
	(void) fclose (fp);
	(void) fclose (fpo);
}


mess_with_network ()
{
	switch (random() % 3) {
	    case 0:
		do_echo (RANDFILE());
		break;
	    case 1:
		do_sink (RANDFILE());
		break;
	    default:
		do_chargen ();
		break;
	}
}

jmp_buf jmpbuf;

static SFD alarmed()
{
	longjmp (jmpbuf, 1);
}

do_echo (infile)
char *infile;
{
	FILE *fp;
	FILE *fpi;
	char buf[BUFSIZ];
	int n;
	int total = 0;
	SFP pstat;

	PP_NOTICE (("do echo of %s", infile));
	if ((fp = serv_open ("echo")) == NULL)
		return;
	if ((fpi = fopen (infile, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, infile, ("Can't open "));
		fclose (fp);
		return;
	}
	pstat = signal (SIGALRM, alarmed);
	if (setjmp (jmpbuf) == 0) {
		alarm (60);
		while ((n = fread (buf, 1, sizeof buf, fpi)) > 0) {
			if (fwrite (buf, 1, n, fp) != n) {
				PP_SLOG (LLOG_EXCEPTIONS, "error", ("write"));
				break;
			}
			total += n;
			if (fread (buf, 1, n, fp) != n)
				PP_LOG (LLOG_EXCEPTIONS, ("copy back failed"));
		}
		alarm(0);
	}
	PP_NOTICE (("Echoed %d bytes", total));
	(void) fclose (fpi);
	(void) fclose (fp);
	signal(SIGALRM, pstat);
}

do_sink (infile)
char *infile;
{
	FILE *fp, *fpi;
	int n;

	PP_NOTICE (("do_sink of %s", infile));
	if ((fp = serv_open ("sink")) == NULL)
		return;

	if ((fpi = fopen(infile, "r")) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, infile, ("Can't open "));
		fclose (fp);
		return;
	}
	n = copy (fpi, fp);
	PP_NOTICE (("Sunk %d bytes", n));
	(void) fclose (fpi);
	(void) fclose (fp);
}

do_chargen ()
{
	FILE *fp;
	char buf[BUFSIZ];
	int i;
	int n;
	int total = 0;
	int iter = random () % 2000;

	PP_NOTICE (("do_chargen %d buffers", iter));
	if ((fp = serv_open ("chargen")) == NULL)
		return;

	for (i = iter; i > 0; i--) { 
		if ((n = fread (buf, 1, sizeof buf, fp)) <= 0)
			break;
		total += n;
	}
	(void) fclose (fp);
	PP_NOTICE (("Received %d bytes", total));
}
	
static int copy (fp1, fp2)
FILE *fp1, *fp2;
{
	char buf[BUFSIZ];
	int total = 0;
	int n;

	while ((n = fread (buf, 1, sizeof buf, fp1)) > 0) {
		if (fwrite (buf, 1, n, fp2) != n) {
			PP_SLOG (LLOG_EXCEPTIONS, "write", ("problems with "));
			return NOTOK;
		}
		total += n;
	}
	if (n == 0)
		return total;
	PP_SLOG (LLOG_EXCEPTIONS, "read", ("problems with "));
	return NOTOK;
}

			

static FILE *serv_open (name)
char *name;
{
	struct servent *sp;
	struct sockaddr_in s_in;
	struct hostent *hp;
	int sd;

	if ((sp = getservbyname (name, "tcp")) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("No such service %s", name));
		return NULL;
	}

	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		PP_SLOG (LLOG_EXCEPTIONS, "socket", ("Can't create"));
		return NULL;
	}
	s_in.sin_family = AF_INET;
	s_in.sin_port = sp -> s_port;

	if ((hp = find_host ()) == NULL) {
		close (sd);
		return NULL;
	}
		
		
	bcopy (hp -> h_addr, (char *)&s_in.sin_addr, hp -> h_length);
	if (connect (sd, &s_in, sizeof s_in) < 0) {
		PP_SLOG (LLOG_EXCEPTIONS, "connect", ("Can't "));
		close (sd);
		return NULL;
	}

	return fdopen (sd, "r+");
}

static char *host_list[500] = {
	"localhost"
	};
static int n_hosts = 1;
#define RANDHOST() (host_list[random() % n_hosts])	

static struct hostent *find_host ()
{
	static int inited = 0;
	char	*host;
	static void init_hosts ();
	int i;
	struct hostent *hp;

	if (!inited) {
		init_hosts ();
		inited ++;
	}
	for (i = 0;i < 20; i++) {
		if ((hp = gethostbyname (host = RANDHOST())) == NULL)
			continue;
		PP_NOTICE (("Using host %s", host));
		return hp;
	}
	return NULL;
}

static void init_hosts ()
{
	char path[BUFSIZ];
	extern char *tbldfldir;
	FILE *fp;
	char *cp;

	getfpath (tbldfldir, "hosts", path);

	if ((fp = fopen (path, "r")) == NULL)
		return;
	while (fgets(path, sizeof path, fp)) {
		if ((cp = index(path, '\n')) != NULL)
			*cp = 0;
		host_list[n_hosts++] = strdup (path);
	}
	fclose (fp);
}

static double fact(n)
int n;
{
	if (n <= 1)
		return 1;
	return fact(n-1) * (double)n;
}

cpu_intensive ()
{
	int niter = random()%1000;
	int i;
	double result = 1.0;

	PP_NOTICE (("working out e with %d iterations", niter));
	/* work out e for now */
	for (i = 1; i < niter; i++) {
		result = result + (double)1 / fact(i);
	}
	PP_NOTICE (("result e is %.20lf", result));
}


mem_intensize ()
{
	int ps = getpagesize ();
	int npages = random() % 100;
	char **pages;
	int i, j;
	char c;

	PP_NOTICE (("memory stuff based on %d pages", npages));
	pages = (char **) calloc(npages, sizeof (char **));
	for (i = 0; i < npages; i++)
		pages[i] = malloc (ps);
	for (j = random() % 100; j > 0; j--) 
		for (i = 0; i < npages; i++) {
			c = pages[i][0] % 4;
			fact(c);
		}
}
