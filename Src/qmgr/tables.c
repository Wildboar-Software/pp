/* tables.c: table handling routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/qmgr/RCS/tables.c,v 6.0 1991/12/18 20:27:38 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/tables.c,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: tables.c,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "types.h"
#include "qmgr.h"

/* Variables */

MsgStruct	*msg_hash[QUE_HASHSIZE];
Chanlist	**chan_list;
int		nchanlist = 0;

/* Procedures & functions */

extern	char	*strdup ();
int		hash ();
void		addtochan ();
void		create_chan ();
Mtalist		*findmtalist ();
MsgStruct	*find_msg ();
Mlist		*findmtamsg ();
Chanlist	*delete_chan = NULLCHANLIST;
Chanlist	*loader_chan = NULLCHANLIST;
Chanlist	*trash_chan = NULLCHANLIST;
Chanlist	*timeout_chan = NULLCHANLIST;
Chanlist	*warn_chan = NULLCHANLIST;
Chanlist	*findchanlist ();
static void	mcontrol ();
static int	chaninsert ();
static int	insertinmta ();
static int 	addmtaid ();
static int	filtermatch ();
static int	compmpduid ();
static int	nullstrcmp ();
static int	bind_result ();
static int 	bind_error ();
static void	setchannels ();

void		insertinchan ();
void		insertindelchan ();
void		insertindrchan ();
void		cache_clear ();
void		cache_set ();
void		cache_inc ();
void		freems ();
void		freefilter ();

int		zapmtamsg ();

/* External functions */
extern struct type_Qmgr_PrioritisedChannel *lpchan ();
extern struct type_Qmgr_ChannelInfo *lchani ();
extern struct type_Qmgr_PrioritisedMta *lpmta ();
extern struct type_Qmgr_MtaInfo	*lmta ();
extern struct type_Qmgr_ProcStatus *lstatus ();
extern struct type_Qmgr_MsgStruct *lmessage ();
extern struct type_Qmgr_MsgStruct *l_cm_message ();
extern struct type_UNIV_UTCTime *ltime ();
extern void addtorunq (), delfromrunq ();
extern void investigate_mta ();
extern MsgStruct *newmsgstruct ();
extern Filter *newfilter ();

extern int maxchansrunning, nchansrunning;

#define STR2QB(s)	str2qb ((s), strlen((s)), 1)

static int check_credentails (cb, name, passwd)
Connblk *cb;
char	*name, *passwd;
{
	extern char *crypt ();
	char 	result[BUFSIZ];
	static Table	*auth= NULLTBL;
	char	*av[30];
	int	ac;
	char	*tbl_passwd = NULLCP;
	char	*tbl_rights = NULLCP;
	char	*tbl_name;
	extern char *qmgr_auth_tbl;

	cb -> cb_authenticated = 0;
	if (auth == NULLTBL) {
		if ((auth = tb_nm2struct(qmgr_auth_tbl)) == NULLTBL) {
			PP_NOTICE (("Accepted Limited Access for %s",
				    name));
			return int_Qmgr_result_acceptedLimitedAccess;
		}
	}
	if (name == NULLCP)
		tbl_name = "anon";
	else	tbl_name = name;

	if (tb_k2val (auth, tbl_name, result, TRUE) == NOTOK) {
		if (name == NULLCP) {
			PP_NOTICE (("Accepted Limited Access for %s",
				    tbl_name));
			return int_Qmgr_result_acceptedLimitedAccess;
		}
		else {
			PP_NOTICE (("Refused connection for %s", tbl_name));
			return NOTOK;
		}
	}

	ac = sstr2arg (result, 30, av, ", \t");
	for (ac --; ac >= 0; ac --) {
		char	*cp;

		if ((cp = index (av[ac], '=')) == NULLCP)
			tbl_passwd = av[ac];
		else {
			*cp ++ = '\0';
			if (lexequ (av[ac], "passwd") == 0)
				tbl_passwd = cp;
			else if (lexequ (av[ac], "rights") == 0)
				tbl_rights = cp;
		}
	}

	if (tbl_passwd) {
		if (passwd == NULLCP) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Password required, none given for %s",
				 tbl_name));
			return NOTOK;
		}
		if (strcmp (tbl_passwd, crypt (passwd, tbl_passwd)) != 0) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Password mismatch for %s", tbl_name));
			return NOTOK;
		}
	}
	/* now authenticated, if required */

	if (tbl_rights == NULLCP) {
		PP_NOTICE (("Limited Access granted for %s (no default rights)",
			    tbl_name));
		cb -> cb_authenticated = 0;
		return int_Qmgr_result_acceptedLimitedAccess;
	}
	if (lexequ (tbl_rights, "none") == 0) {
		PP_NOTICE (("%s explicitly refused", tbl_name));
		return NOTOK;
	}
	if (lexequ (tbl_rights, "limited") == 0) {
		PP_NOTICE (("Limited access granted for %s", tbl_name));
		return int_Qmgr_result_acceptedLimitedAccess;
	}
	if (lexequ (tbl_rights, "full") == 0) {
		PP_NOTICE (("Full Access granted for %s", tbl_name));
		cb -> cb_authenticated = 1;
		return int_Qmgr_result_acceptedFullAccess;
	}
	PP_LOG (LLOG_EXCEPTIONS, ("Unknown rights '%s'", tbl_rights));
	return NOTOK;
}

int start_routine (sd, acs, pep, npe)
int	sd;
struct AcSAPstart *acs;
PE	*pep;
int	*npe;
{
	struct type_Qmgr_BindArgument *ba;
	Connblk *cb;

	PP_TRACE (("start_routine (%d)", sd));
	*pep = NULLPE;
	cb = newcblk (cb_responder);
	cb -> cb_fd = sd;

	*npe = 0;
	if (acs -> acs_ninfo == 0) {
		cb -> cb_authenticated = 0;
		return bind_result (pep, npe, cb,
				    int_Qmgr_result_acceptedLimitedAccess);
	}
	
	if (decode_Qmgr_BindArgument (acs -> acs_info[0], 1,
				       NULLVP, NULLIP, &ba) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("failed to parse connect data [%s]",
					  PY_pepy));
		cb -> cb_authenticated = 0;
		return bind_result (pep, npe, cb,
				     int_Qmgr_result_acceptedLimitedAccess);
	}
	if (ba -> offset == type_Qmgr_BindArgument_noAuthentication) {
		cb -> cb_authenticated = 0;
		free_Qmgr_BindArgument (ba);
		if (check_credentails (cb, NULLCP, NULLCP) == NOTOK)
			return bind_error (pep, npe, cb,
					   int_Qmgr_reason_badCredentials);
		return bind_result (pep, npe, cb,
				    int_Qmgr_result_acceptedLimitedAccess);
	}

	if (ba -> offset == type_Qmgr_BindArgument_weakAuthentication) {
		char	*username;
		char	*pass;
		int	result;

		username = qb2str (ba -> un.weakAuthentication -> username);
		if (ba -> un.weakAuthentication -> passwd)
			pass = qb2str (ba -> un.weakAuthentication -> passwd);
		else pass = NULLCP;

		result = check_credentails (cb, username, pass);

		if (username) free (username);
		if (pass) free (pass);
		free_Qmgr_BindArgument (ba);
		if (result == NOTOK)
			return bind_error (pep, npe, cb,
					   int_Qmgr_reason_badCredentials);
		else
			return bind_result (pep, npe, cb, result);
	}
	free_Qmgr_BindArgument (ba);
	return bind_error (pep, npe, cb, int_Qmgr_reason_congested);
}


