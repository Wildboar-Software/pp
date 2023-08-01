/* structs.c: structure building routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/qmgr/RCS/structs.c,v 6.0 1991/12/18 20:27:38 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/structs.c,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: structs.c,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "types.h"
#include "Qmgr-ops.h"
#include <isode/logger.h>

#define	STR2QB(str)	str2qb (str, strlen(str), 1)
#define OOM()	PP_LOG(LLOG_EXCEPTIONS, ("out of memory"))
struct type_Qmgr_PrioritisedChannel	*lpchan ();
struct type_Qmgr_ChannelInfo		*lchani ();
struct type_Qmgr_PrioritisedMta		*lpmta ();
struct type_Qmgr_MtaInfo		*lmta ();
struct type_Qmgr_ProcStatus		*lstatus ();
struct type_UNIV_UTCTime		*ltime ();
struct type_Qmgr_MsgStruct		*lmessage ();
struct type_Qmgr_PerMessageInfo		*lmsginfo ();
struct type_Qmgr_MPDUIdentifier	*lmpduiden ();
struct type_Qmgr_EncodedInformationTypes*leit ();
struct type_Qmgr_RecipientList		*lreciplist ();
struct type_Qmgr_RecipientInfo		*lrecip ();
struct type_Qmgr_ChannelList		*lchanlist ();

MsgStruct *newmsgstruct ();
static Reciplist *newrecips ();
static LIST_BPT *neweit ();
static MPDUid *newmpduid ();

void	freerecips ();
void	freempdu ();

struct type_Qmgr_ChannelInfo *lchani (clp)
Chanlist	*clp;
{
	struct type_Qmgr_ChannelInfo	*ci;

	PP_DBG (("lchani ()"));

	ci = (struct type_Qmgr_ChannelInfo *)
		calloc (1, sizeof (*ci));
	if (ci == NULL) {
		OOM ();
		return NULL;
	}

	ci -> channel = STR2QB (clp -> channame);
	if (clp -> chan -> ch_show)
		ci -> channelDescription = STR2QB (clp -> chan -> ch_show);
	else
		ci -> channelDescription = STR2QB (clp -> channame);
	
	ci -> oldestMessage = ltime (clp -> oldest);
	ci -> numberMessages = clp -> num_msgs;
	ci -> numberReports = clp -> num_drs;
	ci -> volumeMessages = clp -> volume;
	ci -> numberActiveProcesses = clp -> nactive;
	ci -> status = lstatus (clp);
	ci -> direction = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_BITS);
	switch (clp -> chan -> ch_chan_type) {
	    case CH_IN:
		bit_on (ci -> direction, bit_Qmgr_direction_inbound);
		break;

	    case CH_BOTH:
		bit_on (ci -> direction, bit_Qmgr_direction_inbound);
		/* fall */
	    case CH_OUT:
		bit_on (ci -> direction, bit_Qmgr_direction_outbound);
		break;
	}
	if (clp -> chan -> ch_access == CH_MTS)
		ci -> chantype = int_Qmgr_chantype_mts;
	else
		switch (clp -> chan -> ch_chan_type) {
		    case CH_SHAPER:
		    case CH_WARNING:
		    case CH_DELETE:
		    case CH_QMGR_LOAD:
		    case CH_DEBRIS:
		    case CH_TIMEOUT:
			ci -> chantype = int_Qmgr_chantype_internal;
			break;

		    default:
			ci -> chantype = int_Qmgr_chantype_mta;
			break;
		}
	ci -> maxprocs = clp -> chan -> ch_maxproc;
	ci -> numberMtas = clp -> nmtas;
	return ci;
}

struct type_Qmgr_ProcStatus *lstatus (clp)
Chanlist *clp;
{
	struct type_Qmgr_ProcStatus *ps;

	ps = (struct type_Qmgr_ProcStatus *) calloc (1, sizeof *ps);
	if (ps == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, "memory", ("out of"));
		return NULL;
	}

	ps -> enabled = clp -> chan_enabled;
	if (clp -> lastattempt) {
		if((ps -> lastAttempt = ltime (clp -> lastattempt)) == NULL) {
			free_Qmgr_ProcStatus (ps);
			return NULL;
		}
	}
	if (clp -> lastsuccess)
		ps -> lastSuccess = ltime (clp -> lastsuccess);
	if (clp -> cache.cachetime > current_time) {
		if ((ps -> cachedUntil = ltime (clp -> cache.cachetime))
		    == NULL) {
			free_Qmgr_ProcStatus (ps);
			return NULL;
		}
	}

	return ps;
}

