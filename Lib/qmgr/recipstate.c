/* recipstate.c: support for qmgr responses */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/recipstate.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/recipstate.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: recipstate.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "qmgr.h"
#include "Qmgr-types.h"

#define STR2QB(s)	str2qb((s), strlen((s)), 1)

struct type_Qmgr_DeliveryStatus *deliverystate;

struct type_Qmgr_DeliveryStatus *delivery_setstate (rno, val, msg)
int     rno, val;
char	*msg;
{
	struct type_Qmgr_DeliveryStatus *ds;

	PP_DBG (("delivery_setstate (%d, %d)", rno, val));

	for (ds = deliverystate; ds; ds = ds -> next) {
		if (ds -> IndividualDeliveryStatus -> recipient -> parm == rno) {
			ds -> IndividualDeliveryStatus -> status = val;
			if (msg) {
				if (ds -> IndividualDeliveryStatus -> info)
					qb_free (ds ->
						 IndividualDeliveryStatus ->
						 info);
				ds -> IndividualDeliveryStatus -> info =
					STR2QB (msg);
			}
		}
	}
	return deliverystate;
}

struct type_Qmgr_DeliveryStatus *delivery_init (users)
struct type_Qmgr_UserList *users;
{
	struct type_Qmgr_DeliveryStatus **ds;
	struct type_Qmgr_IndividualDeliveryStatus *ids;

	PP_DBG (("delivery_init (users)"));

	if (deliverystate) {
		free_Qmgr_DeliveryStatus (deliverystate);
		deliverystate = NULL;
	}

	for (ds = & deliverystate; users; users = users -> next,
	     ds = &(*ds)->next) {
		*ds = (struct type_Qmgr_DeliveryStatus *)
			calloc (1, sizeof (**ds));
		ids = (*ds) -> IndividualDeliveryStatus =
			(struct type_Qmgr_IndividualDeliveryStatus *)
				calloc (1, sizeof (struct type_Qmgr_IndividualDeliveryStatus));
		ids -> recipient = (struct type_Qmgr_RecipientId *)
			calloc (1, sizeof *ids -> recipient);
		ids -> recipient -> parm = users -> RecipientId -> parm;
		ids -> status = -1;
		ids -> info = NULL;
	}
	return deliverystate;
}

struct type_Qmgr_DeliveryStatus *delivery_setallstate (val, msg)
int     val;
char	*msg;
{
	struct type_Qmgr_DeliveryStatus *ds;
	int	first = 1;

	PP_DBG (("delivery_setallstate (%d)", val));

	if (deliverystate == NULL)
		return deliverystate;

	for (ds = deliverystate; ds; ds = ds -> next) {
		switch (ds -> IndividualDeliveryStatus -> status) {
		    case int_Qmgr_status_negativeDR:
		    case int_Qmgr_status_positiveDR:
		    case int_Qmgr_status_successSharedDR:
		    case int_Qmgr_status_failureSharedDR:
			break;
		    default:
			ds -> IndividualDeliveryStatus -> status = val;
			if (msg && val == int_Qmgr_status_mtaFailure &&
			    first) {
				if (ds -> IndividualDeliveryStatus -> info)
					qb_free (ds -> IndividualDeliveryStatus
						 -> info);
				ds -> IndividualDeliveryStatus -> info =
					STR2QB (msg);
				first = 0;
			}
			break;
		}
	}
	return deliverystate;
}

struct type_Qmgr_DeliveryStatus *delivery_resetDRs (val)
int     val;
{
	struct type_Qmgr_DeliveryStatus *ds;

	PP_DBG (("delivery_resetDRs (%d)", val));

	if (deliverystate == NULL)
		return deliverystate;

	for (ds = deliverystate; ds; ds = ds -> next) {
		switch (ds -> IndividualDeliveryStatus -> status) {
		    case int_Qmgr_status_negativeDR:
		    case int_Qmgr_status_positiveDR:
		    case int_Qmgr_status_successSharedDR:
		    case int_Qmgr_status_failureSharedDR:
			ds -> IndividualDeliveryStatus -> status = val;
			break;
		    default:
			break;
		}
	}
	return deliverystate;
}
