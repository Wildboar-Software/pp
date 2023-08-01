/* rasn.c: incremental ASN1 reader */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/x400/RCS/rasn.c,v 6.0 1991/12/18 20:25:37 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/x400/RCS/rasn.c,v 6.0 1991/12/18 20:25:37 jpo Rel $
 *
 * $Log: rasn.c,v $
 * Revision 6.0  1991/12/18  20:25:37  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "q.h"
#include <isode/psap.h>
#include <isode/cmd_srch.h>

extern PE		ps2pe_aux ();

static struct qbuf *qbase;
static void add2qb ();
static int check_external ();
static int check_pserr (), qbproc ();
static int attempt_parse (), read_octet_hdr (), read_octet ();
static int proc_body (), proc_hdr (), proc_hdr_aux ();

IFP	asn_procfnx = proc_hdr;
static IFP  hdrfnx, bodyfnx;
static int	ext_type;
#define PENDING (-2)
static int bad = 0, oct_length;

void	asn_init (hf, bf, ext)
IFP	hf, bf;
int	ext;
{
	if (qbase)
		qb_free (qbase);
	qbase = NULL;
	oct_length = bad = 0;
	hdrfnx = hf;
	bodyfnx = bf;
	asn_procfnx = proc_hdr;
	ext_type = ext;
}


static int proc_hdr (qp)
struct qbuf *qp;
{
	char	*pestr;
	int result;
	int	len;

	add2qb (&qbase, qp);

	pestr = qb2str (qbase);
	len = qbase -> qb_len;
	result = attempt_parse (pestr, &len);
	free (pestr);
	switch (result) {
	    case PENDING:
		return OK;
	    case DONE:
	    case NOTOK:
		return result;
	    case OK:
		break;
	}
		
	PP_TRACE (("Header read, %d bytes", len));
	if ((result = qbproc (qbase, &len, NULLIFP)) != OK)
		return result;
	asn_procfnx = proc_body;
	return proc_body ((struct qbuf *)NULL);
}

static int ext_length = 0;

static int attempt_parse (str, len)
char	*str;
int	*len;
{
	PS 	ps;
	PE	pe;
	int	type;
	char	*ptr;
	int	len_left;
	int retval = OK;

	if ((ps = ps_alloc (str_open)) == NULLPS) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't allocate PS"));
		return NOTOK;
	}
	if (str_setup (ps, str, *len, 1) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't bind str to PS"));
		ps_free (ps);
		return NOTOK;
	}

	ext_length = 0;
	if (ext_type) {
		switch (ext_length = check_external (ps)) {
		    case PENDING:
			ps_free (ps);
			return PENDING;
		    case NOTOK:
			ps_free (ps);
			if (hdrfnx && (*hdrfnx) (NULLPE, NOTOK))
				return NOTOK;
			bad = 1;
			*len = 0;
			return OK;
		    default:
			break;
		}
	}

	ptr = ps -> ps_ptr;	/* save where we had got to */
	len_left = *len - (ptr - str);

	if ((pe = ps2pe_aux (ps, 1, 0)) == NULLPE) {
		ps_free (ps);
		return check_pserr (ps);
	}

	if (ps -> ps_cnt <= 0) {
		ps_free (ps);
		return PENDING;
	}
	switch (PE_ID(pe -> pe_class, pe -> pe_id)) {
	    case PE_ID (PE_CLASS_CONT, 0):
		type = MT_UMPDU;
		if (pe -> pe_len == PE_LEN_INDF)
			ext_length += 2;
		break;
	    case PE_ID (PE_CLASS_CONT, 1):
		type = MT_DMPDU;
		(void) str_setup (ps, ptr, len_left, 1);
		break;
	    case PE_ID (PE_CLASS_CONT, 2):
		type = MT_PMPDU;
		(void) str_setup (ps, ptr, len_left, 1);
		break;
	    default:
		pe_free (pe);
		ps_free (ps);
		if (hdrfnx && (*hdrfnx) (NULLPE, NOTOK) != OK)
			return NOTOK;
		bad = 1;
		*len = 0;
		return OK;
	}
	pe_free (pe);

	if ((pe = ps2pe(ps)) == NULLPE)
		return check_pserr (ps);

	if (hdrfnx && (*hdrfnx) (pe, type) != OK)
		retval = NOTOK;
	else 
		retval = type == MT_UMPDU ? OK : DONE;

	*len -= ps -> ps_cnt;
	ps_free (ps);
	pe_free (pe);
	return retval;
}

/* ARGSUSED */
static int proc_done (qb)
struct qbuf *qb;
{
	PP_TRACE(("proc_done()"));
	return DONE;
}

