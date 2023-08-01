/* ut_rts.c - X400 rts utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40088/RCS/ut_rts.c,v 6.0 1991/12/18 20:14:27 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/ut_rts.c,v 6.0 1991/12/18 20:14:27 jpo Rel $
 *
 * $Log: ut_rts.c,v $
 * Revision 6.0  1991/12/18  20:14:27  jpo
 * Release 6.0
 *
 */



#include <isode/rtsap.h>
#include "util.h"
#include "rtsparams.h"
#include "chan.h"
#include "Trans-types.h"

static char		*build_inkey();
static char		*pretty_key = NULLCP;       /* -- for logging only -- */
static void		print_remote_info();
static int		remote_info_not_found ();

char			*remote_site = NULLCP;
char			*undefined_bodyparts = NULLCP;



/* ---------------------  Begin	 Routines  -------------------------------- */




int rts_encode_request (ppe, request, mta, pass, isacs)
PE				*ppe;
struct type_Trans_Bind1988Argument **request;
char				*mta, *pass;
{
	struct type_Trans_Bind1988Argument	*req;

	PP_TRACE (("rts_encode_request (%s %s)", mta, pass));

	req = (struct type_Trans_Bind1988Argument *)
		calloc (1, sizeof (*req));
	if (req == NULL)  return NOTOK;


	if (mta == NULLCP)
		req -> offset = type_MTA_MTABindArgument_no__auth;
	else {
		req->un.auth =
			(struct member_MTA_11 *)
				calloc (1, sizeof (*req->un.auth));
		req->offset = type_MTA_MTABindArgument_auth;
		req -> un.auth -> initiator__name =
			str2qb (mta, strlen (mta), 1);
		req -> un.auth -> initiator__credentials =
			(struct type_MTA_InitiatorCredentials *)
				smalloc (sizeof (struct type_MTA_InitiatorCredentials));
		req -> un.auth -> initiator__credentials ->
			offset = type_MTA_InitiatorCredentials_octetstring;
		req -> un.auth -> initiator__credentials ->
			un.octetstring = str2qb (pass, strlen (pass), 1);
	}
	*request = req;

	if (isacs) {
		if (encode_Trans_Bind1988Result (ppe, 1, 0,
						    NULLCP, req) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't encode bind request: %s", PY_pepy));
			return NOTOK;
		}
	}
	else {
		if (encode_MTA_MTABindArgument (ppe, 1, 0,
						NULLCP, req) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't encode bind request: %s", PY_pepy));
			return NOTOK;
		}
	}

	PP_TRACE (("encode Bind Argument successful!"));

	return OK;
}

int rts_encode_response (ppe, result, mta, pass, isacs)
PE				*ppe;
struct type_Trans_Bind1988Result **result;
char				*mta, *pass;
{
	struct type_Trans_Bind1988Result	*res;

	PP_TRACE (("rts_encode_result (%s %s)", mta, pass));

	res = (struct type_Trans_Bind1988Result *)
		calloc (1, sizeof (*res));
	if (res == NULL)  return NOTOK;


	if (mta == NULLCP)
		res -> offset = type_MTA_MTABindResult_no__auth;
	else {
		res->un.auth =
			(struct member_MTA_12 *)
				calloc (1, sizeof (*res->un.auth));
		res->offset = type_MTA_MTABindResult_auth;
		res -> un.auth -> responder__name =
			str2qb (mta, strlen (mta), 1);
		res -> un.auth -> responder__credentials =
			(struct type_MTA_ResponderCredentials *)
				smalloc (sizeof (struct type_MTA_ResponderCredentials));
		res -> un.auth -> responder__credentials ->
			offset = type_MTA_ResponderCredentials_octetstring;
		res -> un.auth -> responder__credentials ->
			un.octetstring = str2qb (pass, strlen (pass), 1);
	}
	*result = res;

	if (isacs) {
		if (encode_Trans_Bind1988Result (ppe, 1, 0,
						    NULLCP, res) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't encode bind request: %s", PY_pepy));
			return NOTOK;
		}
	}
	else {
		if (encode_MTA_MTABindResult (ppe, 1, 0,
						NULLCP, res) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't encode bind request: %s", PY_pepy));
			return NOTOK;
		}
	}

	PP_TRACE (("encode Bind Argument successful!"));

	return OK;
}




