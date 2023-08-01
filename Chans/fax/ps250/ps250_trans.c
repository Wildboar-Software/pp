/* fax_trans.c: wrappers around basic ps 250 functions */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_trans.c,v 6.0 1991/12/18 20:07:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_trans.c,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: ps250_trans.c,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "ps250.h"

/*
 * wrap up and initialise the tranmission parameters 
 */ 
extern char	*get_com_str();
extern int	fax_debug;

int	fax_trans (fd, tp)
int	fd;
Trans	*tp;
{
	Faxcomm fc;
	char	*cp;
	int	len;

	fc.flags = 0;
	fc.command = COM_TRANS;
	cp = fc.data;
	fc.len = 0;
	*cp ++ = tp -> src_type | tp -> src_addr;
	fc.len ++;
	*cp ++ = tp -> dst_type | tp -> dst_addr;
	fc.len ++;
	*cp ++ = tp -> param1;
	fc.len ++;
	*cp ++ = tp -> param2;
	fc.len ++;
	len = strlen (tp -> telno);
	if (len >= FAX_TELNOSIZE)
		return NOTOK;
	strcpy (cp, tp -> telno);
	len ++;
	cp += len;
	fc.len += len;
	if (tp -> remid[0]) {
		bcopy (tp -> remid, cp, FAX_REMIDSIZE);
		fc.len += FAX_REMIDSIZE;
	}
	else {
		*cp++ = 0;
		fc.len ++;
	}

	return fax_writepacket (fd, &fc);
}

int	fax_revpl (fd, tp)
int	fd;
Trans	*tp;
{
	Faxcomm fc;
	char	*cp;
	int	len;

	fc.flags = 0;
	fc.command = COM_REVPL;
	cp = fc.data;
	fc.len = 0;
	*cp ++ = tp -> src_type | tp -> src_addr;
	fc.len ++;
	*cp ++ = tp -> dst_type | tp -> dst_addr;
	fc.len ++;
	*cp ++ = tp -> param1;
	fc.len ++;
	*cp ++ = tp -> param2;
	fc.len ++;
	len = strlen (tp -> telno);
	if (len >= FAX_TELNOSIZE)
		return NOTOK;
	strcpy (cp, tp -> telno);
	len ++;
	cp += len;
	fc.len += len;
	if (tp -> remid[0]) {
		bcopy (tp -> remid, cp, FAX_REMIDSIZE);
		fc.len += FAX_REMIDSIZE;
	}
	else {
		*cp++ = 0;
		fc.len ++;
	}

	return fax_writepacket (fd, &fc);
}

int	fax_move (fd, tp)
int	fd;
Trans	*tp;
{
	Faxcomm	fc;
	char	*cp;

	fc.flags = 0;
	fc.command = COM_MOVE;
	fc.len = 4;
	cp = fc.data;
	*cp++ = tp -> src_type | tp -> src_addr;
	*cp++ = tp -> dst_type | tp -> dst_addr;
	*cp++ = tp -> param1;
	*cp++ = 0;
	return fax_writepacket (fd, &fc);
}

/*
 * send data to fax 
 */

fax_write (fd, data, len, flags)
int	fd;
char	*data;
int	len, flags;
{
	Faxcomm fc;

	fc.flags = 0;
	fc.len = len + 1;
	fc.command = COM_WRITE;
	fc.data[0] = flags;
	bcopy (data, &fc.data[1], len);
	return fax_writepacket (fd, &fc);
}

/*
 * request data from fax
 */

fax_read(fd, ok)
int	fd, ok;
{
	char	*cp, buffer[FAXBUFSIZ];
	
	cp = buffer;
	*cp ++ = FAX_HEADER;
	*cp ++ = FAX_HEADER;
	if (ok)
		*cp ++ = FAX_CONTROL;
	else
		*cp ++ = FAX_RETRANS;
	*cp ++ = FAX_ADDRESS;
	*cp ++ = 1;
	*cp ++ = COM_READ;
	*cp = fax_checksum (buffer, cp - buffer);
	*cp++;
	if (writex (fd, buffer, cp - buffer) != cp - buffer)
		return NOTOK;
	return OK;
}	

fax_data (fd, data)
int	fd;
Data	*data;
{
	Faxcomm	fc;

	if (fax_readpacket(fd, &fc) == NOTOK)
		return NOTOK;
	if (fc.command != RESP_DATA) 
		return NOTOK;	
	fax_fc2data(&fc, data);
	return OK;
}

/*
 * read response from fax
 */

int fax_getresp(fd, st1)
int	fd;
Stat1	*st1;
{
	Faxcomm fc;
	int	retval = NOTOK;

	if (fax_readpacket (fd, &fc) == NOTOK)
		return NOTOK;
	switch (fc.command) {
	    case RESP_ACK:
/*		if (fax_debug)
			printf("=> ACK ");*/
		retval = OK;
		break;
	    case RESP_STAT1:
		if (fax_debug) 
			PP_LOG(LLOG_DEBUG,
			       ("fax_getresp: STAT1"));
		retval = OK;
		fax_fc2stat1 (&fc, st1);
		if (fax_debug)
			fax_pstat1 (st1);
		break;
	    case RESP_REJ:
		if (fax_debug) 
			PP_LOG(LLOG_NOTICE,
			       ("fax_getresp: REJ"));
		fax_fc2stat1 (&fc, st1);
		if (fax_debug)
			fax_pstat1 (st1);
		if (st1->ce_code != COMERR_OK)
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Command code: %s", get_com_str(st1->ce_code)));
		if (st1->fecode[0])
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Fax error code: %s", 
				faxerr2str(atoi(st1->fecode))));
		break;
	    case RESP_STAT2:
		if (fax_debug) 
			PP_LOG(LLOG_DEBUG,
			       ("fax_getresp: STAT2"));
		break;
	    case RESP_DATA:
		if (fax_debug) 
			PP_LOG(LLOG_DEBUG,
			       ("fax_getresp: DATA"));
		break;
	}
	if (retval == NOTOK)
		return retval;
	return fc.command;
}
		
