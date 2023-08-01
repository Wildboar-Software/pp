/* expand.c */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/expand.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/expand.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: expand.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "expand.h"

/*
 *			E X P A N D . C
 *
 *	Expand expands a text string expanding macros provided
 *  in a vector of key and value pairs.
 *
 *  e.g.	expand(buf, "$A+$B=$(RESULT)", array)
 *  with	Expand array[] = {"A", "1",
 *				 "B", "2",
 *				 "RESULT", "3",
 *				 0, 0};
 *  gives	"1+2=3" in buf.
 *
 *  wja@uk.ac.nott.maths		Wed Feb 22 15:00:26 GMT 1984
 *  DPK@BRL, 13 Apr 84	Made more portable and changed array usage.
 *  jpo@cs.nott.ac.uk 18 July 1989	Changed to structure.
 *  pac@cs.nott.ac.uk 			Mon Mar 12 19:54:39 1990
 *					added expand_dyn
 */


static  char *exname(), *exlookup();

#define	INC	BUFSIZ

char *expand_dyn(fmt, macros)
char	*fmt;
Expand	macros[];
{
	register char *bp = NULLCP, *cp;
	int	ix = 0, len = 0;
	char	name[LINESIZE];

	bp = (char *) malloc(INC * sizeof(*bp));
	len += INC;

	while (fmt && *fmt) {
		if (ix >= len) {
			/* resize */
			len += INC;
			bp = (char *) realloc(bp, (unsigned)(len*sizeof(char)));
		}
		if (*fmt != '$') 
			bp[ix++] =  *fmt++;
		else if (*++fmt == '$')
			bp[ix++] = *fmt++;
		else {
			fmt = exname(name, fmt);
			cp = exlookup (name, macros);
			while (cp && *cp) {
				if (ix >= len) {
					/* resize */
					len += INC;
					bp = (char *) realloc(bp, 
							      (unsigned)(len*sizeof(char)));
				}
				bp[ix++] = *cp++;
			}
		}
	}
	if (ix >= len) {
		/* resize */
		len += INC;
		bp = (char *) realloc(bp, (unsigned)(len*sizeof(char)));
	}
	bp[ix] = '\0';
	len = strlen(bp);
	bp = (char *) realloc(bp, (unsigned) ((len+1)*sizeof(char)));
	return bp;
}

char *expand(buf, fmt, macros)
char *buf, *fmt;
Expand macros[];
{
	register char *bp = buf, *cp;
	char name[LINESIZE];

	while(fmt && *fmt) {
		if(*fmt != '$')
			*bp++ = *fmt++;
		else if(*++fmt == '$')
			*bp++ = *fmt++;
		else {
			fmt = exname(name, fmt);
			cp = exlookup(name, macros);
			while(cp && *cp)
				*bp++ = *cp++;
		}
	}
	*bp = '\0';
	return buf;
}

static char *
exname(buf, str)
register char *buf, *str;
{
	if(*str == '(') {
		str++;
		while(*str != ')' && *str != '\0')
			*buf++ = *str++;
		if(*str != '\0')
			str++;
	}
	else
		*buf++ = *str++;
	*buf = '\0';
	return str;
}

static char *
exlookup(name, list)
register char *name;
Expand list[];
{
	register Expand *ep;

	for (ep = list; ep -> macro; ep++) {
		if(lexequ(name, ep -> macro) == 0)
			return (ep -> expansion);
	}
	return ("");
}
