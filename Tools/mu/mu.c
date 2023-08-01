/* mu.c         MTS user interface */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/mu/RCS/mu.c,v 6.0 1991/12/18 20:31:54 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/mu/RCS/mu.c,v 6.0 1991/12/18 20:31:54 jpo Rel $
 *
 * $Log: mu.c,v $
 * Revision 6.0  1991/12/18  20:31:54  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "util.h"
#include "q.h"
#include "prm.h"
#include <isode/cmd_srch.h>
#include "tb_prm.h"
#include "tb_q.h"
#include "tb_a.h"
#include "tb_dr.h"

#include <signal.h>
#include <isode/usr.dirent.h>
#include <sys/stat.h>


#include "argh.h"       /* jct only */

/* That's enough #includes - ed */

/* tailor'ed directory */
extern  char    *cmddfldir;

#define DEFAULT_FILENAME        "mu.defaults"
#define MAXNAME_LENGTH          200
#define MAXARG_COUNT            200
#define COMMENTCHAR             '#'
#define TOKENCHAR               '$'
#define REPEATCHAR              '*'
#define FILEMARKCHAR            '*'
#define DISP_INDENT             2
#define YES_STRING              "Yy"
#define NO_STRING               "Nn"
#define THREE_BAGS_FULL_STRING  ""      /* joke */
#define CONFIRM_STRING          "Confirm ( Y/N ) "

/* ------ from : ut_msg.c: library of routines to read a message directory struct */

#define NULLDIR ((DIR*)0)
#define NULLDCT ((struct dirent *)0)

extern int      errno;
extern int      fcmp();

static DIR      *ST_dir = NULLDIR;
static char     *ST_array[BUFSIZ];
static char     *ST_name = NULLCP;
static int      ST_curr = NULL;
static int      ST_level = NULL;
static int      ST_no = NULL;
static int      basedir_len;

static int UM_rfile (), UM_push (), UM_isdir (),
	UM_array_free ();
static struct dirent *UM_readdir(), *UM_pop();
/* ------ from : ut_msg.c: library of routines to read a message directory struct */

void    advise();
void    adios();
SFD sig_cleanquit();

struct argdef arg[] = { /* program specific flag definitions */

/*      flag            function        type    match */

	"default",      NULL,           '1',    0,
	"file",         NULL,           'm',    0,
	"value",        NULL,           'm',    0,
	"body",         NULL,           'm',    0,
	"tree",         NULL,           '1',    0,
	"confirm",      NULL,           '0',    0,
	"nofix",        NULL,           '0',    0,
	"prompt",       NULL,           '2',    0,
	"group",        NULL,           '1',    0,
	"logfile",      NULL,           '1',    0,
	"help",         show_args,      '0',    0,
	"-",            show_args,      '0',    0,
	"!",            NULL,           '0',    0,
	"0",            NULL,           '0',    0,
	"1",            NULL,           '0',    0,
	"2",            NULL,           '0',    0,
	"3",            NULL,           '0',    0,
	"4",            NULL,           '0',    0,
	"5",            NULL,           '0',    0,
	"",             NULL,           '?',    0
	/* "" is 'others' list, also terminates structure array  */
};

/* Other global variables */
static  struct  prm_vars        prm;
static  struct  Qstruct         q;

ADDR    *ad_originator;         /* address of sender */
ADDR    *ad_recipient;          /* recipient list */

RP_Buf  r,*rp = &r;             /* reply value structure */
int     debug[6]={0,0,0,0,0,0}; /* interactive debug flags */
int     nest_level=0;           /* to display level of recursion */
int     confirm_input = FALSE;
int     fix_input = TRUE;
FILE    *muout=stdout;          /* progress messages to... */
char    *getcwd();              /* not used */

/* 
	In addition to the LLOG_TRACE, LLOG_DEBUG calls associated
	with debug[0], debug[1] shown below, there are other
	LLOG_FATAL and LLOG_EXCEPTIONS calls.  Note that list_args
	and show_args ( in argh.c ) do not generate LLOG events.

    debug[?]    function
	0       log to muout progress messages, and PPLOG( LLOG_TRACE ...)
	1       log to muout command files as they are processed, and PPLOG( LLOG_DEBUG ...)
	2       list to muout body part files as they are io_tdata'ed
	3       list to muout structures prior to submit io_ calls
	4       abort prior to submit io_ calls
	5       append to logfile rather than overwrite
*/

/* ----------------------------------------------------------------- */
main (argc, argv)
int     argc;
char    **argv;
{
	sys_init (argv[0]);

	signal(SIGINT,sig_cleanquit);

	msg_init();     /* initialise message structures */

	in_all(argc,argv);
	do_all(argc,argv);

	if (muout != stdout)
		fclose(muout);
	exit(0);
}
/* ----------------------------------------------------------------- */
in_all(argc, argv)      /* input from original or constructed command line */
			/* CALLED RECURSIVELY */
