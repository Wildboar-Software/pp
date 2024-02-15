/* ap_lex.c: lexical analyser for address parser */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_lex.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_lex.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_lex.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



/*
Perform lexical analysis on input stream

The stream is assumed to be "unfolded" and the <crlf>/<lwsp> sequence
is NOT checked for.  This must be done by the character-acquisition
routine, if necessary.  In fact, space, tab and newline all return the
same lexical token.  Due to a number of bagbiting mail systems on the
net which cannot handle having a space within a mailbox name, period
(.) has been equated with space.

Letters, numbers, and other graphics, except specials, also all return
the same token.

Note that only printable characters and format effectors are legal.
All others cause an error return.

Only COMMENTs and WORDs have data associated with them.

*/

/*
< 1978  B. Borden       Wrote initial version of parser code
78-80   D. Crocker      Reworked parser into current form
Apr 81  K. Harrenstein  Hacked for SRI
Jun 81  D. Crocker      Back in the fold.  Finished v7 conversion
 */


#include "util.h"
#include "ap.h"
#include "ap_lex.h"


extern char     ap_lxtable[],
		ap_lxtable_per[];   /* ascii chars -> symbolic terminals */
extern int      ap_intype;
extern int      ap_peek = -1;   /* one-character look-ahead */
char            ap_llex;        /* last lexeme returned by ap_lex() */

#ifdef AP_DEBUG
extern char     ap_debug;
char   *namtab[] =
{
    "eo-data",                  /* LV_EOD       0    */
    "error",                    /* LV_ERROR     1    */
    "comma",                    /* LV_COMMA     2    */
    "at",                       /* LV_AT        3    */
    "colon",                    /* LV_COLON     4    */
    "semi",                     /* LV_SEMI      5    */
    "comment",                  /* LV_COMMENT   6    */
    "less",                     /* LV_LESS      7    */
    "grtr",                     /* LV_GRTR      8    */
    "word",                     /* LV_WORD      9    */
    "from",                     /* LV_FROM      10   */
    "domain-literal"            /* LV_DLIT      11   */
};
#endif



/* ---------------------  Begin  Routines  -------------------------------- */

static int ap_char ();
int ap_lex_percent = FALSE;

void ap_use_percent()
{
	ap_lex_percent = TRUE;
}

