/* parse.c: various parsing routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/parse.c,v 6.0 1991/12/18 20:26:48 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/parse.c,v 6.0 1991/12/18 20:26:48 jpo Rel $
 *
 * $Log: parse.c,v $
 * Revision 6.0  1991/12/18  20:26:48  jpo
 * Release 6.0
 *
 */



#include 	"console.h"

/* convert a given string to time */

char	*unparsetime(mytime)
unsigned long mytime;
{
	char	buf[BUFSIZ];
	unsigned long result;
	buf[0] = '\0';
	if ((result = mytime / (60 * 60 * 24)) != 0) {
		sprintf(buf, "%d d",result);
		mytime = mytime % (60 * 60 *24);
	}

	if ((result = mytime / (60 * 60)) != 0) {
		sprintf(buf, (buf[0] == '\0') ? "%s%d h" : "%s %d h", 
			buf, result);
		mytime = mytime % (60 * 60);
	}

	if ((result = mytime / 60 ) != 0) {
		sprintf(buf, (buf[0] == '\0') ? "%s%d m" : "%s %d m", 
			buf, result);
		mytime = mytime % (60);
	}

	if (mytime != 0) 
		sprintf(buf, (buf[0] == '\0') ? "%s%d s" : "%s %d s",
			buf, mytime);

	return strdup(buf);
}

time_t parsetime(s)
char	*s;
{
	int	n;
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
		    case 's':
		    case 'S': 
			break;
		    case 'm': 
		    case 'M':
			n *= 60; 
			break;
		    case 'h':
		    case 'H':
			n *= 3600; 
			break;
		    case 'd': 
		    case 'D':
			n *= 86400; 
			break;
		    case 'w':
		    case 'W':
			n *= 604800;
			break;
		    default:
			break;
		}
		return n + parsetime(s+1);
	}
	else return n + parsetime(s);
}

extern time_t	time();

char *mystrtotime(str)
char	*str;
{
	UTC	utc;
	time_t	newsecs;
	time_t	 current;
	char	*retval;

	newsecs = parsetime(str);
	time(&current);
	
	current += newsecs;
		
	utc = time_t2utc(current);

	retval = utct2str(utc);
	free((char *) utc);
	return strdup(retval);
}

#define MaxCharPerInt 16

/* convert integer to string */
char	*itoa(i)
int 	i;
{
	char 	buf[MaxCharPerInt];

	sprintf(buf,"%d",i);
	
	return strdup(buf);
	
}

/* convert number to volume */

char	*vol2str(vol)
int	vol;
{
	char	buf[BUFSIZ];
	
	num2unit(vol, buf);
	return strdup(buf);
}

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

char	*time_t2RFC(in)
time_t	in;
{
	char	buf[BUFSIZ];
	struct tm *tm;
	struct UTCtime uts;
	struct timeval dummy;
	struct timezone time_zone;
	
	tm = localtime (&in);
	tm2ut (tm, &uts);
	uts.ut_flags |= UT_ZONE;
	gettimeofday(&dummy, &time_zone);
	uts.ut_zone = time_zone.tz_minuteswest;
	UTC2rfc(&uts, buf);
	return strdup(buf);
}


/*  */

/* reordering of domains and addresses */

char	*
reverse_adr(adr)
char	*adr;
{
	extern char	*rindex();

	char	*ret = malloc(strlen(adr) + 1), savech;
	char	*ix = adr, *iy = ret, *dom_end, *cix, *last, *dot;
	int	cont;
	while (*ix != '\0') {
		switch (*ix) {
		    case '@':
		    case '%':
			/* start of domain so reverse */
			*iy++ = *ix++;
			dom_end = ix;
			cont = TRUE;
			while (*dom_end != '\0'
			       && cont == TRUE) {
				switch(*dom_end) {
				    case ',':
				    case ':':
					cont = FALSE;
					break;
				    default:
					dom_end++;
					break;
				}
			}
			/* domain stretches from ix -> dom_end */

			last = dom_end;
			while (last != ix) {
				savech = *last;
				*last = '\0';
				if ((dot = rindex(ix, '.')) == NULLCP) {
					dot = ix;
					cix =  dot;
				} else 
					cix = dot + 1;
				*last = savech;
				do 
				{
					*iy++ = *cix++;
				} while (cix != last);
				
				if (dot != ix) {
					*iy++ = '.';
					last = dot;
				} else
					last = ix;
			}
			ix = dom_end;
			break;
				
		    default:			
			*iy++ = *ix++;
			break;
		}
	}
	*iy = '\0';
	return ret;
}

char	*
reverse_mta(mta)
char	*mta;
{
	extern	char	*index();
	char *adr, *ret, *ix;
	char	buf[BUFSIZ];
	(void) sprintf(buf, "foo@%s", mta);
	ret = reverse_adr(buf);
	ix = index (ret, '@');
	adr = strdup(++ix);
	free(ret);
	return adr;
}

num2unit(num, buf)
int	num;
char	*buf;
{
	int	absolute;

	absolute = (num < 0) ? -num : num;
	
	if (absolute > 1000000)
		(void) sprintf(buf, "%s%.2f M", 
			       (num < 0) ? "-" : "",
			       (double) absolute / 1000000.0);
	else if (absolute > 1000)
		(void) sprintf(buf, "%s%.2f k",
			       (num < 0) ? "-" : "",
			       (double) absolute / 1000.0);
	else
		(void) sprintf(buf, "%s%d", 
			       (num < 0) ? "-" : "",
			       absolute);
}