int     argc;
char    **argv;
{
	int     scan_counter;

	if (debug[0]) {
		fprintf(muout,"in_all nest_level %d\n",nest_level);
		PP_LOG (LLOG_TRACE,("mu: in_all nest_level %d",nest_level));
	}
	/*      printf("argc=%d argv=%h\n",argc,argv); */

	scan_counter=scan_args(argc,argv);      /* prep_args() called automatically */

	get_debug(scan_counter);

	process_args(argv,scan_counter);
}
/* ----------------------------------------------------------------- */
do_all(argc, argv)      /* all post-input operations */
int     argc;
char    **argv;
{
	if (debug[0]){
		list_args(muout,"default");
		list_args(muout,"logfile");
		list_args(muout,"file");
		list_args(muout,"value");
		list_args(muout,"body");
		list_args(muout,"tree");
		list_args(muout,"confirm");
		list_args(muout,"nofix");
/*
		list_args(muout,"prompt");
		list_args(muout,"group");
*/
		list_args(muout,"");
	}

	call_args(argc,argv,muout);     /* pass output file pointer */

	if (ad_originator == NULLADDR)
		adios("","No originator specified");        /* ... and abort */

	if (ad_recipient == NULLADDR)
		adios("","No recipient specified");        /* ... and abort */

	if( debug[3] )
		print_structures();

	if( !debug[4] )
		submit_message(argv);

	return;
}
/* ----------------------------------------------------------------- */
process_args(argv,scan_counter)
char    **argv;
int     scan_counter;
{
	FILE    *fd;
	static  char    default_filename[MAXNAME_LENGTH] = DEFAULT_FILENAME;
	char    *argvtemp[MAXARG_COUNT];
	char    buf[BUFSIZ];
	char    *p, *p_end;
	int     i,n,of,dummy;

	if (scan_counter == 1) {        /* can override mu.defaults from command line only */
		if(p=nextc_arg("default",scan_counter,&n,&of))
			strcpy(default_filename,p);
		else {
			if ((fd = fopen(default_filename,"r")) == NULL )
				sprintf(default_filename,"%s/%s",cmddfldir,DEFAULT_FILENAME);
			else
				fclose(fd);
		}
		read_file(default_filename);    /* into param_, queue_structure */
	}
	/* read all -file arguments */
	while (p=nextc_arg("file",scan_counter,&n,&of)) {
		read_file(p);   /* into param_, queue_,addr_structure */
	}
	/* scan all -body arguments */
	while (p=nextc_arg("body",scan_counter,&n,&of)) {
		try_file(p);    /* open and check file length */
	}
	/* at least one call to next_arg/nextc_arg with different argument is
	necessary before looping on "body" again; "tree" will do nicely ... */

	/* scan all -tree arguments */
	while (p=nextc_arg("tree",scan_counter,&n,&of)) {
		try_directory(p);       /* check that it exists */
	}
	/* similarly... at least one call to next_arg/nextc_arg with different argument is
	necessary before looping on "tree" again; "value" will do nicely ... */

	for (n=0,of=0;(p = nextc_arg("value",scan_counter,&n,&of)) != NULL;) {
		sprintf (buf, "%s", p);
		i = 1;
		while (of-i) {
			p = nextc_arg("value",scan_counter,&dummy,&dummy);
			p_end = index( buf, '\0' );
			sprintf (p_end, " %s", p);      /* i.e. concatenate parameters */
			i++;
		}
		/* this messy construction is necessary to put argvtemp in proper format */

		of = str2arg (buf, MAXARG_COUNT, argvtemp, NULLCP);

		if ( debug[0] ) {
			dump_args(of,argvtemp);
		}
		parse_txt(of,argvtemp);
	}
	return(TRUE);
}

