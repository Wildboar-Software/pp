/* p2flatten: flatten a P2 directory structure into a message again */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Format/p2flatten/RCS/p2flatten.c,v 6.0 1991/12/18 20:20:12 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Format/p2flatten/RCS/p2flatten.c,v 6.0 1991/12/18 20:20:12 jpo Rel $
 *
 * $Log: p2flatten.c,v $
 * Revision 6.0  1991/12/18  20:20:12  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include <sys/stat.h>
#include <isode/psap.h>
#include <stdarg.h>
#include "retcode.h"
#include <isode/cmd_srch.h>
#include "tb_bpt88.h"
#include "IOB-types.h"

extern CMD_TABLE bptbl_body_parts88[/* x400 88 body_parts */];
extern char *quedfldir;
extern char *cont_p2, *cont_p22;
extern void	err_abrt();
static char curdir[MAXPATHLENGTH];   /* directory stack */
static caddr_t read_bpfile();

char    curfile[MAXPATHLENGTH];
int     curdepth = 0;
int     more = TRUE;
int     flatresult = OK, err_fatal = FALSE;

void advise (int, char *what, char *fmt, ...);
void adios (char *what, char* fmt, ...);
#define ps_advise(ps, f) \
	advise (NULLCP, "%s: %s", (f), ps_error ((ps) -> ps_errno))

void fillin_ipn(), fillin_forwarded();

flatten(old,new, x40084, perr)
char    *old;
char    *new;
int	x40084;
char	**perr;
{
	PE      pe = NULLPE;
	struct	type_IOB_InformationObject	*infoobj = NULL;
	char	buf[BUFSIZ];

	curdepth = depth(old) + 1;
	(void) sprintf(curdir, old);
	msg_rinit(old);
	flatresult = OK;

	if (getname(curfile) != OK) {
		(void) sprintf (buf,
				"Source directory '%s' is empty",
				old);
		*perr = strdup(buf);
		PP_LOG(LLOG_EXCEPTIONS,
		       ("No files in %s", old));
		flatresult = NOTOK;
	}
	setname(old);
	more = TRUE;

	fillin_infoobj(&infoobj, x40084, perr);
	
	msg_rend();

	PY_pepy[0] = 0;

	if (flatresult == OK) {
		if (encode_IOB_InformationObject (&pe, 1, 0, NULLCP, infoobj) != OK) {
			PP_LOG(LLOG_EXCEPTIONS,
			       ("p2flatten failed to encode InformationObject [%s]",
				PY_pepy));
			(void) sprintf (buf,
					"Failed to encode message as p2 (%s)",
					(x40084 == TRUE) ? "84" : "88");
			*perr = strdup(buf);
			err_fatal = TRUE;
			flatresult = NOTOK;
		} else
			pe_fragment (pe, 128);
	}

	if (flatresult == OK) {
		setname (new);
		write_bp((x40084 == TRUE) ? cont_p2 : cont_p22, pe, perr);
		if (flatresult == NOTOK) {
			PP_LOG(LLOG_EXCEPTIONS,
				("Chans/p2flatten : write_bp failed"));
			*perr = strdup("Failed to write out p2 body part");
		}
	} else
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/p2flatten : encode_IOB_InformationObject failed"));
	free_IOB_InformationObject(infoobj);
	if (pe != NULLPE) pe_free (pe);

	return flatresult;
}

extern  errno;

/* pe_done: utility routine to do the right thing for pe errors */
int             pe_done (pe, msg)
PE              pe;
char            *msg;
{
	if (pe->pe_errno)
	{
		advise (LLOG_EXCEPTIONS,NULLCP,"%s: %s", msg, pe_error(pe->pe_errno));
		pe_free (pe);
		return 1;
	}
	else
	{
		pe_free (pe);
		return 0;
	}
}

/*  */

