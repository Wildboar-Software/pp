%{
static CMD_TABLE	keywds[] = {
	"if",		IF,
	"else",		ELSE,
	"exit",		EXIT,
	"ignore",	IGNORE,
	"file",		TOFILE,
	"pipe",		TOPIPE,
	"unixfile",	TOUNIXFILE,
	"print",	PRINT,
	0,		-1
};

extern int lineno;
%}

%%
[ \t]+		{;}
\n		{ lineno ++; }
"!="		{ return NE; }
"=="		{ return EQ; }
"&&"		{ return AND; }
"||"		{ return OR; }
"!"		{ return NOT; }
[0-9]+		{
 			yylval.yy_n = atoi (yytext);
			return NUMBER;
		}
"#".*		{;}
"$("[a-z][-a-z0-9]*")"	{
			yytext[strlen(yytext)-1] = 0;
			if ((yylval.yy_sym = lookup (yytext+2)) == NULL)
				yylval.yy_sym = install (strdup (yytext+2),
							 field);
			return FIELD;
		}
[A-Za-z][A-Za-z0-9]*	{
		int n;

		if ((n = cmd_srch (yytext, keywds)) != -1)
			return n;
		if ((yylval.yy_sym = lookup (yytext)) == NULL)
			yylval.yy_sym = install (strdup(yytext), strvar);
		return VARIABLE;
	}

\"	{
		int	c;
		char	*cp, *pp, *ep;
		int	len;
		
		pp = smalloc (len = 20);

		for (ep = (cp = pp) + len - 1;; ) {
			if ((c = input ()) == NULL)
				yyerror ("end-of-file while reading string");
			if (c == '\n')
				yyerror ("newline in string");
			if (c == '"') {
				*cp = NULL;
				yylval.yy_str = pp;
				return STRING;
			}
			if (c == '\\') {
				switch (c = input()) {
				    case 0:
					yyerror ("end-of-file while reading string");
					break;
				    case 'b':
					c = '\b';
					break;
				    case 'f':
					c = '\f';
					break;
				    case 'n':
					c = '\n';
					break;
				    case 'r':
					c = '\r';
					break;
				    default:
					break;
				}
			}
			if (cp >= ep) {
				register int curlen = cp - pp;
				register char *dp;

				if ((dp = realloc (pp,
						   (unsigned) (len *= 2)))
				    == NULL)
					yyerror ("out of memory");
				cp = dp + curlen;
				ep = (pp = dp) + len - 1;
			}
			*cp ++ = c;
		}
	}
"/"	{
		int	c;
		char	*cp, *pp, *ep;
		int	len;
		
		pp = smalloc (len = 20);

		for (ep = (cp = pp) + len - 1;; ) {
			if ((c = input ()) == NULL)
				yyerror ("end-of-file while reading regexp");
			if (c == '\n')
				yyerror ("newline in regexp");
			if (c == '/') {
				*cp = NULL;
				yylval.yy_str = pp;
				return REGEXP;
			}
			if (c == '\\') {
				switch (c = input()) {
				    case 0:
					yyerror ("end-of-file while reading string");
					break;
				    case 'b':
					c = '\b';
					break;
				    case 'f':
					c = '\f';
					break;
				    case 'n':
					c = '\n';
					break;
				    case 'r':
					c = '\r';
					break;
				    default:
					break;
				}
			}
			if (cp >= ep) {
				register int curlen = cp - pp;
				register char *dp;

				if ((dp = realloc (pp,
						   (unsigned) (len *= 2)))
				    == NULL)
					yyerror ("out of memory");
				cp = dp + curlen;
				ep = (pp = dp) + len - 1;
			}
			*cp ++ = c;
		}
	}
.	{ return yytext[0]; }