/* ----------------------------------------------------------------- */
get_debug(scan_counter)         /* check for action flags */
{
	char    deb[2];         /* very short string */
	char    *p;
	int     i,n,of;

	for(i=0;i<=5;i++) {
		deb[0]=('0'+i);
		deb[1]='\0';
		if (nextc_arg(deb,scan_counter,&n,&of)) {
			debug[i] = 1;
			if ( debug[0] ) {
				fprintf(muout,"debug flag %s set\n", deb);
				PP_LOG (LLOG_TRACE,("mu: debug flag %s set", deb));
			}
		}
	}
	if (nextc_arg("nofix",scan_counter,&n,&of))
		if ( debug[0] ) {
			fprintf(muout,"nofix set\n");
			PP_LOG (LLOG_TRACE,("mu: nofix set"));
		}
		fix_input=FALSE;

	if (nextc_arg("confirm",scan_counter,&n,&of))
		if ( debug[0] ) {
			fprintf(muout,"confirm set\n");
			PP_LOG (LLOG_TRACE,("mu: confirm set"));
		}
		confirm_input=TRUE;

	if (p=nextc_arg("logfile",scan_counter,&n,&of)) {
		if ( debug[0] ) {
			fprintf(muout,"logfile set to %s\n", p);
			PP_LOG (LLOG_TRACE,("mu: logfile set to %s", p));
		}
		if (muout != stdout)
			fclose(muout);

		if ( debug[5] ) {
			if ((muout = fopen(p,"a")) == NULL ) {
				advise("","unable to append to %s",p);
				muout = stdout;         /* and send to screen */
			}
		}
		else {
			if ((muout = fopen(p,"w")) == NULL ) {
				advise("","unable to write to %s",p);
				muout = stdout;
			}
		}
	}
}
/* ----------------------------------------------------------------- */
read_file(p)    /* read file *p */
char    *p;
{       
	FILE    *fd;

	if (debug[0]) {
		fprintf(muout,"start read_file : %s\n",p);
		PP_LOG (LLOG_TRACE,("mu: start read_file : %s",p));
	}
	if ((fd = fopen(p,"r")) == NULL ) {
		adios("","unable to read %s",p);        /* ... and abort */
		return;
	}
	/* Rather than call rd_prm, rd_q, rd_adr individually, each of which
	expects to receive a subset of the control parameters, each line is
	read with txt2buf to command-argument form with str2arg (was sstr2arg) then 
	offered to each of txt2prm, txt2q and txt2adr in turn to see if it
	is valid for any - if not then an error is reported. */

	rd_all(fd);
	fclose(fd);

	if (debug[0]) {
		fprintf(muout,"finish read_file : %s\n",p);
		PP_LOG (LLOG_TRACE,("mu: finish read_file : %s",p));
	}
	return;
}
/* ----------------------------------------------------------------- */
try_file(p)     /* open, check file length */
char    *p;
{       
	FILE    *fd;
	long    f;
	char    pp[MAXNAME_LENGTH],*r;

	strcpy(pp,p);
	if (r=index(pp,FILEMARKCHAR)) {
		*r = '\0';
		strcat(pp,r+1);
		if (debug[0]) {
			fprintf(muout,"body part %s FILEMARKCHAR detected : %s\n",p,pp);
			PP_LOG (LLOG_TRACE,("mu: body part %s FILEMARKCHAR detected : %s",p,pp));
		}
	}
	if ((fd = fopen(pp,"r")) == NULL )
		adios("","unable to read body part %s",pp);
	fseek(fd,0L,2);         /* eof */
	if ((f=ftell(fd)) == 0 )
		adios("","body part %s is null file",pp);
	fclose(fd);
	return;
}
/* ----------------------------------------------------------------- */
try_directory(p)        /* check that it exists */
char    *p;
{       
	char    pp[MAXNAME_LENGTH],*r;

	strcpy(pp,p);
	if (r=index(pp,FILEMARKCHAR)) {
		*r = '\0';
		strcat(pp,r+1);
		if (debug[0]) {
			fprintf(muout,"directory %s FILEMARKCHAR detected : %s\n",p,pp);
			PP_LOG (LLOG_TRACE,("mu: directory %s FILEMARKCHAR detected : %s",p,pp));
		}
	}

	if (dir_rinit(pp) != RP_OK)
		adios("","cannot initialise directory %s",pp);
	dir_rend();
	return;
}
/* ----------------------------------------------------------------- */
rd_all  (fd)
FILE    *fd;
{
	char    buf[BUFSIZ],copybuf[BUFSIZ];
	char    *argv[MAXARG_COUNT];
	int     argc,retval,repeat,repeat_offset,user_input,parse_result;
	int     tokenfound;

	for (;;) {
		retval = txt2buf (buf, fd);
		if (rp_isbad (retval))
			return (retval);

		if(debug[1])
			display(buf,nest_level);

		if (*buf == REPEATCHAR) {
			repeat = TRUE;
			repeat_offset = 1;
		}
		else {
			repeat = FALSE;
			repeat_offset = 0;
		}

		for(;;) {
			/* copy buf to copybuf and check for TOKENCHAR */
			user_input=prompt(copybuf,buf,repeat,&tokenfound);

			if (repeat && !tokenfound) {
				advise(NULL,"<<%s>>\nerror : no TOKENCHAR(s) in REPEAT line - REPEATCHAR ignored",buf);
				repeat = FALSE;
			}
			if(repeat && !user_input)       /* no more user input */
				break;
			strip(copybuf);                 /* remove trailing spaces */
			/* argc = str2arg (copybuf+repeat_offset, 50, argv, " \t\n"); */
			argc = str2arg (copybuf+repeat_offset, MAXARG_COUNT, argv, NULLCP);
			argv[argc] = NULLCP;
/*
			if (debug[0]) {
				dump_args(argc,argv);
			}
*/
			if (argc == 0) {        /* ignore empty lines */
				parse_result=OK;
			}
			else {
				parse_result=parse_txt(argc,argv);      /* parse_txt(argc,argv,ftell(fd)); */
			}
			if(repeat || (fix_input && user_input && (parse_result == NOTOK)) ) {
				continue;
			}
			else {
				break;
			}
		}
	}
}
/* ----------------------------------------------------------------- */
parse_txt(argc,argv)    /* returns OK || NOTOK */
int     argc;
char    **argv;

