/* badness.c: routines calculating and using "badness" */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/badness.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/badness.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: badness.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include	"console.h"

#define	DEFAULT_TOTAL_NUMBER	10000.0
#define DEFAULT_TOTAL_VOLUME	1000000.0
struct tailor	*tailors = NULL;
extern double 	atof();
extern char	tailorfile[];
extern int	max_vert_lines;
extern time_t   currentTime;
extern char	*compress();
double	ub_total_number = DEFAULT_TOTAL_NUMBER, 
	ub_total_volume = DEFAULT_TOTAL_VOLUME;
extern Display	*disp;

/*  */

static double	parse_ub(ub)
char	*ub;
{
	double	pre;
	char	*end;
	pre = strtod(ub, &end);
	
	while (*end != '\0' && isspace(*end)) end++;
	if (*end != '\0') {
		switch	(*end) {
		    case 'h':
			pre *= 60.0;
			break;
		    case 'd':
			pre *= 24.0 * 60.0;
			break;
		    case 'k':
			pre *= 1000.0;
			break;
		    case 'M':
			pre *= 1000000.0;
			break;
		    default:
			break;
		}
	}
	return	pre;
}

static void	add_total_ubs(entry)
char	*entry;		
{
	char	*ix, *str, *margv[20];
	int	i, margc;
	char	*buf = strdup(entry);
	
	if ((ix = index(buf, ':')) == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Incorrect console tailor file entry '%s'",buf));
		free(buf);
		return;
	}
	ix++;
	compress(ix, ix);
	margc = sstr2arg(ix, 20, margv, ",");
	i = 0;
	while (i < margc) {
		if ((ix = index(margv[i], '<')) != NULL) {
			*ix = '\0';
			str = margv[i];
			ix++;
			while(*ix != '\0' && isspace(*ix)) ix++;
			while(*str != '\0' && isspace(*str)) str++;
			if (*ix == '\0' || *str == '\0')
				return;
			if (strncmp(str, "num", 3) == 0)
				ub_total_number = parse_ub(ix);
			else if (strncmp(str, "vol", 3) == 0)
				ub_total_volume = parse_ub(ix);
			else
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown console tailor variable '%s'",
					str));
		}
		i++;
	}
	free(buf);
}
	
static struct tailor	*tailor_new(entry)
char	*entry;
{
	struct tailor 	*ret;
	char		*ix, *str, *margv[20];
	int		i, margc;
	char		*buf = strdup(entry);
	ret = (struct tailor *) calloc(1, sizeof(*ret));

	if ((ix = index(buf, ':')) == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Incorrect console tailor file entry '%s'",buf));
		free((char *) ret);
		free(buf);
		return NULL;
	}
	*ix = '\0';
	ret->key = strdup(buf);
	ix++;
	compress(ix, ix);
	margc = sstr2arg(ix, 20, margv, ",");

	i = 0;
	while (i < margc) {
		if ((ix = index(margv[i], '<')) != NULL) {
			*ix = '\0';
			str = margv[i];
			ix++;
			while (*ix != '\0' && isspace(*ix)) ix++;
			while (*str != '\0' && isspace(*str)) str++;
			if (*ix == '\0' || *str == '\0')
				return NULL;

			if (strncmp(str, "num", 3) == 0) 
				ret->ub_number = parse_ub(ix);
			else if (strncmp(str, "vol", 3) == 0) 
				ret->ub_volume = parse_ub(ix);
			else if (strncmp(str, "age", 3) == 0) 
				ret->ub_age = parse_ub(ix);
			else if (strncmp(str, "las", 3) == 0) 
				ret->ub_last = parse_ub(ix);
			else
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown console tailor variable '%s'",
					str));
		}
		i++;
	}
	free(buf);
	return ret;
}

static void tailor_add(plist, new)
struct tailor	**plist, *new;
{
	new->next = *plist;
	*plist = new;
}

void tailor_free(list)
struct tailor	*list;
{
	if (list->key) free(list->key);
	if (list->next) tailor_free(list->next);
	free((char *) list);
}

/*  */