struct type_Qmgr_PrioritisedMta *lpmta (mlp, clp)
Mtalist *mlp;
Chanlist	*clp;
{
	struct type_Qmgr_PrioritisedMta *pmta;

	PP_DBG (("lpmta ()"));

	pmta = (struct type_Qmgr_PrioritisedMta *) calloc (1, sizeof (*pmta));
	if (pmta == NULL) {
		OOM ();
		return NULL;
	}

	pmta -> priority = (struct type_Qmgr_Priority *)
		calloc (1, sizeof *pmta -> priority);
	if (pmta -> priority == NULL) {
		OOM ();
		free_Qmgr_PrioritisedMta (pmta);
		return NULL;
	}

	pmta -> priority -> parm = 1;

	pmta -> mta = lmta (mlp, clp);
	if (pmta -> mta == NULL) {
		free_Qmgr_PrioritisedMta (pmta);
		return NULL;
	}

	return pmta;
}

struct type_Qmgr_MtaInfo *lmta (mlp, clp)
Mtalist *mlp;
Chanlist *clp;
{
	struct type_Qmgr_MtaInfo *mta;

	mta = (struct type_Qmgr_MtaInfo *) calloc (1, sizeof *mta);
	if (mta == NULL) {
		OOM ();
		return NULL;
	}

	mta -> channel = STR2QB (clp -> channame);
	mta -> mta = STR2QB (mlp -> mtaname);

	mta -> oldestMessage = ltime (mlp -> oldest);
	mta -> numberMessage = mlp -> num_msgs;
	mta -> numberDRs = mlp -> num_drs;
	mta -> volumeMessages = mlp -> volume;

	mta -> status = (struct type_Qmgr_ProcStatus *)
		calloc (1, sizeof *mta -> status);
	if (mta -> status == NULL) {
		OOM ();
		free_Qmgr_MtaInfo (mta);
		return NULL;
	}

	mta -> status -> enabled = mlp -> mta_enabled;
	if (mlp -> lastattempt)
		mta -> status -> lastAttempt = ltime (mlp -> lastattempt);
	if (mlp -> lastsuccess)
		mta -> status -> lastSuccess = ltime (mlp -> lastsuccess);

	if (mlp -> cache.cachetime > current_time)
		mta -> status -> cachedUntil = ltime (mlp -> cache.cachetime);
	mta -> active = mlp -> nactive > 0;
	if (mlp -> info)
		mta -> info = STR2QB (mlp -> info);
	else
		mta -> info = NULL;
	return mta;
}

 struct type_UNIV_UTCTime *ltime (when)
time_t	when;
{
	UTC	ut;
	char	*str;
	struct qbuf *qb;

	ut = time_t2utc (when);
	str = utct2str (ut);
	
	qb = STR2QB (str);
	free ((char *)ut);

	return qb;
}


struct type_Qmgr_PrioritisedChannel *lpchan (clp)
Chanlist *clp;
{
	struct type_Qmgr_PrioritisedChannel *pc;

	PP_DBG (("lpchan ()"));

	pc = (struct type_Qmgr_PrioritisedChannel *)
		calloc (1, sizeof *pc);
	if (pc == NULL) {
		OOM ();
		return NULL;
	}

	pc -> channel = lchani (clp);
	pc -> priority = (struct type_Qmgr_Priority *)
		calloc (1, sizeof *pc -> priority);
	if (pc -> priority == NULL) {
		OOM ();
		free_Qmgr_PrioritisedChannel (pc);
		return NULL;
	}

	pc -> priority -> parm = int_Qmgr_Priority_normal;

	return pc;
}

struct type_Qmgr_UserList *lusers (ml, check)
Mlist	*ml;
int	check;
{
	int	i;
	struct type_Qmgr_UserList **upp, *upbase, *up;
	Reciplist *rl;

	PP_TRACE (("lusers (ml, %d) %d", check, ml -> rcount));
	upbase = NULL;
	upp = &upbase;

	for (i = 0; i < ml -> rcount; i++) {
		rl = ml -> recips[i];
		if (check) {
			if (rl -> msg_enabled == 0)
				continue;
			if (rl -> cache.cachetime > current_time)
				continue;
		}
		*upp = (struct type_Qmgr_UserList *) calloc (1, sizeof *up);
		up = *upp;
		upp = &(*upp) -> next;
		up -> RecipientId = (struct type_Qmgr_RecipientId *)
			calloc (1, sizeof *up -> RecipientId);
		up -> RecipientId -> parm = rl->id;
		PP_DBG (("Adding user %d to list", rl->id));
	}
	return upbase;
}

