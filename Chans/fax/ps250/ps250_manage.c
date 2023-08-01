/* ps250_manage.c: general management of fax connections */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_manage.c,v 6.0 1991/12/18 20:07:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_manage.c,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: ps250_manage.c,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 */

#include	"util.h"
#include 	"ps250.h"
#include	"retcode.h"
#include	<sys/termios.h>

/* 
*/

int fax_move_and_check (psm, trans)
PsModem *psm;
Trans	*trans;
{
	Stat1	st1;

	if (fax_move(psm->fd, trans) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("fax_move failed"));
		return NOTOK;
	}
	if (fax_getresp (psm->fd, &st1) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Bad response from fax"));
		return NOTOK;
	}
	return OK;
}

int fax_trans_and_check (psm, trans, listen)
PsModem	*psm;
Trans	*trans;
int	listen;
{
	Stat1	st1;
	int	resp, job;
	int	attempts = 0;
	int	cont = TRUE;
	int	enqSleep = 1;
	psm->connected = FALSE;

	for (attempts = 0; attempts < psm->nattempts; attempts++) {
		if (attempts != 0) {
			sleep (psm->sleepFor);
			fax_sendstop(psm->fd);
			fax_getstat1(psm->fd, &st1);
		}

		if (fax_trans (psm->fd, trans) == NOTOK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unable to initialise fax parameters"));
			return NOTOK;
		}

		if (listen)
			/* do enq1 elsewhere */
			return OK;

		if ((resp = fax_getresp (psm->fd, &st1)) == NOTOK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Bad response from fax"));
			return NOTOK;
		}

		if (trans->telno[0] == 0) 
			/* local print so don't wait */
			return OK;
		cont = TRUE;
		while (cont == TRUE && resp == RESP_STAT1) {

			switch (job = get_job_info(st1.job_info)) {
			    case JOB_READY:
				if ((st1.comm & COMM_GMASK) == COMM_G2
				    || (st1.comm & COMM_GMASK) == COMM_G3) {
					psm->connected = TRUE;
					return OK;
				}
				break;
			    case JOB_IN:
				break;
			    default:
				sprintf(psm->errBuf,
					"Unexpected job info '%d'", job);
				PP_LOG(LLOG_EXCEPTIONS,
				       ("%s", psm->errBuf));
				if (st1.fecode[0]) {
					sprintf(psm->errBuf,
					       "Fax error code: %s",
					       faxerr2str(atoi(st1.fecode)));
					PP_LOG(LLOG_EXCEPTIONS,
					       ("%s", psm->errBuf));
				}
				switch (atoi(st1.fecode)) {
				    case 692:
					/* busy */
					cont = FALSE;
					break;
				    case 690:
					/* no dial */
					return NOTOK;
				    default:
					/* failed */
					psm->connected = TRUE;
					return NOTOK;
				}
			}
			if (cont == TRUE) {
				sleep (enqSleep);
				if (fax_sendenq1(psm->fd) == NOTOK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("sendenq1 failed"));
					return NOTOK;
				}
				if ((resp = fax_getresp (psm->fd, &st1)) == NOTOK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Bad response from fax"));
					return NOTOK;
				}
			}
		}
	}
	return NOTOK;
}

int fax_revpl_and_check (psm, trans)
PsModem *psm;
Trans	*trans;
{
	Stat1	st1;
	
	if (fax_revpl (psm->fd, trans) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unable to initialise fax parameters"));
		return NOTOK;
	}

	if ((fax_getresp (psm->fd, &st1)) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Bad response from fax"));
		return NOTOK;
	}
	fax_pstat1(&st1);
	return OK;
}

int num_sent;

extern char	*get_job_str();

int fax_write_and_check (psm, buf, num, flags)
PsModem	*psm;
char	*buf;
int	num, flags;
{
	Stat1	st1;
	int	resp;

	if (fax_write (psm->fd, buf, num, flags)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("fax_write failed"));
		return NOTOK;
	}
	if ((resp = fax_getresp (psm->fd, &st1)) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Bad response from fax"));
		return NOTOK;
	}
	if (resp == RESP_ACK) {
		num_sent += num;
		return OK;
	}
	while  (resp == RESP_STAT1) {
		switch (get_job_info(st1.job_info)) {
		    case JOB_RETRANS:
			return fax_write_and_check (psm, buf, num, flags);
		    case JOB_READY:
			num_sent += num;
			return OK;
		    case JOB_IN:
			/* wait and send enq1 */
			if (flags & FAX_WRITE_EOF) 
				/* do waiting in term fax */
				return OK;
			sleep (1);
			if (fax_sendenq1(psm->fd) == NOTOK) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("sendenq1 failed"));
				return NOTOK;
			}
			if ((resp = fax_getresp (psm->fd, &st1)) == NOTOK) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Bad response from fax"));
				return NOTOK;
			}
			break;
		    default:
			/* WORK rest of cases */
			PP_LOG(LLOG_EXCEPTIONS,
			       ("fax_write_and_check: Unexpected job info '%s'",
				get_job_str(st1.job_info)));
			if (st1.fecode[0])
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Fax error code: %s", 
					faxerr2str(atoi(st1.fecode))));

			return NOTOK;
		}
	}
	return OK;
}

int fax_read_and_check (psm, data)
PsModem	*psm;
Data	*data;
{
	if (fax_read (psm->fd, TRUE) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("fax_read failed"));
		return NOTOK;
	}
	if (fax_data(psm->fd, data) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("fax_data failed"));
		return NOTOK;
	}
	return OK;
}

