/* mlist: list maintaining and viewing tool */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/mlist/RCS/mlist.c,v 6.0 1991/12/18 20:31:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/mlist/RCS/mlist.c,v 6.0 1991/12/18 20:31:28 jpo Rel $
 *
 * $Log: mlist.c,v $
 * Revision 6.0  1991/12/18  20:31:28  jpo
 * Release 6.0
 *
 */



#include	"head.h"
#include	"adr.h"
#include	"or.h"
#include	"dl.h"
#include 	"alias.h"
#include	"chan.h"
#include 	"retcode.h"
#include	"ap.h"
#include	<pwd.h>
#include	<sys/stat.h>
#include	"sys.file.h"
#include	<signal.h>
#include	<isode/cmd_srch.h>

int (*work_proc) ();
int (*menu_proc) ();

static int init();
static void user_init();
static int display();
static int display_menu();
static int modify();
static int modify_menu();
static int verifyList();
static int verifyUser();
static int inList();
static int addUser();
static int isPP();
static int removeUser();
static int findUser();
static void openpager();
static void closepager();
static void changeAdrType();
static int m_createList();
static int m_alterInLine();
static int m_addReq();
static int m_delReq();
static int m_createListFile();
static int m_alterExpansion();
static int permission();
static void listLists();
static int isModerator();
static int listOnLocal();
static void master_modify();
static void restrict_modify();
static char *lowerfy ();
extern int errno;
extern char *compress();

/* copied from Lib/tai/tai_chan.c */
static CMD_TABLE   chtbl_ad_order[] = {
	"usa",			CH_USA_ONLY,
	"us",			CH_USA_ONLY,
	"uk",			CH_UK_ONLY,
	"usapref",		CH_USA_PREF,
	"uspref",		CH_USA_PREF,
	"ukpref",		CH_UK_PREF,
	0,			-1
	};


extern Name	*new_Name();
extern char	*loc_dom_site,
		*loc_dom_mta,
		*loc_dist_prefix,
		*list_tbl_comment,
		*ad_getlocal();
extern char	*tbldfldir;
char	*pager;
FILE	*out;
char	*list_table_file, *userAlias;
int	haveUid, userId;
int	superUser,
	curmode,
	menudriven,
#ifdef UKORDER
	dmnorder = CH_UK_PREF,
#else
	dmnorder = CH_USA_ONLY,
#endif
	pipeopen = 0,
	adrType = AD_822_TYPE;

typedef	struct _desc {
	char	*name;
	char	*desc;
} Desc;

Desc	*Descriptions = NULL;
int	noLists = 0;
static void init_descs();

main (argc, argv)
int	argc;
char	**argv;
{
	int i, args;

	init(argv[0]);

	superUser = FALSE;
	menudriven = TRUE;

	if (strcmp(argv[0], "malias") == 0) {
		work_proc = display;
		menu_proc = display_menu;
	} else {
		work_proc = modify;
		menu_proc = modify_menu;
	}
	i = 1;
	userAlias = NULLCP;
	haveUid = NOTOK;
	args = TRUE;

	while (args == TRUE && i < argc) {
		if (lexequ(argv[i], "-help") == 0) {
			printf("%s [-order 'domain_order'] [-user 'username'] [-uid 'user Id'] [-x] [-r] [listname ...]\n",
			       argv[0]);
			exit(0);
		} else if (lexequ(argv[i], "-x") == 0) {
			adrType = AD_X400_TYPE;
		} else if (lexequ(argv[i], "-r") == 0) {
			adrType = AD_822_TYPE;
		} else if (lexequ(argv[i], "-user") == 0) {
			/* run as -user foo */
			if (isPP() == NOTOK) {
				fprintf(stderr,
					"You are not a mail superuser and so cannot use the '%s' flag\n",
					argv[i]);
				exit(1);
			}
			if (i+1 < argc) 
				userAlias = argv[++i];
			else {
				fprintf(stderr,
					"No user name given with flag '%s'\n",
					argv[i]);
				exit (0);
			}
		} else if (lexequ(argv[i], "-uid") == 0) {
			/* run as -uid 6 */
			if (isPP() == NOTOK) {
				fprintf(stderr,
					"You are not a mail superuser and so cannot use the '%s' flag\n",
					argv[i]);
				exit(1);
			}
			if (i+1 < argc) {
				haveUid = OK;
				userId = atoi(argv[++i]);
			} else {
				fprintf (stderr,
					 "No user id given with flag '%s'\n",
					 argv[i]);
				exit (0);
			}
		} else if (lexequ(argv[i], "-order") == 0) {
			if (i+1 < argc) {
				if ((dmnorder = cmd_srch(argv[++i], 
							 chtbl_ad_order)) == NOTOK) {
					fprintf(stderr,
						"Unknown domain ordering '%s'\n",
						argv[i]);
					exit (0);
				}
			} else {
				fprintf(stderr,
					"No domain ordering given with flag '%s'\n",
					argv[i]);
				exit (0);
			}
		} else
			args = FALSE;
		if (args == TRUE)
			i++;
	}
					
	user_init();
		
	if (i == argc)
		(*menu_proc)();
	else {
		menudriven = FALSE;
		for (; i < argc; i++)
			(*work_proc) (argv[i]);
	}
}

