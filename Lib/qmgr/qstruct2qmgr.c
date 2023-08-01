/* qstruct2qmgr.c: conversion routine from verbose submit struct to concise
qmgr struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/qstruct2qmgr.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/qstruct2qmgr.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: qstruct2qmgr.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/cmd_srch.h>

/* Qmgr types */
#include "Qmgr-types.h"

/* Sumbit types */
#include "q.h"
#include "prm.h"

extern CMD_TABLE qtbl_con_type[];
/*  */
/* Routines */
extern time_t utctotime();
static struct type_Qmgr_PerMessageInfo *fillin_messageinfo();
static struct type_Qmgr_MPDUIdentifier *fillin_mpduiden();
static struct type_Qmgr_GlobalDomainIdentifier *fillin_gldmdid();
static struct type_Qmgr_CountryName *fillin_country();
static struct type_Qmgr_AdministrationDomainName *fillin_admd();
static struct type_Qmgr_PrivateDomainIdentifier *fillin_prmd();
static struct type_Qmgr_ContentType *fillin_content();
static struct type_Qmgr_EncodedInformationTypes *fillin_eit();
static struct type_Qmgr_Priority *fillin_prio();
static struct type_UNIV_UTCTime *fillin_age();
static struct type_UNIV_UTCTime *fillin_expiryTime();
static struct type_UNIV_UTCTime *fillin_deferredTime();
static struct type_Qmgr_User *fillin_user();
static struct type_Qmgr_RecipientList *fillin_reciplist();
static struct type_Qmgr_RecipientInfo *fillin_recip();
static struct type_Qmgr_ChannelList *fillin_chans();

/* error flag and values */
int fatal_errors;

/*  */

/* routine to initialise a qstruct */
void qinit (que)
Q_struct *que;
{
	que->inbound = NULL;
	que->encodedinfo.eit_types = NULL;
	que->orig_encodedinfo.eit_types = NULL;
	que->ua_id = NULL;
	que->msgid.mpduid_DomId.global_Country = NULL;
	que->msgid.mpduid_DomId.global_Admin = NULL;
	que->msgid.mpduid_DomId.global_Private = NULL;
	que->msgid.mpduid_string = NULL;
	que->trace = NULL;
	que->Oaddress = NULL;
	que->Raddress = NULL;
}

/* conversion routine */
struct type_Qmgr_MsgStruct *qstruct2qmgr (file,prm,que,sender,recip,rcount)
char            *file;
struct prm_vars *prm;
Q_struct        *que;
ADDR            *sender;
ADDR            *recip;
int             rcount;
{
	struct type_Qmgr_MsgStruct *msg;

	/*
	if fatal_errors becomes true something missing that must be there
	*/
	fatal_errors = FALSE;

	msg = (struct type_Qmgr_MsgStruct *) calloc (1, sizeof ( *msg));

	msg->messageinfo = fillin_messageinfo (file,prm,que,sender);
	msg->recipientlist = fillin_reciplist (file,sender,recip,rcount);

/*      if (msg->recipientlist == NULL) {
*               PP_LOG(LLOG_NOTICE,
*                      ("Lib/qmgr/qstruct2qmgr : no recipients for '%s'",file));
*               free_Qmgr_MsgStruct(msg);
*               msg = NULL;
*       }*/

	if (fatal_errors == TRUE) {
		free_Qmgr_MsgStruct (msg);
		msg = NULL;
	}
	return msg;

}

/*  */
/* per message info routines */

/* ARGSUSED */
static struct type_Qmgr_PerMessageInfo *fillin_messageinfo (
					  file,prm,que,sender)
char            *file;
struct prm_vars *prm;
Q_struct        *que;
ADDR            *sender;
{
	struct type_Qmgr_PerMessageInfo *qmi;

	qmi = (struct type_Qmgr_PerMessageInfo *)
		calloc (1, sizeof ( *qmi));

	if (file != NULLCP)
		qmi->queueid = (struct qbuf *)
				str2qb (file, strlen(file), 1);
	else {
		PP_OPER (file, ("Lib/qmgr/qstruct2qmgr: null queueid"));
		fatal_errors = TRUE;
	}
	qmi->mpduiden = fillin_mpduiden (que->msgid);
	qmi->originator = fillin_user (sender,file);
	qmi->contenttype = fillin_content (que);
	qmi->eit = fillin_eit (que);
	qmi->priority = fillin_prio (que);
	qmi->size = que->msgsize;
	qmi->age = fillin_age (que);
	qmi->warnInterval = que->warninterval;
	qmi->numberWarningsSent = que->nwarns;
	qmi->expiryTime = fillin_expiryTime (que);
	qmi->deferredTime = fillin_deferredTime (que);
	if (que->ua_id != NULL)
		qmi->uaContentId = (struct qbuf *)
				   str2qb (que->ua_id,strlen (que->ua_id),1);
	else qmi->uaContentId = NULL;
	if (que -> inbound && que -> inbound -> li_chan)
		qmi -> inChannel =
			str2qb (que -> inbound -> li_chan -> ch_name,
				strlen (que -> inbound -> li_chan -> ch_name),
				1);
	return qmi;
}



