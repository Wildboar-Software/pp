/* pps_mail.c: simple mail interface */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/pps_mail.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/pps_mail.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: pps_mail.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



/* __________________________________________________________________________


PP Simple interface, sort of equivalent to the MMDF ml_ library.
Works through submit though (as does the newer MMDF).
This code makes several assumptions.
        1) The input will always be RFC-822.
        2) The input will always be a local submission -
                i.e. you can't pretend you are a channel.

___________________________________________________________________________ */



#include "head.h"
#include <isode/cmd_srch.h>
#include "prm.h"
#include "q.h"
#include "adr.h"
#include <pwd.h>


#define INIT		1
#define TO		2
#define CC		3
#define HEADER		4
#define BODY		5


extern	char		*loc_dom_site;
extern  char		*local_822_chan;
extern  char		*hdr_822_bp;
extern  char		*ia5_bp;
static	int		state = INIT;
static Q_struct		qs;
static char		*cc[120];
static char		*sender;
static char		*subject;
static char		*to[120];
static int		cccnt;
static int		tocnt;
static struct prm_vars	pv;
static UTC start_time;

/* -- local routines -- */
int			pps_1adr();
int			pps_adr();
int			pps_aend();
int			pps_cc();
int			pps_end();
int			pps_file();
int			pps_hdr();
int			pps_init();
int			pps_tinit();
int			pps_to();
int			pps_txt();
static int		pps_adrhdr();
static void		pps_set();




/* ---------------------  Start of Routines  ------------------------------ */




int pps_init (subj, rp)
char	*subj;		/* the subject field - or not */
RP_Buf *rp;
{
	struct passwd	*pwd;
	LIST_BPT	*new;
	ADDR		*ap;
	UTC	now, utclocalise ();

	if (start_time)
		free ((char *)start_time);
	now = utcnow ();
	start_time = utclocalise (now);
	free ((char *)now);

	if (state != INIT)
		(void) pps_end (NOTOK, rp);
	tocnt = cccnt = 0;
	if (subj)
		subject = strdup (subj);
	else
		subject = NULLCP;

	if (rp_isbad (io_init(rp)))
		return (pps_end (NOTOK, rp));

	if (rp_isbad (io_wprm (&pv, rp)))
		return (pps_end (NOTOK, rp));

	q_init (&qs);
	qs.inbound = list_rchan_new (loc_dom_site, local_822_chan); 

	qs.encodedinfo.eit_types = list_bpt_new (hdr_822_bp);
	new = list_bpt_new (ia5_bp);
	list_bpt_add (&qs.encodedinfo.eit_types, new);


	if (rp_isbad (io_wrq (&qs, rp)))
		return (pps_end (NOTOK, rp));

	if ((pwd = getpwuid (getuid())) == NULL)
		return (pps_end (NOTOK, rp));

	ap = adr_new (pwd->pw_name, AD_822_TYPE, 0);
	ap -> ad_status = AD_STAT_DONE;
	ap -> ad_resp = NO;
	sender = strdup (pwd->pw_name);

	(void) io_wadr (ap, AD_ORIGINATOR, rp);
	adr_tfree (ap);

	if (rp_isbad (rp -> rp_val))
		return (pps_end (NOTOK, rp));

	state = TO;
	return (OK);
}




int pps_adr (adr, rp)
char	*adr;
RP_Buf *rp;
{
	ADDR	*ap;
	int	retval;

	if (state != TO && state != CC) {
		pps_set (rp, RP_MECH, "Out of sync");
		return (pps_end (NOTOK, rp));
	}


	switch (state) {
	case TO:
		to[tocnt++] = strdup (adr);
		break;
	case CC:
		cc[cccnt++] = strdup (adr);
		break;
	default:
		return (pps_end (NOTOK, rp));
	}

	ap = adr_new (adr, AD_822_TYPE, 1);
	retval = io_wadr (ap, AD_RECIPIENT, rp);
	adr_tfree (ap);

	if (rp_isbad (retval))
		return (pps_end (NOTOK, rp));

	return (OK);
}




int pps_aend(rp)
RP_Buf *rp;
{
	extern UTC	utclocalise();
	char		buf[BUFSIZ];

	if (state != TO && state != CC) {
		pps_set (rp, RP_MECH, "Out of sync");
		return (pps_end (NOTOK, rp));
	}
	if (rp_isbad (io_adend (rp)))
		return (pps_end (NOTOK, rp));

	if (rp_isbad (io_tinit (rp)) ||
	    rp_isbad (io_tpart ("hdr.822", FALSE, rp)))
		return (pps_end (NOTOK, rp));

	state = HEADER;

	(void) UTC2rfc (start_time, buf);
	free ((char *)start_time);
	start_time = NULLUTC;
	if (pps_hdr ("Date:", buf, rp) == NOTOK ||
	    pps_hdr ("From:", sender, rp) == NOTOK)
		return NOTOK;
	free (sender);
	sender = NULLCP;

	if (tocnt > 0 && pps_adrhdr ("To:", to, tocnt, rp) != OK)
		return NOTOK;
	if (cccnt > 0 && pps_adrhdr ("Cc:", cc, cccnt, rp) != OK)
		return NOTOK;

	if (isstr (subject)) {
		if (pps_hdr ("Subject:", subject, rp) == NOTOK)
			return NOTOK;
		free (subject);
		subject = NULLCP;
	}

	return (OK);
}




