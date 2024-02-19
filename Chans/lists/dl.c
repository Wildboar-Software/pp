/* dl.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl.c,v 6.0 1991/12/18 20:10:43 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl.c,v 6.0 1991/12/18 20:10:43 jpo Rel $
 *
 * $Log: dl.c,v $
 * Revision 6.0  1991/12/18  20:10:43  jpo
 * Release 6.0
 *
 */



#include "dlist.h"
#include "retcode.h"
#include <isode/quipu/ds_search.h>
#include <isode/quipu/connection.h>	/* ds_search uses di_block - include this for lint !!! */
#include <isode/quipu/dua.h>
#include <stdarg.h>
#include <isode/tailor.h>
#include "dl.h"

extern LLog    *log_dsap;
extern char * tailfile;
extern char * dsa_address;
extern char * myname;
extern char * local_dit;
extern char *postmaster;
extern Attr_Sequence current_print;
extern AttributeType at_Member;
extern AttributeType at_ObjectClass;
extern AttributeType at_Permit;
extern AttributeType at_Policy;
extern char * dn2str();
extern char * dn2ufn();

extern Filter strfilter(), ocfilter(), joinfilter();

DN localdn = NULLDN;
DN user = NULLDN;
extern int str2dl();
DN get_manager_dn();
DN mail2dn();
DN dl_str2ufn ();
char * userstr;
char * strdl = NULLCP;
char manager_mode = FALSE;
char verify = FALSE;
char quiet = FALSE;
char printdl = FALSE;
Attr_Sequence ufn_dl;

ORName * ufn_orName_parse();
extern OR_ptr orAddr_parse_user();

char * pager;
FILE * out;

ORName * user_in_list ();
ORName * addr_in_list ();

main (argc, argv)
int	argc;
char	**argv;
{
DN dn_dl = NULLDN;
int n = 1;
	sys_init(argv[0]);

	quipu_syntaxes ();

	pp_syntaxes ();
	or_myinit();

	dsap_init (&n, &argv);
	log_dsap -> ll_file = strdup ("dsap.log");

	(void) pp_quipu_run ();

	if ((pager = getenv("PAGER")) == NULL)
		pager = "more";

	if (local_dit)
		localdn = str2dn (local_dit);
	
	do_args (argc,argv, &dn_dl);

	if (dn_dl)
	      do_list (dn_dl,strdl);
	else	
	      do_select ();

	dl_unbind ();
}

do_args (argc,argv,dn_dl)
int     argc;
char    **argv;
DN	*dn_dl;
{
int x, res;
char * dsa = NULLCP;
char * passwd = NULLCP;
char buf [LINESIZE];

	for (x=1; x<argc; x++) {
		if (strcmp (argv[x], "-u") == 0) {
		     if (++x == argc) {
			    (void) fprintf (stderr,"user name missing\n");
			    exit (-1);
		     }
		     userstr = argv[x];
		     if ((user = str2dn (argv[x])) == NULLDN) {
			    (void) fprintf (stderr,"Error in DN '%s'\n",argv[x]);
			    exit (-1);
		     }
	      } else if (strcmp(argv[x], "-c") == 0) {
		     if (++x == argc) {
			    (void) fprintf (stderr,"dsa name missing\n");
			    exit (-1);
		     }
		     dsa = argv[x];
	      } else if (strcmp(argv[x], "-m") == 0) {
		     manager_mode = TRUE;
	      } else if (strcmp(argv[x], "-q") == 0) {
		     quiet = TRUE;
	      } else if (strcmp(argv[x], "-p") == 0) {
		     printdl = TRUE;
	      } else if (strcmp(argv[x], "-v") == 0) {
		     verify = TRUE;
	      } else if (*argv[x] == '-') {
		     (void) fprintf (stderr,"Usage: %s [-u <DN>] [-c <dsa>] [listname] -m -q -v -p\n",argv[0]);
		     exit (-1);
	      } else
		     strdl = argv[x];
	}

	if (printdl && !strdl) {
	    (void) fprintf (stderr,"print which list ?\n");
	    exit (-1);
        }

	if (dsa != NULLCP) {
	      FILE    *fp;
	      char    save_bdsa[LINESIZE];
	      
		(void) strcpy (myname = save_bdsa, dsa);
	      dsa_address = NULLCP;

	      /* read tailor file to get address */
	      
	      if( (fp = fopen(isodefile(tailfile, 0), "r")) == (FILE *)NULL) {
		     (void) fprintf (stderr,"can't open %s",tailfile);
		     exit (-1);
	      }

	      while(fgets(buf, sizeof(buf), fp) != NULLCP)
		     if ( (*buf != '#') && (*buf != '\n') )
			    /* not a comment or blank */
			    (void) tai_string (buf);

	      (void) fclose(fp);
	      
	      if (dsa_address == NULLCP)
		     dsa_address = myname;
	}

	if (user == NULLDN) {
		/* try ~/.quipurc */

		FILE * file;	
		char * home, *p, *part1, *part2, *TidyString();
		char Dish_Home[LINESIZE];
		char Read_in_Stuff[LINESIZE];
		
		if (home = getenv ("QUIPURC"))
		    (void) strcpy (Dish_Home, home);
		else
		    if (home = getenv ("HOME"))
			(void) sprintf (Dish_Home, "%s/.quipurc", home);
		    else
			(void) strcpy (Dish_Home, "./.quipurc");

		if ((file = fopen (Dish_Home, "r")) != 0) {
		   while (fgets (Read_in_Stuff, LINESIZE, file) != 0) {
			p = SkipSpace (Read_in_Stuff);
			if (( *p == '#') || (*p == '\0'))
				continue;  /* ignore comments and blanks */
			
			part1 = p;
			if ((part2 = index (p,':')) == NULLCP) 
				continue;
		
			*part2++ = '\0';
			part2 = TidyString (part2);

			if (lexequ (part1, "username") == 0) {
				userstr = strdup (part2);
				user = str2dn (userstr);
			} else if (lexequ (part1, "password") == 0) 
				passwd = strdup (part2);
		   }
		   (void) fclose (file);
		}
	}

	if ((user != NULLDN) && (passwd == NULLCP)) {
	      (void) sprintf (buf,"Enter password for \"%s\": ",userstr);
	      (void) strcpy (buf,getpassword (buf));
	      passwd = buf;
	      
	}

	if (dl_bind (user,passwd) == NOTOK) {
	      (void) fprintf (stderr,"Attempt to connect to DSA failed\n");
	      exit (-1);
	}

	if (strdl)
	        if ((res = str2dl (strdl,localdn, dn_dl)) != OK) {
		   if (res == NOTOK)
			(void) fprintf (stderr,"Unknown list '%s'\n",strdl);
		   else 
			(void) fprintf (stderr,"Temporary failure for '%s'\n",strdl);
		   exit (-1);
		}
}

