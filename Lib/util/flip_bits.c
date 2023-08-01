/* flip_bits: reverse bits in char */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/flip_bits.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/flip_bits.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: flip_bits.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */


static unsigned char	table[] = {
/* 0x00 */	0x00,
/* 0x01 */	0x08,
/* 0x02 */	0x04,
/* 0x03 */	0x0c,
/* 0x04 */ 	0x02,
/* 0x05 */	0x0a,
/* 0x06 */	0x06,
/* 0x07 */	0x0e,
/* 0x08 */	0x01,
/* 0x09 */	0x09,
/* 0x0a */	0x05,
/* 0x0b */	0x0d,
/* 0x0c */	0x03,
/* 0x0d */	0x0b,
/* 0x0e */	0x07,
/* 0x0f */	0x0f,
	};

unsigned char flip_bits(ch)
unsigned char	ch;
{
	unsigned char	ret;
	ret = table[(ch >> 4) & 0x0f];
	ret |= table[ch & 0x0f] << 4;
	return ret;
}