int	fillin_infoobj(pinfoobj, x40084, perr)
struct type_IOB_InformationObject	**pinfoobj;
int	x40084;
char	**perr;
{
	char	buf[BUFSIZ];
	int  type = bp_type(curfile);
	*pinfoobj = (struct type_IOB_InformationObject *) calloc(1,
						  sizeof(struct type_IOB_InformationObject));
	/* what to do about sr ? */
	if (type == ((x40084 == TRUE) ? BPT_HDR_P2 : BPT_HDR_P22)) {
		(*pinfoobj)->offset = type_IOB_InformationObject_ipm;
		fillin_ipm(&((*pinfoobj)->un.ipm), x40084, perr);
	} else if (type == BPT_HDR_IPN) {
		(*pinfoobj)->offset = type_IOB_InformationObject_ipn;
		fillin_ipn (&((*pinfoobj)->un.ipn), x40084, perr);
	} else {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Expected %s header got '%s'",
			(x40084 == TRUE) ? "p2" : "p22",
			curfile));
		(void) sprintf(buf,
			       "Expected %s header got '%s'",
			       (x40084 == TRUE) ? "p2" : "p22",
			       curfile);
		*perr = strdup(buf);
		flatresult = NOTOK;
	}
}

void	fillin_ipn (pipn, x40084, perr)
struct type_IOB_IPN	**pipn;
int 	x40084;
char	**perr;
{
	if ((*pipn = (struct type_IOB_IPN *) 
	     read_bpfile(curfile,
			 &_ZIOB_mod,
			 _ZIPNIOB)) == NULL)
		return;

	if ((*pipn)->choice->offset == choice_IOB_0_non__receipt__fields
	    && getname(curfile) == OK)
		fillin_ipm(&((*pipn) -> choice -> un.non__receipt__fields -> returned__ipm),
			   x40084, perr);
	else
		more = FALSE;
}

int	fillin_ipm(pipm, x40084, perr)
struct type_IOB_IPM	**pipm;
int	x40084;
char	**perr;
{
	*pipm = (struct type_IOB_IPM *)
		calloc(1, sizeof(struct type_IOB_IPM));
	
	fillin_heading(&((*pipm)->heading), x40084, perr);
	if (flatresult == OK)
		fillin_body(&((*pipm)->body), x40084, perr);
}

int	fillin_heading(pheading, x40084, perr)
struct type_IOB_Heading	**pheading;
int	x40084;
char	**perr;
{
	char	buf[BUFSIZ];
	extern char *hdr_p2_bp, *hdr_p22_bp;

	int type = bp_type(curfile);
	if (type == (x40084 == TRUE) ? BPT_HDR_P2 : BPT_HDR_P22) {
		*pheading = (struct type_IOB_Heading *)
			read_bpfile(curfile,
				    &_ZIOB_mod,
				    _ZHeadingIOB);
		if (getname(curfile) != OK)
			more = FALSE;
	} else {
		PP_LOG(LLOG_EXCEPTIONS,
		       ("Expected '%s' got '%s'",
			(x40084 == TRUE) ? hdr_p2_bp : hdr_p22_bp,
			curfile));
		(void) sprintf(buf,
			       "Expected '%s' got '%s'",
			       (x40084 == TRUE) ? hdr_p2_bp : hdr_p22_bp,
			       curfile);
		*perr = strdup(buf);
		flatresult = NOTOK;
	}
}

