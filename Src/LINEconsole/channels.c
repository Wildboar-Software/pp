/* channels.c: channel routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/channels.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/channels.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: channels.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */

#include	"Qmgr-types.h"
#include	"console.h"

struct chan_struct	**globallist = NULL, **chan_matches = NULL, 
			*currentchan = NULL;
int			num_channels = 0, chan_num_matches = 0;
char			*chan_match = NULL;

struct chan_struct	*find_channel();

extern char	*time_t2RFC(), *vol2str(), *cmd_argv[],
	*time_t2str(), *mystrtotime(), *lowerfy();
extern int	cmd_argc;
extern Level	lev;
extern time_t	boottime;
static void create_channel_list(), update_channel_list();
static struct chan_struct *create_channel();
static void display_channel_list();
static void free_channel_list();

extern int total_number_reports, total_number_messages, total_volume, compat;
extern int connected;
extern struct procStatus	*create_status();
extern int info_shown;

int channel_all = FALSE;

#define	CHAN_READ_INTERVAL  60

/* ARGSUSED */
int do_channelread (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_UNIV_UTCTime	**arg;
{
	char	*str;
	UTC	utc;
	      
	utc = (UTC) malloc (sizeof(struct UTCtime));
	utc->ut_flags = UT_SEC;

	utc->ut_sec = CHAN_READ_INTERVAL;

	str = utct2str(utc);
	*arg = str2qb (str, strlen(str), 1);
	return OK;
}

/* ARGSUSED */
int channelread_result (ad, id, dummy, result, roi)
int							ad,
							id,
							dummy;
register struct type_Qmgr_ChannelReadResult		*result;
struct RoSAPindication					*roi;
{
	if (compat) {
		total_number_reports = 0;
		total_number_messages = 0;
		total_volume = 0;
	}
	if (globallist) 
		update_channel_list(result->channels);
	else
		create_channel_list(result->channels);
	order_channels(globallist, num_channels);
	return OK;
}

channel_list()
{
	if (connected == TRUE) {
		if (cmd_argc > 1 && lexequ(cmd_argv[1], "all") == 0)
			channel_all = TRUE;
		else
			channel_all = FALSE;
		my_invoke(chanread, (char **) NULL);
		if (globallist)
			display_channel_list(globallist, num_channels);
	}

}

channel_do_refresh ()
{
	if (connected == TRUE) {
		my_invoke(chanread, (char **) NULL);
		if (lev == channel) {
			if (chan_num_matches > 0 && info_shown == TRUE)
				channel_info_regex();
			else
				display_channel_list(globallist, num_channels);
		}
	}
}

/*  */

extern FILE	*out;

static void display_channel_info(chan)
struct chan_struct	*chan;
{
	char		*str,
			*info_str;
	int		info_strlen = BUFSIZ;
	info_str = calloc(1, (unsigned) info_strlen);

	sprintf(info_str, ssformat, tab, 
		chan->channelname, 
		chan->channeldescrip);
	if (chan->status->cachedUntil != 0)  {
		str = time_t2RFC(chan->status->cachedUntil);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"delayed until",
			str);
		free(str);
	}
	if (chan->oldestMessage != 0 && 
	    (chan->numberMessages != 0 || chan->numberReports != 0)) {
		str = time_t2str(time((time_t *)0) - chan->oldestMessage);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"oldest message",
			str);
		free(str);
	}
	if (chan->numberMessages != 0)
		sprintf(info_str, plus_sdformat, info_str, tab,
			"number of messages",
			chan->numberMessages);
	if (chan->numberReports != 0)
		sprintf(info_str, plus_sdformat, info_str, tab,
			"number of DRs",
			chan->numberReports);

	if (chan->volumeMessages != 0) {
		str = vol2str(chan->volumeMessages);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"volume of messages",
			str);
		free(str);
	}
	if (chan->numberActiveProcesses != 0)
		sprintf(info_str, plus_sdformat, info_str, tab,
			"number of active processes",
			chan->numberActiveProcesses);
	sprintf(info_str, plus_ssformat, info_str, tab,
		"status",
		(chan->status->enabled == TRUE) ? "enabled" : "disabled");
	if (chan->status->lastAttempt != 0) {
		str = time_t2RFC(chan->status->lastAttempt);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"last attempt",
			str);
		free(str);
	}
	if (chan->status->lastSuccess != 0) {
		str = time_t2RFC(chan->status->lastSuccess);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"last success",
			str);
		free(str);
	}

	switch (chan->priority) {
	    case int_Qmgr_Priority_low:
		sprintf(info_str, plus_ssformat, info_str, tab,
			"priority",
			"low");
		break;
	    case int_Qmgr_Priority_normal:
		sprintf(info_str, plus_ssformat, info_str, tab,
			"priority",
			"normal");
		break;
	    case int_Qmgr_Priority_high:
		sprintf(info_str, plus_ssformat, info_str, tab,
			"priority",
			"high");
		break;
	    default:
		break;
	}
	if (chan->maxprocs == 0)
		sprintf(info_str, plus_ssformat, info_str, tab,
			"maximum processes",
			"unlimited");
	else
		sprintf(info_str, plus_sdformat, info_str, tab,
			"maximum processes",
			chan->maxprocs);

	fprintf(out, "%s\n", info_str);
	free(info_str);
}
	
