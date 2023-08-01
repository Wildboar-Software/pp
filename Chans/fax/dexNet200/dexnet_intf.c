/* dexnet_intf.c: interfcae routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/dexnet_intf.c,v 6.0 1991/12/18 20:08:53 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/dexnet_intf.c,v 6.0 1991/12/18 20:08:53 jpo Rel $
 *
 * $Log: dexnet_intf.c,v $
 * Revision 6.0  1991/12/18  20:08:53  jpo
 * Release 6.0
 *
 */

/* Interface to the Dexnet 200 Fax Modem */

#include	"util.h"
#include	"retcode.h"
#include	"adr.h"
#include	"qmgr.h"
#include 	<isode/cmd_srch.h>
#include 	"tb_bpt88.h"
#include	"IOB-types.h"
#include	"MTA-types.h"
#include	"or.h"
#include 	<varargs.h>
#include	<sys/termios.h>
#include	<sys/stat.h>
#include	"../faxgeneric.h"
#include 	"dexnet.h"

#define DEM(faxctl) ((DexModem *)((faxctl)->softc))

extern CHAN	*mychan;
static int	numPages;

/* 
 *	Open dexNet device and initialize
 *	Return OK or NOTOK
 */
dexOpen(faxctl)
FaxCtlr		*faxctl;
{
	DexModem	*dem = DEM(faxctl);
	int	ret;

	ret = openDexModem(dem);
	if (ret != OK) {
		PP_LOG(LLOG_EXCEPTIONS, ("dexOpen: %s", dem->errBuf));
		strcpy(faxctl->errBuf, "Can't open fax device");
		return(ret);
	}
	return OK;
}

/* close dexNet */
dexClose(faxctl)
FaxCtlr		*faxctl;
{
	DexModem	*dem = DEM(faxctl);
	closeDexModem(dem);
}

/*
 *	Copy the phone number from src to dst. Return the address of the next
 *	unused byte in dst. Filter src so as to only copy the following
 *	characters:
 *		0-9, comma (indicates pause), W (indicates wait for dialtone)
 *		P is changed to comma
 *		All other characters are ignored.
 */
static char *
copyPhoneNumber(src, dst)
char	*src, *dst;
{
	for (; *src; src++) {
		if (isdigit(*src) || (*src == ',') || (*src == 'W'))
			*dst++ = *src;
		else if (*src == 'P')
			*dst++ = ',';
	}
	return(dst);
}


#define FAX_TELNOSIZE	28

static int localCall = 7;	
static int longDistance = 10;

/*
 *	Normalize the phone number. In general, copy the phone number 'from' 
 *	into 'to', skipping "phone" whitespace (see macro isPhoneWhite).
 *
 *	Add prefix for local access, long distance and international calls.
 *	Disallow long distance and international calls if necessary.
 *
 *	Return OK or NOTOK and set faxctl->errBuf
 *
 *	NOTE: this is completely USA-centric
 */
static int convertPhoneNumber(faxctl, from, to)
FaxCtlr	*faxctl;
char	*from, *to;
{
	char	*ifrom = from, *ito = to;
	DexModem	*dem = DEM(faxctl);
	int		digits = 0;

	/* count the actual number of digits */
	for (;*ifrom;ifrom++)
		if (isdigit(*ifrom))
			digits++;
	
	if (digits < localCall) {
		;	/* do nothing */
	} else if (digits == localCall) {
		/* add local access if defined */
		if (dem->localPrefix)
			ito = copyPhoneNumber(dem->localPrefix, ito);
	} else if (digits == longDistance) {
		if (dem->noLongDistance) {
			sprintf(faxctl->errBuf, "Long distance calls not allowed");
			return(NOTOK);
		} else if (dem->longdPrefix)
			ito = copyPhoneNumber(dem->longdPrefix, ito);
	} else if (digits > longDistance) {
		char	*plus = index(from, '+');
		if (!plus) {
			sprintf(faxctl->errBuf, "International call not flagged with +");
			return(NOTOK);
		}
		if (dem->noInternational) {
			sprintf(faxctl->errBuf, "International calls not allowed");
			return(NOTOK);
		}
		if (dem->intlPrefix)
			ito = copyPhoneNumber(dem->intlPrefix, ito);
	} else {
		sprintf(faxctl->errBuf, "Invalid telephone number with %d digits", digits);
		return(NOTOK);
	}

	ito = copyPhoneNumber(from, ito);
	*ito = '\0';

	return OK;
}

/*
 *	Initialize for transmission 
 *	Returns: PP return code
 */
