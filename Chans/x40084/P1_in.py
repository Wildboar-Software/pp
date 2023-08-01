-- P1: P1 MHS definitions


-- @(#) $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/P1_in.py,v 6.0 1991/12/18 20:13:50 jpo Rel $
--
-- $Log: P1_in.py,v $
-- Revision 6.0  1991/12/18  20:13:50  jpo
-- Release 6.0
--
-- 
--



P1 DEFINITIONS ::=

%{
#ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/x40084/RCS/P1_in.py,v 6.0 1991/12/18 20:13:50 jpo Rel $";
#endif  lint

#include "head.h"
#include "prm.h"
#include "q.h"
#include "or.h"
#include "dr.h"
#include <isode/cmd_srch.h>

#define PPSTRUCT        "/usr/pp/pp/logs/x400in84.pdus"

extern char             *loc_dom_site;
extern char             *pp_myname;
extern char             *remote_site;
extern char		*undefined_bodyparts;
extern char             *cont_p2;


static DRmpdu           DRstruct;
DRmpdu                  *DRptr = &DRstruct;
static OR_ptr           orptr;
static Q_struct         Qstruct;
Q_struct                *PPQuePtr = &Qstruct;
static int              ad_no;


ADDR                    *adr_new();
char                    *pe2str();
char                    *body_string;
int                     body_len;


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

DECODER do


-- P1 makes use of types defined in the following module:
-- Sa: Recommendation S.a[14]
-- T73: T.73, Section 5

MPDU ::=
            %{
                        q_init (PPQuePtr);   /* initialise Q struct */
                        dr_init (&DRstruct); /* initialise DR struct */
                        PPQuePtr->inbound =
                                list_rchan_new (remote_site, pp_myname);
                        ad_no = 1;
            %}

        CHOICE {
            [0]
                IMPLICIT UserMPDU [[p PPQuePtr]],

                -- service mpdus are sent as ordinary messages

                ServiceMPDU [[p PPQuePtr]]
        }



ServiceMPDU [[P Q_struct *]] ::=
        CHOICE {
            [1]
                IMPLICIT DeliveryReportMPDU [[p PPQuePtr]]
                %{
                        PPQuePtr->msgtype = MT_DMPDU;
                %},

            [2]
                IMPLICIT ProbeMPDU [[p PPQuePtr]]
                %{
                        PPQuePtr->msgtype = MT_PMPDU;
                %}
        }


UserMPDU [[P Q_struct *]] ::=
        SEQUENCE {
            envelope
                %{
                        PPQuePtr->msgtype = MT_UMPDU;
                        ad_no = 1;
                %}
                UMPDUEnvelope [[p PPQuePtr]],

            content
                UMPDUContent
                %{
                        body_string = prim2str ($$, &body_len);
                        PPQuePtr->msgsize = body_len;
                %}
        }

UMPDUEnvelope [[P Q_struct *]] %{
        ADDR    *ad_link;
        char    *sp;
        int     n;
        %} ::=
        SET {
            mpduID
                MPDUIdentifier [[p &parm->msgid]],

            originator
                ORName [[p &sp]]
                %{
                        parm->Oaddress = adr_new (sp, AD_X400_TYPE, 0);
                %},

            original
                EncodedInformationTypes
                        [[p &parm->orig_encodedinfo]] OPTIONAL,

            ContentType [[i n]]
            %{  if (n != 2) {
                        PP_OPER (NULLCP, ("Bad content type %d aborting",n));
                        exit (2);
                }
                parm->cont_type = strdup (cont_p2); 
	    %},

            UAContentId [[s parm->ua_id]] OPTIONAL,

            Priority [[i parm->priority]] DEFAULT normal,

            PerMessageFlag [[p parm]] DEFAULT {},

            deferredDelivery[0]
                IMPLICIT Time
                        [[p &parm->defertime]] OPTIONAL,

            [1]
                IMPLICIT SEQUENCE OF
                        PerDomainBilateralInfo
                        OPTIONAL,

            [2]
                IMPLICIT SEQUENCE OF RecipientInfo [[p &ad_link]]
                %{
                        register ADDR   **ap;
                        for (ap = &(parm->Raddress);
                                *ap; ap = &(*ap)->ad_next);
                        *ap = ad_link;
                %},

            TraceInformation [[p &parm->trace]],

            -- this one's for EAN
            [UNIVERSAL 17]
                IMPLICIT ANY OPTIONAL
        }


UMPDUContent ::=
        OCTETSTRING


-- time

Time [[P UTC *]] ::=
        UniversalTime
        %{
                char    *p = pe2str ($$);
                UTC     ut;

                ut = str2utct (p, strlen(p));
                if (ut) {
                        *parm = (UTC) smalloc (sizeof *ut);
                        **parm = *ut;
                } else
		  	return NOTOK;
                free (p);
        %}


-- various envelope information

MPDUIdentifier [[P MPDUid *]] ::=
        [APPLICATION 4] IMPLICIT SEQUENCE {
            GlobalDomainIdentifier
                [[p &parm->mpduid_DomId]],

            IA5String [[s parm->mpduid_string]]
        }


ContentType ::=
        [APPLICATION 6]
            IMPLICIT INTEGER {
                p2 (2)
            }


UAContentId ::=
        [APPLICATION 10]
            IMPLICIT PrintableString


Priority ::=
        [APPLICATION 7]
            IMPLICIT INTEGER {
                normal (0),

                nonUrgent (1),

                urgent (2)
            }


PerMessageFlag [[P Q_struct *]] ::=
        [APPLICATION 8]
            IMPLICIT BITSTRING {
                discloseRecipients (0),
                conversionProhibited (1),
                alternateRecipientAllowed (2),
                contentReturnRequest (3)
            }
        %{
                if (bit_test ($$, 0))
                        parm -> disclose_recips = TRUE;
		else	parm -> disclose_recips = FALSE;

                if (bit_test ($$, 1))
                        parm -> implicit_conversion_prohibited = TRUE;
		else	parm -> implicit_conversion_prohibited = FALSE;

                if (bit_test ($$, 2))
                        parm -> alternate_recip_allowed = TRUE;
		else	parm -> alternate_recip_allowed = FALSE;

                if (bit_test ($$, 3))
                        parm -> content_return_request = TRUE;
		else	parm -> content_return_request = FALSE;
        %}



-- per-domain bilateral information
-- IGNORED!

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

RecipientInfo [[P ADDR **]]
        %{
                char    *sp;
                int     anint;
        %} ::=
        SET {
            recipient
                ORName [[p &sp]]
                %{
                        *parm = adr_new (sp, AD_X400_TYPE, ad_no);
                        ++ad_no;
                %},

            [0]
                IMPLICIT ExtensionIdentifier [[i (*parm)->ad_extension]],

            [1]
                IMPLICIT PerRecipientFlag [[p &anint]]
                %{
                        prf2mem (anint, &(*parm)->ad_resp,
                                        &(*parm)->ad_mtarreq,
                                        &(*parm)->ad_usrreq);
                %},

            [2]
                IMPLICIT ExplicitConversion [[i anint]]
                %{
                        (*parm)->ad_explicitconversion = anint;
                %}
                DEFAULT {}

            -- this one's for EAN --,
            [UNIVERSAL 2]
                IMPLICIT INTEGER OPTIONAL
        }

ExtensionIdentifier ::=
        INTEGER

PerRecipientFlag [[P int *]] ::=
        BITSTRING -- See Figure 23/X.411
        %{
                *parm = pe2bits ($$);
        %}

ExplicitConversion ::=
        INTEGER {
            iA5TextTeletex (0),

            teletexTelex (1)
        }



-- trace information

TraceInformation [[P Trace **]] %{
        Trace *newtrace;
        %} ::=
        [APPLICATION 9]
            IMPLICIT SEQUENCE OF SEQUENCE {
                domainid
            %{
                Trace   **tp;

                newtrace = (Trace *) smalloc (sizeof (*newtrace));
                bzero ((char *) newtrace, sizeof (*newtrace));
                for (tp = parm; *tp; tp = &(*tp)->trace_next);
                *tp = newtrace;
                newtrace->trace_next = (Trace *) NULL;
            %}
                    GlobalDomainIdentifier
                        [[p &newtrace->trace_DomId]],

                domaininfo
                    DomainSuppliedInfo
                        [[p &newtrace->trace_DomSinfo]]
            }



DomainSuppliedInfo [[P DomSupInfo *]] ::=
        SET {
            arrival[0]
                IMPLICIT Time [[p &parm->dsi_time]],

            deferred[1]
                IMPLICIT Time [[p &parm->dsi_deferred]]
                        OPTIONAL,

            action[2]
                IMPLICIT INTEGER [[i parm->dsi_action]] {
                    relayed (0),

                    rerouted (1)
                },

            converted
                EncodedInformationTypes
                    [[p &parm->dsi_converted]] OPTIONAL,

            previous
                GlobalDomainIdentifier
                    [[p &parm->dsi_attempted_md]] OPTIONAL
        }



-- global domain identifier

GlobalDomainIdentifier [[P GlobalDomId *]] ::=
        [APPLICATION 3]
            IMPLICIT SEQUENCE {
                CountryName [[p &parm->global_Country]],
                AdministrationDomainName
                    [[p &parm->global_Admin]],
                PrivateDomainIdentifier
                    [[p &parm->global_Private]] OPTIONAL
            }


CountryName [[P char **]] ::=
        [APPLICATION 1]
            CHOICE {
                NumericString [[s *parm]],

                PrintableString [[s *parm]]
            }


AdministrationDomainName [[P char **]] ::=
        [APPLICATION 2]
            CHOICE {
                NumericString [[s *parm]],
                PrintableString [[s *parm]]
            }


PrivateDomainIdentifier [[P char **]] ::=
        CHOICE {
            NumericString [[s *parm]],

            PrintableString [[s *parm]]
        }



-- O/R name

ORName [[P char **]] 
        %{ 
                PE      pe_standard = NULLPE; 
                PE      pe_domain = NULLPE; 
                char    buf[1024];
        %} ::=
        [APPLICATION 0]
            IMPLICIT SEQUENCE {
                standard
                        ANY
                        [[a pe_standard]],

                domaindefined
                        ANY
                        [[a pe_domain]]
                        OPTIONAL 
            }
            %{
                *parm = NULLCP;
                if (decode_OR_StandardAttributeList 
                        (pe_standard, 1, NULLIP, NULLVP, &orptr) == NOTOK) {
				(void) sprintf (buf, "problem decoding standard attributes: %s", PY_pepy);
                                advise (NULLCP, "%s", buf);
				or_free (orptr);
				return NOTOK;
                }

                 if (pe_domain && decode_OR_DomainDefinedAttributeList 
                        (pe_domain, 1, NULLIP, NULLVP, &orptr) == NOTOK) {
				(void) sprintf (buf, "problem decoding DD: %s",
						PY_pepy);
                                advise (NULLCP, "%s", buf);
				or_free (orptr);
				return NOTOK;
                }
                or_or2std (orptr, buf, FALSE);
                *parm = strdup (buf);
                or_free (orptr);
                orptr = NULLOR;
            %}



-- encoded information types

EncodedInformationTypes [[P EncodedIT *]] ::=
        [APPLICATION 5] IMPLICIT SET {
            [0]
                IMPLICIT BITSTRING {
                    undefined (0),
                    tLX (1),
                    iA5Text (2),
                    g3Fax (3),
                    tIF0 (4),
                    tTX (5),
                    videotex (6),
                    voice (7),
                    sFD (8),
                    tIF1 (9)
                }
                %{
                        enctypes2mem (pe2bits ($$),
				undefined_bodyparts, &parm->eit_types);
                %}
                -- this OPTIONAL is for EAN -- OPTIONAL,

            [1]
                IMPLICIT G3NonBasicParams [[p &parm->eit_g3parms]]
                OPTIONAL,

            [2]
                IMPLICIT TeletexNonBasicParams OPTIONAL,

            [3]
                IMPLICIT PresentationCapabilities OPTIONAL

        -- other non-basic parameters are for further study

            -- but this one's for EAN --,
            [UNIVERSAL 3]
                IMPLICIT BITSTRING {
                    undefined (0),
                    tLX (1),
                    iA5Text (2),
                    g3Fax (3),
                    tIF0 (4),
                    tTX (5),
                    videotex (6),
                    voice (7),
                    sFD (8),
                    tIF1 (9)
                }
                %{
                        enctypes2mem (pe2bits ($$),
				undefined_bodyparts, &parm->eit_types);
                %} OPTIONAL
        }

G3NonBasicParams [[P long *]] ::=
        BITSTRING {
            twoDimensional (8),
            fineResolution (9),
            unlimitedLength (20),
            b4Length (21),
            a3Width (22),
            b4Width (23),
            uncompressed (30)
        }
        %{ *parm = pe2bits ($$); %}

TeletexNonBasicParams ::=
        SET {
            graphicCharacterSets[0]
                IMPLICIT T61String OPTIONAL,

            controlCharacterSets[1]
                IMPLICIT T61String OPTIONAL,

            pageFormats[2]
                IMPLICIT OCTETSTRING OPTIONAL,

            miscTerminalCapabilities[3]
                IMPLICIT T61String OPTIONAL,

            privateUse[4]
                IMPLICIT OCTETSTRING OPTIONAL
        }

PresentationCapabilities ::=
        T73PresentationCapabilities


T73PresentationCapabilities ::=
        SET { --unimportant-- }


-- Delivery Report MPDU

DeliveryReportMPDU [[P Q_struct *]]::=
        %{ dr_init (&DRstruct); %}
        SEQUENCE {
            envelope
                DeliveryReportEnvelope [[p parm]],

            content
                DeliveryReportContent [[p parm]]
        }


DeliveryReportEnvelope [[P Q_struct *]] %{ char *sp; %} ::=
        SET {
            report %{
                DRptr -> dr_mpduid = (MPDUid *)
                        smalloc (sizeof (MPDUid));
                bzero ((char *)DRptr -> dr_mpduid, sizeof (MPDUid));
                %}
                MPDUIdentifier [[p DRptr->dr_mpduid]],

            originator
                ORName [[p &sp]]
                %{
                        parm->Oaddress = adr_new (sp, AD_X400_TYPE, 0);
                        parm->Oaddress->ad_resp = YES;
                %},

            TraceInformation [[p &DRptr->dr_trace]]
        }


DeliveryReportContent [[P Q_struct *]] ::=
        SET {
            original
                MPDUIdentifier [[p &parm->msgid]],

            intermediate
                TraceInformation [[p &DRptr -> dr_subject_intermediate_trace]]
                OPTIONAL,

                UAContentId [[s parm->ua_id]] OPTIONAL,

            [0]
                IMPLICIT SEQUENCE OF ReportedRecipientInfo
                    [[p parm]],

            returned[1]
                IMPLICIT UMPDUContent 
                %{
                        body_string = prim2str ($$, &body_len);
                        PPQuePtr->msgsize = body_len;
                %} OPTIONAL,

            billingInformation[2]
                ANY OPTIONAL
        }


ReportedRecipientInfo [[P  Q_struct *]]
        %{
                int     anint;
                Rrinfo   *rr;
                ADDR    *ap;
         %} ::=
        SET  %{
                Rrinfo    **rp;

                rr = (Rrinfo *) smalloc (sizeof (*rr));
                bzero ((char *) rr, sizeof (*rr));
                for (rp = &DRptr -> dr_recip_list;
                        *rp; rp = &(*rp)->rr_next);
                *rp = rr;
        %}
        {
            recipient[0]
                %{
                        if (orptr != NULLOR) {
                                or_free (orptr);
                                orptr = NULLOR;
                        }
                        ap = adr_new (NULLCP, AD_X400_TYPE, ad_no);
                        rr -> rr_recip = ad_no ++;
                        ap->ad_status = AD_STAT_DRWRITTEN;
                        adr_add (&parm -> Raddress, ap);
                %}
                IMPLICIT ORName
                        [[p &ap -> ad_value]],

            [1]
                IMPLICIT ExtensionIdentifier [[i ap->ad_extension]],

            [2]
                IMPLICIT PerRecipientFlag [[p &anint]]
                %{
                        prf2mem (anint, &ap->ad_resp,
                                        &ap->ad_mtarreq,
                                        &ap->ad_usrreq);
                %},

            [3]
                IMPLICIT LastTraceInformation
                        [[p rr]],

            intendedRecipient[4]
                %{
                        rr -> rr_originally_intended_recip = 
                                (FullName *) smalloc(sizeof(FullName));
			bzero ((char *) rr -> rr_originally_intended_recip,
				sizeof(FullName));
                %}
                IMPLICIT ORName
                        [[p &rr->rr_originally_intended_recip -> fn_addr]]
                        OPTIONAL,

            [5]
                IMPLICIT SupplementaryInformation
                        [[s rr->rr_supplementary]] OPTIONAL
        }



-- last trace information

LastTraceInformation [[P Rrinfo *]] ::=
        SET {
            arrival[0]
                IMPLICIT Time [[p &parm->rr_arrival]],

            converted
                %{
                        parm->rr_converted = (EncodedIT *)
                                smalloc (sizeof (*parm->rr_converted));
                        bzero ((char *) parm->rr_converted,
                                sizeof (*parm->rr_converted));
                %}

                EncodedInformationTypes [[p parm->rr_converted]] OPTIONAL,

            [1]
                Report [[p &parm->rr_report]]
        }


Report [[P Report *]] ::=
        CHOICE {
            [0]
                IMPLICIT DeliveredInfo
                        [[p &parm->rep.rep_dinfo]]
                %{
                        parm->rep_type = DR_REP_SUCCESS;
                %},

            [1]
                IMPLICIT NonDeliveredInfo
                        [[p &parm->rep.rep_ndinfo]]
                %{
                        parm->rep_type = DR_REP_FAILURE;
                %}
        }


DeliveredInfo [[P Delinfo *]] ::=
        SET {
            delivery[0]
                IMPLICIT Time [[p &parm->del_time]],

            typeOfUA[1]
                %{
                        parm->del_type = 0;
                %}
                IMPLICIT INTEGER [[i parm->del_type]] {
                    public (0),

                    private (1)
                } DEFAULT public
        }


NonDeliveredInfo [[P NonDelinfo *]] ::=
        SET {
            [0]
                IMPLICIT ReasonCode [[i parm->nd_rcode]],

            [1]
                IMPLICIT DiagnosticCode [[i parm->nd_dcode]] OPTIONAL
        }


-- No ReasonCodes specifed because 84 should be able to accept 88 codes
ReasonCode ::=
        INTEGER 


-- No DiagnosticCodes specifed because 84 should be able to accept 88 codes
DiagnosticCode  ::=
        INTEGER


-- supplementary information

SupplementaryInformation ::=
        PrintableString -- length limited and for further study


-- Probe MPDU

ProbeMPDU [[P Q_struct *]]::=
        ProbeEnvelope [[p parm]]

ProbeEnvelope [[P Q_struct *]] %{
        char *sp;
        ADDR *ad_link;
        int     n;
        %} ::=
        SET {
            probe
                MPDUIdentifier [[p &parm->msgid]],

            originator
                ORName [[p &sp]]
                %{
                        parm->Oaddress = adr_new (sp, AD_X400_TYPE, 0);
                %},

                ContentType [[i n]]
                    %{  if (n != 2) {
                        PP_OPER (NULLCP, ("Bad content type %d aborting", n));
                        exit (2);
                        }
                        parm->cont_type = strdup (cont_p2);
		    %},

                UAContentId [[s parm->ua_id]] OPTIONAL,

            original
                EncodedInformationTypes
                         [[p &parm->orig_encodedinfo]] OPTIONAL,

                TraceInformation [[p &parm->trace]],

                PerMessageFlag [[p parm]]
                DEFAULT {},

            contentLength[0]
                IMPLICIT INTEGER [[i parm->msgsize]] OPTIONAL,

            [1]
                IMPLICIT SEQUENCE OF PerDomainBilateralInfo
                OPTIONAL,

            [2]
                IMPLICIT SEQUENCE OF
                        RecipientInfo [[p &ad_link]]
                %{
                        register ADDR   **ap;
                        for (ap = &(parm->Raddress);
                             *ap; ap = &(*ap)->ad_next);
                                        *ap = ad_link;
                %}
}

END


%{
int     pe2bits (pe)
register PE     pe;
{
    int     i,
            j,
            k;

    j = 0;
    for (k = 1 << (i = 0); i < sizeof (j) * 8; i++, k <<= 1) {
        switch (bit_test (pe, i)) {
            case NOTOK:
                break;

            default:
                j |= k;         /* fall */
            case OK:
                continue;
        }

        break;
    }

    return (j);
}

char    *pe2str (pe)
PE      pe;
{
        int     len;

        return prim2str (pe, &len);
}


dump_struct ()
{
        FILE    *fp;

        if ((fp = fopen (PPSTRUCT, "a")) == NULL)
        {
                advise (NULLCP, "Can't open %s", PPSTRUCT);
                return;
        }
        if (rp_isbad (wr_q (PPQuePtr, fp)) )
                advise (NULLCP, "Can't write Qstruct");
        if (PPQuePtr->Oaddress != NULL)
                if (rp_isbad (wr_adr (PPQuePtr->Oaddress, fp, AD_ORIGINATOR)))
                        advise (NULLCP, "Can't write Oaddress");
        if (PPQuePtr->Raddress != NULL)
                if (rp_isbad (wr_adr (PPQuePtr->Raddress, fp, AD_RECIPIENT)))
                        advise (NULLCP, "Can't write Raddress");
        fputs ("-------------------------\n", fp);
        (void) fclose (fp);
}

%}
