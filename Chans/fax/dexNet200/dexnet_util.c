/* dexnet_util.c: various utility routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/dexnet_util.c,v 6.0 1991/12/18 20:08:53 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/dexnet_util.c,v 6.0 1991/12/18 20:08:53 jpo Rel $
 *
 * $Log: dexnet_util.c,v $
 * Revision 6.0  1991/12/18  20:08:53  jpo
 * Release 6.0
 *
 */

/* Modified by Pekka Nikander for sunos 4 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termio.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <setjmp.h>
#include <sgtty.h>
#include <signal.h>
#include <errno.h>
#include "dexnet.h"

/* pp includes */
#include	"util.h"
#include	"retcode.h"
#include	"adr.h"
#include	"qmgr.h"
#include 	<isode/cmd_srch.h>
#include 	"tb_bpt88.h"
#include	"IOB-types.h"
#include	"MTA-types.h"
#include	"or.h"
#include <stdarg.h>
#ifndef sun
#include	<sys/termios.h>
#endif
#include	"../faxgeneric.h"

/*
 *	Driver for Fujitsu dexNET 200 fax modem, adopted for DECstation 
 *	5000. 
 *
 *	This driver is based on a driver written by Jim Knowles (NASA). Many
 *	thanks go Jim for his help.
 *
 *	This driver assumes that hardware (CTS) flow control is in use.
 *	At one point, I attempted to make it work with XON/XOFF. However,
 *	I never could make this work well. The problem was that there
 *	was a non-zero delay between the time that the dexnet buffer was
 *	full and the time that the XOFF was received by this program. 
 *	Invariably, the program would write a few hundred bytes more after
 *	the buffer was full before getting the XOFF. This would corrupt
 *	the buffer resulting in either a loss of a few scan lines or
 *	a complete garbage page.
 *
 *	Note that when the dexnet does a reset, DSR fluctuates, so this pin
 *	must be tied high for the kernel to be happy. Also, DCD fluctates 
 *	in response to the fax telephone handshake; it must also be tied
 *	high for the device driver to be happy.
 *
 *	Note that the error processing is complicated because of the
 *	fact that the entire g3 image may be written to the modem
 *	before any response is received (if the image is short
 *	enough). In this case, the errors (like busy) must be
 *	processed by the end of message procedure.
 *
 *	This driver was developed for a DECstation 5000. The cable used to
 *	connect the dexnet to the 5000 was rs232:
 *
 * Modem (Male)         Dec (Female)
 *        TD 2-------------2
 *        RD 3-------------3
 *       CTS 5-------------5
 *       GND 7-------------7
 *       +12 9-----+-------6 DSR
 *                 |-------8 DCD
 *      DTR 20-------------20
 *
 *	Information about the dexNet 200 can be obtained from
 *
 *            Fujitsu
 *            Imaging Systems of America, Inc
 *            Danbury Connecticut, 06810
 *            (800) 537-1262
 *
 *	Robert Hagens
 *	November, 1991
 */
int		total_cc = 0;
static 	int 	readDebug = 0;
extern int	fax_debug;

#define doIoctl(dem, ioctlName, arg, msg)\
	if (ioctl((dem)->fd, (ioctlName), (arg)) == -1) {\
		sprintf((dem)->errBuf, "ioctl error: %s: %s", msg, sys_errname(errno));\
		return(NOTOK);\
	}

/*
 *  dexNET 200 error response
 */
