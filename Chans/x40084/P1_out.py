-- P1: P1 MHS definitions


-- @(#) $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/P1_out.py,v 6.0 1991/12/18 20:13:50 jpo Rel $
--
-- $Log: P1_out.py,v $
-- Revision 6.0  1991/12/18  20:13:50  jpo
-- Release 6.0
--
--
--


P1 DEFINITIONS ::=

%{
#ifndef lint
static char Rcsid [] = "@(#)$Header";
#endif  lint

#include "head.h"
#include "q.h"
#include "or.h"
#include "dr.h"
#include "rtsparams.h" 
#include <isode/cmd_srch.h>


#define PPSTRUCT		"/usr/pp/pp/logs/x400out84.pdus"


extern char             	*body_string;
extern char             	*cont_p2;
extern int			trace_type;
extern int              	body_len;
extern Q_struct			*PPQuePtr;
extern DRmpdu			*DRptr;
extern ADDR			*ad_list;


static int			aninteger;
struct botheit {
        EncodedIT       *eit;
        LIST_BPT        *bp;
};



/* -- local routines -- */
int				testdebug();
static int			get_content_type();
static int			or_print_struct();
static int			pmf2int();
static int			same_trace();
static void			compress_trace();
static void			compress_trace_admd_style();
static void			compress_trace_localinternal_style();
static void			convert2printablestring();




/* ----------------------  Begin Routines ----------------------------------- */





#ifdef  DEBUG
int     testdebug (pe, s)
register PE     pe;
register char   *s;
{
    register PS ps;
    static int debug = OK;
    static FILE *fptrace;

    switch (debug) {
        case NOTOK:
            return;

        case OK:
            if ((pp_log_norm -> ll_events & LLOG_PDUS) == 0) {
                debug = NOTOK;
                break;
            }
            debug = 1;
            fflush (stdout);
            if (access (PPSTRUCT, 02) == NOTOK) {
                debug = NOTOK;
                break;
            }

            fptrace = fopen (PPSTRUCT, "a");
            if (fptrace == NULL) {
                debug = NOTOK;
                break;
            }
            fprintf (fptrace, "testdebug made with %s\n", pepyid);
            /* and fall... */

        default:
            fflush (stdout);
            fprintf (fptrace, "%s\n", s);

            if ((ps = ps_alloc (std_open)) == NULLPS)
                break;
            if (std_setup (ps, fptrace) != NOTOK)
                (void) pe2pl (ps, pe);
            fprintf (fptrace, "--------\n");
            (void) fflush (fptrace);
            ps_free (ps);
            break;
    }
}

#endif  /* DEBUG */

%}




BEGIN

ENCODER build


-- P1 makes use of types defined in the following module:
-- Sa: Recommendation S.a [14]
-- T73: T.73, Section 5

MPDU ::=
        CHOICE << PPQuePtr->msgtype == MT_UMPDU ? 1 : 2 >> {
            [0]
                IMPLICIT UserMPDU [[p PPQuePtr]],

                -- service mpdus are sent as ordinary messages

                ServiceMPDU [[p PPQuePtr]]
        }



ServiceMPDU [[P Q_struct *]] ::=
        CHOICE << parm->msgtype == MT_DMPDU ? 1 : 2 >> {
            [1]
                IMPLICIT DeliveryReportMPDU [[p parm]],

            [2]
                IMPLICIT ProbeMPDU [[p parm]]
        }



UserMPDU [[P Q_struct *]] ::=
        SEQUENCE {
            envelope
                UMPDUEnvelope [[p parm]],

            content
                UMPDUContent [[o body_string $ body_len]]
        }




UMPDUEnvelope [[P Q_struct *]]
        %{
                char            *cp;
                ADDR            *ap;
                struct botheit  b;
        %} ::=
        SET {
            mpduID
                MPDUIdentifier [[p &parm->msgid]],

            originator %{
                cp = parm->Oaddress->ad_r400adr;
                if (cp == NULLCP)
                        cp = parm->Oaddress->ad_value;
                %}
                ORName [[p cp]],


            original %{
                b.eit = &parm->orig_encodedinfo;
                b.bp = ad_list -> ad_eit;
                %} EncodedInformationTypes [[p &b]],


            ContentType [[i get_content_type (ad_list, parm)]],

            UAContentId [[p parm->ua_id]] OPTIONAL
                        <<parm->ua_id != NULLCP >>,

            Priority [[i parm->priority]] DEFAULT normal 
			<<parm->priority != 0 >>,

            PerMessageFlag [[p parm]]
			DEFAULT {} <<parm-> disclose_recips ||
				parm -> implicit_conversion_prohibited ||
				parm -> alternate_recip_allowed ||
				parm -> content_return_request >>,

            deferredDelivery [0]
                IMPLICIT Time
                        [[p parm->defertime]] OPTIONAL
                        <<parm->defertime != NULLUTC>>,

            [1]
                IMPLICIT SEQUENCE OF
                        PerDomainBilateralInfo OPTIONAL,

            [2]
                IMPLICIT SEQUENCE OF
                        <<ap=parm->Raddress; ap!=NULLADDR; ap=ap->ad_next>>
                        RecipientInfo [[p ap]],

            TraceInformation [[p &parm->trace]]

        }



UMPDUContent ::=
        OCTETSTRING [[o body_string $ body_len]]



-- time

Time [[P UTC]]
        %{
                char *str;
        %} ::=
        %{ str= utct2str (parm); %}
        UniversalTime [[s str]]



-- various envelope information

MPDUIdentifier [[P MPDUid *]]
        %{
                char tbuf[LINESIZE];
        %} ::=
        [APPLICATION 4] IMPLICIT SEQUENCE {
            GlobalDomainIdentifier
                [[p &parm->mpduid_DomId]],

        dummy %{
	    bzero (tbuf, LINESIZE);		
            strncpy (tbuf, parm -> mpduid_string, 32);
        %}
            IA5String [[s tbuf]]
        }



ContentType ::=
        [APPLICATION 6]
            IMPLICIT INTEGER {
                p2 (2)
            }



UAContentId [[P char *]]
        %{
                char tbuf[LINESIZE];
        %} ::=
        %{
		bzero (tbuf, LINESIZE);		
                strncpy (tbuf, parm, 16);
        %}
        [APPLICATION 10]
            IMPLICIT PrintableString [[s tbuf]]


Priority ::=
        [APPLICATION 7]
            IMPLICIT INTEGER {
                normal (0),
                nonUrgent (1),
                urgent (2)
            }



PerMessageFlag [[P Q_struct *]] ::=
        [APPLICATION 8] 
            IMPLICIT BITSTRING [[x int2strb(pmf2int(parm),4) $ 4 ]] {
                discloseRecipients (0),
                conversionProhibited (1),
                alternateRecipientAllowed (2),
                contentReturnRequest (3)
            } 



-- per-domain bilateral information

PerDomainBilateralInfo ::=
        SEQUENCE {
            country
                CountryName,
                AdministrationDomainName,
                BilateralInfo
        }



BilateralInfo ::=
        ANY



-- recipient information

RecipientInfo [[P ADDR *]]
        %{
                char            *cp;
        %} ::=
        SET {
            recipient %{
                cp = parm->ad_r400adr;
                if (cp == NULLCP)
                        cp = parm->ad_value;
                %}
                ORName [[p cp]],

            [0]
                IMPLICIT ExtensionIdentifier [[i parm->ad_extension]],

            [1]
                %{
			int	resp;
			ADDR	*ap;
			for (ap = ad_list; ap; ap = ap -> ad_next)
				if (ap -> ad_no == parm -> ad_no)
					break;
			if (ap == NULLADDR)
				resp = FALSE;
			else	resp = parm -> ad_resp;
                        aninteger = mem2prf (resp, parm->ad_mtarreq,
                                        parm->ad_usrreq);
                %}
                IMPLICIT PerRecipientFlag [[x int2strb (aninteger, 8) $ 8 ]],

            [2]
                IMPLICIT ExplicitConversion
                        [[i parm->ad_explicitconversion]] DEFAULT {}
        }



ExtensionIdentifier ::=
        INTEGER


PerRecipientFlag ::=
        BITSTRING -- See Figure 23/X.411


ExplicitConversion ::=
        INTEGER {
            iA5TextTeletex (0),
            teletexTelex (1)
        }



-- trace information

TraceInformation [[P Trace **]]
        %{
                Trace   *tp;
        %} ::=


	%{
		compress_trace (parm);
	%}


        [APPLICATION 9]
            IMPLICIT SEQUENCE OF
                <<tp = *parm; tp; tp = tp->trace_next>>
                        SEQUENCE {
                                domainid
                                        GlobalDomainIdentifier
                                                [[p &tp->trace_DomId]],
                                domaininfo
                                        DomainSuppliedInfo
                                                [[p &tp->trace_DomSinfo]]
            }



DomainSuppliedInfo [[P DomSupInfo *]]
        %{
                struct botheit b;
        %} ::=
        SET {
            arrival [0] %{
		if (parm -> dsi_time == NULL)
			parm->dsi_time = utcnow ();
		%}
                IMPLICIT Time [[p parm->dsi_time]],

            deferred [1]
                IMPLICIT Time [[p parm->dsi_deferred]] OPTIONAL
                        <<parm->dsi_deferred != NULLUTC>>,

            action [2]
                IMPLICIT INTEGER [[i parm->dsi_action]] {
                    relayed (0),
                    rerouted (1)
                },

            converted %{
                b.eit = &parm->dsi_converted;
                b.bp = NULL;
            %} EncodedInformationTypes
                    [[p &b]] OPTIONAL
                        <<parm->dsi_converted.eit_types != 0>>,

            previous
                GlobalDomainIdentifier
                    [[p &parm->dsi_attempted_md]] OPTIONAL
                        <<parm->dsi_attempted_md.global_Country != NULLCP>>
        }



-- global domain identifier

GlobalDomainIdentifier [[P GlobalDomId *]] ::=
        [APPLICATION 3]
            IMPLICIT SEQUENCE {
                CountryName [[p parm->global_Country]],
                AdministrationDomainName [[p parm->global_Admin]],
                PrivateDomainIdentifier [[p parm->global_Private]] OPTIONAL
                        <<parm->global_Private != NULLCP>>
            }


CountryName [[P char *]] ::=
        [APPLICATION 1]
            CHOICE << (parm && or_str_isns(parm)) ? 1 : 2 >> {
                NumericString [[s parm ? parm : "" ]],
                PrintableString [[s parm ? parm : "" ]]
            }



AdministrationDomainName [[P char *]] ::=
        [APPLICATION 2]
            CHOICE << (parm && or_str_isns(parm)) ? 1 : 2 >> {
                NumericString [[s parm ? parm : ""]],
                PrintableString [[s parm ? parm : ""]]
            }



PrivateDomainIdentifier [[P char *]] ::=
        CHOICE << (parm && or_str_isns(parm)) ? 1 : 2 >> {
            NumericString [[s parm ? parm : ""]],
            PrintableString [[s parm ? parm : ""]]
        }




-- O/R name

ORName [[P char *]]
        %{
                OR_ptr  or;
                PE      peptr = NULLPE; 

                or = or_std2or (parm);
                if (or == NULLOR) {
                        advise (NULLCP, "x400out84: Bad ORName %s", parm);
			return NOTOK;
		}
		or_downgrade(&or);
                or_print_struct (or, 0);

                if (build_OR_ORName (&peptr, 1, 0, NULLCP, or) == NOTOK)
                        if (peptr) {
                                pe_free (peptr);
                                peptr= NULLPE;
                        }
                or_free (or);
        {
        %} ::=
        ANY [[a peptr]]
        %{
        }
        %}



-- encoded information types

EncodedInformationTypes [[P struct botheit *]] ::=
        %{
                if (parm->bp)
                        aninteger = mem2enctypes (parm->bp);
                else
                        aninteger = mem2enctypes (parm->eit->eit_types);
        %}

        [APPLICATION 5] IMPLICIT SET {
            [0]
                IMPLICIT BITSTRING  [[x int2strb (aninteger, 10) $ 10]]
                {
                        undefined(0),
                        tLX(1),
                        iA5Text(2),
                        g3Fax(3),
                        tIF0(4),
                        tTX(5),
                        videotex(6),
                        voice(7),
                        sFD(8),
                        tIF1(9)
                },


            [1]
                IMPLICIT G3NonBasicParams
                        [[x int2strb (parm->eit->eit_g3parms, 30) $ 30]] OPTIONAL
                                <<list_bpt_find (parm->eit->eit_types,
                                                "P2.g3fax")>>,
            [2]
                IMPLICIT TeletexNonBasicParams OPTIONAL,

            [3]
                IMPLICIT PresentationCapabilities OPTIONAL

        -- other non-basic parameters are for further study

        }



G3NonBasicParams ::=
        BITSTRING {
            twoDimensional(8),
            fineResolution(9),
            unlimitedLength(20),
            b4Length(21),
            a3Width(22),
            b4Width(23),
            uncompressed(30)
        }



TeletexNonBasicParams ::=
        SET {
            graphicCharacterSets [0]
                IMPLICIT T61String OPTIONAL,

            controlCharacterSets [1]
                IMPLICIT T61String OPTIONAL,

            pageFormats [2]
                IMPLICIT OCTETSTRING OPTIONAL,

            miscTerminalCapabilities [3]
                IMPLICIT T61String OPTIONAL,

            privateUse [4]
                IMPLICIT OCTETSTRING OPTIONAL
        }



PresentationCapabilities ::=
        T73PresentationCapabilities



T73PresentationCapabilities ::=
        SET { --unimportant-- }




-- Delivery Report MPDU

DeliveryReportMPDU [[P Q_struct *]]::=
        SEQUENCE {
            envelope
                DeliveryReportEnvelope [[p parm]],

            content
                DeliveryReportContent [[p parm]]
        }




DeliveryReportEnvelope [[P Q_struct *]] ::=
        SET {
            report
                MPDUIdentifier [[p DRptr-> dr_mpduid]],

            originator
                ORName [[p parm->Oaddress->ad_r400adr]],

            TraceInformation [[p &DRptr->dr_trace]]
        }



DeliveryReportContent [[P Q_struct *]]
        %{
                Rrinfo   *rq;
        %} ::=
        SET {
            original
                MPDUIdentifier [[p &parm->msgid]],

            intermediate
                TraceInformation [[p &DRptr-> dr_subject_intermediate_trace]]
                OPTIONAL << DRptr -> dr_subject_intermediate_trace>>,

            UAContentId [[p parm->ua_id]] OPTIONAL
                        <<parm->ua_id != NULLCP>>,

            [0]
                IMPLICIT SEQUENCE OF
                        <<rq = DRptr->dr_recip_list; rq;
                          rq = rq->rr_next>>
                ReportedRecipientInfo [[p rq]],

            returned [1]
                IMPLICIT UMPDUContent [[o body_string $ body_len ]] OPTIONAL <<body_len != 0>>,

            billingInformation [2]
                ANY OPTIONAL
        }



ReportedRecipientInfo [[P Rrinfo *]]
        %{
                ADDR    *ap;
        %} ::= %{
                for (ap = PPQuePtr -> Raddress; ap; ap = ap -> ad_next)
                        if (ap -> ad_no == parm -> rr_recip)
                                break;
                if (ap == NULLADDR) {
                        PP_OPER (NULLCP, ("NO match for recip %d terminating",
                                          parm -> rr_recip));
                        exit (3);
                }
        %}
        SET
        {
            recipient [0]
                IMPLICIT ORName [[p ap->ad_r400adr]] %{
		$$ -> pe_class = PE_CLASS_CONT;
		$$ -> pe_id = 0;
		%},

            [1]
                IMPLICIT ExtensionIdentifier [[i ap->ad_extension]],

            [2]
                %{
                        aninteger = mem2prf (ap->ad_resp,
                                         ap->ad_mtarreq,
                                         ap->ad_usrreq);
                %}
                IMPLICIT PerRecipientFlag [[x int2strb (aninteger, 8) $ 8 ]],

            [3] IMPLICIT LastTraceInformation [[p parm]],

            intendedRecipient [4]
                IMPLICIT 
			ORName [[p parm->rr_originally_intended_recip->fn_addr]]
				%{
					$$ -> pe_class = PE_CLASS_CONT;
					$$ -> pe_id = 4;
				%} OPTIONAL
				<<parm->rr_originally_intended_recip &&
                           	parm->rr_originally_intended_recip->fn_addr>> ,

            [5]
                %{
			convert2printablestring (parm -> rr_supplementary);
		%} 
                IMPLICIT SupplementaryInformation
                        	[[o parm -> rr_supplementary $ 
                        	min ((int)strlen(parm->rr_supplementary), 64)]] 
			OPTIONAL
			<<parm->rr_supplementary != NULLCP>>
        }



-- last trace information

LastTraceInformation [[P Rrinfo *]]
        %{
                struct botheit b;
        %} ::=
        SET {
            arrival [0]
                IMPLICIT Time [[p parm->rr_arrival]],

            converted %{
                b.eit = parm->rr_converted;
                b.bp = NULL;
            %} EncodedInformationTypes [[p &b]] OPTIONAL
                        <<parm->rr_converted != 0>>,

            [1]
                Report [[p &parm->rr_report]]
        }



Report [[P Report *]] ::=
        CHOICE <<parm->rep_type == DR_REP_SUCCESS ? 1 : 2 >> {
            [0]
                IMPLICIT DeliveredInfo [[p &parm->rep.rep_dinfo]],

            [1]
                IMPLICIT NonDeliveredInfo [[p &parm->rep.rep_ndinfo]]
        }



DeliveredInfo [[P Delinfo *]] ::=
        SET {
            delivery [0]
                IMPLICIT Time [[p parm->del_time]],

            typeOfUA [1]
                IMPLICIT INTEGER [[i parm->del_type]] {
                    public (0),
                    private (1)
                } DEFAULT public <<parm->del_type != 0>>
        }



NonDeliveredInfo [[P NonDelinfo *]] ::=
        SET {
            [0]
                %{
                        if (parm->nd_rcode > 2)
                                parm->nd_rcode = 0;
                %}
                IMPLICIT ReasonCode [[i parm->nd_rcode]],

            [1]
                %{
                        if (parm->nd_dcode > 11)
                                parm->nd_dcode = 11;
                %}
                IMPLICIT DiagnosticCode [[i parm->nd_dcode]] OPTIONAL
                        <<parm->nd_dcode >= 0>>
        }



ReasonCode ::=
        INTEGER {
            transferFailure (0),

            unableToTransfer (1),

            conversionNotPerformed (2)

        }



DiagnosticCode  ::=
        INTEGER {
            unrecognizedORName (0),

            ambiguousORName (1),

            mtaCongestion (2),

            loopDetected (3),

            uaUnavailable (4),

            maximumTimeExpired (5),

            encodedInformationTypesUnsupported (6),

            contentTooLong (7),

            conversionImpractical (8),

            conversionProhibited (9),

            implicitConversionNotResgistered (10),

            invalidParameters (11)
        }



-- supplementary information

SupplementaryInformation ::=
        PrintableString -- length limited and for further study




-- Probe MPDU

ProbeMPDU [[P Q_struct *]]::=
        ProbeEnvelope [[p parm]]



ProbeEnvelope [[P Q_struct *]]
        %{
                char    *cp;
                ADDR    *ap;
                struct botheit b;
        %} ::=

        SET {
            probe
                MPDUIdentifier [[p &parm->msgid]],


            originator %{
                cp = parm->Oaddress->ad_r400adr;
                if (cp == NULLCP)
                        cp = parm->Oaddress->ad_value;
                %}
                ORName [[p cp]],


            ContentType [[i get_content_type (ad_list, parm)]],

            UAContentId [[p parm->ua_id]] OPTIONAL
                        <<parm->ua_id != NULLCP>>,

            original %{
                b.eit = &parm->orig_encodedinfo;
                b.bp = NULL;
            %} EncodedInformationTypes
                         [[p &b]] OPTIONAL
                                <<parm->orig_encodedinfo.eit_types != 0>>,

            TraceInformation [[p &parm->trace]],

            PerMessageFlag [[p parm]]
		DEFAULT {} << parm -> implicit_conversion_prohibited ||
			parm -> alternate_recip_allowed >>,

            contentLength [0]
                IMPLICIT INTEGER [[i parm->msgsize]] OPTIONAL
			<<parm->msgsize > 0>>,

            [1]
                IMPLICIT SEQUENCE OF PerDomainBilateralInfo
                OPTIONAL,

            [2]
                IMPLICIT SEQUENCE OF
                << ap = parm -> disclose_recips ? parm->Raddress : ad_list; 
		   ap != NULLADDR; ap=ap->ad_next>>
                        RecipientInfo [[p ap]]
}

END




%{


static int get_content_type (ad, qp)
ADDR            *ad;
Q_struct        *qp;
{
        char    *p;

        if (isstr(ad -> ad_content))
                p = ad -> ad_content;
        else    p = qp -> cont_type;

        if (lexequ (p, cont_p2) != 0)
                adios (NULLCP, "Bad content type %s", p);

        return 2;
}



static int pmf2int (parm)  /* --- PerMessageFlag Params -> integer --- */
Q_struct        *parm;
{

	aninteger = 0;

	switch (parm -> msgtype) {
	case MT_PMPDU: 
		/* --- *** --- 
		only the conversionProhibited and alternateRecipientAllowed
		flags affect the deliverability of the message as tested by 
		Probe. - See X.411 page 164.
		--- *** --- */
		
        	if (parm -> implicit_conversion_prohibited) 
                	aninteger |= (1 << 1);
        	if (parm -> alternate_recip_allowed) 
                	aninteger |= (1 << 2);
		break;

	default:
        	if (parm -> disclose_recips) 
                	aninteger |= (1 << 0);
        	if (parm -> implicit_conversion_prohibited) 
                	aninteger |= (1 << 1);
        	if (parm -> alternate_recip_allowed) 
                	aninteger |= (1 << 2);
        	if (parm -> content_return_request) 
                	aninteger |= (1 << 3);
		break;
	}

        return aninteger;
}



/* --- *** ---  
This routine is necessary because sometimes the supplementary
information field has return characeters which is not valid X.409
--- *** --- */

static void convert2printablestring (ptr)
char	*ptr;
{
	char	*cp;

	for (cp = ptr; isstr(cp); cp++)
		switch (*cp) {
		case '\n':
		case '\t':
			*cp = ' ';
			break;
		default:
			break;
		}
}




static void compress_trace (tpp)
Trace		**tpp;
{

	switch (trace_type) {
	case RTSP_TRACE_ADMD:
		compress_trace_admd_style (tpp);
		break;
	case RTSP_TRACE_LOCALINT:
		compress_trace_localinternal_style (tpp);
		break;
	}
	
	return;
}




static void compress_trace_admd_style (tpp)
Trace		**tpp;
{
	Trace	*tlast,
		*tlast_butone = (Trace *)0,
		*tp = *tpp; 


	/* --- *** --- 
	Keep the first and last. Get rid of intermediate. If the ADMD is 
	the same in the first and last trace, then keep only the first. 
	--- *** --- */

	for (tlast = tp; tlast && tlast -> trace_next;
                tlast = tlast -> trace_next) {
                        tlast_butone = tlast;
                        continue;
        }

        if (tlast != tp && same_trace (tlast, tp)) {
                /* -- keep only the first trace -- */
                trace_free (tp -> trace_next);
                tp -> trace_next = (Trace *)0;
        }
        else if (tlast != tp) {
                /* -- keep first and last, remove intermediates -- */
                tlast_butone -> trace_next = (Trace *)0;
                trace_free (tp -> trace_next);
                tp -> trace_next = tlast;
        }

	*tpp = tp;
	return;
}




static void compress_trace_localinternal_style (tpp)
Trace		**tpp;
{
	Trace	*tnew,	
		*tprev = (Trace *)0,
		*tptr,
		*tp = *tpp;

	/* -- *** -- 
	Throw away ALL trace that is not of the local PRMD 
	-- *** -- */

	tnew = trace_new();

	for (tptr = tp; tptr; tptr = tptr -> trace_next) {
		if (!same_trace (tnew, tptr))
			break;
		tprev = tptr;
	}
	
	if (tprev)
		tprev -> trace_next = (Trace *)0;

	trace_free (tptr);
	trace_free (tnew);

	*tpp = tp;

	return;
}




static int same_trace (tp1, tp2)
Trace	*tp1;
Trace	*tp2;
{
	GlobalDomId	*d1 = &tp1 -> trace_DomId;
	GlobalDomId	*d2 = &tp2 -> trace_DomId;


	if (d1 -> global_Country && d2 -> global_Country &&
	    lexequ (d1 -> global_Country, d2 -> global_Country) == 0 &&
	    d1 -> global_Admin && d2 -> global_Admin &&
	    lexequ (d1 -> global_Admin, d2 -> global_Admin) == 0) {

			if (d1 -> global_Private == NULL &&
		    		d2 -> global_Private == NULL)
					return TRUE;

			if (d1 -> global_Private && d2 -> global_Private &&
		    		lexequ (d1 -> global_Private,
					d2 -> global_Private) == 0)
					return TRUE;
	}

	return FALSE;
}




static int or_print_struct (or, indent)
OR_ptr  or;
int	indent;
{
	char	buf[BUFSIZ];
	int	i;

#ifdef  DEBUG
	if (or == NULLOR) return;

	for (i = 0; i < indent; i++)
		buf[i] = ' ';
	buf[i] = '\0';

	PP_TRACE (("%s%s = %s", buf, or_type2name(or->or_type), or->or_value));

	or_print_struct (or -> or_next, indent + 3);
#endif  DEBUG
}


%}
