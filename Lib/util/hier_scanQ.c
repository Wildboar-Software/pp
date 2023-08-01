/* hier_scanQ.c: scan q which may stored in a hierarchy of directories */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/hier_scanQ.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/hier_scanQ.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: hier_scanQ.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include <pwd.h>
#include <isode/usr.dirent.h>
#include <sys/stat.h>

#define SIZE_INCR	1000

#ifndef S_ISDIR
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif

static void addtolist();

free_hier(arr, num)
char	**arr;
int	num;
{
	int	i;
	for (i = 0; i < num; i++)
		free(arr[i]);
	free((char *) arr);
}

static int isNum(str)
char	*str;
{
	int	retval = 0;
	
	while (str != NULLCP && *str != '\0'
	       && (retval = isdigit(*str))) str++;
	return retval;
}

hier_scanQ(dname, base, pnum, parr, psize, nonMsg)
char	*dname;
char	*base;
int	*pnum;
char	***parr;
int	*psize;
IFP	nonMsg;
{
	struct dirent *dp;
	DIR *dirp;
	char	ndname[BUFSIZ];

	if ((dirp = opendir(dname)) == NULL) {
		PP_SLOG (LLOG_EXCEPTIONS, dname,
			 ("Can't open directory"));
		return;
	}

	while ((dp = readdir (dirp)) != NULL) {
		if (strncmp (dp->d_name, "msg.", strlen("msg.")) == 0)
			addtolist(dp->d_name, base, pnum, parr, psize);
		else {
			(void) sprintf (ndname, "%s/%s",
					dname, dp -> d_name);

			if (isNum (dp->d_name)) {
				char	nbase[BUFSIZ];
				struct stat st;

				if (stat (ndname, &st) != NOTOK
				    && S_ISDIR(st.st_mode)) {
					if (base)
						(void) sprintf (nbase, 
								"%s/%s",
								base, 
								dp -> d_name);
					else
						(void) sprintf (nbase, 
								dp -> d_name);
					hier_scanQ(ndname, nbase, pnum, 
						   parr, psize, nonMsg);
				} else if (nonMsg != NULLIFP)
					(*nonMsg) (ndname);

			} else if (nonMsg != NULLIFP && 
				   lexequ(dp->d_name, ".") != 0 &&
				   lexequ(dp->d_name, "..") != 0)
				(*nonMsg) (ndname);
		}
	}

	(void) closedir (dirp);
}

static void addtolist (name, base, pnum, parr, psize)
char	*name;
char	*base;
int	*pnum;
char	***parr;
int	*psize;
{
	char	*np;
	char	fname[BUFSIZ];

	if (base)
		(void) sprintf (np = fname, "%s/%s", base, name);
	else
		np = name;

	if (*psize == *pnum) {
		if (*psize == 0) {
			*psize = SIZE_INCR;
			*parr = (char **) smalloc ((int)sizeof (char *) * (*psize));
		} else {
			*psize += SIZE_INCR;
			*parr = (char **) realloc ((char *) (*parr),
						   (unsigned)sizeof (char *) * (*psize));
			if (*parr == NULLVP)
				PP_OPER (NULLCP,
					 ("FATAL: Out of memory"));
		}
	}
	(*parr)[(*pnum)++] = strdup (np);
}
