/* make-lists.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/make-lists/RCS/make-lists.c,v 6.0 1991/12/18 20:30:32 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/make-lists/RCS/make-lists.c,v 6.0 1991/12/18 20:30:32 jpo Rel $
 *
 * $Log: make-lists.c,v $
 * Revision 6.0  1991/12/18  20:30:32  jpo
 * Release 6.0
 *
 */



/*
 * 
 * Reads usernames aliases file, and creates quipu distribution list
 * channel files.
 * 
 * Usage:
 * 
 * make-lists input channel-file list-directory
 * 
 * A line starting with '#' is a comment. Else, a line containg both
 * ':' and ',' is assumed to be the first line of a list; it continues
 * while succeeding lines start with whitespace.
 * 
 * Note that the list directory must be in a form suitable for
 * prefixing directly a filename (ie ends in / on unix, or is like
 * [zxc] or thing: on VMS), this name is placed into the channel-file.
 * 
 * (nb vms/unix compatible, mainly so I can debug with a *decent* debugger.
 * It'll only be run 'in anger' on un*x) (M.A.Scott)
 * 
 * 
 * Modified, fairly extensively A.Macpherson.
 * 
 */


static char sccsid[] = { "%W%\t-\t%E%\tSTL\n" };

#include <stdio.h>

extern char    *malloc();
extern char    *realloc();
extern char    *strchr(), *strrchr();
extern char    *strcpy();

static int readline ();
static int addusers ();
static void panic ();

#define MAXMODERATOR 6
struct list {
  struct list    *next;		/* link pointer */
  struct list    *left;         /* other link */
  char           *list_name;	/* name of the list (eg "200") */
  char           *users;	/* points to '\n' separated record of users */
  int             users_alloc;	/* currently allocated space INCLUDING the
				 * '\0' */
  int             users_used;	/* currently used space including the null */
  char           *moderator[MAXMODERATOR];	/* list-request person(s) */
};

#define USERS_INCR 512		/* increment for 'users' area */

static struct list *list_pointer;

#ifndef BUFSIZ
#	define	BUFSIZ	255    	/* input line length allowed */
#endif

#ifdef VMS
# define	ERROR	(USER_FATAL|USER_INHIBIT)
# define	OK	USER_SUCCESS
# include	<misc.h>
#else
# define	ERROR	1
# define	OK	0
# define	EOS	'\0'
#endif


#define FIRST	0
#define CONTIN	1

static struct list *get_list();
static void	write_tables();
static int	trim();
static int	analyse_first();
static int	analyse_contin();
static int	analyse_request();

static char    *
    mymalloc(n)
int             n;
{
    char           *p = malloc((unsigned) n);
    
    if (p == 0) {
	perror("mymalloc");
	exit(ERROR);
    }
    return p;
}

main(argc, argv)
int             argc;
char           *argv[];

{
    FILE           *input;
    static char     buffer[BUFSIZ + 1];
    int             state;
    struct list    *current = 0;
    
    if (argc != 4) {
	(void) printf("usage: %s input-file channel-table dislist-directory\n",
		      argv[0]);
	exit(ERROR);
    }
    input = fopen(argv[1], "r");
    if (input == 0) {
	perror("cannot read input");
	exit(ERROR);
    }
    /* read entire input file.... */
    state = FIRST;
    
    while (readline(buffer, BUFSIZ, input) != EOF) {
	
	if (buffer[0] == '#' || buffer[0] == '\0')
	    continue;			/* comment line */
	
	switch (state) {
	case FIRST:
	    if (buffer[0] == ' ') {
		(void) printf("unexpected continuation line - ignored!\n%s\n", buffer);
		continue;
	    }
	    trim(buffer);
	    /*
	     * it's not an error if it's not a list, but we need to check for a
	     * request line
	     */
	    if (analyse_request(buffer, current))
		break;
	    if (analyse_first(buffer, &current))
		state = CONTIN;
	    break;
	    
	case CONTIN:
	    if (!analyse_contin(buffer, current))
		state = FIRST;
	    break;
	    
	default:
	    panic();
	    break;
	}
    }
    (void) fclose(input);
    
    write_tables(argv[2], argv[3]);
}


