/* mta.h: various structure definitions  */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/mta.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: mta.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_MTA
#define _H_MTA


#include "adr.h"
#include "list_bpt.h"
#include <isode/psap.h>

/* -- MTS papameters */

typedef struct	GlobalDomainidentifier {
	char	*global_Country;
	char	*global_Admin;
	char	*global_Private;
	} GlobalDomId;

void	GlobalDomId_new ();
void	GlobalDomId_free ();

typedef struct	MPDUidentifier {
	GlobalDomId	mpduid_DomId;
	char		*mpduid_string;
	} MPDUid;

void	MPDUid_new ();
void	MPDUid_free ();

typedef struct	EncodedInformationTypes {
	LIST_BPT	*eit_types;	/* subset of BodyPartTypes
					 -- also contains OID's */
	long		eit_g3parms;
	long		eit_tTXparms;
	long		eit_presentation;
} EncodedIT;


/* -- MTA parameters */

typedef struct	DomainSuppliedInfo {
	UTC    		dsi_time;
	UTC		dsi_deferred;
	int		dsi_action;
#define ACTION_RELAYED                  0
#define ACTION_ROUTED                   1
	int 		dsi_other_actions;
#define ACTION_REDIRECTED 0x1
#define ACTION_EXPANDED	  0x2
	EncodedIT	dsi_converted;
	GlobalDomId	dsi_attempted_md;
	char		*dsi_attempted_mta;
} DomSupInfo;

void	DomSupInfo_free ();

			/* No need to distinguish types of trace in PP */
			/* Just see if MTA is present */
typedef struct	TraceInformation {
	char				*trace_mta;
	GlobalDomId			trace_DomId;
	DomSupInfo			trace_DomSinfo;
	struct	TraceInformation	*trace_next;
} Trace;

Trace	*trace_new ();
Trace	*trace_dup();
void	trace_add ();
void	trace_free ();

typedef struct full_name {
	char *fn_addr; /* O/R Adress */
	char *fn_dn;  /* DN */
} FullName;

FullName	*fn_new ();
FullName	*fn_dup ();
void		fn_free ();

typedef struct dl_history {
	struct dl_history *dlh_next;
	char *dlh_addr; /* O/R Adress */
	char *dlh_dn;  /* DN */
	UTC	dlh_time;
} DLHistory;

DLHistory	*dlh_new();
void		dlh_add ();
DLHistory	*dlh_dup ();
void		dlh_free ();

typedef struct p3params {
	MPDUid 	mpduid;
	UTC	submit_time;
	char	*content_type;
	struct qbuf *originating_mta_certificate;
	struct qbuf *proof_of_submission;
} P3params;

#endif
