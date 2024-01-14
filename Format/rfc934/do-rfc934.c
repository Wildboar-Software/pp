/* do-rfc934.c: routines to carry out rfc934 conversion */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/rfc934/RCS/do-rfc934.c,v 6.0 1991/12/18 20:21:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/rfc934/RCS/do-rfc934.c,v 6.0 1991/12/18 20:21:02 jpo Rel $
 *
 * $Log: do-rfc934.c,v $
 * Revision 6.0  1991/12/18  20:21:02  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<isode/usr.dirent.h>
#include	"retcode.h"
#include	<isode/cmd_srch.h>
#include	"tb_bpt84.h"


extern CMD_TABLE bptbl_body_parts84[/* x400 84 body_parts */];
extern char *hdr_822_bp;

#define EBch	'-'
#define Stuffing "- "

char	file[MAXPATHLENGTH],
	line[LINESIZE];
int	fp;
int 	more = TRUE;
int	start_depth = 0;
/* number of chars in buffer currently */
int 	noInput = 0;
int	err_fatal = FALSE;

static int recursiveproc();
static int ishdr();
static int file_link();
static void output_startmessage();
static void output_endmessage();
static int depth();
static void output_stuffing();
static void output_endbodypart();
static void output_line();
static int  output_file();
static int output_header();
static void output_ia5();
static int numBodyParts();
static int bpFile();
#define MaxCharPerInt 16

static char	*itoa(i)
int 	i;
{
	char 	buf[MaxCharPerInt];

	sprintf(buf,"%d",i);
	
	return strdup(buf);
	
}

int do_rfc934(from,to,perr)
char	*from,	/* original directory */
	*to,	/* new directory */
	**perr;
{
	char	hdr[MAXPATHLENGTH],
		*stripped_hdr,
		outfile[MAXPATHLENGTH],
		buf[BUFSIZ],
		wrkfile[MAXPATHLENGTH];
	int	result = OK,
		msgnum = 1,
		fd_in,
		first,
		noBps,
		bodynum;
	noInput = 0;
	msg_rinit(from);
	start_depth = depth(from) + 1;
	err_fatal = FALSE;

	noBps = numBodyParts(from);

	if (msg_rfile(hdr) != RP_OK) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/rfc934 directory '%s' is empty",from));
		(void) sprintf (buf,
				"directory '%s' is empty",
				from);
		*perr = strdup(buf);
		return NOTOK;
	}
	if ((stripped_hdr = rindex(hdr,'/')) == NULL)
		stripped_hdr = hdr;
	else
		stripped_hdr++;

	if (ishdr(stripped_hdr) != OK) {
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/rfc934 cannot find hdr in '%s'",hdr));
		(void) sprintf (buf,
				"Did not find valid header in message - unable to flatten");
		err_fatal = TRUE;
		*perr = strdup(buf);
		return NOTOK;
	} 

	if (result != OK)
		return NOTOK;
	if (msg_rfile(file) != RP_OK) {
		/* empty body that's ok */
		result = file_link(from,to,
				   stripped_hdr);
		return OK;
	}

	more = TRUE;
	first = TRUE;

	do {
		if (depth(file) > start_depth) {
			if (first == TRUE) {
				result = put_out_header(hdr, to, stripped_hdr);
				/* open output file */
				sprintf(outfile,"%s/1.ia5",to);
				if ((fp = open(outfile, 
					       O_WRONLY | O_CREAT | O_TRUNC, 
					       0666)) == -1) {
					PP_SLOG(LLOG_EXCEPTIONS, outfile,
						("Can't open file"));
					(void) sprintf(buf,
						       "Unable to open output file '%s'",
						       outfile);
					*perr = strdup(buf);
					return NOTOK;
				}
			}
			result = recursiveproc(msgnum++,perr);
		} else {
			strcpy(wrkfile, file);
			if (msg_rfile(file) != RP_OK)
				more = FALSE;
			if (first == TRUE) {
				if (more == FALSE) {
					/* single body part */
					/* link hdr across */
					result = file_link(from,to,
							   stripped_hdr);
					/* link single body part across */
					result = file_link(from, to, 
							   rindex(wrkfile,'/'));
					return result;
				} else {
					put_out_header(hdr, to, stripped_hdr);
					/* open output file */
					sprintf(outfile,"%s/1.ia5",to);
					if ((fp = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
						PP_SLOG(LLOG_EXCEPTIONS, outfile,
							("Can't open file"));
						(void) sprintf(buf,
							       "Unable to open output file '%s'",
							       outfile);
						*perr = strdup(buf);
						return NOTOK;
					}
					
				}
			}

			if ((fd_in = open(wrkfile, O_RDONLY)) == -1) {
				PP_SLOG(LLOG_EXCEPTIONS, wrkfile,
					("Can't open file"));
				(void) sprintf (buf,
						"Unable to open input file '%s'",
						wrkfile);
				*perr = strdup(buf);
				result = NOTOK;
			}
			if (result == OK) {
				result = output_file(fd_in,start_depth,wrkfile, noBps, &bodynum, perr);
				if (more == FALSE) 
					output_endbodypart(fp,  bodynum, start_depth);
				close(fd_in);
			}
		}
		first = FALSE;
	} while (result == OK && more == TRUE);
	close(fp);
	return result;
}

