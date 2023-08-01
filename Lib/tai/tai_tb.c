/* tai_tb.c: table tailoring info */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/tai/RCS/tai_tb.c,v 6.0 1991/12/18 20:24:59 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/tai/RCS/tai_tb.c,v 6.0 1991/12/18 20:24:59 jpo Rel $
 *
 * $Log: tai_tb.c,v $
 * Revision 6.0  1991/12/18  20:24:59  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"
#include <isode/cmd_srch.h>
#include "table.h"


extern void	err_abrt ();
extern void	tai_error ();

static int	tb_numtables = 0;
static int	tb_maxtb;
static char	tbl_str[] = "tbl";

#define CMDTNAME        1
#define CMDTFILE        2
#define CMDTSHOW        3
#define CMDTFLAGS       4
#define CMDTOVER	5

static  CMD_TABLE tb_cmdtbl[] = {
	"file",         CMDTFILE,
	"flags",        CMDTFLAGS,
	"name",         CMDTNAME,
	"override",	CMDTOVER,
	"show",         CMDTSHOW,
	0,              -1
};
#define N_TBCMDS	((sizeof(tb_cmdtbl)/sizeof(CMD_TABLE))-1)


#define CMDTFLINEAR     1
#define CMDTFDBM        2


static  CMD_TABLE tb_flags[] = {
	"linear",       CMDTFLINEAR,
	"dbm",          CMDTFDBM,
	0,              -1
};



/* ---------------------  Begin  Routines  -------------------------------- */



int tbl_tai (argc, argv)
int     argc;
char    **argv;
{
	register Table  *tbptr;
	char            *p,
			*arg;
	int             ind;

	if (argc < 2 || lexequ (argv[0], tbl_str) != 0)
		return (NOTOK);

        if (tb_maxtb == 0) {
                tb_maxtb = 10;
                tb_all = (Table **)smalloc (sizeof(Table *) * tb_maxtb);
        }
        else if (tb_numtables + 1 == tb_maxtb) {
                tb_maxtb += 10;
                tb_all = (Table **) realloc ((char *)tb_all,
					     (unsigned)sizeof (Table *) * tb_maxtb);
                if (tb_all == NULL)
                        err_abrt (RP_MECH, "Out of space for tables");
        }


	argc--;
	arg = *++argv;
	tb_all[tb_numtables++] = tbptr = (Table *) smalloc (sizeof (Table));
	tb_all [tb_numtables]   = NULLTBL;
	tbptr -> tb_name        = arg;
	tbptr -> tb_show        = arg;
	tbptr -> tb_file        = arg;
	tbptr -> tb_fp          = NULLFILE;
	tbptr -> tb_flags       = TB_DBM;
	tbptr -> tb_override	= NULL;
	argv++;
	argc--;

	for (ind = 0 ; ind < argc ; ind++) {

		if ((p = index (argv[ind], '=')) == NULLCP)
			continue;
		*p++ = '\0';

		switch (cmdbsrch (argv[ind], tb_cmdtbl, N_TBCMDS)) {
		    case CMDTNAME:
			tbptr->tb_name = p;
			break;
		    case CMDTFILE:
			tbptr->tb_file = p;
			break;
		    case CMDTSHOW:
			tbptr->tb_show = p;
			break;
		    case CMDTFLAGS:
			switch (cmd_srch (p, tb_flags)) {
			    case CMDTFDBM:
				tbptr->tb_flags = TB_DBM;
				break;
			    case CMDTFLINEAR:
				tbptr->tb_flags = TB_LINEAR;
				break;
			    default:
				tai_error ("Unknown flag %s for %s",
					   p, tbptr -> tb_name);
				break;
			}
			break;
		    case CMDTOVER:
			{
				char *cp;
				TableOverride *to, **tp;

				if ((cp = index(p, ':')) == NULL)
					tai_error ("Illegal override syntax '%s' for %s",
						   p, tbptr -> tb_name);
				else {
					*cp++ = '\0';
					to = (TableOverride *)
						smalloc (sizeof *to);
					to -> tbo_key = p;
					to -> tbo_value = cp;
					to -> tbo_next = NULL;
					for (tp = &tbptr -> tb_override;
					     *tp; tp = &(*tp) -> tbo_next)
						continue;
					*tp = to;
				}
			}
			break;

		    default:
			tai_error ("unknown key %s for table %s",
				   argv[ind], tbptr -> tb_name);
			break;
		}
	}
	return (OK);
}