struct tailor *find_tailor(name, list)
char		*name;
struct tailor	*list;
{
	while (list != NULL
	       && strcmp(list->key, name) != 0)
		list = list->next;
	return list;
}

/*  */
#include	"tai_defs.inc"
extern char	*myname;

static void tailor_add_one_default(plist, type, hardwired)
struct tailor	**plist;
char		*type,
		*hardwired;
{
	char	*temp,
		buf[BUFSIZ];
	if (find_tailor(type, *plist) == NULL) {
		if ((temp = XGetDefault(disp, myname, type)) != NULL) {
			sprintf(buf, "%s:%s",type,temp);
			tailor_add(plist, tailor_new(buf));
		} else if ((temp = XGetDefault(disp, 
					      APPLICATION_CLASS, 
					      type)) != NULL) {
			sprintf(buf, "%s:%s",type,temp);
			tailor_add(plist, tailor_new(buf));
		} else 
			tailor_add(plist,tailor_new(hardwired));
	}
}

static void tailor_add_defaults(plist)
struct tailor	**plist;
{
	/* mts defaults */
	tailor_add_one_default(plist, "mtsin-chan", default_mtsin_chan);
	tailor_add_one_default(plist, "mtsout-chan", default_mtsout_chan);
	tailor_add_one_default(plist, "mtsboth-chan", default_mtsboth_chan);
	tailor_add_one_default(plist, "mtsin-mta", default_mtsin_mta);
	tailor_add_one_default(plist, "mtsout-mta", default_mtsout_mta);
 	tailor_add_one_default(plist, "mtsboth-mta", default_mtsboth_mta);
 	tailor_add_one_default(plist, "mtsin-msg", default_mtsin_msg);
	tailor_add_one_default(plist, "mtsout-msg", default_mtsout_msg);
	tailor_add_one_default(plist, "mtsboth-msg", default_mtsboth_msg);
	
	/* mta defaults */
	tailor_add_one_default(plist, "mtain-chan", default_mtain_chan);
	tailor_add_one_default(plist, "mtaout-chan", default_mtaout_chan);
	tailor_add_one_default(plist, "mtaboth-chan", default_mtaboth_chan);
	tailor_add_one_default(plist, "mtain-mta", default_mtain_mta);
	tailor_add_one_default(plist, "mtaout-mta", default_mtaout_mta);
	tailor_add_one_default(plist, "mtaboth-mta", default_mtaboth_mta);
	tailor_add_one_default(plist, "mtain-msg", default_mtain_msg);
	tailor_add_one_default(plist, "mtaout-msg", default_mtaout_msg);
	tailor_add_one_default(plist, "mtaboth-msg", default_mtaboth_msg);

	/* internal defaults */
	tailor_add_one_default(plist, "internal-chan", default_internal_chan);
	tailor_add_one_default(plist, "internal-mta", default_internal_mta);
	tailor_add_one_default(plist, "internal-msg", default_internal_msg);
	
	/* passive defaults */
	tailor_add_one_default(plist, "passive-chan", default_passive_chan);
	tailor_add_one_default(plist, "passive-mta", default_passive_mta);
	tailor_add_one_default(plist, "passive-msg", default_passive_msg);
}


/*  */

TaiInit()
{
	FILE	*fp = NULL;
	char	buf[BUFSIZ];

	/* read in console tailor file */
	if (tailorfile[0] != '\0') {
		if ((fp  = fopen(tailorfile, "r")) == NULL
		    && tailorfile[0] != '/') {
			char	*home;
			/* try relative to home directory */
			if ((home = getenv("HOME")) != NULLCP) {
				(void) sprintf(buf, "%s/%s",
					       home, tailorfile);
				fp = fopen(buf, "r");
			}
		}
		
		if (fp != NULL) {
			while (fgets(buf, BUFSIZ, fp) != NULLCP) {
				compress(buf, buf);
				if (buf[0] == '\n' || buf[0] == '\0' 
				    || buf[0] == '#') 
					continue;
				if (strncmp(buf,"totals", strlen("totals")) == 0)
					add_total_ubs(buf);
				else
					tailor_add(&tailors, tailor_new(buf));
			}
			fclose(fp);
		}
	}
	tailor_add_defaults(&tailors);
}

