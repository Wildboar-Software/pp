/* abbrev.c: abbreviates domains */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/tables/abbrev/RCS/abbrev.c,v 6.0 1991/12/18 20:33:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/tables/abbrev/RCS/abbrev.c,v 6.0 1991/12/18 20:33:28 jpo Rel $
 *
 * $Log: abbrev.c,v $
 * Revision 6.0  1991/12/18  20:33:28  jpo
 * Release 6.0
 *
 */



#include	"util.h"

extern char	*strdup (), *index();
static int	LineCount;
static char	*myname;
static char	*aliases[100];
static int	maxalias = 0;
static int	bitmap;

static void do_alias ();
/* ---------------------  Begin	 Routines  -------------------------------- */


void main (ac,av)
int	ac;
char	*av [];
{
	char	buffer [BUFSIZ], *entries[10];
	register char	*alias, *name, *nonwildcard;
	register int	i, NumEntries, use;
	char	rhs[BUFSIZ], entry[BUFSIZ];

	myname = av [0];
	if (ac < 2) {
		fprintf (stderr, "Usage: %s Domain...\n", myname);
		exit (1);
	}


	for (i = 1; i < ac; i++) {
		(void) sprintf (buffer, "X.%s", av[i]);
		aliases[maxalias++] = strdup (buffer);
	}
	LineCount = 0;

	while (fgets (buffer, sizeof buffer, stdin) != NULL) {
		LineCount++;
		alias = &buffer [0];
		printf ("%s", alias);
		if (*alias == '#' || *alias == '\n')
			continue;

		alias [strlen (alias) - 1] = '\0';
		for (name = alias; *name != '\0'; name++)
			if (*name == ':' || *name == '#')
				break;
		if (*name == '#')
			continue;

		if (*name == '\0') {
			fprintf (stderr, "%s: Syntax Error [%s] ",
				myname, alias);
			fprintf (stderr, "at line %d - Ignored\n", LineCount);
			continue;
		}
		*name++ = '\0';

		if (alias[0] == '*'
		    && alias[1] == '.')
			nonwildcard = &(alias[2]);
		else
			nonwildcard = &(alias[0]);

		if ((NumEntries = sstr2arg (name, 10, entries, "|")) < 1) {
			fprintf(stderr, "%s: Syntax Error [%s] ",
				myname, alias);
			fprintf(stderr, "at line %d - Ignored\n", LineCount);
			continue;
		}
		rhs[0] = '\0';
		use = 1;
		for (i = 0; i < NumEntries && use == 1; i++) {
			use = do_entry(entries[i], nonwildcard, entry, alias);
			if (rhs[0] != '\0')
				strcat (rhs, "|");
			strcat (rhs, entry);
		}
		if (use == 1)
			for (i = 0; i < maxalias; i++)
				do_alias (alias, rhs, aliases[i]);
	}
	exit (0);
}

do_entry(name, nonwildcard, rhs, alias)
char	*name;
char 	*nonwildcard;
char	*rhs;
char	*alias;
{
	char	*vec[10];
	int	vecp, i;
	int	use = 0;
	int	gotnorm;
	int	mtaislhs;
	if ((vecp = sstr2arg (name, 10, vec, " \t")) < 1) {
		fprintf(stderr, "%s: Syntax Error [%s] ",
			myname, alias);
		fprintf(stderr, "at line %d - Ignored\n", LineCount);
		return use;
	}
		
	rhs[0] = '\0';
	gotnorm = 0;
	mtaislhs = 0;
	for (i = 0; i < vecp; i++) {
		char *p;

		if (lexnequ (vec[i], "norm+mta", 8) == 0
		      || lexnequ (vec[i], "mta+norm", 8) == 0) { 
			use = 1;
			gotnorm = 1;
			if (rhs[0] != '\0')
				strcat(rhs, " ");
			if ((p = index(vec[i], '=')) != (char *) 0
			    && *(p+1) != NULL)
				strcat(rhs, vec[i]);
			else {
				/* construct norm+mta= */
				strcat(rhs, "norm+mta=");
				strcat(rhs, nonwildcard);
			}
		} else if (lexnequ (vec[i], "norm", 3) == 0) {
			use = 1;
			gotnorm = 1;
			if (rhs[0] != '\0')
				strcat(rhs, " ");
			if ((p = index(vec[i], '=')) != (char *) 0
			    && *(p+1) != NULL)
				strcat(rhs, vec[i]);
			else {
				/* construct norm= */
				strcat(rhs, "norm=");
				strcat(rhs, nonwildcard);
			}
		} else if (lexnequ (vec[i], "synonym", 7) == 0) {
			gotnorm = 1;
			use = 1;
			if (rhs[0] != '\0')
				strcat(rhs, " ");
			strcat(rhs, vec[i]);
		} else if (lexnequ (vec[i], "local", 5) == 0) {
			use = 1;
			if (rhs[0] != '\0')
				strcat(rhs, " ");
			strcat(rhs, vec[i]);
		} else if (lexnequ (vec[i], "mta", 3) == 0) {
			use = 1;
			if (rhs[0] != '\0')
				strcat(rhs, " ");
			if ((p = index(vec[i], '=')) != (char *) 0
			    && *(p+1) != NULL)
				strcat(rhs, vec[i]);
			else 
				/* construct norm= */
				mtaislhs = 1;
		} else {
			if (rhs[0] != '\0')
				strcat(rhs, " ");
			strcat(rhs, vec[i]);
		}
	}
	bitmap = 0;

	if (gotnorm == 0) {
		char	*key;
		/* add norm=lhs */
		if (mtaislhs == 1) 
			key = "norm+mta=";
		else
			key = "norm=";
		if (rhs[0] != '\0')
			strcat(rhs, " ");
		strcat(rhs, key);
		strcat(rhs, nonwildcard);
	} else if (mtaislhs == 1) {
		if (rhs[0] != '\0')
			strcat(rhs, " mta=");
		else
			strcat(rhs, "mta=");
		strcat(rhs, nonwildcard);
	}
	return use;
}

/* ---------------------  Static  Routines  ------------------------------- */



static void do_alias (alias, name, domain)
register char	*alias, *name, *domain;
{
	register char	*a, *d;
	int dn = -1;

	a = &alias [strlen (alias) - 1];
	d = &domain [strlen (domain) - 1];
	for (; a != alias && *a == *d; a--, d--)
		if (*a == '.') {
			dn ++;
			if (bitmap & (1 << dn))
				continue;
			bitmap |= (1 << dn);
			*a = '\00';
			if (strcmp (alias, "*") != 0)
				printf ("%s:%s\n", alias, name);
			*a = '.';
		}
}