/*  */
/* initialisation routines */
static Table	*List = NULLTBL;

extern	char	*list_tbl;

static int init(myname)
char	*myname;
{
	char	buf[BUFSIZ];
	sys_init(myname);
	or_myinit();
	if ((pager = getenv("PAGER")) == NULL)
		pager = "more";
	if ((List = tb_nm2struct(list_tbl)) == NULLTBL) {
		fprintf(stderr, "Cannot initialise table 'list'\n");
		exit(1);
	}
	if (List->tb_file[0] == '/')
		strcpy(buf, List->tb_file);
	else
		sprintf(buf,"%s/%s",tbldfldir,List->tb_file);
	list_table_file = strdup(buf);
}

struct passwd	*pwd;
ADDR		*adr;
extern char	*postmaster,
		*pplogin;


static int isPP()
{
	RP_Buf	rp;
	int	uid;
	struct passwd	*tmp;
	ADDR	*tmpadr;

	if ((uid = getuid()) == 0)
		/* super user */
		return OK;
	if ((tmp = getpwuid(uid)) == NULL) {
		fprintf(stderr, "Cannot get your passwd entry\n");
		exit(1);
	}
	tmpadr = adr_new(tmp->pw_name, AD_ANY_TYPE, 0);

	if (rp_isbad(ad_parse(tmpadr, &rp, dmnorder))) {
		fprintf(stderr,
			"Cannot parse your mail address\nReason : %s\n",
			rp.rp_line);
		exit(1);
	}

	if ((strcmp(tmp->pw_name, pplogin) == 0)
	    || (strcmp(tmpadr->ad_r822adr, postmaster) == 0)) {
		adr_free(tmpadr);
		return OK;
	}
	adr_free(tmpadr);
	return NOTOK;
}
	
static void user_init ()
{
	RP_Buf	rp;
	if (userAlias != NULLCP) {
		if ((pwd = getpwnam(userAlias)) == NULL) {
			fprintf(stderr,
				"Cannot get passwd entry for user '%s'\n",
				userAlias);
			exit (1);
		}
		printf("Running as user '%s'\n", userAlias);
	} else if (haveUid == OK) {
		if ((pwd = getpwuid(userId)) == NULL) {
			fprintf (stderr,
				 "Cannot get passwd entry for uid '%d'\n",
				 userId);
			exit (1);
		}
		printf("Running as uid '%d'\n", userId);
	} else {
		if ((pwd = getpwuid(getuid())) == NULL) {
			fprintf(stderr, "Cannot get your passwd entry\n");
			exit(1);
		}
	}
	adr = adr_new(pwd->pw_name, AD_ANY_TYPE, 0);

	if (rp_isbad(ad_parse(adr, &rp, dmnorder))) {
		fprintf(stderr,
			"Cannot parse your mail address\nReason : %s\n",
			rp.rp_line);
		exit(1);
	}
	if ((strcmp(pwd->pw_name, pplogin) == 0)
	    || (strcmp(adr->ad_r822adr, postmaster) == 0)) {
		printf ("You are a mail super-user\n");
		superUser = TRUE;
	}
}
	
/*  */
/* display mode routines */

static char *getx400name(name)
char	*name;
{
	OR_ptr	or;
	char	buf[BUFSIZ];

	or = or_buildpn(name);
	or = or_default(or);
	or_or2std(or, buf, 0);
	or_free(or);
	return strdup(buf);
}

static dl *readList(name)
char	*name;
{
	dl *list;
	char	newname[BUFSIZ], *x400name, *local;
	
	if (tb_getdl(name,&list,OK) == OK
	    || list != NULL)
		return list;

	/* try local variant of list */
	if ((local = ad_getlocal(name,(name[0] == '/') ? AD_X400_TYPE : AD_822_TYPE)) != NULLCP) {
		if (tb_getdl(local, &list,OK) == OK
		    || list != NULL) {
			free(local);
			return list;
		}
		free(local);
	}

	/* try various conversion on name */

	/* with locdomsite */
	sprintf(newname, "%s@%s",name,loc_dom_site);
	if (tb_getdl(newname,&list,OK) == OK
	    || list != NULL)
		return list;

	/* with locdommta */
	sprintf(newname, "%s@%s",name,loc_dom_mta);
	if (tb_getdl(newname,&list,OK) == OK
	    || list != NULL)
		return list;

	/* x400 type address */
	x400name = getx400name(name);
	if (tb_getdl(x400name, &list,OK) == OK
	    || list != NULL) {
		free(x400name);
		return list;
	}
	free(x400name);
	return NULL;
}

