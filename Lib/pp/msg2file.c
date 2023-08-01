/* msg2file: convert message reference into a filename */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/msg2file.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/msg2file.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: msg2file.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */

int n_subdirs = 2;

/* assume of the form msg.NNNNN-NNN */

int msg2file (name, fullname)
char *name, *fullname;
{
	int copyn = 5;
	int i = 0;
	char *p;

	p = fullname;
	for (i = 0; i < n_subdirs; i++, copyn++) {
		(void) strncpy (p, name, copyn);
		p += copyn;
		*p++ = '/';
		*p = '\0';
	}
	(void) strcpy (p, name);
}

