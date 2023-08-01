/* pp_setuserid.c: set user id to pp owner */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/tai/RCS/pp_setuserid.c,v 6.0 1991/12/18 20:24:59 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/tai/RCS/pp_setuserid.c,v 6.0 1991/12/18 20:24:59 jpo Rel $
 *
 * $Log: pp_setuserid.c,v $
 * Revision 6.0  1991/12/18  20:24:59  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <pwd.h>


extern char     *pplogin;




/* ---------------------  Begin  Routines  -------------------------------- */




int pp_setuserid()
{
	struct  passwd          *pwd;

	if ((pwd = getpwnam(pplogin)) == (struct passwd *)0)
		return (NOTOK);

	if (setuid (pwd->pw_uid) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/pp_setuserid.c/Can't set uid to %d(%s)",
			 pwd -> pw_uid, pplogin));
		return (NOTOK);
	}
	return (OK);
}
