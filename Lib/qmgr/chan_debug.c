/* chan_debug.c: stub version of chan_control for debugging channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/chan_debug.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/chan_debug.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: chan_debug.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "qmgr.h"
static void pdeliverystatus (), pindividualdeliverystatus ();


/* ARGSUSED */
int debug_channel_control (argc, argv, init, work, finish)
int     argc;
char    **argv;
IFP     init;
struct type_Qmgr_DeliveryStatus *(*work)();
IFP	finish;
{
	char	channel[BUFSIZ],
		buf[BUFSIZ];
	int     cont = TRUE,
		num;
	struct type_Qmgr_ProcMsg arg;
	struct type_Qmgr_UserList *temp = NULL,
				  *tail = NULL;
	struct type_Qmgr_DeliveryStatus *result;

	if (isatty (2))
		pp_log_norm -> ll_stat |= LLOGTTY;
	/* input channel structure */
	printf("Input channel name : ");
	if (scanf(" %s",channel) != 1)
		return NOTOK;
	arg.channel = str2qb(channel,strlen(channel),1);
	arg.qid = NULL;
	arg.users = NULL;

	if ((*init)(arg.channel) == OK) {
		printf("Channel initialised\n");
		while(cont == TRUE) {
			if (arg.users != NULL) free_Qmgr_UserList(arg.users);
			arg.users = NULL;
			if (arg.qid != NULL) qb_free(arg.qid);
			arg.qid = NULL;

			temp = tail = NULL;

			printf("Input message id : ");
			if (scanf(" %s",buf) != 1)
				return NOTOK;

			arg.qid = str2qb(buf,strlen(buf),1);

			printf("Input recipient numbers to pass to channel terminated by -1\n\t : ");
			if (scanf(" %d",&num) != 1)
				return NOTOK;

			while (num != -1) {
				temp = (struct type_Qmgr_UserList *) calloc(1, sizeof(*temp));
				temp->RecipientId = (struct type_Qmgr_RecipientId *) calloc(1,sizeof(*(temp->RecipientId)));
				temp->RecipientId->parm = num;
				if (arg.users == NULL) 
					arg.users = tail = temp;
				else {
					tail->next = temp;
					temp = temp;
				}
				if (scanf("%d",&num) != 1)
					return NOTOK;
			}

			result = (*work) (&arg);
			pdeliverystatus(result);

			printf("Do you want to continue ");
			do {
				printf("(y/n) ? ");
				if (scanf("%s", buf) != 1)
					return NOTOK;
			} while (index("yn",*buf) == NULL);
			cont = (*buf == 'y') ? TRUE : FALSE;
		}
		if (finish != NULLIFP)
			(*finish) ();
	} else 
		printf("Channel initialisation failed\n");
	return OK;
}

static void pdeliverystatus (status)
struct type_Qmgr_DeliveryStatus *status;
{
	printf ("Delivery status\n");
	if (status == NULL)
		printf ("Complete failure\n");
	else {
		struct type_Qmgr_DeliveryStatus *ix = status;
		while (ix != NULL)
		{
			pindividualdeliverystatus(ix->IndividualDeliveryStatus);
			ix = ix->next;
		}
	}
}

static void pindividualdeliverystatus(status)
struct type_Qmgr_IndividualDeliveryStatus *status;
{
	printf ("Recipient %d: ", status->recipient->parm);

	switch (status->status) {
	    case int_Qmgr_status_success:
		printf ("success");
		break;
	    case int_Qmgr_status_successSharedDR:
		printf ("successSharedDR");
		break;
	    case int_Qmgr_status_failureSharedDR:
		printf ("failureSharedDR");
		break;
	    case int_Qmgr_status_negativeDR:
		printf ("negativeDR");
		break;
	    case int_Qmgr_status_positiveDR:
		printf ("positiveDR");
		break;
	    case int_Qmgr_status_messageFailure:
		printf ("message failure");
		break;
	    case int_Qmgr_status_mtaFailure:
		printf ("mta failure");
		break;
	    case int_Qmgr_status_mtaAndMessageFailure:
		printf ("mta and message failure");
		break;
	    default:
		printf ("unknown");
		break;
	}
	(void) putchar ('\n');
}