static int writeList(list)
dl	*list;
{
	Name	*ix;

	openpager();

	if (strncmp(list->dl_listname, loc_dist_prefix, strlen(loc_dist_prefix)) == 0) {
		char	*ix2 = list->dl_listname + strlen(loc_dist_prefix);
		if (pipeopen)
			fprintf(out, "Local Expansion of List: %s\t%s\n-------------------------\n",
				ix2, list->dl_desc);
	} else {
		if (pipeopen)
			fprintf(out,"List: %s\t%s\n-----\n",list->dl_listname,list->dl_desc);
	}
	if (pipeopen)
		fprintf(out,"Moderator's mail address: %s\n",
			(list -> dl_moderator == NULL) ? postmaster : list -> dl_moderator);
	if (isModerator(list)) {
		if ((ix = list -> dl_uids) != NULL) {
			if (pipeopen)
				fprintf(out,"Moderator%s uids: ",
					(ix -> next == NULL) ? "" : "s");
			while (ix != NULL) {
				if (pipeopen)
					fprintf(out,"%*s%s\n",
						(ix == list -> dl_uids) ? 0 : strlen("Moderators uids: "),
						"",
						ix -> name);
				ix = ix -> next;
			}
		}
	}

	ix = list -> dl_list;
	if (pipeopen)
		fprintf(out,"Expands to -> ");
	while (ix != NULL && pipeopen) {
		if (pipeopen)
			fprintf(out,"%*s%s\t%s\n",
				(ix == list -> dl_list) ? 0 : strlen("Expands to -> "),
				"",
				ix -> name,
				(ix -> file == NULL) ? "{inline-member}" : "");
		ix = ix -> next;
	}
	if (pipeopen)
		fprintf(out,"\n");
	closepager();
}
		
static int display(name)
char	*name;
{
	dl	*list;
	char	distList[BUFSIZ];

	(void) sprintf(distList, "%s%s", loc_dist_prefix, name);

	if ((list = readList(name)) != NULL
	    || (list = readList(distList)) != NULL) {
	    if (permission(list) != SECRMODE) {
		    writeList(list);
		    dl_free(list);
	    } else
		    printf("You do not have permission to read this list\n");
	} else
		printf("unknown list '%s'\n",name);
}

static int display_menu()
{
	int	quit = FALSE;
	char	buf[BUFSIZ], *tix;

	while (quit == FALSE) {
		printf("\n> ");
		fflush (stdout);
		if (gets (buf) == NULL)
			exit(OK);
		(void) compress (buf, buf);
		if (buf[0] == NULL || strlen(buf) == 0) {
			printf("\nType 'h' or '?' for help\n");
			continue;
		}
		if (strlen(buf) == 1)
			switch (buf[0])
			{
			    case '\0':
			    case '\n':
				printf("\nType 'h' or '?' for help\n");
				continue;
			    case 'q':
			    case 'Q':
				printf("\nBye...\n");
				quit = TRUE;
				continue;
			    case 't':
			    case 'T':
				tix = &(buf[0]);
				while (*tix != '\0'
				       && !isspace(*tix))
					tix ++;
				while (*tix != '\0'
				       && isspace(*tix))
					tix++;
				if (tix == buf)
					tix++;
				changeAdrType(tix);
				continue;
			    case 'h':
			    case 'H':
			    case '?':
				fprintf(stdout,
					"\nOptions are:\n'l' ist the lists,\n'q' uit,\n't'ype - change type of addresses you're inputing,\n\nor the name of the list you wish to view\n");
				continue;
			    case 'l':
			    case 'L':
				listLists();
				continue;
			    default:
				break;
			}
		display(buf);
	}
}

/*  */
/* modify mode routines */

static int modify(name)
char	*name;
{
	dl		*list;
	char		distList[BUFSIZ];
	struct stat	statbuf;
	int 		fd;
	char	buf[BUFSIZ], *inListname;

	(void) sprintf(distList, "%s%s", loc_dist_prefix, name);
	if ((list = readList(name)) == NULL
	    && (list = readList(distList)) == NULL) {
		fprintf(stderr, "Unknown list '%s'\n",name);
		return NOTOK;
	}
	
	/* check in line or in file */
	if (list->dl_file == NULLCP) {
		if (confirm("This list is an in-line list\nThis program is unable to modify in-line lists\nDo you want to send a request to change this ?") == TRUE)
			m_alterInLine(list);
		dl_free(list);
		return NOTOK;
	}

/*	if (listOnLocal(list -> dl_listname, hostname) == FALSE) {
		printf("List '%s' is expanded on machine '%s'\n",name,hostname);
		if (isModerator(list) == TRUE) {
			if (confirm("do you wish to mail a request to change the expansion point ?") == TRUE)
				m_alterExpansion(list);
		} else
			printf("This tool can not modify remotely expanded lists\n");
		dl_free(list);
		return NOTOK;
	}*/

	if (stat(list->dl_file, &statbuf) < 0) {
		if (isModerator(list) == FALSE) {
			sprintf(buf, "File for list '%s' can only be created by a moderator\nDo you wish to send a message to the list moderator ?",name);
			if (confirm(buf) == TRUE)
				m_createListFile(list);
			dl_free(list);
			return NOTOK;
		}
		
		/* work out modes for creation of file */
		printf("Creating file for list '%s'\n",name);
		printf("Do you want other users to be able to add / remove their own names ?");
		if (confirm (NULLCP) == TRUE) {
			printf("Do you want them to be able to add / remove anyone else's name ?");
			if (confirm(NULLCP) == TRUE)
				curmode = FREEMODE;
			else
				curmode = PUBMODE;
		} else {
			printf("Do you want others to be able to see who is on the list ?");
			if (confirm (NULLCP))
				curmode = PRIVMODE;
			else
				curmode = SECRMODE;
		}
		
		printf ("Creating file '%s' with list mode ", list->dl_file);
		switch (curmode) {
		    case FREEMODE:
			printf("free\n");
			break;
		    case PUBMODE:
			printf("public\n");
			break;
		    case PRIVMODE:
			printf("private\n");
			break;
		    case SECRMODE:
			printf("secret\n");
			break;
		}
		if ((fd = creat(list->dl_file, curmode)) < 0) {
			printf("Unable to create file '%s'\n",list->dl_file);
			dl_free(list);
			return NOTOK;
		}
		(void) fchmod(fd, curmode);
		
	} else {
		curmode = (int) statbuf.st_mode;
		curmode = curmode & 0777;
		
		if (!(isModerator(list)
		      || curmode == PUBMODE
		      || curmode == FREEMODE)) {
			printf ("list '%s' can only be updated by it's moderators\n",name);
			if (curmode != SECRMODE) {
				printf("Do you want to see who is on the list ? ");
				if (confirm (NULLCP)) 
					display(name);
			}
			
			if (inList(pwd -> pw_name, list -> dl_list, &inListname) == TRUE) {
				printf("You (%s) are already in this list\nDo you wish to mail a request to be removed",inListname);
				if (confirm(NULLCP) == TRUE)
					m_delReq(list);
			} else {
				printf ("Do you wish to mail a request to be added");
				if (confirm(NULLCP) == TRUE)
					m_addReq(list);
			}
			dl_free(list);
			return NOTOK;
		}
	}

	/* all prelimarys done */
	printf("Modifying list '%s'\n",name);
	if (isModerator(list) || curmode == FREEMODE)
		master_modify(list);
	else
		restrict_modify(list);
	return OK;
}
				
