/* ckchan.c: standalone channel checker */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ckchan/RCS/ckchan.c,v 6.0 1991/12/18 20:28:55 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ckchan/RCS/ckchan.c,v 6.0 1991/12/18 20:28:55 jpo Rel $
 *
 * $Log: ckchan.c,v $
 * Revision 6.0  1991/12/18  20:28:55  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <pwd.h>
#include <isode/usr.dirent.h>
#include <sys/stat.h>
#include	<signal.h>

#include "q.h"
#include "retcode.h"
#include "prm.h"


extern	void getfpath ();
extern CHAN     *ch_nm2struct();
extern char	*quedfldir;
#define	TABSIZE	6

static void     build_chanlist();
static struct my_chan_struct *get_chan();
static struct my_chan_struct *add_chan();
static void order_mtas();
static void add_msg();
static void     add_to_chanlist();
static void     print_chanlist();
static void print_chan();
static void print_mta();
static void print_msg();
static void print_drmta();
static void print_drmsg();
static void dirinit();
static void free_chanlist();
static LIST_RCHAN *getnthchannel();

#define TIMETROUBLE	24 * 60 * 60
time_t	timetrouble = TIMETROUBLE;
#define noMTA	"NO MTA!"

typedef struct recip_struct {
	char			*recip;
	int			status;
	struct recip_struct	*recip_next;
} Recip_struct;

typedef struct msg_struct {
	time_t			age;
	char                    *msg;
	char			*ad_value;
	struct recip_struct	*recips;       
	struct msg_struct       *msg_next;
} Msg_struct;

typedef struct mta_struct {
	char	 		*mta;
	int			nmsgs;
	time_t			oldestmsg;
	struct msg_struct	*msgs;
	struct mta_struct	*mta_next;
} Mta_struct;

typedef struct my_chan_struct {
	char			*name;
	struct mta_struct	*mtas;
	struct mta_struct	*drmtas;
	struct my_chan_struct	*chan_next;
} My_chan_struct;

int                     all,
			verbose,
			summary;
struct my_chan_struct     *chan_list = NULL;

void main (argc, argv)
int     argc;
char    **argv;
{
	struct stat	statbuf;
	struct prm_vars prm;
	Q_struct        que;
	ADDR            *sender = NULL;
	ADDR            *recips = NULL;
	int             rcount;
	int		opt,
			flags;
	extern int	optind;
	extern char	*optarg;
	char		**msgs = NULLVP;
	extern char     *quedfldir;
	extern char     *aquefile, *postmaster;
	int		num, i;
	all = FALSE;
	flags = 0;
	verbose = FALSE;
	summary = FALSE;

	while ((opt = getopt(argc, argv, "vs")) != EOF) {
		switch (opt) {
		    case 'v':
			/* verbose output */
			verbose = TRUE;
			flags++;
			break;
		    case 's':
			/* summary output */
			summary = TRUE;
			flags++;
			break;
		    default:
			printf("usage : %s [-v] [-s] [ chan ...]\n",argv[0]);
			exit(1);
		}
	}

	sys_init(argv[0]);
	(void) signal(SIGPIPE, SIG_DFL);
	dirinit();
	argv += flags + 1;
	
	if (*argv == NULL)
		all=TRUE;
	else
		build_chanlist(argv);

	if (stat(quedfldir, &statbuf) != OK) {
		printf("Unable to find spool area\nPlease inform %s\n", postmaster);
		(void) exit(1);
	}
	printf("Please wait while information is processed...");
	fflush(stdout);
	num = i = 0;
	hier_scanQ (quedfldir, NULLCP, &num, &msgs, &i, NULLIFP);
	for (i = 0; i < num; i++) {
		sender = NULL;
		recips = NULL;
		prm_init (&prm);
		q_init(&que);
		if (rp_isbad(rd_msg_file(msgs[i],
					 &prm,&que,
					 &sender,&recips,
					 &rcount,
					 RDMSG_RDONLY))) {
			rd_end();
			printf("Bad message %s : rd_msg fails\n",msgs[i]);
		} else {
			rd_end();
			add_to_chanlist(msgs[i],&que);
			q_free(&que);
		}

		

	}
	printf("done\n");
	print_chanlist();

	
	if (num > 0 && msgs != NULLVP)
		free_hier(msgs, num);
        free_chanlist();
	exit(0);
}

static void dirinit()
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO,
			  " Unable to change directory to '%s'",
			  quedfldir);
}

