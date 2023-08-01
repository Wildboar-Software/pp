/* faxcodes.h: codes for the Fujitsu */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/dexnet.h,v 6.0 1991/12/18 20:08:53 jpo Rel $
 *
 * $Log: dexnet.h,v $
 * Revision 6.0  1991/12/18  20:08:53  jpo
 * Release 6.0
 *
 *
 */

/* 

	This file contains the fax codes for the Fujitsu dexNet200.

	HISTORY:

				?/?/?	J. Knowles		Created.
				3/24/91		S. Berdia		Revamped.
				11/1/91		R. Hagens		Rewacked
				12/4/91		P. Nikander		Modified.
*/
/* modem control codes */
#define ANS_OFF     	{27,3,0}    		/* set answer mode off 			*/
#define IMAGE_MODE  	"\033\031\001"  	/* set image mode 				*/
#define TEXT_MODE   	"\033\031\000"   	/* set text mode 				*/
#define COMM_MODE   	"\033\031\002"		/* set comm mode 				*/
#define CTS     		"\033\040\001"   	/* set CTS flow control 		*/
#define ZERO_RETRY 	 	"\033\144\000" 		/* set retries to zero 			*/
#define DIAL_NMBR   	"\033\002"  		/* dial number 					*/
#define END_OF_TRANS    "\033\004"  		/* EOT string 					*/
#define ASK_STAT    	"\033\005"  		/* query fax status 			*/
#define BAUD_9600   	"\033\022"  		/* set baud rate to 9600 		*/
#define BAUD_19200  	"\033\024"  		/* set baud rate to 19200 		*/
#define FINE_RES    	"\033\033"  		/* set fine resolution 			*/
#define SEND_TTI    	"\033\034"  		/* send TTI for header 			*/
#define SEND_TIME   	"\033\036"  		/* send time for header 		*/
#define SEND_DATE   	"\033\037"  		/* send date for header 		*/
#define XON_XOFF    	"\033\040\001"  	/* set XON/XOFF flow control 	*/
#define FACTDEF       	"\033\364\033\360\033\365"/* reset factory deflt */
#define RESET			"\033\354"			/* reset */
#define NOSTEPUP		"\033\026\000"		/* disable STEUP on RTP			*/
#define MH_START		"\033\053"			/* start of MH image			*/
#define	MH_END			"\000\000"			/* End of MH image				*/
#define SEND_POLL		"\033\027"			/* Enable send polling			*/
#define ASK_VERSION		"\033\367\367"		/* query version */
#define MONITOR			"\033\003\005"		/* monitor line state */
#define CUTPAGE			"\033\014"			/* cut page immediately */
#define UNDERRUN		"\033\003\127"		/* turn on underrun diag */

#define SPEAKER_ON		"\033\030\002"		/* turn on speaker */

/* printer control codes */
#define BOLD_EXPND  	"\033!4\r"  		/* set bold expand 				*/
#define THIN_LINE   	'\304'      		/* thin line char code 			*/
#define THICK_LINE  	'\337'      		/* thick line char code 		*/
#define EXP_COMP_OFF    {27,87,0,'\222',12} /* expand/compress off, FF 		*/


#define	ESC		0x1b
#define	DLE		0x10
#define	BEL		0x07
#define	ACK		0x06
#define	NAK		0x15
#define	FS		0x1c
#define	HT		0x09
#define	CR		0x0d
#define CAN		0x18
#define	XXON	0x11
#define XXOFF	0x13

/*
 *	dexNET 200 error response
 */
#define	dexUnknown		0
#define	dexNotReady		1
#define dexReady		2
#define	dexConnected	3
#define dexWaiting		4
#define	dexDialing		5
#define	dexSID			6
#define	dexTID			7
#define	dexStandardRes	8
#define	dexFineRes		9
#define dexCallTerm		10
#define	dexAck			11
#define dexNak			12
#define dexRing			13
#define	dexBufferLow	14
#define	dex9600Baud		15
#define	dex7200Baud		16
#define	dex4800Baud		17
#define	dex2400Baud		18
#define dexScanError	19
#define	dexVersion		20
#define dexBusy			21
#define	dexNoResponse	22
#define dexNotAFax		23
#define dexDisconnect	24
#define dexResError		25
#define dexUnderrun		26
#define	dexBufFull		27
#define	dexBufEmpty		28

#define	IssueCommand(fd, cmd, msg)\
	if (writeDexModem(fd, cmd, sizeof(cmd)-1, msg) == NOTOK)\
		return(NOTOK);

#define	IssueData(fd, cmd, len)\
	if (writeDexModem(fd, cmd, len, "IssueData") == NOTOK)\
		return(NOTOK);

typedef struct _dexModem {
	int		fd;				/* fd open to modem */
	char	devName[255];	/* name of device */
	int		highSpeed;		/* true if talk at 19.2K baud */
	int		xmitErrs;		/* errors encountered during transmission */
	char	msgBuf[255];	/* user readable error message stored here */
	char	errBuf[255];	/* unix i.e., cryptic error message stored here */
	char	stationId[25];	/* station identification */
	char	terminalId[25];	/* terminal identification */
	int	msgCode;	/* code accompanies reason in msgBuf */

	char	tti[255];	/* outgoing tti */
	char	*intlPrefix;	/* international prefix */
	char	*longdPrefix;	/* long distance prefix */
	char	*localPrefix;	/* local access prefix (outside line) */
	char	noLongDistance;	/* access control */
	char	noInternational;/* access control */
	int		maxPages;		/* limit number of pages sent */
} DexModem;

extern char			*codeToString();