dexInitXmit(faxctl)
FaxCtlr		*faxctl;
{
	DexModem	*dem = DEM(faxctl);
	char    now[30], telno[FAX_TELNOSIZE];
	char    *date, *tim;
	time_t  clock;
	int		ret = RP_BOK;

	faxctl->qmgrErrCode = int_Qmgr_status_mtaFailure;
	numPages = 0;

	if (convertPhoneNumber(faxctl, faxctl->telno, telno) != OK) {
		faxctl->qmgrErrCode = int_Qmgr_status_negativeDR;
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		return(RP_MECH);
	}

	PP_TRACE(("Dialing %s", telno));
	
	/* currently setup at 19.2K */
	if (setupDexModem(dem, 1) != OK) {
		PP_LOG(LLOG_EXCEPTIONS, ("dexInitXmit: setup %s", dem->errBuf));
		strcpy(faxctl->errBuf, "Can't setup fax device");
		return(RP_MECH);
	}

	time(&clock);
	strcpy(now, ctime(&clock));
	date = now;
	date[10] = (char)0;
	tim = &now[11];
	tim[8] = (char)0;

	if (dialDexModem(dem, telno, date, tim, dem->tti) != OK) {
		PP_LOG(LLOG_EXCEPTIONS, ("dexOpen: dial %s", dem->errBuf));
		strcpy(faxctl->errBuf, "Can't dial remote fax device");
		ret = RP_MECH;
	}
	return(ret);
}

/* 
 *	Abort the call. It would be nice to just send a RESET to the modem,
 *	but it may be in la-la land with CTS off, so the only sure-fire way
 * 	to reset is to use the hardware reset (drop DTR for a bit). Arrgh!
 */
dexAbort(faxctl)
FaxCtlr		*faxctl;
{
	DexModem	*dem = DEM(faxctl);
 	hardResetDexModem(dem);
}

/*
 *	Terminate transmission to remote fax
 *	Returns: OK or NOTOK
 */
dexTermXmit(faxctl)
FaxCtlr		*faxctl;
{
	DexModem	*dem = DEM(faxctl);
	int			ret = OK;

	if ((ret = eomDexModem(dem)) != OK) {
		/* based on actual error, either retry or abort */
		if (dem->msgCode == dexNotAFax) {
		    faxctl->qmgrErrCode = int_Qmgr_status_negativeDR;
		} else {
		    faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
		}
		PP_LOG(LLOG_EXCEPTIONS, ("dexTermXmit: dial %s", dem->msgBuf));
		strcpy(faxctl->errBuf, dem->msgBuf);
		ret = NOTOK;
	}
	return(ret);
}

/*
 *	Set/Test the G3 Parameters
 */

dexSetParams(faxctl, params)
FaxCtlr		*faxctl;
struct type_IOB_G3FacsimileParameters *params;
{
	DexModem		*dem = DEM(faxctl);

	if (params -> non__basic__parameters) {
		if (bit_test(params -> non__basic__parameters,
                             bit_MTA_G3FacsimileNonBasicParameters_two__dimensional) == 1) {
                        PP_LOG(LLOG_EXCEPTIONS,
                               ("Unable to deal with two dimensional fax"));
			sprintf(faxctl->errBuf,
				"Unable to deal with two dimensional fax");
                        return NOTOK;
                }

                if (bit_test(params -> non__basic__parameters,
                             bit_MTA_G3FacsimileNonBasicParameters_uncompressed) == 1) {
                        PP_LOG(LLOG_EXCEPTIONS,
                               ("Unable to deal with uncompressed fax"));
			sprintf(faxctl->errBuf,
				"Unable to deal with uncompressed fax");
                        return NOTOK;
                }
                
                if (bit_test(params -> non__basic__parameters,
                             bit_MTA_G3FacsimileNonBasicParameters_fine__resolution) == 1) 
			IssueData(dem, FINE_RES, "FINE_RES");

	}
	return OK;
}

/* set correct error code based upon dexnet errors */
dexGenQmgrError(faxctl)
FaxCtlr	*faxctl;
{
	DexModem		*dem = DEM(faxctl);
	if (dem->msgBuf[0]) {
		strcpy(faxctl->errBuf, dem->msgBuf);
		switch (dem->msgCode) {
			case dexNotAFax:
				faxctl->qmgrErrCode = int_Qmgr_status_negativeDR;
				break;
			case dexNoResponse: /* no answer, retry message */
			case dexBusy:
			case dexDisconnect: /* dropped line */
				faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
				break;
			/* all else gets MTA failure */
			default:
				faxctl->qmgrErrCode = int_Qmgr_status_mtaFailure;
				break;
		}
	} else {
		PP_LOG(LLOG_EXCEPTIONS,
			("Error sending page: %s", dem->errBuf));
		strcpy(faxctl->errBuf, "Error communicating with fax modem");
	}
}

/*
 *	Send one page of G3 fax
 *	Returns: OK or NOTOK
 */
dexSendPage(faxctl, str, len, last)
FaxCtlr		*faxctl;
char		*str;		/* g3 fax data to send */
int			len;		/* len of str in bytes */
int			last;		/* true if last page */
{
	DexModem		*dem = DEM(faxctl);
	unsigned char	*fuji;
	int				fujiLen;
	int				ret = OK;

	faxctl->qmgrErrCode = int_Qmgr_status_mtaFailure;

	if ((dem->maxPages > 0) && (numPages++ > dem->maxPages)) {
		/* have exceeded page count; stop transmission */
		faxctl->qmgrErrCode = int_Qmgr_status_negativeDR;
		sprintf(faxctl->errBuf, "Transmission truncated: pagecount %d exceeded",
			dem->maxPages);
		return(NOTOK);
	}

	/* 
	 *	Convert G3 fax to (stupidly encoded) dexNet coding.
	 *	conversion will add 4 bytes for every scanline. Thus
	 *	about 1800 scanlines * 4 < 8000
	 */
	fuji = (unsigned char *)malloc(len + 8000);
	fujiLen = fujicnvrt(str, fuji, len);
	ret = sendDexModem(dem, fuji, fujiLen, /* G3 mode */0);
	free(fuji);

	if (ret != OK) {
		dexGenQmgrError(faxctl);
	}
	return(ret);
}