static int recursiveproc(num, perr)
int	num;
char	**perr;
/* uses external file */
{
	int 	mydepth;
	int	fd_in;
	int	result = OK;
	int	bpnum, bodynum = 0;
	char	*dir_stub = NULL, buf[BUFSIZ],
		*ix;
	int	cont,
		msgnum = 1;
	int	noBps;

	mydepth = depth(file);
	if ((ix = rindex(file,'/')) == NULL) 
		dir_stub = strdup(file);
	else{
		while (*(ix-1) == '/')
			ix--;
		*ix = '\0';
		dir_stub = strdup(file);
		*ix = '/';
	}
	if ((ix = rindex(dir_stub, '/')) == NULL)
		ix = dir_stub;
	else
		ix++;
	bpnum = atoi(ix);
	noBps = numBodyParts(dir_stub);

	output_startmessage(fp,num,bpnum,mydepth);
	do {
		/* output file with char stuffing */
		if ((fd_in = open(file, O_RDONLY)) == -1) {
			PP_SLOG(LLOG_EXCEPTIONS, file,
				("Can't open file"));
			(void) sprintf(buf,
				       "Unable to open input file '%s'",
				       file);
			result = NOTOK;
		}
		if (result == OK) {
			result = output_file(fd_in,mydepth,file,noBps,
					     &bodynum,perr);
			close(fd_in);
		}

		if (msg_rfile(file) != RP_OK)
			more = FALSE;
		else {
			if (depth(file) > mydepth)
				result = recursiveproc(msgnum++,perr);
			bodynum = 0;
		}
		cont = FALSE;
		if ((result == OK) 
		    && (more == TRUE) 
		    && (depth(file) == mydepth))
			cont = TRUE;
		if ((ix = rindex(file,'/')) != NULL) {
			while (*(ix-1) == '/')
				ix--;
			*ix = '\0';
		}
		if ((cont == TRUE)
		    && (strcmp(dir_stub,file) != 0))
			cont = FALSE;
		if (ix != NULL)
			*ix = '/';
	} while ((result == OK) && (more == TRUE) && (cont == TRUE));
	if (dir_stub != NULL) free(dir_stub);
	if (noBps > 1 && bodynum != 0)
		output_endbodypart(fp, bodynum, mydepth);
	output_endmessage(fp,num,bpnum,mydepth);
	return result;
}

static int ishdr(name)
char	*name;
{

/*	if (strcmp(name,rcmd_srch(BPT_HDR_P2,bptbl_body_parts84)) == 0)
		return OK;
	if (strcmp(name,rcmd_srch(BPT_HDR_822,bptbl_body_parts84)) == 0)
		return OK;*/
	if (strncmp(name,hdr_822_bp,strlen(hdr_822_bp)) == 0)
		return OK;
	return NOTOK;
}

/*  */
/* input and output routines */
#define	CMASK	0377 /* for making char's > 0 */

#define	Start_message "------------------------------ Start of forwarded message "

