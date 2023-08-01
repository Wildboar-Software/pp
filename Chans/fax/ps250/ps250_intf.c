/* ps250_intf.c: interface routines for Panasonic 250 driver */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_intf.c,v 6.0 1991/12/18 20:07:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_intf.c,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: ps250_intf.c,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 */
#include	"util.h"
#include	"retcode.h"
#include	"qmgr.h"
#include	"chan.h"
#include	"IOB-types.h"
#include	"../faxgeneric.h"
#include 	"ps250.h"

/*
 *	Heuristics for whether to bounce or not on failure
 */

extern int table_verified;

static void isPermanentFail(faxctl)
FaxCtlr	*faxctl;
{
	PsModem	*psm = PSM(faxctl);
	/* set qmgrErrCode to negativeDR in certain situations */
	
	if (psm->connected == FALSE)
		/* no connection so not permanent */
		faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
	else {
		/* connected */
		if (table_verified == TRUE)
			/* in table so not permanent */
			faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
		else
			faxctl->qmgrErrCode = int_Qmgr_status_negativeDR;
	}
}

/*
 * 	Open ps250 device and initialize
 *	Return OK or NOTOK
 */

psOpen (faxctl)
FaxCtlr	*faxctl;
{
	PsModem	*psm = PSM(faxctl);
	
	if (openPsModem(psm) != OK) {
		sprintf(faxctl->errBuf,
			"openPsModem failed [%s]",
			psm->errBuf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		return NOTOK;
	}
	return OK;
}

/*
 * 	Close fax device
 */

psClose (faxctl)
FaxCtlr	*faxctl;
{
	PsModem	*psm = PSM(faxctl);
	
	return closePsModem(psm);
}

/*
 *	Initialise for transmission
 */

extern CHAN	*mychan;

static int isValidPhoneChar (ch)
char	ch;
{
	if (isdigit(ch)
	    || isspace(ch))
		return OK;
	switch (ch) {
	    case '-':
/*	case '+': tested for explicitly */
		return OK;
	    default:
		break;
	}
	return NOTOK;
}
		
static int convertPhoneNumber(psm, from, to, pfatal)
PsModem	*psm;
char	*from;
char	*to;
int	*pfatal;
{
	char	*ifrom = from, *ito = to;
	char	intprefix[FAX_TELNOSIZE];
#define	INT_PREFIX	"int_prefix"	
#define	PAUSE		0x3F
	while (*ifrom != '\0' && isspace(*ifrom)) *ifrom++;
	
	*pfatal = FALSE;

	if (psm->phone_prefix) {
		/* insert phone prefix */
		char *ix = psm->phone_prefix;
		while (*ix != '\0' && (ito - to) < FAX_TELNOSIZE) {
			if (isValidPhoneChar(*ix) != OK) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Illegal phone character '%c' in prefix '%s'",
					*ix, psm->phone_prefix));
				to[0] = '\0';
				sprintf(psm->errBuf,
					"Illegal character '%c'",
					*ix);
				return NOTOK;
			}
			if (isspace(*ix) ||
			    *ix == '-')
				ito++;
			else
				*ito++ = *ix++;
		}
	}

	if (*ifrom == '+') {
		/* insert national prefix */
		ifrom++;
		if (mychan && 
		    mychan -> ch_table && 
		    tb_k2val (mychan -> ch_table, 
			      INT_PREFIX, intprefix, TRUE) != NOTOK) {
			char	*ix = &(intprefix[0]);
			while (*ix != '\0' && 
			       (ito - to) < FAX_TELNOSIZE) {
				if (isValidPhoneChar(*ix) != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Illegal phone character '%c' in international prefix '%s'",
						*ix, intprefix));
					to[0] = '\0';
					sprintf(psm->errBuf,
						"Illegal character '%c'",
						*ix);
					return NOTOK;
				}
				if (isspace(*ix) ||
				    *ix == '-')
					ito++;
				else
					*ito++ = *ix++;
				
			}
		}
	}

	/* any errors from here in are bad errors */
	*pfatal = TRUE;
	while (*ifrom != '\0' && 
	       (ito - to) < FAX_TELNOSIZE) {
		if (isValidPhoneChar(*ifrom) != OK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Illegal phone character '%c' in  '%s'",
				*ifrom, from));
			sprintf(psm->errBuf,
				"Illegal character '%c'",
				*ifrom);
			to[0] ='\0';
			return NOTOK;
		}
		if (isspace(*ifrom) ||
		    *ifrom == '-')
			ifrom++;
		else if (*ifrom == 'P' 
			 || *ifrom == 'p') {
			*ito = PAUSE;
			*ifrom++;
		} else
			*ito++ = *ifrom++;
	}
	if ((ito - to) >= FAX_TELNOSIZE) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Phone number '%s' is too long. Maximum is %d",
			from, FAX_TELNOSIZE));
		sprintf(psm->errBuf,
			"Number too long (max=%d)",
			FAX_TELNOSIZE);
		to[0] = '\0';
		return NOTOK;
	}
	*ito = '\0';
	return OK;
}