static char	*time_t2str(in)
time_t  in;
{
	char	buf[BUFSIZ];
	time_t	result;
	buf[0] = '\0';
	
	if (in < 0)
		return strdup("still in the womb");

	if ((result = in / (60 * 60 * 24)) != 0) {
		sprintf(buf, "%d days",result);
		in = in % (60 * 60 * 24);
	}

	if ((result = in / (60 * 60)) != 0) {
		sprintf(buf, (buf[0] == '\0') ? "%s%d hrs" : "%s %d hrs",
			buf, result);
		in = in % (60 * 60);
	}
	if ((result = in / 60) != 0) {
		sprintf(buf, (buf[0] == '\0') ? "%s%d mins" : "%s %d mins",
			buf, result);
		in = in % 60;
	}

	if (buf[0] == '\0' && in != 0)
		sprintf(buf, "%d secs", in);
	if (buf[0] == '\0')
		sprintf(buf, "just born");
	return strdup(buf);
}

static char	*time_t2RFC(in)
time_t	in;
{
	char	buf[BUFSIZ],
		*str;
	time_t	curr,
		delta;

	curr = time(0);
	delta = curr - in;
	
	str = time_t2str(delta);
	sprintf(buf,"%s%s",
		str,
		(delta > timetrouble) ? " *****" : "");
	free(str);
	return strdup(buf);
}

static void build_chanlist(chans)
char            **chans;
{
	CHAN                    *chan;
	char                    **ix = chans;
	while (*ix != NULL) {
		if (lexequ(*ix, "ALL") == 0)
			all = TRUE;
		else if ((chan = ch_nm2struct(*ix)) == NULLCHAN) 
			printf("unknown channel '%s'\n",*ix);
		else 
			(void) add_chan(chan);
		ix++;
	}
}


static struct mta_struct *get_mta(list, mta)
struct mta_struct	*list;
char			*mta;
{
	struct mta_struct	*ix = list;
	
	while (ix != NULL && (strcmp(ix->mta,mta) != 0))
		ix = ix -> mta_next;
	return ix;
}

static struct msg_struct *get_msg(list, key)
struct msg_struct 	*list;
char			*key;
{
	struct msg_struct	*ix = list;

	while (ix != NULL && (strcmp(ix->msg,key) != 0))
		ix = ix -> msg_next;
	return ix;
}

static struct my_chan_struct *get_chan(chan)
CHAN    *chan;
{
	struct my_chan_struct     *ix = chan_list;

	while ((ix != NULL) && (strcmp(ix->name,chan->ch_name) != 0))
		ix = ix->chan_next;
	return ix;
}

static struct my_chan_struct *add_chan(chan)
CHAN    *chan;
{
	struct my_chan_struct     *temp = (struct my_chan_struct *) calloc(1, sizeof(*temp));
	temp->name = strdup(chan->ch_name);
	temp->chan_next = chan_list;
	chan_list = temp;
	return chan_list;
}

static void add_recip(plist, adr)
struct recip_struct	**plist;
ADDR			*adr;
{
	struct recip_struct	*ret = (struct recip_struct *) calloc(1, sizeof(*ret));

	ret -> recip = strdup(adr -> ad_value);
	ret -> status = adr -> ad_status;

	ret -> recip_next = *plist;
	*plist = ret;
}

static struct msg_struct *new_msg(msg, que, recip)
char		*msg;
Q_struct	*que;
ADDR		*recip;
{
	struct msg_struct	*ret = (struct msg_struct *) calloc(1, sizeof(*ret));

	ret -> age = utc2time_t(que -> queuetime);
	ret -> msg = strdup(msg);
	ret -> ad_value = strdup(que->Oaddress->ad_value);
	add_recip(&ret -> recips, recip);
	
	return ret;
}
static void add_msg(plist, msg)
struct msg_struct	**plist,
			*msg;
{
	struct msg_struct	*ix = *plist;

	if (*plist == NULL
	    || (*plist) -> age > msg -> age) {
		msg -> msg_next = *plist;
		*plist = msg;
	} else {
		while ( ix -> msg_next != NULL
		       && ix -> msg_next -> age <= msg -> age)
			ix = ix -> msg_next;
		msg -> msg_next = ix -> msg_next;
		ix -> msg_next = msg;
	}
}

#define	NEW	0
#define OLD	1

static int update_msg(plist,msg,que,recip)
struct msg_struct	**plist;
char            	*msg;
Q_struct        	*que;
ADDR            	*recip;
{
	struct msg_struct	*temp;

	if ((temp = get_msg(*plist,msg)) == NULL) {
		temp = new_msg(msg, que, recip);
		add_msg(plist, temp);
		return NEW;
	} else {
		add_recip(&temp->recips, recip);
		return OLD;
	}
}

