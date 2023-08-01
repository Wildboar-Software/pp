/* str_util.c: utility routines for manipulating strings of chars inconjunction with bitmaps */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/str_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/str_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: str_util.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */

#include	"fonts.h"

static int numTabs(s)
char    *s;
{
        int     ret = 0;
        for (;s && *s != '\0';s++) 
                if (*s == '\t')
                        ret++;
        return ret;
}

StrPtr new_str(f, s, l)
PPFontPtr	f;
char		*s;
int		l;
{
	int i, j, k, tabs;
	StrPtr	ret = (StrPtr) malloc(sizeof(Str));

	ret->orig = strdup(s);
	tabs = numTabs(s);
	ret->nchars = l + (6 * tabs); /* 6 because l contains number of tabs */

	ret->chars = (CharPtr *) calloc(ret->nchars,
					sizeof(CharPtr));
	for (i = 0, j = 0;i < l; i++) {
		if (tabs > 0 &&
		    s[i] == '\t') {
			tabs--;
			for (k = 0; k < 7; k ++)
				ret->chars[j++] = f->chars[32]; /* space */
		} else
			ret->chars[j++] = f->chars[(int)s[i]];
	}
	return ret;
}

free_str(s)
StrPtr	s;
{
	if (s->chars != NULL)
		free((char *) s->chars);
	if (s->orig)
		free(s->orig);
	free(s);
}

static int longestFit(s, maxwid, pw, ph)
StrPtr	s;
int	maxwid, *pw, *ph;
{
	int	tallest = 0, count = 0;
	int	ix;
	*pw = 0;
	*ph = 0;

	for (ix = 0; ix < s->nchars; ix++) {
                if (s->chars[ix] && count + s->chars[ix]->wid <= maxwid) {
                        count += s->chars[ix]->wid;
                        if (s->chars[ix]->ht > tallest)
                                tallest = s->chars[ix]->ht;
                } else {
                        ix--;
                        break;
                }
        }
        *ph = tallest;
        *pw = count;
        return ix;
}

static int insertcharbitline(into, x, ch, y)
BitLine	into;
int	x;
CharPtr	ch;
int	y;
{
	int	i;
	for (i = 0; i < ch->wid;i++) {
		if (BL_ISSET(i, ch->bits[y]))
			BL_SET(x+i, into);
		else
			BL_CLR(x+i, into);
	}
	return x+i;
}
		
static void insert_1yline(into, x, s, y, num)
BitLine	into;
int	x;
StrPtr	s;
int	y, num;
{
	int	i;

	for (i = 0; i < num; i++)
		x = insertcharbitline(into, x, s->chars[i], y);
}

static int insert_str(bm, s, x, y, dx, dy, widest)
BitMap		bm;
StrPtr		s;
int		x, y, *dx, *dy, widest;
{
	int	wid, ht, numInserted, iy;
	
	numInserted = longestFit(s, widest - x, &wid, &ht);
	
	for (iy = 0; iy < ht; iy++)
		insert_1yline(bm[y+iy], x, s, iy, numInserted);
	*dy = ht;
	*dx = wid;
	return numInserted;
}

int str_into_bitmap(bm, f, x, y, dx, dy, widest, str, len)
BitMap		bm;
PPFontPtr	f;
int		x, y, *dx, *dy, widest;
char		*str;
int		len;
{
	int	numInserted;
	StrPtr	s = new_str(f, str, len);
	numInserted = insert_str(bm, s, x, y, dx, dy, widest);
	free_str(s);
	return numInserted;
}


      