int	fillin_body(pbody, x40084, perr)
struct type_IOB_Body 	**pbody;
int			x40084;
char			**perr;
{
	struct type_IOB_Body	*end = NULL, *list = NULL, *temp;
	char	buf[BUFSIZ];

	while (more == TRUE
	       && curfile != NULL
	       && depth(curfile) >= curdepth
	       && lexnequ(curfile, curdir, strlen(curdir)) == 0
	       && flatresult == OK) {
		
		temp = (struct type_IOB_Body *) 
			calloc(1, sizeof(struct type_IOB_Body));
		
		if (depth(curfile) > curdepth)
			fillin_forwarded(&(temp->BodyPart), x40084, perr);
		else {
			temp->BodyPart = (struct type_IOB_BodyPart *)
				calloc(1, sizeof(struct type_IOB_BodyPart));

			switch (bp_type(curfile)) {
			    case BPT_HDR_P2:
			    case BPT_HDR_P22:
			    case BPT_HDR_IPN:
			    case BPT_P2_DLIV_TXT:
				/* should not get these as dealt with by forwarded */
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unexpected p2 header or delivery text file '%s' ", curfile));
				(void) sprintf (buf,
						"Unexpected p2 header or delivery text file '%s'",
						curfile);
				*perr = strdup(buf);
				flatresult = NOTOK;
				break;
				
			    case BPT_HDR_822:
				PP_LOG(LLOG_EXCEPTIONS,
				       ("822 header in p2 message '%s'", curfile));
				(void) sprintf (buf,
						"822 header in p2 message '%s'",
						curfile);
				*perr = strdup(buf);
				flatresult = NOTOK;
				break;

			    case BPT_IA5:
				temp->BodyPart->offset = type_IOB_BodyPart_ia5__text;
				fillin_ia5(&(temp->BodyPart->un.ia5__text));
				break;

			    case BPT_TLX:
				temp->BodyPart->offset = type_IOB_BodyPart_tlx;
				temp->BodyPart->un.tlx = 
					(struct type_IOB_TLXBodyPart *) 
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZTLXBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in telex bodypart '%s'", 
							curfile));
					(void) sprintf (buf,
							"problem decoding telex bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_VOICE:
				temp->BodyPart->offset = type_IOB_BodyPart_voice;
				temp->BodyPart->un.voice = 
					(struct type_IOB_VoiceBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZVoiceBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in voice bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding voice bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_G3FAX:
				temp->BodyPart->offset = type_IOB_BodyPart_g3__facsimile;
				temp->BodyPart->un.g3__facsimile = 
					(struct type_IOB_G3FacsimileBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZG3FacsimileBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in g3fax bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding g3fax bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_TIF0:
				temp->BodyPart->offset = type_IOB_BodyPart_g4__class1;
				temp->BodyPart->un.g4__class1 = 
					(struct type_IOB_G4Class1BodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZG4Class1BodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in g4 bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding g4 bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_TTX:
				temp->BodyPart->offset = type_IOB_BodyPart_teletex;
				temp->BodyPart->un.teletex = 
					(struct type_IOB_TeletexBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZTeletexBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in teletex bodypart '%s'", 
						curfile));
					(void) sprintf (buf, 
							"problem decoding teletex bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_VIDEOTEX:
				temp->BodyPart->offset = type_IOB_BodyPart_videotex;
				temp->BodyPart->un.videotex =
					(struct type_IOB_VideotexBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZVideotexBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in videotex bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding videotex bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_NATIONAL:
				temp->BodyPart->offset = type_IOB_BodyPart_nationally__defined;
				temp->BodyPart->un.nationally__defined = 
					(struct type_IOB_NationallyDefinedBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZNationallyDefinedBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in nationally defined bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding nationally defined bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_ENCRYPTED:
				temp->BodyPart->offset = type_IOB_BodyPart_encrypted;
				temp->BodyPart->un.encrypted = 
					(struct type_IOB_EncryptedBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZEncryptedBodyPartIOB);

				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in encrypted bodypart '%s'",
						curfile));
					(void) sprintf (buf,
							"problem decoding encrypted bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_SFD:
				temp->BodyPart->offset = type_IOB_BodyPart_sfd;
				temp->BodyPart->un.sfd = 
					(struct type_IOB_SFDBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZSFDBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in SFD bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding SFD bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;

			    case BPT_TIF1:
				temp->BodyPart->offset = type_IOB_BodyPart_mixed__mode;
				temp->BodyPart->un.mixed__mode = 
					(struct type_IOB_MixedModeBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZMixedModeBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in mixed mode bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding mixed mode bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;
			    
			    case BPT_ODIF:
				temp->BodyPart->offset = type_IOB_BodyPart_odif;
				fillin_odif(&(temp->BodyPart->un.odif));
				break;

			    case BPT_ISO6937TEXT:
				temp->BodyPart->offset = type_IOB_BodyPart_iso6937Text;
				temp->BodyPart->un.iso6937Text = 
					(struct type_IOB_ISO6937TextBodyPart *)
						read_bpfile(curfile,
							    &_ZIOB_mod,
							    _ZISO6937TextBodyPartIOB);
				if (flatresult != OK) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unable to read in ISO 6937 bodypart '%s'", 
						curfile));
					(void) sprintf (buf,
							"problem decoding ISO 6937 bodypart");
					err_fatal = TRUE;
					*perr = strdup(buf);
				}
				break;
			    
			    case BPT_BILATERAL:
				if (x40084 == TRUE) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unknown p2 84 body part '%s'", 
						curfile));
					(void) sprintf (buf,
							"Unknown p2 84 body part '%s'", 
							curfile);
					*perr = strdup (buf);
					err_fatal = TRUE;
					flatresult = NOTOK;
				} else {
					temp->BodyPart->offset = type_IOB_BodyPart_bilaterally__defined;
					temp->BodyPart->un.bilaterally__defined = 
						(struct type_IOB_BilaterallyDefinedBodyPart *)
							read_bpfile(curfile,
								    &_ZIOB_mod,
								    _ZBilaterallyDefinedBodyPartIOB);
					if (flatresult != OK) {
						PP_LOG(LLOG_EXCEPTIONS,
						       ("Unable to read in bilaterally defined bodypart '%s'", 
							curfile));
						(void) sprintf (buf,
								"problem decoding bilaterally defined bodypart");
						err_fatal = TRUE;
						*perr = strdup(buf);
					}
				}
				break;

			    case BPT_EXTERNAL:
				if (x40084 == TRUE) {
					PP_LOG(LLOG_EXCEPTIONS,
					       ("Unknown p2 84 body part '%s'", 
						curfile));
					(void) sprintf (buf,
							"Unknown p2 84 body part '%s'", 
							curfile);
					*perr = strdup (buf);
					err_fatal = TRUE;
					flatresult = NOTOK;
				} else {
					temp->BodyPart->offset = type_IOB_BodyPart_externally__defined;
					temp->BodyPart->un.externally__defined = 
						(struct type_IOB_ExternallyDefinedBodyPart *)
							read_bpfile(curfile,
								    &_ZIOB_mod,
								    _ZExternallyDefinedBodyPartIOB);
					if (flatresult != OK) {
						PP_LOG(LLOG_EXCEPTIONS,
						       ("Unable to read in externally defined bodypart '%s'", 
							curfile));
						(void) sprintf (buf,
								"problem decoding externally defined bodypart");
						err_fatal = TRUE;
						*perr = strdup(buf);
					}

				}
				break;
			    
					
			    default:
				PP_LOG(LLOG_EXCEPTIONS,
				       ("Unknown p2 body part '%s'", curfile));
				(void) sprintf (buf,
						"Unknown p2 body part '%s'", 
						curfile);
				*perr = strdup (buf);
				flatresult = NOTOK;
				err_fatal = TRUE;
				break;
			}
			if (flatresult == OK
			    && getname(curfile) != OK)
				more = FALSE;
			
		}
		
		if (flatresult == OK) {
			if (list == NULL)
				list = end = temp;
			else {
				end -> next = temp;
				end = temp;
			}
		}
	}
	*pbody = list;
}
	
