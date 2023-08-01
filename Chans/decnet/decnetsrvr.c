/* decnectsrvr.c: DecNet Server */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/decnet/RCS/decnetsrvr.c,v 6.0 1991/12/18 20:06:35 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/decnet/RCS/decnetsrvr.c,v 6.0 1991/12/18 20:06:35 jpo Rel $
 *
 * $Log: decnetsrvr.c,v $
 * Revision 6.0  1991/12/18  20:06:35  jpo
 * Release 6.0
 *
 */

/* decnet server designed to be started by dniserver (sunlink DNI)

   Unfortunately there seems to be no way of passing parameters
   in to the slave process from dniserver, so we cannot pass in the
   channel name. This is thus hardwired in as `decnet-in'.

   Sorry folks...

   Some design philosophy: we accept all recipient addresses, no matter
   how preposterous. We then generate non-delivery reports. This means
   that users of All-in-One and Mail Router actually get failure reports
   in a timely fashion, rather than several days later which is what happens
   if we reject the recipient addresses at the mail11 protocol level. Of course
   it means that users of straight VMS mail have to wait a few extra seconds
   for their failure notification, but what the hell, they shouldn't be
   using it anyway.

   Mail11 To: and Cc: lines are accepted and mapped to X-Vms-To: and X-Vms-Cc:
   in order to save us the possibly fruitless task of trying to parse and
   convert what the incoming mailer dumps into these records. All recipients
   are treated as primary recipients and inserted into a proper To: line

   We map the sender address into a From: line, after having been converted to
   RFC822. Personal names are mapped into comments, as we cannot guarantee
   the contents of the mail11 personal name data (and it is easier anyway).

   We add a Date: field. This of course is date of acceptance at the incoming
   interface and bears no resemblance to actual date of posting.

 */

#include "head.h"
#include "chan.h"
#include "prm.h"
#include "q.h"
#include "ap.h"
#include "retcode.h"
#include "list_rchan.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <netdni/dni.h>

#define DEC_OK  0x01000000
#define MAXLINE 78                              /* Max characters on header line */
#define NET_TIMEOUT 120         		/* Timeout for net reads/writes */


/* Externals */
extern char *cont_822;

/* Internals */
char *decnet_in = "decnet-in";                  /* Name of this channel */
char *remote_host;                              /* name of the remote host */
CHAN *mychan;
int de_fd       = 3;                            /* The decnet file descriptor */
int cc_ok       = 0;                            /* CC line OK */
int map_space   = 0;                            /* Do we want to map spaces? */
char *de_s2s();
ADDR *to_first  = 0;                            /* Will contain list of recipients */
ADDR *ad_sender;                                /* Will contain sender */


/* ARGSUSED */
main(argc, argv)
int argc;
char *argv[];
{
	int state = 0;
	int addrs, mark;
	RP_Buf rp;
	char record[BUFSIZ];

	/* First initialise the channel */
	chan_init(decnet_in);

	/* Do any initialisation */
	chaninit();

	/* Now start the real work */
	while (rp_isgood(de_get(record, sizeof(record), &mark)))
	{
		switch (state)
		{
		case 0:	/* Start of a message, getting sender */
			do_sender(record);
			addrs = 0;
			state = 1;
			break;
		case 1:	/* Process addressees */
			if (!mark)
			{
				do_recip(record);		
				addrs++;
			}
			else
			{
				if (rp_isbad(io_adend(&rp))
				|| rp_isbad(io_tinit(&rp)) 
				|| rp_isbad(io_tpart("hdr.822", 0, &rp)))
					error(rp.rp_line);
				state = 2;
			}
			break;
		case 2:	/* Now get To: line */
			do_to(record);
			if (cc_ok)
				state = 3;
			else
				state = 4;
			break;
		case 3:	/* Now get Cc: line if allowed */
			do_cc(record);
			state = 4;
			break;
		case 4:	/* Now get Subject: line */
			do_subject(record);
			if (rp_isbad(io_tdend(&rp))
			|| rp_isbad(io_tpart("2.ia5", 0, &rp)))
				error(rp.rp_line);
			state = 5;
			break;
		case 5:	/* Now do the text */
			if (!mark)
				do_txt(record);
			else
			{
				if (rp_isbad(io_tdend(&rp))
				|| rp_isbad(io_tend(&rp)))
					error(rp.rp_line);
				do_status(addrs);
				state = 0;
				PP_NOTICE((">>> Message sent to %d recipients", addrs));
			}
			break;
		}
	}

	/* If here then connection has failed for some reason.
	   This may be normal */
	if (state == 0)
	{
		PP_NOTICE(("Channel terminating normally"));
		io_end(OK);
	}
	else
	{
		PP_NOTICE(("Channel terminated abnormally (state %d)", state));
		io_end(NOTOK);
	}
	exit(0);
}

