/* arg2str: list of arguments to string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/arg2str.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/arg2str.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: arg2str.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"

extern void err_abrt ();

/* convert an array of strings to one or more lines of 'arguments'.
 *
 * this is intended to be used along with the str2arg() routine.
 */

int arg2vstr (linelen, maxlen, buf, argv) /* convert the list to a string */
    int linelen;
    int maxlen;                         /* length of string */
    char *buf;                          /* where to put the output string */
    char **argv;                        /* the argument vector */
{
	unsigned totlen,	/* total length of current line */
		len;		/* length of current argument */
	unsigned gotdelim;	/* a delimiter char is in arg */
	unsigned gotpair;	/* key/value pair */
	char tmpstr[8*BUFSIZ];	/* string under construction  */
	register char *src,
		*bp,
		*dest = NULLCP;

	bp = buf;
	for (totlen = gotpair = 0, *bp = '\0';
	     *argv != (char *) 0; argv++)
	{
		if (gotpair  == 0 && strcmp ("=", *argv) == 0)
		{
			dest = tmpstr;
			gotpair = 2; /* take the next two arguments  */
			continue;
		}


		if(**argv == '\0'){
			gotdelim = TRUE;
			goto nextone;
		}
		for (src = *argv, gotdelim = FALSE; *src != '\0'; src++)
			switch (*src)
			{
			    case ' ':
			    case '\t':
			    case '=':
			    case ',':
			    case ';':
			    case ':':
			    case '/':
			    case '|':
			    case '.':
			    case '#':
				gotdelim = TRUE;
				goto nextone;
			}

	    nextone:
		if (gotpair == 0)
			dest = tmpstr;
		if (gotdelim)
			*dest++ = '"';
		for (src = *argv; *src != '\0'; src++)
		{
			switch (*src)
			{
			    case '\b':
				*dest++ = '\\';
				*dest++ = 'b';
				break;

			    case '\t':
				*dest++ = '\\';
				*dest++ = 't';
				break;

			    case '\f':
				*dest++ = '\\';
				*dest++ = 'f';
				break;

			    case '\r':
				*dest++ = '\\';
				*dest++ = 'r';
				break;

			    case '\n':
				*dest++ = '\\';
				*dest++ = 'n';
				break;

			    case '\\': /*  Added by Doug Kingston  */
			    case '\"':
				*dest++ = '\\';
				*dest++ = *src;
				break;

			    default:
				if (iscntrl (*src))
				{
					*dest++ = '\\';
					*dest++ = ((*src >> 6) & 07) + '0';
					*dest++ = ((*src >> 3) & 07) + '0';
					*dest++ =  (*src & 07) + '0';
				}
				else
					*dest++ = *src;
			}
		}

		if (gotdelim)
			*dest++ = '"';

		switch (gotpair) /* handle key/value differently         */
		{
		    case 2:
			*dest++ = '=';
			gotpair--;
			continue;

		    case 1:
			gotpair = 0;
		}

		*dest = '\0';
		len = dest - tmpstr;

		if (totlen != 0)
		{
			if ((linelen > 0) && ((totlen + len) > linelen))
			{
				*bp ++ = '\n';
				*bp ++ = '\t';
				maxlen -= 2;
				totlen = 8;
			}
			else
			{
				*bp ++ = ' ';
				maxlen --;
				totlen += 1;
			}
			*bp = '\0';
		}
		if (maxlen < len)
			return NOTOK;
		maxlen -= len;
		for (dest = tmpstr; *dest;)
			*bp ++ = *dest++;
		*bp = '\0';
		totlen += len;
	}
	return OK;
}