int parameter_checks (pe, mta, pass, key, checks, isacs)
PE	pe;
char	*mta; 
char	*pass;
char	*key;
int	checks;
int	isacs;
{
	struct type_Trans_Bind1988Argument *ba;
	struct member_MTA_11 *rq;
	char				*remotesite = NULLCP;
	char				*password = NULLCP;
	int				retval = NOTOK;

	PP_TRACE (("parameter_checks('%s', '%s', '%s', '%s')",
		key ? key : "null", 
		mta ? mta : "null",
		pass ? pass : "null", 
		checks ? "true" : "false"));

	if (isacs) {
		if (decode_Trans_Bind1988Argument (pe, 0, NULLIP,
						      NULLVP, &ba) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't decode request: %s", PY_pepy));
			goto parameter_checks_free;
		}
	}
	else {
		if (decode_MTA_MTABindArgument (pe, 0, NULLIP,
						NULLVP, &ba) == NOTOK) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Can't decode request: %s", PY_pepy));
			goto parameter_checks_free;
		}
	}
	print_remote_info (key, ba);

	if (checks == FALSE) {
		retval = OK;
		goto parameter_checks_free;
	}

	if (ba->offset == type_MTA_MTABindArgument_no__auth) {
		PP_NOTICE (("parameter checks are not required"));
		retval = OK;
		goto parameter_checks_free;
	}

	rq = ba->un.auth;

	remotesite = qb2str (rq->initiator__name);

	switch (rq -> initiator__credentials -> offset) {
	    case type_MTA_InitiatorCredentials_octetstring:
		password = qb2str (rq -> initiator__credentials ->
				   un.octetstring);
		break;
	    case type_MTA_InitiatorCredentials_ia5string:
		password = qb2str (rq -> initiator__credentials ->
				   un.ia5string);
		break;
	    default:
		goto parameter_checks_free;
	}

	PP_TRACE (("parameter_checks MTA=%s, P=%s", remotesite, password));

	if (mta && lexequ (remotesite, mta) != 0) {
		PP_LOG (LLOG_EXCEPTIONS,
 			("MTAName mismatch: specified='%s' received='%s'", 
			mta, remotesite));
		goto parameter_checks_free;
	}


	if (lexequ (pass, "dflt") == 0) {
		retval = OK;
		goto parameter_checks_free;
	}


	if (pass && lexequ ((char *)password, pass) != 0) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("Password mismatch: specified='%s' received='%s'", 
			 pass, (char *)password));
		goto parameter_checks_free;
	}

	retval = OK;

parameter_checks_free: ;
	if (ba)		free_Trans_Bind1988Argument (ba);
	if (remotesite)		free (remotesite);
	if (password)		free ((char *)password);

	return retval;
}




/* -- Checks P1 User-data: mTAName and password -- */

rts_decode_request (rts, ppe, prq, chan, isacs)
struct RtSAPstart		*rts;
PE				*ppe;
struct type_Trans_Bind1988Result	**prq;
CHAN				*chan;
int	isacs;
{
	char		*key;
	RtsParams	*rp;
	int		checkit = TRUE;

	key = build_inkey (rts, isacs);

	PP_TRACE (("rts_decode_request (%s)", key));

	if ((rp = tb_rtsparams (chan -> ch_in_table, key)) == NULL) 
		return (remote_info_not_found (rts, ppe, prq, key, chan, isacs));

	/* -- Check that the right info is in the tables -- */
	if (rp->their_internal_ppname == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("No rname specified in the entry '%s'", key));
		return RTS_VALIDATE;
	}

	if (rp->their_name == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("No rmta specified in the entry '%s'", key));
		return RTS_VALIDATE;
	}

	if (lexequ (chan -> ch_in_info, "sloppy") == 0 
		|| lexequ (rp -> info_mode, "sloppy") == 0)
			checkit = FALSE; 

	if (parameter_checks (rts -> rts_data, rp -> their_name,
			      rp -> their_passwd, key, checkit, isacs) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Parameter check failed"));
		return RTS_VALIDATE;
	}

	/* -- encode new Connect Accept Data -- */
	rts_encode_response (ppe, prq, rp -> our_name, rp -> our_passwd, isacs);


	/* -- set the remote site value -- */
	if (remote_site)
		free (remote_site);
	if (rp -> their_internal_ppname == NULLCP) {
		PP_LOG (LLOG_EXCEPTIONS, ("No internal name for %s", key));
		remote_site = strdup (key);
	}
	else remote_site = strdup (rp -> their_internal_ppname);


	 /* --- *** ---
	 if "undefined" set in eit - replace with rp->info_undefined
	 --- *** --- */

	 if (undefined_bodyparts) {
		 free (undefined_bodyparts);
		 undefined_bodyparts = NULLCP;
	}
	if (rp -> info_undefined)
		undefined_bodyparts = strdup (rp -> info_undefined);

	RPfree (rp);
	return OK;
}



/* ---------------------  Static  Routines  ------------------------------- */