static void openpager(ps)
PS ps;
{
	(void) ps_flush (ps);

	if (isatty(fileno((FILE *)ps->ps_addr)) == 0) {
		out = stdout;
	} else if ((out = popen(pager,"w")) == NULL) {
		(void) fprintf(stderr,"unable to start pager '%s'",pager);
		out = stdout;
	}

	ps->ps_addr = (caddr_t) out;
}	

static void closepager(ps)
PS ps;
{
	(void) ps_flush(ps);
	
	if (out != stdout) {
		(void) pclose(out);
		out = stdout;
	}

	ps->ps_addr = (caddr_t) out;
}

do_select ()
{
int	quit = FALSE;
char	buf[BUFSIZ],
	tmpbuf[BUFSIZ],
	ch;
PS	ps;
DN	dn_dl;
DN 	newloc;
int 	res;

	if ((ps = ps_alloc (std_open)) == NULLPS) {
	      (void) fprintf (stderr,"Can't set up output (3)\n");
	      return;
	}
	if (std_setup (ps, stdout) == NOTOK) {
	      (void) fprintf (stderr,"Can't set up output (4)\n");
	      return;
	}

	if (verify) {
		select_all (ps);
		return;
	}
	
	while (quit == FALSE) {
	      (void) printf("\n> ");
	      (void) fflush (stdout);
	      if (gets (buf) == NULL) {
		     (void) printf ("\n");
		     exit(OK);
	      }
	      compress (buf, buf);
	      if (buf[0] == NULL || strlen(buf) == 0) {
		     (void) printf("\nType 'h' or '?' for help\n");
		     continue;
	      }
	      if ((int)strlen(buf) > 1)
		     ch = 'A';
	      else if (*buf == '?')
		     ch = '?';
	      else
		     ch = uptolow(*buf);
	      switch (ch)
	      {
		  case 'h':
		  case '?':
		     (void) printf ("\nOptions are:\n");
		     (void) printf ("  'l'ist the list,\n");
		     (void) printf ("  'c'reate a list,\n");
		     (void) printf ("  'u'pgrade a list to a directory list,\n");
		     (void) printf ("  'w'hich lists am I on,\n");
		     (void) printf ("  'm'ove to somewhere else in the DIT,\n");
		     (void) printf ("  'q'uit,\n");
		     (void) printf ("  OR, the name of a list to visit.\n");
		     break;
		  case 'q':
		     quit = TRUE;
		     break;
		  case 'l':
		     openpager(ps);
		     dl_search (ps);
		     closepager(ps);
		     break;
		  case 'c':
		     if (!user)
			(void) fprintf (stderr,"Don't know your distinguished name !\n");
		     else if (dl_create (ps) == 0)
			(void) printf ("Done\n");
		     else
			(void) fprintf (stderr,"\nList creation failed\n");
		     break;
		  case 'u':
		     if (!user)
			(void) fprintf (stderr,"Don't know your distinguished name !\n");
		     else if (dl_upgrade (ps) == 0)
			(void) printf ("Done\n");
		     else
			(void) fprintf (stderr,"\nList upgrade failed\n");
		     break;
		  case 'w':
		     openpager(ps);
		     which_list (ps);
		     closepager(ps);
		     continue;
		  case 'm':
		     ps_print (ps,"Currently at '");
		     ufn_dn_print (ps,localdn,FALSE);
		     ps_print (ps,"'\nEnter new location: ");
		     (void) ps_flush (ps);
		     if ((gets(tmpbuf) == NULL) || (tmpbuf == NULLCP) || (*tmpbuf == 0)) {
			    ps_print (ps,"\n(exit)\n");
			    clearerr(stdin);
			    break;
		     }
		     compress (tmpbuf, tmpbuf);
		     if ((newloc = dl_str2ufn (tmpbuf)) != NULLDN) {
			(void) printf ("Moved!\n");
			localdn = newloc;
		     }
     		     break;
		  case 'A':
		      if ((res = str2dl (buf,localdn,&dn_dl)) == OK) {
			     (void) fprintf (stderr,"Reading '%s'...\n",buf);
			     do_list (dn_dl,buf);
		      } else {
			      if (res == NOTOK)
			         (void) fprintf (stderr,"Unknown list '%s'\n",buf);
			      else 
			         (void) fprintf (stderr,"Temporary failure '%s'\n",buf);
		      }
		      break;
		  default:
		     (void) printf("\nType 'h' or '?' for help\n");
		     break;
	      }
	}
	ps_free (ps);
}

static ORName * num2or (num)
int num;
{
Attr_Sequence as;
AV_Sequence avs;
int i = 1;

	if (current_print == NULLATTR) {
		(void) fprintf (stderr,"List has changed - check number!!!\n");
		return NULLORName;	
	}

	if ((as = as_find_type (current_print,at_Member)) == NULLATTR) {
		(void) fprintf (stderr,"List has changed - check number!!!\n");
		return NULLORName;	
	}

	for(avs = as->attr_value; avs != NULLAV; avs = avs->avseq_next, i++) 
		if (i == num) {
			current_print = NULLATTR;
			return (ORName *) avs->avseq_av.av_struct;
		}

	(void) fprintf (stderr,"Number too high\n");
	return NULLORName;
	
}

