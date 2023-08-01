/* parsebr.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/parsesbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/parsesbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: parsesbr.c,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 */



#include <util.h>
#include <ctype.h>
#include "ap.h"
#include "chan.h"

#define MAXFIELDS 100

struct fields {
	char	*name;
	int	nlen;
	int	len;
	int	start;
	int	finish;
	int	flags;
#define PARSE_FROM	1
#define PARSE_BODY	2
#define PARSE_SIZE	3
#define PARSE_EXTRA	4
	char	*value;
} fields[MAXFIELDS];

int	nfields = 0;

parse_msg (displayline, from, format, maxlen, msgsize, extra)
char	*displayline;
char 	*from;
char	*format;
int	maxlen;
int	msgsize;
char	*extra;
{
	char	buf[BUFSIZ], buf2[BUFSIZ], *cp;
	struct fields *fp;
	int n;

	break_fields (format);

	while (fgets (buf, sizeof buf, stdin) != NULL) {
		if (buf[0] == '\n')
			break;
		if (lexnequ (buf, "from:", 5) == 0)
			fillin_from (from, buf);
		for (fp = fields; fp < &fields[nfields]; fp++)
			if (lexnequ (buf, fp -> name, fp -> nlen) == 0) {
				compress (buf + fp -> nlen + 1, buf2);
				fp -> value = strdup (buf2);
				break;
			}
	}

	buf[0] = NULL;
	n = fread (buf, 1, sizeof buf, stdin);
	buf[n] = NULL;
	for (cp = buf; *cp; cp++) {
		if (*cp == '\n')
			*cp = ' ';
	}
	compress (buf, buf);
	
	for (fp = fields; fp < &fields[nfields]; fp++) {
		if (fp -> value == NULLCP)
			fp -> value = "";
		if (fp -> flags == PARSE_FROM) {
			if ((cp = index (fp -> value, '<'))) {
				*cp = '\0';
				compress (fp -> value, fp -> value);
			}
		}
		else if (fp -> flags == PARSE_BODY)
			fp -> value = strdup (buf);
		else if (fp -> flags == PARSE_SIZE) {
			(void) sprintf (buf2, "%d", msgsize);
			fp -> value = strdup (buf2);
		}
		else if (fp -> flags == PARSE_EXTRA)
			fp -> value = extra;
	}

	build_string (format, displayline);
	displayline[maxlen] = 0;
}

break_fields (str)
char	*str;
{
	char	*cp, *p;
	struct fields *fp;
	char buf[BUFSIZ];

	if (str == NULL)
		return;
	for (cp = str; *cp; ) {
		if (*cp++ != '%')
			continue;
		if (*cp == '%') {
			cp ++;
			continue;
		}
		fp = &fields[nfields++];
		if (nfields >= MAXFIELDS)
			nfields --;
		fp -> start = cp - str - 1;
		if (isdigit (*cp)) {
			int n = 0;
			while (isdigit (*cp)) 
				n = n * 10 + *cp++ - '0';
			fp -> len = n;
		}

		for (p = cp; isalpha(*p) || *p == '-'; p++)
			continue;
		if (cp == p) {
			nfields --; 
			continue;
		}
		(void) strncpy (buf, cp, p -cp);
		buf[p - cp] = 0;
		fp -> name = strdup (buf);
		fp -> nlen = strlen (buf);

		if(lexequ (buf, "from") == 0)
			fp -> flags = PARSE_FROM;
		else if (lexequ (buf, "body") == 0)
			fp -> flags = PARSE_BODY;
		else if (lexequ (buf, "size") == 0)
			fp -> flags = PARSE_SIZE;
		else if (lexequ (buf, "extra") == 0)
			fp -> flags = PARSE_EXTRA;
		cp = p;
		fp -> finish = cp - str;
	}
}

build_string (format, dest)
char	*format;
char	*dest;
{
	char	*cp;
	int	last;
	struct fields *fp;

	if (nfields == 0) {
		(void) strcpy (dest, format);
		return;
	}
	cp = dest;
	last = 0;
	for (fp = fields; fp < &fields[nfields]; fp ++) {
		(void) strncpy (cp, format + last, fp -> start - last);
		cp += fp -> start - last;

		if (fp -> len) {
			int mlen;

			mlen = strlen (fp -> value);
			if (fp -> len < mlen)
				mlen = fp -> len;

			(void) sprintf (cp, "%-*.*s", fp -> len,
					mlen, fp -> value);
			
			cp += strlen (cp);
		}
		else if (fp -> value) {
			(void) strcpy (cp, fp -> value);
			cp += strlen (fp -> value);
		}
		last = fp -> finish;
	}
	(void) strcpy (cp, format + last);
	*cp = NULL;
}

fillin_from (dest, line)
char *dest, *line;
{
	char *dp;
	char *cp = line + 5; /* skip over from: */
	AP_ptr mbox, domain, tree;


	dest[0] = 0;

	if (ap_s2p (cp, &tree, NULLAPP, NULLAPP,
		    &mbox, &domain, NULLAPP) == (char *)NOTOK)
		return;
#ifdef UKORDER
	if (ap_dmnormalize (domain, CH_UK_PREF) != OK) {
#else
	if (ap_dmnormalize (domain, CH_USA_PREF) != OK) {
#endif
		ap_sqdelete (tree, NULLAP);
		ap_free (tree);
		return;
	}

	dp = ap_p2s_nc (NULLAP, NULLAP, mbox, domain, NULLAP);
	if (dp != (char *)NOTOK && isstr(dp))
		(void) strcpy (dest, dp);
	ap_sqdelete (tree, NULLAP);
	ap_free (tree);
	free (dp);
}
