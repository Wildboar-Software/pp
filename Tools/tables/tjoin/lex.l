%{


#include        "y.tab.h"

#ifdef input
#undef input
#endif
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):bwgetc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#define YY_NO_INPUT

#define         HEAP            30      /*  cleans the heap after a YACC err */
#define         NULL             0

extern  YYSTYPE     		yylval;
extern 	int 			Debug;
extern	char			*av0;
extern	char 			*strdup();

static  char    		buffer [1000];
static  char    		*buffptr = 0;

char    			**memptr, *heapstore [HEAP];

%}

%%

#.*\n			;
[-a-zA-Z0-9\.]*         {
                        yylval.str = strdup (yytext);
                        return (STRING);
                        }

\n                      {
                        return ('\n');
                        }

[,:()]			{
			return (yytext [0]);
			}

.			;

%%



bwgetc (fp)
FILE    *fp;
{
        if (buffptr == 0 || *buffptr == '\00') {
                if (fgets (&buffer [0], 999, fp) == NULL)
                        return (EOF);
                buffptr = &buffer[0];
                memptr = &heapstore[0];
                *memptr = NULL;
	}
        return (*buffptr++);
}

crapline()
{
        char    **p;
	int	len;

	len = strlen (&buffer[0]);
	if (len < 0)
		--len;

        buffer [len] = '\00';

        for (p = &heapstore[0]; *p != NULL; p++) {
		if ((Debug & 1) != 0)
			printf ("Free %s - CrapLine\n", p);
                free (*p);
		p = NULL;
	}
        fprintf (stderr, "*** Error [%s] line %d\n", &buffer [0], yylineno - 1);
}
