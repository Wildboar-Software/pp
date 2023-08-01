/* ps250_basic.c: basic encode decode fax control sequences  */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_basic.c,v 6.0 1991/12/18 20:07:26 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/ps250_basic.c,v 6.0 1991/12/18 20:07:26 jpo Rel $
 *
 * $Log: ps250_basic.c,v $
 * Revision 6.0  1991/12/18  20:07:26  jpo
 * Release 6.0
 *
 */


#include "util.h"
#include "ps250.h"
#ifdef SYS5		/* Can't use isode/sys.file.h due to termios.h */
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

extern void advise (), adios ();
int fax_debug = 0;

static void hexdump();

/* #define CHECKSUM */

/*
 * basic io (chunk level)
 */

#define	INC 	32

int	writex (fd, buffer, size)
int	fd;
char	*buffer;
int	size;
{
	char	*cp = buffer;
	int	done = cp - buffer;
	int	num, n;
	while (done < size) {
		if (size - done < INC)
			num = size - done;
		else
			num = INC;

		if ((n = write (fd, cp, num)) <= 0)
			return NOTOK;
/*		if (fax_debug)
			hexdump ("<=", cp, n);*/
		cp += num;
		done = cp - buffer;
	}
	return done;
}
		
static int readx (fd, buffer, size)
int	fd;
char	*buffer;
int	size;
{
	char	*cp = buffer;
	int	count = 0, n;

	while (size > 0) {
		if ((n = read (fd, cp, size)) <= 0)
			return NOTOK;
/*		if (fax_debug)
			hexdump ("=>", cp, n);*/
		count += n;
		size -= n;
		cp += n;
	}
	return count;
}

/*  
 * basic io (packet level)
 */

int fax_writepacket (fd, fp)
Faxcomm *fp;
{
	char	*cp;
	int	len;
	char	buffer[FAXBUFSIZ];

	cp = buffer;
	*cp ++ = FAX_HEADER;
	*cp ++ = FAX_HEADER;
	*cp ++ = FAX_CONTROL;
	*cp ++ = FAX_ADDRESS;
	len = 1 + fp -> len;
	*cp ++ = len == 256 ? 0 : len;
	*cp ++ = fp -> command;
	if (fp -> len) {
		bcopy (fp -> data, cp, fp -> len);
		cp += fp -> len;
	}
	*cp = fax_checksum (buffer, cp - buffer);
	cp ++;

	if (writex (fd, buffer, cp - buffer) != cp - buffer)
		return NOTOK;
	return OK;
}

int	fax_readpacket (fd, fp)
int	fd;
Faxcomm *fp;
{
	char buffer[FAXBUFSIZ];
	unsigned int	len;

	if ((len = readx (fd, buffer, FAXHDRSIZE)) != FAXHDRSIZE) {
		advise ("read", "Read fax header size failed");
		return NOTOK;
	}
	if (buffer[0] != FAX_HEADER ||
	    buffer[1] != FAX_HEADER ||
	    buffer[2] != FAX_CONTROL ||
	    buffer[3] != FAX_ADDRESS) {
		advise (NULLCP, "Incorrect Fax header");
		return NOTOK;
	}
	len = (unsigned char) buffer[FAXHDRSIZE - 1];
	if (len == 0)
		len = 256;

#ifdef CHECKSUM
	len ++;
#endif
	if (readx (fd, &buffer[FAXHDRSIZE], len) != len) {
		advise ("read", "failed to read %d more bytes", len);
		return NOTOK;
	}
#ifdef CHECKSUM
	if ((tmp = fax_checksum (buffer, FAXHDRSIZE + len)) !=
	    buffer[FAXHDRSIZE + len - 1]) {
		advise (NULLCP, "Checksum failed %d != %d",
			buffer[FAXHDRSIZE + len - 1], tmp);
		return NOTOK;
	}
#endif
	fp -> flags = 0;
	fp -> command = buffer[FAXHDRSIZE];
	fp -> len = len - 1;
	if (fp -> len)
		bcopy (&buffer[FAXHDRSIZE+1], fp -> data, fp -> len);
	return OK;
}

/* 
 * checksum routine
 */

int fax_checksum (bp, len)
int	len;
char	*bp;
{
	char	*cp, *ep = bp + len;
	int sum = 0;

	for (cp = bp + 2; cp < ep; cp ++)
		sum += *cp;
	return 0x100 - (sum % 0x100);
}

/* misc */

static void hexdump (str, bp, n)
char	*str;
char	*bp;
int	n;
{
	static char hex[] = "0123456789ABCDEF";

	fputs (str, stdout);
	while (n -- > 0) {
		putchar (hex[(*bp >> 4) & 0xf]);
		putchar (hex[*bp & 0xf]);
		bp ++;
	}
	putchar ('\n');
}

int fax_simplecom (fd, type)
int     fd;
int     type;
{
        Faxcomm pack;

        pack.flags = 0;
        pack.command = type;
        pack.len = 0;

        return fax_writepacket (fd, &pack);
}