/* long fd_startln;     /* for _offset calculations */

{
	char    *p;
	char    buf[BUFSIZ];
	char    *nargv[MAXARG_COUNT];
	int     nargc;
	int     retval;

	if(**argv == COMMENTCHAR) {
		if (debug[0] && !debug[1]) {    /* avoids duplicatation */
			arg2vstr(BUFSIZ,BUFSIZ,buf,argv);
			display(buf,nest_level);
		}
	}
	else if(**argv == FLAGCHAR) {

		/* dump_args(-1,argv); */
		arg2vstr(BUFSIZ,BUFSIZ,buf,argv);

		/*
		fflush(muout);
		fprintf(muout,"muout flushed\n");
		*/

		if (debug[0]) {
			fprintf(muout,"arg2vstr returns <%s>\n",buf);
			PP_LOG (LLOG_TRACE,("mu: arg2vstr returns <%s>",buf));
		}
		p=malloc(strlen(buf)+6);
		sprintf(p,"DUMMY %s",buf);
		if (debug[0]) {
			fprintf(muout,"\nin_all RECURSION with %s\n\n",p);
			PP_LOG (LLOG_TRACE,("mu: in_all RECURSION with %s",p));
		}
		strip(p);                       /* remove trailing spaces */
		/* nargc = str2arg (p, 50, nargv, " \t\n"); */
		nargc = str2arg (p, MAXARG_COUNT, nargv, NULLCP);
		nargv[nargc] = NULLCP;
/*
		if (debug[0]) {
			dump_args(nargc,nargv);
		}
*/
		if (nargc != 0) {
			nest_level++;

			in_all(nargc,nargv);    /* ***** RECURSION ***** */

			nest_level--;
		}
	}
	else {
		retval = txt2prm (&prm, argv, argc);
		if (rp_isgood(retval) || retval == PRM_END)
			return(OK);

		retval = txt2q (&q, (long)0, argv, argc);
		if (rp_isgood(retval) || retval == Q_END)
			return(OK);

		if (lexequ(argv[0],"Origs") == 0) {
			if (debug[0]) {
				fprintf(muout,"Origs in input\n");
				PP_LOG (LLOG_TRACE,("mu: Origs in input"));
			}
			if (ad_originator == NULLADDR) { /* Only one Origs */
				retval = txt2adr (&ad_originator, 0L, argv, argc);
				if (rp_isgood(retval) || retval == AD_END)
					return(OK);
			}
		}
		else {
			retval = txt2adr (&ad_recipient, 0L, argv, argc);
			if (rp_isgood(retval) || retval == AD_END)
				return(OK);
		}

		/* error if get here */
		arg2vstr(BUFSIZ,BUFSIZ,buf,argv);
		advise(NULL,"error `%s' (retval=%s)",buf,digit_word(retval));
			return(NOTOK);
	}
	return(OK);
}

/* ----------------------------------------------------------------- */
print_structures()      /* print out result of merge */
{
	ADDR    *ap;

	fprintf (muout,"-----------------------------------------------------------------\n");
	prm2txt (muout,&prm);
	q2txt   (muout,&q);
	adr2txt (muout,ad_originator,AD_ORIGINATOR);
	ap = ad_recipient;
	do {
		adr2txt (muout,ap,AD_RECIPIENT);
		ap = ap->ad_next;
	} while ( ap != NULL );
	fprintf (muout,"-----------------------------------------------------------------\n");
}
/* ----------------------------------------------------------------- */
int     prompt(copybuf,buf,repeat,tokenfound)   /* user input to expand embedded TOKENCHAR's */
char    *copybuf,*buf;                          /* buf is original line from file */
int     repeat;
int     *tokenfound;
{
	char    *p;
	char    inbuf[BUFSIZ];
	int     user_input=FALSE;

	strcpy(copybuf,buf);    /* only modify copy */

	*tokenfound = FALSE;    /* to be tested with repeat */

	if( (*buf != COMMENTCHAR) && (index(buf,TOKENCHAR)) ) {
		for(;;) {       /* Loop to allow line re-input if not confirmed */
			p=copybuf;
			while (p = index(p,TOKENCHAR)) {
				p++;
				if ( *p == TOKENCHAR ) {        /* $$ - do not set tokenfound */
					*p = '\0';
					strcat(copybuf,p+1);
				}
				else {
					*tokenfound = TRUE;
					*(--p) = '\0';
					printf("\n%s%c ",copybuf,TOKENCHAR);
					scanfcr(inbuf);         /* scanf("%s",inbuf); */
					if (repeat && (strlen(inbuf) == 0)) {
						user_input=FALSE;
						return(user_input);     /* I HATE THIS */
					}
					else {
						user_input=TRUE;
						strcat(inbuf,p+1);
						strcat(p,inbuf);
					}
				}
			}
			if (user_input && confirm_input) {
				if (confirm(copybuf)) {
					break;
				}
				else {
					strcpy(copybuf,buf);
					continue;       /* re-input whole line */
				}
			}
			else {
				break;          /* confirm_input FALSE or TRUE and confirmed */
			}
		}
	}
	return(user_input);
}

