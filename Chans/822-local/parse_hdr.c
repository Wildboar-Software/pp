/* parse_hdr: attempt to parse a header file */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/822-local/RCS/parse_hdr.c,v 6.0 1991/12/18 20:04:31 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/822-local/RCS/parse_hdr.c,v 6.0 1991/12/18 20:04:31 jpo Rel $
 *
 * $Log: parse_hdr.c,v $
 * Revision 6.0  1991/12/18  20:04:31  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "expand.h"
#include "loc.h"

extern void	compress();

parse_hdr (fd, vars, maxvp)
int	fd;
Expand	vars[];
int	maxvp;
{
	FILE	*fp;
	Symbol	*sp;
	char	*name, *contents;
	int	vp = 0;

	PP_TRACE (("parse_hdr (%d, vars, %d)", fd, maxvp));

	fp = fdopen (dup(fd), "r");

	while (hdr_read (fp, &name, &contents) == OK) {
		if ((sp = lookup (name)) != NULL && sp -> type == field) {
			if (vp >= maxvp)
				adios (NULLCP, "More than %s variables!",
				       maxvp);
			if (contents == NULLCP)
				contents = "";
			if (sp -> str && *sp -> str) {
				char *cp;
				int i;

				cp = smalloc (strlen (sp -> str) +
					      strlen (contents) + 2);
				(void) strcpy (cp, sp -> str);
				(void) strcat (cp, " ");
				(void) strcat (cp, contents);
				free (sp -> str);
				sp -> str = cp;
				for (i = 0; i < vp; i++)
					if (lexequ (vars[i].macro, name) == 0) {
						vars[i].expansion = sp -> str;
						break;
					}
				if (i >= vp) {
					vars[vp].macro = strdup (name);
					vars[vp++].expansion = sp -> str;
				}
			}
			else {
				sp -> str = strdup (contents);
				vars[vp].macro = strdup (name);
				vars[vp++].expansion = sp -> str;
			}
		}
	}
	fclose (fp);
	return vp;
}

hdr_read (fp, np, cp)
FILE	*fp;
char	**np, **cp;
{
	int	c;
	static char	mybuf[8192];
	char *p;

	PP_TRACE (("hdr_read (fp, np, cp)"));

	*cp = NULLCP;
	*np = mybuf;

	for (p = mybuf; (c = getc (fp)) != EOF;) {
		if (c == '\n') {
			if ((c = getc(fp)) == ' ' || c == '\t')
				continue;
			else {
				ungetc (c, fp);
				break;
			}
		}
		else if (c == ':' && *cp == NULLCP) {
			*cp = p + 1;
			*p++ = '\0';
			continue;
		}
		if (p - mybuf < sizeof(mybuf) - 3)
			*p++ = c;
			
	}
	if (c == EOF && p == mybuf)
		return DONE;
	else if (p == mybuf)
		return DONE;

	*p = 0;
	compress (mybuf, mybuf);
	if (*cp != NULLCP)
		(void) compress (*cp, *cp);

	for (p = *np; *p; p++)
		if (isupper(*p))
			*p = tolower (*p);

	PP_TRACE (("hdr_read field=%s contents=%s", *np, *cp));
	return OK;
}
