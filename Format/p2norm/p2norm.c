/* p2norm.c: p2 heading normalisation */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/p2norm/RCS/p2norm.c,v 6.0 1991/12/18 20:20:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/p2norm/RCS/p2norm.c,v 6.0 1991/12/18 20:20:24 jpo Rel $
 *
 * $Log: p2norm.c,v $
 * Revision 6.0  1991/12/18  20:20:24  jpo
 * Release 6.0
 *
 */

#include "head.h"
#include "util.h"
#include "IOB-types.h"
#include "or.h"
#include "adr.h"
#include <stdarg.h>
#include <isode/cmd_srch.h>

extern char	*dn2ufn();
extern char	*local_dit;
extern struct type_IOB_ORName	*orn2orname();

static void parse_cmdline();
void convert_ORDescriptor(), convert_ORName();

main(argc, argv)
int	argc;
char	**argv;
{
	PE 	pe = NULLPE;
	PS	ps = NULLPS;
	int	ishead = TRUE;

	sys_init(argv[0]);
	or_myinit();
	quipu_syntaxes();
	dsap_init ((int *) NULL, (char ***) NULL);
	local_dit = NULLCP; /* hack to get full DNs */
	
	parse_cmdline(argc, argv);

	if (((ps = ps_alloc (std_open)) == NULLPS) ||
	    (std_setup(ps, stdin) == NOTOK))
	{
		PP_LOG (LLOG_EXCEPTIONS, 
			("P22toP2() failed to setup inbound PS"));
		return error_exit();
	}

	if ((pe = ps2pe(ps)) == NULLPE)
	{
		PP_LOG (LLOG_EXCEPTIONS,
			("ps2pe error on input"));
		return error_exit();
	}

	if (ishead == TRUE) 
		convert_p2_header(&pe);
	else
		convert_p2_ipn(&pe);

	if (ps) ps_free(ps);

	if (((ps = ps_alloc (std_open)) == NULLPS) ||
		(std_setup(ps, stdout) == NOTOK))
	{
		PP_LOG (LLOG_EXCEPTIONS,
			("P22toP2() failed to setup outbound PS"));
		return error_exit();
	}
	
	if (pe2ps(ps, pe) == NOTOK) 
	{
		PP_LOG (LLOG_EXCEPTIONS,
			("pe2ps error on input"));
		return error_exit();
	}
	
	if (ps) ps_free(ps);
	if (pe) pe_free(pe);
	exit(0);
	return OK;
}

/*  */
/* parse commmand line arguments */
int	do_downgrade = FALSE, do_normalise = TRUE, internal = FALSE;

#define ARG_NONORM	1
#define ARG_DOWNGRADE	2
#define ARG_EXTERNAL	3
#define ARG_INTERNAL	4

static CMD_TABLE tbl_args[] = {
	"-nonorm",	ARG_NONORM,
	"-downgrade", ARG_DOWNGRADE,
	"-external",	ARG_EXTERNAL,
	"-internal",	ARG_INTERNAL,
	0, -1
};

static void parse_cmdline(argc, argv)
int	argc;
char	**argv;
{
	int i;
	
	for (i = 1; i < argc; i++) {
		switch (cmd_srch(argv[i],
				 tbl_args)) {
		    case ARG_NONORM:
			do_normalise = FALSE;
			break;
			
		    case ARG_DOWNGRADE:
			do_downgrade = TRUE;
			break;

		    case ARG_EXTERNAL:
			internal = FALSE;
			break;

		    case ARG_INTERNAL:	
			internal = TRUE;
			break;

		    default:
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unknown command line argument '%s'",
				argv[i]));
			break;
		}
	}
}

/*  */
/* convert header */

