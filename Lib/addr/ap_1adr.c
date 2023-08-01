/* ap_1adr.c: parse one address - the heart of the parser */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_1adr.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_1adr.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_1adr.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



/*  ADDRESS PARSER, as per:

    "Standard for the Format of ARPA Network Text Messages", D.  Crocker,
    J. Vittal, K. Pogran & D. Henderson, in ARPANET PROTOCOL HANDBOOK, E.
    Feinler & J. Postel (eds), NIC-7104-REV-1, Network Information Center,
    SRI International:  Menlo Park, Ca.  (1978) (NTIS AD-A0038901).

    and

    "Standard for the Format of Arpa Internet Text Messages", Revised
    by D. Crocker, RFC #822, in INTERNET PROTOCOL TRANSITION WORKBOOK,
    Feinler & J. Postel (eds), Network Information Center, SRI
    International:  Menlo Park, Ca.  (March 1982).

    A parsed address is normalized to have routing references placed
    into rfc822-type positioning.

    History:

    Fall 1977     Bruce Borden:  Initial Design & Coding
    Summer 1979   Dave Crocker:  Completed it & fixed bugs
				 Major reorganization & re-labeling
				 Minor changes to semantics
				 Documentation
    Sept 81       Andy Knutsen   case STPREND -> STPHRSE, to allow comments
				 afterwards
    Sept 81       Dave Crocker   generalized the use of STPHRSE, so that
				 STOKEND occurs only on comma or eof.
				 changed again, to cycle only accepting
				 comments
    Nov 82        Dave Crocker   Converted to accept 822 syntax, while
				 trying also to juggle 733...
    Aug 83        Steve Kille    Fix to STEPER

    This module is the real parser, tho it is intended to be co-routined
    with a caller who has something specific to do with ap_1adr's output.
    This is organized so as to make some attempt at limiting core
    consumption.  Fullparse(), however, simply causes a full parse tree to
    be built up in core.

    The implementation deviates somewhat from the above specification.
    Deviations and the use of the package are discussed in the companion
    documentation.

    The parser's behavior is fairly straightforward.  A singly-linked flat
    list of labelled (lexical) nodes is built up.  Ap_1adr() is used to get
    the next "address segment", which consists of all of the lexical nodes
    up to the end of the next host-phrase (i.e., usually that means up to
    the next comma, semi-colon, or right-angle bracket).

    The caller is responsible for initializing state variables, via
    ap_init(), and linking together or using ap_1adr's output segments.

    Note that ap_1adr does NOT interpret address text to cause re-directed
    file input.  The caller must do that.  Ap_pshfil() and ap_popfil() can
    be used to save and restore the parse state and acquire the named file.
    The provision for this stacking, given the co-routining, is the reason
    state information is chained through global variables, rather than
    being saved on local (stack) variables.

    The amount of input processed on a single call may seem strange.  The
    idea is to try to guess the most common use of the routine.  This is
    presumed to be for address checking, in which acquisition of the MBOX
    and DOMAIN text are most important, with the rest actually being thrown
    away.  It is, of course, possible for the core-limiting heuristic to
    lose if a ridiculous number of groups and personal lists are specified
    in a particular way.  I am assuming that won't happen.
*/



#include "util.h"
#include "ap.h"
#include "ap_lex.h"



#define STDOMAIN  0
#define STDTYPE   1
#define STEBAD    2
#define STECMNT   3
#define STEDOMAIN 4
#define STEDONE   5     /* Returned when addresses were NOT found */
#define STEDTYPE  6
#define STEGRP    7
#define STEOK     8     /* Returned when addresses were found */
#define STEPER    9
#define STINIT    10
#define STITER    11
#define STPHRSE   12
#define STSTPER   13


int  ap_intype  = AP_PARSE_733;         /* default to RFC #733 input */
int  ap_outtype = AP_PARSE_822;         /* default to RFC #822 output */
					/* with little endian domains */

int ap_grplev   = 0;                    /* Group nesting depth */
int ap_perlev   = 0;                    /* <> nesting depth */
int ap_routing;                         /* parsing a route */



#ifdef AP_DEBUG

extern AP_ptr           ap_sqinsert ();
extern char             *strdup();
char                    ap_debug=TRUE;     /* True if in debug mode */


char   *statnam[] = {
	"Domain",       "DTypNam",      "BadEnd",       "CmntEnd",
	"DomainE",      "DoneEnd",      "DTypeE",       "GrpEnd",
	"OKEnd",        "PersEnd",      "Init",         "Iterate",
	"Phrase",       "Persstrt",
};


