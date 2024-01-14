/* util.h: various useful utility definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/util.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: util.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_UTIL
#define _H_UTIL


/* -- various common defines -- */


#include <stdio.h>              /* -- minus the ctype stuff -- */
#include <ctype.h>
#include <setjmp.h>
#include <isode/psap.h>
#include <errno.h>
#include "config.h"
#include "ll_log.h"

#define PP	60

/* -- declarations that should have been in the system files -- */

// #ifdef SVR4

#include <stdlib.h>
#include <string.h>

// #else
// #include <strings.h>
// #endif

#ifdef LINUX
#include <unistd.h>
#endif

extern char *multcat ();
extern char *multcpy ();
extern char *smalloc ();

/* -- some common logical values -- */


#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef YES
#define YES     1
#endif

#ifndef NO
#define NO      0
#endif

#ifndef OK
#define OK      0
#endif

#ifndef DONE
#define DONE    1
#endif

#ifndef NOTOK
#define NOTOK   -1
#endif

#ifndef MORE
#define MORE    2
#endif

#ifndef MAYBE
#define MAYBE   1
#endif


/* -- stdio extensions -- */

#ifndef lowtoup
#define lowtoup(chr)            (islower (chr) ? toupper (chr) : chr)
#endif

#ifndef  uptolow
#define uptolow(chr)            (isupper (chr) ? tolower (chr) : chr)
#endif

#ifndef MIN
#define MIN(a,b)                (( (b) < (a) ) ? (b) : (a) )
#endif

#ifndef MAX
#define MAX(a,b)                (( (b) > (a) ) ? (b) : (a) )
#endif

#ifndef isstr
#define isstr(ptr)              ((ptr) != 0 && *(ptr) != '\0')
#endif
#ifndef isnull
#define isnull(chr)             ((chr) == '\0')
#endif



/* -- provide a timeout facility -- */


extern  jmp_buf _timeobuf;

#ifndef timeout
#define timeout(val)            (setjmp(_timeobuf) ? 1 : (_timeout(val), 0))
#endif


/* -- some common extensions -- */

#ifndef LINESIZE
#define LINESIZE        512     /* -- max line length -- */
#endif

#ifndef FILNSIZE
#ifdef MAXNAMLEN
#define FILNSIZE MAXNAMLEN
#else
#define FILNSIZE        256     /* -- max filename length -- */
#endif
#endif

#ifndef	MAXPATHLENGTH
#ifdef MAXPATHLEN
#define MAXPATHLENGTH MAXPATHLEN
#else
#define MAXPATHLENGTH	1024
#endif
#endif

#ifndef MAXFORK
#define MAXFORK         10      /* -- no. of times to try a fork() -- */
#endif

#ifndef NULLCP
#define NULLCP          ((char *)0)
#define NULLVP          ((char **)0)
#endif

#ifndef NULLFILE
#define NULLFILE        ((FILE *)0)
#endif

/* utctime stuff */

extern UTC	time_t2utc ();
extern time_t	utc2time_t ();
extern UTC	utcdup ();
extern UTC 	utcnow ();

#define LOCK_FLOCK	0
#define LOCK_FCNTL	1
#define LOCK_FILE	2
#define LOCK_LOCKF	3

extern FILE	*flckopen ();
extern int	flckclose ();

/* tailoring */
#define TAI_NONE	0000
#define TAI_LOGS	0001
#define TAI_SIGNALS	0002
#define TAI_ALL		(TAI_LOGS|TAI_SIGNALS)

#ifdef SYS5
#define killpg(pid,sig) 	kill(-(pid), (sig))
#endif

#ifndef HAS_FSYNC
#if defined(BSD42) || defined(SVR4) 
#define HAS_FSYNC
#endif
#endif

int prefix(register char *str1, register char *str2);

#endif