MsgStruct *newmsgstruct (msg)
struct type_Qmgr_MsgStruct *msg;
{
	MsgStruct *ms;
	struct type_Qmgr_PerMessageInfo *pmi;

	if ((ms = (MsgStruct *) malloc (sizeof *ms)) == NULLMS)
		return ms;
	bzero ((char *)ms, sizeof *ms);

	ms -> ms_forw = NULLMS;
	ms -> count = 0;
	pmi = msg -> messageinfo;

	ms -> queid = qb2str (pmi -> queueid);
	ms -> mpduid = newmpduid (pmi -> mpduiden);
	ms -> originator = qb2str (pmi -> originator);
	if (pmi -> contenttype)
		ms -> contenttype = qb2str (pmi -> contenttype);
	else ms -> contenttype = NULLCP;
	if (pmi -> eit)
		ms -> eit = neweit (pmi -> eit);
	else ms -> eit = NULL;
	ms -> priority = pmi -> priority -> parm;
	ms -> size = pmi -> size;
	ms -> age = utcqb2time_t (pmi -> age);
	ms -> warninterval = pmi -> warnInterval;
	if (ms -> warninterval == 0)
		ms -> warninterval = 24 * 60 * 60; /* default it */
	else	ms -> warninterval *= 3600;
	ms -> numberwarns = pmi -> numberWarningsSent;
	ms -> expirytime = utcqb2time_t (pmi -> expiryTime);
	if (pmi -> deferredTime)
		ms -> defferedtime = utcqb2time_t (pmi -> deferredTime);
	else 	ms -> defferedtime = 0;
	if ( pmi -> uaContentId)
		ms -> uacontent = qb2str (pmi -> uaContentId);
	else	ms -> uacontent = NULLCP;
	if (pmi -> inChannel) {
		char *p = qb2str (pmi -> inChannel);
		ms -> inchan = ch_nm2struct (p);
		free (p);
	}
	ms -> recips = newrecips (msg -> recipientlist);
	ms -> m_locked = 0;
	return ms;
}

static Reciplist *newrecips (rlist)
struct type_Qmgr_RecipientList *rlist;
{
	Reciplist *nr;
	struct type_Qmgr_RecipientInfo *ri;
	struct type_Qmgr_ChannelList *qclp;
	LIST_RCHAN	*chan;
	char	*p;
	int	i;

	if (rlist == NULL)
		return NULLRL;

	nr = (Reciplist *) smalloc (sizeof *nr);
	bzero ((char *)nr, sizeof *nr);

	ri = rlist -> RecipientInfo;
	nr -> status = ST_NORMAL;
	nr -> id = ri -> id -> parm;
	nr -> user = qb2str (ri -> user);
	nr -> mta = qb2str (ri -> mta);
	nr -> chans = NULL;
	for (qclp = ri -> channelList; qclp; qclp = qclp -> next) {
		chan = list_rchan_new (NULLCP, p = qb2str(qclp -> Channel));
		free (p);
		if (chan)
			list_rchan_add (&nr -> chans, chan);
	}
	nr -> cchan = nr -> chans;
	for (i = ri->channelsDone; i > 0; i--) {
		if (nr -> cchan == NULL) {
			PP_LOG (LLOG_EXCEPTIONS, ("Run out of channels"));
			break;
		}
		nr -> cchan = nr -> cchan -> li_next;
	}
	nr -> msg_enabled = 1;

	nr -> rp_next = newrecips (rlist -> next);

	return nr;
}

static LIST_BPT *neweit (eit)
struct type_Qmgr_EncodedInformationTypes *eit;
{
	LIST_BPT	*bpt = NULLIST_BPT, *bp;
	char	*p;

	for (; eit; eit = eit -> next) {
		bp = list_bpt_new (p = qb2str (eit -> PrintableString));
		if (bp)
			list_bpt_add (&bpt, bp);
		free (p);
	}