static void display_channel(chan)
struct chan_struct	*chan;
{
	/* one line per channel */
	char	str[BUFSIZ];

	sprintf(str,"%s",chan->channelname);
	if (chan -> numberActiveProcesses >= 1)
		sprintf(str, "%s (ACTIVE %d process%s)",
			str, chan->numberActiveProcesses,
			(chan->numberActiveProcesses > 1) ? "s" : "");
	sprintf(str,"%s: %d",
		str,chan->numberMessages);
	if (chan->numberReports != 0)
	  sprintf(str,"%s + %d", str, chan->numberReports);
	if (chan->given_num_mtas != 0)
		sprintf(str,"%s/%d",str, chan->given_num_mtas);
	if (chan->status->enabled != TRUE)
		sprintf(str, "%s (DISABLED)", str);
	
	fprintf(out, "%s\n", str);
}

channel_info()
{
	if (chan_num_matches > 0) {
		openpager();
		display_channel_info(chan_matches[0]);
		closepager();
	}
}

channel_info_regex()
{
	int	i;
	if (chan_num_matches > 0) {
		openpager();
		if (chan_num_matches > 1)
			fprintf(out, "%s: %d matches\n\n",
				chan_match, chan_num_matches);
		for (i = 0; i < chan_num_matches; i++) {
			display_channel_info(chan_matches[i]);
			fprintf(out, "\n");
		}
		closepager();
	}
}

static void display_channel_list(list, num)
struct chan_struct	**list;
int			num;
{
	int	i;
	int	badnessPast = FALSE;

	openpager();

	for (i = 0; i < num; i++) {
		if (badnessPast == FALSE
		    && chanBadness(list[i]) == 0) {
			if (channel_all == FALSE)
				i = num;
			else if (badnessPast == FALSE) {
				fprintf(out, "==================================== Channels below have zero badness\n");
				badnessPast = TRUE;
				display_channel(list[i]);
			}
		} else
			display_channel(list[i]);
	}
	closepager();
}

/*  */

static void	create_channel_list(list)
struct type_Qmgr_PrioritisedChannelList	*list;
/* calloc and fillin in globallist */
{
	struct type_Qmgr_PrioritisedChannelList	*ix = list;
	int	i,
		num = 0;

	while (ix != NULL) {
		num++;

		ix = ix->next;
	}

	num_channels = num;
	globallist = (struct chan_struct **) calloc((unsigned) num, 
						     sizeof(struct chan_struct *));
	chan_matches = (struct chan_struct **) calloc((unsigned) num, 
						     sizeof(struct chan_struct *));
	ix = list;
	i = 0;
	while ((ix != NULL) 
	       && (i < num)) {
		globallist[i] = create_channel(ix);
		i++;
		ix = ix->next;
	}
	
}

static void	update_channel(old, new)
struct chan_struct			*old;
struct type_Qmgr_PrioritisedChannelList	*new;
{
	struct type_Qmgr_ChannelInfo	*info;

	info = new->PrioritisedChannel->channel;
	/* name and description don't change */
	old->oldestMessage = convert_time(info->oldestMessage);
	old->numberMessages = info->numberMessages;
	old->numberReports = info->numberReports;
	old->volumeMessages = info->volumeMessages;
	old->given_num_mtas = info->numberMtas;
	if (compat) {
		total_volume += old->volumeMessages;
		total_number_reports += old->numberReports;
		total_number_messages += old->numberMessages;
	}
	old->numberActiveProcesses = info->numberActiveProcesses;
	update_status(old->status,info->status);
	old->priority = new->PrioritisedChannel->priority->parm;
	old->maxprocs = info->maxprocs;
}