do_list (dn_dl,prompt)
DN dn_dl;
char * prompt;
{
Attr_Sequence as;
int	quit = FALSE;
char	buf[BUFSIZ],
	tmpbuf[BUFSIZ],
	ch;
PS	ps;
ORName * orname;
char 	can_write;
ORName * on_list;
char	re_read = FALSE;

	switch (dir_getdl_aux (dn_dl, &as)) {
	    case OK:
		break;
	    case DONE:
		(void) fprintf (stderr, "Temporary failure to read the list '%s'\n",dn2ufn(dn_dl,FALSE));
		return;
	    default:
		(void) fprintf (stderr,"Can't read the list '%s'\n",dn2ufn(dn_dl,FALSE));
		return;
	}

	if ((ps = ps_alloc (std_open)) == NULLPS) {
	      (void) fprintf (stderr,"Can't set up output\n");
	      return;
	}
	if (std_setup (ps, stdout) == NOTOK) {
	       (void)fprintf (stderr,"Can't set up output (2)\n");
	      return;
	}

	if (printdl) {
	     dl_print (ps,dn_dl);
	     return;
	}
	
	if (verify) {
		if (manager_mode)
			check_dl_members (ps,dn_dl,as,TRUE,TRUE,TRUE,quiet);
		else
			check_dl_members (ps,dn_dl,as,TRUE,TRUE,FALSE,quiet);
		return;
	}

	if (!(can_write = can_user_write (as,user))) {
		if (!quiet)
		   (void) printf("Only the manager can modify this list.\n");
		on_list = user_in_list (as,user);
	}


	while (quit == FALSE) {

		if (re_read) {
			switch (dir_getdl_aux (dn_dl, &as)){
			    case OK:
				break;
			    case DONE:
				(void) fprintf (stderr, "Temporary failure to re-read the list\n");
				return;
			    default:
				(void) fprintf (stderr,"Can't re-read the list!\n");
				return;
			}
			if (!(can_write = can_user_write (as,user)))
				on_list = user_in_list (as,user);

			re_read = FALSE;
		}

	      (void) printf("\n%s> ",prompt);
	      (void) fflush (stdout);
	      if (gets (buf) == NULL) {
		     clearerr (stdin);
		     (void) printf("\n");
		     return;
	      }
	      compress (buf, buf);
	      if (buf[0] == NULL || strlen(buf) == 0) {
		     (void) printf("\nType 'h' or '?' for help\n");
		     continue;
	      }
	      if ((int)strlen(buf) > 1)
		     ch = 'Z';
	      else if (*buf == '?')
		     ch = '?';
	      else
		     ch = uptolow(*buf);
	      switch (ch)
	      {
		  case 'h':
		  case '?':
		     (void) printf ("\nOptions are:\n");
		     (void) printf ("  'p'rint to view the list,\n");
		     (void) printf ("  'f'ind entry in the list\n");
		     (void) printf ("  'c'heck the list, reporting errors,\n");
		     if (can_write) {
			     (void) printf ("  'v'alidate the list, updating errors,\n");
			     (void) printf ("  'r'emove an entry from the list,\n");
			     (void) printf ("  'm'odify attributes (owner, policy and permission),\n");
			     (void) printf ("  'q'uit,\n");
			     (void) printf ("  OR, enter an ORName to add to the list.\n");
		     } else {
			     if (on_list) 
				     (void) printf ("  'r'emove yourself from the list,\n");
			     else if (user)
				     (void) printf ("  'a'dd yourself to the list,\n");
			     (void) printf ("  'q'uit.\n");
		     }
		     break;
		  case 'q':
		     quit = TRUE;
		     break;
		  case 'p':
		     openpager(ps);
		     dl_print (ps,dn_dl);
		     closepager(ps);
		     break;
		  case 'f':
		     (void) printf ("give name to check: ");
		     if ((gets(tmpbuf) == NULL) || (tmpbuf == NULLCP) || (*tmpbuf == 0)) {
			    ps_print (ps,"\n(exit)\n");
			    clearerr(stdin);
			    break;
		     }
		     compress (tmpbuf, tmpbuf);
		     orname = ufn_orName_parse (tmpbuf,as);

		     if (orname != NULLORName) {
			    ps_print (ps,"Found: ");
			    orName_print (ps,orname,UFNOUT);
		     }
		     break;

		  case 'r':
		     if (! can_write) {
				if (on_list) {
					if (! user)
						goto wally;
					(void) mail_manager (ps,user,TRUE,as,dn_dl);
					break;
				} 
				goto wally;
		     }
		     (void) printf ("give username to be removed: ");
		     if ((gets(tmpbuf) == NULL) || (tmpbuf == NULLCP) || (*tmpbuf == 0)) {
			    ps_print (ps,"\n(exit)\n");
			    clearerr(stdin);
			    break;
		     }
		     compress (tmpbuf, tmpbuf);
		     if (isdigit (*tmpbuf))
			orname = num2or (atoi(tmpbuf));
		     else 
			orname = ufn_orName_parse (tmpbuf,as);

		     if (orname != NULLORName) {
			  if (check_list (ps,orname,as,dn_dl,TRUE)) {
			     if (! orname_confirm(ps,orname,TRUE)) {
				    clearerr(stdin);
				    break;
			     }
			    ps_print (ps,"Removing...\n");
			    (void) dl_modify (ps,orname,dn_dl,TRUE);
			    re_read = TRUE;
		          }
		     }
		     break;
		  case 'c':
		     switch (dir_getdl_aux (dn_dl, &as)) {
			 case OK:
			     break;
			 case DONE:
			     fprintf (stderr, "Temporary failure to read the list\n");
			     break;
			 default:
			     (void) fprintf (stderr,"Can't read the list\n");
			     break;
		     }
		     openpager(ps);
		     check_dl_members (ps,dn_dl,as,TRUE,TRUE,FALSE,quiet);
		     closepager(ps);
		     break;
		  case 'v':
		     if (! can_write)
				goto wally;
		     switch (dir_getdl_aux (dn_dl, &as)) {
			 case OK:
			     break;
			 case DONE:
			     (void) fprintf (stderr, "Temporary failure to read the list\n");
			     break;
			 default:
			     (void) fprintf (stderr,"Can't read the list\n");
			     break;
		     }
		     check_dl_members (ps,dn_dl,as,TRUE,TRUE,TRUE,quiet);
		     re_read = TRUE;
		     break;
		  case 'a':
		     if (! can_write) {
			if (on_list) {
				ps_print (ps,"You are already on the list!\n");
			} else {
				if (!user)
					goto wally;
				(void) mail_manager (ps,user,FALSE,as,dn_dl);
			}
		     } else 
			     (void) printf("\nType 'h' or '?' for help\n");
		     break;
		  case 'm':
		     if (! can_write) 
			     goto wally;
		     else 
			     modify_dl_attrs (ps,dn_dl,as);
		     break;
		  case 'Z':
		     if (! can_write) {
wally:;
			if (user) {
				ps_print (ps,"\nYou do not have sufficient access rights to modify the list,\n");
				ps_print (ps,"You should contact the list manager (");
				ufn_dn_print (ps,get_manager_dn(as,TRUE),FALSE);
				ps_print (ps,").\n");
			} else {
				ps_print (ps,"\nI don't know who you are.\n");
			}
			break;

		     } else if ((orname = ufn_orName_parse (buf,NULLATTR)) != NULLORName)
			     if (check_list (ps,orname,as,dn_dl,FALSE)) {
				     if (orname_confirm(ps,orname,FALSE)) {
					     (void) dl_modify(ps,orname,dn_dl,FALSE);
					     re_read = TRUE;
				     }
			     }
		     break;
		  default:
		     (void) printf("\nType 'h' or '?' for help\n");
		     break;
	      }
	}
	ps_free (ps);
}

check_list (ps,orname,as,dn_dl,remove) 
PS ps;
ORName * orname;
Attr_Sequence as;
DN dn_dl;
char remove;
{
ORName * or;

     if (orname->on_dn) 
	if (or = user_in_list (as,orname->on_dn)) 	/* Assign */
		goto out;

     if (orname->on_or) {
	if (or = addr_in_list (as,orname->on_or)) {	/* Assign */
out:;
		if (remove) 
		    return TRUE;
	        else {
		    ps_print (ps,"'");
		    orName_print (ps,or,UFNOUT);
		    ps_print (ps,"' is already on the list!\n");
		    (void) check_ORName (ps,or,TRUE,TRUE,FALSE,dn_dl,FALSE);
		    return FALSE;
		}
	}
     }

     if (remove) {
	    ps_print (ps,"'");
	    orName_print (ps,or,UFNOUT);
	    ps_print (ps,"' is not on the list!\n");
	    return FALSE;
     }

     return check_ORName (ps,orname,TRUE,TRUE,FALSE,dn_dl,FALSE);

}

