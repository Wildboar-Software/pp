/* t-getdomain.c: test results of tb_getdomain */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/tables/domain/RCS/t-getdomain.c,v 6.0 1991/12/18 20:33:42 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/tables/domain/RCS/t-getdomain.c,v 6.0 1991/12/18 20:33:42 jpo Rel $
 *
 * $Log: t-getdomain.c,v $
 * Revision 6.0  1991/12/18  20:33:42  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "chan.h"
#include "table.h"

main (argc, argv)
int	argc;
char	*argv[];
{
	char	buf[BUFSIZ];
	extern int optind;
	int	opt, dmnorder = CH_USA_ONLY;
	char	*table = NULLCP;
	extern char	*optarg;

	sys_init(argv[0]);
	while ((opt = getopt (argc, argv, "bl0123t:T:")) != EOF) 
		switch (opt) {
		    case 'l':
			dmnorder = CH_USA_PREF;
			break;
		    case 'b':
			dmnorder = CH_UK_PREF;
			break;
		    case '0':
                    case '1':
                    case '2':
                    case '3':
                        dmnorder = opt - '0';
                        break;
		    case 't':
		    case 'T':
			table = optarg;
			break;
		    default:
			printf("usage: %s [-b] [-l] [-0] [-1] [-2] [-3]\n",
			       argv[0]);
			exit(1);
			break;
		}

	argc -= optind;	
	argv += optind;
	
	if (table != NULLCP) {
		Table	*Domain;
		if ((Domain = tb_nm2struct("domain")) == NULLTBL) {
			printf("No table 'domain' defined\n");
			exit(1);
		}
		Domain->tb_flags = TB_LINEAR;
		Domain->tb_file = table;
	}
	if (argc == 0)
		while (gets(buf) != NULL) {
			char	*ix = &(buf[0]);
			while (ix != NULLCP &&
			       *ix != '\0'
			       && isspace(*ix)) ix++;
			if (ix != NULLCP && *ix != '\0')
				get_domain(buf, dmnorder);
		}
	else
		while (argc--)
			get_domain(*argv++, dmnorder);
	exit(0);
}

get_domain(dom, order)
char	*dom;
int	order;
{
	char	norm[BUFSIZ], chan[BUFSIZ], *subdom;

	if (tb_getdomain(dom, chan, norm, order, &subdom) != OK)
		printf("Domain '%s' unknown\n",
		       dom);
	else {
		printf("Input '%s'\nNormalised '%s'\n",
		       dom, norm);
		if (subdom != NULLCP) {
			printf("Local sub hub '%s'\n",
			       subdom);
			free(subdom);
		}
		if (chan[0] != '\0')
			printf("Channel key '%s'\n",
			       chan);
		else
			printf("No channel key\n");
	}
	printf("\n");
}

