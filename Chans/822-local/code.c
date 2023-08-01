/* code.c: interpreter code */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/822-local/RCS/code.c,v 6.0 1991/12/18 20:04:31 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/822-local/RCS/code.c,v 6.0 1991/12/18 20:04:31 jpo Rel $
 *
 * $Log: code.c,v $
 * Revision 6.0  1991/12/18  20:04:31  jpo
 * Release 6.0
 *
 */



/* This stuff based heavily on Kernigan & Pike's hoc */

#include "util.h"
#include "loc.h"
#include "retcode.h"
#include "expand.h"
#include <isode/cmd_srch.h>

#ifndef NULLCP
#define NULLCP ((char *)0)
#endif
#define NSTACK	1024
static Datum	stack[NSTACK];
static Datum	*stackp;

#define NPROG	2000
Inst	prog[NPROG];
Inst	*progp, *pc;

static char true[] = "true", false[] = "", rp_agn[] = "again",
	delivered[] = "delivered", permfail[] = "perm";
#define istrue(x)	(*x)

int 	lineno;
int	debug = 0;

static struct {
	char	*name;
	char	*val;
} consts[] = {
	"TRUE",		true,
	"FALSE",	false,
	"OK",		true,
	"PERMFAIL",	permfail,
	"TEMPFAIL",	rp_agn,
	0,		0
};

static CMD_TABLE tbl_codes[] = {
	true,		RP_OK,
	rp_agn,		RP_AGN,
	false,		RP_NOOP,
	permfail,	RP_BAD,
	0,		-1
};

static int exiting;
extern char *strdup ();

initialise ()
{
	Symbol	*sp;
	int	i;

	stackp = stack;
	progp = prog;
	if (!lookup (delivered)) {
		sp = install (delivered, strvar);
		sp -> str = false;
		for (i = 0; consts[i].name; i++) {
			sp = install (consts[i].name, constant);
			sp -> str = consts[i].val;
		}
	}
	exiting = 0;
}

#ifdef DISASS
static CMD_TABLE procs[] = {
	"ifcode",	(int) ifcode,
	"eq",		(int) eq,
	"ne",		(int) ne,
	"or",		(int) or,
	"and",		(int) and,
	"stringpush",	(int) stringpush,
	"tofile",	(int) tofile,
	"topipe",	(int) topipe,
	"exitproc",	(int) exitproc,
	"varpush",	(int) varpush,
	"setdeliver",	(int) setdeliver,
	"eval",		(int) eval,
	"defexitproc",	(int) defexitproc,
	"not",		(int) not,
	"unixmail",	(int) tounixfile,
	"STOP",		0,
	0,		-1
};
#endif

run ()
{
	Symbol	*sp;
	int	retval;
#ifdef DISASS
	Inst	*p;
	char	*cp;

	for (p = prog; p < progp; p++)
		if (cp = rcmd_srch (*p, procs))
			PP_TRACE (("%d %s", p - prog, cp));
		else
			PP_TRACE (("%d %x", p - prog, *p));

#endif
	execute (prog);

	sp = lookup (delivered);
	if (sp == NULL)
		adios (NULLCP, "symbol %s not defined!!", delivered);
	switch (retval = cmd_srch (sp -> str, tbl_codes)) {
	    case RP_AGN:
	    case RP_BAD:
	    case RP_OK:
	    case RP_NOOP:
		break;

	    default:
		adios (NULLCP, "Unknown return state '%s'", sp -> str);
	}
	return retval;
}

execute (p)
Inst	*p;
{
	Inst tmppc;

	for (pc = p; *pc != STOP && !exiting; ) {
		tmppc = *pc ++;	/* work around ultrix bug */
		(*tmppc) ();
	}
}

Datum	pop ()
{
	if (stackp == stack)
		adios (NULLCP, "stack underflow");
	return *--stackp;
}

push (d)
Datum	d;
{
	if (stackp >= &stack[NSTACK])
		adios (NULLCP, "stack overflow");
	*stackp++ = d;
}

Inst	*code (f)
Inst	f;
{
	Inst	*oprogp = progp;

	if (progp >= &prog[NPROG])
		adios (NULLCP, "program too big");
	*progp++ = f;
	return oprogp;
}