/* returns 1 if user list end in ',' else 0 */
static int analyse_first(buffer, current_pointer)
char           *buffer;
struct list   **current_pointer;
{
    struct list    *p;
    char           *rest;
    int             i;
    
    /* get dis list name */
    rest = strchr(buffer, ':');
    if (rest == 0)
	panic();
    *rest++ = EOS;
    
    p = get_list(buffer);
    
    *current_pointer = p;
    return addusers(rest, p);	/* and user list */
}

/* returns 1 if user list end in ',' else 0 */
static int 
    analyse_contin(buffer, current_pointer)
char           *buffer;
struct list    *current_pointer;
{
    if ((strchr(buffer, ':') != 0) || (buffer[0] != ' ')) {
	(void) printf("continuation line of list expected - ignored!\n%s\n",
		      buffer);
	return 0;
    }
    trim(buffer);
    return addusers(buffer, current_pointer);
}


/*
 * check for a -request, ie moderator, line. This is assumed to occur after
 * the list is defined. Duplicate moderators are appended, up to a maximum.
 */
static 
    analyse_request(buffer, current_pointer)
char           *buffer;
struct list    *current_pointer;
{
    char           *moderator = strchr(buffer, ':');
    char           *dash;
    char           *at;
    struct list    *p = current_pointer;
    int             i;
    
    if (moderator == 0)
	return(0);
    
    *moderator++ = EOS;
    dash = strrchr(buffer, '-');
    if (dash == 0 || strncmp(dash + 1, "request", 7) != 0) {
	*--moderator = ':';
	return(0);
    }
    
    *dash = EOS;			/* buffer now points to list name */
    
    /*
     * good chance current list is the one! If not, scan whole list. 
     */
    if ((p == 0) || strcmp(p->list_name, buffer) != 0)
	p = get_list(buffer);
    
    for (i = 0; i < MAXMODERATOR; ++i) {
	if (dash = strchr(moderator, ','))
	    *dash++ = EOS;
	if (p->moderator[i] == 0) 
	    p->moderator[i] = mymalloc(strlen(moderator) + 1);
	at = strchr(moderator, '@');
	if (at != 0)
	    *at = EOS;
	(void) strcpy(p->moderator[i], moderator);
	if (dash)
	    moderator = dash;
	else
	    break;
    }
    if (i >= MAXMODERATOR)
	(void) printf("too many moderators for list %s: %s ignored!\n",
		      buffer, moderator);
    return(1);
}


static void 
    write_tables(channel_file, table_dir)
char           *channel_file, *table_dir;
{
    struct list    *p;
    FILE           *fch;
    void            inorder_lists();
    
    fch = fopen(channel_file, "w");
    if (fch == 0) {
	(void) printf("channel file open error\n");
	perror(channel_file);
	exit(ERROR);
    }
    if (chdir(table_dir) != 0) {
	(void) printf("list directory error\n");
	perror(table_dir);
	unlink(channel_file);
	exit(ERROR);
    }
    
    inorder_lists(list_pointer, fch, table_dir);
    
    if (ferror(fch) != 0 || (fclose(fch) != 0)) {
	(void) printf("channel file write error\n");
	perror(channel_file);
	exit(ERROR);
    };
}

char	modbuff[BUFSIZ];

void
    inorder_lists(p, fch, table_dir)