int ap_lex (lexval, siz)
char    lexval[];
int	siz;
{
	register char   c,
			*lexptr;
	register int    retval;
	char	*lex_table = (ap_lex_percent == FALSE) ? ap_lxtable : ap_lxtable_per;

	siz--; /* need one for terminating '\0' */
	/* -- Skips space, tab and newline -- */
	while ((retval = lex_table[c = ap_char()]) == LT_SPC)
		continue;

	lexptr = lexval;
	*lexptr++ = c;

	switch (retval) {
	case LT_ERR:
		/* -- bad character -- */
		retval = LV_ERROR;
		break;

	case LT_EOD:
		/* -- end of data stream -- */
		retval = LV_EOD;
		break;

	case LT_COM:
		/* -- comma ","  the addr list separator -- */
		retval = LV_COMMA;
		break;

	case LT_AT:
		/* -- At sign "@"  the  node separator -- */
		retval = LV_AT;
		break;


/* ---------------------  data types and group list  -------------------- */

	case LT_COL:
		/* -- colon  ":"  the data type / group -- */
		retval = LV_COLON;
		break;

	case LT_SEM:
		/* -- semicolon ";"  the group end -- */
		retval = LV_SEMI;
		break;



/* -----------------------  person address list  ------------------------ */

	case LT_LES:
		/* -- less-than-sign "<"  the person list -- */

		if (lex_table[c = ap_char()] == LT_LES)
			/* -- << implies redirection -- */
			retval = LV_FROM;
		else {
			/* -- restore xtra char -- */
			ap_peek = c;
			retval = LV_LESS;
		}
		break;


	case LT_GTR:
		/* -- greater-than-sign ">" the end person -- */
		retval = LV_GRTR;
		break;


/* ---------------------  quoted & unquoted words  ------------------------ */

	case LT_SQT:
		/* -- back slash at start of atom - illegal -- */
		retval = LV_ERROR;
		break;
		
	case LT_LTR:
		/* -- letters -- */
	case LT_RPR:
		/* -- right paren ")"  its just char, here -- */

		for (;;) {
			if ((int) (lexptr - &(lexval[0])) >= siz)
				/* too long */
				retval = LV_ERROR;
			else {
				switch (lex_table[*lexptr++ = c = ap_char()]) {
				    case LT_LTR:
				    case LT_SQT:
				    case LT_RPR:
					continue;

				    case LT_ERR:
					retval = LV_ERROR;
					break;

				    case LT_EOD:
					/* -- permit eod to end string -- */
				    default:
					/* -- non-member character -- */
					ap_peek = c;
					lexptr--;
#ifdef AP_733_AT
					if (ap_lex_percent == TRUE &&
					    lexptr == &lexval[2] &&
					    uptolow (lexval[0]) == 'a' &&
					    uptolow (lexval[1]) == 't' )
						retval = LV_AT;
					else
#endif
						retval = LV_WORD;
				}
			}
			break;
		};
		break;


	case LT_QOT:
		/* -- double quote "\""  => string -- */
		retval = LV_WORD;

		/* -- don't put quotes into obvalue  -- */
		--lexptr;

		for (;;) {
			if ((int) (lexptr - &(lexval[0])) >= siz) 
				/* too long */
				retval = LV_ERROR;
			else {
				switch (lex_table[*lexptr++ = c = ap_char()]) {
				    case LT_QOT:
					--lexptr;
					break;

				    case LT_SQT:
					/* -- include next char w/out interpeting --*/
					/* -- and drop on through -- */
					--lexptr;
					*lexptr++ = ap_char();

				    case LT_RPR:
				    case LT_LPR:
				    case LT_ERR:
				    default:
					continue;

				    case LT_EOD:
					retval = LV_ERROR;
				}
			}
			break;
		}

		break;


/* ---------------------------  comment  ---------------------------------- */


	case LT_LPR:
		/* -- left paren "("  -- comment start -- */

		/* -- remove left-most paren -- */
		lexptr--;

		for (retval = 0;;) {
			/* -- retval is count of comment nesting -- */
			if ((int) (lexptr - &(lexval[0])) >= siz) {
				/* too long */
				retval = LV_ERROR;
			} else {
				switch (lex_table[*lexptr++ = c = ap_char()]) {
				    case LT_LPR:
					/* -- nested comments -- */
					/* -- just drop on through -- */
					retval++;

				    default:
					continue;

				    case LT_SQT:
					/* -- include next char w/out interpeting --*/
					--lexptr;
					*lexptr++ = ap_char();
					continue;

				    case LT_RPR:
					if (retval-- > 0)
						continue;
					/* -- remove right-most paren -- */
					lexptr--;
					retval = LV_COMMENT;
					break;

				    case LT_EOD:
				    case LT_ERR:
					retval = LV_ERROR;
					break;
				}
			}
			break;
		}

		break;



/* ------------------------ domain literal  ------------------------------- */



	case LT_LSQ:
		/* -- left squar bracket "[" -- */

		for(;;) {
			if ((int) (lexptr - &(lexval[0])) >= siz) 
				/* too long */
				retval = LV_ERROR;
			else {
				switch (lex_table[*lexptr++ = c = ap_char()]) {
				    default:
					continue;
				    case LT_SQT:
					/* -- include next char w/out interpeting --*/
					--lexptr;
					*lexptr++ = ap_char();
					continue;
				    case LT_RSQ:
					retval = LV_DLIT;
					break;
				    case LT_EOD:
				    case LT_ERR:
					retval = LV_ERROR;
					break;
				}
			}
			break;
		}

		break;
	}




/* -----------------------  cleanup and return  --------------------------- */

	*lexptr = '\0';

#ifdef AP_DEBUG
	if (ap_debug)
		PP_DBG ((" %s", namtab[retval]));
#endif

	return (ap_llex = retval);
}


/* ---------------------  Static Routines  -------------------------------- */


/*
get next input character
*/

static int ap_char()
{
	/* -- handle lookahead and 8th bit -- */

	extern int      (*ap_gfunc)();  /* -- ptr to character get fn -- */
	register int    i;

	if (ap_peek == 0)
		return (0);

	if ((i = ap_peek) > 0) {
		ap_peek = -1;
		return (i);
	}

	/*  -- EOD -- */
	if ((i = ((*ap_gfunc)())) == -1)
		return (0);

	/* -- force error, if eighth bit is on -- */
	return ((isascii (i)) ? i : '\177');
}