static int bind_result (pep, npe, cb, type)
PE	*pep;
int	*npe;
Connblk *cb;
int	type;
{
	struct type_Qmgr_BindResult *br;
	extern char *qmgr_hostname, *ppversion;

	PP_TRACE (("bind_result"));

	br = (struct type_Qmgr_BindResult *) smalloc (sizeof *br);
	br -> result = type;
	br -> information = STR2QB (qmgr_hostname);
	br -> version = STR2QB (ppversion);

	if (encode_Qmgr_BindResult (pep, 1, NULLCP, 0, br) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("failed to encode BindResult [%s]",
			PY_pepy));
		free_Qmgr_BindResult (br);
		return bind_error (pep, npe, cb,
				   int_Qmgr_reason_congested);
	}
	(*pep) -> pe_context = 3;
	free_Qmgr_BindResult (br);
	*npe = 1;
	return ACS_ACCEPT;
}

static int bind_error (pep, npe, cb, type)
PE	*pep;
int	*npe;
Connblk *cb;
int	type;
{
	struct type_Qmgr_BindError *be;

	PP_TRACE (("bind_error"));

	be = (struct type_Qmgr_BindError *) smalloc (sizeof *be);
	be -> reason = type;
	be -> information = NULL;

	cb -> cb_fd = NOTOK;
	freecblk (cb);
	if (encode_Qmgr_BindError (pep, 1, NULLCP, 0, be) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Failed to build BindError [%s]", PY_pepy));
		*pep = NULLPE;
		*npe = 0;
		free_Qmgr_BindError (be);
		return ACS_PERMANENT;
	}
	(*pep) -> pe_context = 3;
	*npe = 1;
	free_Qmgr_BindError (be);
	return ACS_PERMANENT;
}

int add_msg (sd, msg, rox, roi)
int sd;
struct type_Qmgr_MsgStruct *msg;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	MsgStruct *ms, *oldms;
	int	result;

	PP_TRACE (("add_msg (%d, msg, rox, roi)", sd));

	if (submission_disabled == 1)	/* not submitting right now */
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);

	if ((ms = newmsgstruct (msg)) == NULLMS)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);

	if (ms -> inchan && ms -> inchan -> ch_chan_type == CH_IN) {
		Chanlist *clp;

		if ((clp = findchanlist (ms -> inchan)) &&
		    clp -> lastsuccess < ms -> age)
			clp -> lastsuccess = ms -> age;
	}

	if ((oldms = find_msg (ms -> queid)) != NULLMS) {
		result = updatemessage (oldms, ms);
		freems (ms);
	}
	else
		result = insertmessage (ms);

	if (result == OK)
		return RyDsResult (sd, rox -> rox_id, (caddr_t) NULL,
				   ROS_NOPRIO, roi);
	else
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
}

/* ARGSUSED */
int updatemessage (old, new)
MsgStruct *old, *new;
{
	PP_TRACE (("updatemessage ()"));
	/* to be written !! */
	PP_NOTICE (("update on message %s - ignored", new -> queid));

	return OK;
}

int insertmessage (ms)
MsgStruct *ms;
{
	MsgStruct **msp;
	

	PP_TRACE (("insertmessage ()"));

	for (msp = &msg_hash[hash(ms -> queid, QUE_HASHSIZE)]; *msp;
	     msp = &(*msp) -> ms_forw)
		continue;

	(*msp) = ms;

	switch (chaninsert (ms)) {
	    case NOTOK: /* back out change */
		freems (ms);
		(*msp) = NULLMS;
		return NOTOK;
	    case OK: /* normal */
		stats.n_messages_in ++;
		break;
	    case DONE: /* deletion */
		break;
	}
	return OK;
}

