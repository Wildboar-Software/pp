/* p_ut.c: Common Utilities for probe */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/probe/RCS/p_ut.c,v 6.0 1991/12/18 20:32:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/probe/RCS/p_ut.c,v 6.0 1991/12/18 20:32:41 jpo Rel $
 *
 * $Log: p_ut.c,v $
 * Revision 6.0  1991/12/18  20:32:41  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "q.h"
#include "tb_bpt84.h"
#include "tb_p1.h"
#include "retcode.h"
#include <isode/cmd_srch.h>
#include <pwd.h>



/* --- externals --- */
extern CMD_TABLE			bptbl_body_parts84[];
extern CMD_TABLE			p1tbl_encinfoset[];
extern Q_struct				*QuePtr;



/* -- local routines -- */
ADDR *make_adr_new();
int set_encoded();
int set_yn();
void set_from();




/* ---------------------  Begin  Routines  -------------------------------- */




int set_encoded (values)
char	*values;
{
	int     argc;
        char    *argv[10];
	int	retval;

	argv[0] = "=";
	argv[1] = rcmd_srch (EI_BIT_STRING, p1tbl_encinfoset);

	if (!isstr (values)) 
		argv[2] = rcmd_srch(BPT_IA5, bptbl_body_parts84);
	else  	
		argv[2] = values;

	retval = txt2encodedinfo (&QuePtr->encodedinfo, argv, 3);
	return retval;
}




int set_yn (value)
char	*value; 
{ 
	if (!isstr (value))
		return NO;
	if (lexequ (value, "y") == 0)
		return YES;
	if (lexequ (value, "yes") == 0)
		return YES;
	return NO;
}




void set_from()
{
	RP_Buf		rp;
	struct passwd	*pwd,
			*getpwuid();

	if ((pwd = getpwuid(getuid())) == NULL) {
		printf("Cannot get your passwd entry\n");
		exit(0);
	}

	QuePtr -> Oaddress = make_adr_new (pwd->pw_name, AD_ORIGINATOR);
}


ADDR *make_adr_new (value, flag)
char	*value;
int	flag;
{
	static int 	recipno = 0;
	int		type = 0;
	ADDR		*ap = NULLADDR;

	if (index (value, '@'))
		type = AD_822_TYPE;
	else if (value[0] == '/') 
		type = AD_X400_TYPE;
        else
		type = AD_822_TYPE;

	switch (flag) {
	case AD_ORIGINATOR: 
		ap =  adr_new (value, type, 0);
		break;
	case AD_RECIPIENT: 
		ap = adr_new (value, type, ++recipno);
		break;
	default:
		printf ("make_adr_new/Unknown type '%d'\n", flag);
		exit (1);
	}

	return (ap);
}
	
