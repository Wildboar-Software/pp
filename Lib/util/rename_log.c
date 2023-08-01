/* rename_log.c - renames the log file needed for channel initialisation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/util/RCS/rename_log.c,v 6.0 1991/12/18 20:25:18 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/util/RCS/rename_log.c,v 6.0 1991/12/18 20:25:18 jpo Rel $
 *
 * $Log: rename_log.c,v $
 * Revision 6.0  1991/12/18  20:25:18  jpo
 * Release 6.0
 *
 */



#include "head.h" 


rename_log (newname) 
char	*newname;
{
	
	ll_hdinit (pp_log_oper, newname);
        ll_hdinit (pp_log_stat, newname);
        ll_hdinit (pp_log_norm, newname);
}