/* ----------------------------------------------------------------- */
scanfcr(inbuf)  /* equiv. to scanf("%s",inbuf) with CR only allowed */
char    *inbuf;
{       
	int     i;
	char    c;

	for(i=0,inbuf[i]='\0';((inbuf[i]=getchar()) != '\n');i++) {
	}
	inbuf[i]='\0';
}
/* ----------------------------------------------------------------- */
confirm(buf)    /* offer confirmation of user input */
char    *buf;
{
	int     i;
	char    s[10];

	for(;;) {
		printf("\n%s\n\t%s",buf,CONFIRM_STRING);
		scanfcr(s);
		if (s[0] == '\0')
			continue;
		if (index(YES_STRING,s[0])) {
			i=TRUE;
			break;
		}
		if (index( NO_STRING,s[0])) {
			i=FALSE;
			break;
		}
	}
	return(i);
}
/* ----------------------------------------------------------------- */
msg_init()
{
	/*      prm_init (p); */
	q_init(&q);
	ad_init();
}
/* ----------------------------------------------------------------- */
ad_init()   /* -- initialise the linked lists -- */
{
	if (ad_recipient  != NULLADDR)  adr_tfree (ad_recipient);
	if (ad_originator != NULLADDR)  adr_tfree (ad_originator);
	ad_originator   = NULLADDR;
	ad_recipient    = NULLADDR;
}

