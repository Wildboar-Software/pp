%{
#include "util.h"
#include <ctype.h>
#include <isode/cmd_srch.h>
#include "loc.h"

#define YYDEBUG 1
#define	code2(c1,c2)	code (c1); (void) code (c2)
#define	code3(c1,c2,c3)	code (c1); (void) code (c2); (void) code(c3)
%}

%union {
	char	*yy_str;
	int	yy_n;
	Inst	*yy_inst;
	Symbol	*yy_sym;
}

%token	IF ELSE TOFILE TOPIPE IGNORE EXIT PRINT TOUNIXFILE
%token	<yy_sym> VARIABLE FIELD
%token	<yy_str> STRING REGEXP
%token	<yy_n>	NUMBER
%type	<yy_inst> if end condition expression statement statementlist prexp
%type	<yy_n>	ignore 

%right '='
%left OR
%left AND
%left EQ NE
%left NOT

%%

list:		statementlist { code (STOP); }
	;

statementlist:	/* empty */ { $$ = progp; }
	| 	statementlist statement
	;

statement:	expression ';' { code ((Inst)pop); }
	|	';'	{ $$ = progp; }
	|	PRINT prexp ';' { $$ = $2; }
	|	if condition statement end {
			($1)[1] = (Inst)$3;
			($1)[3] = (Inst)$4;
		}
	|	if condition statement end ELSE statement end {
			($1)[1] = (Inst)$3;
			($1)[2] = (Inst)$6;
			($1)[3] = (Inst)$7;
		}
	|	'{' statementlist '}' { $$ = $2; }
	;

if	:	IF	{ $$ = code (ifcode); code3 (STOP, STOP, STOP); }
	;

end	:	/* nothing */	{ code (STOP); $$ = progp; }
	;

condition:	'(' expression ')' { code(STOP); $$ = $2; }
	;

expression:	'(' expression ')' { $$ = $2; }
	|	expression EQ REGEXP { 
			char *cp, *re_comp();
			char	buf[256];
			if (cp = re_comp ($3)) {
				(void) sprintf (buf, "bad RE '%s': %s",
						$3, cp);
				yyerror (buf);
			}
			code3 (stringpush, (Inst)$3, req);
		}
	|	expression NE REGEXP {
			char *cp, *re_comp ();
			char	buf[256];
			if (cp = re_comp ($3)) {
				(void) sprintf (buf, "bad RE '%s': %s",
						$3, cp);
				yyerror (buf);
			}
			code2 (stringpush, (Inst)$3);
			code2 (req, not);
		}
	|	VARIABLE '=' expression { code3 (varpush, (Inst)$1, assign); }
	|	expression EQ expression { code (eq); }
	|	expression NE expression { code (ne); }
	|	expression OR expression { code (or); }
	|	expression AND expression { code (and); }
	|	NOT expression { $$ = $2; code (not); }
	|	ignore TOFILE STRING {
			checkmacro ($3);
			$$ = code3(stringpush, (Inst)$3, tofile); 
			if ($1)
				code (setdeliver);
		}
	|	ignore TOPIPE STRING {
			checkmacro ($3);
			$$ = code3(stringpush, (Inst)$3, topipe);
			if ($1)
				code (setdeliver);
		}
	|	ignore TOUNIXFILE STRING {
			checkmacro ($3);
			$$ = code3 (stringpush, (Inst)$3, tounixfile);
			if ($1)
				code (setdeliver);
		}
	|	EXIT  { $$ = code2(defexitproc, STOP); }
	|	EXIT '(' expression ')' { $$ = $3; code2(exitproc, STOP); }
	|	FIELD { $$ = code3 (varpush, (Inst)$1, eval); }
	|	VARIABLE { $$ = code3 (varpush, (Inst)$1, eval); }
	|	STRING { $$ = code2 (stringpush, (Inst)$1); }
	;

ignore:	/* empty */	{ $$ = 1; }
	|	IGNORE	{ $$ = 0; }
	;

prexp:		expression	{ code(prexpr); }
	|	prexp ',' expression	{ code(prexpr); }
	;

%%

#include "lex.c"