/*  */

add_tailor_to_chan(chan)
struct chan_struct	*chan;
{
	char	buf[BUFSIZ];
	
	sprintf(buf, "%s-chan", chan->channelname);
	
	if ((chan->tai = find_tailor(buf, tailors)) == NULL) {
		/* no explicit try defaults */

		char	*type = NULL,
			*dir = NULL;

		switch (chan->chantype) {
		    case int_Qmgr_chantype_mta:
			type = strdup("mta");
			if (chan->outbound > 0 && chan->inbound > 0)
				dir = strdup("both");
			else if (chan->inbound > 0)
				dir = strdup("in");
			else if (chan->outbound > 0)
				dir = strdup("out");
			break;

		    case int_Qmgr_chantype_mts:
			type = strdup("mts");
			if (chan->outbound > 0 && chan->inbound > 0)
				dir = strdup("both");
			else if (chan->inbound > 0)
				dir = strdup("in");
			else if (chan->outbound > 0)
				dir = strdup("out");
			break;

		    case int_Qmgr_chantype_internal:
			type = strdup("internal");
			break;

		    case int_Qmgr_chantype_passive:
		    default:
			type = strdup("passive");
			break;
		}

		sprintf(buf, "%s%s-chan",
			type,
			(dir == NULL) ? "" : dir);

		if (type) free(type);
		if (dir) free(dir);
		chan->tai = find_tailor(buf, tailors);
	}
}
		
add_tailor_to_mta(chan, mta)
struct chan_struct	*chan;
struct mta_struct	*mta;
{
	char	buf[BUFSIZ];
	
	sprintf(buf, "%s-mta", chan->channelname);
	
	if ((mta->tai = find_tailor(buf, tailors)) == NULL) {
		/* no explicit try defaults */

		char	*type = NULL,
			*dir = NULL;

		switch (chan->chantype) {
		    case int_Qmgr_chantype_mta:
			type = strdup("mta");
			if (chan->outbound > 0 && chan->inbound > 0)
				dir = strdup("both");
			else if (chan->inbound > 0)
				dir = strdup("in");
			else if (chan->outbound > 0)
				dir = strdup("out");
			break;

		    case int_Qmgr_chantype_mts:
			type = strdup("mts");
			if (chan->outbound > 0 && chan->inbound > 0)
				dir = strdup("both");
			else if (chan->inbound > 0)
				dir = strdup("in");
			else if (chan->outbound > 0)
				dir = strdup("out");
			break;

		    case int_Qmgr_chantype_internal:
			type = strdup("internal");
			break;

		    case int_Qmgr_chantype_passive:
		    default:
			type = strdup("passive");
			break;
		}

		sprintf(buf, "%s%s-mta",
			type,
			(dir == NULL) ? "" : dir);

		if (type) free(type);
		if (dir) free(dir);
		mta->tai = find_tailor(buf, tailors);
	}
}

void add_tailor_to_msg(chan, msg)
struct chan_struct	*chan;
struct msg_struct	*msg;
{
	char	buf[BUFSIZ];
	
	if (chan == NULL)
		return;

	sprintf(buf, "%s-msg", chan->channelname);
	
	if ((msg->tai = find_tailor(buf, tailors)) == NULL) {
		/* no explicit try defaults */

		char	*type = NULL,
			*dir = NULL;

		switch (chan->chantype) {
		    case int_Qmgr_chantype_mta:
			type = strdup("mta");
			if (chan->outbound > 0 && chan->inbound > 0)
				dir = strdup("both");
			else if (chan->inbound > 0)
				dir = strdup("in");
			else if (chan->outbound > 0)
				dir = strdup("out");
			break;

		    case int_Qmgr_chantype_mts:
			type = strdup("mts");
			if (chan->outbound > 0 && chan->inbound > 0)
				dir = strdup("both");
			else if (chan->inbound > 0)
				dir = strdup("in");
			else if (chan->outbound > 0)
				dir = strdup("out");
			break;

		    case int_Qmgr_chantype_internal:
			type = strdup("internal");
			break;

		    case int_Qmgr_chantype_passive:
		    default:
			type = strdup("passive");
			break;
		}

		sprintf(buf, "%s%s-msg",
			type,
			(dir == NULL) ? "" : dir);

		if (type) free(type);
		if (dir) free(dir);
		msg->tai = find_tailor(buf, tailors);
	}
}

