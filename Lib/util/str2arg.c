/* str2arg.c: convert a string into a list of args with 
	key = value processing */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/str2arg.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/str2arg.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: str2arg.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"

/*  convert string into argument list
 *
 *  stash a pointer to each field into the passed array.
 *  any common seperators split the words.  extra white-space
 *  between fields is ignored.
 *
 *  if the separator is '=', then the current argument position in
 *  the array points to "=", the next the one is the key and the
 *  value follows it.  This permits detecting variable assignment,
 *  in addition to positional arguments.
 *      i.e.,  key=value ->  = key value
 *
 *  specially-interpreted characters:
 *      space, tab, double-quote, backslash, comma, equal, slash, period,
 *      semi-colon, colon, carriage return, and line-feed (newline).
 *      preceding a special char with a backslash removes its
 *      interpretation.  a backslash not followed by a special is used
 *      to preface an octal specification for one character
 *
 *      a string begun with double-quote has only double-quote and
 *      backslash as special characters.
 *
 *  a field which begins with a hash (#) is interpreted
 *  as marking the rest of the line as a comment and it is skipped, as
 *  are blank lines
 */

extern int errno;

int str2arg (srcptr, maxf, argv)/* convert srcptr to argument list */
	register char *srcptr;  /* source data */
	int maxf;                /* maximum number of permitted fields */
	char *argv[];           /* where to put the pointers */
{
    char gotquote;      /* currently parsing quoted string */
    char lastdlm;       /* last delimeter character     */
    register int ind;
    register char *destptr;

    if (srcptr == 0)
    {
	errno = EINVAL;     /* emulate system-call failure */
	return (NOTOK);
    }

    for (lastdlm = (char) NOTOK, ind = 0, maxf -= 2; *srcptr != '\0'; ind++)
    {
	if (ind >= maxf)
	{
	    errno = E2BIG;      /* emulate system-call failure */
	    return (NOTOK);
	}
	for (argv[ind] = destptr = srcptr; isspace (*srcptr); srcptr++)
		;               /* skip leading white space         */
/**/

	for (gotquote = FALSE; ; )
	{
	    switch (*srcptr)
	    {
		default:        /* just copy it                     */
		    *destptr++ = *srcptr++;
		    break;

		case '\"':      /* beginning or end of string       */
		case '\'':
		    if (gotquote && gotquote == *srcptr)
			    gotquote = FALSE;
		    else if (gotquote)
			    *destptr ++ = *srcptr;
		    else
			    gotquote = *srcptr;
		    srcptr++;
		    break;

		case '\\':      /* quote next character             */
		    srcptr++;   /* just skip the back-slash         */
		    switch (*srcptr)
		    {           /* convert octal values             */
			case 'r':
			    *destptr++ = '\r';
			    srcptr++;
			    break;

			case 'n':
			    *destptr++ = '\n';
			    srcptr++;
			    break;

			case 'b':
			    *destptr++ = '\b';
			    srcptr++;
			    break;

			case 'f':
			    *destptr++ = '\f';
			    srcptr++;
			    break;

			default:
			    if (*srcptr >= '0' && *srcptr <= '7')
			    {
				*destptr = '\0';
				do
				    *destptr = (*destptr << 3) | (*srcptr++ - '0');
				while (*srcptr >= '0' && *srcptr <= '7');
				destptr++;
				break;
			    }    /* otherwise DROP ON THROUGH */
			case '\\':
			case '\"':
			case ' ':
			case ',':
			case '\t':
			case ';':
			case '#':
			case ':':
			case '=':
			case '/':
			case '|':
			    *destptr++ = *srcptr++;
			    break; /* just copy it */
		    }       /* DROP ON THROUGH */
		    break;

		case '=':   /* make '=' prefix to pair  */
		    if (gotquote)
		    {
			*destptr++ = *srcptr++;
			break;
		    }

		    ind++;      /* put value ahead of '=' */

		    argv[ind] = argv[ind - 1];
		    lastdlm = '=';
		    argv[ind - 1] = "=";
		    *destptr = '\0';
		    srcptr++;
		    goto nextarg;

		case ' ':
		case '\t':
		case '\n':
		case '\r':
		case ',':
		case ';':
		case '#':
		case ':':
		case '/':
		case '|':
		    if (gotquote)
		    {           /* don't interpret the char */
			*destptr++ = *srcptr++;
			break;
		    }

		    if (isspace (lastdlm))
		    {
			if (isspace (*srcptr))
			{           /* shouldn't be possible */
			    errno = EINVAL;
			    return (NOTOK);
			}
			else
			    if (*srcptr != '#')
				ind--;  /* "xxx , yyy" is only 2 fields */
		    }
		    lastdlm = *srcptr;
		    srcptr++;
		case '\0':
		    *destptr = '\0';
		    goto nextarg;
	    }

	    lastdlm = (char) NOTOK;     /* disable when field started */
	}

    nextarg:
	if (argv[ind][0] == '\0' && lastdlm == '#')
	    break;      /* rest of line is comment                  */
    }

    argv[ind] = (char *) 0;
    return (ind);
}
