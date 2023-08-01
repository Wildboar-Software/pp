/* q.h - The QUEUE/ADDR Control File structure */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/q.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: q.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_Q
#define _H_Q


#include "adr.h"
#include "mta.h"

typedef struct Qstruct {

	int		msgtype;	/* Type of message */
#define MT_UMPDU                1
#define MT_DMPDU                2
#define MT_PMPDU                3

/* -- MTS Parameters */

	long		msgsize;	/* Size of message */
	UTC		defertime;	/* Deffered time */
	UTC		latest_time;
	char		latest_time_crit;

	char		nwarns;		/* Number of warning msgs to send */
	int		warninterval;	/* Interval between warning msgs */
	int		retinterval;	/* Interval after which return mail */
	char		*cont_type;	/* Content type of message */
	EncodedIT	orig_encodedinfo; /* Original Encoded Info Types */
	int		priority;	/* Priority of message */
#define PRIO_NORMAL             0
#define PRIO_NONURGENT          1
#define PRIO_URGENT             2

	char		disclose_recips;
	char		implicit_conversion_prohibited;
	char		alternate_recip_allowed;
	char		content_return_request;

	char		recip_reassign_prohibited;
	char		recip_reassign_crit;

	char		dl_expansion_prohibited;
	char		dl_expansion_crit;

	char		conversion_with_loss_prohibited;
	char		conversion_with_loss_crit;
	
	char		*ua_id;		/* UA content id */
	char		*pp_content_correlator; 
					/* PP preferred string form  */
	struct qbuf	*general_content_correlator;
	char		content_correlator_crit;
					/* ASN.1 uninterpreted */
					
	FullName	*originator_return_address;
	char		originator_return_address_crit;

	int		forwarding_request;
	char		forwarding_request_crit;

	struct qbuf	*originator_certificate;
	char		originator_certificate_crit;

	struct qbuf	*algorithm_identifier;
	char		algorithm_identifier_crit;
 			/* content confidentiality */

	struct qbuf	*message_origin_auth_check; 
				/* also for Probe */
	char		message_origin_auth_check_crit;
				
	struct qbuf	*security_label;
	char		security_label_crit;

	int		proof_of_submission_request;	/* boolean */
	char		proof_of_submission_crit;

	X400_Extension	*per_message_extensions;
					


	ADDR		*Oaddress;	/* Orig addr - linked list */
	ADDR		*Raddress;	/* Recip addrs - linked list */

	

/* -- MTA AS Parameters */

	LIST_RCHAN	*inbound;	/* Inbound MTA/Channel */
	MPDUid		msgid;		/* Message ID */
	Trace		*trace;		/* Trace info */
	DLHistory	*dl_expansion_history;
	char		dl_expansion_history_crit;



/* -- PP calculated parameters */

	EncodedIT	encodedinfo;	/* Encoded information types */
	UTC		queuetime;	/* Time that message was enqueued */
	UTC		departime;	/* Time that message left the queue */
	int		n_bodyparts;
	int		n_forwarded;

	/* -- Queue Control File offsets for fixed length variables -- */

	off_t		nwarns_offset;	/* offset param for nwarns */
} Q_struct;

extern void	q_init ();
extern void	q_free ();
extern	void	encodedinfo_free ();
extern	void	MPDUid_free ();

#define RDMSG_RDONLY	1
#define RDMSG_RDONLYLCK 2
#define RDMSG_RDWR	3


#define rd_msg(file,prm,que,sndr,recip,rcount) \
	rd_msg_file((file),(prm),(que),(sndr),(recip),(rcount),RDMSG_RDWR)
#endif
