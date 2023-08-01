/* dexnet_conv.c: convert g3 into Fujitsu */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/dexnet_conv.c,v 6.0 1991/12/18 20:09:42 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/dexnet_conv.c,v 6.0 1991/12/18 20:09:42 jpo Rel $
 *
 * $Log: dexnet_conv.c,v $
 * Revision 6.0  1991/12/18  20:09:42  jpo
 * Release 6.0
 *
 */

/* fujicnvrt.c 	-- NASA Ames Research Center. 

		Author:		Jim Knowles
		Internet:	jknowles@trident.arc.nasa.gov
		Sterling Federal Systems
		NAS2-13210
		NASA, Ames Research Center
		Mail Stop 233-18
		Moffett Field CA 94035


 Purpose:	encapsulates ASN.1 g3-encoded scanlines in FCRP -
		Fujitsu proprietary command/response protocol.
		resulting file can be transmitted by Fujitsu fax modem.
		FCRP begins each scanline with ESC + and replaces G3 EOL
		with NUL NUL after padding with zeros to byte-align.
		this module is used by the FaxMail package.

 Input:		ASN.1 G3 encoded fax file received as faxmail enclosure.

 Output:	FCRP encapsulated G3 image file for transmittal.

 Errors:

 External references:
	name		description			source
	----		-----------			------

 Files:
	name		description
	----		-----------
			

 Environment:	4.3 BSD UNIX

 Development History:
	date		initials	description
	Apr  91		jdk		code developer.
*/


#include <fcntl.h>
#include <stdio.h>
#ifdef sun
#include <unistd.h>
#else
#ifdef vax
#include <unistd.h>
#else
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
#endif
#include <sys/types.h>
#include <sys/file.h>
#include <sysexits.h>
#include <errno.h>

#define	MASK		0x80		/* initial mask for bit manipulation */
#define BUF_SIZE	4096		/* buffer size */

#define TRUE            1               /* booleans */
#define FALSE           0
#define BOOL            int
#define NUL             '\000'          /* nul char */
#define ESC             '\033'          /* esc char */
#define PLUS            '\053'          /* + char */
#define FF		'\014'		/* form feed */


/* forward declarations */
static void output();
static void set_bit();
static void clr_bit();

/* global variables */
static int fu_fd;				/* file descriptor for FCRP file */
static int write_cnt;				/* characters written to write_buf */	
static unsigned char g3_ch;			/* character in production */
static unsigned char write_mask;		/* masked bit in char written */
static unsigned char save_ch;			/* last character written */
static unsigned char *buf_ptr;			/* pointer into write_buf */

/*
 *	Convert each scanline to have escape + as prefix and NUL NUL as suffix
 *	Return number of bytes placed into sink
 */
fujicnvrt(source, sink, length)
unsigned char *source, *sink;
int			length;	/* of source; sink assumed to be big enough */
{
	register unsigned char bit_mask;/* masks bit in currently read byte */
	register unsigned char *ptr;	/* pointer into read buffer */
	register unsigned char *eptr;	/* pointer to end of read buffer */
	register int zero_bits = 0;	/* number of consecutive zero bits */
	BOOL past_init = FALSE;		/* past the first eol sequence */
	BOOL new_page = FALSE;		/* start on new page */
	int	scanlines = 0;

	write_cnt = 0;				/* initialize for output */
	write_mask = MASK;
	buf_ptr = sink;				/* set start of write buffer */
	zero_bits = 8;				/* initial NUL (part of eol) */
	if (new_page) {
		g3_ch = FF;
		output(g3_ch);
	}
	output(ESC);
	output(PLUS);
	eptr = source + length;
	for (ptr = source; ptr < eptr; ptr++) {
		for (bit_mask = MASK; bit_mask > 0;) {
			if (*ptr & bit_mask) {
				if (zero_bits >= 11) {
					zero_bits = 0;
					write_mask = MASK;
					if (past_init) {
						g3_ch ^= g3_ch;
						if (save_ch != g3_ch)
							output(g3_ch);
						output(g3_ch);
						output(ESC);
						output(PLUS);
						scanlines++;
					} else {
						past_init = TRUE;
						g3_ch ^= g3_ch;
					}
					if (bit_mask == 0) {
						bit_mask = MASK;
						if (++ptr == eptr)
							break;
					}
				} else {
					set_bit();
					zero_bits = 0;
				}
			} else {
				if (past_init)
					clr_bit();
				zero_bits++;
			}
			bit_mask >>= 1;
		}
	}
	while (write_mask != MASK)	/* write character in production */
		clr_bit();
	if (g3_ch) {
		g3_ch ^= g3_ch;
		output(g3_ch);
	}
	output(g3_ch);
/* 	fprintf(stderr, "%d scanlines\n", scanlines); */
	return(write_cnt);
}



/*++ output()
 * function:	output one character into write buffer
 * parameters:	o_ch - unsigned character to write into buffer.
 * return:	none.
 * global:	buf_ptr - pointer into write buffer.
 *		save_ch - most recent character written.
 *		write_cnt - number of characters written into write buffer.
 *		write_buf - output buffer.
 * called by:	main(), set_bit(), clear_bit().
 * calls:	none.
 --*/

static void
output(o_ch)
register unsigned char o_ch;
{
	*buf_ptr++ = save_ch = o_ch;
	write_cnt++;
}


/*++ set_bit()
 * function:	sets bit in current byte;
 *		outputs to write buffer when byte is full.
 * parameters:	none.
 * return:	none.
 * global:	write_mask - masks current bit in g3_ch.
 *		g3_ch - character currently in production.
 * called by:	main().
 * calls:	output().
 --*/

static void
set_bit()
{
	g3_ch |= write_mask;
	write_mask >>= 1;
	if (write_mask == 0) {
		output(g3_ch);
		write_mask = MASK;
	}
}


/*++ clr_bit()
 * function:	clears bit in current byte;
 *		outputs to write buffer when byte is full.
 * parameters:	none.
 * return:	none.
 * global:	write_mask - masks current bit in g3_ch.
 *		g3_ch - character currently in production.
 * called by:	main().
 * calls:	output().
 --*/

static void
clr_bit()
{
	g3_ch &= ~write_mask;
	write_mask >>= 1;
	if (write_mask == 0) {
		output(g3_ch);
		write_mask = MASK;
	}
}
