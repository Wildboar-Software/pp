/* hdr2fax.c: convert 822 hdr to fax image */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/hdr2fax.c,v 6.0 1991/12/18 20:15:19 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/hdr2fax.c,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: hdr2fax.c,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 */
#include 	<stdio.h>
#include 	"util.h"
#include	<isode/cmd_srch.h>
#include	"or.h"
#include 	"IOB-types.h"
#include	"table.h"
#include	"fonts.h"
#include	"ap.h"
#include	"pg_sizes.h"

extern BitMap	new_bitmap(), file2bitmap();

char		*myname;
static char	buffer[BUFSIZ], buf2[BUFSIZ];
static char	*read822hdr();
static int	draw2Str(), drawStr();
char		*loc_telfax_number;
char		*loc_company;
char		*postscript;
char		*outmta;

#define	OPT_FROM	1
#define	OPT_TO		2
#define OPT_SUBJECT	3
#define OPT_TABLE	4
#define OPT_OUTMTA	6

CMD_TABLE	tbl_options [] = { /* hdr2fax commandline options */
	"-from",	OPT_FROM,
	"-to",		OPT_TO,
	"-subject",	OPT_SUBJECT,
	"-table",	OPT_TABLE,
	"-outmta",	OPT_OUTMTA,
	0,		-1
	};

char	*to, 
	*from, 
	*subject, 
	*page, 
	*display_str,
	*date = NULLCP,
	*from822 = NULLCP;
int	x_start, 
	y_start,
	page_x,
	page_y;
PPFontPtr	large = NULL, 
		bold = NULL,
		norm = NULL;

double	res = 3.85; /*7.7;*/	/* pixels/lines per mm in y direction */
double	xres = FAX_WIDTH_LINES/FAX_WIDTH;	/* pixels per mm in x direct */
int	tab;
Table	*table = NULLTBL;

main(argc, argv)
int	argc;
char	**argv;
{
	struct type_IOB_G3FacsimileBodyPart	*p2;
	BitMap	coversheet;

	sys_init(argv[0]);
	argv++;
	while (*argv != NULL) {
		switch (cmd_srch(*argv,tbl_options)) {
		    case OPT_FROM:
			if (*(argv+1) == NULLCP)
				fprintf(stderr,
				       "no sender given with flag %s", *argv);
			else {
				argv++;
				from = *argv;
			}
			break;
		    
		    case OPT_TO:
			if (*(argv+1) == NULLCP)
				fprintf(stderr,
				       "no recipient given with flag %s", *argv);
			else {
				argv++;
				to = *argv;
			}
			break;

		    case OPT_OUTMTA:
			if (*(argv+1) == NULLCP)
				fprintf(stderr,
				       "no recipient given with flag %s", *argv);
			else {
				argv++;
				if (lexequ(*argv, "none") != 0)
					outmta = *argv;
			}
			break;

		    case OPT_SUBJECT:
			if (*(argv+1) == NULLCP)
				fprintf(stderr,
				       "no subject given with flag %s", *argv);
			else {
				argv++;
				subject = *argv;
			}
			break;
			
		    case OPT_TABLE:
			if (*(argv+1) == NULLCP)
				fprintf(stderr,
				       "no table given with flag %s", *argv);
			else {
				argv++;
				if (lexequ(*argv, "none") != 0
				    && ((table = tb_nm2struct (*argv)) == NULLTBL)) {
					fprintf(stderr,
						"Cannot initialise table '%s'\n", *argv);
					    exit (1);
				    }
			}
			break;

		    default:
			fprintf(stderr,
			       "unknown option '%s'", *argv);
			exit(1);
		}
		argv++;
	}

	if (table == NULLTBL) {
		fprintf(stderr,
			"No table passed over in info field for filter '%s'",
			argv[0]);
		exit(1);
	}
	
	if (initialise_globals() != OK) {
		fprintf(stderr,
			"Unable to initialise fonts");
		exit(1);
	}

	p2 = (struct type_IOB_G3FacsimileBodyPart *) 
		calloc (1, sizeof(struct type_IOB_G3FacsimileBodyPart));

	p2 -> parameters = (struct type_IOB_G3FacsimileParameters *)
		calloc (1, sizeof(struct type_IOB_G3FacsimileParameters));
    
	p2 -> parameters -> optionals = 
		opt_IOB_G3FacsimileParameters_number__of__pages;

	coversheet = new_bitmap(FAX_WIDTH_LINES, FAX_HEIGHT_LINES);

	if (create_cover_sheet(coversheet, 
			       FAX_WIDTH_LINES, FAX_HEIGHT_LINES) != OK)
		exit(1);

	encodeImage(coversheet,
		    FAX_WIDTH_LINES, FAX_HEIGHT_LINES,
		     p2);
	outputFax(p2, stdout);
	free_bitmap(coversheet, FAX_WIDTH_LINES, FAX_HEIGHT_LINES);
	exit(0);
}