static struct type_Qmgr_MPDUIdentifier *fillin_mpduiden (msgid)
MPDUid  msgid;
{
	struct type_Qmgr_MPDUIdentifier *mpdu;

	mpdu = (struct type_Qmgr_MPDUIdentifier *)
		calloc (1, sizeof (*mpdu));

	mpdu->global = fillin_gldmdid (msgid);
	if (msgid.mpduid_string == NULL) {
		PP_OPER(NULLCP,
			("Lib/qmgr/qstruct2qmgr: missing MsgId string"));
		fatal_errors = TRUE;

		mpdu->local = NULL;
	} else
		mpdu->local = str2qb (msgid.mpduid_string,
						strlen (msgid.mpduid_string),
						1);

	return mpdu;
}

static struct type_Qmgr_GlobalDomainIdentifier *fillin_gldmdid (msgid)
MPDUid  msgid;
{
	struct type_Qmgr_GlobalDomainIdentifier *glid;

	glid = (struct type_Qmgr_GlobalDomainIdentifier *)
		calloc (1,sizeof (*glid));

	glid->country = fillin_country (msgid);
	glid->admd = fillin_admd (msgid);
	glid->prmd = fillin_prmd (msgid);
	return glid;
}

static struct type_Qmgr_CountryName *fillin_country (msgid)
MPDUid  msgid;
{
	struct type_Qmgr_CountryName *country;

	country = (struct type_Qmgr_CountryName *)
		  calloc (1,sizeof (*country));

	country->offset = type_Qmgr_CountryName_printable;
	if (msgid.mpduid_DomId.global_Country == NULL) {
		PP_OPER (NULLCP,
			("Lib/qmgr/qstruct2qmgr : missing MsgId country"));
		fatal_errors = TRUE;
		country->un.printable = NULL;

	} else
		country->un.printable = str2qb (
			      msgid.mpduid_DomId.global_Country,
			      strlen (msgid.mpduid_DomId.global_Country),
			      1);

	return country;
}

static struct type_Qmgr_AdministrationDomainName *fillin_admd (msgid)
MPDUid  msgid;
{
	struct type_Qmgr_AdministrationDomainName *admd;

	admd = (struct type_Qmgr_AdministrationDomainName *)
		calloc (1,sizeof (*admd));

	admd->offset = type_Qmgr_AdministrationDomainName_printable;
	if (msgid.mpduid_DomId.global_Admin == NULL) {
		PP_OPER(NULLCP,
			("Lib/qmgr/qstruct2 missing MsgId admin domain"));
		fatal_errors = TRUE;
		admd->un.printable = NULL;
	} else
		admd->un.printable = str2qb (
			       msgid.mpduid_DomId.global_Admin,
			       strlen (msgid.mpduid_DomId.global_Admin),
			       1);
	return admd;
}

static struct type_Qmgr_PrivateDomainIdentifier *fillin_prmd (msgid)
MPDUid  msgid;
{
	struct type_Qmgr_PrivateDomainIdentifier *prmd = NULL;

	if (msgid.mpduid_DomId.global_Private != NULL) {

		prmd = (struct type_Qmgr_PrivateDomainIdentifier *)
			calloc (1,sizeof (*prmd));

		prmd->offset = type_Qmgr_PrivateDomainIdentifier_printable;
		prmd->un.printable = str2qb (
					     msgid.mpduid_DomId.global_Private,
					     strlen (msgid.mpduid_DomId.global_Private),
					     1);
	}
	return prmd;
}

static struct type_Qmgr_ContentType *fillin_content (que)
Q_struct        *que;
{
	char *str = que->cont_type;

	if (str)
		return str2qb (str,strlen (str),1);
	return NULL;
}