/*  */

int volBadness(vol)
int	vol;
{
	int	retval = 0;
	if (ub_total_volume != 0.0)
		retval = (int) (vol*100/ub_total_volume);
	if (vol != 0 && retval == 0)
		retval = 1;
	return retval;
}

int numBadness(num)
int	num;
{
	int 	retval = 0;
	if (ub_total_number != 0.0) 
		retval = (int) (num*100/ub_total_number);
	if (num != 0 && retval == 0)
		return 1;
	return retval;
}

extern int displayInactIns;

int chanBadness(chan)
struct chan_struct	*chan;
{
	double		average_db = 0.0,
			age,
			last;
	
	int		average = 0,
			noFactors = 0;

	register struct tailor	*tai = chan->tai;

	if (tai == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("No tailoring for channel '%s'", chan->channelname));
		return 0;
	}

	if ((chan->inbound <= 0 || displayInactIns == FALSE)
	    && chan->numberMessages == 0 
	    && chan->numberReports == 0)
		return 0;

	if (tai->ub_number != 0.0) {
		if ((chan->numberMessages + chan->numberReports) >= tai->ub_number)
			return max_bad_channel;
		average_db += ((chan->numberMessages+chan->numberReports) * 100)/ tai->ub_number;
		noFactors++;
	}

	if (tai->ub_volume != 0.0) {
		if (chan->volumeMessages >= tai->ub_volume)
			return max_bad_channel;
		average_db += (chan->volumeMessages * 100)/tai->ub_volume;
		noFactors++;
	}

	if (tai->ub_age != 0.0) {
		if (chan->oldestMessage != 0 && 
		    (chan->numberMessages != 0 || chan->numberReports != 0))
			age = (currentTime-chan->oldestMessage) / 60.0;
		else
			age = 0.0;
		if (age >= tai->ub_age)
			return max_bad_channel;
		average_db += (age * 100)/tai->ub_age;
		noFactors++;
	}

	if (tai->ub_last != 0.0) {
		if (chan->status->lastSuccess != 0)
			last = (currentTime - chan->status->lastSuccess)/60.0;
		else
			last = 0.0;
		if (last >= tai->ub_last)
			return max_bad_channel;
		if (last >= tai->ub_last/2) {
			average_db += (last * 100)/tai->ub_last;
			noFactors++;
		}
	}

	/* scale up those that don't use all factors */
	if (noFactors != 0)
		average = (int) (average_db * max_bad_channel / (noFactors * 100));

	if (average == 0 && average_db != 0.0)
		return 1;
	return average;
}

/*  */

int mtaBadness(mta)
struct mta_struct	*mta;
{
	double		average_db = 0.0,
			age,
			last;
	
	int		average = 0,
			noFactors = 0;

	register struct tailor	*tai = mta->tai;

	if (tai == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("No tailoring for mta '%s'", mta->mta));
		return 0;
	}

	if (mta->numberMessages == 0 && mta->numberReports == 0)
		return 0;

	if (tai->ub_number != 0.0) {
		if (mta->numberMessages+mta->numberReports >= tai->ub_number)
			return max_bad_mta;
		average_db += ((mta->numberMessages+mta->numberReports) * 100)/ tai->ub_number;
		noFactors++;
	}

	if (tai->ub_volume != 0.0) {
		if (mta->volumeMessages >= tai->ub_volume)
			return max_bad_mta;
		average_db += (mta->volumeMessages * 100)/tai->ub_volume;
		noFactors++;
	}

	if (tai->ub_age != 0.0) {
		if (mta->oldestMessage != 0 
		    && (mta->numberMessages != 0 || mta->numberReports != 0))
			age = (currentTime-mta->oldestMessage) / 60.0;
		else
			age = 0;
		if (age >= tai->ub_age)
			return max_bad_mta;
		average_db += (age * 100)/tai->ub_age;
		noFactors++;
	}

	if (tai->ub_last != 0.0) {
		if (mta->status->lastSuccess != 0)
			last = (currentTime - mta->status->lastSuccess)/60.0;
		else
			last = 0;
		if (last >= tai->ub_last)
			return max_bad_mta;
		if (last >= tai->ub_last/2) {
			average_db += (last * 100)/tai->ub_last;
			noFactors++;
		}
	}

	/* scale up those that don't use all factors */
	if (noFactors != 0)
		average = (int) (average_db * max_bad_mta / (noFactors * 100));

	if (average == 0 && average_db != 0.0)
		return 1;
	return average;
}