static int modify_menu()
{
	int	quit = FALSE;
	char	buf[BUFSIZ],
	*margv[20],
	ch;
	int	margc;

	while (quit == FALSE) {
		printf("\n> ");
		fflush (stdout);
		if (gets (buf) == NULL)
			exit(OK);
		compress (buf, buf);
		if (buf[0] == NULL || strlen(buf) == 0) {
			printf("\nType 'h' or '?' for help\n");
			continue;
		}
		margc = sstr2arg(buf, 20, margv, " \t");
		if ((int)strlen(margv[0]) > 1)
			ch = 'A';
		else if (margv[0][0] == '?')
			ch = '?';
		else
			ch = uptolow(margv[0][0]);
		switch (ch)
		{
		    case '\0':
		    case '\n':
			printf("\nType 'h' or '?' for help\n");
			continue;
		    case 'h':
		    case '?':
			printf("\nOptions are:\n'l' ist the lists,\n'c' reate a list,\n");
			printf("'p' rint 'listname' to view a list,\n'q' uit,\n't'ype - change type of addresses you're inputing,\n\nor the name of the list you wish to modify\n");
			continue;
		    case 'q':
			printf("\nBye...\n");
			quit = TRUE;
			continue;
		    case 't':
			changeAdrType(margv[1]);
			continue;
		    case 'l':
			listLists();
			continue;
		    case 'c':
			m_createList();
			continue;
		    case 'p':
			if (margc > 1)
				display(margv[1]);
			continue;
		    default:
			break;
		}
		if (modify(margv[0]) == NOTOK)
			printf("\nType 'h' or '?' for help\n");
	}

}

/*  */
/* routines to send messages to postmaster or moderators */

char	input[BUFSIZ];

static int getinput()
{
	fflush(stdout);
	if (gets (input) == NULL) {
		exit (1);
	}
	compress (input, input);
	if ((strlen(input) == 0)
	    || ((strlen(input) == 1) && (input[0] == '\n'))) {
		printf("\t-> ");
		return getinput();
	}
	return OK;
}

static int error_user()
{
	RP_Buf	rp;
	pps_end(NOTOK, &rp);
}

static int error_pps(rp)
RP_Buf	*rp;
{
	fprintf(stderr, "pps_error: %s\n",rp->rp_line);
	fflush(stderr);
}

