/* mtas.c: mta routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/mtas.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/mtas.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: mtas.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */

#include	"console.h"

char	*mta_match = NULLCP;
int	mta_num_matches = 0;
struct mta_struct	**mta_matches = NULL;

extern int	connected;
extern struct procStatus	*create_status();
extern struct chan_struct	*currentchan, *find_channel();
extern char	*time_t2RFC(), *vol2str(), *time_t2str(), *cmd_argv[],
		*itoa(), *mystrtotime();
extern int total_number_reports, total_number_messages, total_volume, compat, 
	cmd_argc;

#define 	MTA_READ_INTERVAL	60	/* in secs */

extern struct msg_struct	**global_msg_list;
extern int		number_msgs;

static void display_mta_list();
static void update_mta_list();
static struct mta_struct **create_mta_list();
static struct mta_struct *create_mta();

extern Level lev;
extern int info_shown;

struct mta_struct	*currentmta = NULL, *find_mta();

/* ARGSUSED */
int do_mtaread (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args; /* args[0] is channel name */
struct type_Qmgr_MtaRead	**arg;
{
	char	*str;
	UTC	utc;

	*arg = (struct type_Qmgr_MtaRead *) malloc(sizeof(**arg));
	utc = (UTC) malloc (sizeof(struct UTCtime));

	utc->ut_flags = UT_SEC;
	utc->ut_sec = MTA_READ_INTERVAL;
	str = utct2str(utc);
	(*arg)->time = str2qb(str, strlen(str), 1);

	(*arg)->channel = str2qb(args[0],
				 strlen(args[0]),
				 1);
}


/* ARGSUSED */
int mtaread_result (ad, id, dummy, result, roi)
int						ad,
						id,
						dummy;
register struct type_Qmgr_PrioritisedMtaList	*result;
struct RoSAPindication				*roi;
{

	struct chan_struct	*chan = currentchan;
	
	
	free_mta_list(&(chan->mtalist), &(chan->num_mtas));
	chan->mtalist = NULL;
	chan->num_mtas = 0;
	if (result != NULL)
		update_mta_list(result, chan);
	else
		update_channel_from_mtas(chan, 0, 0, 0, 0);
	order_mtas(&(chan->mtalist),chan->num_mtas);
	return OK;
}

mta_list()
{
	if (connected == TRUE && currentchan != NULL) {
		my_invoke(mtaread, &(currentchan->channelname));
		display_mta_list(currentchan);
	}
}

mta_do_refresh ()
{
	if (connected == TRUE) {
		channel_do_refresh();
		if (currentchan != NULL) {
			my_invoke(mtaread, &(currentchan->channelname));
			if (mta_match) 
				find_mta_regex(currentchan, mta_match);
			reset_prompt_regex();

			if (lev == mta) {
				if (mta_num_matches > 0 && info_shown == TRUE)
					mta_info_regex();
				else
					display_mta_list(currentchan);
			}
		}
	}
}
					
/*  */	

clear_mta_level()
{
	if (currentchan) {
		free_mta_list(&(currentchan->mtalist), &(currentchan->num_mtas));
		currentchan->mtalist = NULL;
		currentchan->num_mtas = 0;
	}
	currentmta = NULL;
}

free_mta_list(plist, pnum)
struct mta_struct	***plist;
int			*pnum;
{
	int	i = 0;

	while(i < (*pnum)){
		if ((*plist) [i] != NULL) {
			free((*plist)[i]->mta);
			if ((*plist)[i]->info != NULLCP)
				free((*plist)[i] -> info);
			free((char *) (*plist)[i]->status);
		}
		i++;
	}
	if (*plist != NULL)
		free((char *) (*plist));
	*plist = NULL;
	*pnum = 0;
}

static void update_mta_list(new, chan)
struct type_Qmgr_PrioritisedMtaList	*new;
struct chan_struct			*chan;
{
	int					numberMsgs = 0,
						numberDrs = 0,
						numberMtas = 0,
						volume = 0;
	chan->mtalist = create_mta_list(new,&numberMtas,
					&numberDrs,&numberMsgs,&volume,
					chan);
	update_channel_from_mtas(chan, numberMsgs, numberDrs, numberMtas, volume);
}

static struct mta_struct	**create_mta_list(list, pnum, pdr, pmsgnum, pvolume, chan)
struct type_Qmgr_PrioritisedMtaList	*list;
int					*pnum,
  					*pdr,
					*pmsgnum,
					*pvolume;
struct chan_struct			*chan;
{
	struct type_Qmgr_PrioritisedMtaList 	*ix = list;
	struct mta_struct			**mtalist;
	int					i;
	while (ix != NULL) {
		(*pnum)++;

		ix = ix->next;
	}
	
	mtalist = (struct mta_struct	**) calloc( (unsigned)(*pnum), 
						   sizeof(struct mta_struct *));

	ix = list;
	i = 0;
	*pmsgnum = 0;
	*pdr = 0;

	while ((ix != NULL)
	       && ( i < (*pnum))) {
		mtalist[i] = create_mta(ix->PrioritisedMta, chan);
		*pmsgnum += mtalist[i]->numberMessages;
		*pdr += mtalist[i]->numberReports;
		*pvolume += mtalist[i]->volumeMessages;
		i++;
		ix = ix->next;
	}
	return mtalist;
}

static struct mta_struct	*create_mta(themta, chan)
struct type_Qmgr_PrioritisedMta	*themta;
struct chan_struct		*chan;
{
	struct mta_struct		*temp;
	struct type_Qmgr_MtaInfo	*info = themta->mta;

	temp = (struct mta_struct *) malloc(sizeof(*temp));

	temp->mta = qb2str(info->mta);
	temp->oldestMessage = convert_time(info->oldestMessage);
	temp->numberMessages = info->numberMessage;
	temp->numberReports = info->numberDRs;
	temp->volumeMessages = info->volumeMessages;
	temp->status = create_status(info->status);
	temp->priority = themta->priority->parm;
	temp->active = info->active;
	if (info -> info != NULL)
		temp -> info = qb2str(info -> info);
	else
		temp -> info = NULLCP;
	add_tailor_to_mta(chan, temp);
	return temp;
}

update_mta(old, info)
struct mta_struct			*old;
struct type_Qmgr_MtaInfo		*info;
{
	/* name doesn't change */
	old->oldestMessage = convert_time(info->oldestMessage);
	old->numberMessages = info->numberMessage;
	old->numberReports = info->numberDRs;
	old->volumeMessages = info->volumeMessages;
	old->active = info->active;
	if (old -> info) 
		free (old -> info);
	if (info -> info != NULL)
		old -> info = qb2str(info -> info);
	else
		old -> info = NULLCP;
	update_status(old->status,info->status);
}

update_channel_from_mtas(chan, nmsgs, ndrs, nmtas, volume)
struct chan_struct	*chan;
int			nmsgs,
  			ndrs,
			nmtas,
			volume;
{
	int	changed = FALSE;
	
	if (chan -> numberMessages != nmsgs) {
		if (compat)
			total_number_messages 
				+= nmsgs - chan -> numberMessages;
		chan -> numberMessages = nmsgs;
		changed = TRUE;
	}
	
	if (chan -> numberReports != ndrs) {
		if (compat)
			total_number_reports += ndrs - chan -> numberReports;
		chan -> numberReports = ndrs;
		changed = TRUE;
	}

	if (chan -> num_mtas != nmtas) {
		chan -> num_mtas = nmtas;
		changed = TRUE;
	}
	if (chan -> volumeMessages != volume) {
		if (compat)
			total_volume += volume - chan -> volumeMessages;
		chan -> volumeMessages = volume;
		changed = TRUE;
	}

	if (chan->given_num_mtas != chan -> num_mtas) {
		chan -> given_num_mtas = chan->num_mtas;
		changed = TRUE;
	}
}


/*  */
extern int uk_order;
extern char	*reverse_mta();
extern FILE	*out;

static void display_mta(mta)
struct mta_struct	*mta;
{
	char	str[BUFSIZ];
	char	*ch;

	ch = (uk_order)? reverse_mta(mta->mta) : mta->mta;
	sprintf(str, "%s %s: %d",ch, 
		(mta -> active) ? "(ACTIVE)" : "",
		mta->numberMessages);
	if (uk_order && ch != mta->mta) free(ch);
	if (mta->numberReports != 0)
	  sprintf(str, "%s + %d", str, mta->numberReports);
	if (mta->status->enabled != TRUE)
		sprintf(str, "%s (DISABLED)", str);
	fprintf(out, "%s\n", str);
}

display_mta_info(chan,mta)
struct chan_struct	*chan;
struct mta_struct	*mta;
{
	char		*str,
			*info_str,
			*ch;
	int		info_strlen = BUFSIZ;
	char		tmpbuf[BUFSIZ];
	info_str = calloc(1, (unsigned) info_strlen);

	ch = (uk_order) ? reverse_mta(mta->mta) : mta->mta;
	
	sprintf(tmpbuf, "on channel '%s'", chan->channelname);
	sprintf(info_str, ssformat, tab,
		ch, tmpbuf);

	if (uk_order && ch != mta->mta)
		free(ch);

	if (mta->status->cachedUntil != 0) {
		str = time_t2RFC(mta->status->cachedUntil);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"delayed until",
			str);
		free(str);
	}
	if (mta->info != NULLCP)
		sprintf (info_str, plus_ssformat, info_str, tab,
			 "error info",
			 mta -> info);
	
	str = time_t2str(time((time_t *) 0) - mta->oldestMessage);
	sprintf(info_str, plus_ssformat, info_str, tab,
		"oldest message",
		str);
	free(str);

	str = itoa(mta->numberMessages);
	sprintf(info_str, plus_ssformat, info_str, tab,
		"number of messages",
		str);
	free(str);

	if (mta->numberReports != 0) {
	  str = itoa(mta->numberReports);
	  sprintf(info_str, plus_ssformat, info_str, tab,
		  "number of drs",
		  str);
	  free(str);
	}

	str = vol2str(mta->volumeMessages);
	sprintf(info_str, plus_ssformat, info_str, tab,
		"volume of messages",
		str);
	free(str);

	if (mta -> active) 
		sprintf(info_str, plus_ssformat, info_str, tab,
			"active",
			"processes running");

	sprintf(info_str, plus_ssformat, info_str, tab,
		"status",
		(mta->status->enabled == TRUE) ? "enabled" : "disabled");
	if (mta->status->lastAttempt != 0) {
		str = time_t2RFC(mta->status->lastAttempt);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"last attempt",
			str);
		free(str);
	}
	if (mta->status->lastSuccess != 0) {
		str = time_t2RFC(mta->status->lastSuccess);
		sprintf(info_str, plus_ssformat, info_str, tab,
			"last success",
			str);
		free(str);
	}

	switch (mta->priority) {
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

	fprintf(out, "%s", info_str);
	free(info_str);
}

