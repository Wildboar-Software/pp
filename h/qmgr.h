/* qmgr.h: basic qmgr library definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/qmgr.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: qmgr.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_QMGR_DEFS
#define _H_QMGR_DEFS

#include "Qmgr-types.h"
#include "Qmgr-ops.h"

extern struct type_Qmgr_DeliveryStatus *deliverystate;

extern struct type_Qmgr_DeliveryStatus *delivery_setstate (),
	*delivery_init (),
	*delivery_setallstate ();

#define delivery_set(x,y) delivery_setstate((x),(y),NULLCP)
#define delivery_setall(x) delivery_setallstate ((x), NULLCP)


#define CHANNEL_CTX_OID	"0.9.2342.60200172.200.1"
#define CHANNEL_PCI_OID	"0.9.2342.60200172.200.2"
#define QMGR_CTX_OID	"0.9.2342.60200172.201.1"
#define QMGR_PCI_OID	"0.9.2342.60200172.201.2"

#define CHANNEL_AC	str2oid(CHANNEL_CTX_OID)
#define CHANNEL_PCI	str2oid(CHANNEL_PCI_OID)
#define QMGR_AC		str2oid(QMGR_CTX_OID)
#define QMGR_PCI	str2oid(QMGR_PCI_OID)

extern char *dap_user, *dap_passwd;

#endif