struct {
	int		code;
	char	*text;
} responseCodes[] = {
	dexUnknown, "Unknown response",
	dexNotReady, "Not Ready",
	dexReady,	 "Ready",
	dexConnected, "Connected",
	dexWaiting,	"Waiting for call",
	dexDialing,	"Dialing",
	dexSID,		"Station Identification",
	dexTID,		"Terminal Identification",
	dexStandardRes, "Standard Resolution",
	dexFineRes,	"Fine Resolution",
	dexCallTerm,	"Call terminated normally",
	dexAck,		"Ack",
	dexNak,		"Nak",
	dexRing,	"Ring",
	dexBufferLow,"Buffer low",
	dex9600Baud,	"Line speed 9600",
	dex7200Baud,	"Line speed 7200",
	dex4800Baud,	"Line speed 4800",
	dex2400Baud,	"Line speed 2400",
	dexScanError,	"Scan line error",
	dexVersion,		"Version",
	dexBusy,		"Busy",
	dexNoResponse,	"No response",
	dexNotAFax,		"Not a Fax",
	dexDisconnect,	"Line Disconnect",
	dexResError,	"Reserved Error",
	dexUnderrun,	"Buffer Underrun",
	dexBufFull,		"Buffer Full",
	dexBufEmpty,	"Buffer Empty",
	-1, ""
};

/* map an response code to a textual string */
static char *
codeToString(code)
int	code;
{
	int i;
	for (i=0; responseCodes[i].code != -1; i++) {
		if (responseCodes[i].code == code)
			return(responseCodes[i].text);
	}
	return("Code Unknown");
}

/*
 *	Open device. Configure line to device. Return ptr to
 *	modem control block
 */
openDexModem(dem)
DexModem	*dem;
{
	struct sgttyb	ttyb;				/* structure for tty parameters */
	int				disc = NTTYDISC;	/* line	discipline */
	int				lflags = LLITOUT;	/* local flags */
	int				temp = 0;

	if ((dem->fd = open(dem->devName, O_RDWR)) == -1) {
		sprintf(dem->errBuf, "%s: open: %s", dem->devName, 
			sys_errname(errno));
		return(NOTOK);
	}

	/* Set the line discipline */
	doIoctl(dem, TIOCSETD, (char *)&disc, "TIOCSETD");

#ifdef TIOCMODEM
	/* Set this tty to be a modem */
   	doIoctl(dem, TIOCMODEM, (char *)&temp, "TIOCMODEM");
#endif

	/* Hang up the fax - When a close is done on pDevName */
	doIoctl(dem, TIOCHPCL, (char *)NULL, "TIOCHPCL");

	/* Fetch the basic parameters */
	doIoctl(dem, TIOCGETP, (char *)&ttyb, "TIOCGETP");

	/* Set the speed to 9600 */
	ttyb.sg_ispeed = ttyb.sg_ospeed = B9600;
	ttyb.sg_flags = ANYP| RAW | PASS8;
	dem->highSpeed = 0;

	/* Set these parameters */
	doIoctl(dem, TIOCSETP, (char *)&ttyb, "TIOCSETP");

	/* Suppress output translations */
	doIoctl(dem, TIOCLSET, (char *)&lflags, "TIOCLSET");

	return(OK);
}

/* make fd non blocking I/O */
static
nonBlockIO(fd)
int	fd;
{
	int	flags;
	fcntl(fd, F_GETFL, &flags);
	flags |= O_NDELAY;
	fcntl(fd, F_SETFL, &flags);
}

static
blockIO(fd)
int	fd;
{
	int	flags;
	fcntl(fd, F_GETFL, &flags);
	flags &= ~O_NDELAY;
	fcntl(fd, F_SETFL, &flags);
}

/*
 *	Send command to modem. Make sure 'len' bytes were written.
 *	Msg is used only for diagnostics. Return success or failure of op
 */
writeDexModem(dexp, buf, len, msg)
DexModem	*dexp;
unsigned char	*buf;
int		len;
char	*msg;
{
	int cc;
#ifdef	DUMPWRITE
 	if (len < 100) {
		int i;
		printf("writeDexModem: ");
		for (i=0; i<len; i++)
			printf("\\%03o", *(buf+i));
		printf("\n");
	}
#endif	DUMPWRITE

	total_cc += len;
	while ((cc = write(dexp->fd, buf, len)) == -1 && 
		   (errno == EWOULDBLOCK || errno == EAGAIN)) {
		fd_set writefds;

		FD_ZERO(&writefds);
		FD_SET(dexp->fd, &writefds);

		if (select(dexp->fd+1, NULL, &writefds, NULL, NULL) < 0) {
			sprintf(dexp->errBuf, "select: %s", sys_errname(errno));
			return(NOTOK);
		}
	}
	if (cc != len) {
		sprintf(dexp->errBuf, "write: %s", sys_errname(errno));
		return(NOTOK);
	}
	return(OK);
}

