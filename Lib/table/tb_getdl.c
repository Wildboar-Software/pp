/* tb_getdl.c: expand a distribution list */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getdl.c,v 6.0 1991/12/18 20:24:28 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/table/RCS/tb_getdl.c,v 6.0 1991/12/18 20:24:28 jpo Rel $
 *
 * $Log: tb_getdl.c,v $
 * Revision 6.0  1991/12/18  20:24:28  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"chan.h"
#include	"table.h"
#include	"dl.h"
#include	"adr.h"
#include	"retcode.h"
#include	<sys/stat.h>

ADDR		*tb_getModerator();
static Table	*List = NULLTBL;
static Name *getNames();
static Name *getNamesFromFile();
Name *new_Name();
extern	char	*compress ();
/* 
This routine returns the values:
	NOTOK	- Table or routine erroe.
	OK	- List found in Table
	DONE	- temporary error ?
*/
char	*namefile;
 
extern char	*list_tbl;

tb_getdl (key, plist,complain)
char	*key;
dl	**plist;
int	complain;
{
	char	buf[BUFSIZ],
		*ix,
		*start,
		mod[MAXPATHLENGTH],
		*str;
	int	retval = OK;
	ADDR	*adr;

	namefile = NULLCP;
	*plist = NULL;
	PP_DBG(("tb_getdl (%s)", key));

	if (List == NULLTBL) 
		if ((List = tb_nm2struct(list_tbl)) == NULLTBL) {
			PP_LOG(LLOG_EXCEPTIONS, ("tb_getdl (no table)"));
			return DONE;
		}
	
	if (tb_k2val (List, key, buf, TRUE) == NOTOK) {
		if (complain == OK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("Unable to find entry for %s in list table %s",
				key, list_tbl));
			return DONE;
		} 
		return NOTOK;
	}

	*plist = (dl *) calloc(1, sizeof(dl));
	(*plist) -> dl_listname = strdup(key);

	start = &(buf[0]);
	if ((adr = tb_getModerator(key)) == NULLADDR) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Unable to get moderator for dl '%s'",key));
		retval = DONE;
	} else {
		(*plist) -> dl_moderator = strdup(adr->ad_value);
		adr_free(adr);
	}
	if ((ix = index(start, ',')) == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("tb_getdl syntax error for '%s' : listname:[uids],...", key));
		retval = DONE;
	} else {
		*ix = '\0';
		(void) compress(start,mod);
		if (mod[0] != '\0')
			(*plist)->dl_uids = getNames(mod, &retval);
		else
			(*plist)->dl_uids = NULL;
	}

	start = ix+1;
	if (retval == OK) {
		str = start;
		ix = index(str, ',');
		if (ix != NULL)
			*ix++ = '\0';
		(*plist) -> dl_desc = ix;
		(*plist) -> dl_list = getNames(str, &retval);
		(*plist) -> dl_file = namefile;
	}

	return retval;
}

Name *new_Name(str, file)
char	*str;
char	*file;
{
	Name	*ret = (Name *) calloc(1, sizeof(Name));

	ret -> name = strdup(str);
	if (file)
		ret -> file = strdup(file);
	return ret;
}

static Name *getNames(str, pretval)
char	*str;
int	*pretval;
{
	Name	*head = NULL,
		*tail = NULL,
		*temp = NULL;
	char	*ix, *start;

	while ((ix = index(str,'|')) != NULL) {
		*ix = NULL;
		start = str;
		while (isspace(*start)) start++;

		if (strncmp(start,"file=",strlen("file=")) == 0)
			temp = getNamesFromFile(start, pretval);
		else
			temp = new_Name(start, NULLCP);
		if (head == NULL)
			head = tail = temp;
		else 
			tail -> next = temp;
		while (tail -> next != NULL)
			tail = tail->next;
		str = ix + 1;
	}

	start = str;
	while (isspace(*start)) start++;

	if (strcmp(start,"") != 0) {
		if (strncmp(start,"file=",strlen("file=")) == 0)
			temp = getNamesFromFile(start,pretval);
		else
			temp = new_Name(start,NULLCP);
		if (head == NULL)
			head = tail = temp;
		else 
			tail -> next = temp;
	}
	return head;
}

extern char	*loc_dist_prefix;