static void	update_channel_list(new)
struct type_Qmgr_PrioritisedChannelList	*new;
{
	struct type_Qmgr_PrioritisedChannelList *ix = new;
	char					*name = NULL;
	struct chan_struct			*chan;

	while (ix != NULL) {
		if (name != NULL) free(name);
		name = qb2str(ix->PrioritisedChannel->channel->channel);

		chan = find_channel(name);
		if (chan == NULL) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("console: can not find channel %s",name));
			abort();
		}
		update_channel(chan,ix);
		ix = ix->next;
	}
}

static struct chan_struct	*create_channel(chan)
struct type_Qmgr_PrioritisedChannelList *chan;
{
	struct chan_struct		*temp;
	struct type_Qmgr_ChannelInfo	*info;

	temp = (struct chan_struct *) calloc(1, sizeof(*temp));
	info = chan->PrioritisedChannel->channel;
	
	temp->channelname = qb2str(info->channel);
	temp->channeldescrip = qb2str(info->channelDescription);

	temp->oldestMessage = convert_time(info->oldestMessage); 
	temp->numberMessages = info->numberMessages;
	temp->numberReports = info->numberReports;
	temp->volumeMessages = info->volumeMessages;
	temp->given_num_mtas = info->numberMtas;
	if (compat) {
		total_volume += temp->volumeMessages;
		total_number_reports += temp->numberReports;
		total_number_messages += temp->numberMessages;
	}
	temp->numberActiveProcesses = info->numberActiveProcesses;
	temp->status = create_status(info->status);
	if (temp->status->lastSuccess == 0)
		temp->status->lastSuccess = boottime;
	temp->priority = chan->PrioritisedChannel->priority->parm;
	temp->inbound = bit_test(info->direction, bit_Qmgr_direction_inbound);
	temp->outbound = bit_test(info->direction, bit_Qmgr_direction_outbound);
	temp->chantype = info->chantype;
	temp->maxprocs = info->maxprocs;
	add_tailor_to_chan(temp);
	return temp;
}

clear_channel_level()
{
	if (globallist) 
		free_channel_list(globallist, num_channels);
	globallist = NULL;
	if (chan_matches) free(chan_matches);
	chan_matches = NULL;
	num_channels = 0;
}

static void free_channel_list(list, num)
struct chan_struct	**list;
int			num;
{
	int	i =0;
	if (list == NULL)
		return;
	while (i < num) {
		free(list[i]->channelname);
		free(list[i]->channeldescrip);
		free ((char *)list[i]->status);
		i++;
	}
	free((char *) list);
}

/*  */
struct chan_struct *find_channel(name)
char			*name;
{
	int	i = 0;
	while ((i < num_channels) && 
	       (lexequ(globallist[i]->channelname, name) != 0)) 
		i++;

	if (i >= num_channels) 
		return NULL;
	else
		return globallist[i];
}

extern char	*re_comp();
extern int	re_exec();

int	find_channel_regex(name)
char	*name;
{
	int	i = 0;
	char	*diag;

	if ((diag = re_comp(name)) != 0) {
		fprintf(stderr,
			"re_comp error for '%s' [%s]\n", name, diag);
		return 0;
	}
	
	for (i = 0, chan_num_matches = 0; i < num_channels; i++) 
		if (re_exec(lowerfy(globallist[i]->channelname)) == 1) 
			chan_matches[chan_num_matches++] = globallist[i];

	return chan_num_matches;
}


channel_set_current_regex()
{
	int	cont = TRUE, doquit = FALSE, first = TRUE;
	char	buf[BUFSIZ];

	while (cont == TRUE) {
		if (cmd_argc > 1 && first == TRUE) {
			strcpy(buf, cmd_argv[1]);
			first = FALSE;
		} else {
			fprintf(stdout, "match expression (%s) (q=quit) = ", 
				(chan_match == NULLCP) ? "" : chan_match);
			fflush(stdout);

			if (gets(buf) == NULL)
				exit (OK);
		}
		(void) compress(buf, buf);
		
		if (emptyStr(buf) && (chan_match == NULLCP)) 
			fprintf(stdout, "no match set\n");
		else {
			if (!emptyStr(buf)) {
				if (lexequ(buf, "q") != 0) {
					if (chan_match) free(chan_match);
					chan_match = strdup(buf);
				} else
					doquit = TRUE;
			}
			cont = FALSE;
		}
	}
	if (doquit == TRUE)
		return NOTOK;
	if (find_channel_regex(chan_match) == 0) {
		fprintf(stdout, "unable to find match for '%s'\n",
			chan_match);
		return NOTOK;
	}
	return OK;
}
		