/* ps_done: like pe_done */
int             ps_done (ps, msg)
PS              ps;
char           *msg;
{
	if (ps->ps_errno)
	{
		ps_advise (ps, msg);
		ps_free (ps);
		return 1;
	}
	else
	{
		ps_free (ps);
		return 0;
	}
}

getname(str)
char    *str;
{
	/* msg_rfile returns full pathname need to split of curdir */
	if (msg_rfile(str) != RP_OK) {
		str = NULL;
		return NOTOK;
	}
	return OK;
}

/* initialise name for above */
setname (name)
char    *name;
{
	(void) strcpy (curdir, name);
/*      printf ("setname -> %s\n", curdir);*/
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
	PS      psout;
	FILE    *fp;
	static char fullname[MAXPATHLENGTH];
	char	buf[BUFSIZ];
	sprintf(fullname,"%s/%s", curdir, name);
	PP_LOG(LLOG_TRACE,
		("Chans/p2flatten : Writing %s", fullname));

	if((fp = fopen(fullname, "w")) == NULL)
	{
		PP_SLOG(LLOG_EXCEPTIONS, fullname,
			("Can't open file"));
		(void) sprintf (buf,
				"Unable to open output file '%s'",
				fullname);
		*perr = strdup(buf);
		flatresult = NOTOK;
		return NOTOK;
	}

	if ((psout = ps_alloc(std_open)) == NULLPS)
	{
		ps_advise (psout, "ps_alloc");
		(void) fclose (fp);
		flatresult = NOTOK;
		return NOTOK;
	}
	if (std_setup (psout, fp) == NOTOK)
	{
		advise (NULLCP, "std_setup loses", fullname);
		(void) fclose (fp);
		flatresult = NOTOK;
		return NOTOK;
	}

	if(pe2ps(psout, pe) == NOTOK)
	{
		ps_advise(psout, "pe2ps loses");
		flatresult = NOTOK;
		return NOTOK;
	}
	(void) fclose (fp);
	ps_free(psout);
/*      printf (" done\n");*/
	return OK;
}

