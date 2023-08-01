/* ps250_util.c: utility routines for Panasonic 250 fax machine */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_util.c,v 6.0 1991/12/18 20:07:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_util.c,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: ps250_util.c,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 */
#include	"util.h"
#include	"retcode.h"
#include	"qmgr.h"
#include	"IOB-types.h"
#include 	<sys/stat.h>
#include	<sys/termios.h>
#ifdef SYS5		/* Can't use isode/sys.file.h due to termios.h */
#include 	<fcntl.h>
#else
#include 	<sys/file.h>
#endif
#include	"../faxgeneric.h"
#include 	"ps250.h"

static int set_softcar();
static void set_hardcar();

Trans	trans;

/*
 *	Open and initialise fax device
 */

openPsModem(psm)
PsModem	*psm;
{
	static struct	termios tios;

	if ((psm->fd = open (psm->devName, O_RDWR, 0)) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS,
			 psm->devName,
			 ("unable to open fax device"));
		sprintf(psm->errBuf,
			"Unable to open fax device '%s'",
			psm->devName);
		return NOTOK;
	}
	
	/* initialise line parameters */
/*	(void) ioctl (psm->fd, TIOCEXCL, 0); */
	tios.c_iflag = 0;
	tios.c_oflag = 0;
	tios.c_cflag = B19200 | CS8 | CREAD | CSTOPB | PARENB | 
		PARODD | CRTSCTS | HUPCL;
	tios.c_lflag = 0;
	tios.c_cc[VMIN] = 1;
	if (ioctl (psm->fd, TCSETS, &tios) == NOTOK) {
		PP_SLOG(LLOG_EXCEPTIONS, psm->devName,
			("ioctl failed"));
		(void) close (psm->fd);
		sprintf(psm->errBuf,
			"Unable to initialise line parameters");
		return NOTOK;
	}
	sleep(1);
	if (psm->softcar == TRUE)
		return set_softcar(psm);
	return OK;
}

/*
 *	Close fax device
 */

closePsModem (psm)
PsModem	*psm;
{
	int val = TIOCM_DSR|TIOCM_DTR;

	if (psm->softcar == TRUE)
		set_hardcar(psm);
	(void) ioctl (psm->fd, TIOCMBIC, &val);
	return close (psm->fd);
}

/*
 *	Initial fax device for Xmit
 */

dialPsModem(psm, telno)
PsModem	*psm;
char	*telno;
{
		
	trans.src_type = FAX_TYPE_MH;
	trans.src_addr = FAX_MODE_PC;
	trans.dst_type = FAX_TYPE_MH;
	trans.param1 = psm->resolution;
	trans.param2 = FAX_PARAM2_G3 | FAX_PARAM2_OTI_TOP |
                FAX_PARAM2_OTI_BOT | FAX_PARAM2_ECMON;

	if (psm->polled == TRUE)
		trans.param2 |= FAX_PARAM2_POLLED;

	if (atoi(telno) == 0 && telno[0] != '+') {
		PP_NOTICE(("printing fax locally"));
		trans.telno[0] = 0;
		trans.dst_addr = FAX_MODE_PRINTER;
	} else {
		PP_NOTICE(("faxing to %s",
			   telno));
		strcpy(trans.telno, telno);
		trans.dst_addr = FAX_MODE_REMOTE;
	} 

	trans.remid[0] = 0;

	if (fax_trans_and_check (psm, &trans, FALSE) != OK)
		return RP_MECH;
	return OK;
}

/*
 *	Initial fax device for Listen
 */

ps_init_listen(psm)
PsModem	*psm;
{
		
	trans.src_type = FAX_TYPE_MH;
	trans.src_addr = (psm->scanner) ? FAX_MODE_SCANNER : FAX_MODE_REMOTE;
	trans.dst_type = FAX_TYPE_MH;
	trans.dst_addr = FAX_MODE_PC;
	trans.param1 = psm->resolution;
	trans.param2 = FAX_PARAM2_G3 | FAX_PARAM2_OTI_TOP |
                FAX_PARAM2_OTI_BOT | FAX_PARAM2_ECMON;

	if (psm->polled == TRUE)
		trans.param2 |= FAX_PARAM2_POLLED;

	trans.telno[0] = 0;
	trans.remid[0] = 0;

	if (fax_trans_and_check (psm, &trans, TRUE) != OK)
		return RP_MECH;
	return OK;
}