static int  remote_info_not_found (rts, ppe, prq, key, chan, isacs)
struct RtSAPstart		*rts;
PE				*ppe;
struct type_Trans_Bind1988Argument	**prq;
char				*key;
CHAN				*chan;
int	isacs;
{
	RtsParams		*rp;  /* --- contains default values --- */


	PP_TRACE (("remote_info_not_found()"));

	parameter_checks (rts -> rts_data, NULLCP, NULLCP, key, FALSE, isacs);

	if (lexequ (chan -> ch_in_info, "sloppy") != 0) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("No information found in the X.400 incoming tables"));
		return RTS_VALIDATE;
	}

	if ((rp = tb_rtsparams (chan -> ch_in_table, "default")) == NULL) {
		PP_LOG (LLOG_EXCEPTIONS, 
			("No 'default' found in the X.400 incoming tables"));
		return RTS_VALIDATE;
	}


	/* -- encode new Connect Accept Data -- */
	rts_encode_request (ppe, prq, rp -> our_name, rp -> our_passwd, isacs);
	RPfree (rp);

	if (remote_site)  free (remote_site);
	remote_site = strdup (key);

	PP_LOG (LLOG_EXCEPTIONS, 
		("No information found in the X.400 incoming tables - BUT have accepted the connection"));

	return OK;
}



static void print_remote_info (key, ba)
char				*key;
struct type_MTA_MTABindArgument *ba;
{
	static char		buffer[BUFSIZ];
	char			*cp;

	(void) sprintf (buffer, "key='%s' / '%s'",
			key, pretty_key);

	if (ba->offset == type_MTA_MTABindArgument_no__auth) {
		PP_NOTICE (("%s (no more connection data info)",
			    buffer));
		return;
	}

	(void) strcat (buffer, " mta='");

	if ((cp = qb2str (ba->un.auth->initiator__name)) != NULLCP) {
		(void) strcat (buffer, cp);
		(void) strcat (buffer, "'");
		free (cp);
	}
	(void) strcat (buffer, " passwd='");
	switch (ba -> un.auth -> initiator__credentials -> offset) {
	    case type_MTA_InitiatorCredentials_ia5string:
		cp = qb2str (ba -> un.auth
			     -> initiator__credentials -> un.ia5string);
		(void) strcat (buffer, cp);
		free (cp);
		(void) strcat (buffer, "'(IA5)");
		break;
	    case type_MTA_InitiatorCredentials_octetstring:
		cp = qb2str (ba->un.auth -> initiator__credentials
			     -> un.octetstring);
		(void) strcat (buffer, cp);
		free (cp);
		(void) strcat (buffer, "'(OCTS)");
		break;
	    default:
		(void) strcat (buffer, "<secure-credentials>'");
		break;
	}

	PP_NOTICE (("Connection from %s", buffer));
	return;
}



static char *build_inkey (rts, isacs)
struct RtSAPstart		*rts;
{
	struct PSAPaddr		pas;
	struct PSAPaddr		*ps;
	struct NSAPaddr		*na;
	int			nna;
	char			*cp;

	PP_TRACE (("build_inkey()"));

	if (isacs)
		ps = &rts -> rts_start.acs_start.ps_calling;
	else {
		bzero ((char *)&pas, sizeof pas);
		pas.pa_addr = rts -> rts_initiator.rta_addr;
		ps = &pas;
	}
        for (na = ps -> pa_addr.sa_addr.ta_addrs,
                nna = ps -> pa_addr.sa_addr.ta_naddr;
                nna > 0; nna --, na ++) {
                if (na -> na_type == NA_TCP) {
                        PP_TRACE (("Address is TCP, port set to 0 from %d",
                                na -> na_port));
                        na -> na_port = 0;
			/* bug in isode 6.6 */
                        if ((cp = index(na -> na_domain, '+')) != NULL)
                                *cp = '\0';
                }
        }

	if (pretty_key)
		free (pretty_key);
	pretty_key = strdup(paddr2str (ps, NULLNA));

	return _paddr2str (ps, NULLNA, -1);
}


dump_pdu (qb, str)
struct qbuf *qb;
char *str;
{
	struct qbuf *q;
	char	buf[512];
	char	*cp;
	int	len, n;

	if ((pp_log_norm -> ll_events & LLOG_PDUS) == 0)
		return;

	PP_LOG (LLOG_PDUS, ("PDU dump: %s", str));
	for (q = qb -> qb_forw; q != qb; q = q -> qb_forw) {
		for (cp = q -> qb_data, len = q -> qb_len; 
		     len > 0;) {
			n = min (sizeof (buf) / 2 - 1, len);
			buf[explode (buf, (u_char *)cp, n)] = 0;
			ll_printf (pp_log_norm, "%s", buf);

			len -= n;
			cp += n;
		}
	}
	ll_printf (pp_log_norm, "\n-----------\n");
}