	return bpt;
}

			
static MPDUid	*newmpduid (mpduiden)
struct type_Qmgr_MPDUIdentifier *mpduiden;
{
	MPDUid	*mi;
	struct type_Qmgr_GlobalDomainIdentifier *gdi =
		mpduiden -> global;
	struct type_Qmgr_CountryName *co = gdi -> country;
	struct type_Qmgr_AdministrationDomainName *admd =
		gdi -> admd;
	struct type_Qmgr_PrivateDomainIdentifier *prmd =
		gdi -> prmd;
	GlobalDomId *gd;

	mi = (MPDUid *) malloc (sizeof *mi);

	mi -> mpduid_string = qb2str (mpduiden -> local);
	gd = &mi -> mpduid_DomId;

	gd -> global_Country =
		qb2str (co -> offset ==
			type_Qmgr_CountryName_numeric ?
			co -> un.numeric :
			co -> un.printable);
 
	gd -> global_Admin =
		qb2str (admd -> offset ==
			type_Qmgr_AdministrationDomainName_numeric ?
			admd -> un.numeric :
			admd -> un.printable);

	if (prmd)
		gd -> global_Private =
			qb2str (prmd -> offset ==
				type_Qmgr_PrivateDomainIdentifier_numeric ?
				prmd -> un.numeric :
				prmd -> un.printable);
	else	gd -> global_Private = NULLCP;
	return mi;
}

Filter	*newfilter (flp)
struct type_Qmgr_FilterList *flp;
{
	Filter	*f;
	struct type_Qmgr_Filter *fl;

	if (flp == NULL || flp -> Filter == NULL)
		return NULL;
		
	f = (Filter *) calloc (1, sizeof *f);

	fl = flp -> Filter;

	if (fl -> contenttype)
		f -> cont = qb2str (fl -> contenttype);
	if (fl -> eit)
		f -> eit = neweit (fl -> eit);
	if (fl -> priority)
		f -> priority = fl -> priority -> parm;
	else	f -> priority = -1;

	if (fl -> moreRecentThan)
		f -> morerecent = utcqb2time_t (fl -> moreRecentThan);
	if (fl -> earlierThan)
		f -> earlier = utcqb2time_t (fl -> earlierThan);
	f -> maxsize = fl -> maxSize;
	if (fl -> originator)
		f -> orig = qb2str (fl -> originator);
	if (fl -> recipient)
		f -> recip = qb2str (fl -> recipient);
	if (fl -> channel) {
		char *p;

		f -> channel = ch_nm2struct(p = qb2str (fl -> channel));
		free (p);
	}
	if (fl -> mta)
		f -> mta = qb2str (fl -> mta);
	if (fl -> queueid)
		f -> queid = qb2str (fl -> queueid);
	if (fl -> mpduiden)
		f -> mpduid = newmpduid (fl -> mpduiden);
	if (fl -> uaContentId)
		f -> uacontent = qb2str (fl -> uaContentId);
	if (flp -> next)
		f -> next = newfilter (flp -> next);
	return f;
}


struct type_Qmgr_MsgStruct *lmessage (ms)
MsgStruct *ms;
{
	struct type_Qmgr_MsgStruct *qms;

	qms = (struct type_Qmgr_MsgStruct *) calloc (1, sizeof *qms);

	qms -> messageinfo = lmsginfo (ms);
	qms -> recipientlist = lreciplist (ms -> recips, NULLCP);
	return qms;
}

struct type_Qmgr_MsgStruct *l_cm_message (ml)
Mlist *ml;
{
	struct type_Qmgr_MsgStruct *qms;
	struct type_Qmgr_RecipientList **rl;
	int	i;

	qms = (struct type_Qmgr_MsgStruct *) calloc (1, sizeof *qms);

	qms -> messageinfo = lmsginfo (ml -> ms);
	rl = &qms -> recipientlist;
	*rl = (struct type_Qmgr_RecipientList *) smalloc (sizeof **rl);
	(*rl) -> RecipientInfo = lrecip (ml -> ms -> recips,
					 ml -> ms -> recips,
					 NULLCP);
	(*rl) -> next = NULL;
	rl = &(*rl) -> next;
	for ( i = 0; i < ml -> rcount; i++) {
		*rl = (struct type_Qmgr_RecipientList *)
			smalloc (sizeof **rl);
		(*rl) -> RecipientInfo = lrecip (ml -> recips[i],
						 ml -> ms -> recips,
						 ml -> info);
		(*rl) -> next = NULL;
		rl = &(*rl) -> next;
	}
	return qms;
}

