/* ut_f.c: Common Function Routines (used by f_*.c files) */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/RCS/ut_f.c,v 6.0 1991/12/18 20:16:07 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/asn/RCS/ut_f.c,v 6.0 1991/12/18 20:16:07 jpo Rel $
 *
 * $Log: ut_f.c,v $
 * Revision 6.0  1991/12/18  20:16:07  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "asn.h"


extern int	CRline;


typedef struct decoded_struct {
	struct qbuf		*line;
	struct decoded_struct	*next;
} DECODED;


#define NULLDECODED	((DECODED *)0)
#define STR2QB(s)	str2qb(s, strlen(s), 1)




/* ------------------------  Start Routines --------------------------------- */




/* --- *** ---
writes an asn1 decoded body part info into a common struct called ASNBODY
--- *** --- */



struct2body (dstruct, body)
DECODED		*dstruct;
ASNBODY		**body;
{
	DECODED	*dp;
	char	*cp, *np;

	PP_TRACE (("struct2body()"));

	for (dp = dstruct; dp; dp = dp -> next) {
		if ((cp = qb2str (dp -> line)) == NULLCP) {
			PP_LOG (LLOG_EXCEPTIONS,
				("Unable to decode - qb2str error"));
			exit (1);
		}

		rn2n (cp, strlen(cp), &np);
		free (cp);
		asnbody_add (body, np, strlen(np));
	}
}




body2struct (asnbody, dstruct)
ASNBODY		*asnbody;
DECODED		**dstruct;
{
	ASNBODY	*body;
	DECODED	*new, *bak;
	char	*np;

	PP_TRACE (("body2struct()"));


	for (bak=NULLDECODED, body=asnbody; body; body=body->next) {

		n2rn (body->line, body->length, &np); 

		new = (DECODED *) smalloc (sizeof *new);
		bzero (new, sizeof (*new));

		new -> line = str2qb(NULLCP, 0, 1);
		new -> line -> qb_forw -> qb_data = np;
		new -> line -> qb_forw -> qb_len  = strlen(np);


		/* -- free body lines now to reduce space -- */
		if (body->line) free (body->line);
		body -> line = NULLCP;
		body -> length = 0;

		if (bak)
			bak -> next = new;
		else
			*dstruct = new;
		bak = new; 
	}

	asnbody_free (asnbody);
}




qbuf2body (qstruct, body)
struct qbuf	*qstruct;
ASNBODY		**body;
{
	struct qbuf	*qp;
	char		*np;

	PP_TRACE (("qbuf2body()"));
	
	for (qp = qstruct -> qb_forw; qp != qstruct; qp = qp -> qb_forw) {

		if (qp -> qb_len == 0)
			asnbody_add (body, strdup(""), 0);
		else {
			rn2n (qp -> qb_data, qp -> qb_len, &np);
			asnbody_add (body, np, strlen(np));
		}
	}
}




body2qbuf (asnbody, qb)
ASNBODY		*asnbody;
struct qbuf	**qb;
{
	ASNBODY		*body;
	struct qbuf	*new, *bak;
	char		*np;

	PP_TRACE (("body2qbuf()"));

	for (bak=NULL, body=asnbody; body; body=body->next) {

		n2rn (body->line, body->length, &np);

		if (bak == NULL) {
			new = str2qb(NULLCP, 0, 1);
			new -> qb_forw -> qb_data = np;
			new -> qb_forw -> qb_len  = strlen (np);
			*qb = new;
			bak = new -> qb_forw;
		}
		else {
			new = str2qb(NULLCP, 0, 0);
			new -> qb_data = np;
			new -> qb_len  = strlen (np);
			bak -> qb_forw = new;
			new -> qb_back = bak;
			bak = new; 
		}	

		if (body -> line)  free (body -> line);
		body -> line = NULLCP;
		body -> length = NULL;
	}

	bak -> qb_forw = *qb;

	asnbody_free (asnbody);
}




pe_done(pe, msg)	/* -- writes to oper log -- */
PE	pe;
char	*msg;
{

	PP_TRACE (("pe_done(%x)", pe));

	if (pe->pe_errno)
		PP_OPER(NULLCP,
		     ("%s: [%s] %s", msg, PY_pepy, pe_error(pe->pe_errno)));
	else
		PP_LOG(LLOG_EXCEPTIONS,
		     ("pe_done failure: [%s]", PY_pepy));

	exit (1);
}




/* -----------------------  Static  Routines  ------------------------------- */




static rn2n (oldp, len, newpp)  /* convert from \r\n to \n */
char	*oldp;
int	len;
char	**newpp;
{
	char	*op, *buf, *start;
	char	lastr = NULL, lastn = TRUE;
	int	n;


	PP_TRACE (("rn2n(%d)", len));

	n = len * 2;
	start = buf = smalloc (n);
	bzero (buf, n);

	for (op=oldp, n=len; n > 0; n--, op++) {
		switch (*op) {
		case '\r':
			lastr = *op;
			break;
		case '\n':
			*buf++ = *op;
			lastr = lastn = NULL;
			break;
		default:
			if (lastr) {
				*buf++ = lastr;
				lastr = NULL;
			}

			*buf++ = *op;
			break;
		}
	}		

	if (lastn && CRline)
		*buf++ = '\n';

	*buf++ = '\0';
	*newpp = realloc (start, buf - start);
}




static n2rn (oldp, len, newpp)	/* convert from \n -> \r\n */
char	*oldp;
int	len;
char	**newpp;
{
	char	*op, *buf, *start;
	char	lastr = NULL, lastn = TRUE;
	int	n;

	PP_TRACE (("n2rn(%d)", len));


	n = len * 2;
	start = buf = smalloc (n);
	bzero (buf, n);


	for (op=oldp, n=len; n > 0; n--, op++) {
		switch (*op) {
		case '\n':
			*buf++ = '\r';
			*buf++ = '\n';
			lastn = 0;
			break;
		default:
			*buf++ = *op;
			break;
		}
	}

	if (lastn && CRline) {
		*buf++ = '\r';
		*buf++ = '\n';
	}

	*buf++ = '\0';
	*newpp = realloc (start, buf - start);
}