/*
 *	software carrier detect switching routines
 */

static int set_softcar (psm)
PsModem	*psm;
{
	int	softcar;
	
	if (psm->softcar != TRUE)
		return OK;

	if (ioctl (psm->fd, TIOCGSOFTCAR, &softcar) == -1) {
		PP_SLOG (LLOG_EXCEPTIONS, psm->devName,
			 ("ioctl(TIOCGSOFTCAR) failed"));
		sprintf(psm->errBuf,
			"ioctl(TIOCGSOFTCAR) failed");
		return NOTOK;
	}

	if (softcar == 0) {
		/* need to switch on softcar */
		softcar = 1;
		if (ioctl (psm->fd, TIOCSSOFTCAR, &softcar) == -1) {
			PP_SLOG(LLOG_EXCEPTIONS, psm->devName,
				("ioctl(TIOCSSOFTCAR) failed"));
			sprintf(psm->errBuf,
				"ioctl(TIOCSSOFTCAR) failed");
			return NOTOK;
		}
	}
	return OK;
}

static void set_hardcar(psm)
PsModem	*psm;
{
	int	softcar;
	
	if (psm->softcar != TRUE)
		return;
	
	if (ioctl (psm->fd, TIOCGSOFTCAR, &softcar) == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, psm->devName,
			("ioctl(TIOCGSOFTCAR) failed"));
		return;
	}
	
	if (softcar == 1) {
		/* need to switch off softcar */
		softcar = 0;
		if (ioctl (psm->fd, TIOCSSOFTCAR, &softcar) == -1)
			PP_SLOG(LLOG_EXCEPTIONS, psm->devName,
				("ioctl(TIOCSSOFTCAR) failed"));
	}
}

/*
 *	Abort a fax
 */

abortPsModem (psm)
PsModem	*psm;
{
	Stat1	st1;
	fax_sendstop(psm->fd);
	fax_getstat1(psm->fd, &st1);
}

/*
 * wait for end of transmission
 */

eomPsModem(psm)
PsModem	*psm;
{
	Stat1	st1;
	int	cont = TRUE;
	while (cont == TRUE) {
		if (fax_sendenq1(psm->fd) == NOTOK) {
			sprintf(psm->errBuf,
				"sendenq1 failed");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", psm->errBuf));
			return NOTOK;
		}
		cont = FALSE;
		switch (fax_getresp(psm->fd, &st1)) {
		    case RESP_STAT1:
			switch (get_job_info(st1.job_info)) {
			    case JOB_NORM_END:
				break;
			    case JOB_FAULT_JOB_END:
			    case JOB_LINE_DISC:
				sprintf(psm->errBuf,
					"bad termination from fax");
				abortPsModem(psm);
				return NOTOK;
			    case JOB_IN:
				cont = TRUE;
				break;
			    default:
				sprintf(psm->errBuf,
					"unknown termination from fax %o",
					st1.job_info);
				PP_LOG(LLOG_EXCEPTIONS,
				       ("%s", psm->errBuf));
				return NOTOK;
			}
		    default:
			break;
		}
		if (cont == TRUE)
			sleep(1);
	}
	return OK;
}

/*
 *	Initialise modem for fax transmission
 */

static int reinit_fax (fd, trans, src_type, dst_type, param1)
int	fd;
Trans	*trans;
char	src_type;
char	dst_type;
int	param1;
{
	if (src_type != trans->src_type
	    || dst_type != trans -> dst_type
	    || param1 != trans -> param1) {
		/* have to change type */
		trans->src_type = src_type;
		trans->dst_type = dst_type;
		trans -> param1 = param1;
		
		if (fax_move_and_check(fd, trans) == NOTOK) 
			return NOTOK;
	}
	return OK;
}

psModemG3init (psm)
PsModem	*psm;
{
	if (reinit_fax(psm->fd, &trans,
		       FAX_TYPE_MH, FAX_TYPE_MH, psm->resolution) != OK) {
		sprintf(psm->errBuf,
			"initialise failed");
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", psm->errBuf));
		return NOTOK;
	}
	return OK;
}

/*
 *	Initialise modem for ia5 transmission
 */

psModemIA5init (psm)
PsModem	*psm;
{
	if (reinit_fax(psm->fd, &trans,
		       FAX_TYPE_ASCII, FAX_TYPE_RASTER, 
		       psm->resolution) != OK) {
		sprintf(psm->errBuf,
			"initialise failed");
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", psm->errBuf));
		return NOTOK;
	}
	return OK;
}
	