convert_p2_header(ppe)
PE	*ppe;
{
	struct type_IOB_Heading	*head;

	PY_pepy[0] = 0;
	
	if (decode_IOB_Heading (*ppe, 0, NULLIP, NULLVP, &head) != OK) 
	{
		PP_OPER(NULLCP,
			("decode_IOB_Heading() failed : [%s]", PY_pepy));
		return error_exit();
	}
		
	if (PY_pepy[0] != 0)
		PP_LOG (LLOG_EXCEPTIONS,
			("decode_IOB_Heading non-fatal failure [%s]",PY_pepy)); 
	
	convert_header(head);

	if (*ppe) pe_free(*ppe);
	
	PY_pepy[0] = 0;

	if (encode_IOB_Heading (ppe, 0, 0, NULLCP, head) != OK)
	{
		PP_OPER(NULLCP,
			("encode_IOB_Heading() failed : [%s]", PY_pepy));
		return error_exit();
	}
	if (PY_pepy[0] != 0)
		PP_LOG (LLOG_EXCEPTIONS,
			("encode_IOB_Heading non-fatal failure [%s]", PY_pepy));
	return OK;
}

convert_header(hd)
struct type_IOB_Heading	*hd;
{
	if (hd -> this__IPM)
		convert_IPMIdentifier (hd -> this__IPM);
	
	if (hd -> originator)
		convert_ORDescriptor(hd -> originator);

	if (hd -> authorizing__users)
		convert_ORDescriptorSeq(hd -> authorizing__users);

	if (hd -> primary__recipients)
		convert_RecipientSeq(hd -> primary__recipients);

	if (hd -> copy__recipients)
	convert_RecipientSeq(hd -> copy__recipients);
	
	if (hd -> blind__copy__recipients)
		convert_RecipientSeq(hd -> blind__copy__recipients);

	if (hd -> replied__to__IPM)
		convert_IPMIdentifier (hd -> replied__to__IPM);

	if (hd -> obsoleted__IPMs)
		convert_IPMIdentifierSeq (hd -> obsoleted__IPMs);

	if (hd -> related__IPMs)
		convert_IPMIdentifierSeq (hd -> related__IPMs);

	if (hd -> reply__recipients)
		convert_ORDescriptorSeq(hd -> reply__recipients);

	if (do_downgrade == TRUE && hd -> extensions) {
		free_IOB_ExtensionsField(hd -> extensions);
		hd -> extensions = (struct type_IOB_ExtensionsField *) NULL;
	}
}

convert_ORDescriptorSeq (seq)
struct type_IOB_ORDescriptorSequence	*seq;
{
	while (seq != (struct type_IOB_ORDescriptorSequence *) NULL) {
		if (seq -> ORDescriptor)
			convert_ORDescriptor (seq -> ORDescriptor);
		seq = seq -> next;
	}
}

convert_RecipientSeq (seq)
struct type_IOB_RecipientSequence	*seq;
{
	while (seq != (struct type_IOB_RecipientSequence *) NULL) {
		if (seq -> RecipientSpecifier)
			convert_RecipientSpecifier (seq -> RecipientSpecifier);
		seq = seq -> next;
	}
}

convert_RecipientSpecifier (recip)
struct type_IOB_RecipientSpecifier	*recip;
{	
	if (recip -> recipient)
		convert_ORDescriptor(recip -> recipient);
}

convert_IPMIdentifierSeq (seq)
struct type_IOB_IPMIdentifierSequence *seq;
{
	while (seq != (struct type_IOB_IPMIdentifierSequence *) NULL) {
		if (seq -> IPMIdentifier)
			convert_IPMIdentifier(seq -> IPMIdentifier);
		seq = seq -> next;
	}
}
	
convert_IPMIdentifier (ipmid)
struct type_IOB_IPMIdentifier	*ipmid;
{
	char	*str;

	if (ipmid -> user) {
		convert_ORName (&(ipmid -> user),
				&str);
		if (str) free(str);
	}
}