char   *typtab[] =
{
	"Nil",          "Comment",      "DataType",     "Domain",
	"DomainLit",    "Word",         "GpEnd",        "GpName",
	"GpStart",      "Mailbox",      "PersonEnd",    "PersonName",
	"PersonStart"
};
#endif
/**/



/* ---------------------  Begin  Routines  -------------------------------- */


int ap_1adr ()
{
    struct ap_node      base_node;
    AP_ptr              ap_sp = NULLAP,          /* Saved ap node ptr */
    			ap_noname,
			r822_ptr = NULLAP,
			r733_prefptr;
    int                 got_822,
    			noname;
    char                buf[BUFSIZ];
    register int        state;


    ap_routing = DONE;
    ap_noname = NULL;
    ap_ninit (&base_node);

    (void) ap_sqinsert (&base_node, AP_PTR_MORE, ap_pstrt);

    for (state = STINIT, got_822 = FALSE, noname = FALSE, r733_prefptr = NULLAP; ; ) {


#ifdef AP_DEBUG
	if (ap_debug)
		PP_DBG (("=>%d (%s)", state,
				(state >= 0 && state <= 13) ?
					       statnam[state] : "BOGUS!"));
#endif


	switch (state) {
	case STITER:
		/* -- Iteration to get real address -- */
		ap_palloc ();
		state = STINIT;
		/* just drop on through -- */

	case STINIT:
		/* -- start of parse; empty node -- */
		ap_sp = ap_pcur;
		switch (ap_lex (buf, sizeof(buf))) {
		case LV_WORD:
			ap_pfill (AP_GENERIC_WORD, buf);
			if (noname == TRUE) {
				/* no name before so copy buf into ap_noname */
				ap_noname->ap_obvalue =
					(buf == NULLCP) ? NULLCP : strdup(buf);
				ap_noname = NULL;
				noname = FALSE;
			}
			state = STPHRSE;
			break;

		case LV_AT:
			if (!got_822) {
			    got_822 = TRUE;
			    r822_ptr = ap_pcur;
			}
			ap_routing = OK;
#ifdef AP_DEBUG
			if (ap_debug)
				PP_DBG (("(routing)"));
#endif
			state = STDOMAIN;
			break;

		case LV_COLON:
			/* -- data type start -- */
			state = STDTYPE;
			break;

		case LV_SEMI:
			/* -- group list end -- */
			state = STEGRP;
			break;

		case LV_GRTR:
			/* -- personal list end -- */
			state = STEPER;
			break;

		case LV_LESS:
			/* -- allow one angle-bracket, here -- */
			ap_pfill (AP_GENERIC_WORD, "");
			noname = TRUE;
			ap_noname = ap_pcur;
			state = STSTPER;
			break;

		case LV_FROM:
			/* -- file source -- */
			ap_pfill (AP_DATA_TYPE, "Include");
			state = STITER;
			break;

		case LV_COMMENT:
			ap_pfill (AP_COMMENT, buf);
			state = STITER;
			break;

		case LV_COMMA:
			/* -- ignore null addresses -- */
			break;

		case LV_EOD:
			if (ap_perlev != 0 || ap_grplev != 0)
				state = STEBAD;
			else
				state = STEDONE;
			break;

		default:
			state = STEBAD;
			break;
		}

		continue;


/* ---------------------------  Ending  --------------------------------- */


	case STECMNT:
		/* -- accept comments until end -- */
		switch (ap_lex (buf, sizeof(buf))) {
		case LV_COMMENT:
			/* -- just cycle, accepting comments -- */
/*			ap_pfill (AP_COMMENT, buf);*/
			ap_pappend(AP_COMMENT, buf);
			break;

		case LV_COMMA:
			state = STEOK;
			break;

		case LV_SEMI:
			/* -- group list end -- */
			state = STEGRP;
			break;

		case LV_EOD:
			state = STEOK;
			break;

		default:
			state = STEBAD;
			break;
		}

		continue;



	case STEDONE:
		/* -- end clean; no empty nodes? -- */
		ap_7to8 (r733_prefptr, r822_ptr);
		return (DONE);



	case STEOK:
		/* -- end clean -- */
		ap_7to8 (r733_prefptr, r822_ptr);
		return (OK);



	case STEBAD:
		/* -- end error -- */
		ap_clear();             /* Experimental, DPK, 7 Aug 84 */
		return (NOTOK);




/* -----------------------  Gather a phrase  ------------------------------ */


	case STPHRSE:
		/* -- phrase continuation; empty node -- */
		switch (ap_lex (buf, sizeof(buf))) {
		case LV_WORD:
			/* -- append word to phrase, maybe -- */
			ap_pappend (AP_GENERIC_WORD, buf);
			break;

		case LV_AT:
			/* -- mailbox (name) end -- */
			if (!got_822) {
				r822_ptr = ap_sp;
				got_822 = TRUE;
			}
			ap_sqtfix (ap_sp, ap_pcur, AP_MAILBOX);
			ap_palloc ();
			state = STDOMAIN;
			break;

		case LV_LESS:
			/* -- person name end -- */
			state = STSTPER;
			break;

		case LV_COLON:
			/* -- group name end -- */
			 if (ap_grplev++ >= 1 && ap_intype == AP_PARSE_822) {
				/* -- may not be nested -- */

#ifdef AP_DEBUG
				if (ap_debug)
					PP_DBG (("(intype=%d,ap_grplev=%d)",
						ap_intype,
						ap_grplev));
#endif

				state = STEBAD;
				break;
			 }
			ap_sqtfix (ap_sp, ap_pcur, AP_GROUP_START);
			ap_sp -> ap_obtype = AP_GROUP_NAME;
			state = STITER;
			break;

		case LV_SEMI:
			ap_sqtfix (ap_sp, ap_pcur, AP_MAILBOX);
			ap_sp -> ap_obtype = AP_MAILBOX;
			state = STEGRP;
			break;

		case LV_GRTR:
			state = STEPER;
			break;

		case LV_COMMA:
			ap_sqtfix (ap_sp, ap_pcur, AP_MAILBOX);
			state = STEOK;
			break;
		case LV_EOD:
			ap_sqtfix (ap_sp, ap_pcur, AP_MAILBOX);
			state = STEOK;
			break;
		case LV_COMMENT:
			ap_pappend (AP_COMMENT, buf);
			break;
		default:
			state = STEBAD;
			break;
		}

		continue;


/* -------------------------  Address lists  ---------------------------- */


	case STSTPER:
		/* -- personal address list; no empty node -- */
		if (ap_perlev++ > 0 && ap_intype == AP_PARSE_822) {
			/* -- may not be nested -- */
#ifdef AP_DEBUG
			if (ap_debug)
				PP_DBG (("(intype=%d,ap_perlev=%d)",
					ap_intype,
					ap_perlev));
#endif
			state = STEBAD;
			break;
		}
		ap_routing = OK;
		ap_sqtfix (ap_sp, ap_pcur, AP_PERSON_START);
		ap_sp -> ap_obtype = AP_PERSON_NAME;
		state = STITER;
		continue;


	case STEPER:
		if (--ap_perlev < 0) {

#ifdef AP_DEBUG
			if (ap_debug)
				PP_DBG (("(ap_perlev=%d)", ap_perlev));
#endif

			state = STEBAD;
			break;
		}
		ap_pappend (AP_PERSON_END, NULLCP);
		ap_palloc ();   /* SEK add storage */
		state = STECMNT; /* allow comments, etc */
		continue;


	case STEGRP:
		if (--ap_grplev < 0) {

#ifdef AP_DEBUG
			if (ap_debug)
				PP_DBG (("(ap_grplev=%d)", ap_grplev));
#endif

			state = STEBAD;
			break;
		}
		ap_pappend (AP_GROUP_END, NULLCP);
		state = STECMNT;
		continue;


/* --------------------------  Data type ---------------------------------- */


	case STDTYPE:
		/* -- data type name; empty node -- */
		if (ap_intype == AP_PARSE_822) {
			/* -- data types not legal in 822 -- */

#ifdef AP_DEBUG
			if (ap_debug)
				PP_DBG (("(intype=%d)", ap_intype));
#endif

			state = STEBAD;
			break;
		}

		if (ap_lex (buf, sizeof(buf)) != LV_WORD) {
			state = STEBAD;
			continue;
		}
		ap_pfill (AP_DATA_TYPE, buf);
		state = STEDTYPE;
		/* -- Just drop on through -- */


	case STEDTYPE:
		/* -- data type name end; empty node -- */
		state = (ap_lex (buf, sizeof(buf)) == LV_COLON) ? STITER : STEBAD;
		continue;



/* ---------------------------  domain  ----------------------------------- */


	case STDOMAIN:
		/* -- domain/host; no empty parse node -- */
		switch (ap_lex (buf, sizeof(buf))) {
		default:
			state = STEBAD;
			continue;
		case LV_COMMENT:
			ap_pappend (AP_COMMENT, buf);
			continue;
		case LV_DLIT:
			ap_pappend (AP_DOMAIN_LITERAL, buf);
			state = STEDOMAIN;
			continue;
		case LV_WORD:
			ap_pfill (AP_DOMAIN, buf);
			state = STEDOMAIN;
		}

		/* -- just drop on through -- */


	case STEDOMAIN:
		/* -- domain end; no empty parse node -- */
		switch (ap_lex (buf, sizeof(buf))) {
		case LV_AT:
			/* -- sequence of HOST's => @ separation -- */
			if (r733_prefptr == NULLAP)
				r733_prefptr = ap_pcur;
			ap_palloc ();
			/* -- node which points to first routing ref -- */
			state = STDOMAIN;
			break;

		case LV_SEMI:
			state = STEGRP;
			break;
		case LV_GRTR:
			state = STEPER;
			break;
		case LV_COMMA:
			if (ap_routing != DONE)
			    state = STITER;
			else
			    state = STEOK;
			break;
		case LV_EOD:
			if (ap_routing != DONE)
			    state = STEBAD;
			else
			    state = STEOK;
			break;
		case LV_COMMENT:
			ap_pappend (AP_COMMENT, buf);
			break;
		case LV_COLON:
			if (ap_routing != DONE) {
			    ap_routing = DONE;
			    state = STITER;
			    continue;
			}
			/* else drop on through */
		default:
			state = STEBAD;
			break;
		}

		continue;
	}
    }
}