psInitXmit (faxctl)
FaxCtlr	*faxctl;
{
	char	telno[FAX_TELNOSIZE];
	int 	drit = FALSE;
	PsModem	*psm = PSM(faxctl);

	faxctl->qmgrErrCode = int_Qmgr_status_messageFailure;
	
	if (convertPhoneNumber (psm, faxctl->telno, telno, &drit) != OK) {
		sprintf(faxctl->errBuf,
			"Unable to convert telephone number '%s' [%s]",
			faxctl->telno, psm->errBuf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		if (drit == TRUE
		    && table_verified != TRUE)
			/* bad user given phone number */
			faxctl->qmgrErrCode = int_Qmgr_status_negativeDR;
		return RP_MECH;
	}

	if (dialPsModem(psm, telno) != OK) {
		sprintf(faxctl->errBuf,
			"Unable to initial Xmit [%s]",
			psm->errBuf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		isPermanentFail(faxctl);
		return NOTOK;
	}
	return OK;
}

/*
 *	Abort the current call to the fax machine
 */

psAbort (faxctl)
FaxCtlr	*faxctl;
{
	PsModem	*psm = PSM(faxctl);
	abortPsModem(psm);
}

/*
 *	Terminate transmission
 */

psTermXmit (faxctl)
FaxCtlr	*faxctl;
{
	PsModem	*psm = PSM(faxctl);
	if (eomPsModem(psm) != OK) {
		sprintf(faxctl->errBuf,
			"Bad termination of transmission [%s]",
			psm->errBuf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		isPermanentFail(faxctl);
		return NOTOK;
	}
	return OK;
}

/*
 *	Test/Set G3 Fax Parameters
 */

psSetG3Params (faxctl, params)
FaxCtlr	*faxctl;
struct type_IOB_G3FacsimileParameters *params;
{
        PsModem	*psm = PSM(faxctl);

        if (params -> non__basic__parameters) {
                if (bit_test(params -> non__basic__parameters,
                             bit_MTA_G3FacsimileNonBasicParameters_two__dimensional) == 1) {
                        sprintf(faxctl->errBuf,
                                "Unable to deal with two dimensional fax");
                        PP_LOG(LLOG_EXCEPTIONS,
                               ("%s", faxctl->errBuf));
                        return NOTOK;
                }

                if (bit_test(params -> non__basic__parameters,
                             bit_MTA_G3FacsimileNonBasicParameters_uncompressed) == 1) {
                        sprintf(faxctl->errBuf,
                                "Unable to deal with uncompressed fax");
                        PP_LOG(LLOG_EXCEPTIONS,
                               ("%s", faxctl->errBuf));
                        return NOTOK;
                }
                
                if (bit_test(params -> non__basic__parameters,
                             bit_MTA_G3FacsimileNonBasicParameters_fine__resolution) == 1) {
			sprintf(faxctl->errBuf,
				"Unable to deal with fine resolution");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", faxctl->errBuf));
			return NOTOK;
		}
        }
	if (psModemG3init(psm) != OK) {
		sprintf(faxctl->errBuf,
			"Failed to initialise modem for fax transmission [%s]",
			psm->errBuf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		return NOTOK;
	}
        return OK;
}

/*
 *	Send page of G3 fax to fax modem
 */


/*
 *	reverse bit orderings
 */

void reverse_strb(str, len)
char	str[];
int	len;
{
	int	i;
	for (i = 0; i < len; i++)
		str[i] = flip_bits((unsigned char) str[i]);
}

psSendPage (faxctl, str, len, last)
FaxCtlr	*faxctl;
char	*str;
int	len;
int	last;
{
	PsModem	*psm = PSM(faxctl);

	/* G3Fax encoding is reverse bit ordering of X400 encoding */
	reverse_strb(str, len);

	return sendPsModem(psm, str, len, last);
}

/*
 *	Set IA5 transmission parameters
 */

psSetIA5Params (faxctl)
FaxCtlr	*faxctl;
{
	PsModem	*psm = PSM(faxctl);

	if (psModemIA5init(psm) != OK) {
		sprintf(faxctl->errBuf,
			"Failed to initialise modem for ia5 transmission [%s]",
			psm->errBuf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		return NOTOK;
	}
        return OK;
}
	
/*
 *	Send page of ia5 to fax modem
 */

psSendIA5File (faxctl, fileName, last)
FaxCtlr	*faxctl;
char	*fileName;
int	last;
{
	PsModem	*psm = PSM(faxctl);
	FILE	*fp;

	if ((fp = fopen (fileName, "r")) == (FILE *) 0) {
		PP_SLOG (LLOG_EXCEPTIONS, fileName,
			 ("Can't open file"));
		sprintf(faxctl->errBuf,
			"Unable to open IA5 file '%s'",
			fileName);
		return NOTOK;
	}

	if (sendPsModemIA5 (psm, fp, last) != OK) {
		sprintf(faxctl->errBuf,
			"failed to send ia5 file [%s]",
			psm->errBuf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", faxctl->errBuf));
		fclose(fp);
		return NOTOK;
	} 
	fclose(fp);
	return OK;
}

/*
 *	Wait for and store inbound faxes
 */

psReceiveG3Fax (faxctl, pg3fax)
FaxCtlr	*faxctl;
struct type_IOB_G3FacsimileBodyPart     **pg3fax;
{
	PsModem	*psm = PSM(faxctl);
	/* open */
	if (faxctl->open != NULLIFP
	    && (*faxctl->open) (faxctl) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("fax open failed ['%s']",
			faxctl->errBuf));
		return NOTOK;
	}
	/* wait */
	if (ps_wait_for_fax (psm) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("ps_wait_for_fax failed ['%s']",
			faxctl->errBuf));
		return NOTOK;
	}
	/* scan */
	if (ps_init_listen (psm) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("ps_init_fax_listen failed ['%s']",
			faxctl->errBuf));
		return NOTOK;
	}
	ps_wait_for_data(psm);
	
	if (rcvPsModem (psm, pg3fax) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("ps_fax_read_bitstrings failed ['%s']",
			faxctl->errBuf));
		return NOTOK;
	}
	/* close */
	if (faxctl->close != NULLIFP)
		(*faxctl->close)(faxctl);

	return OK;
}

/*
 *	Parse out-info line arguments
 */

psArgParseOut (faxctl, key, value)
FaxCtlr	*faxctl;
char	*key;
char	*value;
{
	PsModem	*psm = PSM(faxctl);

	if (!isstr(value)) {
		sprintf(faxctl->errBuf,
			"No value given with key '%s'",
			key);
		PP_LOG (LLOG_EXCEPTIONS,
			("%s", faxctl->errBuf));
		return NOTOK;
	}

        if (lexequ(key, "out") == 0)
			strcpy(psm->devName, value);
        else if (lexequ(key, "prefix") == 0) 
                psm->phone_prefix = strdup(value);
	else if (lexequ(key, "nattempts") == 0) {
		if ((psm->nattempts = atoi(value)) <= 0) {
			sprintf(faxctl->errBuf,
				"Cannot set number of attempts to '%s'",
				value);
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", faxctl->errBuf));
			return NOTOK;
		}
	} else if (lexequ(key, "sleep") == 0) {
		if ((psm->sleepFor = atoi(value)) <= 0) {
			sprintf(faxctl->errBuf,
				"Cannot set sleep time to '%s'",
				value);
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", faxctl->errBuf));
			return NOTOK;
		}
	} else if (lexequ(key, "softcar") == 0) {
		if (lexequ(value, "used") == 0)
			psm->softcar = TRUE;
	}
	else {
		sprintf(faxctl->errBuf,
			"Unknown info arg '%s=%s",
			key, (isstr(value)) ? value : "(null)");
		return NOTOK;
	}
	return OK;
}

/*
 *	Parse out-info line arguments
 */

psArgParseIn (faxctl, key, value)
FaxCtlr	*faxctl;
char	*key;
char	*value;
{
	PsModem	*psm = PSM(faxctl);
	
	if (!isstr(value)) {
		sprintf(faxctl->errBuf,
			"No value given with key '%s'",
			key);
		PP_LOG (LLOG_EXCEPTIONS,
			("%s", faxctl->errBuf));
		return NOTOK;
	}

        if (lexequ(key, "in") == 0)
                strcpy(psm->devName, value);
        else if (lexequ(key, "master") == 0)
                strcpy(faxctl->fax_recip,value);
	else if (lexequ(key, "subject") == 0)
		 strcpy(faxctl->subject, value);
	else if (lexequ(key, "softcar") == 0) {
		if (lexequ(value, "used") == 0)
			psm->softcar = TRUE;
	}
	else {
		sprintf(faxctl->errBuf,
			"Unknown info arg '%s=%s",
			key, (isstr(value)) ? value : "(null)");
		return NOTOK;
	}
	return OK;
}

/*
 *	Return a fax control block
 */

extern int      optind;
extern char     *optarg;

FaxCtlr	*init_faxctrlr(type, argc, argv)
int	type;
int	argc;
char	**argv;
{
	FaxCtlr	*faxctl;
	PsModem	*psm;
	int	opt;

	faxctl = (FaxCtlr *)malloc(sizeof(FaxCtlr));
        bzero(faxctl, sizeof(FaxCtlr));

	psm = (PsModem *)malloc(sizeof(PsModem));
	bzero(psm, sizeof(PsModem));
	switch (type) {
	    case FAX_OUTBOUND:
		psm->nattempts = 3;
		psm->sleepFor = 30; /* 1/2 min */
		psm->polled = FALSE;
		sprintf(psm->devName, "/dev/faxout"); /* default for now */
		faxctl->setG3Params = psSetG3Params;
		faxctl->sendPage = psSendPage;
		faxctl->setIA5Params = psSetIA5Params;
		faxctl->sendIA5File = psSendIA5File;
		faxctl->arg_parse = psArgParseOut;
		break;
	    case FAX_INBOUND:
		sprintf(psm->devName, "/dev/faxin");
		sprintf(faxctl->channel, "fax");
		sprintf(faxctl->fax_recip, "faxmaster");
		sprintf(faxctl->subject, "Inbound fax message");
		faxctl->receiveG3Fax = psReceiveG3Fax;
		psm->nattempts = 1;
		faxctl->arg_parse = psArgParseIn;
		
		/* now deal with command line args */
		
		while ((opt = getopt(argc, argv, "c:")) != EOF)
			switch(opt) {
			    case 'c':
				/* override channel name */
				strcpy(faxctl->channel, optarg);
				break;
			    default:
				(void) sprintf(faxctl->errBuf,
					       "Unknown command line argument '-%c'",
					       (char) opt);
				PP_LOG(LLOG_EXCEPTIONS,
				       ("%s", faxctl->errBuf));
				free(psm);
				free(faxctl);
				return (FaxCtlr *) NULL;
			
				break;
			}
		break;
	    default:
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unsupported fax type '%d'", type));
		free(psm);
		free(faxctl);
		return (FaxCtlr *) NULL;
	}

	psm->softcar = FALSE;
	psm->resolution = FAX_PARAM1_RES385;
	faxctl->softc = (char *) psm;

	faxctl->open = psOpen;
	faxctl->close = psClose;
	faxctl->initXmit = psInitXmit;
	faxctl->abort = psAbort;
	faxctl->termXmit = psTermXmit;
	
	return faxctl;
}
