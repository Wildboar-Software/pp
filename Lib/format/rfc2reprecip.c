/* rfc2reprecip.c - Converts a RFC string into a Rrseq struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2reprecip.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/rfc2reprecip.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: rfc2reprecip.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <isode/cmd_srch.h>
#include "dr.h"



extern CMD_TABLE        drtbl_rep[];


extern void encodedinfo_free ();

static char             *rfc2lastrace(),
			*rfc2report();

#define	txt2int(n)	atoi(n)


/* ---------------------  Begin  Routines  -------------------------------- */




int rfc2reprecip (rp, ap, str)
Rrinfo          *rp;
ADDR		*ap;
char            *str;
{
	char    *cp = str,
		*bp = str;


	PP_DBG (("Lib/pp/rfc2reprecip(%s)", str));


	/* -- OR address -- */
	if ((cp = index (str, ';')) == NULLCP)
		return NOTOK;
	else {
		*cp++ = '\0';
		rp -> rr_recip = ap -> ad_no;
	}


	/* -- last-trace -- */
	if (*cp == ' ')
		cp++;
	bp = cp;
	cp = rfc2lastrace (rp, bp);
	if (cp == NULLCP)
		return NOTOK;
	if (*++cp != ';')
		return NOTOK;
	else
		cp++;


	/* -- ext -- */
	if (*cp == ' ')
		cp++;
	bp = cp;
	if ((cp = index (bp, ' ')) == NULLCP)
		return NOTOK;
	else {
		*cp++ = '\0';
		if (lexequ (bp, "ext") != 0)
			return NOTOK;
		bp = cp;
		if ((cp = index (bp, ' ')) == NULLCP)
			return NOTOK;
		else {
			*cp++ = '\0';
			rp->rr_recip = txt2int (bp);
		}
	}


	/* -- flags -- */
	bp = cp;
	if ((cp = index (bp, ' ')) == NULLCP)
		return NOTOK;
	else {
		*cp++ = '\0';
		if (lexequ (bp, "flags") != 0)
			return NOTOK;
		bp = cp;
		if ((cp = index (bp, ' ')) != NULLCP)
			*cp++ = '\0';
	}


	/* -- the rest is optional -- */
	if (cp == NULLCP)
		return OK;


	/* -- intended -- */
	bp = cp;
	if ((cp = index (bp, ' ')) == NULLCP)
		return NOTOK;
	else
		*cp++ = '\0';

	if (lexequ (bp, "intended") == 0) {
		bp = cp;
		if ((cp = index (bp, ' ')) != NULLCP)
			*cp++ = '\0';
		if (rp -> rr_originally_intended_recip == NULL) {
			rp -> rr_originally_intended_recip =
				(FullName *) smalloc (sizeof (FullName));
			bzero ((char *)rp -> rr_originally_intended_recip,
			       sizeof (FullName));
		}
		rp->rr_originally_intended_recip -> fn_addr = strdup (bp);
		if (cp == NULLCP)
			return OK;
		bp = cp;
		if ((cp = index (bp, ' ')) == NULLCP)
			return NOTOK;
		else
			*cp++ = '\0';
	}


	/* -- info -- */
	if (lexequ (bp, "info") == 0) {
		bp = cp;
		rp->rr_supplementary = strdup (bp);
	}
	else
		return NOTOK;


	return OK;
}




/* ---------------------  Static  Routines  ------------------------------- */




static char *rfc2lastrace (rp, str)
Rrinfo		*rp;
char            *str;
{
	EncodedIT       *new;
	char            *cp = str,
			*bp = str;


	PP_DBG (("Lib/rfc2lastrace (%s)", str));


	/* -- drc-report -- */
	cp = rfc2report (&rp -> rr_report, str);
	if (cp == NULLCP)
		return NULLCP;
	if (*++cp != ';')
		return NULLCP;
	else
		cp++;



	/* -- date-time -- */
	if (*cp == ' ')
		cp++;
	bp = cp;
	if ((cp = index (bp, ';')) == NULLCP)
		return NULLCP;
	else {
		*cp++ = '\0';
		if (rfc2UTC (bp, &rp->rr_arrival) == NOTOK)
			rp -> rr_arrival = utcnow ();
	}


	/* -- converted (optional) -- */
	bp = cp;
	if (*bp == ' ')
		bp++;

	if ((cp = index (bp, '(')) == NULLCP)
		return --bp;
	else {
		*cp = '\0';
		if (lexequ (bp, "converted") != 0) {
			*cp = '(';
			return --bp;
		}
		else
			bp = ++cp;
	}
	if ((cp = index (bp, ')')) == NULLCP)
		return NULLCP;
	else
		*cp = '\0';

	/* -- create a new Encoded struct -- */
	new = (EncodedIT *) smalloc (sizeof (*new));
	bzero ((char*)new, sizeof (*new));
	if (rfc2encinfo (new, bp) == NOTOK) {
		encodedinfo_free (new);
		return NULLCP;
	}
	else
		rp->rr_converted = new;

	return cp;
}




static char *rfc2report (rp, str)
Report   *rp;
char    *str;
{
	char    *cp = str,
		*bp = str;

	int     retval;


	PP_DBG (("Lib/rfc2report (%s)", str));


	if ((cp = index (str, ' ')) == NULLCP)
		return NULLCP;
	else
		*cp++ = '\0';

	switch (rp->rep_type = cmd_srch (bp, drtbl_rep)) {
	case DR_REP_SUCCESS:
		bp = cp;
		if ((cp = index (bp, ';')) == NULLCP)
			return NULLCP;
		else {
			*cp++ = '\0';
			retval = rfc2UTC (bp, &rp->rep.rep_dinfo.del_time);
			if (retval == NOTOK)
				return NULLCP;
			if (*cp == ' ')
				cp++;
			bp = cp;
			if ((cp = index (bp, ';')) == NULLCP)
				return NULLCP;
			else {
				*cp = '\0';
				rp->rep.rep_dinfo.del_type = txt2int (bp);
				*cp = ';';
				--cp;
			}
		}
		break;


	case DR_REP_FAILURE:
		/* -- reason code -- */
		bp = cp;
		if ((cp = index (bp, '(')) == NULLCP)
			return NULLCP;
		else {
			*cp++ = '\0';
			rp->rep.rep_ndinfo.nd_rcode = atoi (bp);
		}

		/* -- diagnostic code -- */
		bp = cp;
		if ((cp = index (bp, ';')) == NULLCP)
			return NULLCP;
		else {
			bp = ++cp;

			/* -- no diagnostic given -- */
			if ((cp = index (bp, '(')) == NULLCP)
				return --bp;
			else
				*cp++ = '\0';

			bp = cp;

			rp->rep.rep_ndinfo.nd_dcode = atoi (bp);

			if ((cp = index (bp, ';')) == NULLCP)
				return NULLCP;
		}
		break;
	default:
		return NULLCP;
	}


	PP_DBG (("Lib/rfc2report returns ... (%s)", cp));

	return cp;
}
