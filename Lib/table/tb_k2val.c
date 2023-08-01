/* tb_k2val.c: primitive routine - convert key to value */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_k2val.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_k2val.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_k2val.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "table.h"


#define MAXF    80


/*
All the tables with open files
*/

static Table            *tb_xall [MAXF];
static int              curclose;
extern  char            *tbldfldir;
static int tblosearch ();



/* ---------------------  Start  Routines  -------------------------------- */

int tb_k2val (tbl, key, val, first)
register Table  *tbl;
char            *key;
char            *val;
int		first;
{
	FILE    *f;
	char    linebuf[LINESIZE],
		value[LINESIZE];
	int     i, status;


	PP_DBG (("Lib/tb_k2val (%s)", key));

	if (tbl -> tb_override &&
	    tblosearch (tbl, key, val, first) == OK)
		return OK;

	if (TB_SRC (tbl -> tb_flags) == TB_DBM)
		return (tb_dbmk2val (tbl, key, val, first));


	if (tbl -> tb_fp == NULLFILE) {
		/*
		Must open the file....
		*/
		if (*tbl -> tb_file != '/') {
			(void) sprintf (linebuf, "%s/%s",
					tbldfldir, tbl -> tb_file);
			f = fopen (linebuf, "r");
		}
		else
			f = fopen (tbl -> tb_file, "r");

		if (f == NULLFILE) {
			PP_OPER (tbl -> tb_file, ("Cannot open"));
			return (NOTOK);
		}

		tbl -> tb_fp = f;

		for (i = 0 ; i < MAXF ; i++)
			if (tb_xall [(i+curclose) % MAXF] == NULL) {
				tb_xall [(i+curclose) % MAXF] = tbl;
				goto got;
			}

		(void) fclose (tb_xall [curclose] -> tb_fp);
		tb_xall [curclose] -> tb_fp = NULLFILE;
		tb_xall [curclose++] = tbl;
		curclose %= MAXF;

	got:;
	}

	f = tbl -> tb_fp;
	if (first)
		rewind (f);
	while ((status = tab_fetch (f, linebuf, value)) != DONE) {
		if (status == NOTOK)
			continue;
		if (lexequ (linebuf, key) == 0) {
			/*
			Found now return it
			*/
			if (val == NULL)
				return (OK);
			(void) strcpy (val, value);
			return (OK);
		}
	}
	return (NOTOK);
}


int tab_fetch (f, key, val)
register FILE           *f;
char                    *key;
char                    *val;
{
	register char *cp;
	int	c;

	while ((c = getc(f)) != EOF && isspace (c))
		continue;

	if (c == '#') {
		while ((c = getc(f)) != EOF && c != '\n')
			continue;
		return tab_fetch (f, key, val);
	}

	cp = key;
	do {
		if (c == '\n') {
			*cp = '\0';
			PP_LOG (LLOG_EXCEPTIONS, 
				("no value - missing ':' key=%s", key));
			return NOTOK;
		}
		if (c == ':') {
			if (cp > key && cp[-1] == '\\')
				cp [-1] = c;
			else	break;
		}
		else *cp ++ = c;
		if (key - cp > LINESIZE) {
			*cp = '\0';
			PP_LOG (LLOG_EXCEPTIONS, ("Key too long '%s'", key));
			return NOTOK;
		}
	} while ((c = getc (f)) != EOF);
	if (c == EOF)
		return DONE;
	*cp = '\0';

	while ((c = getc(f)) != EOF && c != '\n' && isspace (c))
		continue;

	cp = val;
	do {
		if (c == '\n')
			break;
		else 
			*cp ++ = c;
		if (val - cp > LINESIZE) {
			*cp = '\0';
			PP_LOG (LLOG_EXCEPTIONS, ("Value too long %s:%s",
						key, val));
			return NOTOK;
		}
	} while ((c = getc (f)) != EOF);
	if (c == EOF)
		return DONE;
	*cp = '\0';
	return OK;
}

/* ARGSUSED */
static int  tblosearch (tbl, key, val, first)
Table *tbl;
char *key;
char *val;
int first;			/* may do something with this someday */
{
	TableOverride *to;

	for (to = tbl -> tb_override; to; to = to -> tbo_next)
		if (lexequ (key, to -> tbo_key) == 0) {
			(void) strcpy (val, to -> tbo_value);
			return OK;
		}
	return NOTOK;
}