dl_search (ps) 
PS ps;
{
struct ds_search_arg search_arg;
static struct ds_search_result result;
struct DSError err;
static CommonArgs ca = default_common_args;
Filter filt;
EntryInfo * ptr;

	if ((filt = ocfilter ("ppDistributionList")) == NULLFILTER)
		return;

	search_arg.sra_baseobject = localdn;
	search_arg.sra_filter = filt;
	search_arg.sra_subset = SRA_ONELEVEL;
	search_arg.sra_searchaliases = FALSE;
	search_arg.sra_common = ca; /* struct copy */
	search_arg.sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	search_arg.sra_eis.eis_allattributes = FALSE;
	search_arg.sra_eis.eis_select = NULLATTR;

	if (ds_search (&search_arg, &err, &result) != DS_OK) {
		filter_free (filt);
		log_ds_error (&err);
		ds_error_free (&err);
		(void) fprintf (stderr,"Search returned an error !\n");
		return;
	}

	ps_print (ps,"Found the following lists:");

	for (ptr = result.CSR_entries; ptr != NULLENTRYINFO; ptr=ptr->ent_next) {
		ps_print (ps,"\n  ");
		ufn_dn_print (ps,ptr->ent_dn,FALSE);	
	}
	ps_print (ps,"\n");
}


select_all (ps)
PS ps;
{
struct ds_search_arg search_arg;
static struct ds_search_result result;
struct DSError err;
static CommonArgs ca = default_common_args;
Filter filt;
EntryInfo * ptr;
int res;
Attr_Sequence as;
static PS nps = NULLPS;

    if (nps == NULL
	    && ((nps = ps_alloc (str_open)) == NULLPS)
		    || str_setup (nps, NULLCP, BUFSIZ, 0) == NOTOK) {
	if (nps)
	    ps_free (nps), nps = NULLPS;

	return;
    }


	if ((filt = ocfilter ("ppDistributionList")) == NULLFILTER)
		return;

	search_arg.sra_baseobject = localdn;
	search_arg.sra_filter = filt;
	search_arg.sra_subset = SRA_ONELEVEL;
	search_arg.sra_searchaliases = FALSE;
	search_arg.sra_common = ca; /* struct copy */
	search_arg.sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	search_arg.sra_eis.eis_allattributes = FALSE;
	search_arg.sra_eis.eis_select = NULLATTR;

	if (ds_search (&search_arg, &err, &result) != DS_OK) {
		filter_free (filt);
		log_ds_error (&err);
		ds_error_free (&err);
		ps_printf (ps,"Can't find any lists\n");
		return;
	}

	for (ptr = result.CSR_entries; ptr != NULLENTRYINFO; ptr=ptr->ent_next) {
		nps -> ps_base = NULL, nps -> ps_cnt = 0;
		nps -> ps_ptr = NULL, nps -> ps_bufsiz = 0;

		switch (dir_getdl_aux (ptr->ent_dn, &as)) {
		    case OK:
			res = check_dl_members (nps,ptr->ent_dn,as,TRUE,TRUE,FALSE,quiet);
			if (res == NOTOK) {
			     if (manager_mode) {
				ps_print (ps,"\nErrors found in list '");
				ufn_dn_print (ps,ptr->ent_dn,FALSE);	
				ps_print (ps,"' (manager informed).\n\n");
				*--nps -> ps_ptr = NULL, nps -> ps_cnt++;
				message2manager (ptr->ent_dn,nps->ps_base);
			     } else {
				ps_print (ps,"\nErrors found in list '");
				ufn_dn_print (ps,ptr->ent_dn,FALSE);	
				*--nps -> ps_ptr = NULL, nps -> ps_cnt++;
				ps_printf (ps,"'\n%s\n\n",nps->ps_base);
			     }
			}
			break;
		    case DONE:
			break;
		    default:
			ps_printf (ps,"Can't read the list '%s'\n",dn2ufn(ptr->ent_dn,FALSE));
		}
	}
}

which_list (ps)
PS ps;
{
struct ds_search_arg search_arg;
static struct ds_search_result result;
struct DSError err;
static CommonArgs ca = default_common_args;
Filter filt_dl, filt_dn, filt_oraddr, filt_or, filt_and, filt_dn2;
EntryInfo * ptr;
AttributeValue av;
ORName * or, * orName_parse();
OR_ptr newor, as2or(), saveor;
Attr_Sequence as;

	if (user == NULLDN) {
		(void) fprintf (stderr,"You did not supply a name at the start, so I don't know who you are!\n");
		return;
	}

	or = ufn_orName_parse (userstr,NULLATTR);
	if (dn2addr (or->on_dn, &as) == OK) 
		newor = as2or (as);

	if (newor == NULLOR) {
		(void) fprintf (stderr,"Can't work out your mail adddress\n");
		return;
	}
	or->on_or = newor;

	av = AttrV_alloc ();
	av->av_struct = (caddr_t) or;

	av->av_syntax = at_Member->oa_syntax;

	filt_dn = filter_alloc ();
	filt_dn->flt_next = NULLFILTER;
	filt_dn->flt_type = FILTER_ITEM;
	filt_dn->FUITEM.fi_type = FILTERITEM_EQUALITY;
	filt_dn->FUITEM.UNAVA.ava_type = AttrT_cpy(at_Member);
	filt_dn->FUITEM.UNAVA.ava_value = AttrV_cpy (av);

	filt_dn2 = filter_alloc ();
	filt_dn2->flt_next = filt_dn;
	filt_dn2->flt_type = FILTER_ITEM;
	filt_dn2->FUITEM.fi_type = FILTERITEM_EQUALITY;
	filt_dn2->FUITEM.UNAVA.ava_type = AttrT_cpy(at_Member);
	saveor = or->on_or;
	or->on_or = NULLOR;
	filt_dn2->FUITEM.UNAVA.ava_value = AttrV_cpy (av);
	or->on_or = saveor;

	dn_free (or->on_dn);
	or->on_dn = NULLDN;

	filt_oraddr = filter_alloc ();
	filt_oraddr->flt_next = filt_dn2;
	filt_oraddr->flt_type = FILTER_ITEM;
	filt_oraddr->FUITEM.fi_type = FILTERITEM_EQUALITY;
	filt_oraddr->FUITEM.UNAVA.ava_type = AttrT_cpy(at_Member);
	filt_oraddr->FUITEM.UNAVA.ava_value = av;

	filt_or = joinfilter (filt_oraddr,FILTER_OR);

	if ((filt_dl = ocfilter ("ppDistributionList")) == NULLFILTER)
		return;

	filt_dl->flt_next = filt_or;

	filt_and = joinfilter (filt_dl,FILTER_AND);

	search_arg.sra_baseobject = localdn;
	search_arg.sra_filter = filt_and;
	search_arg.sra_subset = SRA_ONELEVEL;
	search_arg.sra_searchaliases = FALSE;
	search_arg.sra_common = ca; /* struct copy */
	search_arg.sra_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	search_arg.sra_eis.eis_allattributes = FALSE;
	search_arg.sra_eis.eis_select = NULLATTR;

	if (ds_search (&search_arg, &err, &result) != DS_OK) {
		filter_free (filt_and);
		log_ds_error (&err);
		ds_error_free (&err);
		(void) fprintf (stderr,"Search failed\n");
		return;
	}
	filter_free (filt_and);

	ps_print (ps,"You are on the following:");

	for (ptr = result.CSR_entries; ptr != NULLENTRYINFO; ptr=ptr->ent_next) {
		ps_print (ps,"\n  ");
		ufn_dn_print (ps,ptr->ent_dn,FALSE);	
	}
	ps_print (ps,"\n");
}

