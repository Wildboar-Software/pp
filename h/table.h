/* table.h: table structures */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/table.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: table.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_TABLE
#define _H_TABLE


/*
Information kept for all tables
*/

typedef struct TableOverride {
	char *tbo_key;
	char *tbo_value;
	struct TableOverride *tbo_next;
} TableOverride;

typedef struct	tb_struct {
	char	*tb_name;	/* internal name of table		*/
	char	*tb_show;	/* displayable human-oriented string	*/
	char	*tb_file;	/* name of file containing table	*/
	FILE	*tb_fp;		/* stdio file pointer			*/
	int	tb_flags;	/* various bits (type of table, etc)	*/
	TableOverride *tb_override;
				/* overridden tables			*/
} Table;


#define NULLTBL	       ((Table *)0)

#define TB_SRCMASK	07
#define TB_SRC(x)	((x) & TB_SRCMASK)	/* Source of table data */
#define TB_DBM		01			/* Read from DBM database */
#define TB_NS		02			/* Read from Nameserver */
#define TB_LINEAR	04			/* Read from linear file */


extern	Table	*tb_nm2struct();
extern	Table	**tb_all;
extern	int	tb_getdomain ();
extern	int	tb_k2val ();
extern	int	tb_dbmk2val ();
extern	int	tab_fetch ();
#endif
