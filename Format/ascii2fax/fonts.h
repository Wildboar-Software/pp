/* fonts.h: local representation of font */

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/fonts.h,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: fonts.h,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 *
 */
#include	"util.h"
#include	"sys.file.h"

typedef int *BitLine;
typedef	BitLine *BitMap;

#define	BITSPERCHAR	8
#define	BITSPERINT	16
#define MAXCHARPERINT	8

/* nicked from FD_SET etc in sys/types.h */
#define BL_SET(n, l)	((l)[(n)/BITSPERINT] |= (1 << ((n) % BITSPERINT)))
#define BL_CLR(n, l)	((l)[(n)/BITSPERINT] &= ~(1 << ((n) % BITSPERINT)))
#define BL_ISSET(n, l)	((l)[(n)/BITSPERINT] & (1 << ((n) % BITSPERINT)))

#define BL_WD2CHAR(w)	((w)/BITSPERCHAR + 1)
#define BL_WD2INT(w)	((w)/BITSPERINT + 1)

typedef struct _char {
	int	ascii;
	int	wid;	/* in pixels */
	int	ht;	/* in pixels */
	BitMap	bits;	
} Char, *CharPtr;

typedef struct _font {
	int	max_wid, max_ht;
	int	nchars;
	CharPtr	*chars;	/* [nchars] indexed by ascii value */
} PPFont, *PPFontPtr;

typedef struct _str {
	int	nchars;
	CharPtr	*chars;
	char	*orig;
} Str, *StrPtr;
