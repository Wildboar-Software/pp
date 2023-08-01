/* ps250_stat.c: routines to enquiry of PS 250s status */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_stat.c,v 6.0 1991/12/18 20:07:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_stat.c,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: ps250_stat.c,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 */

#include	"util.h"
#include 	"ps250.h"

extern void advise (), adios ();

char	*get_job_str(), *get_com_str();

int fax_getstat1 (fd, sp)
int	fd;
Stat1 *sp;
{
	Faxcomm fc;

	if (fax_readpacket (fd, &fc) == NOTOK)
		return NOTOK;
	if (fc.command != RESP_STAT1)
		return NOTOK;
	return fax_fc2stat1 (&fc, sp);
}

fax_fc2data (fp, dp)
Faxcomm	*fp;
Data	*dp;
{
	char	*cp;

	bzero ((char *)dp, sizeof *dp);
	cp = fp -> data;
	
	dp->pageEnd = (*cp) & DATA_PAGE_END;
	dp->docEnd = (*cp) & DATA_DOC_END;
	
	cp++;
	dp->datalen = fp -> len - 1;
	bcopy(cp, dp->data, dp->datalen);
}

fax_fc2stat1 (fp, sp)
Faxcomm *fp;
Stat1 *sp;
{
	char	*cp;
	
	cp = fp -> data;
	bzero ((char *)sp, sizeof *sp);
	sp -> ce_code = *cp++;
	sp -> mode_cmd = *cp++;
	sp -> ring_in = *cp++;
	sp -> scan_stat = *cp++;
	sp -> print_stat = *cp ++;
	sp -> job_info = *cp ++;
	if (*cp) {
		bcopy (cp, sp -> fecode, 4);
		cp += 4;
	}
	else cp ++;

	sp -> res = *cp ++;
	sp -> comm = *cp ++;
	if (*cp == 0) {
		sp -> remid[0] = 0;
		cp ++;
	}
	else {
		bcopy (cp, sp -> remid, 21);
		cp += 21;
	}

	sp -> errlines = *cp << 8 | cp[1];
	cp += 2;
	sp -> page_no = atoi (cp);
	cp += 3;
	if (cp - fp -> data != fp -> len)
		advise (NULLCP, "Packet length mismatch %d != %d",
			cp - fp -> data, fp -> len);
	return OK;
}