channel_set_current()
{
	int	cont = TRUE, i, doquit = FALSE, first = TRUE;
	char	buf[BUFSIZ];
	
	while (cont == TRUE) {
		if (cmd_argc > 1 && first == TRUE) {
			strcpy(buf, cmd_argv[1]);
			first = FALSE;
		} else {
			fprintf(stdout, "match expression (%s) (q=quit) = ", 
				(chan_match == NULLCP) ? "" : chan_match);
			fflush(stdout);


			if (gets(buf) == NULL)
				exit (OK);
		}
		compress(buf, buf);
		if (emptyStr(buf) && (chan_match == NULLCP)) 
				fprintf(stdout, "no match set\n");
		else if (!emptyStr(buf) && lexequ(buf, "q") == 0) {
			doquit = TRUE;
			cont = FALSE;
		} else {
			char	*expr = (emptyStr(buf)) ? chan_match : buf;
			int num = find_channel_regex(expr);
			switch (num) {
			    case 0:
				fprintf(stdout, "unable to find match for '%s'\n", expr);
				break;
			    case 1:
				cont = FALSE;
				if (expr != chan_match) {
					if (chan_match) free(chan_match);
					chan_match = strdup(expr);
				}
				currentchan = chan_matches[0];
				break;
			    default:
				fprintf(stdout, "'%s' matches %d possible channels (", 
					expr, num);
				for (i = 0; i < num; i++)
					fprintf(stdout, "%s%s",
						chan_matches[i]->channelname,
						(i == num - 1) ? "" : ",");
				fprintf(stdout, ")\n\tneed unique channel\n");
				break;
			}
		}
	}
	if (doquit == TRUE)
		return NOTOK;
	return OK;
}

int is_loc_chan(chan)
struct chan_struct	*chan;
{
	if (chan->chantype == int_Qmgr_chantype_mts
	    && chan->outbound > 0)
		return	TRUE;
	else
		return FALSE;
}

/*  */

channel_next()
{
	if (globallist) {
		int	i = 0;
		if (chan_num_matches > 0) {
			while ((i < num_channels) &&
			       chan_matches[0] != globallist[i])
				i++;
			if (i >= num_channels - 1)
				i = 0;
			else
				i++;
		} 
		if (chan_match) free(chan_match);
		chan_match = strdup(globallist[i]->channelname);
		find_channel_regex(chan_match);
	}
}

channel_previous()
{
	if (globallist) {
		int	i = num_channels -1;
		if (chan_num_matches > 0) {
			while ((i >= 0) &&
			       chan_matches[0] != globallist[i])
				i--;
			if (i <= 0)
				i = num_channels - 1;
			else
				i--;
		} 
		if (chan_match) free(chan_match);
		chan_match = strdup(globallist[i]->channelname);
		find_channel_regex(chan_match);
	}
}
		
/*  */

channel_control(op, chan, mytime)
Operations	op;
char		*chan, *mytime;
{
	char	*args[3];
	args[0] = chan;
	args[1] = (char *) op;
	args[2] = mytime;
	my_invoke(op, args);
}

channel_enable_regex()
{
	int	cont, i = 0;
	char	buf[BUFSIZ];

	if (chan_num_matches > 0) {

		if (chan_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				chan_match, chan_num_matches);

		for (i = 0; i < chan_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (chan_num_matches == 1) {
					fprintf(stdout, "enabling '%s'.\n",
						chan_matches[i]->channelname);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "enable '%s' (y/n/q) ?",
						chan_matches[i]->channelname);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch (buf[0]) {
				    case 'y':
				    case 'Y':
					channel_control(chanstart, 
							chan_matches[i]->channelname, 
							NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = chan_num_matches;
				    default:
					break;
				}
			}
		}

	}
}		 

channel_disable_regex()
{
	int	cont, i;
	char	buf[BUFSIZ];

	if (chan_num_matches > 0) {

		if (chan_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				chan_match, chan_num_matches);

		for (i = 0; i < chan_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (chan_num_matches == 1)  {
					fprintf(stdout, "disabling '%s'.\n",
						chan_matches[i]->channelname);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "disable '%s' (y/n/q) ?",
						chan_matches[i]->channelname);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch (buf[0]) {
				    case 'y':
				    case 'Y':
					channel_control(chanstop, 
							chan_matches[i]->channelname, 
							NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = chan_num_matches;
				    default:
					break;
				}
			}
		}

	}
}		 

