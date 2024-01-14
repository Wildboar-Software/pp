/* adr_check.c: check command line argument is a valid address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/ckadr/RCS/ckadr.c,v 6.0 1991/12/18 20:28:46 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/ckadr/RCS/ckadr.c,v 6.0 1991/12/18 20:28:46 jpo Rel $
 *
 * $Log: ckadr.c,v $
 * Revision 6.0  1991/12/18  20:28:46  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "adr.h"
#include "retcode.h"
#include "list_rchan.h"
#include <stdio.h>

static int	responsibility = YES;
#ifdef UKORDER
static int	dmnorder = CH_UK_PREF;
#else
static int	dmnorder = CH_USA_ONLY;
#endif
static void	parse_address();
static int 	ad_type = AD_822_TYPE;
static int	percents = FALSE;
static int	normalised = APARSE_NORM_NEXTHOP;
static int	full = NOTOK;
static int	adno = 1;
/* ---------------------  Begin  Routines  ---------------------------------- */


extern ADDR *adr_new_new();

void main(argc, argv)
int	   argc;
char	   **argv;
{
	char	buf[BUFSIZ];
	CHAN	*inchan;
	extern int 	optind;
	extern char	*optarg;
	int	opt;
	ad_type = AD_822_TYPE;

	sys_init(argv[0]);
	while ((opt = getopt (argc, argv, "ofrxanpbl0123i:")) != EOF)
		switch (opt) {
		    case 'o':
			/* originator address */
			adno = 0;
			break;
		    case 'i':
			/* inbound channel to set addressing */
			if ((inchan = ch_nm2struct (optarg)) == NULLCHAN) {
				printf("Unknown channel '%s'\n",
				       optarg);
				exit(1);
			}
			if (inchan -> ch_chan_type != CH_IN
			    && inchan -> ch_chan_type != CH_BOTH) {
				printf("Channel '%s' is not an inbound channel\n",
				       inchan -> ch_name);
				exit (1);
			}
			ad_type = inchan -> ch_in_ad_type;
			dmnorder = inchan -> ch_ad_order;

			if (inchan -> ch_in_ad_subtype == AD_JNT
			    || inchan -> ch_in_ad_subtype == AD_REAL733)
				percents = TRUE;

			if (inchan -> ch_domain_norm == CH_DOMAIN_NORM_ALL)
				normalised = APARSE_NORM_ALL;
			    
		    case 'f':
			full = OK;
			break;

		    case 'r':
			ad_type = AD_822_TYPE;
			break;

		    case 'x':
			ad_type = AD_X400_TYPE;
			break;
				
		    case 'a':
			normalised = APARSE_NORM_ALL;
			break;
		    case 'n':
			responsibility = NO;
			break;
		    case 'p':
			percents = TRUE;
			break;
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
		    default:
			printf("usage: %s [-i inboundChan] [-x] [-r] [-a] [-n] [-p] [-b] [-o] [-l] [-0] [-1] [-2] [-3]\n", argv[0]);
			exit(1);
			break;
		}
	argc -= optind;
	argv += optind;
			
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
	int		first, retval;
	LIST_RCHAN	*ix;

	ad = adr_new(str, ad_type, adno);
	ad -> ad_resp = responsibility;
	origType = ad->ad_type;
	ad -> aparse -> percents = percents;
	ad -> aparse -> normalised = normalised;
	ad -> aparse -> dmnorder = dmnorder;
	ad -> aparse -> full = full;

	if (rp_isbad(retval = ad_parse_aux(ad, &rp)))
		printf("Address parsing failed:\nReason: %s\nParsing gave this:\n\n", ad->ad_parse_message);
		



	if (ad->ad_type == AD_X400_TYPE) {
		if (isstr(ad->ad_r400adr))
			printf("%s ->  (x400)  %s\n", str, ad->ad_r400adr);

		if (isstr(ad->ad_r822adr))
			printf("%s -> (rfc822) %s\n", str, ad->ad_r822adr);
	} 
	else {
		if (isstr(ad->ad_r822adr))
			printf("%s -> (rfc822) %s\n", str, ad->ad_r822adr);

		if (isstr(ad->ad_r400adr))
			printf("%s ->  (x400)  %s\n", str, ad->ad_r400adr);
	}

	ix = ad -> ad_outchan;
	first = 0;

	if (!rp_isbad(retval)) {
		while (ix) {
			if (first == 0)
				printf ("\n");

			printf("%s to %s by %s\n",
			       (first++ == 0) ? "Delivered" : "Or delivered",
			       ix  -> li_mta,
			       (ix -> li_chan) ? 
			       ix -> li_chan -> ch_name : "no channel given");
			ix = ix -> li_next;
		}
	}
	printf ("\n");
}
