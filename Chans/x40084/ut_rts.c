/* ut_rts.c - X400 rts utilities */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40084/RCS/ut_rts.c,v 6.0 1991/12/18 20:13:50 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/ut_rts.c,v 6.0 1991/12/18 20:13:50 jpo Rel $
 *
 * $Log: ut_rts.c,v $
 * Revision 6.0  1991/12/18  20:13:50  jpo
 * Release 6.0
 *
 */



#include <isode/rtsap.h>
#include "util.h"
#include "rtsparams.h"
#include "chan.h"
#include "Rts84-types.h"

static char		*build_inkey();
static char		*pretty_key = NULLCP;       /* -- for logging only -- */
static void		print_remote_info();

char			*remote_site = NULLCP;
char			*undefined_bodyparts = NULLCP;

static int remote_info_not_found();



/* ---------------------  Begin	 Routines  -------------------------------- */




int rts_encode_request (ppe, request, mta, pass)
PE				*ppe;
struct type_Rts84_Request	**request;
char				*mta, *pass;
{
	struct type_Rts84_Request	*req;
	struct member_Rts84_0		*rq;

	PP_TRACE (("rts_encode_request (%s %s)", mta, pass));

	req = (struct type_Rts84_Request *) calloc (1, sizeof (*req));
	if (req == NULL) {
		do_reason ("rts_encode_request/Unable to calloc");
		return NOTOK;
	} 

	rq = req->un.choice_Rts84_1 =
		(struct member_Rts84_0 *) calloc (1, sizeof (*rq));
	if (rq == NULL) {
		do_reason ("rts_encode_request/Unable to calloc");
		return NOTOK;
	} 

	if (mta == NULLCP)
		req -> offset = type_Rts84_Request_1;
	else {
		req->offset = type_Rts84_Request_2;
		rq->mTAName = str2qb (mta, strlen (mta), 1);
		rq->password = ia5s2prim (pass, strlen (pass));
	}

	*request = req;

	if (encode_Rts84_Request (ppe, 1, 0, NULLCP, req) == NOTOK) {
		do_reason ("rts_encode_request/Rejected: '%s'", PY_pepy);
		return NOTOK;
	}

	PP_TRACE (("x400out84/encode_Rts84_Request successful!"));
	return OK;
}




int parameter_checks (pe, mta, pass, key, checks)
PE	pe;
char	*mta; 
char	*pass;
char	*key;
int	checks;
{
	struct type_Rts84_Request	*req = NULL;
	struct member_Rts84_0		*rq;
	char				*remotesite = NULLCP;
	char				*password = NULLCP;
	int				retval = NOTOK;

	PP_TRACE (("parameter_checks('%s', '%s', '%s', '%s')",
		key ? key : "null", 
		mta ? mta : "null",
		pass ? pass : "null", 
		checks ? "true" : "false"));

	if (decode_Rts84_Request (pe, 0, NULLIP, NULLVP, &req) == NOTOK) {
		do_reason ("parameter_checks/Rejected: '%s'", PY_pepy);
		goto parameter_checks_free;
	}

	print_remote_info (key, req);

	if (checks == FALSE) {
		retval = OK;
		goto parameter_checks_free;
	}

	if (req->offset == type_Rts84_Request_1) {
		PP_NOTICE (("x400out84/parameter_checks() are not required"));
		retval = OK;
		goto parameter_checks_free;
	}

	rq = req->un.choice_Rts84_1;

	remotesite = qb2str (rq->mTAName);

	if (rq->password->pe_form != PE_FORM_PRIM) {
		do_reason ("parameter_checks/Password has a bad format");
		goto parameter_checks_free;
	}
	else {
		int len;
		password = prim2str (rq->password, &len);
	}

	PP_TRACE (("parameter_checks MTA=%s, P=%s", remotesite, password));

	if (mta && lexequ (remotesite, mta) != 0) {
		do_reason ("parameter_checks/MTAName mismatch: specified='%s' received='%s'", mta, remotesite);
		goto parameter_checks_free;
	}


	if (lexequ (pass, "dflt") == 0) {
		retval = OK;
		goto parameter_checks_free;
	}


	if (pass && lexequ ((char *)password, pass) != 0) {
		do_reason ("parameter_checks/Password mismatch: specified='%s' received='%s'", pass, (char *)password);
		goto parameter_checks_free;
	}

	retval = OK;

parameter_checks_free: ;
	if (req)		free_Rts84_Request (req);
	if (remotesite)		free (remotesite);
	if (password)		free ((char *)password);

	return retval;
}




