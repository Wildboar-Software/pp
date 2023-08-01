/* retcode.h: reply & status codes used throughout pp */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/retcode.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: retcode.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_RETCODE
#define _H_RETCODE



/*
------------------------------------------------------------------------------

			Reply Codes for PP

 Based on: "Revised FTP Reply Codes", by Jon Postel & Nancy Neigus Arpanet
     RFC 640 / NIC 30843, in the "Arpanet Protocol Handbook", E.  Feinler
     and J. Postel (eds.), NIC 7104, Network Information Center, SRI
     International:  Menlo Park, CA.  (NTIS AD-A0038901)

 Basic format:

     0yz: positive completion; entire action done
     1yz: positive intermediate; only part done
     2yz: Transient negative completion; may work later
     3yz: Permanent negative completion; you lose forever

     x0z: syntax
     x1z: general; doesn't fit any other category
     x2z: connections; truly transfer-related
     x3z: user/authentication/account
     x4x: mail
     x5z: file system

     3-bit z field is unique to the reply.  In the following,
     the RP_xVAL defines are available for masking to obtain a field.

------------------------------------------------------------------------------
*/


/*++++++++++++++  Field Definitions & Basic Values  +++++++++++++++++++++ */


/*
Field 1:  Basic degree of success (2-bits)
*/

#define RP_BTYP         0200            /* good vs. bad; on => bad */

#define RP_BVAL         0300            /* basic degree of success */

#define RP_BOK          0000            /* went fine; all done */
#define RP_BPOK         0100            /* only the first part got done */
#define RP_BTNO         0200            /* temporary failure; try later */
#define RP_BNO          0300            /* not now, nor never; you lose */


/*
Field 2:  Basic domain of discourse (3-bits)
*/

#define RP_CVAL         0070            /* basic category (domain) of reply */

#define RP_CSYN         0000            /* purely a matter of form */
#define RP_CGEN         0010            /* couldn't find place for it */
#define RP_CCON         0020            /* data-transfer-related issue */
#define RP_CUSR         0030            /* pertaining to the user */
#define RP_CMAI         0040            /* specific to mail semantics */
#define RP_CFIL         0050            /* file system */
#define RP_CLIO         0060            /* local i/o system */

/*
Field 3:  Specific value for this reply (3-bits)
*/

#define RP_SVAL         0007            /* specific value of reply */


/*++++++++++++++++++++  Specific Success Values  ++++++++++++++++++++++++ */


/*
Complete Success
*/

#define RP_DONE         (RP_BOK | RP_CGEN | 0000)
			/* done (e.g., w/transaction) */
#define RP_OK           (RP_BOK | RP_CGEN | 0001)
			/* general-purpose OK */
#define RP_MOK          (RP_BOK | RP_CMAI | 0000)
			/* message is accepted (w/text) */
#define RP_DOK          (RP_BOK | RP_CGEN | 0003)
			/* accepted for the delayed submission channel  */


/*
Partial Success
*/

#define RP_MAST         (RP_BPOK| RP_CGEN | 0000)
			/* you are the requestor */
#define RP_SLAV         (RP_BPOK| RP_CGEN | 0001)
			/* you are the requestee */
#define RP_AOK          (RP_BPOK| RP_CMAI | 0000)
			/* message address is accepted */
#define RP_HOK          (RP_BPOK| RP_CMAI | 0001)
			/* host processing completed */


/*++++++++++++++++++++  Specific Failure Values  ++++++++++++++++++++++++ */


/*
Partial Failure
*/

#define RP_AGN          (RP_BTNO | RP_CGEN | 0000)
			/* not now; maybe later */
#define RP_TIME         (RP_BTNO | RP_CGEN | 0001)
			/* timeout */
#define RP_NOOP         (RP_BTNO | RP_CGEN | 0002)
			/* no-op; nothing done, this time */
#define RP_EOF          (RP_BTNO | RP_CGEN | 0003)
			/* encountered an end of file */
#define RP_NET          (RP_BTNO | RP_CCON | 0000)
			/* channel went bad */
