/* expand.h: macro expansion structure */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/expand.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: expand.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_EXPAND
#define _H_EXPAND

typedef struct Expand {
	char	*macro;
	char	*expansion;
} Expand;

char	*expand ();
char	*expand_dyn ();
#endif