void fax_pstat1(sp)
Stat1	*sp;
{
	if (sp -> ce_code != COMERR_OK) 
		PP_LOG(LLOG_DEBUG,
		       ("=>Command Code: %s\n",
			get_com_str(sp->ce_code)))
	switch (sp -> mode_cmd & MODE_MASK) {
	    case MODE_WAIT_JOB:
		PP_LOG(LLOG_DEBUG,
		       ("Mode : Wait Job"));
		break;
	    case MODE_WAIT_BLOCK:
		PP_LOG(LLOG_DEBUG,
		       ("Mode : Wait Block"));
		break;
	    case MODE_FAX_WORKING:
		PP_LOG(LLOG_DEBUG,
		       ("Mode : Fax working"));
		break;
	    default:
		PP_LOG(LLOG_DEBUG,
		       ("Unknown Mode : 0x%x", sp -> mode_cmd & MODE_MASK));
		break;
	}
	putchar ('\n');

	if (sp -> ring_in == RING_IN)
		PP_LOG(LLOG_DEBUG,
		       ("Ring in set"));

	if (sp -> scan_stat & SCAN_DOC_SCAN)
		PP_LOG(LLOG_DEBUG,
		       ("Scanner Status: Document scanner"));
	if (sp -> scan_stat & SCAN_NODOC)
		PP_LOG(LLOG_DEBUG,
		       ("Scanner Status: No document on ADF"));
	if (sp -> scan_stat & SCAN_FEEDERR)
		PP_LOG(LLOG_DEBUG,
		       ("Scanner Status: Feeder Error"));
	if (sp -> scan_stat & SCAN_TOOLONG)
		PP_LOG(LLOG_DEBUG,
		       ("Scanner Status: Document too long"));
	if (sp -> scan_stat & SCAN_B4)
		PP_LOG(LLOG_DEBUG,
		       ("Scanner Status: Document B4 size"));

	if (sp -> print_stat & PRINT_PJAM)
		PP_LOG(LLOG_DEBUG,
		       ("Printer Status: Printer Jam"));
	if (sp -> print_stat & PRINT_NOPAPER)
		PP_LOG(LLOG_DEBUG,
		       ("Printer Status: No paper"));

	PP_LOG(LLOG_DEBUG,
	       ("Job Info: %s", get_job_str(sp->job_info)));

	if (sp -> fecode[0]) 
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Fax error code: %s", 
			faxerr2str(atoi(sp -> fecode))));

	if (sp -> res !=0) {
		switch (sp -> res & RES_MASK) {
		    case RES_7_7:
			PP_LOG(LLOG_DEBUG,
			       ("Resolution: 7.7 lines/mm"));
			break;
		    case RES_3_85:
			PP_LOG(LLOG_DEBUG,
			       ("Resolution: 3.85 lines/mm"));
			break;
		    case RES_15_4:
			PP_LOG(LLOG_DEBUG,
			       ("Resolution: 15.4 lines/mm"));
			break;
		    default:
			PP_LOG(LLOG_DEBUG,
			       ("Resolution: Unknown (0x%x)", sp -> res));
			break;
		}
	} else
		PP_LOG(LLOG_DEBUG,
		       ("Resolution: unset"));

	if (sp -> comm != 0) {
		char	buf[BUFSIZ];
		switch (sp -> comm & COMM_GMASK) {
		    case COMM_G2:
			strcpy(buf," G2");
			break;
		    case COMM_G3:
			strcpy(buf, " G3");
			break;
		    default:
			sprintf (buf, " %d", sp -> comm);
			break;
		}
		switch (sp -> comm & COMM_XR_MASK) {
		    case COMM_XMT:
			strcat (buf, " XMT");
			break;
		    case COMM_RCV:
			strcat (buf, " RCV");
			break;
		    default:
			break;
		}

		switch (sp -> comm & COMM_SPEED_MASK) {
		    case COMM_SPEED_2400:
			strcat (buf, " 2400baud");
			break;
		    case COMM_SPEED_4800:
			strcat (buf, " 4800 baud");
			break;
		    case COMM_SPEED_7200:
			strcat (buf, " 7200baud");
			break;
		    case COMM_SPEED_9600:
			strcat (buf, " 9600baud");
			break;
		    default:
			break;
		}

		switch (sp -> comm & COMM_MHR_MASK) {
		    case COMM_MH:
			strcat (buf, " MH");
			break;
		    case COMM_MR:
			strcat (buf, " MR");
			break;
		}
		if ((sp -> comm &  COMM_POLLED) == COMM_POLLED)
			strcat (buf, " Polled");
		PP_LOG(LLOG_DEBUG,
		       ("Comms: %s",buf));
	} else
		PP_LOG(LLOG_DEBUG,
		       ("Comms: unset"));

	if (sp -> remid[0]) 
		PP_LOG(LLOG_DEBUG,
		       ("Remote Id: %s", sp -> remid));

	if (sp -> errlines) 
		PP_LOG(LLOG_EXCEPTIONS,
		       (" Error Lines: %d", sp -> errlines));

	if (sp -> page_no) 
		PP_LOG(LLOG_DEBUG,
		       ("Page No: %d", sp->page_no));
}

static char	ret[128];