static void output_startmessage(fd, num, bpnum, deep)
int	fd;
int	num;
int	bpnum;
int	deep;
{
	char	*cnum = itoa(num);
	if (bpnum != 1)
		write(fd,"\n",strlen("\n"));
	output_stuffing(fd,deep-1);
	write(fd,Start_message,strlen(Start_message));
	write(fd,cnum,strlen(cnum));
	free(cnum);
/*	write(fd," (bodypart ",strlen("(bodypart "));
	cnum = itoa(bpnum);
	write(fd,cnum,strlen(cnum));*/
	write(fd,"\n\n",strlen("\n\n"));
	free(cnum);
}

#define End_message "------------------------------ End of forwarded message "

static void output_endmessage(fd,num,bpnum,deep)
int	fd,
	num,
	bpnum,
	deep;
{
	char	*cnum = itoa(num);
	write(fd,"\n",strlen("\n"));
	output_stuffing(fd,deep-1);
	write(fd,End_message,strlen(End_message));
	write(fd,cnum,strlen(cnum));
	free(cnum);
/*	write(fd," (bodypart ",strlen("(bodypart "));
	cnum = itoa(bpnum);
	write(fd,cnum,strlen(cnum));*/
	if (more == FALSE) 
		write(fd,"\n",strlen("\n"));
	else
		write(fd,"\n\n",strlen("\n\n"));
	free(cnum);
}

#define Bodypart_seperatorstart "------------------------------ Start of body part "
#define Bodypart_seperatorend "------------------------------ End of body part "

static int output_startbodypart(fd, num, deep)
int	fd;
int	num;
int	deep;
{
	char	*cnum = itoa(num);
	write(fd,"\n",strlen("\n"));
	output_stuffing(fd,deep);
	write(fd,Bodypart_seperatorstart,strlen(Bodypart_seperatorstart));
	write(fd,cnum,strlen(cnum));
	write(fd,"\n\n",strlen("\n\n"));
	free(cnum);
}

static void output_endbodypart(fd, num, deep)
int	fd;
int	num;
int 	deep;
{
	char	*cnum = itoa(num);

	write(fd,"\n",strlen("\n"));
	output_stuffing(fd,deep);
	write(fd,Bodypart_seperatorend,strlen(Bodypart_seperatorend));
	write(fd,cnum,strlen(cnum)); 
	if (more == FALSE)
		write(fd,"\n",strlen("\n"));
	else
		write(fd,"\n\n",strlen("\n\n"));
	free(cnum);
}


static int mygetchar(fd)
int	fd;
{
	static unsigned char	buf[MAXPATHLENGTH];
	static unsigned char	*bufp = buf;

	if (noInput == 0) { /* buffer is empty */
		noInput = read(fd, buf, MAXPATHLENGTH);
		bufp = buf;
	}
	return ((--noInput >= 0) ? *bufp++ : EOF);
}

static int get_line(fd,linebuf)
int	fd;
char	linebuf[];
{
	int 	i = 0;
	int 	c;
	while (i < LINESIZE && ((c = mygetchar(fd)) != EOF) && c != '\n')
	       linebuf[i++] = c;
	if (c == '\n')
		linebuf[i++] = c;
	linebuf[i] = '\0';
	return i;
}

static void output_stuffing(fd, deep)
int	fd;
int	deep;
{
	int i = 0;
	while (i++ < (deep - start_depth))
		write(fd,Stuffing,strlen(Stuffing));
}

static void output_line(fd,buf)
int	fd;
char	buf[];
{
	write(fd,buf,strlen(buf));
}

static int output_file(fd_in, deep, filename, noBps,pnum, perr)
int	fd_in,	
	deep;
