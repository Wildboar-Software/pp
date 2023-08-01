/* tb_getuser.c: grab an user */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getuser.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getuser.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_getuser.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "chan.h"
#include        "list_rchan.h"


#define         MAX_ALTERNATIVE_CHANS           20
#define		MAX_USER_ARGS			5

/* ---------------------  Begin  Routines  -------------------------------- */


extern char	*user_tbl, *loc_dom_mta;

LIST_RCHAN	*tb_getuser (key, subdom)
char		*key;
char		*subdom;
{
	char    *argv [MAX_ALTERNATIVE_CHANS],
		*subargv[MAX_USER_ARGS],
		buf [BUFSIZ],
		tblname[MAXPATHLENGTH];
	int	argc, i;
	Table	*User = NULLTBL;
	LIST_RCHAN	*chans = NULLIST_RCHAN, *tmp;

	if (subdom == NULLCP || *subdom == '\0')
		(void) sprintf(tblname,"%s", user_tbl);
	else
		(void) sprintf(tblname,"%s-%s", subdom, user_tbl);


	PP_DBG (("Lib/tb_getuser (%s)", tblname));


	if ((User = tb_nm2struct (tblname)) == NULLTBL) {
		PP_OPER (NULLCP, ("Lib/tb_getuser (no table '%s')", tblname));
		return (NULLIST_RCHAN);
	}

	if (tb_k2val (User, key, buf, TRUE) == NOTOK)
		return (NULLIST_RCHAN);

	if ((argc = sstr2arg (buf, MAX_ALTERNATIVE_CHANS, argv, ",")) == NOTOK)
		return (NULLIST_RCHAN);

	for (i = 0; i < argc; i++) {
		if (!isstr(argv[i]))
			continue;
		    
		if ((sstr2arg (argv[i], MAX_USER_ARGS, subargv, " ")) == NOTOK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unable to parse delivery channels for '%s' in the user table '%s'",  
				key, tblname));
			list_rchan_free(chans);
			return NULLIST_RCHAN;
		}
		if ((tmp = list_rchan_new ((subargv[1] == NULLCP ||
					    *subargv[1] == '\0') ?
					   loc_dom_mta : subargv[1],
					   NULLCP)) == NULLIST_RCHAN) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unable to find delivery channel '%s' for '%s' in the user table '%s'",  
				argv[0], key, tblname));
			list_rchan_free(chans);
			return NULLIST_RCHAN;
		}
		tmp -> li_chan = ch_nm2struct(subargv[0]);
		list_rchan_add (&chans, tmp);
	}
	return (chans);
}