static void update_mta(mta, msg, que, recip)
struct mta_struct	*mta;
char			*msg;
Q_struct		*que;
ADDR			*recip;
{
	time_t	qtime;

	qtime = utc2time_t(que->queuetime);
	if (qtime < mta -> oldestmsg)
		mta -> oldestmsg = qtime;
	if (update_msg(&mta->msgs, msg, que, recip) == NEW)
		mta->nmsgs++;

}

static void new_mta(plist, msg, que, recip, inmta)
struct mta_struct	**plist;
char			*msg;
Q_struct		*que;
ADDR			*recip;
char			*inmta;
{
	struct mta_struct *temp = (struct mta_struct *) calloc(1, sizeof(*temp));
	if (recip ->ad_outchan && recip->ad_outchan->li_mta != NULLCP)
		temp -> mta = strdup(recip -> ad_outchan -> li_mta);
	else if (inmta != NULL)
		temp -> mta = strdup(inmta);
	else
		temp->mta = strdup(noMTA);

	temp -> nmsgs = 1;
	temp -> oldestmsg = utc2time_t(que -> queuetime);
	update_msg(&temp->msgs, msg, que, recip);
	temp -> mta_next = *plist;
	*plist = temp;
}

static void add_mta(msg,que,recip,chan)
char            *msg;
Q_struct        *que;
ADDR            *recip;
CHAN            *chan;
{
	struct my_chan_struct	*temp;
	struct mta_struct	*mta;
	char			*chmta;
	if ((temp = get_chan(chan)) == NULL && all == TRUE)
		temp = add_chan(chan);

	if (temp == NULL)
	/* not interested in this channel */
		return;

	if (recip->ad_outchan && recip->ad_outchan->li_mta)
		chmta = recip->ad_outchan->li_mta;
	else
		chmta = noMTA;
	if ((mta = get_mta(temp -> mtas,chmta)) == NULL)
		new_mta(&(temp -> mtas),
			msg,
			que,
			recip,
			chmta);
	else 
		update_mta(mta,msg,que,recip);
}

static void add_drmta(msg,que,sender,chan)
char            *msg;
Q_struct        *que;
ADDR            *sender;
LIST_RCHAN	*chan;
{
	struct my_chan_struct	*temp;
	struct mta_struct	*mta;
	char			*chmta;

	if (chan->li_chan == NULL 
		|| chan->li_chan->ch_name == NULL) {
			PP_OPER(NULL, ("Outchan for DR not set (sorry can't be of more help !)"));
			return;
		}
	if ((temp = get_chan(chan->li_chan)) == NULL && all == TRUE)
		temp = add_chan(chan->li_chan);

	if (temp == NULL)
	/* not interested in this channel */
		return;
	if (sender->ad_outchan && sender->ad_outchan->li_mta)
		chmta = sender->ad_outchan->li_mta;
	else if (chan->li_mta)
		chmta = chan->li_mta;
	else
		chmta = noMTA;
	if ((mta = get_mta(temp -> drmtas, chmta)) == NULL)
		new_mta(&(temp -> drmtas),
			msg,
			que,
			sender,
			chmta);
	else 
		update_mta(mta,msg,que,sender);
}
	      
static void add_to_chanlist(msg,que)
char            *msg;
Q_struct        *que;
{
	ADDR            *ix = que->Raddress;
	LIST_RCHAN      *chan,
			*chanix;
	while (ix != NULL) {
		if (ix -> ad_status == AD_STAT_DRWRITTEN
		    || ix -> ad_status == AD_STAT_DRREQUIRED) {
			if ((chan = getnthchannel(que->Oaddress->ad_fmtchan,
						  que->Oaddress->ad_rcnt)) != NULL)
				add_drmta(msg,que, que->Oaddress, chan);
			else {
				chanix = que->Oaddress->ad_outchan;

				while (chanix != NULL) {
					add_drmta(msg, que, que->Oaddress, chanix);
					chanix = chanix->li_next;
				}
			}

		} else if (ix -> ad_status != AD_STAT_DONE) {
			/* must be on a channel */
			/* check reformaters */
			if ((chan = getnthchannel(ix->ad_fmtchan, ix -> ad_rcnt)) != NULL) 
				/* still needs reformating */
				add_mta(msg,que,ix,chan->li_chan);
			else {
				/* on outbound channel */
				chanix = ix->ad_outchan;

				while (chanix != NULL) {
					add_mta(msg,que,ix,chanix->li_chan);
					chanix = chanix->li_next;

				}
			}
		}
		ix = ix->ad_next;
	}
}

