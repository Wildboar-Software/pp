/* strcnv.c 1.4 901228 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/charset/RCS/strcnv.c,v 6.0 1991/12/18 20:21:39 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/charset/RCS/strcnv.c,v 6.0 1991/12/18 20:21:39 jpo Rel $
 *
 * $Log: strcnv.c,v $
 * Revision 6.0  1991/12/18  20:21:39  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	<fcntl.h>
#include	<ctype.h>
#include	"charset.h"


extern char	*charset_dir, *charset_mnem;

static IN_CH	*chset = NULL;
static CHARSET	*charsets = NULL;



/* -- local routines -- */
static CHAR8U	*getoutch();
static INT16S	*getinch();
static int	findc();
static int	getchbas();
static int	openfile();
extern off_t lseek ();

static int	strcnv_mnemonics(), strncnv_mnemonics(),
	strncnv_x408();



/* -------------------- Begin  Routines  ------------------------------------ */




int strcnv (r_chset, s_chset, result, source, MnemonicsRequired)
CHARSET	*r_chset;
CHARSET *s_chset;
CHAR8U	*result;
CHAR8U	*source;
int	MnemonicsRequired;
{
	switch (MnemonicsRequired) {
	case TRUE:
		return (strcnv_mnemonics (r_chset, s_chset, result, source));
	default:
		return (-1);
	}
}




int strncnv (r_chset, s_chset, result, source, n, MnemonicsRequired)
CHARSET *r_chset;
CHARSET *s_chset;
CHAR8U	*result;
CHAR8U	*source;
int	n;
int	MnemonicsRequired;
{
	switch (MnemonicsRequired) {
	case TRUE:
		return (strncnv_mnemonics 
				(r_chset, s_chset, result, source, n));
	default:
		return (strncnv_x408 
				(r_chset, s_chset, result, source, n));
	}
}




CHARSET *getchset (s, esc)
char	*s;
INT16S	esc;
{
	register CHARSET	*c, *c1;
	register char	*p;

	for (p = s; *p; p++) 
		if (isascii (*p) && islower (*p)) 
			*p = toupper (*p);
	if (getchbas() == 0) 
		return NULL;
	for (c = charsets; c && strcmp (c->name, s); c = c->next) 
		c1 = c;
	if (!c) {
		c = (CHARSET * ) malloc (sizeof (CHARSET));
		if (!c) 
			return NULL;
		c->next = NULL;
		c->out = getoutch (s, (CHAR8U)0);
		if (c->out) {
			c->name = (char *) strdup (s);
			c->in = getinch (s, (CHAR8U)0);
			c->esc = esc;
		} else { 
			free ((char *)c); 
			c = NULL; 
		}
		if (charsets) 
			c1->next = c; 
		else 
			charsets = c;
	}
	return c;
}




/* -------------------------  Static  Routines  ---------------------------- */




static int strcnv_mnemonics (r_chset, s_chset, result, source)
CHARSET	*r_chset;
CHARSET *s_chset;
CHAR8U	*result;
CHAR8U	*source;
{
	register CHAR8U		*p, *q;
	register CHAR8U		*out;
	register INT16S		*in;
	register int		o;  /* intermediate binary value */
	register int		c;
	register INT16S		esc, esco, mnem;

	p = result; 
	q = source;
	out = r_chset->out; 
	in = s_chset->in;
	esco = r_chset->esc; 
	esc = s_chset->esc;

	while (c = *q++) {
		/* printf (" %c %c %c",c,*q,esc);  */
		if (c == esc) {
			/* two esc in a row -> one escape */
			/* if esc followed by defined mnemonic
			   next char is mnemonic. */

			if (*q == esc) { 
				o = in[esc]; 
				q++; 
			} else { 
				o = findc (q[0] * C256 + q[1]); 
				q += 2;
			}
		} else 
			o = in[c];
		if (!o) 
			o = LOW_LINE; /* if illegal input replace with '_' */

		/* printf (" %d %d %c",o,esco,esco);  */
		if (out[o] == esco) { 
			*p++ = esco; 
			*p++ = esco; 
		} else if (out[o]) 
			*p++ = out[o];
		else {
			mnem = chset[o];
			*p++ = esco;
			*p++ = out [in [mnem / C256 ]];
			*p++ = out [in [mnem % C256 ]];
		}
	}
	*p = '\0';
	return p - result;
}




static int strncnv_mnemonics (r_chset, s_chset, result, source, n)
CHARSET *r_chset;
CHARSET *s_chset;
CHAR8U	*result, *source;
int	n;
{
	register CHAR8U		*p, *q, *e;
	register CHAR8U		*out;
	register INT16S		*in;
	register int		o;  /* intermediate binary value */
	register int		c;
	register INT16S		esc, esco, mnem;

	p = result; 
	q = source;
	e = p + n - 4; /* 4 chars for ending: esc two-char nul */
	out = r_chset->out; 
	in = s_chset->in;
	esco = r_chset->esc; 
	esc = s_chset->esc;

	while ((c = *q++) && p < e) {
		/* printf (" %c %c %c",c,*q,esc);   */
		if (c == esc) {
			/* two esc in a row -> one escape */
			/* if esc followed by defined mnemonic
			   next char is mnemonic. */

			if (*q == esc) { 
				o = in[esc]; 
				q++; 
			} else { 
				o = findc (q[0] * C256 + q[1]); 
				q += 2;
			}
		} else 
			o = in[c];
		if (!o) 
			o = LOW_LINE; /* if illegal input replace with '_' */

		/* printf (" %d %d %c",o,esco,esco);  */
		if (out[o] == esco) { 
			*p++ = esco; 
			*p++ = esco; 
		} else if (out[o]) 
			*p++ = out[o];
		else {
			mnem = chset[o];
			*p++ = esco;
			*p++ = out [in [mnem / C256 ]];
			*p++ = out [in [mnem % C256 ]];
		}
	}
	*p = '\0';
	return p - result;
}




