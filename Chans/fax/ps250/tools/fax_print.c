/* fax_print.c: print given file of p2 or ia5 on fax printer */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/fax/ps250/tools/RCS/fax_print.c,v 6.0 1991/12/18 20:08:25 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/fax/ps250/tools/RCS/fax_print.c,v 6.0 1991/12/18 20:08:25 jpo Rel $
 *
 * $Log: fax_print.c,v $
 * Revision 6.0  1991/12/18  20:08:25  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "chan.h"
#include <stdio.h>
#include "../../faxgeneric.h"
#include "../ps250.h"
#include <stdarg.h>
#include <isode/cmd_srch.h>
#include "retcode.h"
#include "IOB-types.h"

#define	OPT_P2	1
#define OPT_IA5	2
#define OPT_FAX	3
#define OPT_FILE 4
#define OPT_SOFTCAR 5
#define OPT_RES_7_7	6
#define OPT_RES_3_85	7
#define	OPT_RES_15_4	8

CMD_TABLE	tbl_options [] = { /* rfc822norm commandline options */
	"-p2",	OPT_P2,
	"-ia5",	OPT_IA5,
	"-fax",	OPT_FAX,
	"-file",OPT_FILE,
	"-softcar", OPT_SOFTCAR,
	"-3.85", OPT_RES_3_85,
	"-7.7",	OPT_RES_7_7,
	"-15.4", OPT_RES_15_4,
	0,		-1
	};

int	isP2 = TRUE;
char	*file = NULLCP;
int	res;

static int fax_bitstrings(), do_decode_fax();

#ifndef lint
void    advise (int code, char *what, char *fmt, ...)
{
        va_list ap;

        va_start (ap, fmt);

        _ll_log (pp_log_norm, code, what, fmt, ap);

        va_end (ap);
}
#else
/* VARARGS3 */

void    advise (code, what, fmt)
char   *what,
       *fmt;
int     code;
{
        advise (code, what, fmt);
}
#endif

#define ps_advise(ps, f) \
        advise (LLOG_EXCEPTIONS, NULLCP, "%s: %s",\
                (f), ps_error ((ps) -> ps_errno))

extern FaxCtlr	*init_faxctrlr();
extern int	fax_debug;
CHAN	*mychan = NULLCHAN;
int	table_verified = FALSE;

main (argc, argv)
int	argc;
char	**argv;
{
	FILE	*fp;
	FaxCtlr	*faxctl = init_faxctrlr();
	PsModem	*psm = PSM(faxctl);

	sys_init (argv[0]);
	*argv++;
	while	(*argv != NULL) {
		switch (cmd_srch(*argv, tbl_options)) {
		    case OPT_P2:
			isP2 = TRUE;
			break;
		    case OPT_IA5:
			isP2 = FALSE;
			break;
		    case OPT_FAX:
			if (*(argv+1) == NULLCP) {
				fprintf(stderr,
					"no fax device given with flag '%s'\n",*argv);
				exit(1);
			}
			argv++;
			sprintf(psm->devName, "%s", *argv);
			break;

		    case OPT_FILE:
			if (*(argv+1) == NULLCP) {
				fprintf(stderr,
					"no file given with flag '%s'\n",*argv);
				exit(1);
			}
			argv++;
			file = *argv;	
			break;
			
		    case OPT_SOFTCAR:
			psm->softcar = TRUE;
			break;

		    case OPT_RES_3_85:
			res = RES_3_85;
			break;

		    case OPT_RES_7_7:
			res = RES_7_7;
			break;

		    case OPT_RES_15_4:
			res = RES_15_4;
			break;

		    default:
			fprintf(stderr,
				"unknown option '%s'\n", *argv);
			exit(1);
		}
		argv++;
	}

	if (file != NULLCP) {
		if (isP2
		    && (fp = fopen (file, "r")) == (FILE *)0) {
			fprintf(stderr,
				"Can't open file '%s'\n", file);
		exit(1);
		}
	} else
		fp = stdin;
		
	
	if (faxctl->open != NULLIFP
	    && (*faxctl->open) (faxctl) != OK) {
		fprintf(stderr,
			"fax open failed [%s]\n",
			faxctl->errBuf);        
		if (faxctl->close != NULLIFP)
			(*faxctl->close)(faxctl);
		exit(1);
	}

	sprintf (faxctl -> telno, "0");

        if (faxctl->initXmit != NULLIFP
            && rp_isbad((*faxctl->initXmit)(faxctl))) {
		if (faxctl -> abort != NULLIFP)
			(*faxctl->abort) (faxctl);
		fprintf(stderr,
			"unable to initialise fax device for printing [%s]\n",
			faxctl->errBuf);       
		if (faxctl->close != NULLIFP)
			(*faxctl->close)(faxctl);
		exit(1);
	}

	if (isP2) 
		print_P2(faxctl, fp);
	else
		print_ia5(faxctl, file);
	
	if (faxctl->close !=  NULLIFP)
		(*faxctl->close) (faxctl);
	
	if (file != NULLCP) fclose(fp);
		
}

