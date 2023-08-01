/* ut_msg.c: library of routines to read a message directory struct */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_msg.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_msg.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_msg.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */



#include        "head.h"
#include        <isode/usr.dirent.h>
#include        <sys/stat.h>
#include        <isode/cmd_srch.h>


#define NULLDIR ((DIR*)0)
#define NULLDCT ((struct dirent *)0)

extern CMD_TABLE
		qtbl_con_type    [/* content-type */];

extern char     *quedfldir;
extern int      errno;
static int      fcmp();

static DIR      *ST_dir = NULLDIR;
static char     *ST_array[BUFSIZ];
static char     *ST_name = NULLCP;
static int      ST_curr = NULL;
static int      ST_level = NULL;
static int      ST_no = NULL;

int	msg_rend ();

static void UM_array_free ();
static struct dirent *UM_readdir ();
static int UM_isdir ();
static int UM_rfile ();
static void UM_push ();
static struct dirent *UM_pop ();
static int fcmp ();
static int recur_fcmp ();

/* ------------------------  External  Routines  -------------------------- */



static int basedir_len;

int msg_rinit (dir)
char    *dir;
{
	PP_DBG (("Lib/pp/msg_rinit: (%s)", dir));

	if (ST_dir)  (void) msg_rend();

	ST_level = 0;
	ST_name = malloc (LINESIZE);
	bzero (ST_name, LINESIZE);

	(void) strcpy (ST_name, dir);


	if (!UM_isdir (ST_name)) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/pp/msg_rinit: %s is not a dir", ST_name));
		return (RP_FOPN);
	}

	if ((ST_dir = opendir (ST_name)) == NULLDIR) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/pp/msg_rinit: unable to open %s", ST_name));
		return (RP_FOPN);
	}

	basedir_len = strlen(ST_name);
	
	return (RP_OK);

}


int msg_rfile (buf)
char    *buf;
{
	char    *ptr;

	PP_DBG (("Lib/pp/msg_rfile: (%s)", ST_name));

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


int msg_rend()
{
	PP_DBG (("Lib/pp/msg_rend: (%s)", ST_name));

	if (ST_dir) {
		(void) closedir (ST_dir);
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


static int fcmp (f1, f2)
register char   **f1, **f2;
{
	char	*stripedf1,
		*stripedf2;

	stripedf1 = (*f1)+basedir_len+1;
	stripedf2 = (*f2)+basedir_len+1;

	return recur_fcmp(stripedf1,stripedf2);
}

static int recur_fcmp(f1,f2)
register char	*f1,
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




static int UM_rfile (buf)
char    *buf;
{
	struct dirent *dp;

	if (ST_dir == NULLDIR) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/pp/UM_rfile: opendir not performed"));
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


static struct dirent *UM_pop()
{
	char            tbuf[LINESIZE],
			*ptr;
	struct dirent *dp;

	--ST_level;
	ptr = rindex (ST_name, '/');
	(void) strcpy (tbuf, ++ptr);
	*--ptr = '\0';

	PP_DBG (("Lib/pp/UM_pop: (%s, %d)", ST_name, ST_level));

	(void) closedir (ST_dir);
	ST_dir = opendir (ST_name);

	dp = UM_readdir (&tbuf[0]);

	if (dp)
		return (dp);

	if (ST_level)
		return (UM_pop());

	return (NULLDCT);
}


static void UM_push (name)
char    *name;
{
	char    tbuf[LINESIZE];

	(void) sprintf (tbuf, "%s/%s", ST_name, name);

	(void) strcpy (ST_name, tbuf);

	(void) closedir (ST_dir);

	PP_DBG (("Lib/pp/UM_push: (%s, %d)", ST_name, ST_level+1));

	if ((ST_dir = opendir (ST_name)) == NULLDIR) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Lib/pp/UM_push: Unable to open %s", ST_name));
		return;
	}

	++ST_level;
}


static int UM_isdir (name)
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

	if (stat (&tbuf[0], &st_rec) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, tbuf, 
			 ("Lib/pp/UM_isdir: Unable to stat"));
		return (FALSE);
	}

	switch (st_rec.st_mode & S_IFMT) {
	case S_IFDIR:
		PP_DBG (("Lib/pp/UM_isdir (%s = TRUE)", &tbuf[0]));
		return (TRUE);
	default:
		PP_DBG (("Lib/pp/UM_isdir (%s = FALSE)", &tbuf[0]));
		return (FALSE);
	}
}


static struct dirent *UM_readdir (current)
char    *current;
{
	struct dirent   *dp;
	int             passed_current = FALSE;

	if (current)
		PP_DBG (("Lib/pp/UM_readdir (current = %s)", current));

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
		PP_DBG (("Lib/pp/UM_readdir (%s)", dp->d_name));

	return (dp);
}



static void UM_array_free ()
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