can_user_write (as,thisuser)
Attr_Sequence as;
DN thisuser;
{
DN manager;

	if (manager_mode)
		return TRUE;

	if (thisuser == NULLDN)
		return FALSE;

	if ((manager = get_manager_dn (as,TRUE)) == NULLDN)
		return FALSE;

	if (dn_cmp (manager, thisuser) == 0)
		return TRUE; 
	else
		return FALSE;

}

ORName * user_in_list (as,thisuser)
Attr_Sequence as;
DN thisuser;
{
ORName  * or;
Attr_Sequence tmp;
AV_Sequence eptr;

	if (thisuser == NULLDN)
		return NULLORName;

	if ((tmp = as_find_type (as,at_Member)) == NULLATTR)
		return NULLORName;

	for (eptr = tmp->attr_value; eptr != NULLAV; eptr = eptr->avseq_next) {
		or = (ORName *)eptr->avseq_av.av_struct;
		if (dn_cmp (or->on_dn,thisuser) == 0)
			return or;
	}

	return NULLORName;
}


ORName * addr_in_list (as,addr)
Attr_Sequence as;
OR_ptr addr;
{
ORName  * or;
Attr_Sequence tmp;
AV_Sequence eptr;

	if (addr == NULLOR)
		return NULLORName;

	if ((tmp = as_find_type (as,at_Member)) == NULLATTR)
		return NULLORName;

	for (eptr = tmp->attr_value; eptr != NULLAV; eptr = eptr->avseq_next) {
		or = (ORName *)eptr->avseq_av.av_struct;
		if (orAddr_cmp (or->on_or,addr) == 0)
			return or;
	}

	return NULLORName;
}


DNS dl_interact (dns,dn,s)
DNS dns;
DN dn;
char * s;
{
char    buf[LINESIZE];
DNS result = NULLDNS;
DNS newdns;
PS	ps;

	if ((ps = ps_alloc (std_open)) == NULLPS) {
	      (void) fprintf (stderr,"Can't set up output (5)\n");
	      return NULLDNS;
	}
	if (std_setup (ps, stdout) == NOTOK) {
	      (void) fprintf (stderr,"Can't set up output (6)\n");
	      return NULLDNS;
	}
	
	if (dns == NULLDNS)
		return NULLDNS;

	ps_printf (ps,"Please select from the following (matching '%s'):\n",s);
	while (dns != NULLDNS) {
		ps_print (ps,"   ");
		(void) ufn_dn_print_aux (ps,dns->dns_dn,dn,TRUE);
		if (ufn_dl) {
			if (user_in_list (ufn_dl,dns->dns_dn)) {
				DNS tmp;
				ps_print (ps,"(Discarded - not in list)\n");
				tmp = dns;
				dns = dns->dns_next;
				tmp->dns_next = NULLDNS;
				dn_seq_free (tmp);
				continue;
			}
		}
		ps_print (ps," [y/n] ? ");
		(void) ps_flush (ps);

again:;
		if (gets (buf) == NULL) {
			clearerr (stdin);
			ps_print (ps,"(exit)\n");
			return result;
		}

		if ((buf[0] == NULL) 
			|| (strlen(buf) != 1)
			|| ((buf[0] != 'y') && (buf[0] != 'n'))) {
				ps_print (ps,"Please type 'y' or 'n': ");
				(void) ps_flush (ps);
				goto again;
			}

		if (buf[0] == 'y') {
			newdns = dn_seq_alloc();
			newdns->dns_next = result;
			newdns->dns_dn = dn_cpy (dns->dns_dn);
			result = newdns;
			dns = dns->dns_next;
		} else {
			DNS tmp;
			tmp = dns;
			dns = dns->dns_next;
			tmp->dns_next = NULLDNS;
			dn_seq_free (tmp);
		}
	}
	ps_free (ps);
	return result;
}

DN dl_str2ufn (ufn)
char * ufn;
{
DNS dns = NULLDNS;
int n;
char * v[20];
PS	ps;
extern char ufn_notify;
extern int print_parse_errors, parse_status;
static envlist el = NULLEL;
DN dn;
char * ptr;
int old;

	ptr = strdup (ufn);

	if ((ps = ps_alloc (std_open)) == NULLPS) {
	      (void) fprintf (stderr,"Can't set up output (7)\n");
	      return NULLDN;
	}
	if (std_setup (ps, stdout) == NOTOK) {
	      (void) fprintf (stderr,"Can't set up output (8)\n");
	      return NULLDN;
	}

	if (index (ptr,'@') != NULLCP) {
		/* DN or 822 address */

		old = print_parse_errors;
		print_parse_errors = FALSE;

		if ((dn = str2dn (ptr)) != NULLDN) {
			free (ptr);
			print_parse_errors = old;
			return dn;
		}

		parse_status--;		/* ignore error */
		print_parse_errors = old;

		if ((dn = mail2dn (ps, ufn)) != NULLDN) {
			free (ptr);	
			return dn;
		}
	}
	free (ptr);

	if (el == NULLEL) {
		ufn_notify = TRUE;
		if ((el = read_envlist ()) == NULLEL) {
		      (void) fprintf (stderr,"Can't read environment\n");
		      return NULLDN;
		}
	}


	if ((n = sstr2arg (ufn,20,v,",\n")) == NOTOK) {
		(void) fprintf (stderr, "Can't parse input !!!\n");
		return NULLDN;
	}

	ps_print (ps,"Searching...\n");

	if ( ! ufn_match (n,v,dl_interact,&dns,el)) {
		(void) fprintf (stderr, "Try again later !!!\n");
		return NULLDN;
	} else {
		if (dns == NULLDNS) {
			(void) fprintf (stderr, "Nothing matched\n");
			return NULLDN;
		} else if (dns->dns_next == NULLDNS) {
			return dns->dns_dn;
		} else {
			dns = dl_interact (dns,localdn,ufn);
			if (dns == NULLDNS) {
				(void) fprintf (stderr, "Nothing matched\n");
				return NULLDN;
			} else if (dns->dns_next == NULLDNS) {
				return dns->dns_dn;
			} 
			ps_print (ps, "\nYou need to select one name!\n   ");
			return NULLDN;
		}
	}
	/* NOTREACHED */
}