/* This procedure does any initialisation. This includes accepting the
   decnet connection */

chaninit()
{
	extern char *strstr();
	SessionData sd;
	OpenBlock ob;
	struct prm_vars prm;
	RP_Buf rp;

	PP_TRACE(("chaninit()"));

	/* Get access info */
	if (ioctl(de_fd, SES_GET_AI, &ob) < 0)
		error("failed to get access information");
	remote_host = strdup(ob.op_node_name);

	/* Check channel OK */
	if ((mychan = ch_mta2struct(decnet_in, remote_host)) == (CHAN *)0)
		error("Channel not known");

	/* Do any initialising */
	rename_log(mychan -> ch_name);
	PP_NOTICE (("Connection from [%s] on channel %s", remote_host,
		    mychan -> ch_name));

	/* Accept the connection */
	sd.sd_data.im_length = 0;
	if (ioctl(de_fd, SES_ACCEPT, &sd) < 0)
		error("decnet accept failed");

	/* Initialise IO system */
	setuid(geteuid());
	if (rp_isbad(io_init(&rp)))
		error("failed to initialise incoming decnet channel");

	/* Set up the management parameters */
	prm_init(&prm);
	prm.prm_opts = PRM_ACCEPTALL;
	if (rp_isbad(io_wprm(&prm, &rp)))
		error("failed to set management parameters");
	prm_free(&prm);

	/* Check config options */
	map_space = (strstr(mychan->ch_in_info, "map_space") != NULLCP);

	PP_TRACE(("options: map_space(%d)", map_space));

}

/* This procedure takes a string containing the sender name in DEC format
   and starts the message submission process
 */

do_sender(sender)
char *sender;
{
	char *cp;
	Q_struct qp;
	RP_Buf rp;
	char lcl_sender[BUFSIZ];

	PP_TRACE(("do_sender(%s)", sender));

	/* Set up the per message parameters */
	PP_TRACE(("set up message parameters"));
	q_init(&qp);
	/* qp.cont_type = strdup(cont_822); */
	txt2listbpt(&qp.encodedinfo.eit_types, cp = strdup("ia5,hdr.822"));
	free (cp);
	qp.inbound = list_rchan_new(remote_host, mychan -> ch_name);
	if (rp_isbad(io_wrq(&qp, &rp)))
		error(rp.rp_line);
	q_free(&qp);

	/* Set up the sender address */
	PP_TRACE(("set up sender address"));
	sprintf(lcl_sender, "%s::%s", remote_host, sender);
	ad_sender = adr_new(de_s2s(lcl_sender), AD_ANY_TYPE, 0);
	if (rp_isbad(io_wadr(ad_sender, AD_ORIGINATOR, &rp)))
		error(rp.rp_line);

}

/* This procedure takes one recipient address, submits it, and returns
   the DECnet status value. Note that we always accept addresses. If the
   address is incorrect we return a non-delivery report. This is for the
   benefit of the cretinous behaviour of VMS MRgate software which just
   keeps on resubmitting failed mail at short intervals until it times out, 
   several days later.

   A To: line is built up at the same time.

 */

do_recip(recip)
char *recip;
{
	ADDR *ad_recip;
	RP_Buf rp;

	PP_TRACE(("do_recip(%s)", recip));

	/* If we get a failure from io_wadr then this is fairly major,
	as we have asked submit to handle non-delivery reports */

	ad_recip = adr_new(de_s2s(recip), AD_ANY_TYPE, 1);
	if (rp_isbad(io_wadr(ad_recip, AD_RECIPIENT, &rp)))
		error(rp.rp_line);
	adr_add(&to_first, ad_recip);

	de_send_status(OK);
}

/* This procedure takes a Cc: line and maps it to an X-Vms-Cc: line in the
   header
 */

do_cc(cc)
char *cc;
{
	char line[BUFSIZ];

	PP_TRACE(("do_cc(%s)", cc));
	sprintf(line, "X-Vms-Cc: %s\n", cc);
	if (rp_isbad(io_tdata(line, strlen(line))))
		error("failed to write Cc: line");
	
}

/* This procedure takes a To: line and maps it to a X-Vms-To: line in the
   header. It also creates a new To: line out of the list of recipients
   built by do_recip and a From: line out of ad_sender
 */

