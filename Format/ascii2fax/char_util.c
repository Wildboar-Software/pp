/* char_util.c: utility routines for chars */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/char_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/char_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: char_util.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */

#include 	"fonts.h"

extern BitMap	file2bitmap();

CharPtr
new_char()
{
	return (CharPtr) calloc(1, sizeof(Char));
}

free_char(c)
CharPtr	c;
{
	if (c -> bits)
		free_bitmap(c->bits, c->wid, c->ht);
	free((char *)c);
}

char2file(fp, c)
FILE	*fp;
CharPtr	c;
{
	fprintf(fp, "%d\n", c->ascii);
	bitmap2file(fp, c->bits, c->wid, c->ht);
}	

CharPtr
file2char(fp)
FILE	*fp;
{
	char	buf[BUFSIZ];
	CharPtr	ret;

	if (fgets(buf, BUFSIZ, fp) == NULLCP) {
		fprintf(stderr,
			"Incorrect file encoding of bitmap: unexpected EOF\n");
		return (CharPtr) NOTOK;
	}
	
	ret = new_char();
	ret->ascii = (int) strtol(buf, NULL, 0);
	if ((ret->bits = file2bitmap(fp, &(ret->wid), &(ret->ht))) == (BitMap) NOTOK) {
		ret->bits = NULL;
		free_char(ret);
		return (CharPtr) NOTOK;
	}
	return ret;
}