/*
 *	Send page of bits to fax modem
 */

extern int num_sent;

sendPsModem (psm, str, len, last)
PsModem	*psm;
char	*str;
int	len;
int	last;
{
	char	*ix;
	int	sendflag, sendlen;
	
	sendflag = sendlen = num_sent = 0;
	ix = str;
	while (ix - str < len && sendflag == 0) {
		sendlen = ((ix + MAXDATASIZ) > (str + len)) ?
			len - (ix - str) : MAXDATASIZ;
		if (ix - str + sendlen >= len) {
			/* last bit of page */
			if (last)
				sendflag = FAX_WRITE_EOF | FAX_WRITE_EOP;
			else
				sendflag = FAX_WRITE_EOP;
		} else
			sendflag = 0;
		if (fax_write_and_check (psm, ix, 
					 sendlen, sendflag) != OK) {
			sprintf(psm->errBuf,
				"fax_write_and_check failed");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", psm->errBuf));
			return NOTOK;
		}
		if (sendflag) {
			if (last)
				PP_LOG(LLOG_NOTICE,
				       ("EOF (%d sent)", num_sent));
			else
				PP_LOG(LLOG_NOTICE,
				       ("EOP (%d sent)", num_sent));
		}
		ix += sendlen;
	}
	return OK;
}

/*
 *	Send file of IA5 to fax modem
 */

sendPsModemIA5 (psm, fp, last)
PsModem	*psm;
FILE	*fp;
int	last;
{
	int	n, numLines, LinesPerA4 = 24*3;
	char	buf[250];

	numLines = 0;

	while (fgets (buf, sizeof(buf), fp) != NULLCP) {
		if (feof(fp))
			break;
		n = strlen(buf);
		if (n < sizeof(buf)) {
			buf[n++] = '\r';
			if (n < sizeof(buf))
				buf[n++] = '\0';
			n = strlen(buf);
		}
		if (fax_write_and_check (psm, buf, n,
					 (++numLines == LinesPerA4) ? FAX_WRITE_EOP : 0) != OK) {
			sprintf(psm->errBuf,
				"fax_write_and_check failed");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", psm->errBuf));
			return NOTOK;
		}
		if (numLines == LinesPerA4) numLines = 0;
	}
	/* fillout last page */
	n = 0;
	while (numLines++ < LinesPerA4) {
		buf[n++] = '\n';
		buf[n++] = '\r';
	}
	if (n == 0)
		buf[n++] = ' ';
	buf[n] = '\0';
	if (fax_write_and_check (psm, buf, n,
				 (last) ? (FAX_WRITE_EOP | FAX_WRITE_EOF) : FAX_WRITE_EOP) != OK) {
		sprintf(psm->errBuf,
			"fax_write_and_check failed");
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", psm->errBuf));
		return NOTOK;
	}
	return OK;
}

/*  */

/* loop waiting for inbound fax */
ps_wait_for_fax (psm)
PsModem	*psm;
{
	Stat1	st1;
	int	cont = TRUE;

	do {
		if (fax_sendenq1(psm->fd) == NOTOK) {
			sprintf(psm->errBuf,
				"sendenq1 failed");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", psm->errBuf));
			return NOTOK;
		}
		
		if (fax_getresp (psm->fd, &st1) == NOTOK) {
			sprintf(psm->errBuf,
				"bad response from fax");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", psm->errBuf));
			return NOTOK;
		}

		if (st1.ring_in == RING_IN) {
			psm->scanner = FALSE;
			cont = FALSE;
		} else if ((st1.scan_stat & SCAN_DOC_SCAN) == SCAN_DOC_SCAN) {
			psm->scanner = TRUE;
			cont = FALSE;
		}
		
		if (cont == TRUE)
			sleep(1);
	} while (cont == TRUE);
	return OK;
}

/*  */