/*  */

print_P2 (faxctl, fp)
FaxCtlr	*faxctl;
FILE	*fp;
{
	register PE     pe = NULLPE;
        register PS     psin;
	struct type_IOB_G3FacsimileBodyPart	*g3fax = NULL;

	if ((psin = ps_alloc (std_open)) == NULLPS)
        {
                ps_advise (psin, "ps_alloc");
		if (faxctl -> abort != NULLIFP)
			(*faxctl->abort) (faxctl);
		if (faxctl->close != NULLIFP)
			(*faxctl->close)(faxctl);
                exit(1);
        }

	if (std_setup (psin, fp) == NOTOK)
        {
                ps_free (psin);
                advise (LLOG_EXCEPTIONS, NULLCP, "%s: std_setup loses", file);
		if (faxctl -> abort != NULLIFP)
			(*faxctl->abort) (faxctl);
		if (faxctl->close != NULLIFP)
			(*faxctl->close)(faxctl);
                exit(1);
        }

	if (do_decode_fax(psin, &g3fax) == NOTOK) {
		fprintf(stderr,
		       "failed to decode G3FacsimileBodyPart\n");
                ps_done(psin, "ps2pe");
		if (faxctl -> abort != NULLIFP)
			(*faxctl->abort) (faxctl);
		if (faxctl->close != NULLIFP)
			(*faxctl->close)(faxctl);
		exit(1);
        }

        if (g3fax->parameters
            && faxctl->setG3Params != NULLIFP
            && (*faxctl->setG3Params)(faxctl, g3fax->parameters) != OK) {
                fprintf(stderr,
			"setG3Params failed [%s]", faxctl->errBuf);
                return RP_MECH;
        }


	if (fax_bitstrings (faxctl, g3fax, TRUE) != OK) {
		fprintf(stderr,
			"Failed to fax bitstrings\n");
		
		if (faxctl->close != NULLIFP)
			(*faxctl->close)(faxctl);
		exit(1);
	}
	if (pe != NULLPE)
                pe_free (pe);
}

/* 
 * send each page of g3fax to fax machine 
 */

static int fax_bitstrings (faxctl, g3fax, last)
FaxCtlr	*faxctl;
struct type_IOB_G3FacsimileBodyPart	*g3fax;
int last;
{
	struct type_IOB_G3FacsimileData	*ix;
	int retval;
	
	for (ix = g3fax -> data; ix != NULL; ix = ix -> next) {
		int		len;
		char	*str;

		if ((str = bitstr2strb (ix -> element_IOB_0, &len)) == NULLCP) {
			fprintf(stderr,"empty page");
			return NOTOK;
		}
		len = (len + BITSPERBYTE -1)/BITSPERBYTE;

		retval = faxctl->sendPage(faxctl, str, len, 
					  (last && ix -> next == NULL));
		free (str);
		if (retval != OK)
			return retval;
	}
	return  OK;
}

print_ia5 (faxctl, file)
FaxCtlr	*faxctl;
char	*file;
{
        if (faxctl->setIA5Params != NULLIFP
            && (*faxctl->setIA5Params) (faxctl) != OK) {
                PP_LOG(LLOG_EXCEPTIONS,
                       ("setIA5Params failed [%s]",
                        faxctl->errBuf));
                return RP_MECH;
        }
        
        if ((*faxctl->sendIA5File)(faxctl, file, TRUE) != OK) {
		if (faxctl -> abort != NULLIFP)
			(*faxctl->abort) (faxctl);
		fprintf(stderr,
			"sendIA5File failed [%s]",
			faxctl->errBuf);
		if (faxctl->close != NULLIFP)
			(*faxctl->close)(faxctl);
		exit(1);
	}
}


/* 
 * decode from PE to C struct
 */
extern PE	ps2pe_aux ();

