/* argh.c	argument handling routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/mu/RCS/argh.c,v 6.0 1991/12/18 20:31:54 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/mu/RCS/argh.c,v 6.0 1991/12/18 20:31:54 jpo Rel $
 *
 * $Log: argh.c,v $
 * Revision 6.0  1991/12/18  20:31:54  jpo
 * Release 6.0
 *
 */



#include "argh.h"
#include "util.h"

extern	struct	argdef	arg[];		/* defined in main program */
static	struct	argfull *argfull;	/* to point to full array */
static	struct	arginst *ai;		/* to point to linked lists */
static	asource;			/* holds scan_args counter */

struct	sort {
	struct argfull	*tag;
	char		*string;
} argsort[MAXFLAGS];		/* to hold sort sequence */

static	char	*aerrlist[] = {

	/* usage : printf(aerrlist[i],a,b,c,d);  char *a,*b,*c,*d */

	"unknown error\n",						/* 0 */
	"flag %s not known\n",						/* 1 */
	"flag %s ambiguous match\n",					/* 2 */
	"invalid type for flag %s  - fix source\n",			/* 3 */
	"limit of %s argument instances - aborted\n",			/* 4 */
	"flag %s value should be adjacent to '='\n",			/* 5 */
	"flag %s (type adjacent) not followed by value\n",		/* 6 */
	"flag %s - adjacent arguments not allowed without '='\n",	/* 7 */
	"flag %s (type next) must use next argument\n",			/* 8 */
	"flag %s (type next) has no next argument\n",			/* 9 */
	"flag %s (type 0 valued ) should not have value\n",		/*10 */
	"flag %s (type %s valued) has no value%s\n",			/*11 */
	"flag %s (type %s valued) has only %s value%s\n",		/*12 */
	"flag %s (type 1 or more) must have at least one value\n",	/*13 */
	"duplicate flag %s in table - fix source\n",			/*14 */
	"ambiguous flags %s and %s - fix source\n"			/*15 */
};

/* ----------------------------------------------------------------- */
int	prep_args(arg)		/* scan flags list, count, set length,
				sort and calculate match length */
struct	argdef	arg[];
{
	struct	argdef	*a;
	struct	argfull	*af;
	int	i,j,cur,prev;

	for(i=0,a=arg; a->flag[0]; i++,a++);	/* count arg[] */
	flagcount=i;	/* flagcount is structure member for 'others' */

	argfull = (struct argfull *) calloc( (flagcount+1), sizeof(struct argfull) );

	asource=1;	/* initialise scan_counter */

	/* printf("flagcount %d\n",flagcount); */
	
	for(i=0,a=arg,af=argfull; i<=flagcount; i++,a++,af++) {

		af->flag	= a->flag;
		af->function	= a->function;
		af->type	= a->type;
		af->match	= a->match;
		af->length	= 0;
		af->count	= 0;
		af->first	= NULL;
		af->last	= NULL;

		if ( index(VALIDTYPES,af->type) == NULL ) {
			arg_error(3,af->flag);
			cleanquit();
		}

		argsort[i].tag	= af; 		/* pointer to structure */
		argsort[i].string = af->flag;	/* pointer to flag chars. */
		af->length=strlen(af->flag);
		/* printf("length %d  match %d  type %c  %s\n",af->length, af->match, af->type, af->flag); */
	}

	quick_string(argsort,flagcount+1);	/* sort array argsort in flag order */

	argsort[flagcount+1].tag = argsort[0].tag;	/* both ends point to "" */

/*
	for(j=0;j<=flagcount; j++) {
		printf("argsort j=%d string=%s\n",j,argsort[j].string);
	}
*/

	for(j=0,prev=0; j<=flagcount; j++,prev=cur) {
		af = argsort[j].tag;
		cur=mincmp(af->flag,(argsort[j+1].tag)->flag);
		if(!cur) {
			arg_error(14,af->flag);
			return(-1);
		}
		if (j && cur > af->length) {
			arg_error(15,af->flag,(argsort[j+1].tag)->flag);
			return(-1);
		}
		/* printf("sort %d  %s  type %c  cur %d  prev %d\n",j,af,af->type,cur,prev); */
		if(af->match <= 0) {
			if(af->match < 0)
				af->match=af->length;
			else {
				af->match = (cur < prev) ? (prev) : (cur);
			}
		}
	}
	return(0);
}