/*  */

int msgBadness(msg)
struct msg_struct	*msg;
{
	double		average_db = 0.0,
			age;
	
	int		average = 0,
			noFactors = 0;

	register struct tailor	*tai = msg->tai;

	if (tai == NULL) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("No tailoring for msg '%s'", msg->msginfo->queueid));
		return 0;
	}

	if (msg->msginfo->expiryTime != 0
	    && (currentTime - msg->msginfo->expiryTime) >= 0)
		/* expired so max_bad_msg ?? */
		return max_bad_msg;

	if (tai->ub_age != 0.0) {
		age = (currentTime-msg->msginfo->age) / 60.0;
		if (age >= tai->ub_age)
			return max_bad_msg;
		average_db += (age * 100)/tai->ub_age;
		noFactors++;
	}

	if (tai->ub_volume != 0.0) {
		if (msg->msginfo->size > tai->ub_volume)
			return max_bad_msg;
		average_db += (msg->msginfo->size * 100) / tai->ub_volume;
		noFactors++;
	}

	/* scale up those that don't use all factors */
	if (noFactors != 0)
		average = (int) (average_db * max_bad_msg / (noFactors * 100));
	if (average == 0 && average_db != 0.0)
		return 1;
	return average;
}

/*  */
/* colour routines */
extern struct color_item 	*colors;
extern XColor			white,
				black;
extern Pixel			fg, bg;
extern int			num_colors;

unsigned long volcolourOf(vol)
int	vol;
{
	int	bad,
		ix;
	bad = volBadness(vol);
	if (bad == 0)
		return bg;
	else {
		ix = bad*num_colors/ub_total_volume;
		if (ix >= num_colors)
			ix = num_colors-1;
		return colors[ix].colour.pixel;
	}
}

unsigned long numcolourOf(num)
int	num;
{
	int	bad,
		ix;
	bad = numBadness(num);
	if (bad == 0)
		return bg;
	else {
		ix = bad*num_colors/ub_total_number;
		if (ix >= num_colors)
			ix = num_colors-1;
		return colors[ix].colour.pixel;
	}
}

unsigned long chancolourOf (chan)
struct chan_struct	*chan;
{
	int	bad,
		ix;
	/* find nearest index to chanBadness(chan)'s value */
	/* always round up at moment */
	bad = chanBadness(chan);
	if (bad == 0) 
		/* output white border colour */
		return bg;
	else {
		ix = bad*num_colors/max_bad_channel;
		if (ix >= num_colors)
			ix = num_colors-1;
		return colors[ix].colour.pixel;
	}
}

unsigned long mtacolourOf (mta)
struct mta_struct	*mta;
{
	int	bad,
		ix;
	/* find nearest index to mtaBadness(mta)'s value */
	/* always round up at moment */
	bad = mtaBadness(mta);
	if (bad == 0) 
		/* output white border colour */
		return bg;
	else {
		ix = bad*num_colors/max_bad_mta;
		if (ix >= num_colors)
			ix = num_colors-1;
		return colors[ix].colour.pixel;
	}
}
	
unsigned long msgcolourOf (msg)
struct msg_struct	*msg;
{
	int	bad,
		ix;
	/* find nearest index to msgBadness(msg)'s value */
	/* always round up at moment */
	bad = msgBadness(msg);
	if (bad == 0)
		return bg;
	else {
		ix = bad*num_colors/max_bad_msg;
		if (ix >= num_colors)
			ix = num_colors-1;
		return colors[ix].colour.pixel;
	}

}

/*  */
/* border routines */
extern int	max_chan_border,
		max_mta_border,
		max_msg_border;