int	total_nummsgs = 0, total_nummtas = 0;
time_t	veryoldest = 0;
int	total_numdrs = 0, total_numdrmtas = 0;
time_t	veryoldestdr = 0;

static void print_chanlist()
{
	struct my_chan_struct *ix = chan_list;
	

	while (ix != NULL) {
		print_chan(ix);
		ix = ix -> chan_next;
	}
	if (summary == TRUE && 
	    (total_numdrs != 0 || total_nummsgs != 0)) {
		char	*timestring;
		printf ("\nTotals\n======\n");
		if (total_nummsgs != 0) {
			timestring = time_t2RFC(veryoldest);
			printf("%d msgs on %d mtas (oldest %s)\n",
			       total_nummsgs, total_nummtas, timestring);
			free(timestring);
		}
		if (total_numdrs != 0) {
			timestring = time_t2RFC(veryoldestdr);
			printf("%d DRs on %d mtas (oldest %s)\n",
			       total_numdrs, total_numdrmtas, timestring);
			free(timestring);
		}
	}
			
}		

static void print_chan(chanptr)
struct my_chan_struct	*chanptr;
{
	struct mta_struct	*ix;
	int	num_mtas = 0, num_msgs = 0;
	time_t	oldestmsg = 0;
	int	num_drmtas = 0, num_drs = 0;
	time_t	oldestdr = 0;
	char	*timestring;

	if (summary == TRUE) {
		for (ix = chanptr->mtas; 
		     ix != (struct mta_struct *) NULL;
		     ix = ix -> mta_next) {
			num_mtas++;
			num_msgs += ix -> nmsgs;
			if (ix -> oldestmsg < oldestmsg ||
			    oldestmsg == 0)
				oldestmsg = ix -> oldestmsg;
		}
		total_nummtas += num_mtas;
		total_nummsgs += num_msgs;
		if (oldestmsg < veryoldest ||
		    veryoldest == 0)
			veryoldest = oldestmsg;

		for (ix = chanptr -> drmtas;
		     ix != (struct mta_struct *) NULL;
		     ix = ix -> mta_next) {
			num_drmtas++;
			num_drs += ix -> nmsgs;
			if (ix -> oldestmsg < oldestdr ||
			    oldestdr == 0)
				oldestdr = ix -> oldestmsg;
		}
		total_numdrmtas += num_drmtas;
		total_numdrs += num_drs;
		if (oldestdr < veryoldestdr ||
		    veryoldestdr == 0)
			veryoldestdr = oldestdr;
		printf ("%s ", chanptr->name);
		if (num_msgs != 0) {
			timestring = time_t2RFC(oldestmsg);
			printf("%d msgs on %d mtas (oldest %s)\n",
			       num_msgs, num_mtas, timestring);
			free(timestring);
		}
		if (num_drs != 0) {
			if (num_msgs != 0)
				printf ("%*c",strlen(chanptr->name), ' ');
			timestring = time_t2RFC(oldestdr);
			printf("%d DRs on %d mtas (oldest %s)\n",
			       num_drs, num_drmtas, timestring);
			free(timestring);
		}
	} else {
		printf("%s\n",chanptr->name);
	
		order_mtas(&(chanptr->mtas));
		if ((ix = chanptr -> mtas) != NULL)
			printf("%*smessages\n",TABSIZE/2,"");
		while (ix != NULL) {
			print_mta(ix);
			ix = ix -> mta_next;
		}
	
		order_mtas(&(chanptr->drmtas));
		if ((ix = chanptr -> drmtas) != NULL)     
			printf("%*sDRs\n",TABSIZE/2,"");
		while (ix != NULL) {
			print_drmta(ix);
			ix = ix -> mta_next;
		}
	}
}

static void print_mta(mta)
struct mta_struct	*mta;
{
	struct msg_struct	*ix;
	char			*timestring;
	if (verbose == TRUE) {
		printf("%*s%s\n",TABSIZE,"",mta->mta);
		ix = mta->msgs;
		while (ix != NULL) {
			print_msg(ix);
			ix = ix -> msg_next;
		}
	} else {
		timestring = time_t2RFC(mta->oldestmsg);
		printf("%*s%-25s %d%*s%s\n",
		       TABSIZE,"",
		       mta -> mta,
		       mta -> nmsgs,
		       TABSIZE,"",
		       timestring);
		free(timestring);
	}
}

static void print_drmta(mta)
struct mta_struct	*mta;
{
	struct msg_struct	*ix;
	char			*timestring;
	