struct type_Qmgr_PerMessageInfo *lmsginfo (ms)
MsgStruct *ms;
{
	struct type_Qmgr_PerMessageInfo *pmi;

	pmi = (struct type_Qmgr_PerMessageInfo *) calloc (1, sizeof *pmi);

	pmi -> queueid = STR2QB (ms -> queid);
	pmi -> mpduiden = lmpduiden (ms->mpduid);
	pmi -> originator = STR2QB (ms -> originator);
	if (ms -> contenttype)
		pmi -> contenttype = STR2QB (ms -> contenttype);
	if (ms -> eit)
		pmi -> eit = leit (ms -> eit);
	pmi -> priority = (struct type_Qmgr_Priority *)
		calloc (1, sizeof *(pmi -> priority));
	pmi -> priority -> parm = ms -> priority;
	pmi -> size = ms -> size;
	pmi -> age = ltime (ms->age);
	pmi -> warnInterval = ms -> warninterval;
	pmi -> numberWarningsSent = ms -> numberwarns;
	pmi -> expiryTime = ltime (ms->expirytime);
	if (ms -> defferedtime)
		pmi -> deferredTime = ltime (ms -> defferedtime);
	else	pmi -> deferredTime = NULL;
	if (ms -> uacontent)
		pmi -> uaContentId = STR2QB (ms -> uacontent);
	else	pmi -> uaContentId = NULL;
	pmi -> errorCount = ms -> nerrors;
	pmi -> optionals |= opt_Qmgr_PerMessageInfo_errorCount;

	if (ms -> inchan)
		pmi -> inChannel = STR2QB (ms -> inchan -> ch_name);
	return pmi;
}

struct type_Qmgr_MPDUIdentifier *lmpduiden (mpduid)
MPDUid *mpduid;
{
	struct type_Qmgr_MPDUIdentifier *mp;
	struct type_Qmgr_GlobalDomainIdentifier *gdi;
	struct type_Qmgr_CountryName *co;
	struct type_Qmgr_AdministrationDomainName *admd;
	struct type_Qmgr_PrivateDomainIdentifier *prmd;

	mp = (struct type_Qmgr_MPDUIdentifier *) calloc (1, sizeof *mp);

	mp -> local = STR2QB (mpduid -> mpduid_string);

	gdi = mp -> global =
		(struct type_Qmgr_GlobalDomainIdentifier *)
			calloc (1, sizeof *gdi);
	
	co = gdi -> country =
		(struct type_Qmgr_CountryName *) calloc (1, sizeof *co);
	co -> offset = type_Qmgr_CountryName_printable;
	co -> un.printable =
		STR2QB (mpduid -> mpduid_DomId.global_Country);

	admd = gdi -> admd =
		(struct type_Qmgr_AdministrationDomainName *)
			calloc (1, sizeof *admd);
	admd -> offset = type_Qmgr_AdministrationDomainName_printable;
	admd -> un.printable =
		STR2QB (mpduid -> mpduid_DomId.global_Admin);

	if (mpduid -> mpduid_DomId.global_Private) {
		prmd = gdi -> prmd =
			(struct type_Qmgr_PrivateDomainIdentifier *)
				calloc (1, sizeof *prmd);
		prmd -> offset = type_Qmgr_PrivateDomainIdentifier_printable;
		prmd -> un.printable =
			STR2QB (mpduid -> mpduid_DomId.global_Private);
	}
	else
		gdi -> prmd = NULL;

	return mp;;
}

struct type_Qmgr_EncodedInformationTypes *leit (eit)
LIST_BPT *eit;
{
	struct type_Qmgr_EncodedInformationTypes *et = NULL, *ep;
	LIST_BPT *bp;

	for (bp = eit; bp; bp = bp -> li_next) {
		ep = (struct type_Qmgr_EncodedInformationTypes *)
			calloc (1, sizeof *eit);
		if (et == NULL)
			et = ep;
		ep -> PrintableString = STR2QB (bp -> li_name);
		ep = ep -> next;
	}
	return et;
}

struct type_Qmgr_RecipientList *lreciplist (recips, info)
Reciplist *recips;
char	*info;
{
	struct type_Qmgr_RecipientList *rlbase = NULL, *rl = NULL;
	Reciplist *rlp;

	for (rlp = recips; rlp != NULLRL; rlp = rlp -> rp_next) {
		if (rlbase == NULL)
			rlbase = rl = (struct type_Qmgr_RecipientList *)
				calloc (1, sizeof *rl);
		else {
			rl -> next = (struct type_Qmgr_RecipientList *)
				calloc (1, sizeof *rl);
			rl = rl -> next;
		}
		rl -> RecipientInfo = lrecip (rlp, recips, info);
	}
	return rlbase;
}