/* ----------------------------------------------------------------- */
submit_message(argv)
char    **argv;
{
	int     type = OK;
	int     offset;
	char    *p,*r;
	char    t[MAXPATHLENGTH];            /* for copy of directory specified in -tree */
	char    dir_fname[MAXPATHLENGTH];
	char    submit_fname[MAXPATHLENGTH];
	ADDR    *ap;

	if (debug[0]) {
		fprintf(muout,"submit_message io_init\n");
		PP_LOG (LLOG_TRACE,("mu: submit_message io_init"));
	}
	if(rp_isbad(io_init(rp)))
		rp_error(rp);
	if (debug[0]) {
		fprintf(muout,"submit_message io_wprm\n");
		PP_LOG (LLOG_TRACE,("mu: submit_message io_wprm"));
	}
	if(rp_isbad(io_wprm(&prm,rp)))
		rp_error(rp);
	if (debug[0]) {
		fprintf(muout,"submit_message io_wrq\n");
		PP_LOG (LLOG_TRACE,("mu: submit_message io_wrq"));
	}
	if(rp_isbad(io_wrq(&q,rp)))
		rp_error(rp);

	ap = ad_originator;
	if (debug[0]) {
		fprintf(muout,"submit_message io_wadr originator %s\n",ap->ad_value);
		PP_LOG (LLOG_TRACE,("mu: submit_message io_wadr originator %s",ap->ad_value));
	}
	if(rp_isbad(io_wadr(ap,AD_ORIGINATOR,rp)))
		rp_error(rp);

	ap = ad_recipient;
	do {
		if (debug[0]) {
			fprintf(muout,"submit_message io_wadr recipient %s\n",ap->ad_value);
			PP_LOG (LLOG_TRACE,("mu: submit_message io_wadr recipient %s",ap->ad_value));
		}
		if(rp_isbad(io_wadr(ap,AD_RECIPIENT,rp)))
			rp_error(rp);
		ap = ap->ad_next;
	} while ( ap != NULL );

	if (debug[0]) {
		fprintf(muout,"submit_message io_adend\n");
		PP_LOG (LLOG_TRACE,("mu: submit_message io_adend"));
	}
	if(rp_isbad(io_adend(rp)))
		rp_error(rp);

	if(((p=next_arg("body",argv)) !=NULL) || ((p=next_arg("tree",argv)) != NULL)) {

		if (debug[0]) {
			fprintf(muout,"submit_message io_tinit\n");
			PP_LOG (LLOG_TRACE,("mu: submit_message io_tinit"));
		}
		if(rp_isbad(io_tinit(rp)))
			rp_error(rp);

		p=next_arg("",argv);            /* dummy to clear next_args memory */

		if(((p=next_arg("body",argv)) !=NULL) ) {
			do {
				if ((r=index(p,FILEMARKCHAR)) == NULL)
					r=p;
				else
					++r;
				/* pass pointer just after FILECHARMARK */
				if (debug[0]) {
					fprintf(muout,"submit_message io_tpart  file %s\n",r);
					PP_LOG (LLOG_TRACE,("mu: submit_message io_tpart  file %s",r));
				}
				if(rp_isbad(io_tpart(r,FALSE,rp)))
					rp_error(rp);
				if (debug[0]) {
					fprintf(muout,"submit_message io_tdata loop\n");
					PP_LOG (LLOG_TRACE,("mu: submit_message io_tdata loop"));
				}

				loop_data(p);           /* and transfer file */

				if (debug[0]) {
					fprintf(muout,"submit_message io_tdend\n");
					PP_LOG (LLOG_TRACE,("mu: submit_message io_tdend"));
				}
				if(rp_isbad(io_tdend(rp))) {
					rp_error(rp);
				}
			} while(p=next_arg("body",argv));
		}

		p=next_arg("",argv);            /* dummy to clear next_args memory */

		if(((p=next_arg("tree",argv)) !=NULL) ) {

			do {
				strcpy(t,p);            /* copy for FILCHARMARK manipulation */
				if ((r=index(t,FILEMARKCHAR))) {
					*r = '\0';
					strcat(t,r+1);
					offset = r-t;   /* offset to intended name - just after FILECHARMARK */
				}
				else {
					offset = 0;
				}

				if (debug[0]) {
					fprintf(muout,"submit_message dir_rinit %s\n",t);
					PP_LOG (LLOG_TRACE,("mu: submit_message dir_rinit %s",t));
				}
				if ( dir_rinit( t ) != RP_OK )
					adios (NULL,"Cannot initialise directory %s\n",t);

				while (dir_rfile(dir_fname) == RP_OK ) {
					if (debug[0]) {
						fprintf(muout,"mu: dir_rfile %s\n",dir_fname);
						PP_LOG (LLOG_TRACE,("mu: mu/dir_rfile %s",dir_fname));
					}

					strcpy(submit_fname,&dir_fname[offset]);

					if (debug[0]) {
						if (offset) {
							fprintf(muout,"mu: offset %d submitted name io_tpart : %s\n",offset,submit_fname);
							PP_LOG (LLOG_TRACE,("mu: offset %d submitted name io_tpart : %s",offset,submit_fname));
						}
					}
					if(rp_isbad(io_tpart(submit_fname,FALSE,rp)))
						rp_error(rp);
					if (debug[0]) {
						fprintf(muout,"submit_message io_tdata loop\n");
						PP_LOG (LLOG_TRACE,("mu: submit_message io_tdata loop"));
					}

					loop_data(dir_fname);

					if (debug[0]) {
						fprintf(muout,"submit_message io_tdend\n");
						PP_LOG (LLOG_TRACE,("mu: submit_message io_tdend"));
					}
					if(rp_isbad(io_tdend(rp))) {
						rp_error(rp);
					}
				}
				dir_rend();
			} while(p=next_arg("tree",argv));
		}
	}

	if (debug[0]) {
		fprintf(muout,"submit_message io_tend\n");
		PP_LOG (LLOG_TRACE,("mu: submit_message io_tend"));
	}
	if(rp_isbad(io_tend(rp)))
		rp_error(rp);

	if (debug[0]) {
		fprintf(muout,"submit_message io_end\n");
		PP_LOG (LLOG_TRACE,("mu: submit_message io_end"));
	}
	if(io_end(type) == NOTOK)
		rp_error(rp);
	else {
		if (debug[0]) {
			fprintf(muout,"submit_message OK\n");
			PP_LOG (LLOG_TRACE,("mu: submit_message OK"));
		}
	}
	return;
}
/* ----------------------------------------------------------------- */
loop_data(p)
char    *p;
{
	int     length;
	char    buf[BUFSIZ];
	FILE    *fd;
	char    pp[MAXNAME_LENGTH],*r;

	strcpy(pp,p);
	if (r=index(pp,FILEMARKCHAR)) {
		*r = '\0';
		strcat(pp,r+1);
	}

	if ((fd = fopen(pp,"r")) == NULL )
		adios("","unable to open body part for reading %s",pp);

	while ( (length=fread(buf,1,BUFSIZ,fd)) ) {
		if (debug[2])
			fwrite(buf,1,length,muout);
		io_tdata(buf,length);
	}
	fclose(fd);
}
/* ----------------------------------------------------------------- */
rp_error(rp)
RP_Buf  *rp;
{
	adios (NULL,"RP_ERROR %d %s\n",rp->rp_val,rp->rp_line);
}