/* block/read one char from the modem. fd may be in non-blocking mode, so
 use select to block here if we get EWOULDBLOCK */
static unsigned char
readModem(dexp)
DexModem	*dexp;
{
	int				l;
	unsigned char	c = NULL;

	if ((l = read(dexp->fd, &c, 1)) < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			fd_set			readfds;
			FD_ZERO(&readfds);
			FD_SET(dexp->fd, &readfds);

			if (select(dexp->fd+1, &readfds, NULL, NULL, NULL) < 0) {
				perror("select");
			}
			/* could assert (FD_ISSET(dexp->fd, &readfds)) at this point */
			if ((l = read(dexp->fd, &c, 1)) < 0)
				perror("readModem");
		} else
			perror("readModem");
	}

	if (readDebug)
		PP_TRACE(("readModem: x%02x\n", c));
	return(c);
}

/* Return true if there is data ready to be read on modem */
static char
dataPresent(dexp)
DexModem	*dexp;
{
	long	cc;
	if (ioctl(dexp->fd, FIONREAD, &cc) == -1) {
		perror("ioctl: FIONREAD");
		return(0);
	}
	return(cc > 0);
}

/* 
 *	Parse the response contained in buf and return an integer code
 *	corresponding to the response. Some responses contain an ascii
 *	string as part of the response. This is returned in misc
 */
static
readResponse(dexp, misc)
DexModem	*dexp;
char		**misc;	/* ascii part of response (assume misc is big enough) */
{
	static unsigned char	buf[100], *bp;
	int	code = dexUnknown;
	unsigned char	c;

	/* TODO: set timeout to catch hanging read */

	bzero(buf, sizeof(buf));
	bp = buf;

	c = readModem(dexp);
	if (c == ESC) {
		switch(readModem(dexp)) {
			case DLE:
				switch(readModem(dexp)) {
					case '0': code = dexNotReady; break;
					case '1': code = dexReady; break;
					case '2': code = dexConnected; break;
					case '3': code = dexDisconnect; break;
					case '4': code = dexWaiting; break;
					case '5': code = dexNotAFax; break;
					case '6': code = dexDialing; break;
					case '7': code = dexBusy; break;
					case '8': code = dexNoResponse; break;
					case '9':
						code = dexSID;
						while ((c = readModem(dexp)) != CR)
							*bp++ = c;
						break;
					case ':':
						code = dexTID;
						while ((c = readModem(dexp)) != CR)
							*bp++ = c;
						break;
					case '=': code = dexCallTerm; break;
					case 'R': code = dexRing; break;
					case 'W': code = dexBufferLow; break;
					case ';': code = dexResError; break;
					case '<': code = dexUnderrun; break;
					default:
						break;
				}
				break;
			
			case BEL:
				switch(readModem(dexp)) {
					case '0': code = dexStandardRes; break;
					case '1': code = dexFineRes; break;
					case 'R':
						*bp++ = 'R';
						while ((c = readModem(dexp)) != CR)
							*bp++ = c;
						code = dexVersion;
						break;
					default:
						break;
				}
				break;
			case FS:
				switch(readModem(dexp)) {
					case '\'': code = dex9600Baud; break;
					case 'H': code = dex7200Baud; break;
					case '0': code = dex4800Baud; break;
					case CAN: code = dex2400Baud; break;
					case '1': code = dexFineRes; break;
					case 'R':
						*bp++ = 'R';
						while ((c = readModem(dexp)) != CR)
							*bp++ = c;
						code = dexVersion;
						break;
					default:
						break;
				}
			case HT:
				readModem(dexp);	/* returns # of scan lines in err */
				code = dexScanError;
				break;
			case ACK:
				code = dexAck;
				*bp++ = readModem(dexp);
				break;

			case NAK:
				code = dexNak;
				*bp++ = readModem(dexp);
				break;
		}
	} else if (c == XXON) {
		code = dexBufEmpty;
	} else if (c == XXOFF) {
		code = dexBufFull;
	}

	if ((misc != NULL) && (bp != buf))
		*misc = (char *)buf;

	PP_LOG(LLOG_DEBUG, 
	   ("readResponse: %d: %s (%s)", code, codeToString(code), buf));

	return(code);
}

