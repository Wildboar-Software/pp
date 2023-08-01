/* font_util.c: utility routines for fonts */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/font_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/font_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: font_util.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */

#include	"fonts.h"

extern CharPtr	file2char();

PPFontPtr
new_font(nchars)
int	nchars;
{
	PPFontPtr ret = (PPFontPtr) malloc(sizeof(PPFont));
	ret->nchars = nchars;
	ret->chars = (CharPtr *) calloc(ret->nchars,
					sizeof(Char));
	return ret;
}

free_font(f)
PPFontPtr	f;
{
	int	i;
	for (i = 0; i < f->nchars; i++)
		if (f->chars[i])
			free_char(f->chars[i]);
	free((char *) f->chars);
	free((char *) f);
}

font2file(fp, f)
FILE	*fp;
PPFontPtr	f;
{
	int	i;

	fprintf(fp, "%d\n", f->nchars);
	for (i = 0; i < f->nchars; i++)
		char2file(fp, f->chars[i]);
}

PPFontPtr
file2font(fp)
FILE	*fp;
{
	PPFontPtr ret;
	char	buf[BUFSIZ];
	int	i;

	if (fgets(buf, BUFSIZ, fp) == NULLCP) {
                fprintf(stderr,
                       "Incorrect file encoding of font: unexpected EOF");
                return (PPFontPtr) NOTOK;
	}
	
	ret = (PPFontPtr) malloc(sizeof(PPFont));
	ret -> max_wid = ret -> max_ht = 0;
	ret -> nchars = (int)strtol(buf, NULL, 0);
	ret->chars = (CharPtr *) calloc(ret->nchars,
					sizeof(CharPtr));
	for (i = 0; i < ret->nchars; i++) {
		if ((ret->chars[i] = file2char(fp)) == (CharPtr) NOTOK) {
			ret->chars[i] = NULL;
			free_font(ret);
			return (PPFontPtr) NOTOK;
		}
		if (ret->chars[i] -> wid > ret -> max_wid)
			ret -> max_wid = ret -> chars[i] -> wid;
		if (ret->chars[i] -> ht > ret -> max_ht)
			ret -> max_ht = ret -> chars[i] -> ht;
	}
	return ret;
}	