static int proc_body (qp)
struct qbuf *qp;
{
	int	i;
	static int count, used;
	
	if (qp)
		add2qb (&qbase, qp);
	if (bad) {
		i = qbase -> qb_len;
		return qbproc (qbase, &i, bodyfnx);
	}

	if (qbase -> qb_len && count > 0)
		if ((i = qbproc (qbase, &count, bodyfnx)) != OK)
			return i;

	for (;qbase -> qb_len > 0;) {
		char *pestr = qb2str (qbase);
		used = 0;
		i = read_octet_hdr (pestr, qbase -> qb_len,
				    &count, &used);
		free (pestr);
	
		switch (i) {
		    case DONE:
			(void) qbproc (qbase, &used, NULLIFP);
			if (qbase -> qb_len - ext_length > 0) {
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Trailing garbage (%d bytes) on end of PDU",
					qbase -> qb_len - used));
			}
			asn_procfnx = proc_done;
			return DONE;
		    case NOTOK:
			return NOTOK;
		    case PENDING:
			return OK;
		    case OK:
			(void) qbproc (qbase, &used, NULLIFP);
			if ((i = qbproc (qbase, &count, bodyfnx)) != OK)
				return i;
			if (qbase -> qb_len <= 0)
				return OK;
			break;
		}
	}
	return OK;
}


static int check_external (ps)
PS	ps;
{
	PE	pe;
	int extra = 0;
	char *psptr = ps -> ps_ptr;

	/* EXTERNAL Wrapper */
	if ((pe = ps2pe_aux (ps, 1, 0)) == NULLPE)
		return check_pserr (ps);

	if (PE_ID (pe -> pe_class, pe -> pe_id) !=
	    PE_ID (PE_CLASS_UNIV, PE_CONS_EXTN)) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Missing EXTERNAL wrapper; found %s/%d",
			 pe_classlist[pe -> pe_class], pe -> pe_id));
		pe_free (pe);
		return NOTOK;
	}
	if (pe -> pe_len == PE_LEN_INDF)
		extra += 2;
	pe_free (pe);

	if ((pe = ps2pe (ps)) == NULLPE)
		return check_pserr (ps);

	if (PE_ID (pe -> pe_class, pe -> pe_id) !=
	    PE_ID (PE_CLASS_UNIV, PE_PRIM_INT)) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Missing External ID, found %s/%d",
			 pe_classlist[pe -> pe_class], pe -> pe_id));
		pe_free (pe);
		return NOTOK;
	}
	pe_free (pe);

	/* ANY Context 0 wrapper */
	if ((pe = ps2pe_aux (ps, 1, 0)) == NULLPE)
		return check_pserr (ps);

	if (PE_ID (pe -> pe_class, pe -> pe_id) !=
	    PE_ID (PE_CLASS_CONT, 0)) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Not an EXTERNAL ANY type; found %s/%d",
			 pe_classlist[pe -> pe_class], pe -> pe_id));
		pe_free (pe);
		return NOTOK;
	}
	if (pe -> pe_len == PE_LEN_INDF)
		extra += 2;
	pe_free (pe);
	PP_TRACE (("%d bytes read, %d extra at end...",
		   ps -> ps_ptr - psptr, extra));
	/* OK EXTERNAL checks out. */
	return extra;
}

static int qbproc (q, len, fnx)
struct qbuf *q;
int	*len;
IFP	fnx;
{
	struct qbuf *qp;
	int	i;

	if (q == NULL || q -> qb_len <= 0)
		return OK;
	if (q -> qb_len != q ->qb_forw -> qb_len &&
	    qb_pullup (q) == NOTOK)
		adios (NULLCP, "qb_pullup failed");

	for (qp = NULL; *len > 0; ) {
		if (qp == NULL &&
		    (qp = q -> qb_forw) == q)
			break;
		i = min(qp -> qb_len, *len);
		if (fnx) {
			int result;
			result = (*fnx) (qp -> qb_data, i);
			if (result != OK)
				return result;
		}
		*len -= i;
		qp -> qb_data += i;
		qp -> qb_len -= i;
		q -> qb_len -= i;
		if (qp -> qb_len <= 0) {
			remque (qp);
			free ((char *)qp);
			qp = NULL;
		}
	}
	return OK;
}

static void add2qb (qpp, qp)
struct qbuf **qpp, *qp;
{
	struct qbuf *q;

	if (*qpp == NULL) {
		*qpp = (struct qbuf *) smalloc (sizeof *qp);
		bzero ((char *)*qpp, sizeof **qpp);
		(*qpp) -> qb_forw = (*qpp) -> qb_back = *qpp;
	}
	for (q = qp -> qb_forw; q != qp; q = qp -> qb_forw) {
		remque (q);
		insque (q, (*qpp) -> qb_back);
		(*qpp) -> qb_len += q -> qb_len;
		qp -> qb_len -= q -> qb_len;
	}
}

