/* faxgeneric.h: generic fax structure */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Chans/fax/RCS/faxgeneric.h,v 6.0 1991/12/18 20:07:09 jpo Rel $
 *
 * $Log: faxgeneric.h,v $
 * Revision 6.0  1991/12/18  20:07:09  jpo
 * Release 6.0
 *
 *
 */

#ifndef	_H_FAXGEN
#define _H_FAXGEN

/*
 *	The idea is that these routines only need to know about the specific
 *	fax modem. They do not need to know how to generate delivery reports,
 * 	whether it is the first or second DR, etc. 
 *
 *	The softc ptr allows each fax modem to have a specific data structure
 *
 *	open (FaxCtlr *)
 *		called once to open the fax device. 
 *		should return OK or NOTOK.
 *	close (FaxCtlr *)
 *		called when channel is shutting down. 
 *		return not checked.
 *	initXmit (FaxCtlr *)
 *		called once for each recipient, before any data is sent. 
 *		called after phone number is determined and set in FaxCtlr *, 
 *		normally will send dial string down to modem. 
 *		returns PP return code
 *	abort (FaxCtlr *)
 *		called at any time to abort transmission. 
 *		return not checked.
 *	termXmit (FaxCtlr *)
 *		called after all pages have been sent to recipient. 
 *		returns OK or NOTOK
 *	setG3Params (FaxCtlr *, struct type_IOB_G3FacsimileParameters *params)
 *		called once per page to set/test G3 parameters including 
 *		resolution of page. also initialises modem for g3fax 
 *		transmission
 *		returns OK or NOTOK
 *	sendPage (FaxCtlr *, char *bits, int len, int last)
 *		called to send one page of G3 fax.
 *		forth parameter indicates whether last bit of fax message.
 *		note that G3 fax page encoded in X.400 order
 *		returns OK or NOTOK
 *	setIA5Params (FaxCtlr *)
 *		call obnce per file of ia5 to initialise modem for ia5 
 *		transmission
 *		return OK or NOTOK;
 *	sendIA5File (FaxCtlr *, char *file, int last)
 *		send one file of IA5 text. 
 *		third parameter indicates whether last bit of fax message.
 *		returns OK or NOTOK
 *	arg_parse (FaxCtlr *, char *key, char *value)
 *		parse an info line argument
 *		return OK or NOTOK
 *
 *	Whenever any routine returns an error status, it should place a
 *	error message into the field 'errBuf'. 
 *
 *	If initXmit(), sendPage(), sendIA5File(), or termXmit() return an 
 * 	error status, they
 *	must also set the field 'qmgrErrCode'. This code is used to pass 
 *	error status information back to the qmgr. The only values that
 *	should be returned in this field are
 *		int_Qmgr_status_negativeDR - permanent failure; 
 *			send DR
 *		int_Qmgr_status_messageFailure - transient failure, 
 *			retry message
 *		int_Qmgr_status_mtaFailure - transient failure, 
 *			retry channel 
 *	In the case of the permanent failure, the generic fax driver code
 *	will worry about converting a negativeDR status into a shared DR
 *	status (if necessary). Also, the generic fax code will automatically
 *	build an error report (with 'errBuf' as the message).
 */

/*
 *	Generic control data structure for each fax modem
 */
typedef struct _faxCtlr {
	char	*softc;		/* controller specific state */
	int	(*open)();	/* open routine */
	int	(*close)();	/* close routine */
	int	(*arg_parse)();	/* parse info line argument */
	char	errBuf[BUFSIZ];	/* error messages go here */
	int	qmgrErrCode;	/* passed back to Qmgr */	

/* outbound parameters and function pointers */
	char	telno[255];	/* telephone number */
	int	(*initXmit)();	/* initialize for transmission */
	int	(*abort)();	/* abort transmission */
	int	(*termXmit)();	/* terminate transmission */
	int	(*setG3Params)();	/* set G3 fax parameters */
	int	(*sendPage)();	/* send one page of G3 fax */
	int	(*setIA5Params)();	/* set of ia5 transmission */
	int	(*sendIA5File)(); /* send one file of IA5 text */

/* inbound parameters and function pointers */
	char	channel[BUFSIZ];	/* inbound channel name */
	char	fax_recip[BUFSIZ];	/* recip address for incoming faxes */
	char	subject[BUFSIZ];	/* standard string for subject */
					/* strings on submitted messages */
	int	(*receiveG3Fax)();	/* wait and get incoming fax */
} FaxCtlr;

#define NULLFAXCTRLR	((FaxCtlr *)0)

extern	FaxCtlr	*int_faxctrlr();

#define BITSPERBYTE	8


#define FAX_OUTBOUND	0
#define	FAX_INBOUND	1
#define FAX_BOTH	2
#endif
