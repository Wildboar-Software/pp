/* tb_nm2struct: convert a table name to a struct pointer */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_nm2struct.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_nm2struct.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_nm2struct.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "table.h"


Table   *
tb_nm2struct(name)
char    *name;
{
	register Table  **tbl;

	if (tb_all == (Table **)0)
		return( (Table *)0);

	for(tbl = tb_all ; *tbl != (Table *)0 ; tbl++)
		if(lexequ(name, (*tbl)->tb_name) == 0)
			return(*tbl);

	return( (Table *)0);
}