ADDR	*tb_getModerator(list)
char	*list;
{
	char	moderator[LINESIZE],
		*ix;
	ADDR	*adr;
	RP_Buf	rp;
	extern char	*getpostmaster();

	if ((ix = index(list, '@')) != NULL)
		*ix = '\0';
	
	if (strncmp(list, loc_dist_prefix, strlen(loc_dist_prefix)) == 0) {
		/* attempt to strip of loc_dist_prefix to find moderator */
		(void) sprintf(moderator, 
			       "%s-request",
			       (list+strlen(loc_dist_prefix)));
		adr = adr_new (moderator, AD_822_TYPE, 0);
		adr -> ad_resp = NO;
#ifdef UKORDER
		if (!rp_isbad(ad_parse(adr, &rp, CH_UK_PREF))) {
#else
		if (!rp_isbad(ad_parse(adr, &rp, CH_USA_PREF))) {
#endif
			if (ix != NULL) *ix = '@';
			return adr;
		}
		adr_free(adr);
	}
		
	(void) sprintf(moderator,"%s-request",list);
	adr = adr_new (moderator, AD_822_TYPE, 0);
	adr -> ad_resp = NO;
#ifdef UKORDER
	if (!rp_isbad(ad_parse(adr, &rp, CH_UK_PREF))) 
#else
	if (!rp_isbad(ad_parse(adr, &rp, CH_USA_PREF))) 
#endif
		return adr;
	adr_free(adr);
	
	adr = adr_new ((ix = getpostmaster(AD_822_TYPE)),
		       AD_822_TYPE, 0);
	adr -> ad_resp = NO;
#ifdef UKORDER
	if (rp_isbad(ad_parse(adr, &rp, CH_UK_PREF))) {
#else
	if (rp_isbad(ad_parse(adr, &rp, CH_UK_PREF))) {
#endif
		PP_OPER(NULLCP,
		       ("Failed to parse postmaster '%s' [%s]",
			ix, adr->ad_parse_message));
		adr_free(adr);
		return NULLADDR;
	}
	return adr;
}

static Name *getNamesFromFile(file,pretval)
char	*file;
int	*pretval;
{
	Name	*head = NULL,
		*tail = NULL,
		*temp;
	char	entry[LINESIZE],
		fullname[MAXPATHLENGTH],
		*ix;
	struct stat statbuf;
	extern char	*tbldfldir;
	FILE	*fd;
	int	len;

	if ((ix = index(file,'=')) == NULLCP) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("error in parsing '%s'",file));
		return NULL;
	}
	ix++;
	(void) compress(ix,ix);

	if (ix[0] != '/') 
		(void) sprintf(fullname, "%s/%s", tbldfldir, ix);
	else
		(void) sprintf(fullname, "%s", ix);

	if (namefile == NULLCP) namefile = strdup(fullname);
	
	
	if (stat (fullname, &statbuf) != OK) {
		PP_OPER(fullname, ("unable to stat file"));
		*pretval = DONE;
		return NULL;
	}

	if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
		PP_OPER(fullname, ("Not a regular file"));
		*pretval = DONE;
		return NULL;
	}

	if ((fd = fopen(fullname,"r")) == NULL) {
		PP_OPER(fullname, ("Can't open file"));
		*pretval = DONE;
		return NULL;
	}

	while (fgets(entry, LINESIZE, fd) != NULL) {
		(void) compress(entry, entry);
		if (entry[0] == '\n' || entry[0] == '#' || entry[0] == '\0')
			continue;

		len = strlen(entry);
		if (entry[len-1] == '\n') entry[len-1] = '\0';
		temp = new_Name(entry, fullname);
		if (head == NULL)
			head = tail = temp;
		else {
			tail->next = temp;
			tail = temp;
		}
	}
	(void) fclose(fd);
	return head;
}

name_free(list)
Name	*list;
{
	if (list == NULL)
		return;
	if (list -> next) 
		name_free(list -> next);
	if (list -> name)
		free(list -> name);
	if (list -> file)
		free(list->file);

	free((char *) list);
}
			
dl_free(list)
dl	*list;
{
	if (list == NULLDL)
		return;
	if (list -> dl_listname)
		free(list -> dl_listname);
	if (list -> dl_moderator)
		free(list -> dl_moderator);
	if (list -> dl_uids)
		name_free(list -> dl_uids);
	if (list -> dl_list)
		name_free(list -> dl_list);
	if (list -> dl_file)
		free(list -> dl_file);
	free((char *) list);
}