/*  */

/* replace \n's with \n's ! */

static char	*mystrdup(str)
char	*str;
{
	char	*retstr = malloc((strlen(str)+1) * sizeof(char));
	char	*ix, *iy;
	
	for (ix = str, iy = retstr; *ix != '\0'; ix++, iy++) {
		if (*ix == '\\') {
			switch (*(ix+1)) {
			    case 'n':
				*iy = '\n';
				ix++;
				break;
			    default:
				*iy = *ix;
				break;
			}
		} else
			*iy = *ix;
	}
	*iy = '\0';
	return retstr;
}

extern PPFontPtr file2font();

int
initialise_globals()
{
	FILE	*fp;
	x_start = LEFT_PAD_LINES;
	y_start = TOP_PAD_LINES;
	page = NULLCP; page_x = 0; page_y = 0;
	tab = 50; /* mm */
	loc_company = NULLCP;
	loc_telfax_number = NULLCP;
	postscript = NULLCP;

	if (table != NULLTBL) {
		if (tb_k2val (table, "keyfont", buffer, TRUE) != NOTOK) {
			if ((fp = fopen(buffer, "r")) == NULL
			    || (large = file2font(fp)) == (PPFontPtr) NOTOK)
				return NOTOK;
			fclose(fp);
		}
			    
		if (tb_k2val (table, "valuefont", buffer, TRUE) != NOTOK) {
			if ((fp = fopen(buffer, "r")) == NULL
			    || (bold = file2font(fp)) == (PPFontPtr) NOTOK)
				return NOTOK;
			fclose(fp);
		}
		if (tb_k2val (table, "textfont", buffer, TRUE) != NOTOK) {
			if ((fp = fopen(buffer, "r")) == NULL
			    || (norm = file2font(fp)) == (PPFontPtr) NOTOK)
				return NOTOK;
			fclose(fp);
		}
		if (tb_k2val (table, "xstart", buffer, TRUE) != NOTOK)
			x_start = str2mm(buffer) * xres + X_ALLOWED_NOT_PRINT;
		if (tb_k2val (table, "ystart", buffer, TRUE) != NOTOK)
			y_start = (str2mm(buffer) + Y_ALLOWED_NOT_PRINT_TOP) * res;
		if (tb_k2val (table, "tab", buffer, TRUE) != NOTOK)
			tab = str2mm(buffer);
		if (tb_k2val (table, "coverpage", buffer, TRUE) != NOTOK)
			page = strdup(buffer);
		if (tb_k2val (table, "localOrg", buffer, TRUE) != NOTOK) 
			loc_company = mystrdup(buffer);
		if (tb_k2val (table, "localNumber", buffer, TRUE) != NOTOK)
			loc_telfax_number = strdup(buffer);
		if (tb_k2val (table, "postscript", buffer, TRUE) != NOTOK)
			postscript = mystrdup(buffer);
	} else
		return NOTOK;
	return OK;
}

