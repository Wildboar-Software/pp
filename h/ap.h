/* ap.h: RFC-822 address parser definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/ap.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: ap.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_AP
#define _H_AP


#include "util.h"


struct ap_node
{
    char    ap_obtype;                /* parsing type of this object */

#define AP_NIL                  0
#define AP_COMMENT              1     /* comment (...) */
#define AP_DATA_TYPE            2     /* "data-type" (e.g., :include:...,) */
#define AP_DOMAIN               3     /* host-part of address */
#define AP_DOMAIN_LITERAL       4     /* domain literal */
#define AP_GENERIC_WORD         5     /* generic word */
#define AP_GROUP_END            6     /* end of group list */
#define AP_GROUP_NAME           7     /* name of group */
#define AP_GROUP_START          8     /* start of group address list x:.. ; */
#define AP_MAILBOX              9     /* mailbox-part of address */
#define AP_PERSON_END           10    /* end of personal address list */
#define AP_PERSON_NAME          11    /* name of perrson */
#define AP_PERSON_START         12    /* start of personal addr list <...> */


    char    ap_ptrtype;               /* next node is continuation of this */
				      /* address, start of new, or null   */
#define AP_PTR_NIL              0     /* there is no next node */
#define AP_PTR_NXT              1     /* next is start of new address */
#define AP_PTR_MORE             2     /* next is part of this address */

    char    *ap_obvalue;              /* ptr to str value of object  */

/* new fields added for efficiency during parsing */
    int     ap_normalised;	      /* boolean indicate whether domain */
    				      /* has been normalised */
    int     ap_recognised;	      /* boolean indicate whether domain */
    				      /* was recognised by normalisation */
    int     ap_islocal;		      /* boolean indicate whether domain */
    				      /* is local */
    char    *ap_localhub;	      /* name of local hub */
    char    *ap_chankey;	      /* key to use in look up of chan table */
    char    *ap_error;		      /* error from look up */
    struct ap_node   *ap_next;        /* pointer to next node */
};



typedef struct  ap_node         *AP_ptr;
#define NULLAP                  ((AP_ptr)0)
#define NULLAPP                 ((AP_ptr *)0)
#define BADAP                   ((AP_ptr) -1)



/*
Environment for the address parser
*/

#define AP_TYPE_MASK		(~0003)

#define AP_PARSE_SAME           0000  /* do not transorm the address */
#define AP_PARSE_733            0001  /* follow RFC #733 rules */
#define AP_PARSE_822            0002  /* follow RFC #822 rules */
#define AP_PARSE_BIG            0010  /* Use Big-endian domains, FLAG */

/*
For use when getting indirect input
*/

struct ap_prevstruct
{
    FILE        *ap_curfp;            /* handle on current file input */
    int         (*ap_prvgfunc) ();    /* getchar function for that input */
    int         ap_opeek;             /* with this as peek-ahead char for it*/
    int         ap_ogroup;            /* nesting level of group list */
    int         ap_operson;           /* nesting level of personal list */
    struct ap_prevstruct   *ap_next;
};



extern  AP_ptr  ap_1delete();
extern  AP_ptr  ap_add();
extern  AP_ptr  ap_alloc();
extern  AP_ptr  ap_append();
extern  AP_ptr  ap_move();
extern  AP_ptr  ap_new();
extern  AP_ptr  ap_normalize();
extern  AP_ptr  ap_pcur;
extern  AP_ptr  ap_pinit();
extern  AP_ptr  ap_pstrt;
extern  AP_ptr  ap_s2t();
extern  AP_ptr  ap_sqdelete();
extern  AP_ptr  ap_sqinsert();
extern  AP_ptr  ap_sqmove();
extern  AP_ptr  ap_t2p();
extern  AP_ptr  ap_t2s();
extern  char    ap_llex;
extern  char*   ap_dmflip();
extern  char*   ap_p2s();
extern	char*	ap_p2s_nc();
extern  char*   ap_s2p();
extern  int     ap_1adr();
extern  int     ap_dmnormalize();
extern  int     ap_flget();
extern  int     ap_fpush();
extern  int     ap_lex();
extern  int     ap_ppush();
extern	void	ap_7to8 ();
extern	void	ap_ninit ();
extern	void	ap_palloc ();
extern	void	ap_pfill ();
extern	void	ap_clear ();
extern	void	ap_pappend ();
extern	void	ap_sqtfix ();
extern	void	ap_insert ();
extern	void	ap_fllnode ();
extern	void	ap_free ();
extern	void	ap_iinit ();
extern	void	ap_delete ();
extern	void 	ap_pnsrt ();
extern	void	ap_val2str ();
extern  void	ap_use_percent();
#endif
