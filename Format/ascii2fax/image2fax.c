/* image2fax.c: conversion routines from PP image to x400 fax */
/* code "borrowed" from QUIPU */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/image2fax.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/image2fax.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: image2fax.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */

#include	<stdio.h>
#include	"util.h"
#include	"IOB-types.h"
#include	"g3.h"
#include 	<varargs.h>
#include	"fonts.h"

# define BITSPERBYTE	8

void    advise ();
void    adios ();

#define ps_advise(ps, f) \
	advise (NULLCP, "%s: %s", (f), ps_error ((ps) -> ps_errno))

static char	*store = NULLCP;
static int	storeix = 0, store_allocd = 0;
static void	putwhitespan(), putblackspan(),
		putcode(), puteol(), putinit(),
		myputchar(), putbit();
static int shdata;
static int shbit = 0x80;

encodeImage(page, wd, ht, fax)
BitMap	page;
unsigned int	wd, ht;
struct type_IOB_G3FacsimileBodyPart	*fax;
{
	int		row;

	putinit();

	for(row = 0; row < ht; row++) {

		tofax(page,row,wd);
	
	}
       
	for( row = 0; row < 5; row++ )
		puteol( );
    
	flushbits(fax);

}

/*  */

/* encode a row of the image */

tofax(image,row,wd)
BitMap	image;
int	row;
unsigned int	wd;
{
    int c = 0;
    int	n = 0;

    while(n < wd) { 
	c = 0;
	while(!BL_ISSET(n, image[row]) && (n < wd)) {
	    c++;
	    n++;
	}
	putwhitespan(c);
	c = 0;
	if(n == wd)
	    break;
	while(BL_ISSET(n, image[row]) && (n < wd)) {
	    c++;
	    n++;
	}
	putblackspan(c);
    }
    puteol();
}

/*  */
/* wrap up encoded page in ASN1 */

flushbits(fax)
struct type_IOB_G3FacsimileBodyPart	*fax;
{
	struct type_IOB_G3FacsimileData	*new, *ix;
	
	if (shbit != 0x01 ){
		myputchar(shdata);
		shdata = 0;
		shbit = 0x01;
	}
	
	new = (struct type_IOB_G3FacsimileData *) 
		calloc (1, sizeof(struct type_IOB_G3FacsimileData));
	new -> element_IOB_0 = strb2bitstr (store,
					   (storeix * BITSPERBYTE),
					   PE_CLASS_UNIV,
					   PE_PRIM_BITS);
	if (fax -> data == NULL)
		fax -> data = new;
	else {
		for (ix = fax -> data; ix -> next != NULL; ix = ix -> next);
		ix -> next = new;
	}
	fax -> parameters -> number__of__pages++;
	storeix = 0;
}

/*  */


/*  */
/* encode and splurt out the fax */

outputFax (fax,fp)
struct type_IOB_G3FacsimileBodyPart	*fax;
FILE					*fp;
{
	PE	pe = NULLPE;
	PS	psout;

	encode_IOB_G3FacsimileBodyPart (&pe, 1, 0, NULLCP, fax);

	/* IMPLICIT TAG */
	pe->pe_class = PE_CLASS_CONT;
	pe->pe_id = 3;

	/* output */
	if ((psout = ps_alloc(std_open)) == NULLPS) {
		ps_advise (psout, "ps_alloc");
		exit(1);
	}

	if (std_setup (psout, fp) == NOTOK) {
		advise (NULLCP, "std_setup loses");
		exit (1);
	}

	if (pe2ps(psout, pe) == NOTOK) {
		ps_advise(psout, "pe2ps loses");
		exit(1);
	}

	ps_free(psout);

	if (pe != NULLPE) pe_free (pe);
}

/*  */
/* basic output routines for fax bits */

static void putwhitespan(c)
int c;
{
    int tpos;
    tableentry *te;

    if(c>=64) {
	tpos = (c/64)-1;
	te = mwtable+tpos;
	c -= te->count;
	putcode(te);
    }
    tpos = c;
    te = twtable+tpos;
    putcode(te);
}

static void putblackspan(c)
int c;
{
    int tpos;
    tableentry *te;

    if(c>=64) {
	tpos = (c/64)-1;
	te = mbtable+tpos;
	c -= te->count;
	putcode(te);
    }
    tpos = c;
    te = tbtable+tpos;
    putcode(te);
}

static void putcode(te)
tableentry *te;
{
    unsigned int mask;
    int code;

    mask = 1<<(te->length-1);
    code = te->code;
    while(mask) {
 	if(code&mask)
	    putbit(1);
	else
	    putbit(0);
	mask >>= 1;
    }

}

static void puteol()
{
    int i;

    for(i=0; i<11; i++)
	putbit(0);
    putbit(1);
}

static void putinit()
{
    shdata = 0;
    shbit = 0x01;
    storeix = 0;
    puteol();
}

#define	INC	512

extern unsigned char flip_bits();

static void myputchar (ch)
char	ch;
{
	if (storeix == store_allocd) {
		/* resize */
		store_allocd += INC;
		if (store != NULLCP) 
			store = realloc (store, 
					 (unsigned) (store_allocd * sizeof (char)));
		else
			store = calloc ((unsigned) store_allocd,
					sizeof (char *));
	}
	store[storeix++] = flip_bits((unsigned char) ch);
}

static void putbit(d)
int d;
{
    if(d) 
	shdata = shdata|shbit;
    shbit = shbit<<1;

    if((shbit&0xff) == 0) {
	myputchar(shdata);
	shdata = 0;
	shbit =  0x01;
    }
}

/*    ERRORS */

#ifndef lint
void    adios (va_alist)
va_dcl
{
	va_list ap;

	va_start (ap);

	_ll_log (pp_log_norm, LLOG_FATAL, ap);

	va_end (ap);

	_exit (1);
}
#else
/* VARARGS2 */

void    adios (what, fmt)
char   *what,
       *fmt;
{
	adios (what, fmt);
}
#endif


#ifndef lint
void    advise (va_alist)
va_dcl
{
	int     code;
	va_list ap;

	va_start (ap);

	code = va_arg (ap, int);

	_ll_log (pp_log_norm, code, ap);

	va_end (ap);
}
#else
/* VARARGS3 */

void    advise (code, what, fmt)
char   *what,
       *fmt;
int     code;
{
	advise (code, what, fmt);
}
#endif