mta_info_regex()
{
	int	i;
	if (currentchan && mta_num_matches > 0) {
		openpager();
		if (mta_num_matches > 1)
			fprintf(out, "%s: %d matches\n\n",
				mta_match, mta_num_matches);
		for (i = 0; i < mta_num_matches; i++) {
			display_mta_info(currentchan, mta_matches[i]);
			fprintf(out, "\n");
		}
		closepager();
	}
}

static void display_mta_list(chan)
struct chan_struct	*chan;
{
	int	i;
	
	openpager();

	for (i = 0; i < chan->num_mtas; i++)
		display_mta(chan->mtalist[i]);
	closepager();
}

/*  */

struct mta_struct	*find_mta(chan, name)
struct chan_struct	*chan;
char			*name;
{
	int	i = 0;
	while ((i < chan->num_mtas) &&
	       (strcmp(chan->mtalist[i]->mta, name) != 0))
		i++;
	if (i >= chan->num_mtas)
		return NULL;
	else
		return chan->mtalist[i];
}	

extern char	*re_comp();
extern int	re_exec();

int find_mta_regex(chan, name)
struct chan_struct	*chan;
char			*name;
{
	int	i = 0;
	char	*diag;

	if ((diag = re_comp(name)) != 0) {
		fprintf(stderr,
			"re_comp error for '%s' [%s]\n", name, diag);
		return 0;
	}

	if (mta_matches) {
		free(mta_matches);
		mta_matches = NULL;
	}
	mta_matches = (struct mta_struct **) calloc(chan->num_mtas,
						    sizeof(struct mta_struct *));

	for (i = 0, mta_num_matches = 0; i < chan->num_mtas; i++)
		if (re_exec(lowerfy(chan->mtalist[i]->mta)) == 1)
			mta_matches[mta_num_matches++] = chan->mtalist[i];

	return mta_num_matches;
}	