static int do_decode_fax (psin, pfax)
register PS	psin;
struct type_IOB_G3FacsimileBodyPart	**pfax;
{
	struct type_IOB_G3FacsimileData		*new, *tail = NULL;
	int					pages;
	register PE     			pe = NULLPE;
	
	/* get first chunk */
	if ((pe = ps2pe_aux (psin, 1, 0)) == NULLPE) {
		ps_done (psin, "ps2pe_aux");
		return NOTOK;
	}
	
	if ((PE_ID(pe -> pe_class, pe -> pe_id) !=  
	     PE_ID(PE_CLASS_UNIV, PE_CONS_SEQ))
	    && (PE_ID(pe -> pe_class, pe -> pe_id) !=
		PE_ID(PE_CLASS_CONT, (PElementID) 0x03))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unexpected pe in G3FacsimileBodyPart: expected sequence"));
		return NOTOK;
	}
	
	pe_free (pe);

	*pfax = (struct type_IOB_G3FacsimileBodyPart *) calloc (1,
							       sizeof (struct type_IOB_G3FacsimileBodyPart));
	
	/* get and decode parameters */
	if ((pe = ps2pe (psin)) == NULLPE) { /* EOF or error? */
		ps_done (psin, "ps2pe");
		return NOTOK;
        }
        PY_pepy[0] = 0;

	if (decode_IOB_G3FacsimileParameters(pe, 1, 
					    NULLIP, NULLVP, 
					    &((*pfax) -> parameters)) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("decode_IOB_G3FacsimileParameters failure [%s]",
			PY_pepy));
		pe_done(pe, "Parse failure IOB_G3FacsimileParameters");
		return NOTOK;
	}
	
	if (PY_pepy[0] != 0)
		PP_LOG(LLOG_EXCEPTIONS,
		       ("parse_IOB_G3FacsimileParameters non fatal failure [%s]",
                        PY_pepy));

	pe_free (pe);

	/* get next chunk */
	if ((pe = ps2pe_aux (psin, 1, 0)) == NULLPE) {
		ps_done (psin, "ps2pe_aux");
		return NOTOK;
	}
	
	if (PE_ID(pe -> pe_class, pe -> pe_id) != 
	    PE_ID(PE_CLASS_UNIV, PE_CONS_SEQ)) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unexpected pe in G3FacsimileBodyPart: expected sequence"));
		return NOTOK;
	}

	pe_free (pe);
	
	pages = 0;
	while ((pe = ps2pe (psin)) != NULLPE) {
		switch (PE_ID(pe -> pe_class, pe -> pe_id)) {
		    case PE_ID (PE_CLASS_UNIV, PE_PRIM_BITS):
			break;
		    case PE_ID (PE_CLASS_UNIV, PE_UNIV_EOC) :
			pe_free(pe);
			continue;
		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unexpected pe in G3FacsimileBodyPart: expected bitstring"));
			return NOTOK;
		}
		new = (struct type_IOB_G3FacsimileData *) (calloc (1,
								  sizeof(struct type_IOB_G3FacsimileData)));
		new -> element_IOB_0 = prim2bit(pe);
		if ( (*pfax) -> data == NULL)
			(*pfax) -> data = tail = new;
		else {
			tail -> next = new;
			tail = tail -> next;
		}
		pages++;
	}
	
	if ((*pfax) -> parameters -> optionals == opt_IOB_G3FacsimileParameters_number__of__pages
	    && (*pfax) -> parameters -> number__of__pages != pages) {
		PP_LOG(LLOG_EXCEPTIONS, 
		       ("Incorrect number of pages in G3FacsimileBodyPart: expected %d, got %d",
			(*pfax) -> parameters -> number__of__pages, pages));
		return NOTOK;
	}
	return OK;
}

/*
 * various isode-like routines
 */

/* pe_done: utility routine to do the right thing for pe errors */
int             pe_done (pe, msg)
PE              pe;
char            *msg;
{
        if (pe->pe_errno)
        {
                PP_OPER(NULLCP,
                        ("%s: [%s] %s",msg,PY_pepy,pe_error(pe->pe_errno)));
                pe_free (pe);
                return NOTOK;
        }
        else
        {
                pe_free (pe);
                return OK;
        }
}

/* ps_done: like pe_done */
int             ps_done (ps, msg)
PS              ps;
char           *msg;
{
        ps_advise (ps, msg);
        ps_free (ps);
        return NOTOK;
}

void adios (char *what, char* fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
	_ll_log (pp_log_norm, LLOG_FATAL, what, fmt, ap);
    va_end (ap);
    _exit (1);
}