struct list *p;
FILE        *fch;
char        *table_dir;
{
    FILE *flist;
    int   i;
    
    if (!p)
	return;
    
    inorder_lists(p->next, fch, table_dir);
    
    if (p->moderator[0] || strchr(p->users, '\n') != (char *)0) {
	flist = fopen(p->list_name, "w");
	if (flist == 0) {
	    (void) printf("list file open error\n");
	    perror(p->list_name);
	    exit(ERROR);
	}
	/*
	 * there's a bug in printf in VMS C, at least for old versions, for >512
	 * chars output at once, so...
	 */
#ifdef VMS
    {
	char           *c;
	
	for (c = p->users; *c != EOS; ++c)
	    fputc(*c, flist);
	fputc('\n', flist);
    }
#else
	(void) fputs(p->users, flist);
	(void) fputc('\n', flist);
#endif
	if (ferror(flist) != 0 || (fclose(flist) != 0)) {
	    (void) printf("list file write error\n");
	    perror(p->list_name);
	    exit(ERROR);
	}
	
	(void) fprintf(fch, "%s:%s",	/* listname */
		       p->list_name,
		       p->moderator[0] != 0 ? p->moderator[0]: "postmaster");
	for (i = 1; i < MAXMODERATOR && p->moderator[i] != 0; ++i)
	    (void) fprintf(fch, "|%s", p->moderator[i]);
	(void) fprintf(fch, ",file=%s%s,%s\n",
		       table_dir, p->list_name, 
		       p->list_name);	/* filename, description */
	strcpy(modbuff, p->list_name);
	strcat(modbuff, "-request");
	if ( p->moderator[0] == 0 || strcmp(modbuff, p->moderator[0])) {
	    (void) fprintf(fch, "%s-request:postmaster,%s", p->list_name,
			   p->moderator[0] != 0 ? p->moderator[0]: "postmaster");
	    for (i = 1; i < MAXMODERATOR && p->moderator[i] != 0; ++i)
		(void) fprintf(fch, "|%s", p->moderator[i]);
	    (void) fprintf(fch, ",Moderators for list %s\n",
			   p->list_name);	/* filename, description */
	}
    }
    inorder_lists(p->left, fch, table_dir);
}

/* remove blanks from string in place */
static int 
    trim(s)
char           *s;
{
    char           *t;
    
    for (t = s; *s != EOS; ++s)
	if (*s != ' ' && *s != '\t')
	    *t++ = *s;
    *t++ = EOS;
    return;
}


/* add users to list, converting ',' to '\n',and increasing space as needed */
/* returns 1 if a trailing comma found, else 0 */
static int 
    addusers(users, p)
char           *users;
struct list    *p;
{
    int             l = strlen(users);
    char           *s;
    int             comma;
    
    /* get enough space and some more */
    if (p->users_used + l > p->users_alloc) {
	p->users = realloc(p->users, (unsigned) (p->users_alloc + l + USERS_INCR));
	if (p->users == 0) {
	    (void) printf("out of memory!\n");
	    exit(ERROR);
	}
	p->users_alloc += (l + USERS_INCR);
    }
    /* copy across list */
    comma = 0;
    for (s = p->users + p->users_used - 1; *users != EOS; ++s, ++users)
	if (comma = (*users == ','))
	    *s = '\n';
	else
	    *s = *users;
    
    p->users_used += l;
    
    return comma;
}


static int 
    readline(buffer, len, input)
char           *buffer;
int             len;
FILE           *input;
{
    int             i;
    
    if (fgets(buffer, len, input) == 0) {
	if (!feof(input)) {
	    perror("read error");
	    exit(ERROR);
	} else
	    return EOF;
    }
    i = strlen(buffer);
    if (buffer[i - 1] != '\n')
	(void) printf("line too long (data may be lost):\n%s\n", buffer);
    else
	buffer[i - 1] = EOS;
    return 0;
}


static void panic()
{
    (void) printf("internal error!!\n");
    exit(ERROR);
}

static struct list *
    get_list(name)
char *name;
{
    register struct list **p;
    int           lr;
    
    /* Double de-reference so only one pointer needed, and
       the empty tree is not a special case */
    
    for (p = &list_pointer; *p != 0; ) {
	if ((lr = strcmp((*p)->list_name, name)) == 0){
	    return(*p);
	}
	p = (lr > 0) ? &(*p)->next : &(*p)->left;
    }
    
    /* grab some store and init it... */
    *p = (struct list *) mymalloc(sizeof(struct list));
    
    (*p)->users = mymalloc(USERS_INCR + 1);
    (*p)->users[0] = EOS;
    (*p)->users_alloc = USERS_INCR + 1;
    (*p)->users_used = 1;		/* the EOS */
    for (lr = 0; lr < MAXMODERATOR; ++lr)
	(*p)->moderator[lr] = 0;
    
    (*p)->next = (struct list *) 0;
    (*p)->left = (struct list *) 0;
    
    (*p)->list_name = mymalloc(strlen(name) + 1);
    (void) strcpy((*p)->list_name, name);	/* copy list name */
    return (*p);    
}