/* 
 *	physically reset dexnet by dropping DTR for 4 seconds.
 *	note: this is a *real time* constraint. DTR must be dropped
 *	for at least 3.1 seconds but not more than 5.9 seconds. Thank
 *	you very much, fujitsu.
 */
hardResetDexModem(dem)
DexModem *dem;
{
   ioctl(dem->fd, TIOCSDTR, 0);
   ioctl(dem->fd, TIOCCDTR, 0);
   sleep(4);
   ioctl(dem->fd, TIOCSDTR, 0);

   /* we don't yet know if this worked, but if it did, the speed is now 
	* 9600. If it didn't, we're screwed anyway.
	*/
   dem->highSpeed = 0;
}

char nulls[] = {'\000', '\000'};
/*
 *	Set up the modem. Return OK or NOTOK
 *	This can be called no matter what state the modem
 *	is in.
 */
setupDexModem(dem, fast)
DexModem *dem;
int		fast;	/* true if 19.2K baud */
{
#ifdef sun
	struct termios	tty_struct;
#endif
	struct sgttyb	ttyb;
	char			*p;

	fax_debug = 1;

	/*
	 *	This should bring the modem back to 9600 baud, text mode
	 */
	hardResetDexModem(dem);

	/*
	 *	Make sure tty is at 9600 baud
	 */
	doIoctl(dem, TIOCGETP, (char *)&ttyb, "TIOCGETP");
	ttyb.sg_ispeed = ttyb.sg_ospeed = B9600;
	doIoctl(dem, TIOCSETP, (char *)&ttyb, "TIOCSETP");

#ifdef sun
	/* 
	 * Set up hardware flow control 
	 */
	doIoctl(dem, TCGETS, (char *)&tty_struct, "TCGETS");
	tty_struct.c_cflag |= CRTSCTS;
	tty_struct.c_cflag |= CLOCAL;
	doIoctl(dem, TCSETS, (char *)&tty_struct, "TCSETS");
#endif

	/*
	 *	Return to factory defaults
	 */
	if (writeDexModem(dem, FACTDEF, sizeof(FACTDEF)-1, "FACTDEF:") == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS, (dem->errBuf));
		return(NOTOK);
	}
	if (readResponse(dem, NULL) != dexReady) {
		strcat(dem->errBuf, "Modem not ready (FACTDEF)");
		return(NOTOK);
	}

	/* soft reset as well */
	IssueCommand(dem, RESET, "RESET");
	if (readResponse(dem, NULL) != dexReady) {
		strcat(dem->errBuf, "Modem not ready (RESET)");
		return(NOTOK);
	}

	IssueCommand(dem, ASK_VERSION, "Check Version");
	readResponse(dem, &p);
	if ((p == NULL) || ((strcmp(p, "REV1.9") != 0) &&
			    (strcmp(p, "REV2.0") != 0))) {
		strcat(dem->errBuf, "Modem software version mismatch: expect rev1.9 or rev2.0");
		return(NOTOK);
	}

	IssueCommand(dem, SPEAKER_ON, "Set Speaker On");

	if (fast) {
		/* Send at 19200 */
		dem->highSpeed = 1;
		IssueCommand(dem, BAUD_19200, "Set baud 19200");

		/* Set the speed to 19200 */
		doIoctl(dem, TIOCGETP, (char *)&ttyb, "TIOCGETP");
		ttyb.sg_ispeed = ttyb.sg_ospeed = B19200;
		doIoctl(dem, TIOCSETP, (char *)&ttyb, "TIOCSETP");
	} else {
		dem->highSpeed = 0;
		IssueCommand(dem, BAUD_9600, "Set baud 9600");
	}

	/*
	 *	WARNING: response code parse routines now assume zero retry
	 *	will be set. This parameter changes the order and kind of
	 *	responses that the dexnet will generate
	 */
  	IssueCommand(dem, ZERO_RETRY, "Set ZERO_RETRY");

	/* make sure modem is ready */
	IssueCommand(dem, ASK_STAT, "Check Status");
	if (readResponse(dem, NULL) == dexReady)
		return(OK);
	else {
		/* try reset again (needed when running at 19200 */
		IssueCommand(dem, RESET, "RESET");
		if (readResponse(dem, NULL) == dexReady) {
			IssueCommand(dem, ZERO_RETRY, "Set ZERO_RETRY");
			return(OK);
		} else {
			strcat(dem->errBuf, "Modem not ready");
			return(NOTOK);
		}
	}

}