/* copied from Chans/fax_aux.c */
get_fax_number(adr, ptelno, porg)
char	*adr;
char	**ptelno, **porg;
{
	OR_ptr	or, or2;
	char	*telno = NULLCP;

	*porg = NULLCP;
	*ptelno = NULLCP;

	or = or_std2or(adr);
	or2 = or;
	while (or2 = or_locate (or2, OR_DD)) {
		if (cmd_srch(or2->or_ddname, 
			     ortbl_ddvalid) == OR_DDVALID_FAX) {
			telno = strdup (or2->or_value);
			break;
		} else if (or2 -> or_next != NULLOR)
			or2 = or2 -> or_next;
		else
			break;
		
	}
	
	/* use outmta */
	if (telno == NULLCP
	    && outmta != NULLCP)
		telno = strdup(outmta);

	if (telno != NULLCP
	    && table != NULLTBL
	    && tb_k2val (table, telno, buffer, TRUE) != NOTOK) {
		free(telno);
		tblentry2num(buffer, ptelno, porg);
	} else {
		*ptelno = telno;
	}
	or_free(or);
}

tblentry2num(buf, ptelno, porg)
char	*buf;
char	**ptelno;
char	**porg;
{
	char	*telno = NULLCP, *org = NULLCP, *ix, *iy;

	if ((ix = index(buf, ',')) == NULLCP) {
		/* must all be number */
		compress(buf,buf);
	} else {
		*ix++ = '\0';
		compress(buf, buf);
		/* for now rest is company */
		if (*ix != '\0') 
			org = strdup(ix);
	}
	telno = malloc(strlen(buf)*sizeof(char));
	
	ix = buf;
	iy = telno;
	
	while (*ix != '\0')
		switch (*ix) {
		    case 'p':
		    case 'P':
			ix++;
			break;
		    default:
			*iy++ = *ix++;
			break;
		}

	*iy = '\0';

	if (ptelno != NULL) 
		*ptelno = telno;
	else if (telno != NULLCP)
		free(telno);
	      
	if (porg != NULL)
		*porg = org;
	else if (org != NULLCP)
		free(org);
}

extern char	*loc_or;