/*
 *	Open file and send as text to fax modem. 
 *	Reads entire file into memory, converts each linefeed to carriage
 *	return, linefeed. Return OK or NOTOK
 */
dexSendIA5File(faxctl, fileName, last)
FaxCtlr		*faxctl;
char		*fileName;	/* file to send */
int			last;		/* true if last bodypart */
{
	struct stat		sbuf;
	unsigned char	*buf;
	int				len = 0, bufLen;
	int				lines = 0;
	unsigned char 	*p;
	int				cc;
	FILE			*fp;
	int				ret;
	DexModem		*dem = DEM(faxctl);

	faxctl->qmgrErrCode = int_Qmgr_status_mtaFailure;

	if ((fp = fopen(fileName, "r")) == NULL) {
		PP_SLOG(LLOG_EXCEPTIONS, fileName,
			("Can't open file"));
		strcpy(faxctl->errBuf, "Error communicating with fax modem");
		return(NOTOK);
	}

	fstat(fileno(fp), &sbuf);
	bufLen = sbuf.st_size*2;	/* worst case estimate */
	p = buf = (unsigned char *)malloc(bufLen);
	bzero(buf, bufLen);

	while (fgets(p, bufLen - len, fp) != NULL) {
		cc = strlen(p);
		p[cc++] = '\r';	/* add carriage return */
		len += cc;
		p += cc;
		lines++;
	}

	/* assume 66 lines per page */
	if ((dem->maxPages > 0) && ((lines/66) > dem->maxPages)) {
		/* exceeded page count */
		faxctl->qmgrErrCode = int_Qmgr_status_negativeDR;
		sprintf(faxctl->errBuf, "Attempt to send more than %d pages",
			dem->maxPages);
		free (buf);
		return(NOTOK);
	}

	ret = sendDexModem(dem, buf, len, /* textmode true */ 1);
	free(buf);

	if (ret != OK) {
		dexGenQmgrError(faxctl);
	}
	return(ret);
}

/*
 * parse info line arguments
 *
 *	out: name of device
 *	tti: ng tti field
 *	noLong: disallow long distance calls
 *	noIntl: disallow international calls
 *	localPrefix: local access prefix (i.e., outside line)
 *	longPrefix: long distance access prefix
 *	intlPrefix: international prefix
 *	max_pages: limit number of pages sent to this value
 */
int dexArgParse (faxctl, key, value)
FaxCtlr	*faxctl;
char	*key;
char	*value;
{
	DexModem	*dem = DEM(faxctl);

	if (lexequ(key, "out") == 0)
		strcpy(dem->devName, value);
	else if (lexequ(key, "tti") == 0)
		strcpy(dem->tti, value);
	else if (lexequ(key, "localPrefix") == 0)
		dem->localPrefix = strdup(value);
	else if (lexequ(key, "longPrefix") == 0)
		dem->longdPrefix = strdup(value);
	else if (lexequ(key, "intlPrefix") == 0)
		dem->intlPrefix = strdup(value);
	else if (lexequ(key, "long") == 0)
		if (lexequ(value, "ok") == 0)
			dem->noLongDistance = 0;
		else
			dem->noLongDistance = 1;
	else if (lexequ(key, "intl") == 0)
		if (lexequ(value, "ok") == 0)
			dem->noInternational = 0;
		else
			dem->noInternational = 1;
	else if (lexequ(key, "maxPages") == 0)
		dem->maxPages = atoi(value);
	else {
		sprintf(faxctl->errBuf,
		       "Unknown info arg '%s=%s",
			key, value);
		return NOTOK;
	}
	return OK;
}

/*
 *	Return a fax control block
 */

FaxCtlr	*init_faxctrlr()
{
	FaxCtlr	*faxctl;
	DexModem	*dem;

	faxctl = (FaxCtlr *)malloc(sizeof(FaxCtlr));
        bzero(faxctl, sizeof(FaxCtlr));

	dem = (DexModem *)malloc(sizeof(DexModem));
	bzero(dem, sizeof(DexModem));
	sprintf(dem->devName, "/dev/tty01"); /* default for now */
	faxctl->softc = (char *) dem;

        faxctl->open = dexOpen;
        faxctl->close = dexClose;
        faxctl->initXmit = dexInitXmit;
        faxctl->abort = dexAbort;
        faxctl->termXmit = dexTermXmit;
        faxctl->setG3Params = dexSetParams;
        faxctl->sendPage = dexSendPage;
	faxctl->setIA5Params = NULLIFP;
        faxctl->sendIA5File = dexSendIA5File;
	faxctl->arg_parse = dexArgParse;
	return faxctl;
}
