/* p2explode: explode a P2 structure up into component parts in hierachy */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/p2explode/RCS/p2explode.c,v 6.0 1991/12/18 20:20:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/p2explode/RCS/p2explode.c,v 6.0 1991/12/18 20:20:02 jpo Rel $
 *
 * $Log: p2explode.c,v $
 * Revision 6.0  1991/12/18  20:20:02  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"
#include <isode/psap.h>
#include <isode/cmd_srch.h>
#include "tb_bpt88.h"
#include "IOB-types.h"
#include <stdarg.h>
#include "q.h"
#include "list_bpt.h"
static int process();
static FILE *open_IOB_file();
static char curdir[MAXPATHLENGTH];   /* directory stack */
int	err_fatal = NOTOK;

extern CMD_TABLE bptbl_body_parts88[/* x400 84 body_parts */];
extern char *quedfldir;
extern char *hdr_p22_bp, *hdr_p2_bp, *hdr_ipn_bp, *cont_p22, *cont_p2;
LIST_BPT	*outbound_bpts = NULLIST_BPT;
LIST_BPT	*outbound_hdrs = NULLIST_BPT;

void    advise (int, char *, char *, ...);
void    adios (char *, char *, ...);
#define ps_advise(ps, f) \
	advise (LLOG_EXCEPTIONS, NULLCP, "%s: %s",\
		(f), ps_error ((ps) -> ps_errno))
char	*makename();

unflatten(old, new, x40084, qp, perr)
char	*old,
	*new;
int	x40084;
Q_struct	*qp;
char		**perr;
{
	char    curpart[LINESIZE], buf[BUFSIZ];
	
	setname (new);       /* set initial directory */
	msg_rinit(old);
	err_fatal = NOTOK;
	if (!(msg_rfile(curpart) == RP_OK && isP2(curpart, x40084))) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Chans/p2explode cannot find %s file in '%s'",
			(x40084 == TRUE) ? cont_p2 : cont_p22,
			old));
		(void) sprintf (buf,
				"Unable to find %s file in '%s'",
				(x40084 == TRUE) ? cont_p2 : cont_p22,
				old);
		*perr = strdup(buf);
		return NOTOK;
	}
	if (process (curpart, x40084, qp, perr) == NOTOK)
		return NOTOK;
	return OK;
}

/*
 * isP2: check to see if the current body part should be
 * processed or not.
 */

isP2(curpart, x40084)
char    *curpart;
int	x40084;
{
	char	*cp;

	if ((cp = rindex(curpart,'/')) != NULLCP)
		cp ++;
	else cp = curpart;

	return (strcmp(cp, ((x40084 == TRUE) ? cont_p2 : cont_p22)) == 0);
}

extern  errno;

/* linkthem: no reformatting needed so just link across */

linkthem(src, dest, perr)
char    *src, *dest, **perr;
{
	char	buf[BUFSIZ];

	if(link(src, dest) < 0 && errno != EEXIST) {
		PP_SLOG(LLOG_EXCEPTIONS, "link",
			("cannot link %s to %s", src, dest));
		(void) sprintf (buf,
				"Unable to link %s to %s",
				src, dest);
		*perr = strdup(buf);
		return NOTOK;
	}
	return OK;
}

