/* de_wtmail.c: nitty gritty of decnet */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/decnet/RCS/de_wtmail.c,v 6.0 1991/12/18 20:06:35 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/decnet/RCS/de_wtmail.c,v 6.0 1991/12/18 20:06:35 jpo Rel $
 *
 * $Log: de_wtmail.c,v $
 * Revision 6.0  1991/12/18  20:06:35  jpo
 * Release 6.0
 *
 */

/*
 *      This module contains all the nitty gritty stuff which actually
 *      does the decnet communications
 */

#include "util.h"
#include "retcode.h"
#include "chan.h"
#include "ap.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdni/dni.h>
#include <ctype.h>

#define DEC_OK 0x01000000
#define CC     0x10
#define CC_OK  0x20
#define MAXDNIBYTES 128         /* Maximum bytes in a line */
#define NET_TIMEOUT 120		/* Timeout for net reads/writes */

/* Externals */

extern int de_fd;
extern int mrgate;
extern int cc_ok;
extern int map_space;
extern char space_char;
extern char toupper();
char de_status_text[BUFSIZ];

/* Internals */
char *user_addr(), *rfc_de();

/* This routine opens a logical link */

de_nopen(host, err)
char *host;
char *err;
{
	struct ses_io_type sesopts;
	OpenBlock ob;
	static OpenBlock template =
	{
		"",             /* Host name (filled in later) */
		"",             /* Task name */
		27,             /* Mail 11 */
		"",             /* Userid */
		"",             /* Account */
		"",             /* Password */
		{               /* Connect data */
			16,     /* 16 bytes of data */
			3,      /* protocol version 3 */
			0,      /* ECO level 0 */
			0,      /* Customer ECO level 0 */
			0,      /* OS type (other) */
			0,0,0,0,/* No options */
			CC,     /* Send CC: line */
			0,      /* Reserved */
			0,      /* reserved */
			0,      /* Reserved */
			0,      /* RMS RFM transfer mode */
			0,      /* RMS RAT type */
			0,      /* Reserved */
			0       /* Reserved */
		}
	};

	PP_NOTICE(("opening connection to [%s]", host));

	/* First open the logical link device */
	PP_DBG(("attempting to open logical link device"));
	if ((de_fd = open("/dev/dni", O_RDWR)) < 0)
	{
		strcpy(err, "failed to open logical link device");
		return RP_LIO;
	}

	/* Now set up I/O options. We need record mode */
	sesopts.io_flags = SES_IO_RECORD_MODE;
	PP_DBG(("attempting to set record mode"));
	if (ioctl(de_fd, SES_IO_TYPE, &sesopts) < 0)
	{
		strcpy(err, "failed to set i/o options");
		return RP_LIO;
	}

	/* Now request link, including requesting CC: mode */
	ob = template;
	strcpy(ob.op_node_name, host);
	if (ioctl(de_fd, SES_LINK_ACCESS, &ob) < 0)
	{
		switch (errno & 0xff)
		{
		case FAILED:    /* Connection failed */
		case NODE_UNREACH: /* Node unreacheable */
			sprintf(err, "failed to connect to host [%s], unreacheable", host);
			return RP_NIO;
		case BAD_NAME: /* Invalid host name */
		case NODE_NAME:/* Unrecognised node name */
			sprintf(err, "failed to connect to host [%s], bad host", host);
			return RP_NO;
		case BAD_OBJECT:/* Mail11 object does not exist */
		case OBJ_NAME: /* Ditto */
			sprintf(err, "failed to connect to host [%s], mail object not present", host);
			return RP_NIO;
		case BY_OBJECT: /* Mail 11 won't talk to us */
			sprintf(err, "failed to connect to host [%s], mail object not communicating", host);
			return RP_NIO;
		default:
			sprintf(err, "failed to connect to host [%s], error = %d", host, errno & 0xff);
			return RP_NIO;
		}
	}

	/* Check if CC: is aceptable */
	PP_TRACE(("connection data 0x%x(%d)", ob.op_opt_data.im_data[8], ob.op_opt_data.im_length));
	if (ob.op_opt_data.im_length == 0)
		cc_ok = FALSE;
	else
		cc_ok = (ob.op_opt_data.im_data[8] & CC_OK) == CC_OK;

	PP_NOTICE(("connection open"));
	strcpy(err, "connection open");
	return RP_OK;
}

/* This routine closes a logical link */

de_nclose(type)
int type;
{
	SessionData sd;

	PP_NOTICE(("closing connection"));
	sd.sd_reason = 0;
	sd.sd_data.im_length = 0;
	if (ioctl(de_fd, SES_DISCONNECT, &sd) < 0)
	{
		PP_NOTICE(("failed to close connection, so aborting link"));
		ioctl(de_fd, SES_ABORT, &sd);
	}
	close(de_fd);
	PP_TRACE(("connection closed"));
}

/* This routine sends a `mark', used to delimit various parts of the protocol */

de_mark()
{
	char mark = 0;

	PP_TRACE(("de_mark()"));
	if (timeout(NET_TIMEOUT))
	{
		PP_NOTICE(("timed out while sending mark"));
		return RP_NIO;
	}
	if (write(de_fd, &mark, 1) < 0)
	{
		timeout(0);
		PP_NOTICE(("failed to transmit mark"));
		return RP_NIO;
	}
	timeout(0);
	return RP_OK;
}