int pps_tinit(rp)
RP_Buf *rp;
{

	if (rp_isbad (io_tdata ("\n", 1))) {
		pps_set (rp, RP_MECH, "Write error");
		return (NOTOK);
	}
	if (rp_isbad (io_tdend (rp)) ||
	    rp_isbad (io_tpart ("1.ia5", FALSE, rp)))
		return (pps_end (NOTOK, rp));
	state = BODY;
	return (OK);
}




int pps_txt (str, rp)
char	*str;
RP_Buf	*rp;
{
	if (state != BODY) {
		pps_set (rp, RP_MECH, "Out of sync");
		return (pps_end (NOTOK, rp));
	}
	if (rp_isbad (io_tdata (str, strlen (str)))) {
		pps_set (rp, RP_FIO, "Bad data transfer");
		return (pps_end (NOTOK, rp));
	}
	return (OK);
}

int pps_file (fp, rp)
FILE	*fp;
RP_Buf	*rp;
{
	char	copybuf[BUFSIZ];
	int	n;

	if (state != BODY) {
		pps_set (rp, RP_MECH, "Out of sync");
		return (pps_end (NOTOK, rp));
	}
	while (!feof (fp) && !ferror(fp)) {
		if ((n = fread (copybuf, sizeof (char),
				sizeof (copybuf), fp)) > 0)
			if (rp_isbad (io_tdata (copybuf, n))) {
				pps_set (rp, RP_FIO, "Bad data transfer");
				return pps_end (NOTOK, rp);
			}
	}
	return (OK);
}




int pps_end (type, rp)
int	type;
RP_Buf	*rp;
{
	int	i;

	if (type == OK && state == BODY) {
		if (rp_isbad (io_tdend (rp)))
			return (pps_end (NOTOK, rp));
		if (rp_isbad (io_tend (rp)))
			return (pps_end (NOTOK, rp));
	}

	(void) io_end (type);
	for (i = 0; i < tocnt; i++)
		free (to[i]);
	tocnt = 0;
	for (i = 0; i < cccnt; i++)
		free (cc[i]);
	cccnt = 0;
	if (subject) free (subject);
	if (sender) free (sender);
	sender	= NULLCP;
	subject = NULLCP;
	state	= INIT;
	return (type);
}




int pps_to(rp)
RP_Buf	*rp;
{
	if (state == TO || state == CC) {
		state = TO;
		return (OK);
	}
	pps_set (rp, RP_MECH, "Out of sync");
	return (pps_end (NOTOK, rp));
}




int pps_cc(rp)
RP_Buf	*rp;
{
	if (state == TO || state == CC) {
		state = CC;
		return (OK);
	}
	pps_set (rp, RP_MECH, "Out of sync");
	return pps_end(NOTOK, rp);
}




int pps_hdr (name, contents, rp)
char		*name,
		*contents;
RP_Buf	*rp;
{
	char	buffer[LINESIZE];

	if (state != HEADER) {
		pps_set (rp, RP_MECH, "Out of sync");
		return (pps_end (NOTOK, rp));
	}
	(void) sprintf (buffer, "%-10s%s\n", name, contents);
	if (rp_isbad (io_tdata (buffer, strlen (buffer)))) {
		pps_set (rp, RP_MECH, "Write error");
		return pps_end (NOTOK, rp);
	}
	return (OK);
}




int pps_1adr (subj, addr, rp)
char	*subj,
	*addr;
RP_Buf	*rp;
{
	if (pps_init (subj, rp) == OK &&
		pps_adr (addr, rp) == OK &&
		pps_aend(rp) == OK &&
		pps_tinit(rp) == OK)
			return (OK);

	return (NOTOK);
}




/* ---------------------  Static  Routines  ------------------------------- */




static int pps_adrhdr (field, adr_array, count, rp)
char	    *field;
char	    *adr_array[];
int	    count;
RP_Buf	*rp;
{
	char	buffer[LINESIZE];
	int	i;
	
	if (state != HEADER) {
		pps_set (rp, RP_MECH, "Out of sync");
		return (pps_end (NOTOK, rp));
	}
	
	(void) sprintf (buffer, "%-10s", field);
	
	for (i = 0; i < count; i++) {
		(void) strcat (buffer, adr_array[i]);
		if (i != count - 1 ) {
			if ((int)strlen (buffer) > 64) {
				(void) strcat (buffer, ",\n\t");
				(void) io_tdata (buffer, strlen (buffer));
				buffer[0] = '\0';
			}
			else	(void) strcat (buffer, ", ");
		}
	}
	
	if (buffer[0])
		(void) io_tdata (buffer, strlen (buffer));
	
	if (rp_isbad (io_tdata ("\n", 1))) {
		pps_set (rp, RP_MECH, "Write error");
		return pps_end (NOTOK, rp);
	}
	
	return (OK);
}

static void pps_set (rp, val, str)
RP_Buf *rp;
int val;
char *str;
{
	rp -> rp_val = val;
	(void) strcpy (rp -> rp_line, str);
}
