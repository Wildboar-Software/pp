/* p_stdin.c: Gets values for the probe via stdin */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/probe/RCS/p_stdin.c,v 6.0 1991/12/18 20:32:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/probe/RCS/p_stdin.c,v 6.0 1991/12/18 20:32:41 jpo Rel $
 *
 * $Log: p_stdin.c,v $
 * Revision 6.0  1991/12/18  20:32:41  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "q.h"
#include "tb_q.h"
#include <isode/cmd_srch.h>


#define  GetStr(buf)			(gets (buf) != NULL && isstr(buf))


/* --- externals --- */
extern CMD_TABLE			qtbl_que[];
extern Q_struct				*QuePtr;
extern ADDR				*make_adr_new();


/* --- static routines  --- */
static void 				prompt_encoded();
static int					prompt_yn();



/* ---------------------  Begin  Routines  -------------------------------- */




void prompt_probe_stdin()
{
	char    buf[BUFSIZ];
	int	found;
	ADDR	*ap;

	/* --- Print a message --- */
	printf ("\n\n");
	printf (
"******************************************************************************");
	printf ("\n\n");
	printf ("This is a probe generation utility. Please note that the Recipient address \n"); 
	printf ("field is recursive, and that each address should be input, a line at a time\n\n"); 
	printf (
"******************************************************************************");
	printf ("\n\n");


	/* --- Originator --- */
	printf ("From:   ");
	printf ("%s\n", QuePtr -> Oaddress  -> ad_value);


	/* --- Recipients --- */
	printf ("To:     ");
	for (found = FALSE; GetStr(buf);) {
		ap = make_adr_new (buf, AD_RECIPIENT);
		adr_add (&QuePtr -> Raddress, ap);
		printf ("        ");
		found = TRUE;
	}
	if (found == FALSE) {
		printf ("*** Error: No Recipients specified ***\n"); 
		exit (1);
	}


	/* --- Content Length --- */
	printf ("%s:  ", rcmd_srch(Q_MSGSIZE, qtbl_que));
	GetStr(buf);
	QuePtr -> msgsize = atoi(buf);


	/* --- EncodedInformationTypes --- */
	(void) prompt_encoded();


	/* --- UA id --- */
	printf ("%s:  ", rcmd_srch(Q_UA_ID, qtbl_que));
	GetStr(buf);
	if (isstr(buf))
		QuePtr -> ua_id = strdup (buf); 


	/* --- Per Message Flag --- */
	QuePtr -> implicit_conversion_prohibited = 
		prompt_yn (rcmd_srch (Q_IMPLICIT_CONVERSION, qtbl_que));

	QuePtr -> alternate_recip_allowed = 
		prompt_yn (rcmd_srch (Q_ALTERNATE_RECIP_ALLOWED, qtbl_que));

	printf ("\n");
}



/* ---------------------  Static Routines  -------------------------------- */



static int prompt_yn (name)
char	*name; 
{ 
	char	buf[BUFSIZ];
	int	retval;
	
	if (name == NULLCP) { 
		 printf ("*** Error: Internal error no key specified ***\n");
                exit (1);
        }
	
	printf ("%s (y/n):  ", name);
	if (gets(buf) == NULL)
		exit (1);
	retval = set_yn(buf);
	return retval;
}



static void prompt_encoded()
{
	char	buf[BUFSIZ];
	int	retval;		


	for (;;) {
		printf ("%s:  ", rcmd_srch(Q_ENCODED_INFO, qtbl_que));
		if (gets(buf) == NULL)	exit (1);
		retval = set_encoded (buf);
		if (retval == NOTOK)
			continue;
		return;
	}
}