char	*filename;
int	noBps;
int	*pnum;
char	**perr;
{
	char	*ix = NULL, buf[BUFSIZ],
		*ix2;
	/* reset input buffer */
	noInput = 0;
	
	if (((ix = rindex(filename,'/')) != NULL)
	    && (strncmp(++ix, hdr_822_bp, strlen(hdr_822_bp)) == 0)) {
		output_header(fd_in, FALSE);
		output_line(fp, "\n");
		return OK;
	} else if (strcmp(ix,rcmd_srch(BPT_P2_DLIV_TXT, bptbl_body_parts84)) == 0) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/rfc934 : illegal file type '%s' ignoring it",filename));
		return OK;
	}

	if (((ix = rindex(filename,'.')) != NULL) &&
		(strcmp(++ix, rcmd_srch(BPT_IA5, bptbl_body_parts84)) == 0)) {
		*(ix - 1) = '\0';
		ix2 = rindex(filename,'/');
		*ix2++ = '\0';
		*pnum = atoi(ix2);
		output_ia5(fd_in, deep, *pnum, noBps);
		*(ix - 1) = '.';
		*(ix2 - 1) ='/';
		return OK;
	}
	
	PP_LOG(LLOG_EXCEPTIONS,
	       ("Chans/rfc934 : illegal file type '%s' BOMBING OUT",filename));
	(void) sprintf(buf,
		       "illegal file type '%s' unable to flatten",
		       filename);
	*perr = strdup(buf);
	err_fatal = TRUE;
	return NOTOK;
}

static int output_header(fd_in, first)
int	fd_in;
int	first;
{
	int	msgtype;

	msgtype = FALSE;
	while(get_line(fd_in,line) != 0) { 
		if (first == TRUE
		    && line[0] == '\n'
		    && msgtype == FALSE)
			output_line(fp, "Message-Type: Multiple Part");
		output_line(fp, line);
		if (strncmp(line, "Message-Type", strlen("Message-Type")) == 0)
			msgtype = TRUE;
	}
	return msgtype;
}

static void output_ia5(fd_in, deep, bp_num, noBps)
int	fd_in,
	deep,
	bp_num,
	noBps;
{
	if (noBps > 1)
		output_startbodypart(fp, bp_num, deep);
	while (get_line(fd_in,line) != 0) {
		if (line[0] == EBch)
			output_stuffing(fp, deep);
		output_line(fp, line);
	}
}


static int depth (filename)
char    *filename;
{
	char    *p;
	int     count = 0;

	for (p = filename; *p; p++)
		if (*p == '/') {
			count ++;
			while(*p == '/') p++;
		}
	return count;
}

static int file_link(orig,tmp,filename)
char	*orig,	/* original message directory */
	*tmp,	/* new temporary directory */
	*filename;	/* file to link across */
{
	char	old[MAXPATHLENGTH],	/* old file */
	        new[MAXPATHLENGTH];	/* new link */
	struct stat statbuf;
	int	result = OK;

	(void) sprintf(old, "%s/%s",orig,filename);

	(void) sprintf(new, "%s/%s",tmp,filename);

	if ((stat(old, &statbuf) == OK)
	    && (stat(new, &statbuf) != OK)
	    && (link(old, new) != -1)) {
		result = OK;
	} else
		result = NOTOK;

	return result;
}

put_out_header(old, to, stripped_hdr)
char	*old,
	*to,
	*stripped_hdr;
{
	int	fd_in;
	char	outfile[MAXPATHLENGTH];
	int	gotMsgType = FALSE;
	if ((fd_in = open(old, O_RDONLY)) == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, old,
			("Can't open file"));
		return NOTOK;
	}
					
	sprintf(outfile,"%s/%s",to, stripped_hdr);
	if ((fp = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
		PP_SLOG(LLOG_EXCEPTIONS, outfile,
			("Can't open file"));
		return NOTOK;
	}
	gotMsgType = output_header(fd_in, TRUE);
	close(fd_in);
	close(fp);
	return OK;
}

static int	bpFile(entry)
struct dirent *entry;
{
	if (strcmp(entry->d_name, "..") == 0
	     || strcmp(entry->d_name, ".") == 0
	     || strncmp(entry->d_name, hdr_822_bp, strlen(hdr_822_bp)) == 0)
		return 0;
	return 1;
}
	    
static int numBodyParts(dir)
char	*dir;
{
	struct dirent	**namelist = NULL;

	int ret = _scandir(dir, &namelist, bpFile, NULLIFP);

	if (namelist) free ((char *) namelist);
	return ret;
}