int chanborderOf(chan)
struct chan_struct	*chan;
{
	int	bad,
		wid;
	bad = chanBadness(chan);
	if (bad == 0)
		return 1;
	else {
		wid = max_chan_border*bad/max_bad_channel + 1;
		if (wid > max_chan_border)
			wid = max_chan_border;
		return wid;
	}
}


int mtaborderOf(mta)
struct mta_struct	*mta;
{
	int	bad,
		wid;
	bad = mtaBadness(mta);
	if (bad == 0)
		return 1;
	else {
		wid = max_mta_border*bad/max_bad_mta + 1;
		if (wid > max_mta_border)
			wid = max_mta_border;
		return wid;
	}
}

int msgborderOf(msg)
struct msg_struct	*msg;
{
	int	bad,
		wid;
	bad = msgBadness(msg);
	if (bad == 0)
		return 1;
	else {
		wid = max_msg_border*bad/max_bad_msg + 1;
		if (wid > max_msg_border)
			wid = max_msg_border;
		return wid;
	}
}

/*  */

/* ordering routines */
extern int			num_channels;
extern struct chan_struct	**ordered_list;

static int channel_compare(one, two)
struct chan_struct	**one,
			**two;
/* return -1 if one worse state then two */
{
	int	onebad,
		twobad;
	onebad = chanBadness((*one));
	twobad = chanBadness((*two));
	if (onebad < twobad)
		return 1;
	else if (onebad > twobad)
		return -1;
	else 
		return 0;
}

order_display_channels()
/* order display_list */
{
	qsort((char *) &(ordered_list[0]),num_channels,
	      sizeof(ordered_list[0]),(IFP)channel_compare);
}


static int mta_compare (one, two)
struct mta_struct	**one,
			**two;
/* returns -1 if ione in worse state than two */
{
	int	onebad,
		twobad;
	onebad = mtaBadness((*one));
	twobad = mtaBadness((*two));
	if (onebad < twobad)
		return 1;
	else if (onebad > twobad)
		return -1;
	else 
		return 0;
}

order_mtas(plist,num)
struct mta_struct	***plist;
int			num;
{
	struct mta_struct **temp = *plist;
	qsort((char *)&(temp[0]),num,sizeof(temp[0]),(IFP)mta_compare);
}

extern int			number_msgs;
extern struct msg_struct	**global_msg_list;

static int msg_compare (one, two)
struct msg_struct	**one,
			**two;
/* returns -1 if ione in worse state than two */
{
	int	onebad,
		twobad;
	onebad = msgBadness((*one));
	twobad = msgBadness((*two));
	if (onebad < twobad)
		return 1;
	else if (onebad > twobad)
		return -1;
	else 
		return 0;
}

order_msgs()
{
	qsort((char *) &(global_msg_list[0]),number_msgs,
	      sizeof(global_msg_list[0]),(IFP)msg_compare);
}

/*  */

int	NumVertLines(chan)
struct chan_struct	*chan;
{
	int	ret = 1 + (max_vert_lines * chanBadness(chan))/max_bad_channel;
	return ret;
}

extern XFontStruct		*disabledFont,
				*activeFont,
				*normalFont;

XFontStruct	*chanFont(chan)
struct chan_struct	*chan;
{
	if (chan == NULL)
		return normalFont;

	if (chan->status->enabled == FALSE)
		return disabledFont;
	else if (chan->numberActiveProcesses > 0)
		return activeFont;
	else
		return normalFont;
}
	
XFontStruct	*mtaFont(mta)
struct mta_struct	*mta;
{
	if (mta == NULL)
		return normalFont;

	if (mta->status->enabled == FALSE)
		return disabledFont;
	else if (mta -> active)
		return activeFont;
	else
		return normalFont;
}

XFontStruct	*msgFont(msg)
struct msg_struct	*msg;
{
	struct recip	*ix;

	if (msg == NULL)
		return normalFont;
	    
	ix = msg->reciplist;

	while (ix != NULL) {
		if (ix -> status -> enabled == FALSE)
			return disabledFont;
		ix = ix -> next;
	}
	return normalFont;
}