channel_clear_regex()
{
	int	cont, i;
	char	buf[BUFSIZ];

	if (chan_num_matches > 0) {

		if (chan_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				chan_match, chan_num_matches);

		for (i = 0; i < chan_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (chan_num_matches == 1) {
					fprintf(stdout, "clearing '%s'.\n",
						chan_matches[i]->channelname);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "clear '%s' (y/n/q) ?",
						chan_matches[i]->channelname);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch (buf[0]) {
				    case 'y':
				    case 'Y':
					channel_control(chanclear, 
							chan_matches[i]->channelname, 
							NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = chan_num_matches;
				    default:
					break;
				}
			}
		}

	}
}		 

channel_force(chan)
struct chan_struct	*chan;
{
	
	if (chan->status->enabled != TRUE)
		channel_control(chanstart, chan->channelname, NULLCP);
	if (chan->status->cachedUntil != 0)
		channel_control(chanclear, chan->channelname, NULLCP);
}

channel_below_clear_regex()
{
	int	cont, i;
	char	buf[BUFSIZ];
	
	if (chan_num_matches > 0) {
		if (chan_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				chan_match, chan_num_matches);
		for (i = 0; i < chan_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (chan_num_matches == 1) {
					fprintf(stdout, "clearing entities below '%s'.\n",
						chan_matches[i]->channelname);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "clear entities below '%s' (y/n/q) ?",
						chan_matches[i]->channelname);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch (buf[0]) {
				    case 'y':
				    case 'Y':
					currentchan = chan_matches[i];
					my_invoke(mtaread, 
						  &(chan_matches[i]->channelname));
					mta_downforce_list(chan_matches[i]);
					channel_force(chan_matches[i]);
					currentchan = NULL;
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = chan_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

channel_delay_regex()
{
	int	cont, i, dcont;
	char	buf[BUFSIZ];
	char	delay[BUFSIZ];

	delay[0] = '\0';

	if (chan_num_matches > 0) {

		if (chan_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				chan_match, chan_num_matches);

		for (i = 0; i < chan_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (chan_num_matches == 1) {
					fprintf(stdout, "delaying '%s'.\n",
						chan_matches[i]->channelname);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "delay '%s' (y/n/q) ?",
						chan_matches[i]->channelname);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch (buf[0]) {
				    case 'y':
				    case 'Y':
					dcont = TRUE;
					while (dcont == TRUE) {
						fprintf(stdout, "Delay (in s m h d and/or w) [%s] = ",
							(delay[0] != '\0') ? delay : "");
						fflush(stdout);
		
						if (gets(buf) == NULL)
							exit(OK);
						compress(buf, buf);
		
						if (emptyStr(buf)) {
							if (delay[0] != '\0')
								/* happy with the one we got */
								dcont = FALSE;
							else
								fprintf(stdout, "no delay given\n");
						} else {
							dcont = FALSE;
							strcpy(delay, buf);
						}
					}
					
					channel_control(chancacheadd, 
							chan_matches[i]->channelname, 
							delay);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'Q':
				    case 'q':
					cont = FALSE;
					i = chan_num_matches;
				    default:
					break;
				}
			}
		}

	}
}		 

/* ARGSUSED */
int do_channelcontrol (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_Qmgr_ChannelControl	**arg;
/* args[0] = channel */
/* args[1] = stop,start,clear,cacheadd */
/* args[2] = time */
{
	char	*timestr;
	*arg = (struct type_Qmgr_ChannelControl *) malloc(sizeof(**arg));
	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	(*arg)->channel = str2qb(args[0],
				 strlen(args[0]),
				 1);
	switch ((Operations) args[1]) {
	    case chanstart:
		(*arg)->control->offset = type_Qmgr_Control_start;
		break;
	    case chanstop:
		(*arg)->control->offset = type_Qmgr_Control_stop;
		break;
	    case chanclear:
		(*arg)->control->offset = type_Qmgr_Control_cacheClear;
		break;
	    case chancacheadd:
		(*arg)->control->offset = type_Qmgr_Control_cacheAdd;
		timestr = mystrtotime(args[2]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
		break;
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console : '%s' unknown control param for channels",
			args[1]));
		return NOTOK;
	}
	return OK;
}

/* ARGSUSED */
int channelcontrol_result (ad, id, dummy, result, roi)
int							ad,
							id,
							dummy;
register struct type_Qmgr_PrioritisedChannelList	*result;
struct RoSAPindication					*roi;
{
	if (compat) {
		total_number_reports = 0;
		total_number_messages = 0;
		total_volume = 0;
	}
	if (globallist) 
		update_channel_list(result);
	else
		create_channel_list(result);
	return OK;

}
