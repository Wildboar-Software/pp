#include <config.h>

#ifdef SYS5

/* sys5_regex.c: system 5 Regex emulation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/sys5_regex.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/sys5_regex.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: sys5_regex.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */

/*
 *	Emulation of UCB regular expressions routines using
 *	the ATT variety.  Note that the regex syntax is not
 *	identical (ATT has extra features) so it may be necessary
 *	to preprocess the string before passing to regcmp().
 *
 *	I am not sure if my mapping of the return from regex() is
 *	adequate. It may also be necessary to count '(...)$n' parts
 *	of the regular expression to be able to provide return value
 *	storage for regex().
 *
 *	You probably need to lib agains libPW for these routines but
 *	it may be necessary to include an explicit "-lc" before the
 *	"-lPW" so as not to pick up other (unwanted) PWB routines.
 */

static char * complied_regex;

char * re_comp(s)
char *s;
{
    char * res;
    extern char * regcmp();

    if (s == (char *)0 || *s == '\0')
	return (char *)0;

    res = regcmp(s, 0);
    if (res == (char *)0)
	return "invalid regular expression";
    
    if (complied_regex != (char *)0)
	free(complied_regex);
    
    complied_regex = res;
    return (char *)0;
}

re_exec(s)
char *s;
{
    extern char * regex();

    if (complied_regex == (char *)0)
	return -1;

    if (regex(complied_regex, s) == (char *)0)
	return 0;
    else
	return 1;
}
#else

sys5_regex_stub () {}

#endif /* SYS5 */