char	*get_com_str(com)
int	com;
{
	switch (com) {
	    case COMERR_UNDEF_COMMAND:
		sprintf (ret, "Undefined Command");
		break;
	    case COMERR_UNAC_COMMAND:
		sprintf (ret, "Unacceptable Command");
		break;
	    case COMERR_PARAM:
		sprintf (ret, "Command Parameter Error");
		break;
	    case COMERR_FRAME:
		sprintf (ret, "Frame Error");
	    default:
		sprintf (ret, "Unknown response code 0x%x", com);
		break;
	}
	return ret;
}

char	*get_job_str(job)
int	job;
{
	switch (get_job_info (job)) {
	    case JOB_IN:
		sprintf (ret, "Job In");
		break;
	    case JOB_READY:
		sprintf (ret, "Job ready");
		break;
	    case JOB_DATA_AVAIL:
		sprintf (ret, "Job data avilable");
		break;
	    case JOB_RETRANS:
		sprintf (ret, "Job retransmit last block");
		break;
	    case JOB_NORM_END:
		sprintf (ret, "Normal end");
		break;
	    case JOB_FAULT_JOB_END:
		sprintf (ret, "Fault job end");
		break;
	    case JOB_LINE_DISC:
		sprintf (ret, "Line disconnected");
		break;
	    default:
		sprintf (ret, "Unknown value %o", job);
		break;
	}
	return ret;
}	

int 	get_job_info(job)
int	job;
{
	if ((job & JOB_READY) == JOB_READY)
		return JOB_READY;
	if ((job & JOB_DATA_AVAIL) == JOB_DATA_AVAIL)
		return JOB_DATA_AVAIL;
	if ((job & JOB_RETRANS) == JOB_RETRANS)
		return JOB_RETRANS;
	if ((job & JOB_NORM_END_MASK) == JOB_NORM_END)
		return JOB_NORM_END;
	if ((job & JOB_NORM_END_MASK) == JOB_FAULT_JOB_END)
		return JOB_FAULT_JOB_END;
	if ((job & JOB_LINE_DISC_MASK) == JOB_LINE_DISC)
		return JOB_LINE_DISC;
	if ((job & JOB_IN))
		return JOB_IN;
	return NOTOK;
}

void	timet2fax (t, buf)
time_t	t;
char	buf[];
{
	struct tm *tm;

	tm = localtime (&t);
	(void) sprintf (buf, "%02d%02d%02d%02d%02d",
			tm -> tm_min, tm -> tm_hour, tm -> tm_mday,
			tm -> tm_mon + 1, tm -> tm_year);
}

int fax_idset (fd, ip)
int	fd;
Idset *ip;
{
	Faxcomm fc;
	char	*cp;

	fc.flags = 0;
	fc.command = COM_IDSET;

	if (ip -> clock[0]) {
		bcopy (ip -> clock, fc.data, FAX_CLOCKLEN);
		fc.len = FAX_CLOCKLEN;
		cp = fc.data + FAX_CLOCKLEN;
	}
	else	{
		fc.data[0] = 0;
		cp = fc.data + 1;
		fc.len = 1;
	}

	if (ip -> local[0]) {
		bcopy (ip -> local, cp, FAX_LOCALIDLEN);
		fc.len += FAX_LOCALIDLEN;
	}
	else {
		*cp = 0;
		fc.len ++;
	}

	return fax_writepacket (fd, &fc);
}

fax_getstat2 (fd, sp)
int	fd;
Stat2	*sp;
{
	Faxcomm fc;
	char	*cp;

	if (fax_readpacket (fd, &fc) == NOTOK)
		return NOTOK;
	if (fc.command != RESP_STAT2) {
		advise (NULLCP, "Not a stat2 response");
		return NOTOK;
	}

	if (fc.data[0] == 0) {
		sp -> clock[0] = 0;
		cp = fc.data + 1;
	}
	else {
		bcopy (fc.data, sp -> clock, FAX_CLOCKLEN);
		cp = fc.data + FAX_CLOCKLEN;
	}

	if (*cp)
		bcopy (cp, sp -> local, FAX_LOCALIDLEN);
	else
		sp -> local[0] = 0;
	return OK;
}

	
		
