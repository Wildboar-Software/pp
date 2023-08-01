/* tb_getprog.c: get prog/shell channel entry */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/shell/RCS/tb_getprog.c,v 6.0 1991/12/18 20:11:52 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/shell/RCS/tb_getprog.c,v 6.0 1991/12/18 20:11:52 jpo Rel $
 *
 * $Log: tb_getprog.c,v $
 * Revision 6.0  1991/12/18  20:11:52  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"table.h"
#include	"prog.h"

extern char	*compress();

#define 	DEFAULT_TIMEOUT	5*60	/* 5 mins */

ProgInfo *tb_getprog (key, tbl)
char		*key;
Table		*tbl;

{
	char	buf[BUFSIZ],
		*timeout_value,
		*ix,*ix2;
	struct passwd *pwd;
	char	*cp;
	ProgInfo *prog;

	PP_DBG(("tb_getprog (%s)", key));

	if (tbl == NULLTBL) {
		PP_LOG(LLOG_FATAL, ("tb_getprog (no table)"));
		return NULLPROG;
	}

	if (tb_k2val (tbl, key, buf, TRUE) == NOTOK)
		return (NULLPROG);

	/* syntax uid,timeout,cmd line */

	if ((ix = index(buf,',')) == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("incorrect table syntax for entry '%s' (missing ,)",
			key));
		return NULLPROG;
	}
	*ix++ = '\0';

	prog = (ProgInfo *) smalloc (sizeof *prog);
	bzero ((char *)prog, sizeof *prog);

	/* uid is either a name or a uid/gid pair */
	if (isdigit (*buf) && (cp = index (buf, '/'))) {
		prog -> username = NULLCP;
		*cp ++ = NULL;
		prog -> uid = atoi (buf);
		prog -> gid = atoi (cp);
		prog -> home = strdup ("/tmp");
	}
	else {
		prog->username = strdup(buf);
	
		if ((pwd = getpwnam(prog->username)) == NULL) {
			PP_OPER(NULLCP,
				("tb_getprog no user '%s' with key '%s'",
				 prog->username, key));
			
			prog_free (prog);
			return NULLPROG;
		}
		prog -> uid = pwd -> pw_uid;
		prog -> gid = pwd -> pw_gid;
                prog -> home = strdup (pwd -> pw_dir);
		if (pwd -> pw_shell)
			prog -> shell = strdup (pwd -> pw_shell);
	}
	if (prog -> shell == NULLCP)
		prog -> shell = strdup ("/bin/sh");

	timeout_value  = ix;
	if ((ix = index(timeout_value, ',')) == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("incorrect table syntax for '%s' entry (missing ,:)",
			key));
		prog_free (prog);
		return NULLPROG;
	}
	*ix++ = '\0';
	(void) compress(timeout_value, timeout_value);
	if ((ix2 = index(timeout_value, '|')) != NULLCP) 
		*ix2++ = '\0';

	if (timeout_value[0] == '\0') 
		prog -> timeout_in_secs = DEFAULT_TIMEOUT;
	else
		prog -> timeout_in_secs = atoi(timeout_value);
	
	if (ix2 != NULLCP
	    && lexnequ(ix2, "solo", strlen("solo")) == 0)
		prog -> solo = TRUE;
	else
		prog -> solo = FALSE;

	prog -> cmd_line = strdup(ix);
	return prog;
}

void prog_free (prog)
ProgInfo *prog;
{
	if (prog == NULLPROG)
		return;
	if (prog -> cmd_line)
		free (prog -> cmd_line);
	if (prog -> username)
		free (prog -> username);
	if (prog -> home)
		free (prog -> home);
	if (prog -> shell)
		free (prog -> shell);
	free ((char *)prog);
}
