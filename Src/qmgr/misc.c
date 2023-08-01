/* misc.c: misc routines required by qmgr */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/qmgr/RCS/misc.c,v 6.0 1991/12/18 20:27:38 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/misc.c,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: misc.c,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 */



#include "types.h"

int	hash (str, n) /* case independant hash function */
char	*str;
int	n;
{
	extern char chrcnv[];
	int	res = 0;
	char	*cp;

	for (cp = str; *cp; cp++) {
		res = 3 * res + chrcnv[*cp];
		res %= n;
	}
	return res;
}

void cache_set (cp, utct)
Cache	*cp;
struct type_UNIV_UTCTime *utct;
{
	cp -> cachetime = utcqb2time_t (utct);
	cp -> cacheplus = 0;
}

void cache_clear (cp)
Cache *cp;
{
	cp -> cachetime = cp -> cacheplus = 0;
}

void cache_inc (cp, factor)
Cache *cp;
time_t	factor;
{
	int rfact;

	if (cp -> cacheplus < CACHE_MAX)
		cp -> cacheplus ++;
	cp -> cachetime = current_time + factor * cp -> cacheplus;

	/* round up the cache time - helps cluster mta's etc. */
	rfact = (factor/10) * cp -> cacheplus;
	if (rfact <= 0)
		rfact = 1;
	cp -> cachetime += rfact - cp -> cachetime % rfact;
}

time_t	utcqb2time_t (qb)
struct qbuf *qb;
{
	char *p = qb2str (qb);
	UTC	ut;
	time_t	t;

	ut = str2utct(p, strlen (p));
	t = utc2time_t (ut);
	free (p);
	return t;
}

