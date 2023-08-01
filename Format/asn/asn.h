/* asnfilter.h: Used by the asnfilter code */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Format/asn/RCS/asn.h,v 6.0 1991/12/18 20:15:43 jpo Rel $
 *
 * $Log: asn.h,v $
 * Revision 6.0  1991/12/18  20:15:43  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_ASNFILTER
#define _H_ASNFILTER



typedef struct funct_struct {
	char	*name;		/* plain, motis-86-6937, generaltxt ... etc */
	int	(*ffunc) ();	/* function performing the asn encode/decode */
} ASNFUNCT;



typedef struct  asn_cmd_line_struct {
	char		*in_charset;
	char		*out_charset;
	ASNFUNCT	in_asn;
	ASNFUNCT	out_asn;
} ASNCMD;



typedef struct  asn_body_struct {	/* -- holds the body part contents -- */
	char			*line;
	int			length;
	struct asn_body_struct	*next;
} ASNBODY;


extern int		errno;
extern char		*sys_errlist[];
extern PE		asn_rd_stdin();


#define NULLASNCMD	((ASNCMD *)0)
#define NULLASNBODY	((ASNBODY *)0)
#define IGNORE_PRMS	1

#endif