static int strncnv_x408 (r_chset, s_chset, result, source, n)
CHARSET	*r_chset;
CHARSET	*s_chset;
CHAR8U	*result, *source;
int	n;
{
	register CHAR8U 	*p, *q, *e;
	register CHAR8U		*out;
	register INT16S		*in;
	register int		o;  /* intermediate binary value */
	register int		c;

	p = result;
	q = source;
	e = q + n;
	out = r_chset->out; 
	in = s_chset->in;

	/* --- *** ---
	1) No mnemonics are used for unknown symbols.
		They are just replaced with '?'. Provided they are NOT 
		accented letters or umlauts. 
	2) Accented letters or umlauts are 2 bit combinations: 
		the first is a a mark, the second a basic Latin character. 
		In this case only the character is printed.
	--- *** --- */

	for (c = *q++; q <= e; c = *q++) {
		o = in[c];
		if (!o)
			o = QUESTION_MARK;
			/* if illegal input replace with '?' */

		if (out[o]) 
			*p++ = out[o];
		else if (c >= 192 && c <= 207) /* an accent ? */
			if ((q[0] >= 'A' && q[0] <= 'Z') ||
			    (q[0] >= 'a' && q[0] <= 'z'))
				continue;	
			else
				*p++ = out[QUESTION_MARK];
		else
			*p++ = out[QUESTION_MARK];
	}

	*p = '\0';
	return p - result;
}




static int findc (c)
register unsigned int	c; /* c is ascii value of two-byte char mnem */
{
	register INT16S *cs;
	register int	i;

	i = chset[0] + 1;
	cs = &chset[i];
	while (--i > 0 && c != *--cs)
		;

	/* printf (" %d ",i); */
	return i;  /* zero == not found */
}




static int openfile (dir, file, mode)
char	*dir, *file;
int	mode;
{
	char	fn[LSIZE];
	register char	*p, *q;
	register int	f;

	(void) strcpy (fn, dir);
	(void) strcat (fn, "/");

	q = fn + strlen (fn);
	for (p = file; *p; p++)
		*q++ = (isascii (*p) && islower (*p))
		 ? toupper (*p) : *p;
	*q = 0;

	f = open (fn, mode);
	if (f < 0) 
		(void) fprintf (stderr, "\n\n*** Error: opening file %s\n", fn);
	return f;
}




static int getchbas()
{
	INT16S		sz[2];
	register int	f;
	unsigned	sz1;

	if (!chset) {
		f = openfile (charset_dir, charset_mnem, O_RDONLY);
		if (f < 0) 
			return NULL;
		if (read (f, (char *)sz, sizeof (INT16S)) != sizeof (INT16S)) 
			goto err;
		sz1 = sz[0] * sizeof (INT16S);
		(void) lseek (f, 0L, 0);
		if ((chset = (IN_CH * ) malloc (sz1)) == NULL) 
			goto err;
		if (read (f, (char *)chset, (int)sz1) != sz1) 
			goto err;
		/* printf ("charsz %d chars %22.22s\n",sz1,chset); */
		(void) close (f);
	}
	return 1;

err:
	(void) close (f);
	return NULL;
}




static INT16S *getinch (charset, c)
char	*charset;
CHAR8U	c;
{
	int	f, sz;
	IN_CH	 * in;

	sz = C256 * sizeof (IN_CH);
	in = (IN_CH * ) malloc ((unsigned)sz);
	if (!in) 
		return NULL;
	f = openfile (charset_dir, charset, O_RDONLY);
	if (f < 0) 
		return NULL;
	if (read (f, (char *)in, sz) != sz) { 
		(void) close (f); 
		return NULL; 
	}
	(void) close (f);
	in[0] = c;
	/*  printf (" %c %d ",c,in[0]);	 */
	return in;
}




static CHAR8U *getoutch (charset, c)
char	*charset;
CHAR8U	c;
{
	int	f;
	unsigned	sz1;
	OUT_CH		 * out;

	if (getchbas() == 0) 
		return NULL;
	sz1 = chset[0];
	f = openfile (charset_dir, charset, O_RDONLY);
	if (f < 0) 
		return NULL;
	(void) lseek (f, C256L * sizeof (INT16S), 0);
	out = (OUT_CH * ) malloc (sz1);
	if (!out) 
		goto err;
	if (read (f, (char *)out, (int)sz1) != sz1) 
		goto err;
	(void) close (f);
	out[0] = c;
	return out;
err:
	(void) close (f);
	return NULL;
}
