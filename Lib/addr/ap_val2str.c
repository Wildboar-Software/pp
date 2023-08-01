/* ap_val2str: converts values to strings */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_val2str.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_val2str.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_val2str.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



/*
This function is just barely usable.  The hole problem of
quoted strings is hard to get right especially when the
mail system is trying to make up for human forgetfulness.
		-DPK-

SEK - have improved this somewhat by giving knowledge of
various object types.  Does not handle strings of spaces.
*/

#include	"util.h"
#include	"ap.h"

void ap_val2str (buf, value, obtype) /* -- convert to canonical str -- */
char                    *buf,
			*value,
			obtype;
{
	int             got_spcl;
	int             in_quote;
	register char   *from_ptr,
			*to_ptr;

	PP_DBG (("Lib/addr/ap_val2str ('%s', '%d')", value, obtype));

	if (obtype == AP_COMMENT) {
		for (from_ptr = value, to_ptr = buf; *from_ptr != '\0';
		     *to_ptr++ = *from_ptr++)

			switch (*from_ptr) {
			case '\r':
			case '\n':
			case '\\':
			case '(':
			case ')':
				*to_ptr++ = '\\';
			}

		*to_ptr = '\0';
		return;
	}


	if (obtype == AP_DOMAIN_LITERAL) {
		for (from_ptr = value, to_ptr = buf; *from_ptr != '\0';
		     *to_ptr++ = *from_ptr++)

			switch (*from_ptr) {
			case '\r':
			case '\n':
			case '\\':
				*to_ptr++ = '\\';
				continue;
			case '[':
				if (from_ptr != value)
					*to_ptr++ = '\\';
				continue;
			case ']':
				if (*(from_ptr + 1) != (char) 0)
					*to_ptr++ = '\\';
			}

		*to_ptr = '\0';
		return;
	}


	in_quote = FALSE;

	for (got_spcl = FALSE, from_ptr = value; *from_ptr != '\0';
	     from_ptr++) {
		switch (*from_ptr) {
		    case '"':
			/* -- flip-flop -- */
			in_quote = (in_quote == TRUE ? FALSE : TRUE);
			break;

		    case '\\':
		    case '\r':
		    case '\n':
			if (in_quote == FALSE) {
				got_spcl = TRUE;
				goto copyit;
			}
			break;

		    case '<':
		    case '>':
		    case '@':
		    case ',':
		    case ';':
		    case ':':
		    case '\t':
		    case '[':
		    case ']':
		    case '(':
		    case ')':
			if (in_quote == FALSE) {
				got_spcl = TRUE;
				goto copyit;
			}
			break;

		    case ' ':
			if (in_quote == FALSE)
				if ((obtype == AP_DOMAIN) ||
				    (obtype == AP_MAILBOX)) {
					got_spcl = TRUE;
					goto copyit;
				}
				else {
					/* -- hacked to handle " at " -- */
					/* -- this really is needed -- */

					if ((uptolow (*(from_ptr + 1))
					     == 'a') &&
					    (uptolow (*(from_ptr + 2))
					     == 't') &&
					    (*(from_ptr + 3)
					     == ' ')) {
						got_spcl = TRUE;
						goto copyit;
					}
				}
			break;


		    case '.':
			if (in_quote == FALSE &&
			    ((obtype == AP_GROUP_START) ||
			     (obtype == AP_PERSON_START))) {
				got_spcl = TRUE;
				goto copyit;
			}
			break;
		    default:
			if (!isascii(*from_ptr) || iscntrl(*from_ptr))
				if (in_quote == FALSE) {
					got_spcl = TRUE;
					goto copyit;
				}
			break;
		}
	}



	/* -- if were in a quote, something's wrong -- */
	got_spcl = in_quote;

copyit:
	to_ptr = buf;

	if (got_spcl)
		*to_ptr++ = '"';

	for (from_ptr = value; *from_ptr != '\0'; *to_ptr++ = *from_ptr++)
		switch (*from_ptr) {
		case '\r':
		case '\n':
		case '\\':
		case '"':
			if (got_spcl)
				*to_ptr++ = '\\';
		}

	if (got_spcl)
		*to_ptr++ = '"';

	*to_ptr = '\0';

}