/* This function will dial a number, set the date, time and
	TTI strings on the page header.  A OK return signifies
	a successful completion.  A null number will cause this
	routine to return a NOTOK.  The date, time and TTI are
	optional.  To not send these, just pass in NULLs.
*/
dialDexModem(dem, snumber, sdate, stime, stti)
DexModem	*dem;
char *snumber, *sdate, *stime, *stti;
{
    /* dial number */
	if (snumber == NULL) {
		strcat(dem->errBuf, "can't dial NULL number");
		return(NOTOK);
	}

	IssueCommand(dem, DIAL_NMBR, "Dial number");
	IssueData(dem, snumber, strlen(snumber));
	IssueData(dem, "\r", 1);

	/* Send date if not NULL */
	if (sdate) {
	    IssueCommand(dem, SEND_DATE, "Send date");
	    IssueData(dem, sdate, strlen(sdate));
		IssueData(dem, "\r", 1);
	}

	/* Send the time if not NULL */
	if (stime) {
	    IssueCommand(dem, SEND_TIME, "Send time");
	    IssueData(dem, stime, strlen(stime));
		IssueData(dem, "\r", 1);
	}

	/* Send the TTI if not NULL */
	if (stti) {
	    IssueCommand(dem, SEND_TTI, "Send TTI");
	    IssueData(dem, stti, strlen(stti));
		IssueData(dem, "\r", 1);
	}
    return(OK);
}

/*
 *	Interpret response code; possible set error message in msgBuf.
 *	return OK or NOTOK depending on code received. It is assumed that
 *	if NOTOK is returned, a message will be in msgBuf. 
 */
static
processResponse(dem, code, buf)
DexModem	*dem;
int			code;
char		*buf;
{
	int	retcode = OK;
	if (buf == NULL)
		buf = "";
	    
	dem->msgCode = dexReady; /* no error */

	switch (code) {
		case dexAck: 
			PP_TRACE(("Page %s sent ok", buf));
			break;
		case dexNak: 
			PP_TRACE(("Page %s sent with errors", buf));
			dem->xmitErrs++; 
			break;
		case dexConnected:
			PP_TRACE(("Connected"));
			if (dem->stationId[0])
				PP_TRACE(("\tto station %s", dem->stationId));
			if (dem->terminalId[0])
				PP_TRACE(("\tto terminal %s", dem->terminalId));
			break;

		case dexNoResponse:
			sprintf(dem->msgBuf, "No response from remote fax");
			PP_TRACE(("%s", dem->msgBuf));
			dem->msgCode = dexNoResponse;
			retcode = NOTOK;
			break;
			
		case dexBusy:
			sprintf(dem->msgBuf, "Remote fax line is busy");
			PP_TRACE(("%s", dem->msgBuf));
			dem->msgCode = dexBusy;
			retcode = NOTOK;
			break;

		case dexSID:
			strcpy(dem->stationId, buf);
			break;

		case dexTID:
			strcpy(dem->terminalId, buf);
			break;

		case dexNotAFax:
			/* fatal message */
			sprintf(dem->msgBuf, "Remote device is not a fax");
			PP_TRACE(("%s", dem->msgBuf));
			retcode = NOTOK;
			dem->msgCode = dexNotAFax;
			break;
		
		case dexDisconnect:
			/* fatal message */
			sprintf(dem->msgBuf, "Lost connection to fax machine");
			PP_TRACE(("%s", dem->msgBuf));
			dem->msgCode = dexDisconnect;
			retcode = NOTOK;
			break;

		case dexNotReady:
			/* msgBuf already set by previous response */
			PP_TRACE(("%s", dem->msgBuf));
			retcode = NOTOK;
			break;
		
		case dexReady:
			PP_TRACE(("Call terminated normally"));
			break;
		
		default:
			PP_TRACE(("%s", codeToString(code)));
	}
	return(retcode);
}

