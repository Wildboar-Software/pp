/* chan.h: the description of a channel structure */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/chan.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: chan.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_CHAN
#define _H_CHAN

#include "table.h"
#include "list_bpt.h"



#define CH_MAX_SORT			10


typedef struct	chan_struct {
	char	    *ch_name;		/* channel name */
	char	    *ch_progname;	/* prog name referencing this chan */
	char	    *ch_show;		/* Display field of the chan title */
	LIST_BPT    *ch_key;		/* The key to this chan */
	
	int	    ch_chan_type;	/* What kind of channel is this */
#define CH_SHAPER		0
#define CH_IN			1
#define CH_OUT			2
#define CH_BOTH			3
#define CH_WARNING		4
#define CH_DELETE		5
#define CH_QMGR_LOAD		6
#define CH_DEBRIS		7
#define CH_TIMEOUT		8
#define CH_POSTMASTER		9
#define CH_SPLITTER		10

	char	    *ch_drchan;		/* Channel for DRs */
					
	int	    ch_cost;		/* How expensive to run */
					
	char	    ch_sort[CH_MAX_SORT];  /* How to sort the msgs */
#define CH_SORT_USR		01
#define CH_SORT_MTA		02
#define CH_SORT_PRIORITY	03
#define CH_SORT_TIME		04
#define CH_SORT_SIZE		05
#define CH_SORT_NONE		06

	char	    *ch_in_info;	/* Info locally interpreted by chan */
	char	    *ch_out_info;	/* Info locally interpreted by chan */
	char	    *ch_content_in;	/* Incoming content of a message */
	char	    *ch_content_out;	/* Outgoing content of a message */

	int	    ch_in_ad_type;		/* Address in types */
	int	    ch_out_ad_type;		/* Address out types */
	int	    ch_in_ad_subtype;	/* Address subtypes */
					/* Used outbound */
					/* on inbound to turn on percentages */
					/* as routing */
	int	    ch_out_ad_subtype;	/* address subtype outbound */
	
	int	    ch_ad_order;	/* Ordering of the incoming domain */
					/* for submit to interpret */
#define CH_USA_ONLY		00
#define CH_USA_PREF		01
#define CH_UK_ONLY		02
#define CH_UK_PREF		03


	LIST_BPT    *ch_hdr_in;		/* Incoming header types */
	LIST_BPT    *ch_hdr_out;	/* Outgoing header types  */
	LIST_BPT    *ch_bpt_in;		/* Incoming body part types */
	LIST_BPT    *ch_bpt_out;	/* Outgoing body part types  */
	Table	    *ch_mta_table;	/* MTA matching table */
	Table	    *ch_table;		/* outbound table */
	Table	    *ch_in_table;	/* inbound table */

	Table	    *ch_auth_tbl;	/* Table of authentication stuff */
	char	    *ch_mta;		/* Mta to route through */
	unsigned int
	    ch_probe:1,	/* Does the channel support probes */
	    ch_domain_norm:1,	/* parse all domains or just next hop */
#define CH_DOMAIN_NORM_PARTIAL	0
#define CH_DOMAIN_NORM_ALL	1
	    ch_conversion:2,	/* what sort of conversion this does */
#define CH_CONV_NONE		0
#define CH_CONV_1148		1
#define CH_CONV_CONVERT		2
#define CH_CONV_WITHLOSS	3
	    ch_access:1,		/* The type of channel access */
#define CH_MTS			00
#define CH_MTA			01
	   ch_trace_type:2,	/* Text type of trace for messages */
					/* interpreted by submit */
#define CH_TRACE_RECEIVED	1
#define CH_TRACE_VIA		2
#define CH_TRACE_X400		3
	   ch_solo_proc:1,
	/* what to do with DRs for unroutable senders */
#define CH_BADSENDER_STRICT	0
#define CH_BADSENDER_SLOPPY	1
	   ch_badSenderPolicy:2,
#define CH_STRICT_CHECK		0
#define CH_SLOPPY_CHECK		1
	   ch_strict_mode:1;

	int	    ch_maxproc;		/* Max instances of channel qmgr */
					/* can run */
} CHAN;


#define NULLCHAN		((CHAN *)0)

extern	CHAN			*ch_nm2struct();
extern	CHAN			*ch_mta2struct();
extern	CHAN			**ch_all;  /* all chans from tailor file */

#endif