do_to(to)
char *to;
{
	char line[BUFSIZ];
	ADDR *ad;

	PP_TRACE(("do_to(%s)", to));
	sprintf(line, "X-Vms-To: %s\n", to);
	if (rp_isbad(io_tdata(line, strlen(line))))
		error("failed to write X-Vms-To: line");

	for (ad = to_first, strcpy(line, "To: "); ad; ad = ad->ad_next)
	{
		if ((strlen(line) + strlen(ad->ad_value) + 4) > MAXLINE)
		{
			strcat(line, "\n");
			if (rp_isbad(io_tdata(line, strlen(line))))
				error("failed to write To: line");
			strcpy(line, "    ");
		}
		strcat(line, ", ");
		strcat(line, ad->ad_value);
	}
	strcat(line, "\n");
	if (rp_isbad(io_tdata(line, strlen(line))))
		error("failed to write To: line");
	sprintf(line, "From: %s\n", ad_sender->ad_value);
	PP_NOTICE(("From %s", ad_sender->ad_value));
	if (rp_isbad(io_tdata(line, strlen(line))))
		error("failed to write From: line");

}

/* This procedure takes a Subject: line and inserts it into the header
 * also inserts Date: line
 */

do_subject(subject)
char *subject;
{
	char line[BUFSIZ], buf[BUFSIZ];
	UTC     ut, lt;
	extern UTC utclocalise();

	PP_TRACE(("do_subject(%s)", subject));
	sprintf(line, "Subject: %s\n", subject);
	if (rp_isbad(io_tdata(line, strlen(line))))
		error("failed to write Subject: line");

	/* Do date */
	ut = utcnow();
	lt = utclocalise(ut);

	UTC2rfc(lt, buf);
	sprintf(line, "Date: %s\n", buf);
	if (rp_isbad(io_tdata(line, strlen(line))))
		error("failed to write Date: line");
	free ((char *)ut);
	free ((char *)lt);

}

/* This procedure takes a single text record and inserts it into the body
 */

do_txt(line)
char *line;
{
	PP_TRACE(("do_txt(%s)", line));

	if ((strlen(line) && rp_isbad(io_tdata(line, strlen(line))))
	|| rp_isbad(io_tdata("\n", 1)))
		error("failed to write text line");

}

/* This procedure sends `n' OK status messages back to DECnet */

do_status(n)
int n;
{

	PP_TRACE(("do_status(%d)", n));

	for ( ; n > 0 ; n--)
		de_send_status(OK);

}

/* This procedure winds up in the event of error. It prints a notice in the
   log, aborts the decnet connection (if still extant), aborts the message
   submission process, and exits. It will be called in the event of any
   failure in the protocol, as there is *no* recovery mechanism.
 */

error(s)
char *s;
{
	SessionData sd;

	PP_NOTICE(("error exit [%s]", s)); 

	/* Abort the DECnet link (don't bother to check return status) */
	sd.sd_reason = 0;
	sd.sd_data.im_length = 0;
	ioctl(de_fd, SES_ABORT, &sd);

	/* Abort message submission process */
	io_end(NOTOK);

	/* Farewell */
	exit(1);

}

/* This procedure sends a status to the DECnet */

de_send_status(value)
int value;
{
	long status;

	PP_TRACE(("de_send_status(%d)", value));

	status = DEC_OK;
	if (timeout(NET_TIMEOUT))
	{
		error("decnet timeout in write");
	}
	if (write(de_fd, (char *)&status, sizeof(status)) != sizeof(status))
		error("failed to write status value");
	timeout(0);

}

/* This procedure takes an address in DECNet form and returns a parse tree.
   Mail11 personal names are converted to comments. If the address
   is surrounded by quotes then we assume it is not for conversion and feed
   it into the normal address parser. We also force the address (not the
   comments) into lower case, as I hate machines that SHOUT at me. Internal
   (to the address) quotes are removed, as these seem to be generated by
   MRGATE somewhat gratuitously.
 */