static struct type_Qmgr_EncodedInformationTypes *fillin_eit (que)
Q_struct        *que;
{
	struct type_Qmgr_EncodedInformationTypes *head = NULL,
						 *tail = NULL,
						 *temp = NULL;
	LIST_BPT *ix = que->encodedinfo.eit_types;

	while (ix != NULL) {
		temp = (struct type_Qmgr_EncodedInformationTypes *)
			calloc (1, sizeof (*temp));
		temp->PrintableString = str2qb (ix->li_name, 
					       strlen (ix->li_name),
					       1);
		if (head == NULL)
			head = tail = temp;
		else {
			tail->next = temp;
			tail = temp;
		}
		ix = ix->li_next;
	}
	return head;
}

static struct type_Qmgr_Priority *fillin_prio (que)
Q_struct        *que;
{
	struct type_Qmgr_Priority *prio;

	prio = (struct type_Qmgr_Priority *) calloc (1,sizeof (*prio));
	prio->parm = que->priority;
	return prio;
}

static struct type_UNIV_UTCTime *fillin_age (que)
Q_struct        *que;
{
	struct type_UNIV_UTCTime *ti;
	char *str;

	str = utct2str (que->queuetime);
	ti = str2qb (str,strlen (str),1);

	return ti;
}


static struct type_UNIV_UTCTime *fillin_expiryTime (que)
Q_struct        *que;
{
	time_t t;
	UTC utc;
	char *str;

	/* expiry time = queuetime + retinterval */

	t = utc2time_t (que -> queuetime);
	t += que -> retinterval * 60 * 60;
	utc = time_t2utc (t);
	str = utct2str (utc);
	free ((char *)utc);
	return str2qb (str, strlen (str), 1);
}

static struct type_UNIV_UTCTime *fillin_deferredTime (que)
Q_struct        *que;
{
	struct type_UNIV_UTCTime *ti;
	char *str;

	if (que -> defertime)
		str = utct2str (que->defertime);
	else
		return NULL;

	ti = str2qb (str, strlen (str), 1);

	return ti;
}

static struct type_Qmgr_User *fillin_user (usr,file)
ADDR            *usr;
char            *file;
{
	struct type_Qmgr_User *user = NULL;
	switch (usr->ad_type) {
	    case AD_X400_TYPE:
		if (usr->ad_r400adr == NULL) {
			PP_OPER(NULLCP,
				("Lib/qmgr/qstruct2qmgr: ad_r400adr = nil in %s",file));
			fatal_errors = TRUE;
		} else
			user = (struct type_Qmgr_User *)
				str2qb (usr->ad_r400adr, strlen (usr->ad_r400adr), 1);
		break;
	    case AD_822_TYPE:
		if (usr->ad_r822adr == NULL) {
			PP_OPER(NULLCP,
				("Lib/qmgr/qstruct2qmgr: ad_r822adr = nil in %s",file));
			fatal_errors = TRUE;
		} else 
			user = (struct type_Qmgr_User *)
				str2qb (usr->ad_r822adr, strlen (usr->ad_r822adr), 1);
		break;
	    case AD_ANY_TYPE:
		if (usr->ad_value == NULL) {
			PP_OPER(NULLCP,
				("Lib/qmgr/qstruct2qmgr: ad_value = nil in %s",file));
			fatal_errors = TRUE;
		} else
			user = (struct type_Qmgr_User *)
				str2qb (usr->ad_value, strlen (usr->ad_value), 1);
		break;
	    default:
		PP_OPER (NULLCP,
			("Lib/qmgr/qstruct2qmgr: unknown address type in %s",
			file));
		fatal_errors = TRUE;
		break;
	}
	return user;
}

/*  */
/* recip list routines */
static struct type_Qmgr_RecipientList *fillin_reciplist (file,sender,recip,rcount)
char            *file;
ADDR            *sender;
ADDR            *recip;
int             rcount;
{
	struct type_Qmgr_RecipientList *ri, *head, *tail;
	int                     ix;
	ADDR                    *adr;

	head = tail = NULL;
	ix = 0;
	adr = recip;

	/* deal with sender */
	ri = (struct type_Qmgr_RecipientList *) calloc(1, sizeof(*ri));
	ri->RecipientInfo = fillin_recip(file, sender);
	head = tail = ri;

	while (ix++ < rcount && adr != NULLADDR) {
		if (adr->ad_status != AD_STAT_DONE) {
			/* okay to pass to qmgr */

			ri = (struct type_Qmgr_RecipientList *)
				calloc (1, sizeof (*ri));
			ri->RecipientInfo = fillin_recip (file,adr);

			/* add to list */
			tail->next = ri;
			tail = ri;
		}
		adr = adr->ad_next;
	}
	if (adr)
		PP_LOG(LLOG_NOTICE,
		       ("qstruct2qmgr/fillin_reciplist: discrepancy between rcount and recip (number in recip list < rcount)"));
	return head;
}

