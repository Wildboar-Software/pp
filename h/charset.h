/* charset.h */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/charset.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: charset.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */


#ifndef _H_CHARSET
#define _H_CHARSET


/* -***-
The character set conversion filter is written by Keld Simonsen (keld@dkuug.dk).
Please refer to the PP documentation Volume 1 on more information 
about its modification.
-***- */



/* --- the following 3 parameters may need modifying --- */
typedef unsigned char	CHAR8U;	/* must be able to hold values 0-255 */
typedef short int	INT16S;	/* Must be able to hold 16 bits signed int */
#define DEFAULT_ESCAPE	29	/* ASCII GS */


#define LOW_LINE	64	/* LOW LINE  '_' */ 
				/* if menmonics are required */
#define QUESTION_MARK	32	/* QUESTION MARK '?' */
				/* if X.408 conv is required */

#ifndef TRUE
#define TRUE		1
#endif

#ifndef FLASE
#define FALSE		0
#endif


#define IN_CH		INT16S
#define OUT_CH		CHAR8U


typedef struct charset	CHARSET;
#define C256L		256L
#define C256		256	/* max val of any char encoded as one byte */
#define LSIZE		256	/* max byte chars on a line, in a file name */


struct charset
{
	CHARSET		*next;
	INT16S		ecma;
	char		*name;
	IN_CH		*in;
	OUT_CH		*out;
	INT16S		esc;
};


extern	CHARSET		*getchset();

#endif