static int read_octet_hdr (str, len, countp, usedp)
char	*str;
int	len;
int	*countp;
int	*usedp;
{
	static PS ps;

	*usedp = 0;
	if (len < 1)
		return OK;
	if (ps == NULLPS)
		if ((ps = ps_alloc (str_open)) == NULLPS) {
			PP_LOG (LLOG_EXCEPTIONS, ("Can't setup PS"));
			return NOTOK;
		}
	if (str_setup (ps, str, len, 1) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't attach string to PS"));
		return NOTOK;
	}

	return read_octet (ps, countp, usedp);
}

#define MAXSTACK 100
static int stackidx = -1;
static struct stack {
#define STACK_EOC 1
#define STACK_LEN 2
	int 	type;
	int	len;
} mystack[MAXSTACK];

static int read_octet (ps, countp, usedp)
PS	ps;
int	*countp;
int	*usedp;
{
	PE	pe;
	char	*psptr;

	PP_TRACE (("read_octet() stackidx = %d u=%d c=%d ol=%d", stackidx,
		   *usedp, *countp, oct_length));
	if (stackidx >= 0 && mystack[stackidx].type == STACK_LEN &&
	    mystack[stackidx].len <= oct_length) {
		stackidx --;
		PP_TRACE (("read_octet length done, poping stack"));
		if (stackidx < 0)
			return DONE;
		else
			return read_octet (ps, countp, usedp);
	}

	psptr = ps -> ps_ptr;
	if ((pe = ps2pe_aux (ps, 1, 0)) == NULLPE)
		return check_pserr (ps);

	if (pe -> pe_class == PE_CLASS_UNIV &&
	    pe -> pe_id == PE_UNIV_EOC) {
		pe_free (pe);
		PP_TRACE (("stackidx %d, type=%d", stackidx,
			   mystack[stackidx].type));
		if (stackidx < 0 || mystack[stackidx].type != STACK_EOC) {
			PP_LOG (LLOG_EXCEPTIONS, ("Unexpected EOC string"));
			return NOTOK;
		}
		PP_TRACE (("read_octet EOC found, poping stack"));
		*usedp += ps -> ps_ptr - psptr;
		oct_length += ps -> ps_ptr - psptr;
		stackidx --;
		if (stackidx == -1) {
			PP_TRACE (("read_octet used %d", *usedp));
			return DONE;
		}
		else
			return  read_octet (ps, countp, usedp);
	}
	
	if (pe -> pe_class != PE_CLASS_UNIV ||
	    pe -> pe_id != PE_PRIM_OCTS) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Not an OCTET STRING expecting %s/%d got %s/%d",
			 pe_classlist[PE_CLASS_UNIV], PE_PRIM_OCTS,
			 pe_classlist[pe -> pe_class], pe -> pe_id));
		return NOTOK;
	}
	if (pe -> pe_form != PE_FORM_PRIM) {
		*usedp += ps -> ps_ptr - psptr;
		oct_length += ps -> ps_ptr - psptr;
		if (stackidx >= MAXSTACK - 1) {
			PP_LOG (LLOG_EXCEPTIONS, ("Internal stack overflow"));
			return NOTOK;
		}
		stackidx ++;
		if (pe -> pe_len == PE_LEN_INDF) {
			PP_TRACE (("read_octet cons INDF found, pushing..."));
			mystack[stackidx].type = STACK_EOC;
		}
		else {
			PP_TRACE (("read_octet cons len %d found pushing",
				   pe -> pe_len));
			mystack[stackidx].type = STACK_LEN;
			mystack[stackidx].len = oct_length + pe -> pe_len;
		}
		return read_octet (ps, countp, usedp);
	}
	stackidx ++;
	mystack[stackidx].type = STACK_LEN;
	mystack[stackidx].len = oct_length + pe -> pe_len;
	*countp = pe -> pe_len;
	oct_length += pe -> pe_len;
	*usedp += ps -> ps_ptr - psptr;
	PP_TRACE (("read_octet used %d count %d ol=%d", *usedp, *countp,
		   oct_length));
	pe_free (pe);
	return OK;
}

static int check_pserr (ps)
PS	ps;
{
	int	retval = PENDING;

	switch (ps -> ps_errno) {
	    case PS_ERR_EOF:
	    case PS_ERR_EOFLEN:
	    case PS_ERR_EOFID:
	    case PS_ERR_NONE:
		break;

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("ps2pe error: %s",
					  ps_error (ps -> ps_errno)));
		retval = NOTOK;
		break;
	}
	return retval;
}
