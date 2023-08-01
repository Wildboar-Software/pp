/* msgs.c: msgs routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/msgs.c,v 6.0 1991/12/18 20:26:30 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/msgs.c,v 6.0 1991/12/18 20:26:30 jpo Rel $
 *
 * $Log: msgs.c,v $
 * Revision 6.0  1991/12/18  20:26:30  jpo
 * Release 6.0
 *
 */

#include "console.h"

static struct msg_struct	**create_msg_list();
static void			display_msg_list();

extern struct procStatus	*create_status();
extern char	*time_t2RFC(), *vol2str(), *time_t2str(), *cmd_argv[],
	*mystrtotime(), *itoa();
extern Level lev;
extern int	info_shown, cmd_argc;

struct msg_struct	**msg_matches = NULL;
int    			msg_num_matches = 0;
char			*msg_match = NULLCP;

struct msg_struct	**global_msg_list = NULL,
			*find_msg();
int			number_msgs = 0;

#define 	MSG_READ_INTERVAL	60	/* in secs */

/* ARGSUSED */
int do_readchannelmtamessage (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args; 
/* args[0] = chan args[1] = mta or name in case of local chan */
struct type_Qmgr_MsgRead	**arg;
{
	char	*str;
	UTC	utc;

	*arg = (struct type_Qmgr_MsgRead *) malloc(sizeof(**arg));
	bzero ((char *) *arg, sizeof(**arg));
	/* fillin time */
	utc = (UTC) malloc (sizeof(struct UTCtime));

	utc->ut_flags = UT_SEC;
	utc->ut_sec = MSG_READ_INTERVAL;
	str = utct2str(utc);
	(*arg)->time = str2qb(str, strlen(str), 1);
	if (args[0] != NULLCP)
		(*arg)->channel = str2qb(args[0], strlen(args[0]), 1);
	if (args[1] != NULLCP)
		(*arg)->mta = str2qb(args[1], strlen(args[1]), 1);

}

/* ARGSUSED */
int readchannelmtamessage_result (ad, id, dummy, result, roi)
int						ad,
						id,
						dummy;
register struct type_Qmgr_MsgList		*result;
struct RoSAPindication				*roi;
{
	free_msg_list();
	global_msg_list = create_msg_list(result->msgs);
	order_msgs();
	return OK;
}

extern struct chan_struct	*currentchan;
extern struct mta_struct	*currentmta;
extern int			connected;

msg_list()
{
	if (connected == TRUE
	    && currentchan != NULL
	    && currentmta != NULL) {
		char	*args[2];
		args[0] = currentchan->channelname;
		args[1] = currentmta->mta;
		my_invoke(readchannelmtamessage, args);
		if (global_msg_list)
			display_msg_list(global_msg_list, number_msgs);
	}
}

msg_do_refresh ()
{
	char	*args[2];
	if (!connected)
		return;
	if (currentchan && currentmta) {
		mta_do_refresh();
		args[0] = currentchan->channelname;
		args[1] = currentmta->mta;
		my_invoke(readchannelmtamessage, args);
		if (msg_match)
			find_msg_regex(msg_match);
		reset_prompt_regex();

		if (lev == msg) {
			if (msg_num_matches > 0 && info_shown == TRUE)
				msg_info_regex();
			else if (global_msg_list)
					display_msg_list(global_msg_list, 
							 number_msgs);
		}
	}
}

/*  */
extern int	uk_order;
extern char	*reverse_adr();
extern FILE *out;

static void display_msg(msg)
struct msg_struct	*msg;
{
	char	str[BUFSIZ];
	char	*ch1, *ch2;

	if (msg->reciplist == NULL
	    || msg->reciplist->next == NULL)
		/* delivery report */
		sprintf(str,"dr (%s) to %s",
			msg->msginfo->queueid,
			msg->msginfo->originator);
	else {
		struct recip	*ix = msg->reciplist;
		int		found = 0;

		while (ix != NULL && found == 0) {
			if (ix->actChan != NULL 
			    && currentchan != NULL 
			    && strcmp(ix->actChan, 
				      currentchan->channelname) == 0) {
				/* channel matches */
				if (is_loc_chan(currentchan) == TRUE) { 
					if (ix->recipient && currentmta
					    && strcmp(ix->recipient, currentmta->mta) == 0)
						found = 1;
				} else if (ix->mta != NULL 
					   && currentmta != NULL
					   && strcmp(ix->mta, 
						     currentmta->mta) == 0)
					/* mta matches */
					found = 1;
			}
			if (found == 0)
				ix = ix->next;
		}
		if (ix != NULL && ix->id == 0) {
			/* delivery report ? */
			ch1 = (uk_order) ? reverse_adr(ix -> recipient)
				: ix -> recipient;
			sprintf(str, "dr (%s) to %s",
				msg->msginfo->queueid,
				ch1);
			if (uk_order && ch1 != ix -> recipient) free(ch1);
		} else {
			if (ix == NULL) ix = msg->reciplist->next;

			ch1 = (uk_order) ? reverse_adr(ix -> recipient) : ix -> recipient;
			ch2 = (uk_order) ? reverse_adr(msg->msginfo->originator) : msg->msginfo->originator;

			sprintf(str, "%s to %s from %s",
				msg->msginfo->queueid,
				ch1, ch2);
			if (uk_order) {
				if (ch1 != ix -> recipient) free(ch1);
				if (ch2 != msg->msginfo->originator) free(ch2);
			}
		}
	}

	fprintf(out, "%s\n", str);
}

display_msg_info(msg)
struct msg_struct	*msg;
{
	struct recip	*ix;
	char		*str, *ch,
			*info_str,
			temp[BUFSIZ];
	int		info_strlen = BUFSIZ;
	info_str = calloc(1, BUFSIZ);

	sprintf(temp, "%s", msg->msginfo->queueid);
	resize_info_str(&info_str, &info_strlen, strlen(temp)+1);
	sprintf(info_str, "%s\n", temp);

	ch = (uk_order) ? reverse_adr(msg->msginfo->originator) 
		: msg->msginfo->originator;
	sprintf(temp, ssformat, tab, "originator ", ch);
	if (uk_order && ch != msg->msginfo->originator) free(ch);
	resize_info_str(&info_str, &info_strlen, strlen(temp+1));
	sprintf(info_str, "%s%s\n", info_str, temp);

	if (msg->msginfo->size != 0) {
		char	size[BUFSIZ];
		
		if (msg->msginfo->size > 1000000)
			(void) sprintf(size, "%.2f M",
				       (double) (msg->msginfo->size)/1000000.0);
		else if (msg->msginfo->size > 1000)
			(void) sprintf(size, "%.2f k",
				       (double) (msg->msginfo->size)/1000.0);
		else
			(void) sprintf(size, "%d bytes", 
				       msg -> msginfo -> size);
		sprintf(temp, ssformat, tab,
			"size", size);
		resize_info_str(&info_str, &info_strlen, strlen(temp)+1);
		sprintf(info_str, "%s%s\n", info_str, temp);
	}
	if (msg->msginfo->uaContentId != NULLCP) {
		sprintf(temp, ssformat, tab,
			"ua content id",
			msg->msginfo->uaContentId);
		resize_info_str(&info_str, &info_strlen, strlen(temp)+1);
		sprintf(info_str, "%s%s\n", info_str, temp);
	}

	if (msg->msginfo->inChannel != NULLCP) {
		sprintf(temp, ssformat, tab,
			"inbound channel",
			msg->msginfo->inChannel);
		resize_info_str(&info_str, &info_strlen, strlen(temp)+1);
		sprintf(info_str, "%s%s\n", info_str, temp);
	}
	
	ix = msg->reciplist;
	if (ix != NULL) ix = ix->next;
	while (ix != NULL) {
		sprintf(info_str, "%s\n", info_str);
		ch = (uk_order) ? reverse_adr(ix -> recipient) 
			: ix -> recipient;
		sprintf(temp, ssformat, tab,
			"to",
			ch);
		if (uk_order && ch != ix -> recipient) free(ch);

		sprintf(temp, "%s(id %d)", temp, ix->id);

		if (ix->info != NULLCP)
			sprintf(temp, plus_ssformat, temp, tab,
				"error info",
				ix -> info);

		if (ix->chansOutstanding == NULL)
			sprintf(temp, plus_ssformat, temp, tab,
				"",
				"awaiting DRs");
		else
			sprintf(temp, plus_ssformat, temp, tab,
				"remaining channels",
				ix->chansOutstanding);
		if (ix->status->cachedUntil != 0) {
			str = time_t2RFC(ix->status->cachedUntil);
			sprintf(temp, plus_ssformat, temp, tab,
				"delayed until",
				str);
			free(str);
		}
		sprintf(temp, plus_ssformat, temp, tab,
			"status",
			(ix->status->enabled == TRUE) ? "enabled" : "disabled");
		
		if (ix->status->lastAttempt != 0) {
			str = time_t2RFC(ix->status->lastAttempt);
			sprintf(temp, plus_ssformat, temp, tab,
				"last attempt",
				str);
			free(str);
		}

		if (ix->status->lastSuccess != 0) {
			str = time_t2RFC(ix->status->lastSuccess);
			sprintf(temp, plus_ssformat, temp, tab,
				"last success",
				str);
			free(str);
		}
		resize_info_str(&info_str, &info_strlen, strlen(temp)+1);
		sprintf(info_str, "%s%s\n", info_str, temp);
		ix = ix->next;
	}
	sprintf(info_str, "%s\n", info_str);

	temp[0] = '\0';
	if (msg->msginfo->contenttype != NULLCP) 
		sprintf(temp, plus_ssformat, temp, tab,
			"content type",
			msg->msginfo->contenttype);

	if (msg->msginfo->eit != NULL)
		sprintf(temp, plus_ssformat, temp, tab,
			"eits",
			msg->msginfo->eit);
	str = time_t2str(time((time_t *) 0) - msg->msginfo->age);
	sprintf(temp, plus_ssformat, temp, tab,
		"Age",
		str);
	free(str);
	if (msg->msginfo->expiryTime != 0) {
		str = time_t2RFC(msg->msginfo->expiryTime);
		sprintf(temp, plus_ssformat, temp, tab,
			"Expiry time",
			str);
		free(str);
	}
	if (msg->msginfo->deferredTime != 0) {
		str = time_t2RFC(msg->msginfo->deferredTime);
		sprintf(temp, plus_ssformat, temp, tab,
			"Deferred until",
			str);
		free(str);
	}
	switch (msg->msginfo->priority) {
	    case int_Qmgr_Priority_low:
		sprintf(temp, plus_ssformat, temp, tab,
			"priority",
			"low");
		break;
	    case int_Qmgr_Priority_normal:
		sprintf(temp, plus_ssformat, temp, tab,
			"priority",
			"normal");
		break;
	    case int_Qmgr_Priority_high:
		sprintf(temp, plus_ssformat, temp, tab,
			"priority",
			"high");
		break;
	    default:
		break;
	}
	if (msg->msginfo->errorCount != 0)
		sprintf(temp, plus_sdformat, temp, tab,
			"number of errors",
			msg->msginfo->errorCount);
	resize_info_str(&info_str, &info_strlen, strlen(temp));
	sprintf(info_str, "%s%s", info_str, temp);

	fprintf(out, "%s", info_str);
	free(info_str);
}


msg_info_regex()
{
	int	i;
	if (msg_num_matches > 0) {
		openpager();
		if (msg_num_matches > 1)
			fprintf(out, "%s: %d matches\n\n",
				msg_match, msg_num_matches);
		for (i = 0; i < msg_num_matches; i++) {
			display_msg_info(msg_matches[i]);
			fprintf(out, "\n");
		}
		closepager();
	}
}

static void display_msg_list(list, num)
struct msg_struct	**list;
int			num;
{
	int	i;
	openpager();

	for (i = 0;i < num; i++)
		display_msg(list[i]);
	closepager();
}

msg_force_list(list, num)
struct msg_struct	**list;
int			num;
{
	int	i;
	for (i = 0;i < num; i++)
		msg_force(list[i]);
}

/*  */
static char	*create_eits(eit)
struct type_Qmgr_EncodedInformationTypes	*eit;
{
	struct type_Qmgr_EncodedInformationTypes *ix;
	char	*str,
		buf[BUFSIZ];
	int	first;
	if (eit == NULL)
		return NULL;
	ix =eit;
	first = TRUE;

	while (ix != NULL) {
		str = qb2str(ix->PrintableString);
		if (first == TRUE) {
			sprintf(buf,"%s",str);
			first = FALSE;
		} else 
			sprintf(buf,"%s, %s",buf, str);
		free(str);
		ix = ix->next;
	}
	return strdup(buf);
}

static struct permsginfo	*create_msginfo(info)
struct type_Qmgr_PerMessageInfo	*info;
{
	struct permsginfo	*temp;

	temp = (struct permsginfo *) calloc(1,sizeof(*temp));

	temp->queueid = qb2str(info->queueid);
	temp -> originator = qb2str(info->originator);
	if (info->contenttype != NULL)
		temp->contenttype = qb2str(info->contenttype);
	else
		temp->contenttype = NULL;
	temp->eit = create_eits(info->eit);
	temp->age = convert_time(info->age);
	temp->size = info->size;
	temp->priority = info->priority->parm;
	if (info->expiryTime == NULL)
		temp->expiryTime = 0;
	else
		temp->expiryTime = convert_time(info->expiryTime);
	if (info->deferredTime == NULL)
		temp->deferredTime = 0;
	else 
		temp->deferredTime = convert_time(info->deferredTime);
	if (info->optionals & opt_Qmgr_PerMessageInfo_errorCount)
		temp->errorCount = info->errorCount;
	if (info->inChannel != NULL)
		temp->inChannel = qb2str(info->inChannel);
	else
		temp->inChannel = NULLCP;
	if (info->uaContentId != NULL)
		temp -> uaContentId = qb2str(info->uaContentId);
	else
		temp -> uaContentId = NULLCP;
	return temp;
}

static char			*create_chans(list, done, pfirst)
struct type_Qmgr_ChannelList	*list;
int				done;
char				**pfirst;
{
	int	count;
	char	buf[BUFSIZ],
		*str;

	struct type_Qmgr_ChannelList	*ix;
	count = 0;
	ix = list;
	while (count < done && ix != NULL) {
		ix = ix->next;
		count++;
	}
	if (ix == NULL)
		/* all done waiting for delivery notifcation */
		return NULL;

	str = qb2str(ix->Channel);
	sprintf(buf,"%s",str);
	free(str);
	*pfirst = qb2str(ix->Channel);
	ix = ix->next;

	while (ix != NULL) {
		str = qb2str(ix->Channel);
		sprintf(buf, "%s, %s",buf,str);
		free(str);
		ix = ix->next;
	}
	return strdup(buf);
}
	
static struct recip		*create_recip(recip)
struct type_Qmgr_RecipientInfo	*recip;
{
	struct recip	*temp;

	temp = (struct recip *) calloc(1,sizeof(*temp));

	temp->id = recip->id->parm;

	temp -> recipient = qb2str(recip->user);
	temp->mta = qb2str(recip->mta);
	temp->chansOutstanding = create_chans(recip->channelList,
					      recip->channelsDone,
					      &temp->actChan);
	temp->status = create_status(recip->procStatus);
	if (recip -> info != NULL)
		temp -> info = qb2str (recip -> info);
	return temp;
}
	
	
static struct recip 		*create_reciplist(list)
struct type_Qmgr_RecipientList	*list;
{
	struct recip	*head = NULL,
			*tail = NULL,
			*temp;

	while (list != NULL) {
		if (list->RecipientInfo->id != 0) {
			temp = create_recip(list->RecipientInfo);
			if (head == NULL)
				tail = head = temp;
			else {
				tail->next = temp;
				tail = temp;
			}
		}
		list = list->next;
	}
	return head;
}

static struct msg_struct	*create_msg(msg)
struct type_Qmgr_MsgStruct	*msg;
{
	struct msg_struct	*temp;
	
	temp = (struct msg_struct *) calloc(1, sizeof(*temp));
	temp->msginfo = create_msginfo(msg->messageinfo);
	temp->reciplist = create_reciplist(msg->recipientlist);
	add_tailor_to_msg(currentchan, temp);
	return temp;
}

static struct msg_struct	**create_msg_list(list)
struct type_Qmgr_MsgStructList	*list;
{
	struct type_Qmgr_MsgStructList	*ix = list;
	struct msg_struct		**msglist;
	int				i;
	number_msgs = 0;
	while (ix != NULL) {
		number_msgs++;
		ix = ix->next;
	}

	msglist = (struct msg_struct **) calloc((unsigned) number_msgs, 
						sizeof(struct msg_struct *));
	
	ix = list;
	i = 0;
	while ((ix != NULL)
	       & (i < number_msgs)) {
		msglist[i] = create_msg(ix->MsgStruct);
		i++;
		ix = ix->next;
	}
	return msglist;
}

clear_msg_level()
{
	free_msg_list();
	if (msg_matches) {
		free(msg_matches);
		msg_matches = NULL;
	}
	msg_num_matches = 0;
}

free_msg_list()
{
	int i = 0;
	if (global_msg_list == NULL || number_msgs == 0)
		return;

	while (i < number_msgs) {
		free_permsginfo(global_msg_list[i]->msginfo);
		free_reciplist(global_msg_list[i]->reciplist);
		i++;
	}
	free((char *) global_msg_list);
	global_msg_list = NULL;
	number_msgs = 0;
}

free_permsginfo(info)
struct permsginfo	*info;
{
	free(info->queueid);
	free(info->originator);
	if (info->eit != NULL) free(info->eit);
	if (info->inChannel != NULL) free(info->inChannel);
	if (info->uaContentId != NULLCP) free(info->uaContentId);
	free((char *) info);
}

free_reciplist(list)
struct recip	*list;
{
	struct recip	*ix = list,
			*temp;

	while (ix != NULL) {
		if (ix->recipient) free(ix->recipient);
		if (ix->mta != NULL) free(ix->mta);
		if (ix->actChan != NULL) free(ix->actChan);
		if (ix->chansOutstanding != NULL) free(ix->chansOutstanding);
		if (ix -> info != NULL) free(ix->info);
		free((char *) ix->status);
		temp = ix;
		ix = ix->next;
		free((char *) temp);
	}
}

/*  */

struct recip	*find_recip(msg, id)
struct msg_struct	*msg;
int			id;
{
	struct recip	*ix;
	if (msg == NULL)
		return NULL;
	ix = msg->reciplist;
	
	while (ix != NULL
	       && ix -> id != id)
		ix = ix->next;
	return ix;
}

struct msg_struct *find_msg(qid)
char	*qid;
{
	int	i = 0;

	while ((i < number_msgs) &&
	       (strcmp(global_msg_list[i]->msginfo->queueid,qid) != 0))
		i++;

	if (i >= number_msgs)
		return NULL;
	else
		return global_msg_list[i];
}

extern char	*re_comp();
extern int	re_exec();

int	find_msg_regex(name)
char	*name;
{
	int	i = 0;
	char	*diag;

	if ((diag = re_comp(name)) != 0) {
		fprintf(stderr,
			"re_comp error for '%s' [%s]\n", name, diag);
		return 0;
	}
	
	if (msg_matches) {
		free(msg_matches);
		msg_matches = NULL;
	}

	msg_matches = (struct msg_struct **) calloc(number_msgs,
						   sizeof(struct msg_struct *));
 
	for (i = 0, msg_num_matches = 0; i < number_msgs; i++) 
		if (re_exec(lowerfy(global_msg_list[i]->msginfo->queueid)) == 1) 
			msg_matches[msg_num_matches++] = global_msg_list[i];

	return msg_num_matches;
}

msg_set_current_regex()
{
	int	cont = TRUE, quit = FALSE, first = TRUE;
	char	buf[BUFSIZ];

	while (cont == TRUE) {
		if (cmd_argc > 1 && first == TRUE) {
			strcpy(buf, cmd_argv[1]);
			first = FALSE;
		} else {
			fprintf(stdout, "match expression (%s) (q=quit) = ", 
				(msg_match == NULLCP) ? "" : msg_match);
			fflush(stdout);

			if (gets(buf) == NULL)
				exit (OK);
		}
		compress(buf, buf);
		
		if (emptyStr(buf) && msg_match == NULLCP)
			fprintf(stdout, "no match set\n");
		else {
			if (!emptyStr(buf)) {
				if (lexequ(buf, "q") != 0) {
					if (msg_match) free(msg_match);
					msg_match = strdup(buf);
				} else
					quit = TRUE;
			}
			cont = FALSE;
		}
	}
	if (quit == TRUE)
		return NOTOK;
	if (find_msg_regex(msg_match) == 0) {
		fprintf(stdout, "unable to find match for '%s'\n",
			msg_match);
		return NOTOK;
	}
	return OK;
}

msg_set_current()
{
	int	cont = TRUE, i, quit = FALSE, first = TRUE;
	char	buf[BUFSIZ];

	while (cont == TRUE) {
		if (cmd_argc > 1 && first == TRUE) {
			strcpy(buf, cmd_argv[1]);
			first = FALSE;
		} else {
			fprintf(stdout, "match expression (%s) (q=quit) = ", 
				(msg_match == NULL) ? "" : msg_match);
			fflush(stdout);

			if (gets(buf) == NULL)
				exit (OK);
		}
		compress(buf, buf);
		
		if (emptyStr(buf) && msg_match == NULLCP)
			fprintf(stdout, "no match set\n");
		else if (!emptyStr(buf) && lexequ(buf, "q") == 0) {
			quit = TRUE;
			cont = FALSE;
		} else {
			char	*exp = (emptyStr(buf)) ? msg_match : buf;
			int num = find_msg_regex(exp);
			switch (num) {
			    case 0:
				fprintf(stdout, "unable to find match for '%s'\n", exp);
				break;
			    case 1:
				cont = FALSE;
				if (exp != msg_match) {
					if (msg_match) free(msg_match);
					msg_match = strdup(exp);
				}
				break;
			    default:
				fprintf(stdout, "'%s' matches %d possible msgs (",
					exp, num);
				for (i = 0; i < num; i++)
					fprintf(stdout, "%s%s",
						msg_matches[i]->msginfo->queueid,
						(i == num -1) ? "" : ",");
				fprintf(stdout, ")\n\tneed unqiue msg\n");
				break;
			}
		}
	}
	if (quit == TRUE)
		return NOTOK;
	return OK;
}

/*  */

msg_next()
{
	if (global_msg_list) {
		int i = 0;
		
		if (msg_num_matches > 0) {
			while (i < number_msgs &&
			       msg_matches[0] != global_msg_list[i])
				i++;
			if (i >= number_msgs - 1)
				i = 0;
			else
				i++;
		}
		if (msg_match) free(msg_match);
		msg_match = strdup(global_msg_list[i]->msginfo->queueid);
		find_msg_regex(msg_match);
	}
}

msg_previous()
{
	if (global_msg_list) {
		int i = number_msgs - 1;
		
		if (msg_num_matches > 0) {
			while (i <= 0 &&
			       msg_matches[0] != global_msg_list[i])
				i--;
			if (i <= 0)
				i = number_msgs - 1;
			else
				i--;
		}
		if (msg_match) free(msg_match);
		msg_match = strdup(global_msg_list[i]->msginfo->queueid);
		find_msg_regex(msg_match);
	}
}

/*  */


msg_control(op, msg, usrs, mytime)
Operations	op;
char		*msg, *usrs, *mytime;
{

	/* arg1 = QID arg2 = time arg3 = control arg4.. = userlist */
	char	*usrlist[100], **args;
	int	numberUsrs, i = 0, ix;

	numberUsrs = sstr2arg(usrs, 100, usrlist, ",");
	
	args = (char **) calloc((unsigned)(4+numberUsrs), sizeof(char *));
	args[0] = msg;
	args[1] = mytime;
	args[2] = (char *) op;
	ix = 3;
	while(i < numberUsrs && ix < (3+numberUsrs)) 
		args[ix++] = usrlist[i++];
	my_invoke(op, args);
	free((char *) args);
}

msg_enable_regex()
{
	int	cont, i = 0, ucont;
	char	buf[BUFSIZ], users[BUFSIZ];
	users[0] = '\0';
	
	if (msg_num_matches > 0) {
		if (msg_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				msg_match, msg_num_matches);
		for (i = 0; i < msg_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (msg_num_matches == 1) {
					fprintf(stdout, "enabling '%s'.\n",
						msg_matches[i]->msginfo->queueid);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "enable '%s' (y/n/q) ?",
						msg_matches[i]->msginfo->queueid);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					ucont = TRUE;
					while (ucont == TRUE) {
						fprintf(stdout, "User list (,) (*=all) = ");
						fflush(stdout);
						if (gets(buf) == NULL)
							exit (OK);
						compress(buf, buf);
						if (emptyStr(buf)) {
							if (users[0] != '\0')
								ucont = FALSE;
							else 
								fprintf(stdout, "no user list given\n");
						} else {
							ucont = FALSE;
							strcpy(users, buf);
						}
					}
					msg_control(msgstart,
						    msg_matches[i]->msginfo->queueid, 
						    users, NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = msg_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

msg_disable_regex()
{
	int	cont, i = 0, ucont;
	char	buf[BUFSIZ], users[BUFSIZ];
	users[0] = '\0';
	
	if (msg_num_matches > 0) {
		if (msg_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				msg_match, msg_num_matches);
		for (i = 0; i < msg_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (msg_num_matches == 1) {
					fprintf(stdout, "disabling '%s'.\n",
						msg_matches[i]->msginfo->queueid);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "disable '%s' (y/n/q) ?",
						msg_matches[i]->msginfo->queueid);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					ucont = TRUE;
					while (ucont == TRUE) {
						fprintf(stdout, "User list (,) (*=all) = ");
						fflush(stdout);
						if (gets(buf) == NULL)
							exit (OK);
						compress(buf, buf);
						if (emptyStr(buf)) {
							if (users[0] != '\0')
								ucont = FALSE;
							else 
								fprintf(stdout, "no user list given\n");
						} else {
							ucont = FALSE;
							strcpy(users, buf);
						}
					}
					msg_control(msgstop,
						    msg_matches[i]->msginfo->queueid, 
						    users, NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = msg_num_matches;
				    default:
					break;
				}
			}
		}
	}
}


msg_clear_regex()
{
	int	cont, i = 0, ucont;
	char	buf[BUFSIZ], users[BUFSIZ];
	users[0] = '\0';
	
	if (msg_num_matches > 0) {
		if (msg_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				msg_match, msg_num_matches);
		for (i = 0; i < msg_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (msg_num_matches == 1) {
					fprintf(stdout, "clearing '%s'.\n",
						msg_matches[i]->msginfo->queueid);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "clear '%s' (y/n/q) ?",
						msg_matches[i]->msginfo->queueid);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					ucont = TRUE;
					while (ucont == TRUE) {
						fprintf(stdout, "User list (,) (*=all) = ");
						fflush(stdout);
						if (gets(buf) == NULL)
							exit (OK);
						compress(buf, buf);
						if (emptyStr(buf)) {
							if (users[0] != '\0')
								ucont = FALSE;
							else 
								fprintf(stdout, "no user list given\n");
						} else {
							ucont = FALSE;
							strcpy(users, buf);
						}
					}
					msg_control(msgclear,
						    msg_matches[i]->msginfo->queueid, 
						    users, NULLCP);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = msg_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

msg_clear_above_regex()
{
	int	cont, i = 0, above = FALSE;
	char	buf[BUFSIZ];
	
	if (msg_num_matches > 0) {
		if (msg_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				msg_match, msg_num_matches);
		for (i = 0; i < msg_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (msg_num_matches == 1) {
					fprintf(stdout, "clearing entities above '%s'.\n",
						msg_matches[i]->msginfo->queueid);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "clear entities above '%s' (y/n/q) ?",
						msg_matches[i]->msginfo->queueid);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					msg_force(msg_matches[i]);
					above = TRUE;
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = msg_num_matches;
				    default:
					break;
				}
			}
		}
		if (above == TRUE)
			mta_clear_above_regex();
	}
}

msg_force (msg)
struct msg_struct	*msg;
{
	char	**args = NULL;

	int	numberUsrs = 0, force;
	int	ix;
	struct recip	*temp;
	if (msg == NULL
	    || msg->msginfo == NULL) return;

	for (temp = msg->reciplist, numberUsrs = 0; 
	     temp != NULL; temp = temp -> next, numberUsrs++)
		continue;

	args = (char **) calloc((unsigned)(4+numberUsrs), sizeof(char *));
	args[0] = msg->msginfo->queueid;
	args[1] = NULL;

	ix = 3;
	force = FALSE;
	for (temp = msg->reciplist; temp != NULL; temp = temp -> next)
		if (temp->status
		    && temp->status->enabled != TRUE) {
			args[ix++] = itoa(temp->id);
			force = TRUE;
		}
	if (force == TRUE) {
		args[ix] = NULLCP;
		args[2] = (char *) msgstart;
		my_invoke(msgstart, args);
	}

	ix = 3;
	force = FALSE;

	for (temp = msg->reciplist; temp != NULL; temp = temp -> next)
		if (temp->status
		    && temp->status->cachedUntil != 0) {
			args[ix++] = itoa(temp->id);
			force = TRUE;
		}
	if (force == TRUE) {
		args[ix] = NULLCP;
		args[2] = (char *) msgclear;
		my_invoke(msgstart, args);
	}
	for (ix = 3; args[ix] != NULLCP && ix < 4+numberUsrs; ix++)
		free (args[ix]);
	free((char *) args);
}	

msg_delay_regex()
{
	int	cont, i = 0, ucont, dcont;
	char	buf[BUFSIZ], users[BUFSIZ], delay[BUFSIZ];
	users[0] = '\0';
	delay[0] = '\0';
	
	if (msg_num_matches > 0) {
		if (msg_num_matches > 1)
			fprintf(stdout, "%s: %d matches\n\n",
				msg_match, msg_num_matches);
		for (i = 0; i < msg_num_matches; i++) {
			cont = TRUE;
			while (cont == TRUE) {
				if (msg_num_matches == 1) {
					fprintf(stdout, "delaying '%s'.\n",
						msg_matches[i]->msginfo->queueid);
					buf[0] = 'y';
				} else {
					fprintf(stdout, "delay '%s' (y/n/q) ?",
						msg_matches[i]->msginfo->queueid);
					fflush(stdout);
					if (gets(buf) == NULL)
						exit(OK);
					compress(buf, buf);
				}
				switch(buf[0]) {
				    case 'y':
				    case 'Y':
					ucont = TRUE;
					while (ucont == TRUE) {
						fprintf(stdout, "User list (,) (*=all) = ");
						fflush(stdout);
						if (gets(buf) == NULL)
							exit (OK);
						compress(buf, buf);
						if (emptyStr(buf)) {
							if (users[0] != '\0')
								ucont = FALSE;
							else 
								fprintf(stdout, "no user list given\n");
						} else {
							ucont = FALSE;
							strcpy(users, buf);
						}
					}
					dcont = TRUE;
					while (dcont == TRUE) {
						fprintf(stdout, "Delay (in s m h d and/or w) = ");
						fflush(stdout);

						if (gets(buf) == NULL)
							exit (OK);
						compress(buf, buf);
						if (emptyStr(buf)) {
							if (delay[0] != '\0')
								dcont = FALSE;
							else 
								fprintf(stdout, "no user list given\n");
						} else {
							dcont = FALSE;
							strcpy(delay, buf);
						}
					}
					msg_control(msgcacheadd,
						    msg_matches[i]->msginfo->queueid, 
						    users, delay);
				    case 'n':
				    case 'N':
					cont = FALSE;
					break;
				    case 'q':
				    case 'Q':
					cont = FALSE;
					i = msg_num_matches;
				    default:
					break;
				}
			}
		}
	}
}

struct type_Qmgr_UserList	*create_userlist(qid, argv)
char	*qid,
	**argv;
{
	struct type_Qmgr_UserList	*temp,
					*head = NULL,
					*tail = NULL;
	int				i = 0;
	struct msg_struct		*msg;
	struct recip			*ix;

	if (strcmp(argv[i], "*") == 0) {
		if ((msg = find_msg(qid)) == NULL)
			return NULL;
		ix = msg -> reciplist;
		while (ix != NULL) {
			temp = (struct type_Qmgr_UserList *) 
				calloc (1, sizeof(*temp));
			temp->RecipientId = (struct type_Qmgr_RecipientId *) 
				malloc(sizeof(struct type_Qmgr_RecipientId));
			temp->RecipientId->parm = ix -> id;
			ix = ix -> next;
			if (head == NULL)
				head = tail = temp;
			else {
				tail->next = temp;
				tail = tail->next;
			}
		}
	} else {
		while (argv[i] != NULL) {
			temp = (struct type_Qmgr_UserList *) 
				calloc (1, sizeof(*temp));
			temp->RecipientId = (struct type_Qmgr_RecipientId *) 
				malloc(sizeof(struct type_Qmgr_RecipientId));
			temp->RecipientId->parm = atoi(argv[i++]);
		
			if (head == NULL)
				head = tail = temp;
			else {
				tail->next = temp;
				tail = tail->next;
			}
		}
	}
	return head;
}

/* ARGSUSED */
int do_msgcontrol (ad, ds, args, arg)
int				ad;
struct client_dispatch		*ds;
char				**args;
struct type_Qmgr_MsgControl	**arg;
/* args[0] = qid */
/* args[1] = time */
/* args[2] = stop, start, clear cacheadd*/
/* args[3..] = NULL terminated list of users */
{
	char	*timestr;
	*arg = (struct type_Qmgr_MsgControl *) malloc(sizeof(**arg));
	
	(*arg)->qid = str2qb(args[0],
			     strlen(args[0]),
			     1);

	(*arg)->users = create_userlist(args[0], &(args[3]));

	(*arg)->control = (struct type_Qmgr_Control *)
		malloc(sizeof(struct type_Qmgr_Control));

	switch ((Operations) args[2]) {
	    case msgstart:
		(*arg)->control->offset = type_Qmgr_Control_start;
		break;
	    case msgstop:
		(*arg)->control->offset = type_Qmgr_Control_stop;
		break;
	    case msgclear:
		(*arg)->control->offset = type_Qmgr_Control_cacheClear;
		break;
	    case msgcacheadd:
		(*arg)->control->offset = type_Qmgr_Control_cacheAdd;
		timestr = mystrtotime(args[1]);
		(*arg)->control->un.cacheAdd = str2qb(timestr,strlen(timestr),1);
		free(timestr);
		break;
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("console : '%s' unknown control param for msgs",
			args[1]));
		return NOTOK;
	}
	return OK;
}

/* ARGSUSED */
msgcontrol_result (sd, id, dummy, result, roi)
int					sd,
	                                id,
	                                dummy;
struct type_Qmgr_Pseudo__newmessage 	*result;
struct RoSAPindication			*roi;
{
	return OK;
}