static int countChans(list)
struct type_Qmgr_ChannelList    *list;
{
	int     count = 0;
	while (list != NULL) {
		count++;
		list = list->next;
	}

	return count;
}


static struct type_Qmgr_RecipientInfo *fillin_recip (file, recip)
char            *file;
ADDR            *recip;
{
	struct type_Qmgr_RecipientInfo *ri;

	ri = (struct type_Qmgr_RecipientInfo *) calloc (1,sizeof (*ri));
	ri->user = fillin_user (recip,file);
	ri->id = (struct type_Qmgr_RecipientId *) calloc (1, sizeof *ri->id);
	ri -> id -> parm = recip->ad_no;


	/* --- *** ---
	Note:   if a non DR is to be sent for that recipient then the
		output channel and mta is not required as the sender's
		info is used instead
		BUT mta is not OPTIONAL in ASN.1 so put dummy value in 
	--- *** --- */


	if (recip->ad_outchan
	    && recip->ad_outchan->li_mta != NULLCP)
		ri->mta = (struct type_Qmgr_Mta *)
			str2qb (recip->ad_outchan->li_mta,
				strlen (recip->ad_outchan->li_mta), 1);
	else if (recip->ad_status == AD_STAT_DRWRITTEN) 
		ri->mta = (struct type_Qmgr_Mta *)
			str2qb("Delivery Report Written",
				strlen("Delivery Report Written"), 1);
	else {
		PP_OPER (NULLCP,
			("Lib/qmgr/qstruct2qmgr: %s '%s'",
			"missing recipient mta name in",
			file));
		fatal_errors = TRUE;
	}


	ri->channelList = fillin_chans (
					file,
					recip->ad_status,
					recip->ad_fmtchan,
					recip->ad_outchan
					);
	if (recip->ad_status == AD_STAT_DRWRITTEN)
		ri -> channelsDone = countChans(ri->channelList);
	else
		ri -> channelsDone = recip -> ad_rcnt;
	return ri;
}


static struct type_Qmgr_ChannelList *fillin_chans (file,status,fmt,out)
char            *file;
int             status;
LIST_RCHAN      *fmt,
		*out;
{
	struct type_Qmgr_ChannelList *head = NULL, *ptr, *tmp;
	LIST_RCHAN *ix;

	/* formatters */
	ix = fmt;
	while (ix != NULL) {
		tmp = (struct type_Qmgr_ChannelList *)
			calloc (1,sizeof (*tmp));
		tmp->next = NULL;
		if ((ix->li_chan != NULL)
		   && (ix->li_chan->ch_name != NULL))
			tmp->Channel = (struct type_Qmgr_Channel *)
					str2qb (ix->li_chan->ch_name,
					       strlen (ix->li_chan->ch_name),
					       1);
		else {
			PP_OPER (NULLCP,
				("Lib/qmgr/qstruct2qmgr: %s '%s'",
				"null channel name in",
				file));
			fatal_errors = TRUE;
		}

		/* add to list */
		if (head == NULL)
			head = tmp;
		else {
			ptr = head;
			while (ptr->next != NULL)
				ptr = ptr->next;
			ptr->next = tmp;
		}
		ix = ix->li_next;
	}

	/* outputters */
	ix = out;
	if ((ix == NULL) && (status != AD_STAT_DRWRITTEN)) {
			PP_OPER (NULLCP,
				("Lib/qmgr/qstruct2qmgr: %s '%s'",
				"missing out channel in",
				file));
			fatal_errors = TRUE;
	} 

	while (ix != NULL) {
		tmp = (struct type_Qmgr_ChannelList *)
			calloc (1,sizeof (*tmp));
		tmp->next = NULL;
		if ((ix->li_chan != NULL)
			&& (ix->li_chan->ch_name != NULL))
			tmp->Channel = (struct type_Qmgr_Channel *)
				str2qb (ix->li_chan->ch_name,
				strlen (ix->li_chan->ch_name),
				1);
		else {
			PP_OPER (NULLCP,
				("Lib/qmgr/qstruct2qmgr: %s '%s'",
				"null out channel name in",
				file));
			fatal_errors = TRUE;
		}

		/* add to list */
		if (head == NULL)
			head = tmp;
		else {
			ptr = head;
			while (ptr->next != NULL)
				ptr = ptr->next;
			ptr->next = tmp;
		}
		ix = ix->li_next;
	}
	return head;
}