#define RP_BHST         (RP_BTNO | RP_CCON | 0001)
			/* foreign host screwed up */
#define RP_DHST         (RP_BTNO | RP_CCON | 0002)
			/* host went away */
#define RP_NIO          (RP_BTNO | RP_CCON | 0004)
			/* general net i/o problem */
#define RP_NS           (RP_BTNO | RP_CCON | 0005)
			/* temporary nameserver failure */
#define RP_FIO          (RP_BTNO | RP_CFIL | 0000)
			/* error reading/writing file */
#define RP_FCRT         (RP_BTNO | RP_CFIL | 0001)
			/* unable to create file */
#define RP_FOPN         (RP_BTNO | RP_CFIL | 0002)
			/* unable to open file */
#define RP_LIO          (RP_BTNO | RP_CLIO | 0000)
			/* general local i/o problem */
#define RP_LOCK         (RP_BTNO | RP_CLIO | 0001)
			/* resource currently locked */


/*
Complete Failure
*/

#define RP_MECH         (RP_BNO | RP_CGEN | 0000)
			/* bad mechanism/path; try alternate? */
#define RP_NO           (RP_BNO | RP_CGEN | 0001)
			/* general-purpose NO */
#define RP_BAD          (RP_BNO | RP_CGEN | 0002)
			/* another general purpose NO */
#define RP_PROT         (RP_BNO | RP_CCON | 0000)
			/* general prototocol error */
#define RP_RPLY         (RP_BNO | RP_CCON | 0001)
			/* bad reply code (PERMANENT ERROR)   */
#define RP_NAUTH        (RP_BNO | RP_CUSR  | 0001)
			/* bad authorisation */
			/* SEK this will be used for user checks*/
#define RP_NDEL         (RP_BNO | RP_CMAI | 0000)
			/* couldn't deliver */
#define RP_HUH          (RP_BNO | RP_CSYN | 0000)
			/* couldn't parse the request */
#define RP_NCMD         (RP_BNO | RP_CSYN | 0001)
			/* no such command defined */
#define RP_PARM         (RP_BNO | RP_CSYN | 0002)
			/* bad parameter */
#define RP_UCMD         (RP_BNO | RP_CSYN | 0003)
			/* command not implemented */
#define RP_PARSE        (RP_BNO| RP_CSYN | 0004)
			/* address parse error */
#define RP_USER         (RP_BNO | RP_CUSR | 0000)
			/* unknown user */


/*
Structure of a Reply String
*/


struct rp_construct     /* for constant reply conditions */
{
    unsigned char       rp_cval;
    char                rp_cline[50];
};


struct rp_bufstruct     /* for reading reply strings */
{
    unsigned char       rp_val;
    char                rp_line[256];
};


typedef struct rp_bufstruct     RP_Buf;

#define rp_conlen(bufnam) (strlen (bufnam.rp_cline) + sizeof (bufnam.rp_cval))



/*
Pseudo-functions to access reply info
*/

#define rp_gval(val)    ((val) & 0377)
			/* get the value of the return code */
/*
The next three give the field's bits, within the whole value
*/

#define rp_gbval(val)   ((val) & RP_BVAL)
			/* get the basic part of return value */
#define rp_gcval(val)   ((val) & RP_CVAL)
			/* get the domain part of value */
#define rp_gsval(val)   ((val) & RP_SVAL)
			/* get the specific part of value */

/*
The next three give the numeric value withing the field
*/

#define rp_gbbit(val)   (((val) >> 6) & 03)
			/* get the basic part right-shifted */
#define rp_gcbit(val)   (((val) >> 3 ) & 07)
			/* get the domain part right-shifted */
#define rp_gsbit(val)   ((val) & 07)
			/* get the specific part right-shifted */
#define rp_isgood(val)  (! rp_isbad(val))
			/* is return value positive? */
#define rp_isbad(val)   ((val) & 0200)
			/* is return value negative? */

extern char *rp_valstr ();


#endif
