/* rd_buf.c: low level line reading of Control and DR Txt Files */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_buf.c,v 6.0 1991/12/18 20:22:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/io/RCS/rd_buf.c,v 6.0 1991/12/18 20:22:26 jpo Rel $
 *
 * $Log: rd_buf.c,v $
 * Revision 6.0  1991/12/18  20:22:26  jpo
 * Release 6.0
 *
 */



#include "head.h"

#define COMMENTCHAR     '#'
int                     protocol_mode = 0;

static int glinread ();


/* ---------------------  Begin  Routines  -------------------------------- */




int txt2buf (linebuf, len, fp)
FILE           *fp;
int		len;
char           *linebuf;
{
	int             n;

	PP_DBG (("Lib/io/txt2buf()"));

	if ((n = glinread (fp, linebuf, len)) < 0)
		return (RP_EOF);
	if (n == 0)             /* time for a resync */
		return (RP_PARM);
	if (n == 1)
		return (RP_DONE);
	if (linebuf[--n] != '\n')
		return (RP_MECH);
	linebuf[n] = '\0';
	return (RP_OK);
}



/* ---------------------  Static  Routines  ------------------------------- */



static int glinread (fp, buf, size)
register FILE  *fp;
char           *buf;
int	size;
{
	register int    c;
	register int    i;
	extern  int protocol_mode;

	PP_DBG (("Lib/io/glinread()"));

	if (feof(fp))
		return EOF;

	for (--size, i = 0; i < size; i++)
		switch (c = getc (fp)) {
		case EOF:
			return (EOF);

		case '\0':
			return (0);

		case '\n':
			*buf++ = c;
			if (protocol_mode == 0) {
				/* -- first char on next line -- */
				(void) ungetc (c = getc (fp), fp);
				if (c == COMMENTCHAR) {
					/*
					check for comments within folding.
					Ignore everything up to and
					including new line character which
					is not followed by COMMENTCHAR,
					this could probably be recursive and
					clever - but what the hell
					*/
					while (42) {
						if ((c = getc (fp)) == '\n') {
							(void) ungetc (c =
							       getc (fp), fp);
							if (c != COMMENTCHAR)
								break;
						}
					}
				}

				if (c == ' ' || c == '\t') {
					/* continuation line */
					c = getc (fp);
					*(buf - 1) = ' ';
					break;
				}
			}
			*buf = 0;
			return (++i);

		default:
			*buf++ = c;
		}

	/*
	 * if ever get here then line too long
	 */
	*buf = 0;
	return (i);
}