	if (verbose == TRUE) {
		printf("%*s%s\n",TABSIZE,"",mta->mta);
		ix = mta->msgs;
		while (ix != NULL) {
			print_drmsg(ix);
			ix = ix -> msg_next;
		}
	} else {
		timestring = time_t2RFC(mta->oldestmsg);
		printf("%*s%-25s %d%*s%s\n",
		       TABSIZE,"",
		       mta -> mta,
		       mta -> nmsgs,
		       TABSIZE,"",
		       timestring);
		free(timestring);
	}
}
char	*status_str[] = {
	"unknown",
	"pend",
	"dr req",
	"dr written",
	"done"
	};

static void print_msg(msg)
struct msg_struct	*msg;
{
	char			*timestring = time_t2RFC(msg->age);
	struct recip_struct	*ix = msg -> recips;

	printf("%*s%-25s %s\n",
	       2*TABSIZE,"",
	       msg->msg,
	       timestring);
	printf("%*sfrom %-30s",
	       3*TABSIZE,"",
	       msg->ad_value);	       

	if (ix == NULL)
		printf (" %s\n", status_str[AD_STAT_DRWRITTEN]);
	else
		printf("\n");

	while (ix != NULL) {
		printf("%*sto %-30s %s\n",
		       3*TABSIZE,"",
		       ix -> recip,
		       status_str[ix -> status]);
		ix = ix -> recip_next;
	}
	free(timestring);
}

static void print_drmsg(msg)
struct msg_struct	*msg;
{
	char			*timestring = time_t2RFC(msg->age);
	struct recip_struct	*ix = msg -> recips;

	printf("%*s%-25s %s\n",
	       2*TABSIZE,"",
	       msg->msg,
	       timestring);

	printf("%*sto %-30s\n",
	       3*TABSIZE,"",
	       msg->ad_value);	       

	while (ix != NULL) {
		printf("%*sfrom recip %-30s\n",
		       3*TABSIZE,"",
		       ix -> recip);
		ix = ix -> recip_next;
	}
	free(timestring);
}

static LIST_RCHAN *getnthchannel(chans,num)
LIST_RCHAN      *chans;
int             num;
{
	LIST_RCHAN      *ix = chans;
	int             icount = 0;

	while ((ix != NULL) && (icount++ < num)) 
		ix = ix->li_next;

	return ix;
}

static void insert_mta(plist, mta)
struct mta_struct	**plist,
			*mta;
{
	struct mta_struct	*ix = *plist;

	if (*plist == NULL
	    || (*plist) -> oldestmsg > mta -> oldestmsg) {
		mta -> mta_next = *plist;
		*plist = mta;
	} else {
		while ( ix -> mta_next != NULL
		       && ix -> mta_next -> oldestmsg <= mta -> oldestmsg)
			ix = ix -> mta_next;
		mta -> mta_next = ix -> mta_next;
		ix -> mta_next = mta;
	}
}

static void order_mtas(plist)
struct mta_struct	**plist;
{
	struct mta_struct	*new = NULL,
				*head = *plist,
				*tail;

	while (head != NULL) {
		tail = head -> mta_next;
		insert_mta(&new, head);
		head = tail;
	}

	*plist = new;
}

/*  */

static void free_recips(recips)
struct recip_struct	*recips;
{
	struct recip_struct	*temp;
	
	while (recips != NULL) {
		temp = recips;
		if (temp->recip)
			free(temp->recip);
		recips = temp -> recip_next;
		free((char *) temp);
	}
}

static void free_msgs (msgs)
struct msg_struct	*msgs;
{
	struct msg_struct	*temp;

	while (msgs != NULL) {
		temp = msgs;
		if (temp->msg)
			free(temp->msg);
		if (temp->ad_value)
			free(temp->ad_value);
		if (temp->recips)
			free_recips(temp->recips);
		msgs = temp->msg_next;
		free((char *) temp);
	}
}

static void free_mtas(mtas)
struct mta_struct	*mtas;
{
	struct mta_struct	*temp;

	while (mtas != NULL) {
		temp = mtas;
		if (mtas->mta)
			free(mtas->mta);
		if (mtas->msgs)
			free_msgs(mtas->msgs);
		mtas = mtas->mta_next;
		free((char *) temp);
	}
}

static void free_chanlist()
{
	struct my_chan_struct	*temp;
	while (chan_list != NULL) {
		temp = chan_list;
		if (temp -> name)
			free(temp->name);
		if (temp->mtas)
			free_mtas(temp->mtas);
		if (temp->drmtas)
			free_mtas(temp->drmtas);
		chan_list = temp->chan_next;
		free((char *) temp);
	}
}
       
