/* gen_io2dr.c: write a DR submitted by an X400 incomming channel */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_io2dr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/gen_io2dr.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: gen_io2dr.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include        <sys/types.h>
#include        <sys/stat.h>
#include        "head.h"
#include 	"q.h"
#include        "dr.h"


extern char             msg_fullpath[];
extern long             msg_size;
extern char		*dr_file;
extern int              errno,
			protocol_mode;

extern void	check_dr_crits ();

/* -- local routines -- */
int			gen_io2dr();
static void		reset_protocol_mode();
static void		unset_protocol_mode();




/* ---------------------  Begin  Routines  -------------------------------- */




int gen_io2dr(qp)
Q_struct *qp;
{
	struct stat             st_rec;
	FILE                    *fp;
	DRmpdu                  DeliveryReport, *dr;
	char                    *fname,
				buf[MAXPATHLENGTH];
	int                     retval;

	PP_TRACE (("gen_io2dr (protocol_mode=%d)", protocol_mode));

	dr_init (dr = &DeliveryReport);

	/* -- read the DR from stdin -- */

	if (rp_isbad (retval = rd_dr (dr, stdin))) {
		switch (retval) {
		case RP_EOF:
			PP_LOG (LLOG_EXCEPTIONS,("no more input"));
			return (RP_EOF);
		case RP_PARM:
			PP_LOG (LLOG_EXCEPTIONS, ("gen_io2dr Invalid param"));
			return (RP_PARM);
		default:
			PP_LOG (LLOG_EXCEPTIONS, 
				("gen_io2dr read error %d", retval));
			return (retval);
		}
	}

	check_dr_crits (qp, dr);
	trace_add (&dr -> dr_trace, trace_new());

	/* -- write the DR into the PP queue -- */

	(void) sprintf (buf, "%s/%s%d", msg_fullpath, dr_file,
			DR_FILENO_DEFAULT);
	fname = strdup (buf);
	if ((fp = fopen (fname, "w")) == NULLFILE) {
		PP_LOG (LLOG_FATAL, ("gen_io2dr (unable to open %s)", fname));
		return (RP_FIO);
	}

	unset_protocol_mode();
	retval = wr_dr (dr, fp);
	reset_protocol_mode();

	if (rp_isbad (retval)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("gen_io2dr Unable to write"));
		(void) fclose (fp);
		return (RP_PARM);
	}

	if (check_close (fp) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, "gen_io2dr",
			 ("gen_io2dr Unable to close"));
		return (RP_FIO);
	}

	if (stat (fname, &st_rec) == NOTOK)
		PP_LOG (LLOG_EXCEPTIONS,
			("gen_io2dr: Unable to stat %s %d", fname, errno));
	msg_size = st_rec.st_size;

	dr_free (dr);

	PP_TRACE (("gen_io2dr returns (%d)", retval));

	return (retval);
}



/* ---------------------  Static  Routines  ------------------------------- */



static void unset_protocol_mode ()
{
	reset_protocol_mode();
	protocol_mode = 0;
	return;
}


static void reset_protocol_mode ()
{
	static int      save = -1;

	if (save == -1)
		save = protocol_mode;
	else {
		protocol_mode = save;
		save = -1;
	}
}