mta_set_current_regex()
{
	int	cont = TRUE, quit = FALSE, first = TRUE;
	char	buf[BUFSIZ];

	while (cont == TRUE) {
		if (cmd_argc > 1 && first == TRUE) {
			strcpy(buf, cmd_argv[1]);
			first = FALSE;
		} else {
			fprintf(stdout, "match expression (%s) (q=quit) = ", 
				(mta_match == NULLCP) ? "" : mta_match);
			fflush(stdout);

			if (gets(buf) == NULL)
				exit (OK);
		}
		compress(buf, buf);
		
		if (emptyStr(buf) && mta_match == NULLCP)
			fprintf(stdout, "no match set\n");
		else {
			if (!emptyStr(buf)) {
				if (lexequ(buf, "q") != 0) {
					if (mta_match) free(mta_match);
					mta_match = strdup(buf);
				} else
					quit = TRUE;
			}
			cont = FALSE;
		}
	}
	if (quit == TRUE)
		return NOTOK;
	if (find_mta_regex(currentchan, mta_match) == 0) {
		fprintf(stdout, "unable to find match for '%s'\n",
			mta_match);
		return NOTOK;
	}
	return OK;
}

mta_set_current()
{
	int	cont = TRUE, i, quit = FALSE, first = TRUE;
	char	buf[BUFSIZ];

	if (currentchan == NULL)
		channel_set_current();

	while (cont == TRUE) {
		if (cmd_argc > 1 && first == TRUE) {
			strcpy(buf, cmd_argv[1]);
			first = FALSE;
		} else {
			fprintf(stdout, "match expression (%s) (q=quit) = ", 
				(mta_match == NULL) ? "" : mta_match);
			fflush(stdout);

			if (gets(buf) == NULL)
				exit (OK);
		}
		compress(buf, buf);

		if (emptyStr(buf) && mta_match == NULLCP)
			fprintf(stdout, "no match set\n");
		else if (!emptyStr(buf) && lexequ(buf, "q") == 0) {
			quit = TRUE;
			cont = FALSE;
		} else {
			char	*exp = (emptyStr(buf)) ? mta_match : buf;
			int num = find_mta_regex(currentchan, exp);
			switch (num) {
			    case 0:
				fprintf(stdout, "unable to find match for '%s'\n", exp);
				break;
			    case 1:
				cont = FALSE;
				if (exp != mta_match) {
					if (mta_match) free(mta_match);
					mta_match = strdup(exp);
				}
				currentmta = mta_matches[0];
				break;
			    default:
				fprintf(stdout, "'%s' matches %d possible mtas (",
					exp, num);
				for (i = 0; i < num; i++)
					fprintf(stdout, "%s%s",
						mta_matches[i]->mta,
						(i == num -1) ? "" : ",");
				fprintf(stdout, ")\n\tneed unqiue mta\n");
				break;
			}
		}
	}
	if (quit == TRUE)
		return NOTOK;
	return OK;
}