/* this routine sends a single address and returns a success (or otherwise)
	code */

int de_wto(adr)
char *adr;
{

	PP_TRACE(("de_wto(%s)", adr));

	/* Extract the user part and send it */
	if (de_send(user_addr(adr)) != RP_OK)
		return RP_NIO;

	switch (de_status())
	{
	case RP_NIO:
		return RP_NIO;
	case RP_OK:
		return RP_AOK;
	case RP_NO:
		return RP_USER;
	default:
		return RP_NO;
	}
}


/* This routine sends a single, null terminated string to the decnet link */

de_send(buf)
char *buf;
{
	int size = strlen(buf);

	PP_TRACE(("de_send(%s)", buf));
	/* If size is greater than the maximum that the remote system
	can handle (usually about 127 bytes) then truncate */
	if (size > MAXDNIBYTES) size = MAXDNIBYTES;
	if (timeout(NET_TIMEOUT))
	{
		PP_NOTICE(("connection timed out"));
		return RP_NIO;
	}
	if (write(de_fd, buf, size) < 0)
	{
		timeout(0);
		PP_NOTICE(("failed to transmit data, errno = %d", errno & 0xff));
		PP_NOTICE(("data was `%s'", buf));
		return RP_NIO;
	}
	timeout(0);
	return RP_OK;
}

/* This routine gets a status code from the decnet connection */

int de_status()
{
	char txt[LINESIZE];
	long status;
	int len;

	PP_TRACE(("de_status()"));

	strcpy(de_status_text, "");
	if (timeout(NET_TIMEOUT))
	{
		PP_NOTICE(("timeout while transmitting status"));
		return RP_NIO;
	}
	if (read(de_fd, (char *)&status, sizeof(status)) < sizeof(status))
	{
		timeout(0);
		return (RP_NIO);
	}
	timeout(0);
	PP_DBG(("got 0x%lx", status));

	/* If bit 0 is set then good status */
	if ((status & DEC_OK) == DEC_OK)
	{
		return (RP_OK);
	}
	else
	{
		if (timeout(NET_TIMEOUT))
		{
			PP_NOTICE(("timeout while reading status"));
			return RP_NIO;
		}
		while ((len = read(de_fd, txt, sizeof(txt))) >= 0)
		{
			if (len == 1 && txt[0] == 0)
			{
				timeout(0);
				return RP_NO;
			}
			else
			{
				txt[len] = '\0';
				if (sizeof(de_status_text) - strlen(de_status_text) > len)
				{
					strcat(de_status_text, txt);
					strcat(de_status_text, "\n");
				}
			}
		}
		/* If we get here, the connection has dropped */
		timeout(0);
		return (RP_NIO);
	}
}

/* This routine takes an RFC822 message and returns the user part forced
   to upper case, `_' mapped to space if necessary */

char *user_addr(addr)
char *addr;
{
	AP_ptr  tree, group, name, local, domain, route;
	char *s;
	char *final;

	PP_DBG(("user_addr(%s)", addr));
	if (ap_s2p(addr, &tree, &group, &name, &local, &domain, &route) == (char *)NOTOK)
	{
		return "Orphanage";
	}
	for (s = local->ap_obvalue; *s = toupper(*s); s++)
	{
		if (map_space && *s == space_char) *s = ' ';
	}

	/* Allocate enough space for string + MRGATE:: if needed */
	final = malloc(strlen(local->ap_obvalue) + 10);

	/* Tack on an MRGATE if talking to one */
	strcpy(final, (mrgate) ? "MRGATE::" : "");
	strcat(final, local->ap_obvalue);

	PP_DBG(("returns: %s", final));
	return (final);
}

/* This procedure takes an RFC822 string and converts it to DecNet format */

char *rfc_de(addr)
char *addr;
{
	AP_ptr  tree, group, name, local, domain, route, a;
	char *final;
	int size = 0;

	PP_DBG(("rfc_de(%s)", addr));
	if (ap_s2p(addr, &tree, &group, &name, &local, &domain, &route) == (char *)NOTOK)
	{
		return "Orphanage";
	}

	/* OK, now troll down the tree adding up the sizes so we can alloc
	   a big enough string */

	for (a = tree; a ; a = a->ap_next)
	{
		switch(a->ap_obtype)
		{
		case AP_MAILBOX:
			size += strlen(a->ap_obvalue) + 1;
			break;
		case AP_DOMAIN:
			size += strlen(a->ap_obvalue) + 2;
			break;
		}
	}

	/* Allocate the space */
	final = malloc(size);

	/* Now build the decnet address */

	for (a = tree, strcpy(final, ""); a ; a = a->ap_next)
	{
		if (a->ap_obtype == AP_DOMAIN)
		{
			strcat(final, a->ap_obvalue);
			strcat(final, "::");
		}
	}
	for (a = tree; a ; a = a->ap_next)
	{
		if (a->ap_obtype == AP_MAILBOX)
			strcat(final, a->ap_obvalue);
	}

	PP_TRACE(("returns: %s", final));
	return (final);
}