/*
 *	Send file in mode specified (g3 or ascii). Return OK if sent,
 *	NOTOK if not sent. 
 */
sendDexModem(dem, data, dataLen, textMode)
DexModem		*dem;
unsigned char	*data;	/* data to send */
int				dataLen;	/* of buf */
int		textMode;	/* true if text, false if image */
{
	char	*mesg;
	int		retcode = OK;
	int		blockSize = 512;

	PP_TRACE(("Enter sendDexModem"));

	dem->msgBuf[0] = (char)0;
	dem->errBuf[0] = (char)0;

	/* turn on image if g3 and not running at 19.2 */
	if ((!textMode) && (!dem->highSpeed)) {
		IssueCommand(dem, IMAGE_MODE, "Set Image Mode");
	}

/* TODO: what if this blocks */
	nonBlockIO(dem->fd);

	while (dataLen > 0) {
		int				bytesToWrite;
		unsigned char	*buf;

		buf = data;
		bytesToWrite = dataLen > blockSize ? blockSize : dataLen;
		data += bytesToWrite;
		dataLen -= bytesToWrite;
	
		while (1) {
			int	cc;
			cc = write(dem->fd, buf, bytesToWrite);
			if (cc == bytesToWrite)
				break;
			else if ((cc == -1) && ((errno == EWOULDBLOCK) || (errno == EAGAIN))) {
				fd_set	readfds, writefds;

				FD_ZERO(&readfds);
				FD_ZERO(&writefds);
				FD_SET(dem->fd, &readfds);
				FD_SET(dem->fd, &writefds);

				if (select(dem->fd+1, &readfds, &writefds, NULL, NULL) < 0) {
					sprintf(dem->errBuf, "select: %s", sys_errname(errno));
					retcode = NOTOK;
					goto quit;
				}
				if (FD_ISSET(dem->fd, &readfds)) {
					int	code;

					code = readResponse(dem, &mesg);
					retcode = processResponse(dem, code, mesg);
					if (retcode == NOTOK) goto quit;
				}
			} else if (cc > 0) {
				bytesToWrite -= cc;
				buf += cc;
			} else {
				sprintf(dem->errBuf, "write: %s", 
					sys_errname(errno));
				retcode = NOTOK;
				goto quit;
			}
		}
	}

quit:
	PP_TRACE(("Leave sendDexModem, ret %d", retcode));
	return(retcode);
}

eomDexModem(dem)
DexModem	*dem;
{
	int		retcode = OK;
	int		code;

	blockIO(dem->fd);

	if (writeDexModem(dem, END_OF_TRANS, sizeof(END_OF_TRANS)-1, 
		"send EOT") == NOTOK) {
		sprintf(dem->msgBuf, "Error communicating with local modem");
		return(NOTOK);
	}

	do {
		char	*mesg = NULL;
		code = readResponse(dem, &mesg);
		retcode = processResponse(dem, code, mesg);
	} while (!((retcode == NOTOK) || (code == dexReady)));

	return(retcode);
}

closeDexModem(dem)
DexModem	*dem;
{
	if (dem->highSpeed)
		IssueCommand(dem, BAUD_9600, "Set baud 9600");
	close(dem->fd);
	return(OK);
}
