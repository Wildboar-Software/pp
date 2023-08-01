/* conf.c: configuration information for body part filters */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/charset/RCS/charset_conf.c,v 6.0 1991/12/18 20:21:39 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/charset/RCS/charset_conf.c,v 6.0 1991/12/18 20:21:39 jpo Rel $
 *
 * $Log: charset_conf.c,v $
 * Revision 6.0  1991/12/18  20:21:39  jpo
 * Release 6.0
 *
 */



#ifndef FILE_CHARSETS
#define FILE_CHARSETS "CHARSETS"
#endif

#ifndef FILE_CHARDEFS
#define FILE_CHARDEFS "CHARDEFS"
#endif

#ifndef FILE_CHARMAP
#define FILE_CHARMAP "CHARMAP.10646"
#endif

#ifndef FILE_CHARMNEM
#define FILE_CHARMNEM "CHARMNEM"
#endif

#ifndef CSDIR
#define CSDIR "."
#endif


char	*charset_dir 	= CSDIR;
char	*charset_sets	= FILE_CHARSETS;
char	*charset_defs	= FILE_CHARDEFS;
char	*charset_map	= FILE_CHARMAP;
char	*charset_mnem	= FILE_CHARMNEM;
