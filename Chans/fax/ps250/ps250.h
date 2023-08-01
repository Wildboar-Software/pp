/* ps250.h: header file for the Panasonic 250 fax machine */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250.h,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: ps250.h,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 *
 */

#include <isode/manifest.h>

/*
 General format of command/response packet

 Header		2 bytes
 Control	1 byte
 Address	1 byte
 Length		1 byte
 Information	variable
 check sum	1 byte
*/


#define FAX_HEADER	0x10
#define FAX_ADDRESS	0x00
#define FAX_CONTROL	0x1C
#define FAX_RETRANS	0x38

/* COMMAND codes */
#define COM_STOP	0x11
#define COM_ENQ1	0x12
#define COM_ENQ2	0x13
#define COM_TRANS	0x21
#define COM_REVPL	0x22
#define COM_READ	0x30
#define COM_WRITE	0x31
#define COM_MOVE	0x32
#define COM_IDSET	0x40

/* response codes */
#define RESP_ACK	0x50
#define RESP_REJ	0x51
#define RESP_STAT1	0x52
#define RESP_STAT2	0x53
#define RESP_DATA	0x54

/* Command Error codes */

#define COMERR_OK		0
#define COMERR_UNDEF_COMMAND	0x40
#define COMERR_UNAC_COMMAND	0x41
#define COMERR_PARAM		0x42
#define COMERR_FRAME		0x43

#define MODE_MASK		0xf0
#define MODE_WAIT_JOB		0x00
#define MODE_WAIT_BLOCK		0x20
#define MODE_FAX_WORKING	0x30

#define STATE_MASK		0x0f
#define STATE_COMPUTER		0x03
#define STATE_FAX		0x04

#define RING_IN			0x01

#define SCAN_DOC_SCAN		0x01
#define SCAN_NODOC		0x02
#define SCAN_FEEDERR		0x04
#define SCAN_TOOLONG		0x08
#define SCAN_B4			0x10

#define PRINT_PJAM		0x04
#define PRINT_NOPAPER		0x10

#define JOB_IN			0x01
#define JOB_READY		0x13
#define JOB_DATA_AVAIL		0x23
#define JOB_RETRANS		0x33
#define JOB_NORM_END_MASK	0x03
#define JOB_NORM_END		0x00
#define JOB_FAULT_JOB_END	0x02
#define JOB_LINE_DISC_MASK	0x1f
#define JOB_LINE_DISC		0x12

#define RES_MASK		0x07
#define RES_7_7			0x01
#define RES_3_85		0x02
#define RES_15_4		0x03

#define COMM_GMASK		0x03
#define COMM_G2			0x01
#define COMM_G3			0x03

#define COMM_XR_MASK		0x05
#define COMM_XMT		0x05
#define COMM_RCV		0x01

#define COMM_SPEED_MASK		0x1B
#define COMM_SPEED_2400		0x03
#define COMM_SPEED_4800		0x0B
#define COMM_SPEED_7200		0x13
#define COMM_SPEED_9600		0x1B

#define COMM_MHR_MASK		0x23
#define COMM_MH			0x03
#define COMM_MR			0x23

#define COMM_POLLED		0x83

#define FAXMINSIZE	7
#define FAXBUFSIZ	262	/* max fax trans/recv buffer */
#define FAXHDRSIZE	5

typedef struct faxcomm {
	int flags;
	int len;
	int command;
	char data[FAXBUFSIZ];
} Faxcomm;

typedef struct stat1 {
	int ce_code;
	int mode_cmd;
	int ring_in;
	int scan_stat;
	int print_stat;
	int job_info;
	char fecode[4];
	int	res;
	int	comm;
	char	remid[21];
	int	errlines;
	int	page_no;
} Stat1;

#define FAX_CLOCKLEN	11
#define FAX_LOCALIDLEN	21
typedef struct stat2 {
	char	clock[FAX_CLOCKLEN];
	char	local[FAX_LOCALIDLEN];
} Stat2, Idset;

#define FAX_TELNOSIZE	28
#define FAX_REMIDSIZE	21

#define FAX_TYPE_MH	0x00
#define FAX_TYPE_RASTER	0x20
#define FAX_TYPE_ASCII	0x30

#define FAX_MODE_PC		0x01
#define FAX_MODE_SCANNER	0x02
#define FAX_MODE_PRINTER	0x03
#define FAX_MODE_REMOTE		0x04

#define FAX_PARAM1_RES385	0x02
#define FAX_PARAM1_RES77	0x01
#define FAX_PARAM1_RES154	0x03
#define FAX_PARAM1_RES77G	0x09

#define FAX_PARAM2_G2		0x03
#define FAX_PARAM2_G3		0x02
#define FAX_PARAM2_POLLING	0x10
#define FAX_PARAM2_POLLED	0x20
#define FAX_PARAM2_OTI_TOP	0x08
#define FAX_PARAM2_OTI_BOT	0x04
#define FAX_PARAM2_ECMON	0x40

typedef struct trans {
	char	src_type;
	char	src_addr;
	char	dst_type;
	char	dst_addr;
	int	param1;
	int	param2;
	char	telno[FAX_TELNOSIZE];
	char	remid[FAX_REMIDSIZE];
} Trans;

#define MAXDATASIZ	254

#define DATA_PAGE_END	0x08
#define DATA_DOC_END	0x04

typedef struct data {
	char	data[MAXDATASIZ];
	int	datalen;
	int	pageEnd;
	int	docEnd;
} Data;

#define FAX_WRITE_EOF	0x04
#define FAX_WRITE_EOP	0x08
#define FAX_WRITE_MOVE	0x10

#define FAX_DEVICE	"/dev/fax"

void	timet2fax ();
int	fax_readpacket (), fax_writepacket ();
int	fax_getstat1 (), fax_getstat2 ();
int	fax_idset (), fax_simplecom ();
void	fax_pstat1 ();

#define fax_sendstop(fd)	fax_simplecom((fd), COM_STOP)
#define fax_sendenq1(fd)	fax_simplecom((fd), COM_ENQ1)
#define fax_sendenq2(fd)	fax_simplecom((fd), COM_ENQ2)

#define BITSPERBYTE	8


typedef struct _psModem {
	int	fd;		/* fd open to modem */
	char	devName[BUFSIZ];	/* name of device */
	int	resolution;	/* resolution to use */
	int	polled;		/* whether using polling */
	char	*phone_prefix;	/* prefix to add to all phone numbers */
	int	softcar;	/* whether softcar is in use or not */
	int	nattempts;	/* number of times to try to connect to remote site */
	int	sleepFor;	/* seconds to sleep for between attempts */
	int	connected;	/* whether managed to connect or not */
	int	scanner;	/* whether data is waiting on scanner */
				/* or is actual inbound fax */
	char	errBuf[BUFSIZ];
} PsModem;

#define PSM(faxctl) ((PsModem *) ((faxctl)->softc))

extern char	*faxerr2str();