/* ----------------------------------------------------------------- */
int	scan_args(argc,argv)	/* returns incremented scan_counter */
int	argc;
char	**argv;
{
	struct	argdef	*a=arg;		/* passed in */
	struct	argfull	*af=argfull;	/* generated internally */
	char	**argvtemp=argv;
	char	*cp,*v;
	char	nullbyte='\0';
	int	i,c,ca,offset,n;

	if (!flagcount) {
		if ( (i=prep_args(arg)) )
			cleanquit();
	}
/*
	The argument list is scanned, checking each argument against
	entries in structure 'argfull'.  Linked lists are generated 
	for all arguments of the same type, in command line sequence.
	Any parameters which do not match one of the ->type's are
	accumulated in a list of 'others'.

	 sample			flag
	-samplearg		complete match (adj) ok  ( unless !ADJARGS )
	-sample arg		complete match (next) ok
	       ^ offset
	-sample=arg		complete match with = (adj) ok
	-sample= arg		complete match with = (next) - error - "value should be adjacent to '='"
	        ^ offset
	-samarg			incomplete match (adj) - error - "flag not known/ambiguous match"
	-sam arg		incomplete match (next) ok
	-sam=arg		incomplete match with =arg ok
	-sam= arg		incomplete match with = (next) arg - error - "value should be adjacent to '='"
	     ^ offset
*/
	for(argcpi=1; argcpi < argc; argcpi++) {	/* loop until arguments exhausted */
		cp = *++argvtemp;
		if (**argvtemp == FLAGCHAR) {

			/* now test against each allowed type in turn */

			for(i=0,af=argfull; i < flagcount; i++,af++) {
				if (!mystrncmp(af->flag, &cp[1], af->match)) {

					/* found a match */

					if(( v = index(cp,EQUCHAR) ) != NULL) {
						if(*++v == '\0') {
							arg_error(5,af->flag);
							break;
						}
					}
					else {
						v = &nullbyte;
						if(af->length == af->match) {
							v=cp + af->match + 1;		/* value starts at end of match (could point at '\0') */
						}
						else {

							if ( (ONECHARARGS) && (af->match == 1)) { /* allow one char */
								v = cp + 2 ;
							}
							else {

								/* have matched on af->match chars  - try on ca chars */

								ca=strlen(cp)-1;		/* length of current argument less '-' */
								if (mystrncmp(af->flag, &cp[1], ca))	{ /* doesn't match */

									/* now - try on af->length chars */

									if (mystrncmp(af->flag, &cp[1], af->length))	{ /* doesn't match */
										arg_error(2,af->flag);
										break;
									}
									else {	/* does match af->length chars */
										v=cp + af->length + 1;
									}
								}
								else {	/* does match ca chars */
									v=cp + ca + 1;
								}
							}
						}
						if ((*v != '\0') && !ADJARGS) {
							arg_error(7,af->flag);
							break;
						}
					}	/* endif == EQUCHAR */
					offset=(v-cp);
					c = count_to_flag(argc,argvtemp,v);
					/* printf ("flag %s c-flag%s\n",af->flag,digit_word(c)); */

					switch(af->type) {

					case 'a':
						if (*v == '\0')
							arg_error(6,af->flag);
						else
							take_next(1,af,&argvtemp,v,offset);
						break;

					case 'n':
						if (*v != '\0') {
							arg_error(8,af->flag);
							break;
						}
						if (c < 1 ) {
							arg_error(9,af->flag);
							break;
						}
						else
							take_next(1,af,&argvtemp,v,offset);
						break;

					case 'm':
						if (c < 1 )
							arg_error(13,af->flag);
						else
							take_next(c,af,&argvtemp,v,offset);
						break;

					case 'z':
						take_next(c,af,&argvtemp,v,offset);
						break;
							
					case '0':
						if (c && ZEROCHECK)
							arg_error(10,af->flag);
						else
							take_next(1,af,&argvtemp,*argvtemp,0); /* from add_link */
						break;

					default	:
						/* digit flag type 1-9 */
						n = af->type - '0';
						if( c < n ) {
							if ( !c )
								arg_error(11,af->flag,digit_word(n),(n == 1 ? "" : "s" ) );
							else {
								arg_error(12,af->flag,digit_word(n),digit_word(c),(c == 1 ? "" : "s" ));
							}
						}
						else
							take_next(n,af,&argvtemp,v,offset);
						break;

					}	/* end switch */
					break;
				}
				else { /* no match */
					if (i == flagcount-1 ) { /* have tried all flags */
						arg_error(1,cp);
						/* take_next(1,&arg[flagcount],&argvtemp,*argvtemp,0); */
						take_next(1,af+flagcount,&argvtemp,*argvtemp,0);	/* make it an 'other' */
					}
				}		/* end if */
			}			/* end for */
		}				/* end if == FLAGCHAR */
		else {
			/* this is an 'other' */
			/* add argi to list pointed to by *next */
			take_next(1,af+flagcount,&argvtemp,*argvtemp,0); /* from add_link */
		}
	}	/* endfor */
	return(asource++);	/* return value THEN increment */
}

