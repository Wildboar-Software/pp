/* cons-misc.c: misc routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/cons-misc.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/cons-misc.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: cons-misc.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */
#include "util.h"
#include <sys/time.h>
#include "qmgr-int.h"
#include "Qmgr-types.h"


/*  */

time_t convert_time(qb)
struct type_UNIV_UTCTime	*qb;
{
	char	*str;
	time_t	temp;
	if (qb == NULL)
		return 0;
	str = qb2str(qb);
	temp = utc2time_t(str2utct(str, strlen(str)));
	free(str);
	return temp;
}

/*  */

ProcStatus	*convert_ProcStatus(pepsy)
register struct type_Qmgr_ProcStatus	*pepsy;
{
	ProcStatus	*ret = (ProcStatus *) malloc(sizeof(*ret));
	
	ret->enabled = pepsy->enabled;
	ret->lastAttempt = convert_time(pepsy->lastAttempt);
	ret->cachedUntil = convert_time(pepsy->cachedUntil);
	ret->lastSuccess = convert_time(pepsy->lastSuccess);
	return ret;
}
/*  */

long 	parseUnitStr (s, isTime)
char	*s;
int	isTime;
{
	long n;
	if (s == NULLCP || *s == NULL) return 0;
	while(*s != NULL && isspace(*s)) s++;
	n = 0;
	while (*s != NULL && isdigit(*s)) {
		n = n * 10 + *s - '0';
		s++;
	}
	while (*s != NULL && isspace(*s)) s++;
	if (*s != NULL && isalpha(*s)) {
		switch (*s) {
		    case 's': /* seconds */
		    case 'S': 
			break;
		    case 'm':  /* minutes */
		    case 'M':  /* or millions */
			if (isTime == TRUE)
				n *= 60; 
			else
				n *= 1000000;
			break;
		    case 'h': /* hours */
		    case 'H':
			n *= 3600; 
			break;
		    case 'd': /* days */
		    case 'D':
			n *= 86400; 
			break;
		    case 'w': /* weeks */
		    case 'W':
			n *= 604800;
			break;
		    case 'k': /* kilo */
		    case 'K':
			n *= 1000;
			break;
		    default:
			break;
		}
		return n + parseUnitStr(s+1, 1);
	}
	else return n + parseUnitStr(s, 1);
}

/*  */

char	*unparseUnitStr(n, isTime)
long	n;
int	isTime;
{
	char	buf[BUFSIZ];
	buf[0] = '\0';
	if (isTime == TRUE) {
		unsigned long	res;
		if ((res = n / (60 * 60 * 24)) != 0) {
			(void) sprintf(buf, "%d d", res);
			n = n % (60 * 60 * 24);
		}
		if ((res = n / (60 * 60)) != 0) {
			(void) sprintf(buf,
				('\0' == buf[0]) ? "%s%d h" : "%s %d h",
				buf, res);
			n = n % (60 * 60);
		}
		if ((res = n / 60) != 0) {
			(void) sprintf(buf, 
				('\0' == buf[0]) ? "%s%d m" : "%s %d m",
				buf, res);
			n = n % 60;
		}
		
		if (0 != n)
			(void) sprintf(buf, 
				('\0' == buf[0]) ? "%s%d s" : "%s %d s",
				buf, n);
	} else {
		/* print to largest unit with 2 significant figures */
		long	absolute = (n < 0) ? -n : n;

		if (absolute > 1000000)
			(void) sprintf(buf, "%s%.2f M", 
				       (n < 0) ? "-" : "",
				       (double) absolute / 1000000.0);
		else if (absolute > 1000)
			(void) sprintf(buf, "%s%.2f k",
				       (n < 0) ? "-" : "",
				       (double) absolute / 1000.0);
		else
			(void) sprintf(buf, "%s%d", 
				       (n < 0) ? "-" : "",
				       absolute);
	}
	return strdup(buf);
}

/*  */

char *mystrtotime(str)
char	*str;
{
	UTC	utc;
	time_t	newsecs;
	time_t	 current;
	char	*retval;

	newsecs = (time_t) parseUnitStr(str, TRUE);
	(void) time(&current);
	
	current += newsecs;
		
	utc = time_t2utc(current);

	retval = utct2str(utc);
	free((char *) utc);
	return strdup(retval);
}

/*  */

char	*time_t2RFC(in)
time_t	in;
{
	char	buf[BUFSIZ];
	struct tm *tm;
	struct UTCtime uts;
#ifdef SVR4
	extern long timezone;
#else
	struct timeval dummy;
	struct timezone time_zone;
#endif
	
	tm = localtime (&in);
	tm2ut (tm, &uts);
	uts.ut_flags |= UT_ZONE;
#ifdef SVR4
	uts.ut_zone = timezone/60;
#else
	(void) gettimeofday(&dummy, &time_zone);
	uts.ut_zone = time_zone.tz_minuteswest;
#endif
	if (UTC2rfc(&uts, buf) == OK)
		return strdup(buf);
	return NULLCP;
}
