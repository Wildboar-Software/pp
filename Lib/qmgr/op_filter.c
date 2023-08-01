/* op_filter.c: filter operations */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_filter.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/op_filter.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: op_filter.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "qmgr-int.h"
#include "consblk.h"
#include "Qmgr-types.h"
#include <isode/cmd_srch.h>

#define	STR2QB(x)	str2qb((x), strlen(x), 1)

static CMD_TABLE filter_keys[] = {
#define F_CONTENT	1
	QFILTER_CONTENT,	F_CONTENT,
#define F_EIT		2
	QFILTER_EIT,		F_EIT,
#define F_PRIO		3
	QFILTER_PRIO,		F_PRIO,
#define F_RECENT	4
	QFILTER_MORERECENT,	F_RECENT,
#define F_EARLIER	5
	QFILTER_EARLIER,	F_EARLIER,
#define F_MAXSIZE	6
	QFILTER_MAXSIZE,	F_MAXSIZE,
#define F_ORIG		7
	QFILTER_ORIG,		F_ORIG,
#define	F_RECIP		8
	QFILTER_RECIP,		F_RECIP,
#define F_CHANNEL	9
	QFILTER_CHANNEL,	F_CHANNEL,
#define F_MTA		10
	QFILTER_MTA,		F_MTA,
#define F_QUEID		11
	QFILTER_QUEUEID,	F_QUEID,
#define F_MPDU		12
	QFILTER_MPDU,		F_MPDU,
#define F_UA		13
	QFILTER_UA,		F_UA,
	NULLCP,			NOTOK
	};


static int add_filter (key, val, f)
char *key, *val;
struct type_Qmgr_Filter *f;
{
	switch (cmd_srch (key, filter_keys)) {
	    case F_CONTENT:
		if (f->contenttype)
			return NOTOK;
		f -> contenttype = STR2QB(val);
		break;

	    case F_EIT:
		{
			struct type_Qmgr_EncodedInformationTypes *eit;
			eit = (struct type_Qmgr_EncodedInformationTypes *)
				smalloc (sizeof *eit);
			eit -> PrintableString = STR2QB (val);
			eit -> next = f -> eit;
		}
		break;

	    case F_PRIO:
		if (f -> priority)
			return NOTOK;
		f -> priority = (struct type_Qmgr_Priority *)
			smalloc (sizeof *f->priority);
		f -> priority -> parm = atoi (val);
		break;

	    case F_RECENT:
		if (f -> moreRecentThan)
			return NOTOK;
		f -> moreRecentThan = STR2QB(val);
		break;
	    case F_EARLIER:
		if (f -> earlierThan)
			return NOTOK;
		f -> earlierThan = STR2QB(val);
		break;
	    case F_MAXSIZE:
		if (f -> maxSize)
			return NOTOK;
		f -> maxSize = atoi(val);
		break;
	    case F_ORIG:
		if (f -> originator)
			return NOTOK;
		f -> originator = STR2QB(val);
		break;
	    case F_RECIP:
		if (f -> recipient)
			return NOTOK;
		f -> recipient = STR2QB (val);
		break;
	    case F_CHANNEL:
		if (f -> channel)
			return NOTOK;
		f -> channel = STR2QB (val);
		break;
	    case F_MTA:
		if (f -> mta)
			return NOTOK;
		f -> mta = STR2QB (val);
		break;
	    case F_QUEID:
		if (f -> queueid)
			return NOTOK;
		f -> queueid = STR2QB (val);
		break;
	    case F_UA:
		if (f -> uaContentId)
			return NOTOK;
		f -> uaContentId = STR2QB (val);
		break;
	    case F_MPDU:
	    default:
		return NOTOK;
	}
	return OK;
}

/* ARGSUSED */
int arg_filter(ad, args, arg)
int	ad;
char	**args;			/* list of key=value things */
struct type_Qmgr_ReadMessageArgument **arg;
{
	char *cp;
	struct type_Qmgr_Filter *f;
	struct type_Qmgr_FilterList *filterl;

	*arg = (struct type_Qmgr_ReadMessageArgument *)
		smalloc (sizeof **arg);
	(*arg) -> interval = NULL;
	(*arg) -> filters = filterl =
		(struct type_Qmgr_FilterList *)smalloc (sizeof *filterl);
	filterl -> next = NULL;
	filterl -> Filter = f = (struct type_Qmgr_Filter *)
		smalloc (sizeof *f);
	bzero ((char *)f, sizeof *f);

	while (*args) {
		if ((cp = index(*args, '=')) == NULLCP)
			return NOTOK;
		*cp++ = '\0';
		if (add_filter (*args, cp, f) == NOTOK)
			return NOTOK;
		args ++;
	}
	return OK;
}
