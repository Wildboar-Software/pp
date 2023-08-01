/* single_parse.c: test program to parse a single address from stdin */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-single.c,v 6.0 1991/12/18 20:30:40 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/misc/RCS/t-single.c,v 6.0 1991/12/18 20:30:40 jpo Rel $
 *
 * $Log: t-single.c,v $
 * Revision 6.0  1991/12/18  20:30:40  jpo
 * Release 6.0
 *
 */



/* ---------------------------------------------------------------------------

This tool allows a user to input one line of addresses from stdin.
Each address on this line id parsed, normalised and output on stdout.

The tool is useful for examining (via gdb or such) how the address
parsing and normalising routines operate.

As it was derived during the development of the rfc822norm filter, it
takes that filters arguments, in particular -822, -733, -jnt, -bigend,
-littleend and -fold.

---------------------------------------------------------------------------- */



#include	"util.h"
#include	<isode/cmd_srch.h>
#include	"ap.h"
#include	"chan.h"
#include	"retcode.h"


#define	OPT_822		1
#define OPT_733		2	
#define	OPT_JNT		3
#define	OPT_BIGEND	4
#define OPT_LITTLEEND	5
#define OPT_FOLD	10

CMD_TABLE	tbl_options [] = { /* single_parse commandline options */
	"-822",		OPT_822,
	"-733",		OPT_733,
	"-jnt",		OPT_JNT,
	"-bigend",	OPT_BIGEND,
	"-littleend",	OPT_LITTLEEND,
	"-fold",	OPT_FOLD,
	0,		-1
	};

typedef enum {maj_none, rfc822, rfc733, jnt} Major_options;
typedef enum {min_none, bigend, littleend} Minor_options;

static int	getch();
int		pcol;
int		nonempty;
int		fold_width;
int		order_pref;
char		*myname;
int		nadrs;
extern AP_ptr	ap_pinit();
extern int	ap_outtype;
extern int	ap_perlev;
extern char	*calloc();

#define	DEFAULT_FOLD_WIDTH	78

/* ARGSUSED */
main(argc,argv)
int	argc;
char	**argv;
{
	/* parse flags */
	Major_options maj = maj_none;
	Minor_options mino = min_none;

	myname = *argv++;
	sys_init(myname);
/*	malloc_debug(2);*/
	ap_outtype = AP_PARSE_SAME;
	fold_width = DEFAULT_FOLD_WIDTH;
	order_pref = CH_USA_PREF;

	/* parse command arguments */
	while (*argv != NULL) {
		switch(cmd_srch(*argv,tbl_options)) {
		    case -1:
			printf("unknown option '%s'",*argv);
			exit(1);

		    case OPT_FOLD:
			if (*(argv+1) == NULL) {
				printf("no fold width given");
				exit(1);
			} else {
				++argv;
				fold_width = atoi(*argv);
			}
			break;

		    case OPT_822:
			if ((maj == maj_none) || (maj == rfc822)) {
				ap_outtype |= AP_PARSE_822;
				maj = rfc822;
				break;
			} 

		    case OPT_733:
			if ((maj == maj_none) || (maj == rfc733)) {
				ap_outtype |= AP_PARSE_733;
				maj = rfc733;
				break;
			} 

		    case OPT_JNT:
			if ((maj == maj_none || maj == jnt)
			    && (mino == min_none || mino == bigend)) {
				ap_outtype |= AP_PARSE_822;
				maj = jnt;
				ap_outtype |= AP_PARSE_BIG;
				mino = bigend;
				order_pref = CH_UK_PREF;
				break;
			}
			printf("multiple major parse options");
			exit(1);

		    case OPT_BIGEND:
			if (mino == min_none || mino == bigend) {
				ap_outtype |= AP_PARSE_BIG;
				mino = bigend;
				order_pref = CH_UK_PREF;
				break;
			}

		    case OPT_LITTLEEND:
			if (mino == min_none || mino == littleend) {
				mino = littleend;
				break;
			}
			printf("multiple minor parse options");
			exit(1);

		    default:
			printf("unknown option '%s'",*argv);
			exit(1);
		}
		argv++;
	}

	/* ap_outype set so read in address */
	proc();
	exit(0);
}

/*  */
/* parse one line of addresses fro stdin to stdout */
proc ()
{
	int	amp_fail;
	int	res;
	int	done;

	ap_clear();
	nadrs = 0;
	amp_fail = FALSE;
	nonempty = FALSE;
	done = FALSE;
	while (done == FALSE) {
		if (ap_pinit(getch) == BADAP) {
			printf("ap_pinit failed\n");
			exit(1);
		}
		res = ap_1adr();	/* parse one adr */
		switch(res) {
		    case DONE:	/* done */
			done = TRUE;
			break;
		    case NOTOK:
			amp_fail = TRUE;
			break;
		    default:
			break;
		}

		if (done == FALSE) {
			res = out_adr(ap_pstrt);
		}
		ap_sqdelete(ap_pstrt, NULLAP);
		ap_free(ap_pstrt);
		ap_pstrt = NULLAP;
	}

	putchar('\n');
	if (ap_perlev) {
		printf("nested level %d\n",ap_perlev);
		amp_fail++;
	}
	if (amp_fail == TRUE)
		printf("Parse error in original version\n");
}

static int	out_adr(ap)
register AP_ptr	ap;
{
	AP_ptr  loc_ptr,        /* -- in case fake personal name needed -- */
		group_ptr,
		name_ptr,
		dom_ptr,
		route_ptr,
		return_tree;
	char	*addrp;
	int	len;

	if (ap->ap_obtype == AP_NIL)
		return OK;

	if (nadrs != 0) {
		printf(", ");
		pcol += 2;
	}

	ap = ap_normalize(ap, order_pref);

	return_tree = ap_t2p (ap, &group_ptr, &name_ptr, &loc_ptr,
			      &dom_ptr,	&route_ptr);

	if (return_tree == BADAP) {
		printf("error from ap_t2p\n");
		exit(1);
	} else {
		addrp = ap_p2s(group_ptr, name_ptr, loc_ptr, 
			       dom_ptr, route_ptr);

		if (addrp == (char *) NOTOK) {
			printf("error from ap_p2s()\n");
		}
	}

	if ((len = strlen(addrp)) > 0) { /* print */
		pcol += len;
		if (pcol >= fold_width && nonempty) {
			pcol = 2 + len;
			printf("\n%*s", 2, "");
		} else
			nonempty = TRUE;
		
		printf("%s",addrp);
		nadrs++;
	}
	free(addrp);
	return OK;
}

static int	getch()
{
	return	getchar();
}
