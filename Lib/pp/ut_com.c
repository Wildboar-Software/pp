/* ut_com.c Buffer utilities used by all the tx_*.c files */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_com.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_com.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_com.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include "util.h"



/* ---------------------  General  Routines  ------------------------------ */




char *Bf_put (buf, str)  /* cat str LWSP > buf */
char    *buf;
char    *str;
{
	while (*buf++ = *str++);
	return (buf);
}


extern	int	arg2vstr ();

int argv2fp (fp, argv)
FILE    *fp;
char    **argv;
{
	extern int      protocol_mode;
	char            buffer[10*BUFSIZ];

	if (arg2vstr (protocol_mode ? 0 : 72, sizeof buffer, buffer, argv) == NOTOK)
		return NOTOK;
	PP_DBG (("argv2fp(%d) -> '%.199s'", protocol_mode, buffer));
	(void) fputs (buffer, fp);
	(void) putc ('\n', fp);
	return ferror(fp) ? NOTOK : OK;
}




static char genbuf[10*BUFSIZ];
static char *_genbuf;


void genreset ()
{
	_genbuf = genbuf;
}



char  *genstore (str)
char    *str;
{
	char    *p = _genbuf;
	while (*_genbuf++ = *str++)
		continue;
	return p;
}