/*  */

mta_next ()
{
	if (currentchan && currentchan -> num_mtas > 0) {
		int	i = 0;
		if (mta_num_matches > 0) {
			while (i < currentchan -> num_mtas &&
			       currentchan->mtalist[i] != mta_matches[0])
				i++;
			if (i >= currentchan -> num_mtas - 1)
				i = 0;
			else
				i++;
		}
		if (mta_match) free(mta_match);
		mta_match = strdup(currentchan->mtalist[i]->mta);
		find_mta_regex(currentchan, mta_match);
	}
}

mta_previous ()
{
	if (currentchan && currentchan -> num_mtas > 0) {
		int	i = currentchan -> num_mtas - 1;
		if (mta_num_matches > 0) {
			while (i >= 0 &&
			       currentchan->mtalist[i] != mta_matches[0])
				i--;
			if (i <= 0)
				i = currentchan -> num_mtas - 1;
			else
				i--;
		}
		if (mta_match) free(mta_match);
		mta_match = strdup(currentchan -> mtalist[i]-> mta);
		find_mta_regex(currentchan, mta_match);
	}
}

/*  */


mta_control(op, chan, mta, mytime)
Operations	op;
char		*chan, *mta, *mytime;
{
	char	*args[4];
	args[0] = chan;
	args[1] = mta;
	args[2] = (char *) op;
	args[3] = mytime;
	my_invoke(op, args);
}