/* ----------------------------------------------------------------- */
display (s,level)
char    *s;
int     level;
{
	fprintf(muout,"%*s%s\n",level * DISP_INDENT, "", s);
	PP_LOG (LLOG_DEBUG,("mu: input line %s", s));
}
/* ----------------------------------------------------------------- */
/* VARARGS */
void    adios (what, fmt, a, b, c, d, e, f, g, h, i, j)
char          *what,*fmt,*a,*b,*c,*d,*e,*f,*g,*h,*i,*j;
{
	PP_LOG (LLOG_FATAL,(fmt, a, b, c, d, e, f, g, h, i, j));
	advise (what, fmt, a, b, c, d, e, f, g, h, i, j);
	cleanquit ();
}
/* ----------------------------------------------------------------- */
/* VARARGS */
void    advise (what, fmt, a, b, c, d, e, f, g, h, i, j)
char           *what,*fmt,*a,*b,*c,*d,*e,*f,*g,*h,*i,*j;
{
	if (muout != stdout) {
		fprintf (muout, fmt, a, b, c, d, e, f, g, h, i, j);
		if (what)
			(void) fputc (' ', muout), perror (what);
		else
			(void) fputc ('\n', muout);
		(void) fflush (muout);
	}
	fprintf (stderr, fmt, a, b, c, d, e, f, g, h, i, j);
	if (what)
		(void) fputc (' ', stderr), perror (what);
	else
		(void) fputc ('\n', stderr);
	(void) fflush (stderr);
	PP_LOG (LLOG_EXCEPTIONS,(fmt, a, b, c, d, e, f, g, h, i, j));
}
/* ----------------------------------------------------------------- */
/* *****************  These routines taken from :- *******************
	: ut_msg.c : library of routines to read a message directory struct */

dir_rinit (dir)         /* was msg_rinit (dir) */
char    *dir;
{       /* Now pass full or relative path for each invocation - unlike msg_rinit */

	PP_DBG (("mu: dir_rinit: (%s)", dir));

	if (ST_dir)  dir_rend();

	ST_level = 0;
	ST_name = malloc (LINESIZE);
	bzero (ST_name, LINESIZE);

	/*
		full path not assumed
	*/
	(void) sprintf (ST_name, "%s", dir);

	if (!UM_isdir (ST_name)) {
		PP_LOG (LLOG_FATAL,("mu: dir_rinit: %s is not a dir", ST_name));
		return (RP_FOPN);
	}

	if ((ST_dir = opendir (ST_name)) == NULLDIR) {
		PP_LOG (LLOG_FATAL,("mu: dir_rinit: unable to open %s", ST_name));
		return (RP_FOPN);
	}

	basedir_len = strlen(ST_name);

	return (RP_OK);

}
/* ----------------------------------------------------------------- */
dir_rfile (buf)         /* was msg_rfile (dir) */
char    *buf;
{
	char    *ptr;

	PP_DBG (("mu: dir_rfile: (%s)", ST_name));

	if (ST_no == NULL) {
		while (UM_rfile (buf) == RP_OK) {
			ptr = strdup (buf);
			ST_array [ST_no] = ptr;
			++ST_no;
		}

		qsort ((char *)ST_array,
			ST_no,
			(sizeof (ST_array[0])),
			fcmp);

	}

	if (ST_curr < ST_no) {
		(void) strcpy (buf, ST_array[ST_curr]);
		++ST_curr;
		return (RP_OK);
	}

	return (RP_DONE);
}
/* ----------------------------------------------------------------- */
dir_rend()              /* was msg_rend (dir) */
{
	PP_DBG (("mu: dir_rend: (%s)", ST_name));

	if (ST_dir) {
		closedir (ST_dir);
		ST_dir = NULLDIR;
	}

	if (ST_name) {
		free (ST_name);
		ST_name = NULLCP;
	}

	if (ST_no)
		UM_array_free();

	ST_level = NULL;
	return (RP_OK);
}
/* ----------------------------------------------------------------- */
fcmp (f1, f2)
register char   **f1, **f2;
{
	char    *stripedf1,
		*stripedf2;

	stripedf1 = (*f1)+basedir_len+1;
	stripedf2 = (*f2)+basedir_len+1;

	return recur_fcmp(stripedf1,stripedf2);
}
/* ----------------------------------------------------------------- */
recur_fcmp(f1,f2)
register char   *f1,
		*f2;
{
	/* atoi stops at the first non digit */
	int f1digit = atoi(f1);
	int f2digit = atoi(f2);

	if (f1digit < f2digit)
		return -1;

	else if (f1digit > f2digit)
		return 1;
	else {
		int f1isdir, f2isdir;
		char *ixf1, *ixf2;
		/* dificult case may have to recurse */
		f1isdir = ((ixf1 = index(f1,'/')) != NULL);
		f2isdir = ((ixf2 = index(f2,'/')) != NULL);
		if (f1isdir && f2isdir)
			return recur_fcmp(++ixf1,++ixf2);
		return 0;
	}

}
/* --------------------------  Static  Routines  -------------------------- */
static UM_rfile (buf)
char    *buf;
{
	struct dirent   *dp;

	if (ST_dir == NULLDIR) {
		PP_LOG (LLOG_FATAL,("mu: UM_rfile: opendir not performed"));
		return (RP_DONE);
	}


	dp = UM_readdir (NULLCP);

	if (dp == NULLDCT)
		if (ST_level == 0)
			return (RP_DONE);
		else
			if ((dp = UM_pop()) == NULLDCT)
				return (RP_DONE);

	if (UM_isdir (dp->d_name)) {
		/*
		new subdir
		*/
		UM_push (dp->d_name);
		return (UM_rfile (buf));
	}


	if (isstr (ST_name))
		(void) sprintf (buf, "%s/%s", ST_name, dp->d_name);
	else
		(void) strcpy (buf, dp->d_name);

	return (RP_OK);

}
/* ----------------------------------------------------------------- */
static struct dirent *UM_pop()
{
	struct dirent   *dp;
	char            tbuf[LINESIZE],
			*ptr;