/* ------------------------------------------------------------------------ */



void ap_7to8 (r733_prefptr, r822_ptr)
AP_ptr                  r733_prefptr,
			r822_ptr;
{
	AP_ptr          routbase;
	AP_ptr          ap;
	char            *perneeded;


	PP_DBG (("Lib/addr/ap_7to8()"));

	/* -- don't have to move it to 822 style -- */
	if (r733_prefptr == NULLAP)   return;

	if (ap_pstrt -> ap_obtype == AP_MAILBOX) {
		perneeded = strdup (r822_ptr -> ap_obvalue);
		ap_pappend (AP_PERSON_END, NULLCP);
	}
	else
		perneeded = NULLCP;

	/* -- move the sequence to newroute -- */
	routbase = ap_alloc ();

	while (r733_prefptr -> ap_ptrtype != AP_PTR_NIL &&
	       r733_prefptr -> ap_ptrtype != AP_PTR_NXT &&
	       r733_prefptr -> ap_next != NULLAP) {

		switch (r733_prefptr -> ap_next -> ap_obtype) {
		default:
			/* -- only move domain info -- */
			goto endit;

		case AP_COMMENT:
			if (routbase -> ap_next != NULLAP) {
				/* -- try to append, not prepend, cmnts -- */
				PP_DBG (("comment appended'%s'",
				       r733_prefptr->ap_next->ap_obvalue));
				(void) ap_move (routbase -> ap_next,
						r733_prefptr);
				continue;
			}
			/* else drop on through */

		case AP_DOMAIN_LITERAL:
		case AP_DOMAIN:
			PP_DBG (("val='%s'",
				r733_prefptr -> ap_next -> ap_obvalue));
			(void) ap_move (routbase, r733_prefptr);
			continue;
		}
	}


endit:
	/*
	treatment here depends on whether we have an 822 route already.
	Note that 822 pointer is NOT easy,  as an easy pointer was
	too hard!
	There is a need to copy first part of route to r822_ptr
	*/

	if (r822_ptr -> ap_obtype != AP_DOMAIN &&
	    r822_ptr -> ap_obtype != AP_DOMAIN_LITERAL) {

		ap_insert (r822_ptr, AP_PTR_MORE,
			   ap_new (r822_ptr -> ap_obtype,
			   r822_ptr -> ap_obvalue));

		free (r822_ptr -> ap_obvalue);

		ap_fllnode (r822_ptr, routbase -> ap_next -> ap_obtype,
			    routbase -> ap_next -> ap_obvalue);

		(void) ap_sqinsert (r822_ptr, AP_PTR_MORE,
				    routbase -> ap_next -> ap_next);

		/* -- add it before local-part -- */
		ap_free (routbase -> ap_next);
		ap_free (routbase);
	}
	else {
		ap = r822_ptr;
		while (ap -> ap_next -> ap_obtype == AP_DOMAIN ||
		       ap -> ap_next -> ap_obtype == AP_DOMAIN_LITERAL)
				ap = ap -> ap_next;
		(void) ap_sqinsert (ap, AP_PTR_MORE, routbase -> ap_next);
	}


	/* -- need to kludge person name -- */
	if (perneeded != 0) {
		ap_insert (r822_ptr, AP_PTR_MORE,
			   ap_new (r822_ptr -> ap_obtype,
			   r822_ptr -> ap_obvalue));
		free (r822_ptr -> ap_obvalue);
		ap_fllnode (r822_ptr, AP_PERSON_NAME, perneeded);
		free (perneeded);
	}
}