static int m_createList()
{
	dl	*list;
	RP_Buf	rp;
	printf("Please wait while I initialise things ...");
	fflush(stdout);
	
	if (pps_1adr("Distribution/mailing List creation request",
		     postmaster,
		     &rp) != OK)
		return error_pps(&rp);
	printf("done\n");
	printf("Name of new list -> ");
	if (getinput() == NOTOK)
		return error_user();
	if ((list = readList(input)) != NULL) {
		printf("'%s' list already exists ('%s')\n",
		       input,
		       list -> dl_listname);
		dl_free(list);
		return error_user();
	}
	if (pps_txt("\nName of new list: ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(input,&rp) != OK)
		return error_pps(&rp);
	printf("\nDescription of list (one line only) -> ");
	if (getinput() == NOTOK)
		return error_user();
	if (pps_txt("\nFunction: ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(input,&rp) != OK)
		return error_pps(&rp);
	printf("\nModerators login names (separated by , again one line only)\n\t-> ");
	if (getinput() == NOTOK)
		return error_user();
	if (pps_txt("\nModerators: ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(input,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\n",&rp) != OK)
		return error_pps(&rp);

	printf("\nDo you want to pass this message on to the postmaster ?");
	if (confirm (NULLCP) == FALSE) {
		pps_end(NOTOK,&rp);
		return 0;
	}
	printf("Please wait while I tidy things up ...");
	fflush(stdout);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);
	printf("done\n");
	fflush(stdout);
	printf ("Your request has been passed to an administrator\n");
	printf ("You will be notified in a short time when the list has been created\n");
	printf ("You can then use this program to enter names into the list\n\n");
	fflush (stdout);
	return 0;
}

static int m_alterInLine(list)
dl	*list;
{
	RP_Buf	rp;
	printf("Sending message to postmaster...");
	fflush(stdout);
	if (pps_1adr("Distribution/mailing list alteration request",
		     postmaster,
		     &rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\nPlease could you change the list ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(list->dl_listname,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\nfrom being inline to being contained within a file\n\nThank you\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);
	printf("sent\n");
	fflush(stdout);
	return 0;
}

static int m_addReq(list)
dl	*list;
{
	RP_Buf	rp;
	printf("Sending request message to moderator...");
	fflush(stdout);
	if (pps_1adr("List addition request",
		     (list -> dl_moderator == NULL) ? postmaster : list -> dl_moderator,
		     &rp) != OK) 
		return error_pps(&rp);

	if (pps_txt("\nPlease add me to the list ",&rp) != OK) 
		return error_pps(&rp);
	if (pps_txt(list -> dl_listname,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\n\nThank you.\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);
	printf("sent\n");
	fflush(stdout);
	return 0;
}

static int m_delReq(list)
dl	*list;
{
	RP_Buf	rp;
	printf("Sending request message to moderator...");
	fflush(stdout);
	if (pps_1adr("Deletion from list request",
		     (list -> dl_moderator == NULL) ? postmaster : list -> dl_moderator,
		     &rp) != OK) 
		return error_pps(&rp);

	if (pps_txt("\nPlease remove me from the list ",&rp) != OK) 
		return error_pps(&rp);
	if (pps_txt(list -> dl_listname,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\n\nThank you.\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);
	printf("sent\n");
	fflush(stdout);
	return 0;
}

static int m_createListFile(list)
dl	*list;
{
	RP_Buf	rp;
	printf("Sending request message to moderator...");
	fflush(stdout);
	if (pps_1adr("List file creation request",
		     (list -> dl_moderator == NULL) ? postmaster : list -> dl_moderator,
		     &rp) != OK) 
		return error_pps(&rp);

	if (pps_txt("Please create the file for the list ",&rp) != OK) 
		return error_pps(&rp);
	if (pps_txt(list -> dl_listname,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\n\nThank you.\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);
	printf("sent\n");
	fflush(stdout);
	return 0;
}

static int m_alterExpansion(list)
dl	*list;
{
	RP_Buf	rp;
	printf("Please wait while I initialise things ...");
	fflush(stdout);
	if (pps_1adr("List expansion alteration request",
		     postmaster,
		     &rp) != OK)
		return error_pps(&rp);

	if (pps_txt("\nPlease change the host the list ",&rp) != OK) 
		return error_pps(&rp);
	if (pps_txt(list -> dl_listname,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(" is distributed from\nto ->",&rp) != OK)
		return error_pps(&rp);
	printf("done\n");
	printf("Please input the host the list should be distrubuted from ->");
	if (getinput() == NOTOK)
		return error_user();
	if (pps_txt(input,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\n\nThank you.\n",&rp) != OK)
		return error_pps(&rp);

	printf("Please wait while I tidy things up ...");
	fflush(stdout);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);
	printf("done\n");
	fflush(stdout);
	return 0; 
}

/*  */
/* general routines */
#define	INC	3

static void init_descs()
{
	int	size = INC;
	char	buf[BUFSIZ],
		*temp,
		*name,
		*ix;
	FILE	*fd;

	if ((fd = fopen(list_table_file, "r")) == NULL) {
		fprintf(stderr,
			"Unable to open list of list file '%s' : %s\n",
			list_table_file, sys_errname (errno));
		return;
	}

	Descriptions = (Desc *) calloc((unsigned int) size, sizeof(Desc));
	
	while (fgets(buf, BUFSIZ, fd) != NULLCP) {
		temp = strdup(buf);
		if (temp[0] == '\n') {
			free(temp);
			continue;
		}
		
		if (temp[0] != '#') {
			if (strncmp(temp,loc_dist_prefix,strlen(loc_dist_prefix)) == 0)
				name = temp+strlen(loc_dist_prefix);
			else
				name = temp;
			ix = index(name, ':');
			if (ix == NULL) {
				printf("unknown syntax line in list table '%s'\n",temp);
				free(temp);
				continue;
			}
			*ix++ = '\0';
			ix = rindex(ix, ',');
			if (ix != NULL)
				ix++;
		} else {
			if (strncmp(&temp[1], 
				    list_tbl_comment, 
				    strlen(list_tbl_comment)) == 0) {
				name = temp+strlen(list_tbl_comment)+1;
				ix = NULL;
			} else
				continue;
		}
		if (noLists >= size) {
			size += INC;
			Descriptions = (Desc *) realloc((char *) Descriptions, 
							(unsigned int)(size*sizeof(Desc)));
		}

		Descriptions[noLists].name = strdup(name);
		if (ix == NULLCP)
			Descriptions[noLists].desc = NULLCP;
		else
			Descriptions[noLists].desc = strdup(ix);
		noLists++;
		free(temp);
	}
	Descriptions = (Desc *) realloc((char *) Descriptions,
					(unsigned int) (noLists*sizeof(Desc)));
}

static void listLists()
{
	int	i;

	openpager();
	
	if (Descriptions == NULL)
		init_descs();
	for (i = 0; i < noLists && pipeopen; i++) {	
		if (Descriptions[i].desc == NULLCP)
			fprintf(out, "%s", Descriptions[i].name);
		else
			fprintf(out, "%-30s: %s",
				Descriptions[i].name,Descriptions[i].desc);
	}
	closepager();
}

/*  */
/* pager routines */

SFP	oldpipe;

static SFD piped(sig)
int sig;
{
	pipeopen = 0;
}

static void openpager()
{
	oldpipe = signal(SIGPIPE, piped);
	if (isatty(1) == 0)
		out = stdout;
	else if ((out = popen(pager,"w")) == NULL) {
		fprintf(stderr, "unable to start pager '%s': %s\n",
			pager, sys_errname (errno));
		out = stdout;
	}
	pipeopen = 1;
}	

static void closepager()
{
	if (out != stdout) {
		pclose(out);
		out = stdout;
	}
	pipeopen = 0;
	signal(SIGPIPE, oldpipe);
}

/**/

confirm (str)
char	*str;
{
    register char   c;

    if (str != NULL) {
	    fputs (str, stdout);
	    fflush(stdout);
    }
    printf (" [y/n] ");
    fflush (stdout);

    c = ttychar ();

    switch (c)
    {
	case 'Y':
	case 'y':
	    printf ("yes\r\n");
	    fflush (stdout);
	    return (TRUE);

	case 'N':
	case 'n':
	    printf ("no\r\n");
	    fflush(stdout);
	    return (FALSE);
	default:
	    printf ("Type y or n\n");
	    fflush(stdout);
	    return (confirm (NULLCP));
    }
}


ttychar ()
{
    register int	c;
    char		buf[LINESIZE];

    fflush (stdout);
    if (fgets (buf, LINESIZE,  stdin) == NULL)
	    exit (1);
    c = buf[0];

    c = toascii (c);    /* get rid of high bit */

    if (c == '\r')
	c = '\n';

    return (c);
}

/*  */
static int permission(list)
dl *list;
{
	struct stat	statbuf;
	int		mode;

	if (isModerator(list) == TRUE)
		/* allowed to do anything */
		return FREEMODE;
	if (list->dl_file == NULLCP
	    || stat(list->dl_file, &statbuf) < 0)
		/* no file so can't do anything */
		return TOPMODE;

	mode = (int) statbuf.st_mode;
	mode = mode & 0777;

	return mode;
}

/*  */
static int listOnLocal(name, host)
char	*name,
	host[];
{
	char	*p;

	if ((p = index(name, '@')) == NULL) {
		/* do x400 */
		sprintf(host,"unknown");
		return FALSE;
	}
	p++;
	sprintf(host,"%s",p);
	if (strcmp(p, loc_dom_mta) == 0
	    || strcmp(p, loc_dom_site) == 0)
		return TRUE;

	/* do x400 */
	return FALSE;
}
		
static int isModerator(list)
dl	*list;
{
	Name	*ix = list -> dl_uids;

	if (superUser == TRUE)
		return TRUE;

	while (ix != NULL && (strcmp(ix -> name, pwd -> pw_name) != 0))
		ix = ix -> next;

	return (ix == NULL) ? FALSE : TRUE;
}

/*  */
/* main modify work routines */
static char	*ix;

static int	getbufchar()
{
	char	ret = *ix;
	if (ret != 0) ix++;
	return (ret == 0) ? EOF : ret;
}

static void master_modify(list)
dl	*list;
{
	FILE	*curfp;
	int	quit = FALSE,
		done;
	char	tmpbuf[LINESIZE], *tix,
		buf[BUFSIZ],
		*adrstr, *orig_line, *inListname;

	if ((curfp = flckopen(list->dl_file, "r+")) == NULL) {
		fprintf (stderr, "Can't open %s: %s\n",
			 list -> dl_file, sys_errname (errno));
		return;
	}
	
	flckclose(curfp);

	while (quit == FALSE) {
		fprintf(stdout, "\n%s%s", 
			list->dl_listname,	
			(menudriven == TRUE) ? ">> " : "> ");
		fflush(stdout);
		if (gets(tmpbuf) == NULL) {
			exit(0);
		}
		compress (tmpbuf, tmpbuf);

		ix = &(tmpbuf[0]);
		switch (tmpbuf[0])
		{
		    case '\0':
		    case '\n':
			printf("\nType 'h' or '?' for help\n");
			continue;

		    case 't':
		    case 'T':
			tix = &(tmpbuf[0]);
			while (*tix != '\0'
			       && !isspace(*tix))
				tix ++;
			while (*tix != '\0'
			       && isspace(*tix))
				tix++;
			if (tix == tmpbuf)
				tix++;
			changeAdrType(tix);
			continue;

		    case 'p':
		    case 'P':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				ix = &(tmpbuf[1]);
				while (*ix != '\0'
				       && isspace(*ix)) ix++;
				display(list -> dl_listname);
				continue;
			}
		    case 'v':
		    case 'V':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				ix = &(tmpbuf[1]);
				while (*ix != '\0'
				       && isspace(*ix)) ix ++;
				verifyList(list);
				continue;
			}
			
		    case 'h':
		    case 'H':
		    case '?':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				fprintf(stdout,
					"\nOptions are:\n'p' rint list,\n'v' erify list,\n'a' dd user,\n'f' ind user,\n'r' emove users,\n'l' ist the lists,\n'c' reate a new list,\n't'ype - change type of addresses you're inputting,\n");
				if (menudriven == TRUE)
					fprintf(stdout, "'q' uit and return to top menu.\n\n");
				else
					fprintf(stdout, "'q' uit.\n\n");
				fprintf(stdout, "If the input is none of the above options,\nit is assumed to be a user name to be added\n");
				fflush (stdout);
				continue;
			}

		    case 'l':
		    case 'L':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				listLists();
				continue;
			}
			
		    case 'c':
		    case 'C':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				ix = &(tmpbuf[1]);
				while (*ix != '\0' &&
				       isspace(*ix)) ix++;
				m_createList();
				continue;
			}
		    case 'q':
		    case 'Q':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				ix = &(tmpbuf[1]);
				while (*ix != '\0'
				       && isspace(*ix)) ix ++;
				if (*ix == '\0') {
					quit = TRUE;
					continue;
				}
				ix = &(tmpbuf[0]);
				break;
			}
		    case 'r':
		    case 'R':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				ix = &(tmpbuf[1]);
				while (*ix != '\0'
				       && isspace(*ix)) ix++;
				if (*ix == '\0') {
					printf ("give username to be removed (regex): ");
					(void) fflush (stdout);
					if (gets(tmpbuf) == NULL) 
						exit(0);
					compress (tmpbuf, tmpbuf);
					ix = &(tmpbuf[0]);
				}
				removeUser(ix, list);
				continue;
			}

		    case 'f':
		    case 'F':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				ix = &(tmpbuf[1]);
				while (*ix != '\0'
				       && isspace(*ix)) ix++;

				if (*ix == '\0') {
					printf ("give username to search for (regex): ");
					if (gets(tmpbuf) == NULL) 
						exit(0);
					compress (tmpbuf, tmpbuf);
					ix = &(tmpbuf[0]);
				} 

			
				findUser(ix, list);
				continue;
			}
		    case 'a':
		    case 'A':
			if (tmpbuf[1] == '\0'
			    || isspace(tmpbuf[1])) {
				ix = &(tmpbuf[1]);
				while (*ix != '\0'
				       && isspace (*ix)) ix++;

				if (*ix == '\0') {
					printf("give username to be added: ");
					if (gets(tmpbuf) == NULL)
						exit(0);
					compress (tmpbuf, tmpbuf);
					ix = &(tmpbuf[0]);
				} 
			}

		    default:
			break;
		}
		done = FALSE;

		while (done == FALSE) {
			orig_line = ix;
			if (ap_pinit(getbufchar) == BADAP) {
				printf("Cannot parse '%s'\n",
				       orig_line);
				done = FALSE;
			} else {
				switch (ap_1adr()) {
				    case DONE:
					done = TRUE;
					break;
				    case NOTOK:
					break;
				    default:
					if (ap_t2s(ap_pstrt, &adrstr) != BADAP) {
						if (rp_isbad(verifyUser(adrstr, buf)))
							printf("Illegal address '%s'\n",
							       adrstr);
						else {
							if (inList(adrstr, list -> dl_list, &inListname) == TRUE) {
								printf("User '%s' already in list",
								       adrstr);
								if (lexequ(inListname, adrstr) != 0) 
									printf(" as '%s'", inListname);
								printf("\n");
							} else 
								addUser(adrstr, list);
						}
						free(adrstr);
						ap_pstrt = NULLAP;
					}
					break;
				}
			}
		}

	}
	if (menudriven == FALSE) 
		printf("\nBye...\n");
		
}

static void restrict_modify(list)
dl	*list;
{
	char	buf[LINESIZE],
		adr[BUFSIZ],
		*nameinlist;

	printf("Would you like to see who's in the list ?");
	if (confirm(NULLCP) == TRUE) 
		display(list -> dl_listname);
	if (!rp_isbad(verifyUser(pwd->pw_name, adr))) {
		if (inList(adr, list -> dl_list, &nameinlist) == TRUE) {
	   
			printf("You (%s) are already in this list\nDo you wish to be removed ?",pwd -> pw_name);
			if (confirm(NULLCP) == TRUE) {
				sprintf(buf, "^%s$",adr);
				removeUser(buf, list);
			} else 
				printf("You only have permission to add or remove yourself from this list\n");
		} else {
			printf ("You (%s) are not in this list\nDo you wish to be added ?", pwd -> pw_name);
			if (confirm(NULLCP) == TRUE)
				addUser(adr, list);
			else
				printf("You only have permission to add or remove yourself from this list\n");
		}
	}
			
}

/*  */
/* checking routines */

static int verifyUser(name, addr)
char	*name,
	*addr;
{
	RP_Buf	rp;
	ADDR	*temp;
	int	ret,
		type;
	
	temp = adr_new(name, adrType, 0);
	
	if (rp_isbad(ret = ad_parse(temp, &rp, dmnorder)))
		fprintf(stderr, "Failed to parse '%s' (%s)\n",
			name,
			rp.rp_line);
	if (temp -> ad_r400adr != NULL 
	    && temp->ad_type == AD_X400_TYPE)
		sprintf(addr, "%s", temp->ad_r400adr);
	else if (temp -> ad_r822adr != NULL)
		sprintf(addr, "%s", temp->ad_r822adr);
	adr_free(temp);
	
	return ret;
}

static int verifyList(list)
dl	*list;
{
	char	*name,
		buf[BUFSIZ];

	
	Name	*ix = list -> dl_list;
	while (ix != NULL) {
		printf("Checking '%s'...",ix -> name);
		if (!rp_isbad(verifyUser(ix -> name, buf))) {
			printf("ok\n");
			ix = ix -> next;
		} else {
			name = strdup(ix -> name);
			ix = ix -> next;
			removeUser(name,list);
			free(name);
		}
/*		if (temp != NULL) *temp = '@';*/
	}
}

static int inList(name, list, plistname)
char	*name;
Name	*list;
char	**plistname;
{
	while (list != NULL 
	       && ap_equ(list -> name, name) != TRUE)
		list = list -> next;
	if (list != NULL)
		*plistname = list->name;
	return (list != NULL) ? TRUE : FALSE;
}

/*  */
/* list maintaince routines */
extern char *re_comp();
extern int re_exec();

static int findUser  (name, list)
char	*name;
dl	*list;
{
	char	*diag;
	Name	*ix;

	if ((diag = re_comp(name)) != 0) {
		fprintf(stderr,
			"re_comp error for '%s' [%s]\n", name, diag);
		return;
	}
	ix = list -> dl_list;
	openpager();
	while (ix != NULL && pipeopen) {
		if (re_exec(lowerfy(ix -> name)) == 1) {
			fprintf(out, "%s\n", ix -> name);
			fflush(stdout);
		}
		ix = ix -> next;
	}
 	closepager();
}
	
static int removeUser(name, list)
char	*name;
dl	*list;
{
	char   	*diag;
	Name    *ix,
		*temp;
	char	buf[BUFSIZ];
	int	len = 0,
		found;
	FILE	*curfp;

	if ((diag = re_comp(name)) != 0) {
		fprintf (stderr,
			 "re_comp error for '%s' [%s]\n",name,diag);
		return;
	}
	if ((curfp = flckopen (list->dl_file, "r+")) == NULL) {
		fprintf (stderr, "Can't open %s: %s\n",
			 list -> dl_file, sys_errname (errno));
		return;
	}

	rewind(curfp);
	ix = list -> dl_list;
	found = FALSE;
	while (ix != NULL) {
		temp = NULL;
		sprintf(buf, "Remove user '%s' ?", ix -> name);
		if (re_exec(lowerfy(ix -> name)) == 1 
		    && confirm(buf) == TRUE) {
			/* remove from list -> dl_list */
			if (ix -> file == NULL) 
				printf("User '%s' specified in-line, cannot remove\n",ix->name);
			else {
				found = TRUE;
				temp = list -> dl_list;
				if (ix == list -> dl_list) {
					list -> dl_list = temp -> next;
				} else {
					while (temp -> next != ix)
						temp = temp -> next;
					temp -> next = ix -> next;
					temp = ix;
				}
			}
		} else if (ix -> file != NULL) {
			/* put it out again */
			fputs (ix -> name, curfp);
			fputc('\n', curfp);
			len += strlen(ix -> name) + 1;
		}
		ix = ix -> next;
		if (temp != NULL) {
			temp -> next = NULL;
			name_free(temp);
		}
	}
	ftruncate(fileno(curfp), (off_t) len);
	flckclose(curfp);
	if (found == FALSE)
		printf("No matches\n");
}

static int addUser(name, list)
char	*name;
dl	*list;
{
	FILE	*curfp;
	Name	*temp,
		*ix;

	if ((curfp = flckopen(list->dl_file, "a")) == NULL) {
		fprintf (stderr, "Can't open file %s: %s\n",
			 list -> dl_file, sys_errname (errno));
		return;
	}
	
	printf ("adding user '%s'\n", name);
	fputs (name, curfp);
	fputc ('\n', curfp);
	flckclose (curfp);
	temp = new_Name(name, list->dl_file);
	if (list -> dl_list == NULL)
		list -> dl_list = temp;
	else {
		ix = list -> dl_list;

		while (ix -> next != NULL)
			ix = ix -> next;
		ix -> next = temp;
	}
}

static char *lowerfy (s)
char	*s;
{
	static char	buf[BUFSIZ];
	char	*cp;

	for (cp = buf; *s; s++)
		if (isascii (*s) && isupper (*s))
			*cp++ = tolower (*s);
		else
			*cp ++ = *s;
	*cp = NULL;
	return buf;
}

static void changeAdrType(buf)
char	*buf;
{
	if (buf[0] == '\0')
		printf ("Currently inputing %s style addresses\n",
			(adrType == AD_X400_TYPE) ? "x400" : "rfc822");
	else if (lexnequ(buf, "x400", strlen("x400")) == 0)
		adrType = AD_X400_TYPE;
	else if (lexnequ(buf, "rfc822", strlen("rfc822")) == 0)
		adrType = AD_822_TYPE;
	else
		printf("Unknown address type '%s' - try %s or %s",
		       buf, "x400", "rfc822");
}