/*
 * read_bpfile: read in a body part and stuff in pe and decode.
 */

static caddr_t read_bpfile(name, pmod, idx)
char    *name;
modtyp	*pmod;
int	idx;
{
	PS      psout;
	PE      pe;
	FILE    *fp;
	caddr_t	ret;

	PP_LOG(LLOG_TRACE,
		("Chans/p2flatten : Reading %s", name));

	if((fp = fopen(name, "r")) == NULL)
	{
		PP_SLOG(LLOG_EXCEPTIONS, name,
			("Can't open file"));
		flatresult = NOTOK;
		return NULL;
	}

	if ((psout = ps_alloc(std_open)) == NULLPS)
	{
		ps_advise (psout, "ps_alloc");
		(void) fclose (fp);
		flatresult = NOTOK;             
		return NULL;
	}
	if (std_setup (psout, fp) == NOTOK)
	{
		advise (NULLCP, "std_setup loses", name);
		(void) fclose (fp);
		flatresult = NOTOK;
		return NULL;
	}

	if((pe = ps2pe(psout)) == NULLPE)
	{
		ps_advise(psout, "ps2pe loses");
		flatresult = NOTOK;             
		return NULL;
	}

	if (dec_f(idx, pmod, pe, 0, NULL, NULLCP, &ret) == NOTOK) {
		PP_LOG (LLOG_EXCEPTIONS,
			("Can't decode - %s",
			 PY_pepy));
		flatresult = NOTOK;
		return NULL;
	}
	
	pe_free(pe);
	(void) fclose (fp);
	ps_free(psout);
	return ret;
}

bp_type (str)
char    *str;
{
	char *s, *ix;
	int retval;
	if ((s = rindex(str,'/')) == NULLCP)
		s = str;
	else
		s++;
	/* search for those files that aren't num.str */
	if (strcmp(s,rcmd_srch(BPT_HDR_P2,bptbl_body_parts88)) == 0)
		return BPT_HDR_P2;
	if (strcmp(s,rcmd_srch(BPT_HDR_P22,bptbl_body_parts88)) == 0)
		return BPT_HDR_P22;
	if (strcmp(s,rcmd_srch(BPT_HDR_822,bptbl_body_parts88)) == 0)
		return BPT_HDR_822;
	if (strcmp(s,rcmd_srch(BPT_HDR_IPN,bptbl_body_parts88)) == 0)
		return BPT_HDR_IPN;
	if (strcmp(s,rcmd_srch(BPT_P2_DLIV_TXT,bptbl_body_parts88)) == 0)
		return BPT_P2_DLIV_TXT;

	/* not hdr so n.xxx where n is number and xxx bpt name */

	if ((ix = index(s,'.')) == NULL) {
/*              printf("cannot find '.' char in %s",s);*/
		retval = -1;
	} else {
		ix++;
		retval = (cmd_srch(ix, bptbl_body_parts88));    /* returns # defined number of bp type */
	}
	if (retval == -1) {
		/* bomb out */
		PP_LOG(LLOG_EXCEPTIONS,
			("Chans/p2flatten : Unknown body type '%s'",str));
		flatresult = NOTOK;
	}
	return retval;
}

depth (file)
char    *file;
{
	char    *p;
	int     count = 0;

	for (p = file; *p; p++)
		if (*p == '/') {
			count ++;
			while(*p == '/') p++;
		}
	return count;
}