void or ()
{
	Datum d1, d2;

	d1 = pop ();
	d2 = pop ();
	PP_TRACE (("%s or %s", d1.str, d2.str));
	d1.str = (istrue(d1.str) || istrue(d2.str) ? true : false);
	push (d1);
}

void and ()
{
	Datum d1, d2;

	d1 = pop ();
	d2 = pop ();
	PP_TRACE (("%s and %s", d1.str, d2.str));
	d1.str = (istrue(d1.str) && istrue(d2.str) ? true : false);
	push (d1);
}
		

void eq ()
{
	Datum d1, d2;

	d2 = pop ();
	d1 = pop ();
	PP_TRACE (("%s eq %s", d1.str, d2.str));
	d1.str = (lexequ (d1.str, d2.str) == 0 ? true : false);
	push (d1);
}

void	ne ()
{
	PP_TRACE (("not (psuedo)"));
	eq ();
	not ();
}

void	req ()
{
	Datum d1, d2;
	char	*cp, *dp, *re_comp ();
	static char *rbuf;
	static int rbsize;
	int s;

	d2 = pop ();
	d1 = pop ();

	PP_TRACE (("%s req %s", d1.str, d2.str));
	if (cp = re_comp (d2.str))
		adios (NULLCP, "Bad re expression %s: %s", d2.str, cp);
	s = strlen (d1.str);
	if (rbuf == NULLCP)
		rbuf = malloc ((unsigned)(rbsize = (s + 1)));
	else if (s + 1 > rbsize)
		rbuf = realloc (rbuf, (unsigned) (rbsize = s + 1));
	for (cp = d1.str, dp = rbuf; *cp; cp ++) {
		if (isascii (*cp) && isupper (*cp))
			*dp++ = tolower (*cp);
		else *dp ++ = *cp;
	}
	*dp = '\0';

	if (re_exec (rbuf))
		d1.str = true;
	else	d1.str = false;
	push (d1);
}

void not ()
{
	Datum d;

	d = pop ();
	PP_TRACE (("not %s", d.str));
	d.str = istrue(d.str) ? false : true;
	push (d);
}

void	eval ()
{
	Datum	d;

	d = pop ();

	PP_TRACE (("eval %s", d.sym -> str ? d.sym -> str :
		 "<empty>"));
	if (d.sym -> str == (char *)0)
		d.str = false;
	else	d.str = d.sym -> str;
	push(d);
}

void stringpush ()
{
	Datum	d;

	d.str = (char *)*pc ++;
	PP_TRACE (("stringpush %s", d.str));
	push (d);
}

void varpush ()
{
	Datum	d;

	d.sym = (Symbol *)(*pc ++);
	PP_TRACE (("varpush %s", d.sym -> str));
	push (d);
}

void	ifcode ()
{
	Datum	d;
	Inst	*savepc = pc;

	PP_TRACE (("ifcode"));
	execute (savepc+3);	/* then part */

	d = pop ();
	if (istrue(d.str)) {
		PP_TRACE (("then code"));
		execute (*((Inst **)(savepc)));
	}
	else if (*((Inst **)(savepc+1))) { /* else part */
		PP_TRACE (("else code"));
		execute (*((Inst **)(savepc + 1)));
	}
	PP_TRACE (("endif"));
	if (!exiting)
		pc = *((Inst **)(savepc + 2)); /* next statement */
}

void tofile ()
{
	Datum	d;

	d = pop ();
	PP_TRACE (("tofile %s", d.str));
	switch (putfile (d.str)) {
	    case RP_OK:
		d.str = true;
		break;
	    case RP_AGN:
		d.str = rp_agn;
		break;
	    default:
	    case RP_BAD:
		d.str = false;
		break;
	}
	push (d);
}

void tounixfile ()
{
	Datum	d;

	d = pop ();
	PP_TRACE (("tounixfile %s", d.str));
	switch (putunixfile (d.str)) {
	    case RP_OK:
		d.str = true;
		break;
	    case RP_AGN:
		d.str = rp_agn;
		break;
	    default:
	    case RP_BAD:
		d.str = false;
		break;
	}
	push (d);
}