/* ARGSUSED */
int	read_msg (sd, rma, rox, roi)
int	sd;
struct type_Qmgr_ReadMessageArgument *rma;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	MsgStruct **msp, *ms;
	struct type_Qmgr_MsgStructList **mlp;
	struct type_Qmgr_MsgList *base;
	Filter	*filter;

	PP_TRACE (("read_msg (%d)", sd));

	if ((base = (struct type_Qmgr_MsgList *) calloc (1, sizeof *base))
	    == NULL)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
	mlp = &base -> msgs;

	filter = newfilter (rma -> filters);

	for (msp = msg_hash; msp < &msg_hash[QUE_HASHSIZE]; msp++) {
		for (ms = *msp; ms; ms = ms -> ms_forw) {
			if (filtermatch (filter, ms)) {
				*mlp = (struct type_Qmgr_MsgStructList *)
					calloc (1, sizeof (**mlp));
				
				(*mlp) -> MsgStruct = lmessage (ms);
				mlp = &(*mlp) -> next;
			}
		}
	}
	freefilter (filter);

	if (RyDsResult (sd, rox -> rox_id, (caddr_t) base, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	free_Qmgr_MsgList (base);

	return OK;
}

/* ARGSUSED */
int	channel_list (sd, arg, rox, roi)
int	sd;
struct type_UNIV_UTCTime *arg;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	struct type_Qmgr_ChannelReadResult *crr;
	struct type_Qmgr_PrioritisedChannelList **list;
	Chanlist	**clp;

	PP_TRACE (("channel_list (%d)", sd));

	crr = (struct type_Qmgr_ChannelReadResult *) smalloc (sizeof *crr);
	list = &crr -> channels;
	crr -> channels = NULL;
	crr -> load1 = 100 * stats.runnable_chans;
	crr -> load2 = 100 * stats.ops_sec;
	crr -> currchans = nchansrunning;
	crr -> maxchans = maxchansrunning;

	for (clp = chan_list; clp < &chan_list[nchanlist]; clp ++) {
		*list = (struct type_Qmgr_PrioritisedChannelList *)
			calloc (1, sizeof (**list));

		if (*list == NULL ||
		    ((*list) -> PrioritisedChannel = lpchan (*clp)) == NULL) {
			free_Qmgr_ChannelReadResult (crr);
			return error (sd, error_Qmgr_congested, (caddr_t) NULL,
				      rox, roi);
		}

		list = &((*list) -> next);
	}

	if (RyDsResult (sd, rox -> rox_id, (caddr_t) crr, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");

	free_Qmgr_ChannelReadResult (crr);
	return OK;
}

int chan_control (sd, in, rox, roi)
int	sd;
struct type_Qmgr_ChannelControl *in;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	Chanlist *clp;
	struct type_Qmgr_PrioritisedChannelList *pcl;
	struct type_Qmgr_PrioritisedChannel *pc;
	Connblk *cb;
	char	*p;

	PP_TRACE (("chan_control (%d)", sd));

	if ((cb = findcblk (sd)) == NULLCB || cb -> cb_type != cb_responder)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
	if (cb -> cb_authenticated == 0)
		return error (sd, error_Qmgr_authenticationFailure,
			      (caddr_t) NULL, rox, roi);

	p = qb2str (in -> channel);
	clp = findchanlist (ch_nm2struct (p));
	free (p);
	if (clp == NULLCHANLIST)
		return error (sd, error_Qmgr_noSuchChannel, (caddr_t) NULL,
			      rox, roi);

	switch (in -> control -> offset) {
	    case type_Qmgr_Control_stop:
		clp -> chan_enabled = 0;
		clp -> chan_syssusp = 0;
		break;

	    case type_Qmgr_Control_start:
		if (clp -> chan_enabled == 0)
			clp -> chan_update = 1;
		clp -> chan_enabled = 1;
		clp -> chan_syssusp = 0;
		break;

	    case type_Qmgr_Control_cacheClear:
		cache_clear (&clp -> cache);
		clp -> chan_update = 1;
		break;

	    case type_Qmgr_Control_cacheAdd:
		cache_set (&clp -> cache, in -> control -> un.cacheAdd);
		clp -> nextevent = clp -> cache.cachetime;
		break;

	    default:
		return error (sd, error_Qmgr_illegalOperation, (caddr_t) NULL,
			      rox, roi);
	}

	pcl = (struct type_Qmgr_PrioritisedChannelList *) malloc (sizeof *pcl);

	if ((pc = lpchan (clp)) == NULL)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);

	pcl -> PrioritisedChannel = pc;
	pcl -> next = NULL;
	if (RyDsResult (sd, rox -> rox_id, (caddr_t) pcl, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	free_Qmgr_PrioritisedChannelList (pcl);
	return OK;
}

/*ARGSUSED */
int	chan_begin (sd, arg, rox, roi)
int	sd;
struct type_Qmgr_FilterList *arg;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	struct type_Qmgr_FilterList *out;

	PP_TRACE (("chan_begin (%d)", sd));

	out = (struct type_Qmgr_FilterList *) calloc (1, sizeof(*out));
	if (out == NULL || (out -> Filter = (struct type_Qmgr_Filter *)
			    calloc (1, sizeof (struct type_Qmgr_Filter)))
	    == NULL)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);

	if (RyDsResult (sd, rox -> rox_id, (caddr_t) out, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	free_Qmgr_FilterList (out);
	return OK;
}

int	mta_control (sd, arg, rox, roi)
int	sd;
struct type_Qmgr_MtaControl *arg;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	Chanlist	*clp = NULLCHANLIST;
	Mtalist	       *mlp;
	struct type_Qmgr_MtaInfo *mta;
	CHAN	*chan;
	Connblk *cb;
	char	*p;

	PP_TRACE (("mta_control (%d)", sd));

	if ((cb = findcblk (sd)) == NULLCB || cb -> cb_type != cb_responder)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
	if (cb -> cb_authenticated == 0)
		return error (sd, error_Qmgr_authenticationFailure,
			      (caddr_t) NULL, rox, roi);

	p = qb2str (arg -> channel);
	if ((chan = ch_nm2struct (p)) != NULLCHAN)
		clp = findchanlist (chan);
	free (p);

	if (clp == NULLCHANLIST)
		return error (sd, error_Qmgr_noSuchChannel, (caddr_t) NULL,
			      rox, roi);
	p = qb2str (arg -> mta);
	mlp = findmtalist (clp, p);
	free(p);

	if (mlp == NULLMTALIST)
		return error (sd, error_Qmgr_mtaNotInQueue, (caddr_t) NULL,
			      rox, roi);

	switch (arg -> control -> offset) {
	    case type_Qmgr_Control_stop:
		mlp -> mta_enabled = 0;
		break;
	    case type_Qmgr_Control_start:
		if (mlp -> mta_enabled == 0) {
			mlp -> mta_changed = 1;
			clp -> chan_update = 1;
		}
		mlp -> mta_enabled = 1;
		break;
	    case type_Qmgr_Control_cacheClear:
		cache_clear (&mlp -> cache);
		mlp -> mta_changed = 1;
		clp -> chan_update = 1;
		remque (mlp);
		insque (mlp, clp -> mtas -> mta_forw);
		break;
	    case type_Qmgr_Control_cacheAdd:
		cache_set (&mlp -> cache, arg -> control -> un.cacheAdd);
		mlp -> mta_changed = 1;
		clp -> chan_update = 1;
		remque (mlp);
		insque (mlp, clp -> mtas -> mta_back);
		break;
	    default:
		return error (sd, error_Qmgr_illegalOperation, (caddr_t) NULL,
			      rox, roi);
	}

	if ((mta = lmta (mlp, clp)) == NULL)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
	if (RyDsResult (sd, rox -> rox_id, (caddr_t) mta, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	free_Qmgr_MtaInfo (mta);
	return OK;
}

int	mta_read (sd, arg, rox, roi)
int	sd;
struct type_Qmgr_MtaRead *arg;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	Chanlist *clp;
	struct type_Qmgr_PrioritisedMtaList **qmlpp,  *qmlp;
	char	*p;
	Mtalist *mlp;

	PP_TRACE (("mta_read (%d)", sd));

	qmlp = 0;
	qmlpp = &qmlp;

	p = qb2str (arg -> channel);
	clp = findchanlist (ch_nm2struct (p));
	free (p);
	if (clp == NULLCHANLIST)
		return error (sd, error_Qmgr_noSuchChannel, (caddr_t) NULL,
			      rox, roi);

	for (mlp = clp -> mtas -> mta_forw; mlp != clp -> mtas;
	     mlp = mlp -> mta_forw) {
		*qmlpp = (struct type_Qmgr_PrioritisedMtaList *)
			calloc (1, sizeof(**qmlpp));

		if (*qmlpp == NULL ||
		    ((*qmlpp) -> PrioritisedMta = lpmta (mlp, clp)) == NULL) {
			free_Qmgr_PrioritisedMtaList (qmlp);
			return error (sd, error_Qmgr_congested, (caddr_t) NULL,
				      rox, roi);
		}

		qmlpp = &((*qmlpp) -> next);
	}

	if (RyDsResult (sd, rox -> rox_id, (caddr_t) qmlp, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	free_Qmgr_PrioritisedMtaList (qmlp);
	return OK;
}

int msg_control (sd, arg, rox, roi)
int	sd;
struct type_Qmgr_MsgControl *arg;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	char	*q;
	Reciplist *rl;
	struct type_Qmgr_UserList *ul;
	Connblk *cb;
	MsgStruct *ms;

	PP_TRACE (("msg_control (%d)", sd));
	if ((cb = findcblk (sd)) == NULLCB || cb -> cb_type != cb_responder)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
	if (cb -> cb_authenticated == 0)
		return error (sd, error_Qmgr_authenticationFailure,
			      (caddr_t) NULL, rox, roi);

	q = qb2str (arg -> qid);
	if ((ms = find_msg (q)) == NULLMS)
		return error  (sd, error_Qmgr_noSuchChannel, (caddr_t)NULL,
			       rox, roi);
	for (rl = ms -> recips; rl; rl = rl -> rp_next) {
		for (ul = arg -> users; ul; ul = ul -> next) {
			if (ul -> RecipientId -> parm == rl -> id)
				mcontrol (ms, rl, arg -> control);
		}
	}
	if (RyDsResult (sd, rox -> rox_id, (caddr_t) NULL, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	return OK;
}

static void mcontrol (ms, rl, ctrl)
MsgStruct *ms;
Reciplist	*rl;
struct type_Qmgr_Control *ctrl;
{
	LIST_RCHAN	*cl;
	char	*p;
	Chanlist *clp = NULL;
	Mlist *ml;
	Mtalist *mlp;

	PP_TRACE (("mcontrol (%s, %d)", ms -> queid, rl -> id));

	switch (rl -> status) {
	    case ST_NORMAL:

		if ((cl = rl -> cchan) == NULL)
			return;
		clp = findchanlist (cl -> li_chan);
		p = chan2mta (cl -> li_chan, rl);
		break;

	    case ST_DR:
		{
			Reciplist *rlp2;

			for (rlp2 = ms -> recips; rlp2; rlp2 = rlp2 -> rp_next)
				if (rlp2 -> id == 0)
					break;
			if (rlp2 == NULL)
				return;
			clp = findchanlist(rlp2 -> chans->li_chan);
			
			p = chan2mta (clp -> chan, rlp2);
			break;
		}

	    case ST_DELETE:
		clp = delete_chan;
		p = chan2mta (delete_chan -> chan, rl);
		break;

	    case ST_TIMEOUT:
		clp = timeout_chan;
		p = chan2mta (timeout_chan -> chan, rl);
		break;

	    case ST_WARNING:
		clp = warn_chan;
		p = chan2mta (warn_chan -> chan, rl);
		break;

	    default:
		return;

	}
	if (clp == NULLCHANLIST)
		return;

	if ((mlp = findmtalist (clp, p)) == NULLMTALIST)
		return;

	for (ml = mlp -> msgs -> ml_forw; ml != mlp -> msgs;
	     ml = ml -> ml_forw) {
		if (ml -> ms == ms)
			break;
	}
	if (ml == mlp -> msgs)
		return;

	switch (ctrl -> offset) {
	    case type_Qmgr_Control_stop:
		rl -> msg_enabled = 0;
		break;
	    case type_Qmgr_Control_start:
		rl -> msg_enabled = 1;
		break;
	    case type_Qmgr_Control_cacheClear:
		cache_clear (&rl -> cache);
		break;
	    case type_Qmgr_Control_cacheAdd:
		cache_set (&rl -> cache, ctrl -> un.cacheAdd);
		break;
	}
	mlp -> mta_changed = 1;
	clp -> chan_update = 1;
}

int qmgrcontrol (sd, op, rox, roi)
int	sd;
int	op;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	Connblk *cb;

	PP_TRACE (("qmgrcontrol (%d)", sd));

	if ((cb = findcblk (sd)) == NULLCB || cb -> cb_type != cb_responder)
		return error (sd, error_Qmgr_congested, (caddr_t) NULL,
			      rox, roi);
	if (cb -> cb_authenticated == 0)
		return error (sd, error_Qmgr_authenticationFailure,
			      (caddr_t) NULL, rox, roi);
	switch (op) {
	    case int_Qmgr_QMGROp_abort:
		exit (0);
		break;

	    case int_Qmgr_QMGROp_gracefulTerminate:
		setchannels (1);
		opmode = OP_SHUTDOWN;
		break;

	    case int_Qmgr_QMGROp_restart:
		setchannels (0);
		opmode = OP_RESTART;
		break;

	    case int_Qmgr_QMGROp_rereadQueue:
		if (loader_chan) {
			cache_clear (&loader_chan -> cache);
			loader_chan -> nextevent = current_time;
		}
		break;

	    case int_Qmgr_QMGROp_disableSubmission:
		submission_disabled = 1;
		break;

	    case int_Qmgr_QMGROp_enableSubmission:
		submission_disabled = 0;
		if (loader_chan) {
			cache_clear (&loader_chan -> cache);
			loader_chan -> nextevent = current_time;
		}
		break;

	    case int_Qmgr_QMGROp_disableAll:
	    case int_Qmgr_QMGROp_enableAll:
		setchannels (op == int_Qmgr_QMGROp_enableAll ? 1 : 0);
		break;

	    case int_Qmgr_QMGROp_increasemaxchans:
		maxchansrunning ++;
		break;

	    case int_Qmgr_QMGROp_decreasemaxchans:
		maxchansrunning --;
		maxchansrunning = max (1, maxchansrunning);
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS,  ("Unknown operation %d", op));
		break;
	}
	if (RyDsResult (sd, rox -> rox_id, (caddr_t) NULL, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	return OK;
}

int msgread (sd, mr, rox, roi)
int	sd;
struct type_Qmgr_MsgRead *mr;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	struct type_Qmgr_MsgStructList **qmlpp;
	struct type_Qmgr_MsgList *base;
	Mtalist *mlp;
	Chanlist *clp;
	Mlist *ml;
	char	*p;

	PP_TRACE (("msgread (%d)", sd));

	p = qb2str (mr -> channel);
	clp = findchanlist (ch_nm2struct (p));
	free (p);
	if (clp == NULLCHANLIST)
		return error (sd, error_Qmgr_noSuchChannel, (caddr_t) NULL,
			      rox, roi);

	p = qb2str (mr -> mta);
	mlp = findmtalist (clp, p);
	free (p);

	if (mlp == NULLMTALIST)
		return error (sd, error_Qmgr_mtaNotInQueue, (caddr_t) NULL,
			      rox, roi);

	base = (struct type_Qmgr_MsgList *) calloc (1, sizeof *base);
	qmlpp = &base -> msgs;
	
	for (ml = mlp -> msgs -> ml_forw; ml != mlp -> msgs;
	     ml = ml -> ml_forw) {
		*qmlpp = (struct type_Qmgr_MsgStructList *)
			calloc (1, sizeof **qmlpp);
		(*qmlpp) -> MsgStruct = l_cm_message (ml);
		qmlpp = &(*qmlpp) -> next;
	}

	if (RyDsResult (sd, rox -> rox_id, (caddr_t)base, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	free_Qmgr_MsgList (base);

	return OK;
}

int qmgrstatus (sd, rox, roi)
int	sd;
struct RoSAPinvoke *rox;
struct RoSAPindication *roi;
{
	struct type_Qmgr_QmgrStatus *qs;
	int i;

	PP_TRACE (("qmgrstatus (%d)", sd));

	if ((qs = (struct type_Qmgr_QmgrStatus *)malloc (sizeof *qs)) == NULL)
		return error (sd, error_Qmgr_congested, (caddr_t)NULL,
			      rox, roi);

	qs -> boottime = ltime(stats.boottime);
	qs -> messagesIn = stats.n_messages_in;
	qs -> messagesOut = stats.n_messages_out;
	qs -> addrIn = stats.n_addr_in;
	qs -> addrOut = stats.n_addr_out;
	qs -> opsPerSec = stats.ops_sec * 100.0;
	qs -> runnableChans = stats.runnable_chans * 100.0;
	qs -> msgsInPerSec = stats.msg_sec_in * 100.0;;
	qs -> msgsOutPerSec = stats.msg_sec_out * 100.0;
	qs -> maxChans = maxchansrunning;
	qs -> currChans = nchansrunning;
	qs -> totalMsgs = qs -> totalVolume = qs -> totalDrs = 0;
	for (i = 0; i < nchanlist; i++) {
		qs -> totalMsgs += chan_list[i] -> num_msgs;
		qs -> totalVolume += chan_list[i] -> volume;
		qs -> totalDrs += chan_list[i] -> num_drs;
	}
	
	if (RyDsResult (sd, rox -> rox_id, (caddr_t)qs, ROS_NOPRIO, roi)
	    == NOTOK)
		ros_adios (&roi -> roi_preject, "RESULT");
	free_Qmgr_QmgrStatus (qs);

	return OK;
}

/* private functions */

/* Channel functions */

static int	chaninsert (ms)
MsgStruct *ms;
{
	LIST_RCHAN	*clp;
	Reciplist *rlp;
	int	num = 0;

	PP_TRACE (("chaninsert ()"));

	for (rlp = ms -> recips; rlp; rlp = rlp -> rp_next) {

		if ( rlp -> id == 0)
			continue;
		num ++;
		rlp -> status = ST_NORMAL;
		if ((clp = rlp -> cchan) == NULL) {
			insertindrchan (ms, rlp);
			PP_NOTICE (("Message %s to %s added - DR",
				    ms -> queid, rlp -> user));
			continue;
		}

		insertinchan (clp -> li_chan, ms, rlp,
			      chan2mta (clp -> li_chan, rlp));
		PP_NOTICE (("Message %s to %s added",
			    ms -> queid, rlp -> user));
	}
	if (num == 0) {
		ms -> recips -> cchan = NULL;
		insertindelchan (ms);
		PP_NOTICE (("Message %s added - for deletion", ms -> queid));
		return DONE;
	}
	else
		stats.n_addr_in += num;
	return OK;
}

static void add_mta_hash (clp, mlp)
Chanlist *clp;
Mtalist *mlp;
{
	int idx;

	idx = hash(mlp -> mtaname,MTA_HASHSIZE);
	mlp -> hash_next = clp -> mta_hash[idx];
	clp -> mta_hash[idx] = mlp;
}


void insertinchan (chan, ms, ri, mta)
CHAN *chan;
MsgStruct *ms;
Reciplist *ri;
char	*mta;
{
	Mtalist	       *mlp;
	Chanlist	*clp;

	PP_TRACE (("insertinchan (%s, %s, id=%d, mta=%s)", chan -> ch_name,
		   ms -> queid, ri -> id, mta));

	clp = findchanlist (chan);
	if (clp == NULLCHANLIST) {
		PP_LOG (LLOG_EXCEPTIONS, ("No channel for %s",
					  chan -> ch_name));
		return;
	}
	if ((mlp = findmtalist(clp, mta)) == NULLMTALIST) {
		mlp = (Mtalist *) calloc (1, sizeof *mlp);
		mlp -> msgs = (Mlist *) calloc (1, sizeof *mlp -> msgs);
		mlp -> msgs -> ml_back = mlp -> msgs -> ml_forw = mlp -> msgs;
		insque (mlp, clp -> mtas -> mta_back);
		mlp -> mtaname = strdup (mta);
		mlp -> mta_enabled = 1;
		mlp -> nextevent = current_time;
		mlp -> clp = clp;
		clp -> nmtas ++;
		if (clp -> mta_hash)
			add_mta_hash (clp, mlp);
			
		if (clp -> nmtas == 1)
			addtorunq (clp);
	}
	if (insertinmta (mlp, ms, ri, clp) == OK ) {
		if (ri -> status == ST_DR)
			clp -> num_drs ++;
		else
			clp -> num_msgs ++;
		clp -> volume += ms -> size;
		if (mlp -> nextevent < clp -> nextevent &&
		    clp -> cache.cachetime < current_time)
			clp -> nextevent = mlp -> nextevent;
		if (clp -> oldest == 0 || ms -> age < clp -> oldest)
			clp -> oldest = ms -> age;
		if (mlp -> oldest == 0 || ms -> age < mlp -> oldest)
			mlp -> oldest = ms -> age;
		
	}
}

void insertindelchan (ms)
MsgStruct *ms;
{
	Reciplist *ri, *ri2;

	PP_TRACE (("insertindelchan ()"));

	if (delete_chan == NULLCHANLIST) {
		PP_LOG(LLOG_EXCEPTIONS, ("No delete channel!"));
		return;
	}

	ri = ms -> recips;
	ri -> status = ST_DELETE;
	ri2 = ri -> rp_next ? ri -> rp_next : ri;
	insertinchan (delete_chan -> chan, ms, ri, 
		      chan2mta(delete_chan -> chan, ri2));
}

void insertindrchan (ms, rlp)
MsgStruct *ms;
Reciplist *rlp;
{
	Reciplist *r;
	LIST_RCHAN	*cp;

	PP_TRACE (("insertindrchan ()"));

	rlp -> status = ST_DR;
	for (r = ms -> recips; r; r = r -> rp_next)
		if (r -> id == 0)
			break;
	if (r == NULLRL)
		return;
	if ((cp = r -> cchan) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, ("DR chan appears to have finished"));
		return;
	}
	insertinchan (cp -> li_chan, ms, rlp, chan2mta (cp -> li_chan, r));
}

Chanlist *findchanlist (chan)
CHAN	*chan;
{
	Chanlist	**clpp;

	if (chan == NULLCHAN)
		return NULLCHANLIST;

	for (clpp = chan_list; clpp < &chan_list[nchanlist]; clpp++)
		if ((*clpp) -> chan == chan)
			return *clpp;

	return NULLCHANLIST;
}

static void	setchannels (state)
int	state;
{
	Chanlist **cpp;

	for (cpp = chan_list; cpp < &chan_list[nchanlist]; cpp++) {
		(*cpp) -> chan_enabled = state;
		(*cpp) -> chan_update = 1;
	}
}



static int chan_sort (clp1, clp2)
Chanlist **clp1, **clp2;
{
	switch ((*clp1) -> chan -> ch_chan_type) {
	    case CH_WARNING:
	    case CH_DELETE:
	    case CH_QMGR_LOAD:
	    case CH_DEBRIS:
	    case CH_TIMEOUT:
	    case CH_SHAPER:
		switch ((*clp2) -> chan -> ch_chan_type) {
		    case CH_BOTH:
		    case CH_OUT:
		    case CH_IN:
			return -1;

		    default:
			break;
		}
		return 0;

	    case CH_IN:
		switch ((*clp2) -> chan -> ch_chan_type) {
		    case CH_IN:
			return 0;

		    default:
			break;
		}
		return 1;

	    case CH_BOTH:
	    case CH_OUT:
		switch ((*clp2) -> chan -> ch_chan_type) {
		    case CH_IN:
			return -1;
		    case CH_OUT:
		    case CH_BOTH:
			return ((*clp1) -> averaget -
				(*clp2) -> averaget) * 100.0;
		    default:
			break;
		}
		return 1;
	}
	return 0;
}

void sort_chans ()
{
	int i;

	if (nchanlist)
		qsort ((char *)chan_list, nchanlist,
		       sizeof (Chanlist *), (IFP)chan_sort);
	PP_TRACE (("Channels resorted"));
	for (i = 0; i < nchanlist; i++)
		PP_TRACE (("Channel %d - %s (%g)", i,
			    chan_list[i] -> channame,
			    chan_list[i] -> averaget));
}


int delfromchan (clp, mlp, ml, rno)
Chanlist *clp;
Mtalist *mlp;
Mlist	*ml;
int	rno;
{
	int	i;
	int st = ST_NORMAL;

	PP_TRACE (("delfromchan (chan=%s, id=%d)",
		clp -> channame, rno));

	msg_unlock (ml -> ms);

	for (i = 0; i < ml -> rcount; i++)
		if (rno == ml -> recips[i] -> id)
			break;
	if (i < ml -> rcount) {
		st = ml -> recips[i] -> status;
		ml -> recips[i] -> ml = NULLMLIST;
		for (i++; i < ml -> rcount; i++)
			ml -> recips[i-1] = ml -> recips[i];
	}
	else
		PP_LOG (LLOG_EXCEPTIONS, ("can't locate correct recipient"));
	ml -> rcount --;
	if (ml -> rcount == 0) {
		ml -> recips = NULL;
		return zapmtamsg (ml, mlp, clp, st);
	}
	return 0;
}

/* Host functions */

static int csort_prio (ml1, ml2)
Mlist	*ml1, *ml2;
{
	if (ml1 -> ms -> priority == ml2 -> ms -> priority)
		return 0;
	if (ml1 -> ms -> priority == int_Qmgr_Priority_high)
		return 1;
	if (ml2 -> ms -> priority == int_Qmgr_Priority_high)
		return -1;
	if (ml1 -> ms -> priority == int_Qmgr_Priority_normal)
		return 1;
	if (ml1 -> ms -> priority == int_Qmgr_Priority_normal)
		return -1;
	return 0;
}

static int csort_time (ml1, ml2)
Mlist	*ml1, *ml2;
{
	if (ml1 -> ms -> age < ml2 -> ms -> age)
		return 1;
	if (ml2 -> ms -> age < ml1 -> ms -> age)
		return -1;
	return 0;
}

static int csort_size (ml1, ml2)
Mlist	*ml1, *ml2;
{
	if (ml1 -> ms -> size < ml2 -> ms -> size)
		return 1;
	if (ml2 -> ms -> size > ml1 -> ms -> size)
		return -1;
	return 0;
}

static mtaenqueue (ml, mlp, clp)
Mlist	*ml;
Mtalist  *mlp;
Chanlist *clp;
{
	int	i, msort;
	Mlist	*m;
	IFP	sfnx[CH_MAX_SORT - 1];

	if (clp -> chan -> ch_sort[1] == 0) {
		insque (ml, mlp -> msgs -> ml_back);
		return;
	}

	msort = 0;
	for (i = 1; i < CH_MAX_SORT && clp -> chan -> ch_sort[i]; i++) {
		switch (clp -> chan -> ch_sort[i]) {
		    case CH_SORT_USR:
		    case CH_SORT_MTA:
			PP_LOG (LLOG_EXCEPTIONS, ("Bad sort criteria for %s",
						  clp -> channame));
			insque (ml, mlp -> msgs -> ml_back);
			return;

		    case CH_SORT_NONE:
			break;

		    case CH_SORT_PRIORITY:
			sfnx[msort++] = csort_prio;
			continue;
		    case CH_SORT_TIME:
			sfnx[msort++] = csort_time;
			continue;
		    case CH_SORT_SIZE:
			sfnx[msort++] = csort_size;
			continue;

		    default:
			break;
		}
		break;
	}
	for (m = mlp -> msgs -> ml_forw; m != mlp -> msgs; m = m -> ml_forw) {

		for (i = 0; i < msort; i++) {
			switch ((*sfnx[i])(m, ml)) {
			    case 0:
				continue;
			    case -1:
				insque (ml, m -> ml_back);
				return;

			    case 1:
				break;
			}
			break;
		}
	}
	insque (ml, mlp -> msgs -> ml_back);
}


static int insertinmta (mlp, ms, rlp, clp)
Mtalist	       *mlp;
MsgStruct *ms;
Reciplist *rlp;
Chanlist *clp;
{
	Mlist	*ml;
	int	result;

	PP_TRACE (("insertinmta (%s, id=%d)",
		   mlp -> mtaname, rlp -> id));

	if (mlp -> lastone && mlp -> lastone -> ms == ms) {
		PP_TRACE (("Lastone hit"));
		(void) addmtaid (mlp -> lastone, rlp);
		return DONE;
	}
	if (rlp -> status != ST_DR) {
		for (ml = mlp -> msgs -> ml_forw; ml != mlp -> msgs;
		     ml = ml -> ml_forw) {
			if (ml -> ms -> m_locked)
				continue;
			if (ml -> ms == ms) {
				(void) addmtaid (ml, rlp);
				mlp -> lastone = ml;
				return DONE;
			}
		}
	}
	ml = (Mlist *) calloc (1, sizeof *ml);
	ml -> ms = ms;
	ml -> mlp = mlp;

	ms -> count ++;
	PP_TRACE (("Message reference count = %d", ms -> count));

	mtaenqueue (ml, mlp, clp);

	if ((result = addmtaid (ml, rlp)) == OK) {
		if (rlp -> status == ST_DR)
			mlp -> num_drs ++;
		else
			mlp -> num_msgs ++;
		mlp -> volume += ms -> size;
		if (mtaready(mlp))
			mlp -> nextevent = current_time;
	}
	mlp -> lastone = ml;
	return result;
}

static int addmtaid (ml, rlp)
Mlist	*ml;
Reciplist *rlp;
{
	Reciplist **ri;

	PP_TRACE (("addmtaid (%d)", rlp -> id));

	for (ri = ml -> recips; ri && ri < &ml->recips[ml->rcount];
	     ri ++)
		if (*ri == rlp)
			return DONE;

	if (ml -> rcount == 0) {
		ml -> recips = (Reciplist **) malloc (sizeof (ri));
		ml -> recips[0] = rlp;
		rlp -> ml = ml;
		ml -> rcount = 1;
		return OK;
	}

	ml -> rcount ++;
	ml -> recips = (Reciplist **) realloc ((char *)ml -> recips,
				 (unsigned) ml -> rcount * sizeof (ri));
	ml -> recips[ml -> rcount - 1] = rlp;
	rlp -> ml = ml;
	return DONE;
}

Mtalist *findmtalist (clp, mta)
Chanlist *clp;
char	*mta;
{
	Mtalist	       *mlp;

	PP_TRACE (("findmtalist (%s)", mta));

	if (clp -> mta_hash) {
		mlp = clp->mta_hash[hash(mta, MTA_HASHSIZE)];
		while (mlp) {
			if (lexequ (mta, mlp -> mtaname) == 0)
				return mlp;
			mlp = mlp -> hash_next;
		}
		return NULLMTALIST;
	}

	for (mlp = clp -> mtas -> mta_forw; mlp != clp -> mtas;
	     mlp = mlp -> mta_forw)
		if (lexequ (mta, mlp -> mtaname) == 0)
			return mlp;
	return NULLMTALIST;
}

int zapmta (mlp)
Mtalist	*mlp;
{
	Chanlist *clp;
	PP_TRACE (("zapmta (%s)", mlp -> mtaname));

	if (mlp -> num_msgs != 0 || mlp -> num_drs != 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("zapmta called but msg not dead"));
		return ZAP_NONE;
	}
	if ((clp = mlp -> clp) != NULLCHANLIST)
		clp -> nmtas --;
	else {
		PP_LOG (LLOG_EXCEPTIONS, ("mta does not have chan"));
		return ZAP_NONE;
	}
	if (clp -> nmtas < 0) {
		PP_LOG (LLOG_EXCEPTIONS, ("mtas gone negative on %s (%d)",
					  clp -> channame, clp -> nmtas));
		clp -> chan_update = 1;
	}
	if (clp -> mta_hash) {	/* remove from hash list */
		Mtalist **mlpp;

		for (mlpp = &clp -> mta_hash[hash(mlp->mtaname, MTA_HASHSIZE)];
		     *mlpp; mlpp = &(*mlpp) -> hash_next) {
			if (*mlpp == mlp) {
				*mlpp = mlp -> hash_next;
				break;
			}
		}
	}
	remque (mlp);

	if (mlp -> msgs)
		free ((char *) mlp -> msgs);
	if (mlp -> info)
		free (mlp -> info);
	if (mlp -> mtaname)
		free (mlp -> mtaname);
	free ((char *) mlp);
	if (clp && clp -> nmtas == 0) {
		delfromrunq (clp);
		clp -> oldest = 0;
	}
	return ZAP_MTA;
}

int zapmtamsg (ml, mlp, clp, st)
Mlist	*ml;
Mtalist	*mlp;
Chanlist	*clp;
int st;
{
	int result = ZAP_MSG;

	if (mlp == NULLMTALIST) {
		PP_LOG (LLOG_EXCEPTIONS, ("Null mtalist param"));
		return ZAP_NONE;
	}
	
	PP_TRACE (("zapmtamsg (mta=%s, chan=%s)",
		mlp -> mtaname, clp -> channame));

	if (st == ST_DR)
		clp -> num_drs --;
	else
		clp -> num_msgs --;
	clp -> volume -= ml -> ms -> size;
	if (ml -> ms -> age <= clp -> oldest)
		clp -> chan_update = 1;
	if (st == ST_DR)
		mlp -> num_drs --;
	else
		mlp -> num_msgs --;
	mlp -> volume -= ml -> ms -> size;
	if (ml -> ms -> age <= mlp -> oldest)
		mlp -> mta_changed = 1;

	remque (ml);
	ml -> ms -> count --;
	PP_TRACE (("Message reference count = %d",
		ml -> ms -> count));

	if (ml -> info)
		free (ml -> info);
	if (ml -> recips)
		free ((char *)ml -> recips);
	if (mlp -> lastone == ml)
		mlp -> lastone = NULL;
	free ((char *)ml);

	if (mlp -> num_msgs == 0 && mlp -> num_drs == 0)
		result |= zapmta (mlp);
	return result;
}

static Mlist *findnextml ();

/* Msg functions */

Mlist *nextmsg (clp, mlpp, lock, samemta)
Chanlist *clp;
Mtalist **mlpp;
int	lock;
int	samemta;
{
	Mlist	*ml;
	Mtalist *mlp;

	PP_TRACE (("nextmsg (clp, mlp)"));

	if ((mlp = *mlpp) != NULLMTALIST) {
		if (mlp -> mta_changed) {
			investigate_mta (mlp, current_time);
			mlp -> mta_changed = 0;
		}
		if (mlp -> error_count > 5 &&
		    clp -> chan -> ch_sort[0] == CH_SORT_MTA) {
			mlp -> error_count = 0;
			if (mlp -> cache.cachetime < current_time)
				cache_inc (&mlp -> cache, cache_time * 3 / 2);
			PP_NOTICE (("Avoiding MTA %s - too many errors",
				    mlp -> mtaname));
		}
		else if (mlp -> mta_enabled &&
		    mlp -> cache.cachetime <= current_time &&
		    (ml = findnextml (mlp, current_time, lock)) != NULL)
			return ml;
		mlp -> nactive --;
	}

	if (samemta) {
		*mlpp = NULL;
		return (Mlist *)0;
	}

	if (*mlpp) {
		PP_TRACE (("nothing on current MTA list - trying others"));
		*mlpp = NULLMTALIST;
	}

	for (mlp = clp -> mtas -> mta_forw; mlp != clp -> mtas;
	     mlp = mlp -> mta_forw) {
		if (mlp -> mta_changed) {
			investigate_mta (mlp, current_time);
			mlp -> mta_changed = 0;
		}
		if (! mtaready (mlp)) {
			PP_TRACE (("mta %s not ready", mlp -> mtaname));
			continue;
		}

		if (mlp -> cache.cachetime > current_time) {
			PP_TRACE (("Mta %s cached for %d seconds",
				   mlp -> mtaname,
				   mlp -> cache.cachetime - current_time));
			continue;
		}

		if ((ml = findnextml (mlp, current_time, lock)) != NULL) {
			if (lock)
				mlp -> nactive ++;
			*mlpp = mlp;
			remque (mlp);
			insque (mlp, clp -> mtas);
			PP_TRACE (("Found a ml"));
			return ml;
		}
		PP_TRACE (("Mta %s no messages ready", mlp -> mtaname));
	}
	return (Mlist *)0;
}

static Mlist *findnextml (mlp, now, lock)
Mtalist	*mlp;
time_t	now;
int	lock;
{
	Mlist	*ml;

	PP_TRACE (("findnextml (mta=%s)", mlp -> mtaname));

	for (ml = mlp -> msgs -> ml_forw; ml != mlp -> msgs;
	     ml = ml -> ml_forw) {
		if (!msgready (ml)) {
			PP_TRACE (("Message %s not ready", ml -> ms -> queid));
			continue;
		}

		if (msgiscached (ml, now)) {
			PP_TRACE (("Message %s cached", ml -> ms -> queid));
			continue;
		}

		if (ml -> ms -> defferedtime &&
		    ml -> ms -> defferedtime > now) {
			PP_TRACE (("Message defered"));
			continue;
		}

		if (lock)
			ml -> ms -> m_locked = 1;
		return ml;
	}
	return (Mlist *)0;
}

void zapmsg (ms)
MsgStruct *ms;
{
	MsgStruct **mspp;

	PP_TRACE (("zapmsg (%s)", ms -> queid));

	mspp = &msg_hash[hash(ms -> queid, QUE_HASHSIZE)];

	for (; *mspp; mspp = &(*mspp) -> ms_forw)
		if (*mspp == ms) {
			*mspp = (*mspp) -> ms_forw;
			freems (ms);
			PP_TRACE (("Message zapped"));
			return;
		}
	PP_LOG (LLOG_EXCEPTIONS, ("Can't locate message"));
}


Mlist	*findmtamsg (mlp, qid)
Mtalist	*mlp;
char *qid;
{
	MsgStruct *ms;
	Mlist	*ml;

	ms = find_msg (qid);

	for (ml = mlp -> msgs -> ml_forw; ml != mlp -> msgs;
	     ml = ml -> ml_forw)
		if (ml -> ms == ms)
			return ml;
	return NULL;
}

MsgStruct *find_msg (qid)
char *qid;
{
	MsgStruct	*ms;

	PP_TRACE (("find_msg (%s)", qid));
	for (ms = msg_hash[hash (qid, QUE_HASHSIZE)]; ms; ms = ms -> ms_forw)
		if ( strcmp (ms -> queid, qid) == 0)
			return ms;
	return NULLMS;
}

/* ARGSUSED */
void kill_msg (ms)
MsgStruct *ms;
{
	PP_TRACE (("kill_msg"));
}

static int filtermatch (f, ms)
Filter *f;
MsgStruct *ms;
{
	PP_TRACE (("filtermatch ()"));

	if (f == NULLFL)
		return 0;

	if (f -> cont) {
		if (lexequ (f -> cont, ms -> contenttype) != 0)
			return filtermatch (f -> next, ms);
	}

	if (f -> eit) {
		LIST_BPT *bp, *bp2;

		for (bp = ms -> eit; bp; bp = bp -> li_next) {
			for (bp2 = f -> eit; bp2; bp2 = bp2 -> li_next) {
				if (lexequ (bp -> li_name,
					    bp2 -> li_name) != 0)
					return filtermatch (f -> next, ms);
			}
		}
	}

	if (f -> orig) {
		if (lexequ (f -> orig, ms -> originator) != 0)
			return filtermatch (f -> next, ms);
	}
	
	if (f -> recip) {
		Reciplist *rl, *rl0;

		for (rl0 = ms -> recips; rl0; rl0 = rl0 -> rp_next)
			if (rl0 -> id == 0)
				break;

		for (rl = ms -> recips; rl; rl = rl -> rp_next) {
			switch (rl -> status) {
			    case ST_DELETE:
				continue; /* we ignore recips that dead */

			    case ST_TIMEOUT:
			    case ST_WARNING:
			    case ST_NORMAL:
				if (rl -> id == 0)
					continue;
				if (lexequ (rl -> user, f -> recip) == 0)
					break;
				continue;

			    case ST_DR:
				if (rl0 &&
				    lexequ (rl0 -> user, f -> recip) == 0)
					break;
				continue;
			    default:
				PP_LOG (LLOG_EXCEPTIONS,
					("filtermatch - Unknown state %d",
					 rl -> status));
				continue;
			}
			break;
		}
		if (rl == NULLRL)
			return filtermatch (f -> next, ms);
	}
	if (f -> channel) {
		Reciplist *rl, *rl0;

		for (rl0 = ms -> recips; rl0; rl0 = rl0 -> rp_next)
			if (rl0 -> id == 0)
				break;
		if (rl0 == NULLRL)
			return filtermatch (f -> next, ms);

		for (rl = ms -> recips; rl; rl = rl -> rp_next) {
			switch (rl -> status) {
			    case ST_DR:
				if (f -> channel == rl0 -> chans -> li_chan)
					break;
				continue;

			    case ST_DELETE:
				if (f -> channel == delete_chan -> chan)
					break;
				continue;

			    case ST_TIMEOUT:
				if (f -> channel == timeout_chan -> chan)
					break;
				continue;

			    case ST_WARNING:
				if (warn_chan &&
				    f -> channel == warn_chan -> chan)
					break;
				continue;

			    case ST_NORMAL:
				if (rl -> cchan &&
				    f -> channel == rl -> cchan -> li_chan)
					break;
				continue;
			    default:
				PP_LOG (LLOG_EXCEPTIONS,
					("filtermatch - unknown state %d",
					 rl -> status));
				break;
			}
		}
		if (rl == NULLRL)
			return filtermatch (f -> next, ms);
	}

	if (f -> mta) {
		Reciplist *rl;

		for (rl = ms -> recips; rl; rl = rl -> rp_next) {
			switch (rl -> status) {
			    case ST_DR:
			    case ST_NORMAL:
				if (lexequ (rl -> mta, f -> mta) == 0)
					break;
				if (rl -> ml && rl -> ml -> mlp &&
				    lexequ (rl -> ml -> mlp -> mtaname,
					    f -> mta) == 0)
					break;
				continue;

			    case ST_DELETE:
				if (delete_chan &&
				    lexequ (f -> mta,
					    delete_chan -> channame) == 0)
				    break;
				continue;
			    case ST_TIMEOUT:
				if (timeout_chan &&
				    lexequ (f -> mta,
					    timeout_chan -> channame) == 0)
					break;
				continue;
			    case ST_WARNING:
				if (warn_chan &&
				    lexequ (f -> mta,
					    warn_chan -> channame) == 0)
					break;
				continue;
			    default:
				PP_LOG (LLOG_EXCEPTIONS,
					("filtermatch - Bad state %d", rl -> status));
				continue;
			}
			break;
		}
		if (rl == NULLRL)
			return filtermatch (f -> next, ms);
	}

	if (f -> queid) {
		if (lexequ (f -> queid, ms -> queid) != 0)
			return filtermatch (f -> next, ms);
	}

	if (f -> mpduid) {
		if (!compmpduid (f -> mpduid, ms -> mpduid))
			return filtermatch (f -> next, ms);
	}
	if (f -> uacontent) {
		if (lexequ (f -> uacontent, ms -> uacontent) != 0)
			return filtermatch (f -> next, ms);
	}
	if (f ->morerecent) {
		if (ms -> age <= f -> morerecent)
			return filtermatch (f -> next, ms);
	}
	if (f -> earlier) {
		if (ms -> age >= f -> earlier)
			return filtermatch (f -> next, ms);
	}
	if (f -> priority != -1) {
		if (ms -> priority != f -> priority)
			return filtermatch (f -> next, ms);
	}
	if (f -> maxsize) {
		if (ms -> size > f -> maxsize)
			return filtermatch (f -> next, ms);

	}
	return 1;
}

static int compmpduid (m1, m2)
MPDUid *m1, *m2;
{
	PP_TRACE (("compmpduid ()"));

	if (!nullstrcmp (m1 -> mpduid_string, m2 -> mpduid_string))
		return 0;

	if (!nullstrcmp (m1 -> mpduid_DomId.global_Country,
			 m2 -> mpduid_DomId.global_Country))
		return 0;
	if (!nullstrcmp (m1 -> mpduid_DomId.global_Admin,
			 m2 -> mpduid_DomId.global_Admin))
		return 0;
	if (!nullstrcmp (m1 -> mpduid_DomId.global_Private,
			 m2 -> mpduid_DomId.global_Private))
		return 0;
	return 1;
}

static int nullstrcmp (s1, s2)
char	*s1, *s2;
{
	if (s1 && s2)
		return lexequ (s1, s2) == 0;
	if (s1 == NULLCP && s2 == NULLCP)
		return 1;
	return 0;
}

int mtaready (mlp)
Mtalist *mlp;
{
	if (mlp -> nactive > 0) {
		PP_TRACE (("mta %s started already",
			   mlp -> mtaname));
		return FALSE;
	}
	if (mlp -> mta_enabled == 0) {
		PP_TRACE (("Mta %s not enabled",
			   mlp -> mtaname));
		return FALSE;
	}
	return TRUE;
}

msgready (ml)
Mlist	*ml;
{
	Reciplist **rl;

	if (ml -> ms -> m_locked) {
		PP_TRACE (("Message locked"));
		return FALSE;
	}
	for (rl = ml -> recips; rl < &ml -> recips[ml->rcount]; rl ++) {
		if ((*rl) -> msg_enabled == 0) {
			PP_TRACE (("Message disabled"));
			return FALSE;
		}
	}
		
	return TRUE;
}

msgiscached (ml, now)
Mlist *ml;
time_t now;
{
	Reciplist **rl;

	for (rl = ml -> recips; rl < &ml -> recips[ml->rcount]; rl ++) {
		if ((*rl) -> cache.cachetime <= now)
			return FALSE;
	}
	return TRUE;
}

void clear_msgcache (ml)
Mlist *ml;
{
	Reciplist **rl;

	for (rl = ml -> recips; rl < &ml -> recips[ml->rcount]; rl ++) {
		cache_clear (&(*rl) -> cache);
	}
}

void msgcache_inc (ml, plus)
Mlist *ml;
time_t plus;
{
	Reciplist **rl;

	for (rl = ml -> recips; rl < &ml -> recips[ml->rcount]; rl ++) {
		cache_inc (&(*rl) -> cache, plus);
	}
}

time_t msgmincache (ml)
Mlist *ml;
{
	time_t minc = 0;
	int	flag = 0;
	Reciplist **rl;

	for (rl = ml -> recips; rl < &ml -> recips[ml->rcount]; rl ++) {
		if (!flag) {
			flag = 1;
			minc = (*rl) -> cache.cachetime;
		}
		else
			minc = minc < (*rl) -> cache.cachetime ? minc :
				(*rl) -> cache.cachetime;
	}
	return minc;
}

msg_unlock (ms)
MsgStruct *ms;
{
	Reciplist *rlp;
	Chanlist *clp;
	Mtalist *mlp;
	Mlist *ml;

	PP_TRACE (("msg_unlock"));
	ms -> m_locked = 0;

	for (rlp = ms -> recips; rlp; rlp = rlp -> rp_next) {

		if (rlp -> id == 0 ||
		    rlp -> msg_enabled == 0)
			continue;

		if ((ml = rlp -> ml) == NULLMLIST) {
			PP_TRACE (("recipient not on mlist!"));
			continue;
		}

		if ((mlp = ml -> mlp) == NULLMTALIST) {
			PP_LOG (LLOG_EXCEPTIONS, ("message not on MTA"));
			continue;
		}


		if (current_time > rlp -> cache.cachetime)
			rlp -> cache.cachetime = current_time;
		if (ml -> ms -> defferedtime > rlp -> cache.cachetime)
			rlp -> cache.cachetime = ml -> ms -> defferedtime;

		if (rlp -> cache.cachetime < mlp -> nextevent &&
		    mlp -> cache.cachetime < current_time)
			mlp -> nextevent = rlp -> cache.cachetime;

		if ((clp = mlp -> clp) == NULLCHANLIST) {
			PP_LOG (LLOG_EXCEPTIONS, ("mta not on channel!"));
			continue;
		}
		if (clp -> chan_enabled == 0)
			continue;
		if (rlp -> cache.cachetime < clp -> nextevent &&
		    clp -> cache.cachetime < current_time)
			clp -> nextevent = rlp -> cache.cachetime;
	}
}

char	*chan2mta (chan, rlp)
CHAN *chan;
Reciplist *rlp;
{
	if (chan -> ch_sort[0] == CH_SORT_NONE)
		return chan -> ch_name;
	else if (chan -> ch_sort[0] == CH_SORT_USR)
		return rlp -> user;
/* removed for JA@UCL
	else	if (chan -> ch_mta)
		return chan -> ch_mta;
*/
	else
		return rlp -> mta;
}