/* ----------------------------------------------------------------- */
void	add_link(af,v,offset,argn,argof)	/* extend linked list for flagnumber
 					   ONLY called from take_next() */
struct	argfull	*af;
char	*v;
int	offset,argn,argof;
{
	static	int argi_count=0;	/* to count number of entries */

	ai = (struct arginst *) malloc(sizeof(struct arginst));

	/* printf("add_link: %s %x %d %d\n",af->flag,ai,argcpi,offset); */
	if(!af->first) {
		af->first=ai;
	}
	else {
 		(af->last)->next=ai;
	}
	ai->argcp=v;		/* actual string of argument value */
	ai->argci=argcpi;
	ai->argoi=offset;
	ai->argn=argn;
	ai->argof=argof;
	ai->argsrc=asource;	/* indicates source of value */
	ai->argused=0;
	ai->next=NULL;
	af->count++;
	af->last=ai;
	argi_count++;
	if ( argi_count > MAXARGS ) {
		arg_error(4,digit_word(MAXARGS));
		cleanquit();
	}
	return;
}

/* ----------------------------------------------------------------- */
void	take_next(i,af,p,v,offset)	/* take next i arguments  ( assumes i > 0 )
					   offset used if first argument is adjacent  */
int	i,offset;
struct	argfull	*af;
char	***p,*v;
{
	int	argn=1;		/* argn of argof=i */
	int	argof=i;
	
	if (*v != '\0') {
		add_link(af,v,offset,argn,argof);
		i--;
		argn++;
	}
	while(i) {
		(*p)++;
		argcpi++;
		add_link(af,**p,0,argn,argof);
		i--;
		argn++;
	}
	return;
}

/* ----------------------------------------------------------------- */
void	arg_error(i,a,b,c,d)	/* general error message output */
int	i;
char	*a,*b,*c,*d;
{
	printf("argh! : ");
	printf(aerrlist[i],a,b,c,d);
	return;
}

/* ----------------------------------------------------------------- */
int	count_to_flag(argc,argvtemp,v)	/* count arguments up to next flag */
int	argc;
char	**argvtemp,*v;
{
	int	i;
	i=argcpi;
	argvtemp++;
	/* NOTE: this relies on left-right evaluation with && */
	while ((i < (argc-1)) && (**argvtemp != FLAGCHAR)) {
		/* printf("loop   i %d  **argvtemp %c\n",i,**argvtemp); */
		i++;
		argvtemp++;
	};
	return(i-argcpi	+ ( *v == '\0' ? 0 : 1 ));
}
/* ----------------------------------------------------------------- */
void	show_args(argc,argv,fp,af)	/* display (sorted) list of flags, arguments found */
int	argc;
char	**argv;
FILE	*fp;
struct	argfull	*af;	/* points to command which invoked display */
{
	int	i;

	fprintf(fp,"\n%s : ",ARGHVER);
	fprintf(fp,"%d flags  %d command line arguments",flagcount,argc);
	for(i=0; i <= flagcount; i++) {
		af = argsort[i].tag;
		fprintf(fp,(*af->flag == '\0') ? "\nothers ":"\nflag   ");
		fprintf(fp,"%-15s\011type %c\011match %d\011count %d : ",af->flag,af->type,af->match,af->count);
		ai=af->first;
		while (ai) {
			/* fprintf(fp," %s",&argv[ai->argci][ai->argoi]); */
			fprintf(fp," %s",ai->argcp);
			ai=ai->next;
		}
	}
	fprintf(fp,"\n");
	return;
}
/* ----------------------------------------------------------------- */
void	call_args(argc,argv,fp)	/* call any routines corresponding
				   to arguments found in flag
				   declaration order */
FILE	*fp;
int	argc;
char	**argv;
{
	struct	argfull	*af;
	int	i;

	for(i=0,af=argfull; i <= flagcount; i++,af++) {
		if((af->count) && (af->function != NULL)) {
			(*af->function)(argc,argv,fp,af);
		}
	}
	return;
}
/* ----------------------------------------------------------------- */
void	list_args(fp,f)		/* list arguments for flag f to file fp */
FILE	*fp;
char	*f;
{
	char	*p;
	fprintf(fp,"%s_args : ",f);
	while( p = next_arg(f) ) {
		fprintf(fp,"<%s>",p);
	}
	fprintf(fp,"\n");
	return;
}
/* ----------------------------------------------------------------- */
char	*next_arg(f)		/* get next arg for flag f - no ambiguity
				in specification of f allowed  - any level */
char	*f;
{
	int	n,of;		/* place holders */
	int	s = 0;		/* source not significant */
	return(nextc_arg(f,s,&n,&of));
}

/* ----------------------------------------------------------------- */
char	*nextc_arg(f,s,n,of)		/* get next arg for flag f with counts - no ambiguity
					in specification of f allowed */