/* process: start the splitting operation on the given file */
static int     process (file, x40084, qp, perr)
register char  *file;
int		x40084;
Q_struct	*qp;
char		**perr;
{
	register PE     pe;
	register PS     psin;
	FILE    	*fp;
	char		buf[BUFSIZ];
	int		retval = NOTOK;
	struct type_IOB_InformationObject *infoob;

	if((fp = fopen (file, "r")) == (FILE *)0) {
		PP_SLOG(LLOG_EXCEPTIONS, file,
		       ("Can't open file"));
		(void) sprintf (buf,
				"Unable to open input file '%s'",
				file);
		*perr = strdup(buf);
		return NOTOK;
	}

	if ((psin = ps_alloc (std_open)) == NULLPS)
	{
		(void) fclose (fp);
		ps_advise (psin, "ps_alloc");
		return NOTOK;
	}

	if (std_setup (psin, fp) == NOTOK)
	{
		ps_free (psin);
		(void) fclose (fp);
		advise (LLOG_EXCEPTIONS, NULLCP, "%s: std_setup loses", file);
		return NOTOK;
	}

	if ((pe = ps2pe (psin)) == NULLPE) { /* EOF or error? */
		(void) fclose (fp);
		err_fatal = OK;
		(void) sprintf(buf,
			"Unable to parse the p2 - ps2pe failed");
		*perr = strdup(buf);
		ps_done(psin, "ps2pe");
		return NOTOK;
	}
	PY_pepy[0] = 0;

	if(decode_IOB_InformationObject(pe, 1, NULLIP, NULLVP, &infoob) == NOTOK) {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("decode_IOB_InformationObject failure [%s]", 
			PY_pepy));
		pe_done(pe, "Parse failure IOB_InformationObject");
		err_fatal = OK;
		(void) sprintf (buf,
				"Unable to parse the p2 [%s]",
				PY_pepy);
		*perr = strdup (buf);
		(void) fclose(fp);
		return NOTOK;
	}
	if (PY_pepy[0] != 0)
		PP_LOG(LLOG_EXCEPTIONS,
		       ("parse_IOB_InformationObject non fatal failure [%s]",
			PY_pepy));
	if (write_out_parts (infoob, x40084, qp, perr) == OK)
		retval = OK;
	if( pe != NULLPE)
		pe_free (pe);
	(void) fclose (fp);
	return retval;
}

write_out_parts (infoob, x40084, qp, perr)
struct type_IOB_InformationObject *infoob;
int	x40084;
Q_struct	*qp;
char		**perr;
{
	char	buf[BUFSIZ];

	switch (infoob -> offset) {
	    case type_IOB_InformationObject_ipm:
		return write_ipm (infoob -> un.ipm, x40084, qp, perr);

	    case type_IOB_InformationObject_ipn:
		return write_ipn (infoob -> un.ipn, x40084, qp, perr);

	    default:
		PP_LOG (LLOG_EXCEPTIONS,
			("Unknown P2 type %d", infoob -> offset));
		(void) sprintf (buf,
				"Unknown P2 type %d",
				infoob -> offset);
		*perr = strdup(buf);
		return NOTOK;
	}
}

int	write_ipm (ipm, x40084, qp, perr)
struct type_IOB_IPM *ipm;
int x40084;
Q_struct	*qp;
char		**perr;
{
	if (write_heading (x40084, ipm -> heading, qp, perr) == NOTOK) 
		return NOTOK;

	return write_bodies (ipm -> body, 1, x40084, qp, perr);
}