AP_ptr de_s2t(s)
char *s;
{
	AP_ptr  ap,
		ap_start,
		local_host = NULLAP,
		route_host = NULLAP,
		personal = NULLAP;
	int state = 0;
	char *t, *token, *token_end, *last_token;

	PP_TRACE(("de_s2t(%s)", s));
	for (ap_start = ap = ap_alloc(), t = s; *t; t++)
	{
		switch (state)
		{
		case 0: /* Start of things. Looking for non_space */
			if (*t == '"') /* Assume RFC822 address */
			{
				token = t + 1;
				state = 5;
			}
			else
			{
				*t = tolower(*t);
				token = t;
				state = 1;
				last_token = 0;
			}
			break;
		case 1: /* Looking at host names or mailbox names */
			if (*t == ':') /* Could be start of a :: */
			{
				state = 2;
			}
			else if (*t == ' ') /* Could be mailbox */
			{
				token_end = t;
				state = 3;
			}
			else
			{
				*t = tolower(*t);
			}
			break;
		case 2: /* Just got a :, looking for the next */
			if (*t == ':') /* Bingo! Got a :: */
			{
				*(t - 1) = '\0';

				/* Now tack it into the parse tree */
				if (strstr(token, "mrgate") == 0 &&
					strstr(token, "a1") == 0 &&
					!( last_token && strstr(token, last_token)))
				{
					ap_fllnode(ap, AP_DOMAIN, token);
					route_host = local_host;
					local_host = ap;
					ap->ap_next = ap_alloc();
					ap->ap_ptrtype = AP_PTR_MORE;
					ap = ap->ap_next;
					last_token = token;
				}
				token = t + 1;
				state = 1;
			}
			else  /* Error */
			{
				return (BADAP);
			}
			break;
		case 3: /* May be end of user name */
			if (*t == '"')  /* personal name */
			{
				*token_end = '\0';
				ap_fllnode(ap, AP_MAILBOX, token);
				token = t + 1;
				state = 4;
			}
			else if (*t != ' ') /* Bother, not finished */
			{
				*t = tolower(*t);
				state = 1;
			}
			break;
		case 4: /* Collecting personal name */
			break;
		case 5: /* processing quoted address */
			if (*t == '"')
				*t = '\0';
			break;
		}
	}
	/* Finish off if necessary */
	if (state == 3)
		*token_end = '\0';
	if ((state == 1 || state == 3) &&
		!(last_token && strstr(token, last_token)))
		ap_fllnode(ap, AP_MAILBOX, token);
	if (state == 4)
	{
		*(t - 1) = '\0';
		personal = ap_new(AP_COMMENT, token);
	}

	/* If state 5 then was processing RFC address */
	if (state == 5)
		ap_start = ap_s2t(token);
	else /* We have to take last host and put after mailbox */
	{
		if (route_host)
		{
			ap_move(route_host, local_host);
			if (personal)
				ap_insert(local_host, AP_PTR_MORE, personal);
		}
		else if (local_host)
		{
			ap->ap_next = local_host;
			ap->ap_ptrtype = AP_PTR_MORE;
			local_host->ap_next = NULLAP;
			local_host->ap_ptrtype = AP_PTR_NIL;
			ap_start = ap;
			if (personal)
				ap_insert(local_host, AP_PTR_MORE, personal);
		}
		else if (personal)
			ap_insert(ap, AP_PTR_MORE, personal);

	}

	/* Last of all we cruise down the tree removing spurious "s
	in DOMAIN and MAILBOX tokens. Also at this point we map ' ' to '_'
	if required */
	for (ap = ap_start; ap; ap = ap->ap_next)
	{
		if (ap->ap_obtype == AP_DOMAIN || ap->ap_obtype == AP_MAILBOX)
		{
			char *s1, *d;

			for (s1 = d = ap->ap_obvalue; *s1; s1++)
				if (map_space && *s1 == ' ') *d++ = '_';
				else if (*s1 != '"') *d++ = *s1;
			*d = '\0';
		}
	}

	return (ap_start);
}

/* This procedure takes a decnet address and returns it mapped to RFC822
 */

char *de_s2s(de)
char *de;
{
	AP_ptr  tree;
	char *rfc;

	PP_TRACE(("de_s2s(%s)", de));
	ap_norm_all_domains();
	if ((tree = de_s2t(de)) != BADAP)
	{
		ap_t2s(ap_normalize(tree, mychan->ch_ad_order), &rfc);
		PP_TRACE(("returns: `%s'", rfc));
		return (rfc);
	}
	PP_TRACE(("returns (fail): `%s'", de));
	return (de);
}

/* This procedure gets a single decnet record, null terminates it (stripping
   off terminating carriage control if present). It sets mark if the mark
   record was read.
 */

de_get(s, n, m)
char *s;
int n, *m;
{
	int no_read;

	PP_TRACE(("de_get()"));

	/* Read a record */
	if (timeout(NET_TIMEOUT))
	{
		PP_NOTICE(("read timed out"));
		return RP_NIO;
	}
	if ((no_read = read(de_fd, s, n - 1)) < 0)
	{
		PP_DBG(("read failed [%d]", errno));
		return RP_NIO;
	}
	timeout(0);

	/* Is it a mark? */
	if (no_read == 1 && s[0] == '\0')
	{
		*m = TRUE;
		PP_DBG(("returns `mark'"));
		return RP_OK;
	}

	/* terminate it, removing \r\n if present */
	if (no_read >= 2 && s[no_read - 2] == '\r' && s[no_read - 1] == '\n')
		s[no_read - 2] = '\0';
	else
		s[no_read] = '\0';

	/* clear mark and return */
	*m = 0;
	PP_DBG(("returns `%s'", s));
	return RP_OK;

}