	--ST_level;
	ptr = rindex (ST_name, '/');
	(void) strcpy (tbuf, ++ptr);
	*--ptr = '\0';

	PP_DBG (("mu: UM_pop: (%s, %d)", ST_name, ST_level));

	closedir (ST_dir);
	ST_dir = opendir (ST_name);

	dp = UM_readdir (&tbuf[0]);

	if (dp)
		return (dp);

	if (ST_level)
		return (UM_pop());

	return (NULLDCT);
}
/* ----------------------------------------------------------------- */
static UM_push (name)
char    *name;
{
	char    tbuf[LINESIZE];

	(void) sprintf (tbuf, "%s/%s", ST_name, name);

	(void) strcpy (ST_name, tbuf);

	closedir (ST_dir);

	PP_DBG (("mu: UM_push: (%s, %d)", ST_name, ST_level+1));

	if ((ST_dir = opendir (ST_name)) == NULLDIR) {
		PP_LOG (LLOG_FATAL,("mu: UM_push: Unable to open %s", ST_name));
		return;
	}

	++ST_level;
}
/* ----------------------------------------------------------------- */
static UM_isdir (name)
char    *name;
{
	struct stat     st_rec;
	char            tbuf[LINESIZE];

	if (!isstr (name))
		return (FALSE);

	(void) strcpy (&tbuf[0], ST_name);

	if (strcmp (ST_name, name) != 0) {
		(void) strcat (&tbuf[0], "/");
		(void) strcat (&tbuf[0], name);
	}
	fprintf(muout,"mu/UM_isdir testing %s as %s\n",name,tbuf);

	if (stat (&tbuf[0], &st_rec) == NOTOK) {
		PP_LOG (LLOG_FATAL, ("mu: UM_isdir: Unable to stat %s %d",&tbuf[0], errno));
		return (FALSE);
	}

	switch (st_rec.st_mode & S_IFMT) {
	case S_IFDIR:
		PP_DBG (("mu/: UM_isdir (%s = TRUE)", &tbuf[0]));
		return (TRUE);
	default:
		PP_DBG (("mu: UM_isdir (%s = FALSE)", &tbuf[0]));
		return (FALSE);
	}
}
/* ----------------------------------------------------------------- */
static struct dirent *UM_readdir (current)
char    *current;
{
	struct dirent   *dp;
	int             passed_current = FALSE;

	if (current)
		PP_DBG (("mu: UM_readdir (current = %s)", current));

	for (dp=readdir(ST_dir); dp != NULLDCT; dp=readdir(ST_dir)) {
		if (strcmp (dp->d_name, ".")  == 0)   continue;
		if (strcmp (dp->d_name, "..") == 0)  continue;
		if (current)
			if (strcmp (dp->d_name, current) == 0) {
				passed_current = TRUE;
				continue;
			}
			else if (passed_current == FALSE)
				continue;
		break;
	}

	if (dp)
		PP_DBG (("mu: UM_readdir (%s)", dp->d_name));

	return (dp);
}
/* ----------------------------------------------------------------- */
static UM_array_free ()
{
	int     i;

	for (i=0; i < ST_no; i++)
		if (ST_array[i])  {
			free (ST_array [i]);
			ST_array[i] = NULLCP;
		}

	ST_no = NULL;
	ST_curr = NULL;
}
/* *****************  End routines taken from ut_msg ******************* */
/* ----------------------------------------------------------------- */
strip(x)        /* strip trailing spaces and tabs */
char    *x;
{
	int     i = strlen(x);
	char    *p;

	PP_DBG (("mu: strip trailing spaces & tabs from <%s>", x));

	if ( i ) {
		p = &x[ i - 1 ];
		while ( i && (*p == ' ' || *p == '\t') ) {
			*p-- = '\0';
			i--;
		}
	}
}
/* ----------------------------------------------------------------- */
dump_args(argc, argv)
int     argc;
char    **argv;
{
	int     i = 0;
	int     j = argc;

	fprintf(muout,"dump_args argc %d\n",argc);
	PP_LOG (LLOG_TRACE,("dump_args argc %d",argc));
	while (j && (argv[i] != NULLCP)) {
		fprintf(muout,"dump_args argv[%d] %s\n",i,argv[i]);
		PP_LOG (LLOG_TRACE,("dump_args argv[%d] %s",i,argv[i]));
		j--;
		i++;
	}
}

/* ARGSUSED */
SFD sig_cleanquit (sig)
int sig;
{
	cleanquit();
}