mta_enable_regex()
{
	int	cont, i = 0;
	char	buf[BUFSIZ];
	
	if (currentchan && mta_num_matches > 0) {
		if (mta_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				mta_match, mta_num_matches);
		for (i = 0; i < mta_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (mta_num_matches == 1) {
					fprintf(stdout, "enabling '%s'.\n",
						mta_matches[i]->mta);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "enable '%s' (y/n/q) ?",
						mta_matches[i]->mta);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					mta_control(mtastart,
						    currentchan->channelname,
						    mta_matches[i]->mta,
						    NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = mta_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

mta_disable_regex()
{
	int	cont, i = 0;
	char	buf[BUFSIZ];
	
	if (currentchan && mta_num_matches > 0) {
		if (mta_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				mta_match, mta_num_matches);
		for (i = 0; i < mta_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (mta_num_matches == 1) {
					fprintf(stdout, "disabling '%s'.\n",
						mta_matches[i]->mta);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "disable '%s' (y/n/q) ?",
						mta_matches[i]->mta);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					mta_control(mtastop,
						    currentchan->channelname,
						    mta_matches[i]->mta,
						    NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = mta_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

mta_clear_regex()
{
	int	cont, i = 0;
	char	buf[BUFSIZ];
	
	if (currentchan && mta_num_matches > 0) {
		if (mta_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				mta_match, mta_num_matches);
		for (i = 0; i < mta_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (mta_num_matches == 1) {
					fprintf(stdout, "clearing '%s'.\n",
						mta_matches[i]->mta);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "clear '%s' (y/n/q) ?",
						mta_matches[i]->mta);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					mta_control(mtaclear,
						    currentchan->channelname,
						    mta_matches[i]->mta,
						    NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = mta_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

mta_force (chan, mta)
struct chan_struct	*chan;
struct mta_struct	*mta;
{
	if (mta->status->enabled != TRUE)
		mta_control(mtastart, chan->channelname, 
			    mta->mta, NULLCP);
	if (mta->status->cachedUntil != 0)
		mta_control(mtaclear, chan->channelname,
			    mta->mta, NULLCP);

}

mta_clear_below_regex()
{
	int	cont, i = 0;
	char	buf[BUFSIZ];
	
	if (currentchan && mta_num_matches > 0) {
		if (mta_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				mta_match, mta_num_matches);
		for (i = 0; i < mta_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (mta_num_matches == 1) {
					fprintf(stdout, "clearing entities below '%s'.\n",
						mta_matches[i]->mta);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "clear entities below '%s' (y/n/q) ?",
						mta_matches[i]->mta);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					mta_downforce(currentchan,
						      mta_matches[i]);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = mta_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

mta_clear_above_regex()
{
	int	cont, i = 0, above = FALSE;
	char	buf[BUFSIZ];
	
	if (currentchan && mta_num_matches > 0) {
		if (mta_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				mta_match, mta_num_matches);
		for (i = 0; i < mta_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (mta_num_matches == 1) {
					fprintf(stdout, "clearing entities above '%s'.\n",
						mta_matches[i]->mta);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "clear entities above '%s' (y/n/q) ?",
						mta_matches[i]->mta);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					mta_force(currentchan,
						  mta_matches[i]);
					above = TRUE;
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = mta_num_matches;
				    default:
					break;
				}
			}
		}
		if (above == TRUE)
			channel_force(currentchan);

	}
}

mta_downforce_list (chan)
struct chan_struct	*chan;
{
	int	i;
	for (i = 0; i < chan->num_mtas; i++)
		mta_downforce(chan, chan->mtalist[i]);
}
	
mta_downforce (chan, mta)
struct chan_struct	*chan;
struct mta_struct	*mta;
{
	struct msg_struct	**oldlist;
	int	oldnum;
	char	*args[2];

	args[0] = chan -> channelname;
	args[1] = mta->mta;
	oldlist = global_msg_list;
	oldnum = number_msgs;
	global_msg_list = NULL;
	number_msgs = 0;
	currentmta = mta;
	my_invoke(readchannelmtamessage, args);
	msg_force_list(global_msg_list);
	currentmta = NULL;
	free_msg_list();
	global_msg_list = oldlist;
	number_msgs = oldnum;

	mta_force(chan, mta);
}

mta_delay_regex()
{
	int	dcont, cont, i = 0;
	char	buf[BUFSIZ], delay[BUFSIZ];
	delay[0] = '\0';
	if (currentchan && mta_num_matches > 0) {
		if (mta_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				mta_match, mta_num_matches);
		for (i = 0; i < mta_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (mta_num_matches == 1) {
					fprintf(stdout, "delaying '%s'.\n",
						mta_matches[i]->mta);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "delay '%s' (y/n/q) ?",
						mta_matches[i]->mta);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					dcont = TRUE;
					while (dcont == TRUE) {
						fprintf(stdout, "Delay (in s m h d and/or w) [%s] = ");
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
					mta_control(mtacacheadd, 
						    currentchan->channelname,
						    mta_matches[i]->mta, 
						    delay);

				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = mta_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

/* ARGSUSED */
int do_mtacontrol (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_Qmgr_MtaControl	**arg;
/* args[0] = channel */
/* args[1] = mta */
/* args[2] = stop,start,clear,cacheadd */
/* args[3] = time */
{
	char	*timestr;
	*arg = (struct type_Qmgr_MtaControl *) malloc(sizeof(**arg));
	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	(*arg)->channel = str2qb(args[0],
				 strlen(args[0]),
				 1);
	(*arg)->mta = str2qb(args[1],
			     strlen(args[1]),
			     1);

	switch ((Operations) args[2]) {
	    case mtastart:
		(*arg)->control->offset = type_Qmgr_Control_start;
		break;
	    case mtastop:
		(*arg)->control->offset = type_Qmgr_Control_stop;
		break;
	    case mtaclear:
		(*arg)->control->offset = type_Qmgr_Control_cacheClear;
		break;
	    case mtacacheadd:
		(*arg)->control->offset = type_Qmgr_Control_cacheAdd;
		timestr = mystrtotime(args[3]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
		break;
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console : '%s' unknown control param for mtas",
			args[2]));
		return NOTOK;
	}
	return OK;
}

/* ARGSUSED */
int mtacontrol_result (ad, id, dummy, result, roi)
int						ad,
						id,
						dummy;
register struct type_Qmgr_MtaInfo		*result;
struct RoSAPindication				*roi;
{
	char			*chan_name,
				*name;
	struct chan_struct	*chan;
	struct mta_struct	*mta;

	chan_name = qb2str(result->channel);
	chan = find_channel(chan_name);

	if (chan == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console: mtacontrol_result can't find channel %s",chan_name));
		return NOTOK;
	}

	name = qb2str(result->mta);
	mta = find_mta(chan, name);

	if (mta == NULL) {
#ifdef not_ipm_yet
		if (forceDown == TRUE) {
			free(chan_name);
			free(name);
			return OK;
		}
#endif
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console:can not find mta %s on channel %s", 
			name, chan_name));
		abort();
	} else {
		update_mta(mta,result);
	}
	order_mtas(&(chan->mtalist),chan->num_mtas);
	free(chan_name);
	free(name);
	return OK;
}