char    *readoctet (file, len)
char    *file;
int     *len;
{
	char    *p;
	int     fd;
	struct stat st;

	PP_LOG(LLOG_TRACE,
		("Chans/p2flatten reading octet %s",file));

	if ((fd = open(file ,0)) == NOTOK)
	{
		PP_SLOG(LLOG_EXCEPTIONS, file,
			("Can't open file"));
		flatresult = NOTOK;
		return NULLCP;
	}
	if (fstat (fd, &st) == NOTOK)
	{
		PP_SLOG(LLOG_EXCEPTIONS, file,
			("Can't stat file"));
		flatresult = NOTOK;
		return NULLCP;
	}
	*len = st.st_size;
	p = smalloc (*len + 1);
	if (read (fd, p, *len) != *len) {
		advise (LLOG_EXCEPTIONS, "read", "Read failed");
		flatresult = NOTOK;
	}
	close (fd);
/*      printf ("done\n"); */
	return p;
}

#define BUF_INC	128

char    *readCRLFstring (file, plen)
char    *file;
int	*plen;
{
	char    *p;
	FILE 	*fd;
	unsigned int	c,
		len,
		count;

	PP_LOG(LLOG_TRACE,
		("Chans/p2flatten reading CRLF string %s",file));

	if ((fd = fopen(file ,"r")) == (FILE *) 0)
	{
		PP_SLOG(LLOG_EXCEPTIONS, file,
			("Can't open file"));
		flatresult = NOTOK;
		return NULLCP;
	}
	len = BUF_INC;
	p = smalloc (len);
	count = 0;
	while ((c = getc(fd)) != EOF) {
		if (c == (int) ('\n')) {
			if (count >= len) {
				len += BUF_INC;
				p = realloc(p, len);
			}
			p[count++] = '\r';
		}
		if (count >= len) {
			len += BUF_INC;
			p = realloc(p, len);
		}
		p[count++] = (char) c;
	}
	if (count >= len) {
		len += 1;
		p = realloc(p, len);
	}
	*plen = count;	
	fclose (fd);
/*      printf ("done\n"); */
	return p;
}

fillin_ia5(pia5text)
struct type_IOB_IA5TextBodyPart	**pia5text;
{
	int	len = 0;
	char	*str;

	*pia5text = (struct type_IOB_IA5TextBodyPart *)
		calloc(1, sizeof(struct type_IOB_IA5TextBodyPart));
	(*pia5text)->parameters = (struct type_IOB_IA5TextParameters *)
		calloc(1, sizeof(struct type_IOB_IA5TextParameters));
	(*pia5text)->parameters->parm = int_IOB_Repertoire_ia5;
	str = readCRLFstring(curfile, &len);
	(*pia5text)->data = str2qb(str, len, 1);

	free(str);
}
	
fillin_odif(podif)
struct type_IOB_ODIFBodyPart	**podif;
{
	int	len = 0;
	char	*str;
	
	str = readoctet(curfile, &len);
	if (str != NULL) {
		*podif = str2qb(str, len, 1);
		free(str);
	}
}
	
void fillin_forwarded(pbp, x40084, perr)
struct type_IOB_BodyPart	**pbp;
int	x40084;
char	**perr;
{
	int	oldepth = curdepth;
	char	olddir[MAXPATHLENGTH];
	char	*ix = rindex(curfile, '/');
	(void) sprintf(olddir, curdir);

	*pbp = (struct type_IOB_BodyPart *)
		calloc (1, sizeof(struct type_IOB_BodyPart));
	(*pbp)->offset = type_IOB_BodyPart_message;
	(*pbp)->un.message = (struct type_IOB_MessageBodyPart *)
		calloc(1, sizeof(struct type_IOB_MessageBodyPart));
	
	curdepth = depth(curfile);
	if (ix != NULLCP)
		*ix = '\0';
	(void) sprintf(curdir, curfile);
	if (ix != NULLCP)
		*ix = '/';
	
	PY_pepy[0] = 0;
	if (bp_type(curfile) == BPT_P2_DLIV_TXT) {
		if (((*pbp)->un.message->parameters =
		     (struct type_IOB_MessageParameters *)
		     read_bpfile(curfile,
				 &_ZIOB_mod,
				 _ZMessageParametersIOB)) == NULL) 
			return;
		if (getname(curfile) != OK)
			more = FALSE;
	} else 
		(*pbp)->un.message->parameters = (struct type_IOB_MessageParameters *) calloc (1, sizeof(struct type_IOB_MessageParameters));

	fillin_ipm(&((*pbp)->un.message->data), x40084, perr);

	curdepth = oldepth;
	(void) sprintf(curdir, olddir);
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
