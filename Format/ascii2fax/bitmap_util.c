/* bitmap_util.c: utility routines for a bitmap */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/bitmap_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/bitmap_util.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: bitmap_util.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */
#include	"fonts.h"

BitMap
new_bitmap(wid, ht)
int	wid, ht;
{
	BitMap	ret = (BitMap) calloc(ht, sizeof (BitLine));
	int	i;
	for (i = 0; i < ht; i++)
		ret[i] = (BitLine) calloc(BL_WD2INT(wid), sizeof(int));
	return ret;
}

clr_bitmap(map, wid, ht)
BitMap  map;
int     wid, ht;
{
        int     i;
        for (i =0;i < ht;i++)
                bzero((char *)map[i], BL_WD2INT(wid) * sizeof(int));
}

free_bitmap(b, w, h)
BitMap	b;
int	w, h;
{
	int	i;
	for (i = 0; i < h; i++)
		free(b[i]);
	free(b);
}

bitmap2file(fp, b, w, h)
FILE	*fp;
BitMap	*b;
int	w, h;
{
	int	iy, ix, count = 0;
	fprintf(fp, "%d %d\n", w, h);
	for (iy = 0; iy < h; iy++) 
		for (ix = 0; ix < BL_WD2INT(w); ix ++) {
			fprintf(fp, "0x%8.8x\n", b[iy][ix]);
#ifdef NOTDEF
			if (++count >= 10) {
				fprintf(fp, "\n");
				count = 0;
			}
#endif
		}
#ifdef NOTDEF
	if (w || h)
		fprintf(fp, "\n");
#endif
}	

BitMap
file2bitmap(fp, pw, ph)
FILE	*fp;
int	*pw, *ph;
{
	char	buf[BUFSIZ], *argv[15];
	int	argc, ia, iw, ih;
	long	temp;
	BitMap	ret;

	if (fgets(buf, BUFSIZ, fp) == NULLCP) {
		fprintf(stderr,
			"Incorrect file encoding of bitmap: unexpected EOF\n");
		return (BitMap) NOTOK;
	}
	
	if ((argc = sstr2arg(buf, 15, argv, " ")) != 2) {
		fprintf(stderr,
			"Incorrect file encoding of bitmap: expecting 'width height'\n");
		return (BitMap) NOTOK;
	}
	
	*pw = (int) strtol(argv[0], NULL, 0);
	*ph = (int) strtol(argv[1], NULL, 0);
	
	ret = new_bitmap(*pw, *ph);
	
	ia = argc = 0;
	for (ih = 0; ih < *ph; ih ++) 
		for (iw = 0; iw < BL_WD2INT(*pw); iw ++) {
			if (ia == argc) {
				/* read in another line */
				if (fgets(buf, BUFSIZ, fp) == NULLCP) {
					fprintf(stderr,
						"Incorrect file encoding of bitmap: unexpected EOF\n");
					return (BitMap) NOTOK;
				}
	
				argc = sstr2arg(buf, 15, argv, " ");
				ia = 0;
			}
			temp = strtol(argv[ia++], NULL, 0);
			
			ret[ih][iw] = (int) temp;
		}
	return ret;
}

	
int
insertbitmap(to, to_wid, to_ht,
	     from, from_wid, from_ht,
	     x, y)
BitMap	to;
int	to_wid, to_ht;
BitMap	from;
int	from_wid, from_ht,
	x, y;
{
	int	ix, iy;
	if (from_wid + x > to_wid) {
		fprintf(stderr,
		       "insertbitmap: bitmap too wide to insert");
		return NOTOK;
	}
	if (from_ht + y > to_ht) {
		fprintf(stderr,
		       "insertbitmap: bitmap too tall to insert");
		return NOTOK;
	}

	for (iy = 0; iy < from_ht && y+iy < to_ht; iy++) {
		for (ix = 0; ix < from_wid && x+ix < to_wid; ix++) {
			if (BL_ISSET(ix, from[iy]))
				BL_SET(x+ix, to[y+iy]);
			else
				BL_CLR(x+ix, to[y+iy]);
		}
	}
	return OK;
}
