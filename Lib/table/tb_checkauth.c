/* tb_checkauth.c: authentication checks for submission */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_checkauth.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_checkauth.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_checkauth.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "chan.h"

#define         MAX_USER_ARGS           5
#define         MAX_USERIDS             50

extern char *crypt ();

/* ---------------------  Begin  Routines  -------------------------------- */

int tb_checkauth (key, channel, username, passwd)
char            *key;
CHAN            *channel;
char            *username;
char            *passwd;
{
	int     argc, argc2;
	char    *argv [MAX_USER_ARGS],
		*argv2[MAX_USERIDS],
		buf [BUFSIZ];
	int     i;

	PP_DBG (("Lib/tb_checkauth (%s, %s)", key, channel -> ch_name));

	if (channel -> ch_auth_tbl == NULL) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Warning: Channel %s (%s) doesn't have an authentication table",
			channel -> ch_name, channel -> ch_show));
		return OK;
	}

	if (tb_k2val (channel -> ch_auth_tbl, key, buf, TRUE) == NOTOK)
		return (NOTOK);

	if ((argc = sstr2arg (buf, MAX_USER_ARGS, argv, "|")) == NOTOK)
		return (NOTOK);

	if (username && argc > 1) {
		if ((argc2 = sstr2arg (argv[1], MAX_USERIDS, argv2, ","))
		    == NOTOK)
			return NOTOK;

		for (i = 0;  i < argc2; i++)
			if (lexequ (argv2[i], username) == 0)
				return OK;
	}

	if (passwd && argc > 2) {
		if (strcmp (passwd, crypt (passwd, argv[2])) == 0)
			return OK;
	}
	PP_LOG(LLOG_EXCEPTIONS, ("Username %s not allowed for %s on chan %s",
				 username, key, channel -> ch_name));
	return (NOTOK);
}