/* loop waiting for inbound data */
ps_wait_for_data (psm)
PsModem	*psm;
{
	Stat1	st1;

	if (fax_getresp (psm->fd, &st1) == NOTOK) {
		sprintf(psm->errBuf,
			"bad response from fax");
		PP_LOG(LLOG_EXCEPTIONS,
		       ("%s", psm->errBuf));
		return NOTOK;
	}
	
	while (get_job_info(st1.job_info) != JOB_DATA_AVAIL) {
		sleep (1);
		
		if (fax_sendenq1(psm->fd) == NOTOK) {
			sprintf(psm->errBuf,
				"sendenq1 failed");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", psm->errBuf));
			return NOTOK;
		}
		

		if (fax_getresp (psm->fd, &st1) == NOTOK) {
			sprintf(psm->errBuf,
				"bad response from fax");
			PP_LOG(LLOG_EXCEPTIONS,
			       ("%s", psm->errBuf));
			return NOTOK;
		}
	}

	return OK;
}
		
/*  */
/* receive data from fax */

rcvPsModem(psm, pg3fax)
PsModem	*psm;
struct type_IOB_G3FacsimileBodyPart	**pg3fax;
{
	struct type_IOB_G3FacsimileBodyPart *ret = (struct type_IOB_G3FacsimileBodyPart *) 
		calloc (1, sizeof (struct type_IOB_G3FacsimileBodyPart));
	struct type_IOB_G3FacsimileData	*ix, *t = NULL;
	int	retval, last;

	ret -> parameters = (struct type_IOB_G3FacsimileParameters *)
		calloc(1, sizeof(struct type_IOB_G3FacsimileParameters));
	ret -> parameters -> optionals =
		opt_IOB_G3FacsimileParameters_number__of__pages;
	
	do {
		if ((retval = rcvPsPage(psm, &ix, &last)) != OK)
			return retval;
		if (ret->data == NULL)
			ret -> data = t = ix;
		else {
			t -> next = ix;
			t = ix;
		}
		ret -> parameters -> number__of__pages++;
	} while (last != TRUE);
	*pg3fax = ret;
	return retval;
}

/*  */
/* receive page from fax */

static void add_to_strb(pstr, ptotal, plus, len)
char	**pstr;
int	*ptotal;
char	*plus;
int	len;
{
	char	*ret;

	if (*pstr == NULLCP)
		ret = malloc(len * sizeof(char));
	else
		ret = realloc(*pstr, (len+(*ptotal)) * sizeof(char));
	
	bcopy (plus, (ret + (*ptotal)), len);
	*ptotal += len;
	*pstr = ret;
}

rcvPsPage(psm, pdata, plast)
PsModem	*psm;
struct type_IOB_G3FacsimileData **pdata;
int	*plast;
{
	Stat1	st1;
	Data	data;
	int	cont, total;
	char	*str = NULLCP;

	*pdata = (struct type_IOB_G3FacsimileData *) NULL;
	*plast = FALSE;
	cont = TRUE;
	total = 0;

	do {
		if (fax_read_and_check(psm, &data) == NOTOK)
			return NOTOK;
		reverse_strb(data.data, data.datalen);
		add_to_strb(&str, &total, data.data, data.datalen);
		if (data.docEnd)
			*plast = TRUE;
		if (data.pageEnd)
			cont = FALSE;
		if (cont == TRUE) {
			if (fax_sendenq1(psm->fd) == NOTOK) {
				sprintf(psm->errBuf,
					"sendenq1 failed");
				PP_LOG(LLOG_EXCEPTIONS,
				       ("%s", psm->errBuf));
				if (str != NULLCP) free(str);
				return NOTOK;
			}
			if (fax_getresp (psm->fd, &st1) == NOTOK) {
				sprintf(psm->errBuf,
					"Bad response from fax");
				PP_LOG(LLOG_EXCEPTIONS,
				       ("%s", psm->errBuf));
				if (str != NULLCP) free(str);
				return NOTOK;
			}
			if (get_job_info(st1.job_info) != JOB_DATA_AVAIL) {
				sprintf(psm->errBuf,
					"No data waiting to be read");
				PP_LOG(LLOG_EXCEPTIONS,
				       ("%s", psm->errBuf));
				if (str != NULLCP) free(str);
				return NOTOK;
			}
		}
	} while (cont == TRUE);
	
	if (str != NULLCP) {
		*pdata = (struct type_IOB_G3FacsimileData *) 
			malloc(sizeof(struct type_IOB_G3FacsimileData));
		(*pdata) -> element_IOB_0 = 
			strb2bitstr (str, (total * BITSPERBYTE), 
				     PE_CLASS_UNIV,
				     PE_PRIM_BITS);
		(*pdata) -> next = NULL;
		free (str);
	}
	return OK;
}