int
create_cover_sheet(p, wid, ht)
BitMap	p;
int	wid, ht;
{
	int	x = x_start, y = y_start, nx, ny1, remain;
	char	*rfchdr = NULLCP, *telno = NULLCP, *org = NULLCP;
	AP_ptr	tree, group, name, local, domain, route;

	if (page != NULLCP) {
		BitMap	b;
		int	w, h;
		FILE	*fp;
		if ((fp = fopen(page, "r")) == NULL) {
			PP_SLOG(LLOG_EXCEPTIONS, page,
				("Can't open cover page bitmap file"));
			return NOTOK;
		}
		if ((b = file2bitmap(fp, &w, &h)) == (BitMap) NOTOK) {
			fprintf(stderr,
			       "Unable to read in bitmap from file '%s'",
				page);
			return NOTOK;
		}
		/* center on x */
		remain = (wid - w - page_x)/2;
		
		if (insertbitmap(p, wid, ht, 
				 b, w, h, 
				 remain + page_x, page_y) == NOTOK) {
			free_bitmap(b, w, h);
			return NOTOK;
		}
		free_bitmap(b, w, h);
	}
	
	rfchdr = read822hdr(&subject);

	if (to != NULLCP) {
		OR_ptr	or, or2;
		int	doneTo = FALSE;
		or = or_std2or (to);
		
		or2 = or;
		while (or2 = or_locate (or2, OR_DD)) {
			if (cmd_srch(or2->or_ddname, 
				     ortbl_ddvalid) == OR_DDVALID_ATTN) {
				char	*str;
				(void) sprintf(buf2, 
					       "For the attention of: ");
				str = mystrdup(or2 -> or_value);
				draw2Str(p, wid, ht,
					 &x, &y,
					 buf2, large,
					 str, bold);
				free(str);
				doneTo = TRUE;
			} 
			if (or2 -> or_next != NULLOR)
				or2 = or2 -> or_next;
			else
				break;
		}
		
		if (doneTo == FALSE) {
			if (or_getpn(or, buffer) != TRUE)
				buffer[0] = '\0';
			if (or2 = or_locate (or, OR_OU)) {
				/* see if local or not */
				or_or2std(or2, buf2, FALSE);
				if (lexequ(buf2, loc_or) != 0) {
					/* not local so add in mail box */
					if (or_or2rfc (or, buf2) == NOTOK)
						/* failed to convert to 822 */
						/* use x400 address */
						strcpy(buf2, to);
					if (buffer[0] == '\0')
						strcpy(buffer, buf2);
					else {
						strcat(buffer, " (");
						strcat(buffer, buf2);
						strcat(buffer, ")");
					}
				}
			}
			
			if (buffer[0] == '\0')
				strcpy(buffer, to);
			(void) sprintf(buf2, "To: ");
			draw2Str(p, wid, ht,
				 &x, &y,
				 buf2, large,
				 buffer, bold);
		}			    

		get_fax_number(to, &telno, &org);
		if (org != NULLCP) {
			char	*cp;
			(void) sprintf (buf2, "At: ");
			cp = mystrdup(org);
			draw2Str (p, wid, ht,
				  &x, &y, 
				  buf2, large,
				  cp, bold);
			free(org);
			free(cp);
		}
		if (telno != NULLCP && atoi(telno) != 0) {
				(void) sprintf (buf2, "TeleFax-No: ");

				draw2Str (p, wid, ht,
					  &x, &y, 
					  buf2, large,
					  telno, bold);
				free(telno);
		} 
		or_free(or);
	}

	y += large->max_ht*2;

	/* try one from hdr.822 first */
	if (from822 != NULLCP
	    && ap_s2p(from822,
		      &tree, &group, &name, 
		      &local, &domain, &route) != (char *) NOTOK) {
		char	*m;
		if (name != NULLAP) {
			char	*ix;
			m = ap_p2s(NULLAP, name,
				   NULLAP, NULLAP,
				   NULLAP);
			if ((ix = rindex(m, '<')) != NULLCP)
				*ix = '\0';
			strcpy(buffer, m);
			free (m);
		} else
			strcpy(buffer, local->ap_obvalue);
		m = ap_p2s(NULLAP, NULLAP,
			   local, domain,
			   NULLAP);
		strcat(buffer, "(email=\"");
		strcat(buffer, m);
		strcat(buffer, "\")");
		free (m);
		ap_sqdelete(tree, NULLAP);
		ap_free(tree);
		free(from822);

		(void) sprintf(buf2, "From: ");
		draw2Str (p, wid, ht,
			  &x, &y, 
			  buf2, large,
			  buffer, bold);
	
	} else if (from != NULLCP) {
		/* try one from envelope */
		OR_ptr	or, or2;
		or = or_std2or (from);
		
		or2 = or;
		if (or_getpn(or, buffer) == TRUE) {
			if (or2 = or_locate(or, OR_OU)) {
				/* see if local or not */
				or_or2std(or2, buf2, FALSE);
				if (lexequ(buf2, loc_or) != 0) {
					or2 = or;
					(void) strcat (buffer, " (");
					while (or2 = or_locate (or2, OR_OU)) {
						(void) sprintf(buffer, "%s%s;",
							       buffer, or2 -> or_value);
						if (or2 -> or_next != NULLOR)
							or2 = or2 -> or_next;
						else
							break;
					}
					if (or2 = or_locate (or, OR_O)) 
						(void) sprintf(buffer, "%s%s;", 
							       buffer, or2 -> or_value);

					if (or2 = or_locate (or, OR_PRMD)) 
						(void) sprintf(buffer, "%s%s;", 
							       buffer, or2 -> or_value);

					if ((or2 = or_locate (or, OR_ADMD)) 
					    && lexequ(or2 -> or_value, " ") != 0)
						(void) sprintf(buffer, "%s%s;", 
							       buffer, or2 -> or_value);

					if (or2 = or_locate (or, OR_C)) 
						(void) sprintf(buffer, "%s%s", 
							       buffer, or2 -> or_value);
					(void) strcat (buffer, ")");
				}
			}
		} else if ((or2 = or_find(or, OR_DD, "RFC-822")) != NULLOR)
			or_ps2asc(or2->or_value, buffer);
		else
			strcat(buffer, from);

		(void) sprintf(buf2, "From: ");
		draw2Str (p, wid, ht,
			  &x, &y, 
			  buf2, large,
			  buffer, bold);
	
		or_free(or);
	}

	if (loc_company != NULLCP) {
		(void) sprintf(buf2, "Organisation: ");
		(void) sprintf(buffer, loc_company);
		draw2Str (p, wid, ht,
			  &x, &y, 
			  buf2, large,
			  buffer, bold);
	}

	if (loc_telfax_number != NULLCP) {
		(void) sprintf(buf2, "TeleFax-No: ");
		(void) sprintf(buffer, "%s", loc_telfax_number);
		draw2Str (p, wid, ht,
			  &x, &y, 
			  buf2, large,
			  buffer, bold);
	}

	y += large->max_ht*2;

	if (date != NULLCP) {
		(void) sprintf(buf2, "Date: ");
		draw2Str (p, wid, ht,
			  &x, &y, 
			  buf2, large,
			  date, bold);
		free(date);
		date = NULLCP;
	}

	if (subject != NULLCP) {
		(void) sprintf(buf2, "Subject: ");
		draw2Str (p, wid, ht,
			  &x, &y, 
			  buf2, large,
			  subject, bold);
	}

	y += large->max_ht;
	
	if (rfchdr != NULLCP) {
		drawStr(p, wid, ht,
			norm,
			x, y, 
			rfchdr,
			&nx, &y);
		free(rfchdr);
		rfchdr = NULLCP;
	}

	y += large->max_ht;

	if (postscript != NULLCP) {
		drawStr(p, wid, ht,
			norm,
			x, y,
			postscript,
			&nx, &ny1);
		y = ny1;
	}
	return OK;
}