char	*f;
int	s,*n,*of;	/* source, which, of how many */
{
	static	char	lastf[MAXFLAGLEN]="?";	/* to compare with last call */
	static	int	source;			/* ditto */
	struct	argfull	*af=argfull;
	int	i;

	if ((mystrcmp(f,lastf)) || (source != s)) {	/* new value */
		/* printf("new value <%s>{%d}, old value <%s>{%d}\n",f,s,lastf,source); */
		strcpy(lastf,f);	/* store for future reference */
		source = s;		/* ditto */
		if (*f == '\0') {	/* null string for 'others' */
			ai=(af+flagcount)->first;
		}
		else {
			for(i=0,af=argfull; i <= flagcount; i++,af++) {
				if(mystrcmp(f,af->flag))
					continue;
				/* printf("flag found"); */
				ai=af->first;
				break;
			}
		}
	}
	else {
		ai=ai->next;
	}

	if(source) {	/* searching for source */
		while ( ai && (ai->argsrc != source)) 
			ai=ai->next;
	}
	if(ai) {
		if(source)
			ai->argsrc = 0;	/* clear out source indicator */
		*n	= ai->argn;
		*of	= ai->argof;
		return(ai->argcp);
	}
	else {
		*n	= 0;
		*of	= 0;
		return(NULL);
	}
}

/* ----------------------------------------------------------------- */
char	*digit_word(x)	/* DANGER - do not expect to work for two
			calls both >10 or negative in same printf() */
int	x;
{
	int sign;
	static char buf[10];
	static char *words[]= {
		"zero",
		"one",
		"two",
		"three",
		"four",
		"five",
		"six",
		"seven",
		"eight",
		"nine"
	};
	if (x < 0) {
		sign = TRUE;
		x = -x;
	}
	else
		sign = FALSE;

	if (x < 10)
		if (!sign)
			return(words[x]);
		else
			sprintf( buf, "%s%s", (sign? "minus ":""), words[x] );
	else
		sprintf( buf, "%s%d", (sign? "minus ":""), x );
	return( buf );
}
/* ----------------------------------------------------------------- */
void	quick_string(a,count)	/* quick sort string array
				a[i]->tag 	is tag pointer
				a[i]->string	is pointer to character string
						- to be sorted */
struct	sort	a[];
int	count;
{
	qs_string(a,0,count-1,count);
}
void	qs_string(a,left,right,count)	/* recursive sort */
struct	sort	a[];
int	left,right,count;
{
	register int	i,j;
	register char	*x,*y;	/* pointers to characters being sorted */

	i=left;
	j=right;
	x=a[(left+right)/2].string;

	do {
		while(mystrcmp(a[i].string,x)<0 && i<right) i++;
		while(mystrcmp(a[j].string,x)>0 && j>left) j--;
		if( i <= j ) {

			y = a[i].string;
			a[count+1].tag = a[i].tag;	/* swap tag pointers */

			a[i].string = a[j].string;
			a[i].tag = a[j].tag;

			a[j].string = y;
			a[j].tag = a[count+1].tag;

			i++;
			j--;
		}
	} while ( i <= j );

	if( left < j )	qs_string(a,left,j,count);
	if( i < right )	qs_string(a,i,right,count);
}
/* ----------------------------------------------------------------- */
int	mincmp(a,b)	/* minimum length reqd. to distinguish a, b
			   - this relies on the behaviour of strncmp,
			   when one or other string is exhausted first */

char	*a,*b;
{
	int	i;
	if (!mystrcmp(a,b))
		return(0);		/* cannot distinguish */

	if( a == NULL ||  b == NULL )	/* strictly NULLCP */
		return(1);

	if( &a == '\0' ||  &b == '\0' )
		return(1);

	for(i=1;;i++) {
		/* printf("a %s b %s i %d\n",a,b,i); */
		if(mystrncmp(a,b,i))	/* different at i */
			break;
	}
	return(i);
}
/* ----------------------------------------------------------------- */
mystrcmp(a,b)	/* to avoid null dereference */
char	*a,*b;
{
	if ((a == NULL) && (b == NULL))
		return(0);

	if ((a == NULL))
		return(-1);

	if ((b == NULL))
		return(1);

	return(strcmp(a,b));
}
/* ----------------------------------------------------------------- */
mystrncmp(a,b,i)	/* to avoid null dereference */
char	*a,*b;
int	i;
{
	if ((a == NULL) && (b == NULL))
		return(0);

	if ((a == NULL))
		return(-1);

	if ((b == NULL))
		return(1);

	return(strncmp(a,b,i));
}
/* ----------------------------------------------------------------- */

cleanquit ()
{
	exit (1);
}
