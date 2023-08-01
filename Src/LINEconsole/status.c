/* status.c: status routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/status.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/status.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: status.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */

#include	"console.h"

time_t convert_time(qb)
struct type_UNIV_UTCTime	*qb;
{
	char	*str;
	time_t	temp;
	if (qb == NULL)
		return 0;
	str = qb2str(qb);
	temp = utc2time_t(str2utct(str, strlen(str)));
	free(str);
	return temp;
}

struct procStatus	*create_status(status)
struct type_Qmgr_ProcStatus	*status;
{
	struct procStatus	*temp;

	temp = (struct procStatus *) malloc(sizeof(*temp));

	temp->enabled = status->enabled;
	temp->lastAttempt = convert_time(status->lastAttempt);
	temp->cachedUntil = convert_time(status->cachedUntil);
	temp->lastSuccess = convert_time(status->lastSuccess);
	return temp;
}

void	update_status(old, new)
struct procStatus		*old;
struct type_Qmgr_ProcStatus	*new;
{
	time_t	tmp;
	old->enabled = new->enabled;

	old->lastAttempt = convert_time(new->lastAttempt);
	
	old->cachedUntil = convert_time(new->cachedUntil);

	if ((tmp = convert_time(new->lastSuccess)) != 0)
		old->lastSuccess = tmp;
}

/*  */

extern FILE	*out;
extern int	connected;
extern int	total_volume, total_number_messages, total_number_reports;
extern char	*time_t2RFC();

time_t	boottime = 0;
int	messagesIn = 0, messagesOut = 0, addrIn = 0, addrOut = 0, 
	maxChans = 0, currChans = 0;
double  opsPerSec = 0.0, runnableChans = 0.0, 
	msgsInPerSec = 0.0, msgsOutPerSec = 0.0;
int	compat = FALSE;
extern int	authorised;
char	*Qversion = NULLCP, *Qinformation = NULLCP;
extern  char *actual_host;

print_status()
{
	char	buf[BUFSIZ];
	openpager();
	
	if (connected == FALSE)
		fprintf(out, "Unconnected\n");
	else {
		if (!compat)
			my_invoke(qmgrStatus, (char **) NULL);
		/* connected */
		fprintf(out, "Connected to %s%s\n", (Qinformation) ? Qinformation : actual_host,
			(authorised == TRUE) ? " with full access" : "");

		if (Qversion)
			fprintf(out, "running %s\n",  Qversion);
		if (total_volume != 0 
		    && (total_number_messages != 0
			|| total_number_reports != 0)) {
			num2unit(total_volume, buf);
			fprintf(out, "Total Volume = %s\n", buf);
			
			fprintf(out,"%d msg%s + %d report%s\n", 
				total_number_messages,
				(total_number_messages == 1) ? "" : "s",
				total_number_reports,
				(total_number_reports == 1) ? "" : "s");
		}
		if (!compat) {
#define	NUMBUF	15
			char	adIn[NUMBUF], mgIn[NUMBUF], adOut[NUMBUF], mgOut[NUMBUF], *str;
			str = time_t2RFC(boottime);
			num2unit(addrIn, adIn);
			num2unit(messagesIn, mgIn);
			num2unit(addrOut, adOut);
			num2unit(messagesOut, mgOut);
			fprintf(out, "running since %s\nInbound %s Message%s to %s Recipient%s\nOutbound %s Message%s to %s Recipient%s\n",
				       str, mgIn, (messagesIn != 1) ? "s" : "",
				       adIn, (addrIn != 1) ? "s" : "",
				       mgOut, (messagesOut != 1) ? "s" : "",
				       adOut, (addrOut != 1) ? "s" : "");
			free(str);
		} else 
			fprintf(out, "Running in 5.0 compatability mode\n");
	}
	closepager();
}

/* ARGSUSED */
int qmgrStatus_result (ad, id, dummy, result, roi)
int	ad,
	id,
	dummy;
struct type_Qmgr_QmgrStatus	*result;
struct RoSAPindication	*roi;
{
	if (result) {
		if (result->boottime) 
			boottime = convert_time(result->boottime);
		messagesIn = result->messagesIn;
		messagesOut = result->messagesOut;
		addrIn = result->addrIn;
		addrOut = result->addrOut;
		opsPerSec = (double) result->opsPerSec;
		runnableChans = (double) result->runnableChans;
		msgsInPerSec = (double) result->msgsInPerSec;
		msgsOutPerSec = (double) result->msgsOutPerSec;
		maxChans = result->maxChans;
		currChans = result->currChans;
		total_volume = result->totalVolume;
		total_number_messages = result->totalMsgs;
		total_number_reports = result->totalDrs;
	}
	return OK;
}