/*  */

static int
draw2Str(p, w, h, px, py, s1, f1, s2, f2)
BitMap	p;
int	w, h;
int	*px, *py;
char	*s1;
PPFontPtr	f1;
char	*s2;
PPFontPtr	f2;
{
	int	nx, y1, y2;

	drawStr(p, w, h,
		f1,
		*px, *py,
		s1,
		&nx, &y1);
	if (nx - *px < tab * xres)
		nx = *px + (tab * xres);
	drawStr(p, w, h,
		f2,
		nx, *py,
		s2,
		&nx, &y2);
	*py = (y2 > y1) ? y2 : y1;
}
	
static int	drawStr (p, w, h, f, x, y, s, nx, ny)
BitMap	p;
int	w, h;
PPFontPtr	f;
int	x, y;
char	*s;
int	*nx, *ny;
{
	int	nolines, ix, my_y = y, longest_x = 0;
	char	*lines[100];

	nolines = sstr2arg (s, 100, lines, "\n");
	
	for (ix = 0; ix < nolines; ix++) {
		int	dy, i = 0, dx;
		char	*buffer = lines[ix];
		char	*cix = &(buffer[0]);
		/* take care of any folding */
		while (cix - buffer < (int)strlen(buffer)) {
			i = str_into_bitmap(p,f,x,my_y, &dx, &dy,
					    FAX_WIDTH_LINES - LEFT_PAD_LINES,
					    cix,
					    strlen(cix));
			cix += i;
			if (dx > longest_x) longest_x = dx;
			my_y += dy;
		}
	}

	*ny = my_y;
	*nx = x + longest_x;
}
		
/*  */

str2mm(s)
char	*s;
{
	int	n, div = 1;
	if (*s == NULL) return 0;
	
	n = 0;
	while (*s != NULL && isspace(*s)) s++;
	while (*s != NULL && isdigit(*s)) {
		n = n * 10 + *s - '0';
		s++;
	}
	if (*s == '.') {
		/* after decimal points */
		s++;
		while (*s != NULL && isdigit(*s)) {
			n = n * 10 + *s - '0';
			if (div == 1)
				div = 10;
			else
				div *= 10;
			s++;
		}
	}
	while (*s != NULL && isspace(*s)) s++;
	if (*s != NULL && isalpha(*s)) {
		if (lexequ (s, "cm") == 0) 
			n *= 10;
		else if (lexequ (s, "in") == 0) {
			n *= 254;
			if (div == 1)
				div = 10;
			else
				div *= 10;
		}
	}
	n = n / div;
	return n;
}

/*  */
/* straight from submit/rd_rfchdr.c */