ORName * ufn_orName_parse (str,thisdl)
char * str;
Attr_Sequence thisdl;
{
ORName * or, *newor;
char * ptr;
int old;
Attr_Sequence as;
OR_ptr as2or();

	or = (ORName *) smalloc (sizeof (ORName));

	if ( (ptr=index (str,'$')) == NULLCP) {
		if ((or->on_dn = dl_str2ufn (str)) == NULLDN) 
			return (NULLORName);

		ufn_dl = thisdl;

		if (dn2addr (or->on_dn, &as) == OK)
			or->on_or = as2or (as);
		else
			or->on_or = NULLOR;

		if (thisdl) {
			if (user_in_list(thisdl,or->on_dn))
				return or;

			ORName_free (or);
			(void) fprintf (stderr,"User not in list !\n");
			return NULLORName;
		}
		return or;
	}

	*ptr--= 0;
	if (isspace (*ptr)) {
		*ptr = 0;
	}
	ptr++;
	ptr++;

	if (*str == 0)
		or->on_dn = NULLDN;
	else {
		if ((or->on_dn = dl_str2ufn (str)) == NULLDN)
			return (NULLORName);
		ufn_dl = thisdl;
	}

	ptr = SkipSpace(ptr);	
	if (*ptr == 0) {
		or->on_or = NULLOR;
		if (or->on_dn != NULLDN)
			if (dn2addr (or->on_dn, &as) == OK)
				or->on_or = as2or (as);
	} else if ((or->on_or = orAddr_parse_user (ptr)) == NULLOR)
		return (NULLORName);
	
	if (thisdl) {
		if (or->on_dn && user_in_list(thisdl,or->on_dn))
			return or;
		if (or->on_or && (newor = addr_in_list(thisdl,or->on_or)))
			return newor;

		ORName_free (or);
		(void) fprintf (stderr,"User not in list !\n");
		return NULLORName;
	}
	return (or);
}

char	input[BUFSIZ];

static int getinput()
{
	RP_Buf	rp;

	(void) fflush(stdout);
	if (gets (input) == NULL) {
		clearerr(stdin);
		pps_end(NOTOK,&rp);
		return NOTOK;
	}
	compress (input, input);
	if (*input == '\0') {
		pps_end(NOTOK,&rp);
		return NOTOK;
	}
	return OK;
}

static int readinput()
{
	(void) fflush(stdout);
	if (gets (input) == NULL) {
		clearerr(stdin);
		return NOTOK;
	}
	compress (input, input);
	if (*input == '\0') 
		return NOTOK;

	return OK;
}

static int error_user()
{
	RP_Buf	rp;
	pps_end(NOTOK, &rp);
	return NOTOK;
}

static int error_pps(rp)
RP_Buf	*rp;
{
	(void) printf("pps_error: %s\n",rp->rp_line);
	(void) fflush(stdout);
	return NOTOK;
}

/**/

orname_confirm (ps,orname,remove)
PS ps;
ORName * orname;
char remove;
{
    register char   c;

    if (remove)
	    ps_printf (ps,"Remove ");
    else
	    ps_printf (ps,"Add ");
    orName_print (ps,orname,UFNOUT);

    ps_printf (ps," [y/n/d] ? ");
    ps_flush (ps);

again:;
    c = ttychar ();

    switch (c)
    {
	case 'Y':
	case 'y':
	    (void) printf ("yes\r\n");
	    (void) fflush (stdout);
	    return (TRUE);

	case 'N':
	case 'n':
	    (void) printf ("no\r\n");
	    (void) fflush(stdout);
	    return (FALSE);

	case 'D':
	case 'd':
	    (void) printf ("display...\r\n");
	    if (orname->on_dn) 
		dl_showentry (ps,orname->on_dn);
	    else 
		orName_print (ps,orname,UFNOUT);
	    if (remove)
	    	ps_printf (ps,"Remove [y/n] ? ");
	    else
	    	ps_printf (ps,"Add [y/n] ? ");
	    ps_flush (ps);
	    goto again;

	default:
	    (void) printf ("Type y(es), n(o) or (d)isplay ");
	    (void) fflush(stdout);
	    goto again;
   }
}

confirm (str)
char	*str;
{
    register char   c;

    if (str != NULL) {
	    (void) printf(str);
	    (void) fflush(stdout);
    }

again:;

    (void) printf (" [y/n] ");
    (void) fflush (stdout);

    c = ttychar ();

    switch (c)
    {
	case 'Y':
	case 'y':
	    (void) printf ("yes\r\n");
	    (void) fflush (stdout);
	    return (TRUE);

	case 'N':
	case 'n':
	    (void) printf ("no\r\n");
	    (void) fflush(stdout);
	    return (FALSE);
	default:
	    (void) printf ("Type y or n\n");
	    (void) fflush(stdout);
	    goto again;
   }
}

ttychar ()
{
    register int	c;
    char		buf[LINESIZE];

    (void) fflush (stdout);
    if (fgets (buf, LINESIZE,  stdin) == 0)
	clearerr(stdin);

    c = buf[0];

    c = toascii (c);    /* get rid of high bit */

    if (c == '\r')
	c = '\n';

    return (c);
}

dl_create_dsa (ps)
PS ps;
{
DN dn;
int  res;
char *listname;
char *description;
DN owner;

	(void) printf("\nName of new list -> ");
	if (readinput() == NOTOK)
		return NOTOK;

	if ((res = str2dl (input,localdn,&dn)) != NOTOK) {
		if (res == OK) {
			(void) printf("'%s' list already exists...\n",input);
			do_list (dn,input);
		} else
			(void) printf("Temporary directory failure\n",input);
		dn_free (dn);
		return NOTOK;
	}

	listname = strdup(input);

	(void) printf ("Do local users '%s-request' and '%s' exist ",listname,listname);

	if (confirm (NULLCP) == FALSE) {
		(void) printf("You need to create some local PP mail addresses\n");
		(void) printf("(contact the postmaster if you don't know how)\n\n");
		(void) printf("You need to create an alias of the form\n");
		(void) printf("   %s-request: alias <user>\n",listname);
		(void) printf("and a user of the form\n");
		(void) printf("   %s:dirlist [<host>]\n",listname);
		(void) printf("\nThen try again!\n\n");
		return NOTOK;
	}

	(void) printf("\nOwner of new list -> ");
	if (readinput() == NOTOK)
		return NOTOK;

	if ((owner = dl_str2ufn(input)) == NULLDN) {
		(void) printf("\nInvalid Owner");
		return NOTOK;
	}

	(void) printf("Description of list (one line only) -> ");
	if (readinput() == NOTOK)
		return NOTOK;
	description = strdup(input);

	(void) printf("Creating entry for '%s-request'...\n",listname);
	(void) fflush (stdout);
	if (! add_list_request (ps, localdn, listname, owner)) {
		return NOTOK;
	}

	(void) printf("Creating entry for '%s'...\n",listname);
	(void) fflush (stdout);
	if (! add_new_list (ps,localdn, listname, owner, description,NULLATTR)) {
		return NOTOK;
	}

	(void) str2dl (listname,localdn,&dn);
	do_list (dn,listname);

	return OK;
}