int	write_bodies (bpp, number, x40084, qp, perr)
struct type_IOB_Body *bpp;
int	number;
int	x40084;
Q_struct	*qp;
char		**perr;
{
	struct type_IOB_BodyPart *bp;
	FILE	*fp;
	char	buf[BUFSIZ];

	while (bpp) {
		bp = bpp -> BodyPart;
		bpp = bpp->next;

		switch (bp -> offset) {
		    case type_IOB_BodyPart_ia5__text:
			if ((fp = open_IOB_file (BPT_IA5, number, 
						 qp, perr)) == NULLFILE)
				return NOTOK;
			dumpCRLFstring (fp, bp -> un.ia5__text -> data);
			if (fclose (fp) == EOF)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_tlx:
			if (write_pefile (BPT_TLX, number, 1,
					  (caddr_t)bp -> un.tlx,
					  &_ZIOB_mod, _ZTLXBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_voice:
			if (write_pefile (BPT_VOICE, number, 2,
					  (caddr_t) bp -> un.voice,
					  &_ZIOB_mod, _ZVoiceBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;


		    case type_IOB_BodyPart_g3__facsimile:
			if (write_pefile (BPT_G3FAX, number, 3,
					  (caddr_t) bp -> un.g3__facsimile,
					  &_ZIOB_mod, _ZG3FacsimileBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_g4__class1:
			if (write_pefile (BPT_TIF0, number, 4,
					  (caddr_t) bp -> un.g4__class1,
					  &_ZIOB_mod, _ZG4Class1BodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_teletex:
			if (write_pefile (BPT_TTX, number, 5,
					  (caddr_t) bp -> un.teletex,
					  &_ZIOB_mod, _ZTeletexBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_videotex:
			if (write_pefile (BPT_VIDEOTEX, number, 6,
					  (caddr_t) bp -> un.videotex,
					  &_ZIOB_mod, _ZVideotexBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_encrypted:
			if (write_pefile (BPT_ENCRYPTED, number, 8,
					  (caddr_t) bp -> un.encrypted,
					  &_ZIOB_mod, _ZEncryptedBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_message:
			pushdir (number);
			if (do_fip (bp -> un.message, x40084, qp, perr) == NOTOK)
				return NOTOK;
			popdir ();
			break;
			
		    case type_IOB_BodyPart_sfd:
			if (write_pefile (BPT_SFD, number, 10,
					  (caddr_t) bp -> un.sfd,
					  &_ZIOB_mod, _ZSFDBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;
			
		    case type_IOB_BodyPart_mixed__mode:
			if (write_pefile (BPT_TIF1, number, 11,
					  (caddr_t) bp -> un.mixed__mode,
					  &_ZIOB_mod, _ZMixedModeBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;
		    case type_IOB_BodyPart_odif:
			if (write_odif (bp -> un.odif, number, qp, perr) == NOTOK)
				return NOTOK;
			break;

		    case type_IOB_BodyPart_nationally__defined:
			if (write_pefile (BPT_NATIONAL, number, 7,
					  (caddr_t) bp -> un.nationally__defined,
					  &_ZIOB_mod, _ZNationallyDefinedBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;
			
		    case type_IOB_BodyPart_bilaterally__defined:
			if (write_pefile (BPT_BILATERAL, number, 14,
					  (caddr_t) bp -> un.bilaterally__defined,
					  &_ZIOB_mod, _ZBilaterallyDefinedBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;
			
		    case type_IOB_BodyPart_externally__defined:
			if (write_pefile (BPT_EXTERNAL, number, 15,
					  (caddr_t) bp -> un.externally__defined,
					  &_ZIOB_mod, _ZExternallyDefinedBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;
		    
		    case type_IOB_BodyPart_iso6937Text:
			if (write_pefile (BPT_ISO6937TEXT, number, 13,
					  (caddr_t) bp -> un.iso6937Text,
					  &_ZIOB_mod, _ZISO6937TextBodyPartIOB,
					  qp, perr) == NOTOK)
				return NOTOK;
			break;
			
		    default:
			PP_LOG (LLOG_EXCEPTIONS,
				("Unknown body part type %d",
				 bp -> offset));
			(void) sprintf (buf,
					"Unknown body part type %d",
					bp->offset);
			*perr = strdup (buf);
			return NOTOK;
		}
		number ++;
	}
	return OK;
}

write_heading (x40084, heading, qp, perr)
int	x40084;
struct type_IOB_Heading	*heading;
Q_struct	*qp;
char		**perr;
{
	FILE	*fp;
	PE	pe;
	char	buf[BUFSIZ];

	if (encode_IOB_Heading(&pe, 1, NULL, NULLCP, heading) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't encode heading - %s",
					  PY_pepy));
		(void) sprintf (buf,
				"Unable to encode heading - %s",
				PY_pepy);
		*perr = strdup(buf);
	}

	if ((fp = open_IOB_file ((x40084 == TRUE) ? BPT_HDR_P2 : BPT_HDR_P22,
				 0, qp, perr)) == NULLFILE) {
		pe_free (pe);
		return NOTOK;
	}

	if (write_pe (pe, fp) == NOTOK) {
		(void) fclose (fp);
		pe_free (pe);
		*perr = strdup("Unable to write_heading");
		return NOTOK;
	}

	pe_free (pe);
	if (fclose (fp) == EOF)
		return NOTOK;
	return OK;
}
	
write_pefile (type, number, tag, parm, pmod, idx, qp, perr)
int	type, number, tag;
caddr_t	parm;
modtyp	*pmod;
int	idx;
Q_struct	*qp;
char	**perr;
{
	FILE	*fp;
	PE	pe;
	char	buf[BUFSIZ];
	if (enc_f(idx, pmod, &pe, 1, NULL, NULLCP, parm) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS, ("Can't encode - %s",
					  PY_pepy));
		(void) sprintf (buf,
				"Unable to encode - %s",
				PY_pepy);
		*perr = strdup(buf);
		return NOTOK;
	}
	/* IMPLICIT TAG */
	pe->pe_class = PE_CLASS_CONT;
	pe->pe_id = tag;

	if ((fp = open_IOB_file (type, number, qp, perr)) == NULLFILE) {
		pe_free (pe);
		return NOTOK;
	}
	if (write_pe (pe, fp) == NOTOK) {
		(void) fclose (fp);
		pe_free (pe);
		*perr = strdup("Unable to write_pefile");
		return NOTOK;
	}
	pe_free (pe);
	if (fclose (fp) == EOF)
		return NOTOK;
	return OK;
}

int	write_odif (op, number, Qp, perr)
struct type_IOB_ODIFBodyPart *op;
int	number;
Q_struct	*Qp;
char	**perr;
{
	struct qbuf *qp;
	FILE	*fp;

	if ((fp = open_IOB_file (BPT_ODIF, number, Qp, perr)) == NULLFILE)
		return NOTOK;

	for (qp = op -> qb_forw; qp != op; qp = qp -> qb_forw)
		if (fwrite (qp -> qb_data, 1, qp -> qb_len, fp)
		    != qp -> qb_len) {
			(void) fclose (fp);
			return NOTOK;
		}
	if (fclose (fp) == EOF)
		return NOTOK;
	return OK;
}

write_ipn (ipn, x40084, qp, perr)
struct type_IOB_IPN *ipn;
int	x40084;
Q_struct	*qp;
char	**perr;
{
	PE 	pe;
	if (ipn->choice->offset == choice_IOB_0_non__receipt__fields
	    && ipn->choice->un.non__receipt__fields->returned__ipm != NULL) {
		pushdir(2);
		if (write_ipm (ipn -> choice -> un.non__receipt__fields->returned__ipm, x40084, qp, perr) == NOTOK)
			return NOTOK;
		free_IOB_IPM(ipn -> choice -> un.non__receipt__fields->returned__ipm);
		ipn -> choice -> un.non__receipt__fields -> returned__ipm = NULL;
		popdir();
	}
	
	if (encode_IOB_IPN (&pe, 1, 0, NULLCP, ipn) == NOTOK) 
		return NOTOK;
	
	if (write_bp (hdr_ipn_bp, pe, perr) == NOTOK)
		return NOTOK;
	return OK;
}

int	do_fip (forwarded, x40084, qp, perr)
struct type_IOB_MessageBodyPart *forwarded;
int	x40084;
Q_struct	*qp;
char	**perr;
{
	PE	pe;

	if (forwarded -> parameters != NULL
	    && (forwarded -> parameters -> delivery__time != NULL
		|| forwarded -> parameters -> delivery__envelope != NULL)) {
		/* have delivery info put out */
		if (encode_IOB_MessageParameters(&pe, 1, 0, 
						NULLCP, 
						forwarded -> parameters) == NOTOK) {
			*perr = strdup("Unable to encode IOB_MessageParameters for forwarded message");
			return NOTOK;
		}
		if (write_bp (rcmd_srch (BPT_P2_DLIV_TXT, bptbl_body_parts88),
			      pe,perr) == NOTOK)
			return NOTOK;
	}
	return write_ipm (forwarded -> data, x40084, qp, perr);
}

/* pe_done: utility routine to do the right thing for pe errors */
int             pe_done (pe, msg)
PE              pe;
char            *msg;
{
	if (pe->pe_errno)
	{
		PP_OPER(NULLCP,
			("%s: [%s] %s",msg,PY_pepy,pe_error(pe->pe_errno)));
		pe_free (pe);
		return NOTOK;
	}
	else
	{
		pe_free (pe);
		return OK;
	}
}

/* ps_done: like pe_done */
int             ps_done (ps, msg)
PS              ps;
char           *msg;
{
	ps_advise (ps, msg);
	ps_free (ps);
	return NOTOK;
}

/*  */
/* convert from \r\n to \n */

dumpCRLFstring(fp, pstr)
FILE *fp;
struct type_UNIV_IA5String *pstr;
{
	char	lastc = NULL;
	char	*cp;
	struct qbuf *qb;
	int	n;

	for (qb = pstr -> qb_forw; qb != pstr; qb = qb -> qb_forw) {
		for (n = qb -> qb_len, cp = qb -> qb_data;
		     n > 0; n--, cp++) {
			switch (*cp) {
			    case '\r':
				lastc = *cp;
				break;

			    case '\n':
				putc (*cp, fp);
				lastc = NULL;
				break;

			    default:
				if (lastc) {
					putc (lastc, fp);
					lastc = NULL;
				}
				putc (*cp, fp);
				break;
			}
		}
	}
	if (lastc) putc (lastc, fp);
}

/*
 * write_bp: write a body part contained in pe out to the file named
 * name. name is converted into the correct place in the directory
 * tree.
 */

write_bp(name, pe, perr)
char    *name;
PE      pe;
char	**perr;
{
	FILE    *fp;
	char	filename[MAXPATHLENGTH], buf[BUFSIZ];
	
	if (name[0] == '/' ||
	    strncmp (name, "./", 2) == 0 || strncmp (name, "../", 3) == 0)
		sprintf(filename,"%s",name);
	else 
		sprintf(filename, "%s/%s",curdir,name);
	PP_LOG(LLOG_TRACE,
 		("Chans/p2explode writing %s...", filename));

	if((fp = fopen(filename, "w")) == NULL)
	{
		PP_SLOG(LLOG_EXCEPTIONS, filename,
			("Can't open file"));
		(void) sprintf (buf,
				"Unable to open output file '%s'",
				filename);
		*perr = strdup(buf);
		return NOTOK;
	}

	if (write_pe (pe, fp) == NOTOK) {
		(void) sprintf (buf,
				"Failed to write output file '%s'",
				filename);
		*perr = strdup(buf);
		return NOTOK;
	}
	(void) fclose (fp);
	return OK;
}

write_pe (pe, fp)
PE	pe;
FILE	*fp;
{
	PS	psout;
	if ((psout = ps_alloc(std_open)) == NULLPS)
	{
		ps_advise (psout, "ps_alloc");
		(void) fclose (fp);
		return NOTOK;
	}
	if (std_setup (psout, fp) == NOTOK)
	{
		advise (LLOG_EXCEPTIONS, NULLCP, "std_setup loses");
		(void) fclose (fp);
		return NOTOK;
	}

	if(pe2ps(psout, pe) == NOTOK)
	{
		ps_advise(psout, "pe2ps loses");
		return NOTOK;
	}
	ps_free (psout);
	return OK;
}

/* pushdir: used in forwarded ip messages. Decend a level in the
 * directory hierachy.
 */
pushdir (n)
int     n;
{
	char    *p;
	char    name_buf[MAXPATHLENGTH];

	(void) sprintf(name_buf, "%d.ipm", n);
	p = makename (name_buf);
	setname (p);            /* clever bit - set new dir name */
	if( mkdir(p, 0755) == NOTOK)
		adios("mkdir", "Can't create directory %s", p);
	PP_LOG(LLOG_TRACE, ("Created %s\n", p));
}

/* makename: convert name into directory path */
char    *makename (name)
char    *name;
{
	static char name_buf[MAXPATHLENGTH];
	if (name[0] == '/' || strncmp (name,"./", 2) == 0 ||
	    strncmp (name, "../", 3) ==0)
		/* fullname already */
		return strdup(name);
	else 
		(void) sprintf (name_buf, "%s/%s", curdir, name);
	return name_buf;
}

/* initialise name for above */
setname (name)
char    *name;
{
	(void) strcpy (curdir, name);
/*	printf ("setname -> %s\n", curdir);*/
}

/* popdir: counterpart of pushdir */
popdir()
{
	char    *p;

	if(( p = rindex(curdir, '/')) != NULLCP)
		*p = '\0';
	else{
    		advise (LLOG_EXCEPTIONS,NULLCP, "Popdir - underflow");
 	}
}

pepy2type (type)
int	type;
{
	switch (type) {
		case 0:
			return BPT_IA5;
		case 1:
			return BPT_TLX;
		case 2:
			return BPT_VOICE;
		case 3:
			return BPT_G3FAX;
		case 4:
			return BPT_TIF0;
		case 5:
			return BPT_TTX;
		case 6:
			return BPT_VIDEOTEX;
		case 7:
			return BPT_NATIONAL;
		case 8:
			return BPT_ENCRYPTED;
/*		case 9:
			forwarded message should be dealt with else where */
		case 10:
			return BPT_SFD;
		case 11:
			return BPT_TIF1;
		case 12:
			return BPT_ODIF;
		default:
			PP_LOG(LLOG_EXCEPTIONS,
				("Chans/p2explode unknown bpt type %d\n",type));
			return BPT_UNDEFINED;
	}
}

static FILE *open_IOB_file(type, num, qp, perr)
int	type,
	num;
Q_struct	*qp;
char	**perr;
{
	char	*p = NULLCP,
		filename[MAXPATHLENGTH], buf[BUFSIZ];
	FILE	*fp;

	if ((p = rcmd_srch(type, bptbl_body_parts88)) != NULLCP
	    && qp != NULL 
	    && list_bpt_find (qp->encodedinfo.eit_types,
			      p) == NULLIST_BPT) {
		/* HACK for undeclared bp that are supported by outchan */
		if (outbound_bpts &&
		      list_bpt_find (outbound_bpts, p) != NULLIST_BPT)
			PP_LOG(LLOG_NOTICE,
			       ("Bodypart '%s' present but undeclared. Outbound channel supports it so letting through", p));
		else if (outbound_hdrs &&
			 list_bpt_find (outbound_hdrs, p) != NULLIST_BPT)
			PP_LOG(LLOG_NOTICE,
			       ("Headertype '%s' present but undeclared. Outbound channel supports it so letting through", p));
		else {
			sprintf (buf, 
				 "body type '%s' present but undeclared in envelope",
				 p);
			*perr = strdup(buf);
			err_fatal = OK;
			return NULL;
		} 
	}
	
	if(p == NULLCP) {
		advise(LLOG_EXCEPTIONS, NULLCP, "Unknown body type %d", type);
		sprintf (filename, "%s/%d", curdir,num);
	} if (type == BPT_HDR_P2 || type == BPT_HDR_P22) 
		sprintf (filename, "%s/%s", curdir, p);
	else
    		sprintf(filename, "%s/%d.%s", curdir,num, p);
		
	PP_LOG(LLOG_EXCEPTIONS,
	       ("Chans/p2explode writing %s",filename));

	if ((fp = fopen(filename, "w")) == NULL)
	{
		PP_SLOG(LLOG_EXCEPTIONS, filename,
			("Can't open file"));
		(void) sprintf(buf,
			       "Failed to open output file '%s'",
			       filename);
		*perr = strdup(buf);
		return NULL;
	}
	return fp;
}


void adios (char *what, char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	_ll_log (pp_log_norm, LLOG_FATAL, what, fmt, ap);
	va_end (ap);
	_exit (1);
}

void advise (int code, char *what, char *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	_ll_log (pp_log_norm, code, what, fmt, ap);
	va_end (ap);
}