/* -- basic states for the state machine -- */
#define HDV_EOH			0
#define HDV_NEW			1
#define HDV_MORE		2

/* -- munge the header lines of the message -- */
#define HDR_FROM		1
#define HDR_SENDER		2
#define HDR_CC			3
#define HDR_TO			4
#define HDR_SUBJECT		5
#define	HDR_DATE		6

static	CMD_TABLE  htbl_rfc [] = {
	"from",			HDR_FROM,
	"sender",		HDR_SENDER,
	"cc",			HDR_CC,
	"to",			HDR_TO,
	"subject",		HDR_SUBJECT,
	"date",			HDR_DATE,
	0,			0
	};

static int getline (bp, fp)
char	**bp;
FILE	*fp;
{
	static char *buf;
	static int bufsiz = 0;
	int	count;
	char	*cp;
	int	c;

	if (buf == NULLCP)
		buf = smalloc (bufsiz = BUFSIZ);
	for (cp = buf, count = 0; ; count ++) {
		if (count >= bufsiz - 5) {
			int curlen = cp - buf;
			buf = realloc (buf, (unsigned) (bufsiz += BUFSIZ));
			if (buf == NULL) {
				fprintf(stderr,
				       "Out of memory");
				exit(1);
			}
			cp = buf + curlen;
		}
		switch (c = getc (fp)) {
		    case '\n':
			if ((c = getc(fp)) == ' ' || c == '\t') {
				*cp ++ = '\n';
				continue;
			}
			ungetc (c, fp);
			break;
		    case EOF:
			if (cp == buf)
				return NOTOK;
			break;
		    default:
			*cp ++ = c;
			continue;
		}
		break;
	}
	*cp = '\0';
	*bp = buf;
	return OK;
}

static int hdr_parse (txt, name, contents)    /* -- parse one header line -- */
register char		*txt;		  /* -- a line of header text -- */
char			**name;		  /* -- location of field's name -- */
char			**contents;    /* -- location of field's contents -- */
{
	char		linetype;


	PP_DBG (("submit/hdr_parse (%s)", txt));


	if (isspace (*txt)) {
		/* -- continuation text -- */
		if (*txt == '\n' || *txt == '\0')
			return (HDV_EOH);
		linetype = HDV_MORE;
	}
	else  {
		linetype = HDV_NEW;

		*name = txt;
		while (*txt && *txt != ':')
			txt ++;
		if (*txt == '\0' || *txt == '\n')
			return NOTOK;
		*txt ++ = '\0';

		(void) compress (*name, *name);
	}


	*contents = txt;

	return (linetype);
}

static char	*read822hdr (psubject)
char	**psubject;
{
	char	*retstr = NULLCP, *bp, *temp, *addon, *name, *contents;
	int	retval, type, len;

	while (getline (&bp, stdin) == OK) {
		switch (retval = hdr_parse (bp, &name, &contents)) {
		    case HDV_MORE:
			continue;
		    case HDV_NEW:
			if ((type = cmd_srch (name, htbl_rfc)) > 0) {
				switch (type) {
				    case HDR_DATE:
					date = strdup(contents);
					break;

				    case HDR_SUBJECT:
					*psubject = strdup(contents);
					break;
				    case HDR_FROM:
					from822 = strdup(contents);
				    case HDR_SENDER:
				    case HDR_CC:
				    case HDR_TO:
					len = strlen(name) + strlen(contents) + 2;
					addon = calloc(len, sizeof(char));
					(void) sprintf(addon, "%s:%s",name,contents);

					if (retstr == NULLCP)
						retstr = addon;
					else {
						len = strlen(addon) + strlen(retstr) + 1;
						temp = calloc(len, sizeof(char));
						(void) sprintf (temp, "%s\n%s", retstr, addon);
						free(retstr);
						free(addon);
						retstr = temp;
					}
					break;
				    default:
					break;
				}
			}
			continue;
		    case HDV_EOH:
			break;
		    case NOTOK:
			fprintf(stderr,
				"hdr2fax: Unable to parse '%s' as key:field", bp);
			break;
		}
		break;
	}
	return retstr;
}
