%union {
	char *str;
}

%token  <str> STRING

%type	<str>	host
%%
file		:	line
		|       file line
		;

line		:	'\n'
		|	STRING ':' hostlist '\n' {
				/* Direct Connet or AR hosts */
				if (CheckHost ($1) != 0)
					StoreHost ($1);
				    else
					FreeHostInfo (HostList);
				free ($1);
				HostList = NULL;
				}
		|	STRING ':' '(' chanlist ')' '\n' {
				if (CheckChan (ChanList) != 0)
					StoreChans ($1);
				    else
					FreeChanInfo (ChanList);
				free ($1);
				ChanList = NULL;
				}
		|	error '\n' {
				crapline ();
				HostList = NULL;
				ChanList = NULL;
				}
		;

hostlist	:	host {
				StartHostList ($1);
				ChanList = NULL;
				}
		|	hostlist ',' host {
				AddToHostList ($3);
				ChanList = NULL;
				}
		;

host		:	STRING '(' ')' {
				/* This is an Application Relay */
				$$ = $1;
				}
		|	STRING '(' chanlist ')' {
				/* This is a Direct Connect */
				$$ = $1;
				}
		;

chanlist	:	STRING {
				StartChanList ($1);
				}
		|	chanlist ',' STRING {
				AddToChanList ($3);
				}
		;

%%

#include        <stdio.h>
#include	"tjoin.h"

extern  int     yylineno;

int	PrintNode (), CheckARs ();
SBUFF	*InitStringStore ();

char	*av0, *MyName;
HOST	*HostList = NULL;	/* Stores Host info on input line basis */
CHAN	*ChanList = NULL;	/* Stores Chan info on input host basis */
CHAN	*ValidChanList = NULL;
MTA	*MtaTree = NULL;	/* Root of in core database */
SBUFF	*ARStringStore, *HostStringStore;
int	Debug = 0, PrintRoute = 0, DirectFirst = 0, ComplexOutput = 0;

main (argc, argv)
int     argc;
char    *argv [];
{
	register int	i;

	av0 = argv [0];
	MyName = NULL;
	for (i = 1; i < argc; i++) {
		if (*argv [i] == '-')
			DoFlag (&argv [i][1]);
		    else
			if (MyName == NULL)
				MyName = argv [i];
			    else
				DefineChan (argv [i]);
	}
	if (ValidChanList == NULL)
		Usage ();
	ARStringStore = InitStringStore ();
	HostStringStore = InitStringStore ();
	yyparse ();
	WalkTree (MtaTree, CheckARs);
	/*
	 * Now that all AR names point to MTA structs - Zap space used
	 * to store names.
	 */
	FreeStringStore (ARStringStore);
	if (Debug == 2)	DebugTree (MtaTree, 0);
	WalkTree (MtaTree, PrintNode);
	exit (0);
}