struct type_Qmgr_RecipientInfo *lrecip (rlp, rl0, info)
Reciplist *rlp;
Reciplist *rl0;
char	*info;
{
	struct type_Qmgr_RecipientInfo *ri;

	ri = (struct type_Qmgr_RecipientInfo *) calloc (1, sizeof *ri);

	ri -> user = STR2QB (rlp -> user);
	ri -> id = (struct type_Qmgr_RecipientId *)
		calloc (1, sizeof *ri -> id);
	ri -> id -> parm = rlp -> id;
	ri -> mta = STR2QB (rlp -> mta);

	switch (rlp -> status) {
	    case ST_NORMAL:
		{
			LIST_RCHAN *lcp;

			ri -> channelsDone = 0;
			ri -> channelList = lchanlist (rlp -> chans);
			for (lcp = rlp -> chans; lcp && lcp != rlp -> cchan;
			     lcp = lcp -> li_next)
				ri -> channelsDone ++;
		}
		break;

	    case ST_DR:
		ri -> channelsDone = 0;
		ri -> channelList = lchanlist (rl0 -> chans);
		break;

	    case ST_DELETE:
		ri -> channelsDone = 0;
		ri -> channelList = (struct type_Qmgr_ChannelList *)
			smalloc (sizeof *ri -> channelList);
		ri -> channelList -> Channel =
			STR2QB (delete_chan -> channame);
		ri -> channelList -> next = NULL;
		break;

	    case ST_TIMEOUT:
		ri -> channelsDone = 0;
		ri -> channelList = (struct type_Qmgr_ChannelList *)
			smalloc (sizeof *ri -> channelList);
		ri -> channelList -> Channel =
			STR2QB (timeout_chan -> channame);
		ri -> channelList -> next = NULL;
		break;
	}

	ri -> procStatus = (struct type_Qmgr_ProcStatus *)
		calloc (1, sizeof *ri -> procStatus);
	ri -> procStatus -> enabled = rlp -> msg_enabled;
	if (rlp -> cache.cachetime > current_time)
		ri -> procStatus -> cachedUntil =
			ltime (rlp -> cache.cachetime);
	if (info)
		ri -> info = STR2QB (info);
	else	ri -> info = NULL;
	return ri;
}

struct type_Qmgr_ChannelList *lchanlist (chans)
LIST_RCHAN *chans;
{
	struct type_Qmgr_ChannelList *clp;

	if (chans == NULL)
		return NULL;

	clp = (struct type_Qmgr_ChannelList *) calloc (1, sizeof *clp);

	clp -> Channel = STR2QB (chans -> li_chan -> ch_name);
	clp -> next = lchanlist (chans -> li_next);
	return clp;
}

	
void freems (ms)
MsgStruct *ms;
{
	freerecips (ms->recips);
	if (ms -> uacontent)
		free (ms -> uacontent);
	list_bpt_free (ms -> eit);
	if (ms -> contenttype)
		free (ms -> contenttype);
	if (ms -> originator)
		free (ms -> originator);
	freempdu (ms -> mpduid);
	if (ms -> queid)
		free (ms -> queid);
	free ((char *)ms);
}

void freerecips (rlist)
Reciplist *rlist;
{
	if (rlist == NULLRL)
		return;
	freerecips (rlist -> rp_next);
	list_rchan_free (rlist -> chans);
	free (rlist -> mta);
	free (rlist -> user);

	free ((char *)rlist);
}

void freempdu (mp)
MPDUid *mp;
{
	MPDUid_free (mp);
	free ((char *)mp);
}

void freefilter (f)
Filter	*f;
{
	if (f -> cont)
		free (f -> cont);
	if (f -> eit)
		list_bpt_free (f -> eit);
	if (f -> orig)
		free (f -> orig);
	if (f -> recip)
		free (f -> recip);
	if (f -> mta)
		free (f -> mta);
	if (f -> queid)
		free (f -> queid);
	if (f -> mpduid)
		freempdu (f -> mpduid);
	if (f -> uacontent)
		free (f -> uacontent);
	if (f -> next)
		freefilter (f -> next);
	free ((char *)f);
}
