/* ut_qid.c: A qid2dir mapping utility */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_qid.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_qid.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_qid.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "util.h"
#include        "adr.h"
#include        <sys/stat.h>

extern char     *quedfldir;     /* -- "/usr/spool/pp/queues" -- */
extern char     *bquedir;       /* -- "base" -- */

static int QID_adr_chks ();
static int QID_get_dirname ();


/* ---------------------  Begin  Routines  -------------------------------- */




/* converts qid + reformatter list into directory with message in */

int qid2dir (qid, ap, exists, cpp)
char    *qid;           /* -- queue id (given) -- */
ADDR    *ap;            /* -- recipient address (given) -- */
int     exists;         /* -- the directory should exist (complain if not) */
char    **cpp;          /* -- malloced to contain the dir name (updated) -- */
{
	struct stat st;
	PP_DBG (("Lib/pp/qid2dir (%s)", qid));

	if (*cpp) free (*cpp);
	*cpp = NULLCP;


	/* -- checks that a valid recipient address has been specified -- */

	if (ap == NULLADDR || ap -> ad_status == AD_STAT_DONE) {
		PP_LOG (LLOG_EXCEPTIONS, ("Lib/qid2dir: %s",
					  ap ? "bad address status" :
					  "no address!"));
		return (NOTOK);
	}


	/* -- *** ---
	gets the current dir for that msg version associated with that
	recipient address
	--- *** --- */


	if (QID_get_dirname (ap, qid, cpp) == NOTOK)
		return (NOTOK);


	/* -- checks that the directory exists by stat ing it -- */

	if (exists == TRUE && (stat(*cpp, &st) == NOTOK ||
			       (st.st_mode & S_IFMT) != S_IFDIR)) {
		PP_SLOG (LLOG_EXCEPTIONS, *cpp,
			 ("Lib/qid2dir:Missing directory"));
		if (*cpp) free (*cpp);
		*cpp = NULLCP;
		return (NOTOK);
	}
	return (OK);
}



/* ---------------------  Static  Routines  ------------------------------- */

static int QID_get_dirname (ap, msg_dir, cpp)
ADDR                    *ap;
char                    *msg_dir, **cpp;
{
	char            buf[MAXPATHLENGTH], *cp;
	LIST_RCHAN      *rp = ap->ad_fmtchan;
	int             i;


	PP_DBG (("Lib/pp/QID_get_dirname()"));

	if (ap->ad_rcnt == 0) {
		*cpp = malloc ((unsigned) (strlen (msg_dir) + 1
					 + 1 + strlen (bquedir)));
		(void) sprintf (*cpp, "%s/%s", msg_dir, bquedir);
		return (OK);
	}
	else if (rp == NULLIST_RCHAN && ap->ad_rcnt != 0) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/pp/QID_get_dirname - missing reformatters"));
		return (NOTOK);
	}

	cp = NULLCP;
	for (i = ap -> ad_rcnt; i > 0; i --) /* find current formatter */
		if (rp == NULL) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Lib/pp/QID_get_dirname (rfmt err)"));
			return NOTOK;
		}
		else {
			cp = rp -> li_dir;
			rp = rp -> li_next;
		}

	if (cp) {	/* new style format */
		*cpp = malloc ((unsigned) (strlen(msg_dir) + 1 +
					   strlen (cp) + 1));
		(void) sprintf(*cpp, "%s/%s", msg_dir, cp);
		return OK;
	}

	/* old style long names - 14 chars is sooo restrictive - sigh */

	rp = ap -> ad_fmtchan;
	cp = buf;


	for (i=ap->ad_rcnt; i > 0; i--)
		if (rp) {
			if (cp != buf)
				*cp ++ = '.';
			(void) strcpy (cp, rp->li_chan->ch_name);
			cp += strlen(cp);
			rp = rp->li_next;
		}
		else {
			PP_LOG (LLOG_EXCEPTIONS,
				("Lib/pp/QID_get_dirname missing reformatters"));
			return (NOTOK);
		}



	*cpp = (char *) malloc ((unsigned) (strlen (msg_dir) + 1
					  + 1 + strlen (bquedir)
					  + 1 + strlen (buf)));
	(void) sprintf (*cpp, "%s/%s.%s", msg_dir, bquedir, buf);


	return (OK);
}