void convert_ORName (pname, pdn)
struct type_IOB_ORName	**pname;
char **pdn;
{
	ORName	*orn;
	
	*pdn = NULLCP;

	if ((orn = orname2orn (*pname)) == NULLORName)
		return;

	if (do_normalise == TRUE) {
		Aparse_ptr	ap = aparse_new();
		
		ap->orname->on_or = or_tdup(orn->on_or);
		ap -> ad_type = AD_X400_TYPE;
		ap -> dmnorder = CH_USA_PREF;
		ap -> normalised = APARSE_NORM_NEXTHOP;
		ap -> percents = TRUE;
		ap -> internal = internal;
		if (aparse_norm (ap) == OK) {
			or_free(orn->on_or);
			orn->on_or = ap->orname->on_or;
			ap->orname->on_or = NULLOR;
		}
		aparse_free(ap);
		free((char *) ap);
	}

	if (do_downgrade == TRUE) {
		/* downgrade orn -> on_or */
		or_downgrade(&(orn->on_or));

		if ((*pname) -> directory__name && orn->on_dn != NULL) {
			*pdn = dn2ufn(orn->on_dn, FALSE);
			dn_free (orn->on_dn);
			orn->on_dn = NULL;
		}
	}

	free_IOB_ORName (*pname);
	*pname = orn2orname(orn);
}

void convert_ORDescriptor (desc)
struct type_IOB_ORDescriptor	*desc;
{
	char	*dn;

	if (!desc || !desc -> formal__name) 
		return;

	convert_ORName (&(desc->formal__name), &dn);

	if (dn != NULLCP) {
		char	*ffn, *res;
		int	len;
		if (desc -> free__form__name != NULL) {
			/* incorporate dn as comment in free__form__name */
			ffn = qb2str (desc -> free__form__name);
			free_IOB_FreeFormName (desc -> free__form__name);
		} else
			ffn = NULLCP;

		len = strlen(dn) + strlen(" (DN=)") + 1;
		if (ffn != NULLCP)
			len += strlen(ffn);
		res = malloc(len * sizeof(char));

		if (ffn == NULLCP)
			sprintf(res, "(DN=%s)", dn);
		else
			sprintf(res, "%s (DN=%s)", ffn, dn);
		
		desc -> free__form__name = 
			str2qb(res, strlen(res), 1);
		if (dn) free(dn);
		if (ffn) free(ffn);
		if (res) free(res);
	}
}

/*  */
/* convert ipn */

convert_p2_ipn(ppe)
PE	*ppe;
{
	struct type_IOB_IPN	*head;

	PY_pepy[0] = 0;

	if (decode_IOB_IPN (*ppe, 0, NULLIP, NULLVP, &head) != OK) 
	{
		PP_OPER(NULLCP,
			("decode_IOB_IPN() failed : [%s]", PY_pepy));
		return error_exit();
	}
		
	if (PY_pepy[0] != 0)
		PP_LOG (LLOG_EXCEPTIONS,
			("decode_IOB_IPN non-fatal failure [%s]",PY_pepy)); 
	
	convert_ipn(head);

	if (*ppe) pe_free(*ppe);
	
	PY_pepy[0] = 0;

	if (encode_IOB_IPN (ppe, 0, 0, NULLCP, head) != OK)
	{
		PP_OPER(NULLCP,
			("encode_IOB_IPN() failed : [%s]", PY_pepy));
		return error_exit();
	}
	if (PY_pepy[0] != 0)
		PP_LOG (LLOG_EXCEPTIONS,
			("encode_IOB_IPN non-fatal failure [%s]", PY_pepy));
	return OK;
}

convert_ipn(ipn)
struct type_IOB_IPN *ipn;
{

	if (ipn -> subject__ipm)
		convert_IPMIdentifier (ipn -> subject__ipm);

	if (ipn -> ipn__originator)
		convert_ORDescriptor (ipn -> ipn__originator);
	
	if (ipn -> ipn__preferred__recipient)
		convert_ORDescriptor (ipn -> ipn__preferred__recipient);

	/* should really do something about ReturnedIPMField 
	   in non recipt fields but will assume message has 
	   been exploded */
}

/*  */
/* misc */

error_exit()
{
	exit(1);
}

#ifndef lint

void    advise (int code, char *what, char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);

    (void) _ll_log (pp_log_norm, code, what, fmt, ap);

    va_end (ap);
}

#else
/* VARARGS */

void    advise (code, what, fmt)
char    *what,
        *fmt;
int      code;
{
    advise (code, what, fmt);
}
#endif

