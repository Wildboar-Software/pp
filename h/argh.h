/* argh.h: Used by the mu program */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/argh.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: argh.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_ARGH
#define _H_ARGH



#define ARGHVER         "argh v. 1.1"   /* Version number */
#define MAXARGS         100     /* Maximum argument instances provided for */
#define MAXFLAGS        50      /* Maximum number of flags
				   needed to size sorting array argsort[] */
#define FLAGCHAR        '-'     /* introduces a flag on command line */
#define EQUCHAR         '='     /* to separate a partial but
				   unambiguous flag from its value */
#define MAXFLAGLEN      30                      /* max flag length */
#define VALIDTYPES      "0123456789amnz?"       /* allowed flag types */
#define ADJARGS         TRUE                    /* allow adjacent args */
#define ONECHARARGS     FALSE                   /* allow unambiguous one-byte flags without EQUCHAR */
#define ZEROCHECK       FALSE                   /* test type '0' against next arg */

int     flagcount;      /* Count from calling-program-specific table */
int     argcpi;         /* Current parameter index */
int     argsource;      /* Current argument source */

void    add_link();
void    take_next();
void    arg_error();
void    show_args();
void    call_args();
void    list_args();
void    quick_string();
void    qs_string();

int     scan_args();
int     prep_args();
int     count_to_flag();

char    *next_arg();
char    *nextc_arg();
char    *digit_word();

struct  arginst {               /* instance of one argument type */
	char    *argcp;         /* character pointer */
	int     argci;          /* argument counter e.g. argv[ai->argci] */
	int     argoi;          /* offset e.g. argv[ai->argci][ai->argoi] */
	int     argn;           /* instance in this run */
	int     argof;          /* ... of total instances in this run */
	int     argsrc;         /* source - i.e. which call to scan_args */
	int     argused;        /* used marker TRUE -> has been used */
	struct  arginst *next;  /* linked list */
};

struct  argdef {                /* user supplied flag list */
	char    *flag;          /* see below */
	void    (*function)();
	char    type;
	int     match;
};

struct  argfull {               /* argument ( flag ) definitions */

/*      char    flag[MAXFLAGLEN];       character(s) to follow '-' */
	char    *flag;          /* character(s) to follow '-' */

	void    (*function)();  /* to hold pointer to function */

	char    type;           /*      a : valued : adjacent to flag
					n : valued : next argument
					0 : lone ( no assoc. value )
					1 : valued : adjacent or next
					2 : as v but takes 2 args
					3 : as v but takes 3 args
					... etc. (up to 9)
					m : 1 or more ( up to next flag )
					z : as m but zero args allowed */

	int     match;          /* Minimum length for unambiguous
				   matching. These can be calculated
				  >0 indicates length to use
				   0 indicates calculation reqd.
				  -1 indicates exact match reqd.
				   If matching allowed, type a is not.
				   nor v(adjacent) less than
				   strlen(flag) unless EQUCHAR used */

	int     length;         /* length of flag */
	int     count;                  /* how many instances */
	struct  arginst *first,*last;   /* pointers to first, last instance */
};


#endif
