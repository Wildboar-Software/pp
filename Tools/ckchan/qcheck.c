/* qcheck.c: stand alone checker of q */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ckchan/RCS/qcheck.c,v 6.0 1991/12/18 20:28:55 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ckchan/RCS/qcheck.c,v 6.0 1991/12/18 20:28:55 jpo Rel $
 *
 * $Log: qcheck.c,v $
 * Revision 6.0  1991/12/18  20:28:55  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/usr.dirent.h>
#include "q.h"
#include "retcode.h"
#include "prm.h"


static int QPrint();
static int RecipPrint();
static LIST_RCHAN *getnthchannel();
static char     **get_all_messages();

main (argc, argv)
int     argc;
char    **argv;
{
	char    **all_msgs;
	if (argc < 2) {
		printf("usage : %s [all | msg.XXXX ...]\n",argv[0]);
		exit(1);
	}
	sys_init(argv[0]);
	argv++;

	if (strcmp(*argv,"all") == 0) {
		/* get all messages in addr, directory */
		all_msgs = get_all_messages();
		qcheck(all_msgs);
	} else
		qcheck(argv);
}

int     isMsg(entry)
struct dirent   *entry;
{
	if (strncmp(entry->d_name,"msg.",4) == 0)
		return 1;
	else 
		return 0;
}

static char     **get_all_messages()
{
	int             num,
			i = 0;
	struct dirent   **namelist = NULL,
			**ix;
	char            **ret;
	extern char     *quedfldir;

	num = scandir(quedfldir, &namelist, isMsg, NULL);

	ret = (char **) calloc((num+1), sizeof(char *));
	ix = namelist;
	while ((i < num) && (*ix != 0)) {
		ret[i++] = (*ix)->d_name;
		ix++;
	}

	if (namelist) free((char *) namelist);

	return ret;
}


qcheck(argv)
char **argv;
{
	char            *file = NULL;
	struct prm_vars prm;
	Q_struct        que;
	ADDR            *sender = NULL;
	ADDR            *recips = NULL;
	ADDR    *ix;
	int             rcount;

	prm_init (&prm);
	q_init (&que);
	qinit (&que);


	while ((file = *argv++) != NULLCP) {
		sender = NULL;
		recips = NULL;
		prm_init (&prm);
		q_init (&que);
		if (rp_isbad(rd_msg(file,&prm,&que,&sender,&recips,&rcount))) {
			rd_end();
			printf("Bad message %s : rd_msg fails\n",file);
		} else {
			rd_end();
			QPrint(file,&que,sender);
			ix = recips;
			while (ix != NULL) {
				RecipPrint(ix);
				ix = ix->ad_next;
			}
			printf("***** End of Message *****\n\n");

		}
		q_free (&que);
	}
}


static int QPrint(file, que,sender)
char            *file;
Q_struct        *que;
ADDR            *sender;
{
	printf("***** Message %s *****\n",file);
	printf("From %s # On host %s # Inbound channel %s\n\n",
	       (sender->ad_type == AD_X400_TYPE) ?
	       sender->ad_r400adr : sender->ad_r822adr,
	       que->inbound && que -> inbound -> li_mta ?
	       que -> inbound -> li_mta : "(none)",
	       que->inbound && que -> inbound -> li_chan ?
	       que -> inbound->li_chan->ch_name : "(none)");
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

static RecipPrint(recip)
ADDR    *recip;
{
	LIST_RCHAN      *ix;
	printf("To %s #",
	       (recip->ad_type == AD_X400_TYPE) ? recip->ad_r400adr : recip->ad_r822adr);
	if (recip->ad_outchan == NULL)
		printf(" NO OUTBOUND CHANNEL # Status ");
	else
		printf(" On outbound channel %s # Status ",
		       recip->ad_outchan->li_chan->ch_name);

	switch(recip->ad_status) {
	    case AD_STAT_PEND:
		printf("pending\n");
		break;
	    case AD_STAT_DRREQUIRED:
		printf("delivery notification required\n");
		break;
	    case AD_STAT_DRWRITTEN:
		printf("delivery notification written\n");
		break;
	    case AD_STAT_DONE:
		printf("done\n");
		return;
	    case AD_STAT_UNKNOWN:
	    default:
		printf("unknown\n");
		break;
	}

	if ((ix = getnthchannel(recip->ad_fmtchan,recip->ad_rcnt)) != NULL) {
		printf("Reformating channels left = ");
		while (ix != NULL) {
			printf(" %s ->  ",ix->li_chan->ch_name);
			ix = ix->li_next;
		}
	printf("\n");
	}
}
