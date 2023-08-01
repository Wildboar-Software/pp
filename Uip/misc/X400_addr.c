/* x400_adr.c: basically ckadr but stripped */
/*	returns the x400 form of the given addresses */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/misc/RCS/X400_addr.c,v 6.0 1991/12/18 20:39:34 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/misc/RCS/X400_addr.c,v 6.0 1991/12/18 20:39:34 jpo Rel $
 *
 * $Log: X400_addr.c,v $
 * Revision 6.0  1991/12/18  20:39:34  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "adr.h"
#include "retcode.h"
#include "list_rchan.h"


static int	responsibility = NO;
static int	parse= CH_UK_PREF;
static void	parse_address();


/* ---------------------  Begin  Routines  ---------------------------------- */

main(argc, argv)
int	   argc;
char	   **argv;
{
	char	buf[BUFSIZ];
	int	i = 1;
	int	opt;

	sys_init(argv[0]);
	
	argc--;
	argv++;
		
	if (argc == 0) 
		while (gets(buf) != NULL)
			parse_address(buf);
	else 
		while (argc--)
			parse_address(*argv++);
}

static void parse_address(str)
char	*str;
{
	ADDR		*ad;
	RP_Buf		rp;
	int		origType;
	int		type, first;
	LIST_RCHAN	*ix;


	if (index (str, '@'))
		type = AD_822_TYPE; 
	else if (str[0] == '/')
		type = AD_X400_TYPE;
	else 
		type = AD_822_TYPE;

	ad = adr_new(str, type, 0);
	ad -> ad_resp = responsibility;
	origType = ad->ad_type;

	if (rp_isbad(ad_parse(ad, &rp, parse)))
		printf("Address parsing failed:\nReason: %s\nParsing gave this:\n\n", ad->ad_parse_message);
	if (isstr(ad->ad_r400adr))
		printf("%s\n", ad->ad_r400adr);
}