/* -- Checks P1 User-data: mTAName and password -- */

rts_decode_request (rts, ppe, prq, chan)
struct RtSAPstart		*rts;
PE				*ppe;
struct type_Rts84_Request	**prq;
CHAN				*chan;
{
	char		*key;
	RtsParams	*rp;
	int		checkit = TRUE;

	key = build_inkey (rts);

	PP_TRACE (("x400in84/rts_decode_request (%s)", key));


	if ((rp = tb_rtsparams (chan->ch_in_table, key)) == NULL) 
		return (remote_info_not_found (rts, ppe, prq, key, chan));


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
			      rp -> their_passwd, key, checkit) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Parameter check failed"));
		return RTS_VALIDATE;
	}


	/* -- encode new Connect Accept Data -- */
	rts_encode_request (ppe, prq, rp -> our_name, rp -> our_passwd);


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





static int  remote_info_not_found (rts, ppe, prq, key, chan)
struct RtSAPstart		*rts;
PE				*ppe;
struct type_Rts84_Request	**prq;
char				*key;
CHAN				*chan;
{
	RtsParams		*rp;  /* --- contains default values --- */


	PP_TRACE (("remote_info_not_found()"));

	parameter_checks (rts -> rts_data, NULLCP, NULLCP, key, FALSE);

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
	rts_encode_request (ppe, prq, rp -> our_name, rp -> our_passwd);
	RPfree (rp);

	if (remote_site)  free (remote_site);
	remote_site = strdup (key);

	PP_LOG (LLOG_EXCEPTIONS, 
		("No information found in the X.400 incoming tables - BUT have accepted the connection"));

	return OK;
}




static void print_remote_info (key, req)
char				*key;
struct type_Rts84_Request	*req;
{
	static char		databuf[BUFSIZ];
	char			buf[BUFSIZ];	
	PE			pe;
	char			*cp;


	bzero (databuf, BUFSIZ);

	(void) sprintf (databuf, "key='%s'  ", key);

	if (pretty_key) {
		(void) sprintf (buf, "key-pretty='%s'  ", pretty_key);
		(void) strcat (databuf, buf);
	}

	if (req->offset == type_Rts84_Request_1) {
		(void) strcat (databuf, "(no more connection data info)");
		goto print_remote_info_end;
	}


	if ((cp = qb2str (req->un.choice_Rts84_1->mTAName)) != NULLCP) {
		(void) sprintf (buf, "mta='%s'  ", cp);
		(void) strcat (databuf, buf);
		free (cp);
	}

	pe = req->un.choice_Rts84_1->password;

	if (pe->pe_form == PE_FORM_PRIM)
		if (pe -> pe_prim != 0) {
			int len;
			cp = prim2str (pe, &len);
			(void) sprintf (buf, "password='%s'  ", cp);
			(void) strcat (databuf, buf);
			free (cp);
		}


print_remote_info_end:	;
	PP_NOTICE (("Connection from %s", databuf));
	return;
}




static char *build_inkey (rts)
struct RtSAPstart		*rts;
{
	struct PSAPaddr		pas;
	struct NSAPaddr		*na;
	int	nna;
	char	*cp;

	PP_TRACE (("build_inkey()"));

	bzero ((char *)&pas, sizeof pas);
	pas.pa_addr = rts -> rts_initiator.rta_addr;
	for (na = pas.pa_addr.sa_addr.ta_addrs,
		nna = pas.pa_addr.sa_addr.ta_naddr;
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

	if (pretty_key)		free (pretty_key);	
	pretty_key = strdup(paddr2str (&pas, NULLPA));

	return _paddr2str (&pas, NULLPA, -1);
}

dump_pdu (qb, str)
struct qbuf *qb;
char *str;
{
	struct qbuf *q;
	char	buf[512];
	char	*cp;
	int	len, n;

	PP_LOG (LLOG_PDUS, ("PDU dump: %s", str));
	if ((pp_log_norm -> ll_events & LLOG_PDUS) == 0)
		return;

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
