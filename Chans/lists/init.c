/* init.c: */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/init.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/init.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: init.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include <isode/quipu/attrvalue.h>

AttributeType at_Member;
AttributeType at_Owner;
AttributeType at_Permit;
AttributeType at_Policy;
AttributeType at_ORAddress;
AttributeType at_RFC822;
AttributeType at_GroupMember;
AttributeType at_RoleOccupant;

OID dl_oc;
OID role_oc;

extern LLog * log_dsap;

pp_quipu_init (str)
char * str;
{
        pp_syntaxes ();
        pp_initialise(str, 0);
        or_myinit();
}

mhs_syntaxes ()
{
	orAddr_syntax ();
	orName_syntax ();
	permit_syntax ();
	policy_syntax ();
}

pp_syntaxes () {
	mhs_syntaxes ();
        policy_syntax ();
}

pp_quipu_run ()
{
int result = TRUE;
int ufn_init ();

	if ((dl_oc = name2oid ("ppDistributionList")) == NULLOID) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("ppDistributionList unknown Objectclass"));
	}
	if ((role_oc = name2oid ("organizationalRole")) == NULLOID) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("organizationalRole unknown Objectclass"));
	}
		
		/* DL Mandatory */
	if ((at_Member = AttrT_new ("mhsDLMembers")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("mhsDLMembers attribute unknown"));
	}
	if ((at_Owner	= AttrT_new ("Owner")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("Owner attribute unknown"));
	}
	if ((at_Permit = AttrT_new ("mhsDLSubmitPermissions")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("mhsDLSubmitPermissions attribute unknown"));
	}

		/* DL Optional */
	if ((at_Policy = AttrT_new ("dl-policy")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("dl-policy attribute unknown"));
	}

		/* MHS mail box attrs */
	if ((at_ORAddress = AttrT_new ("mhsORAddresses")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("mhsORAddresses attribute unknown"));
	}
	if ((at_RFC822 = AttrT_new ("rfc822Mailbox")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("rfc822Mailbox attribute unknown"));
	}

		/* X500 group member */
	if ((at_GroupMember = AttrT_new ("member")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("member attribute unknown"));
	}
	if ((at_RoleOccupant = AttrT_new ("RoleOccupant")) == NULLAttrT) {
		result = FALSE;
		LLOG (log_dsap,LLOG_EXCEPTIONS,("RoleOccupant attribute unknown"));
	}

	if (result)
		result = ufn_init();
	else
		(void) ufn_init ();

	return result;
}