dl_create (ps)
PS ps;
{
RP_Buf	rp;
DN dn;
int  res;
char *get_spostmaster();

	postmaster = get_spostmaster();

	if (manager_mode) 
		return dl_create_dsa(ps);

	(void) printf("Please wait...");
	(void) fflush(stdout);
	
	if (pps_1adr("Directory based distribution list creation request",
		     postmaster,
		     &rp) != OK)
		return error_pps(&rp);

	(void) printf("\nName of new list -> ");
	if (getinput() == NOTOK)
		return error_user();

	if ((res = str2dl (input,localdn,&dn)) != NOTOK) {
		dn_free (dn);
		if (res == OK)
			(void) printf("'%s' list already exists\n",input);
		else
			(void) printf("Temporary directory failure\n",input);
		return error_user();
	}

	if (pps_txt("\nPlease create a directory based mail list:\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\n   cn= ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(input,&rp) != OK)
		return error_pps(&rp);

	if (pps_txt("\n   Owner= ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(dn2str(user),&rp) != OK)
		return error_pps(&rp);

	(void) printf("Description of list (one line only) -> ");
	if (getinput() == NOTOK)
		return error_user();
	if (pps_txt("\n   Description= ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(input,&rp) != OK)
		return error_pps(&rp);


	if (pps_txt("\n\n(You can use the \"dl -m\" command to do this!)\n",&rp) != OK)
		return error_pps(&rp);

	if (confirm ("Do you want to pass this request on to the postmaster ?") == FALSE) {
		pps_end(NOTOK,&rp);
		return 0;
	}

	(void) printf("Sending...");
	(void) fflush(stdout);
	if (pps_txt("\n(auto-mailing from the 'dl' tool)\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);

	(void) printf ("\nYour request has been passed to an administrator\n");
	(void) printf ("You will be notified in a short time when the list has been created\n");
	(void) printf ("You can then use this program to enter names into the list\n\n");
	(void) fflush (stdout);
	return 0;
}

dl_upgrade_dsa (ps,list,listname)
PS ps;
dl	*list;
char * listname;
{
RP_Buf	rp;
DN dn;
int  res;
char *description;
DN owner;
Attr_Sequence nas, members = NULLATTR;
Attr_Sequence mail2member ();
Name * dlm;

	if ((res = str2dl (listname,localdn,&dn)) != NOTOK) {
		if (res == OK) {
			(void) printf("'%s' list already exists...\n",input);
			do_list (dn,listname);
		} else
			(void) printf("Temporary directory failure\n",input);
		dn_free (dn);
		return NOTOK;
	}

	if ((owner = mail2dn (ps, list->dl_moderator)) == NULLDN) {
		(void) printf("\nInvalid Owner");
		return NOTOK;
	}

	description = list->dl_desc;

	for (dlm = list -> dl_list; dlm != NULL; dlm = dlm -> next) {
		if ((nas = mail2member (ps, dlm->name)) == NULLATTR) {
			(void) printf("\naddress failed '%s'",dlm->name);
			return NOTOK;
		}
		members = as_merge (members, nas);
	}

	(void) printf("Creating entry for '%s-request'...\n",listname);
	(void) fflush (stdout);
	if (! add_list_request (ps, localdn, listname, owner)) {
		return NOTOK;
	}

	(void) printf("Creating entry for '%s'...\n",listname);
	(void) fflush (stdout);
	if (! add_new_list (ps,localdn, listname, owner, description,members)) {
		return NOTOK;
	}

	(void) printf("Directory entries created.\n\n");

	(void) printf("WARNING: the address to DN mapping is experimental,\n");
	(void) printf("you should check the members of the list carefully\n\n");

	(void) printf("You need to modify the PP tables form\n");
	(void) printf("   %s:list [<host>]\nto\n",listname);
	(void) printf("   %s:dirlist [<host>]\n",listname);

	if (confirm ("Do you want to pass a request on to the postmaster ?") == FALSE) {
		(void) str2dl (listname,localdn,&dn);
		do_list (dn,listname);
		return OK;
	}

	(void) printf("Sending...");
	(void) fflush(stdout);

	if (pps_1adr("Directory based distribution list upgrade request",
		     postmaster,
		     &rp) != OK)
		return error_pps(&rp);

	if (pps_txt("\nPlease change the PP table entry for the list\n'",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(listname,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("' from 'list' to 'dirlist'\n",&rp) != OK)
		return error_pps(&rp);

	if (pps_txt("\n(auto-mailing from the 'dl' tool)\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);

	(void) str2dl (listname,localdn,&dn);
	do_list (dn,listname);
	return OK;
}

dl_upgrade (ps)
PS ps;
{
dl	*list;
RP_Buf	rp;
DN dn;
int  res;
char *get_spostmaster();

	postmaster = get_spostmaster();

	(void) printf("\nName of list to upgrade -> ");
	if (readinput() == NOTOK)
		return NOTOK;

	switch (tb_getdl (input, &list, OK)) {
	    case OK:
		break;
	    case DONE:
		if (list != NULL)
			dl_free(list);
		(void) printf ("Temporary failure to expand list");
		return NOTOK;
	    default:
		if (list != NULL)
			dl_free(list);
		(void) printf ("Failed to expand list");
		return NOTOK;
	} 

	if (manager_mode) 
		return dl_upgrade_dsa(ps,list,input);
	
	(void) printf("Please wait ...");
	(void) fflush(stdout);
	
	if (pps_1adr("Directory based distribution list upgrade request",
		     postmaster,
		     &rp) != OK)
		return error_pps(&rp);

	if ((res = str2dl (input,localdn,&dn)) != NOTOK) {
		dn_free (dn);
		if (res == OK)
			(void) printf("'%s' list already exists\n",input);
		else
			(void) printf("Temporary directory failure\n",input);
		return error_user();
	}

	if (pps_txt("\nPlease convert the file based mail list\n   ",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt(input,&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("\ninto a directory based mail list\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_txt("(You can do this using the 'dl' program)\n",&rp) != OK)
		return error_pps(&rp);

	(void) printf("\nDo you want to pass this request on to the postmaster ?");
	if (confirm (NULLCP) == FALSE) {
		pps_end(NOTOK,&rp);
		return 0;
	}

	(void) printf("Sending...");
	(void) fflush(stdout);

	if (pps_txt("\n(auto-mailing from the 'dl' tool)\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);

	(void) printf ("\nYour request has been passed to an administrator\n");
	(void) printf ("You will be notified in a short time when the list has been converted\n");
	(void) printf ("You can then use this program to enter names into the list\n\n");
	(void) fflush (stdout);
	return 0;
}


mail_manager (ps,dn,remove,as,dn_dl)
PS ps;
DN dn;
char remove;
Attr_Sequence as;
DN dn_dl;
{
DN manager, get_manager_dn();
DN nice_manager;
RP_Buf	rp;
Attr_Sequence das;
OR_ptr or,as2or ();
char man_buf [LINESIZE];

	if (remove)
		ps_print (ps,"\nSorry, you can't remove youself from the list.\n");
	else	
		ps_print (ps,"\nSorry, you can't add youself to the list.\n");

	if ((manager = get_manager_dn(as,FALSE)) == NULLDN)
		return NOTOK;
	if ((nice_manager = get_manager_dn(as,TRUE)) == NULLDN)
		return NOTOK;

	if (dn2addr (manager, &das) != OK)
		return NOTOK;

	if ((or = as2or (das)) == NULLOR)
		return NOTOK;

	or_or2rfc (or,man_buf);

	/* Could be neat - and check the submit permissions to make sure 
	 * the user is allowed on the list ! 
         */

	ps_print (ps, "Do you want to mail a request to the Manager,\n");
	ufn_dn_print (ps,nice_manager,TRUE);
	ps_print (ps, " ?");
	(void) ps_flush (ps);

	if (! confirm (NULLCP)) 
		return NOTOK;

	(void) printf("Please wait...\n");
	(void) fflush(stdout);
	
	if (pps_1adr("Directory based distribution list modification request",
		     man_buf,
		     &rp) != OK)
		return error_pps(&rp);

	if (remove) {
		if (pps_txt("\nPlease remove\n   ",&rp) != OK)
			return error_pps(&rp);
	} else {
		if (pps_txt("\nPlease add\n   ",&rp) != OK)
			return error_pps(&rp);
	}

	if (pps_txt(dn2ufn(dn,FALSE),&rp) != OK)
		return error_pps(&rp);

	if (remove) {
		if (pps_txt("\nfrom the list\n   ",&rp) != OK)
			return error_pps(&rp);
	} else {
		if (pps_txt("\nto the list\n   ",&rp) != OK)
			return error_pps(&rp);
	}

	if (pps_txt(dn2ufn(dn_dl,FALSE),&rp) != OK)
		return error_pps(&rp);

	if (pps_txt(".\n(auto-mailing from the 'dl' tool)\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);

	(void) printf ("Your request has been passed to the manager.\n");
	(void) fflush (stdout);

	return 0;

}

message2manager (dn_dl,message)
DN dn_dl;
char * message;
{
DN manager, get_manager_dn();
RP_Buf	rp;
Attr_Sequence as;
Attr_Sequence das;
OR_ptr or,as2or ();
char subject [LINESIZE];
char man_buf [LINESIZE];

	switch (dir_getdl_aux (dn_dl, &as)) {
	    case OK:
		break;
	    default:
		return NOTOK;
	}

	if ((manager = get_manager_dn(as,FALSE)) == NULLDN)
		return NOTOK;

	if (dn2addr (manager, &das) != OK)
		return NOTOK;

	if ((or = as2or (das)) == NULLOR)
		return NOTOK;

	or_or2rfc (or,man_buf);

	sprintf (subject,"Error in mailing list '%s'",dn2ufn(dn_dl,FALSE));
	if (pps_1adr(subject, man_buf, &rp) != OK)
		return error_pps(&rp);

	if (pps_txt(message,&rp) != OK)
		return error_pps(&rp);

	if (pps_txt("\n(auto-mailing from the 'dl' tool)\n",&rp) != OK)
		return error_pps(&rp);
	if (pps_end(OK,&rp) != OK)
		return error_pps(&rp);

	return 0;

}

modify_dl_attrs (ps,dn_dl,as)
PS ps;
DN dn_dl;
Attr_Sequence as;
{
Attr_Sequence tmp,ntmp;
DN dnm, ndnm, get_manager_dn();
char buffer [LINESIZE];

	if (( dnm = get_manager_dn(as,TRUE)) == NULLDN) 
		ps_print (ps,"Entry has no Owner !!!\n");
	else {
		ps_print (ps,"Owner: ");
		ufn_dn_print (ps,dnm,TRUE);

retry_manager:;
	       ps_print (ps,"\nEnter new name ('return' for no change): ");
	       ps_flush (ps);
	       if (readinput() == OK) {
		       if ( (ndnm = dl_str2ufn (input)) == NULLDN) 
			       goto retry_manager;

		       if (! dl_modify_owner (ps,dnm,ndnm,dn_dl,quiet))
			       return;
	       }

	       if (((tmp = as_find_type (as,at_Permit)) == NULLATTR) 
		   || (tmp->attr_value == NULLAV))
		       ps_print (ps,"\nEntry has no mhsDLSubmitPermissions !!!\n");
	       else {
		       ps_print (ps,"\nSubmit Permissions,");
		       avs_seq_print (ps,tmp->attr_value, UFNOUT);
	       }
	}

retry_permit:;
	ps_print (ps,"\nEnter new permission ('return' for no change): ");
	ps_flush (ps);
	if (readinput() == OK) {
		sprintf (buffer,"mhsDLSubmitPermissions=%s",input);
		if ( (ntmp = str2as (buffer)) == NULLATTR) 
			goto retry_permit;

		if (! dl_modify_attr (ps,tmp,ntmp,dn_dl,quiet))
			return;
	}

	if (((tmp = as_find_type (as,at_Policy)) == NULLATTR) 
	    || (tmp->attr_value == NULLAV))
		ps_print (ps,"\nEntry has no dl-policy\n");
	else {
		ps_print (ps,"\nList Policy, ");
		avs_seq_print (ps,tmp->attr_value, UFNOUT);
	}

retry_policy:;
	ps_print (ps,"\nEnter new policy ('return' for no change): ");
	ps_flush (ps);
	if (readinput() == OK) {
		sprintf (buffer,"dl-policy=%s",input);
		if ( (ntmp = str2as (buffer)) == NULLATTR) 
			goto retry_policy;

		if (! dl_modify_attr (ps,tmp,ntmp,dn_dl,quiet))
			return;
	}

	ps_print (ps,"\nDone!");

}

#ifndef lint

void    advise (int code, char *what, char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);

    (void) _ll_log (log_dsap, code, what, fmt, ap);

    va_end (ap);
}

#else
/* VARARGS */

void    advise (code, what, fmt)
char    *what,
	*fmt;
int      code;
{
    advise (code, what, fmt);
}
#endif