void topipe ()
{
	Datum d;

	d = pop ();
	PP_TRACE (("topipe %s", d.str));
	switch (putpipe (d.str)) {
	    case RP_OK:
		d.str = true;
		break;
	    case RP_BAD:
		d.str = false;
		break;
	    default:
	    case RP_AGN:
		d.str = rp_agn;
		break;
	}
	PP_TRACE (("topipe = %s", d.str));
	push (d);
}

void	setdeliver ()
{
	Datum	d;
	Symbol	*sym;

	d = pop ();
	PP_TRACE (("setdelivered %s", d.str));
	sym = lookup (delivered);
	if (sym == NULL)
		adios (NULLCP, "symbol %s not defined!!", delivered);
	sym -> str = d.str;
	if (lexequ (d.str, rp_agn) == 0)
		exiting = 1;
	push (d);
}

void exitproc ()
{
	Datum d;
	Symbol	*sym;

	d = pop ();
	PP_TRACE (("exitproc %s", d.str));
	sym = lookup (delivered);
	if (sym == NULL)
		adios (NULLCP, "symbol %s not defined!!", delivered);
	sym -> str = d.str;
	exiting = 1;
}

void	defexitproc ()
{
	Symbol	*sym;
	Datum d;

	PP_TRACE (("defexitproc"));
	sym = lookup (delivered);
	if (sym == NULL)
		adios (NULLCP, "symbol %s not defined!!", delivered);
	d.str = sym -> str;
	push (d);
	exiting = 1;
}

void	prexpr ()
{
	Datum	d;

	d = pop ();
	printit (d.str);
}

void assign ()
{
	Datum	d1, d2;

	d1 = pop ();
	d2 = pop ();
	if (d1.sym -> type != strvar)
		adios (NULLCP, "assignment to non-variable '%s'",
		       d1.sym -> name);
	PP_TRACE (("%s = %s", d1.sym -> name, d2.str));
	d1.sym -> str = d2.str;
	push (d2);
}

checkmacro (s)
char	*s;
{
	char	*p, *q;

	for (q = s; p = index(q, '$'); q = p + 1) {
		if (p[1] == '(') {
			char *cp, buf[1024];

			for (p += 2, cp = buf; *p; p++)
				if (*p == ')')
					break;
				else *cp ++ = *p;
			*cp = 0;
			if (lookup (buf) == (Symbol *)0)
				install (strdup (buf), field);
		}
	}
}

static Symbol *symlist;


Symbol	*lookup (s)
char	*s;
{
	Symbol	*sp;

	for (sp = symlist; sp; sp = sp -> next)
		if (strcmp (sp -> name, s) == 0)
			return sp;
	return 0;
}


Symbol	*install (str, t)
char	*str;
stype	t;
{
	Symbol *sp;

	PP_TRACE (("Install %s",str));
	sp = (Symbol *) smalloc (sizeof *sp);
	sp -> name = str;
	sp -> type = t;
	sp -> str = false;

	sp -> next = symlist;
	symlist = sp;
	return sp;
}

reset_syms ()
{
	Symbol **spp, *sp;

	PP_TRACE (("reset symbols"));
	for (spp = &symlist; *spp; ) {	/* half hearted free attempt */
		sp = *spp;
		spp = &(*spp) -> next;
		if (sp -> name && *sp -> name)
			free (sp -> name);
		free ((char *)sp);
	}
	symlist = 0;
}

int yywrap ()
{
	return 1;
}

yyerror (s)
char	*s;
{
	extern char yytext[];

	adios (NULLCP, "%s near line %d (last token was '%s')",
	       s, lineno, yytext);
}

fillin_var (str, dest)
char	*str;
char	dest[];
{
	Symbol *sp;

	if (sp = lookup (str))
		(void) sprintf (dest, "PATH=%s", sp -> str);
}

install_vars (vars, n, maxv)
Expand vars[];
int	n;
int	maxv;
{
	int i;
	Symbol *sp;

	for (i = n, sp = symlist; i < maxv && sp;
	     sp = sp -> next) {
		if (sp -> type == field)
			continue;
		vars[i].macro = strdup (sp -> name);
		vars[i].expansion = strdup (sp -> str);
		i++;
	}
	vars[i].macro = vars[i].expansion = NULLCP;

}

void create_var (var)
Expand *var;
{
	Symbol *sp;

	if ((sp = lookup (var -> macro)) == NULL)
		sp = install (var -> macro, field);
	sp -> str = var -> expansion;
}
