/* alias.h: alias structure definition (man page ALIASES(5)) */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/alias.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: alias.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_ALIAS
#define _H_ALIAS

#define		ALIAS_EXTERNAL		3

typedef struct  alias_struct {
	int             alias_type;
#define         ALIAS_SYNONYM           1
#define         ALIAS_PROPER            2
	char            alias_user[LINESIZE];
	int		alias_external;
	int		alias_ad_type;
} ALIAS;


#define NULLALIAS                       ((ALIAS *)0)




#endif
