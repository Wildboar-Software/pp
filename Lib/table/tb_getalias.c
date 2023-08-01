/* tb_getalias.c: grab an alias */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getalias.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getalias.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_getalias.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        <isode/cmd_srch.h>
#include        "table.h"
#include	"adr.h"
#include        "alias.h"


#define         MAX_ALIAS_ARGS          10


static CMD_TABLE   aliastbl_type[] = {
	"synonym",              ALIAS_SYNONYM,
	"alias",                ALIAS_PROPER,
	0,                      -1
	};

static CMD_TABLE   aliastbl_qual[] = {
	"822", 			AD_822_TYPE,
	"x400",			AD_X400_TYPE,
	"external",		ALIAS_EXTERNAL,
	0, 			-1
	};



/* ---------------------  Begin  Routines  -------------------------------- */


/*
This routine returns the values:
	NOTOK   -  Table or routine error.
	OK      -  Alias found in Table
	DONE    -  Alias not found in Table (- alias is a proper user name)
*/

extern	char	*alias_tbl;

int tb_getalias (key, alias, subdom)
char            *key;
ALIAS           *alias;
char		*subdom;
{

	char    *argv [MAX_ALIAS_ARGS],
	buf [BUFSIZ], tblname[MAXPATHLENGTH];
	int	argc, i, qual;
	Table    *Alias = NULLTBL;


	if (subdom == NULLCP || *subdom == '\0')
		(void) sprintf(tblname,"%s", alias_tbl);
	else
		(void) sprintf(tblname,"%s-%s", subdom, alias_tbl);

	PP_DBG (("Lib/tb_getalias (%s)", tblname));


	if ((Alias = tb_nm2struct (tblname)) == NULLTBL) {
		PP_OPER (NULLCP, ("Lib/tb_getalias (no table '%s')",tblname));
		return (NOTOK);
	}

	if (tb_k2val (Alias, key, buf, TRUE) == NOTOK)
		return (DONE);


	/* -- get alias arguments -- */

	if ((argc = sstr2arg (buf, MAX_ALIAS_ARGS, argv, "\t ")) == NOTOK)
		return (NOTOK);


	switch (alias->alias_type = cmd_srch (argv[0], aliastbl_type)) {
	    case ALIAS_SYNONYM:
	    case ALIAS_PROPER:
		break;
	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/tb_getalias (err type=%d)", alias->alias_type));
		return (NOTOK);
	}


	(void) strcpy (&alias->alias_user[0], argv[1]);

	alias->alias_external = 0;
	alias->alias_ad_type = AD_ANY_TYPE;

	for (i = 2; i < argc; i++) {
		if (!isstr(argv[i]))
			continue;
		switch ((qual = cmd_srch (argv[i], aliastbl_qual))) {
		    case AD_X400_TYPE:
		    case AD_822_TYPE:
			alias->alias_ad_type = qual;
			break;
		    case ALIAS_EXTERNAL:
			alias->alias_external = qual;
			break;
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Lib/tb_getalias unknown qualifier '%s'",
				 argv[i]));
			return (NOTOK);
				
		}
	}
	return (OK);
}
