-- P1.py - MHS P1 definitions

-- @(#) $Header: /xtel/pp/pp-beta/Tools/mpp84/RCS/P1.py,v 6.0 1991/12/18 20:31:34 jpo Rel $
--
-- $Log: P1.py,v $
-- Revision 6.0  1991/12/18  20:31:34  jpo
-- Release 6.0
--
--
--



P1 DEFINITIONS  ::=

%{
#ifndef lint
static char *rcsid = "$Header: /xtel/pp/pp-beta/Tools/mpp84/RCS/P1.py,v 6.0 1991/12/18 20:31:34 jpo Rel $";
#endif  lint

void    parse_p2 ();
%}

BEGIN

PRINTER print

-- P1 makes use of types defined in the following module:
-- Sa: Recommendation S.a[14]
-- T73: T.73, Section 5

MPDU ::=
        CHOICE {
            usermpdu
                [0] IMPLICIT UserMPDU,

            ServiceMPDU
        }

ServiceMPDU ::=
        CHOICE {
            reportmpdu [1]
                IMPLICIT DeliveryReportMPDU,

            probempdu [2]
                IMPLICIT ProbeMPDU
        }

UserMPDU ::=
        SEQUENCE {
            envelope
                UMPDUEnvelope,

            content
                UMPDUContent
        }

UMPDUEnvelope ::=
        SET {
            mpduID
                MPDUIdentifier,

            originator
                ORName,

            encodedtypes
                EncodedInformationTypes
                OPTIONAL,

            contentype
                ContentType,

            uacontentID
                UAContentId OPTIONAL,

            priority
                Priority DEFAULT normal,

            permsgflag
                PerMessageFlag DEFAULT {},

            deferredDelivery[0]
                IMPLICIT Time
                OPTIONAL,

            perdomain [1]
                IMPLICIT SEQUENCE OF PerDomainBilateralInfo
                OPTIONAL,

            recipient [2]
                IMPLICIT SEQUENCE OF RecipientInfo,

            trace
                TraceInformation
        }

UMPDUContent ::=
        OCTETSTRING
        %{ parse_p2 ($$, $$_len); %}


-- time

Time ::=
        UniversalTime


-- various envelope information

MPDUIdentifier ::=
        [APPLICATION 4] IMPLICIT SEQUENCE {
            globalid
                GlobalDomainIdentifier,
                IA5String
        }

ContentType ::=
        [APPLICATION 6]
                IMPLICIT INTEGER {
                        p2(2)
                }

UAContentId ::=
        [APPLICATION 10]
            IMPLICIT PrintableString

Priority ::=
        [APPLICATION 7]
                IMPLICIT INTEGER {
                        normal(0),
                        nonUrgent(1),
                        urgent(2)
                }

PerMessageFlag ::=
        [APPLICATION 8]
            IMPLICIT BITSTRING {
                discloseRecipients(0),
                conversionProhibited(1),
                alternateRecipientAllowed(2),
                contentReturnRequest(3)
            }

-- per-domain bilateral information

PerDomainBilateralInfo ::=
        SEQUENCE {
            country
                CountryName,
            admd
                AdministrationDomainName,
            bilateralinfo
                BilateralInfo
        }

BilateralInfo ::=
        ANY

-- recipient information

RecipientInfo ::=
        SET {
            recipname
                ORName,

            extensionid [0]
                IMPLICIT ExtensionIdentifier,

            perecipflag [1]
                IMPLICIT PerRecipientFlag,

            explicitconversion [2]
                IMPLICIT ExplicitConversion DEFAULT {}

        }

ExtensionIdentifier ::=
        INTEGER

PerRecipientFlag ::=
        BITSTRING -- See Figure 23/X.411

ExplicitConversion ::=
        INTEGER {
            iA5TextTeletex(0),
            teletexTelex(1)
        }


-- trace information

TraceInformation ::=
        [APPLICATION 9]
            IMPLICIT SEQUENCE OF SEQUENCE {
                domainid
                    GlobalDomainIdentifier,

                domainsupplied
                    DomainSuppliedInfo
            }

DomainSuppliedInfo ::=
        SET {
            arrival [0]
                IMPLICIT Time,

            deferred [1]
                IMPLICIT Time
                OPTIONAL,

            action [2]
                IMPLICIT INTEGER {
                    relayed(0),

                    rerouted(1)
                },

            encodedtypes
                EncodedInformationTypes
                OPTIONAL,

            previous
                GlobalDomainIdentifier OPTIONAL
        }


-- global domain identifier

GlobalDomainIdentifier ::=
        [APPLICATION 3]
            IMPLICIT SEQUENCE {
                country
                        CountryName,
                admd
                        AdministrationDomainName,
                prmd
                        PrivateDomainIdentifier OPTIONAL
            }

CountryName ::=
        [APPLICATION 1]
            CHOICE {
                NumericString,
                PrintableString
            }

AdministrationDomainName ::=
        [APPLICATION 2]
            CHOICE {
                NumericString,
                PrintableString
            }

PrivateDomainIdentifier ::=
        CHOICE {
            NumericString,
            PrintableString
        }


-- O/R name

ORName ::=
        [APPLICATION 0]
            IMPLICIT SEQUENCE {
                standard
                    StandardAttributeList,

                domaindefined
                    DomainDefinedAttributeList
                    OPTIONAL
            }

StandardAttributeList ::=
        SEQUENCE {
            country
                CountryName OPTIONAL,

            admd
                AdministrationDomainName OPTIONAL,

            x121 [0]
                IMPLICIT X121Address OPTIONAL,

            terminalID [1]
                IMPLICIT TerminalID OPTIONAL,

            prmd [2]
                PrivateDomainName OPTIONAL,

            org [3]
                IMPLICIT OrganizationName OPTIONAL,

            uaID [4]
                IMPLICIT UniqueUAIdentifier OPTIONAL,

            pname [5]
                IMPLICIT PersonalName
                OPTIONAL,

            orgUnit [6]
                IMPLICIT SEQUENCE OF OrganizationalUnit OPTIONAL
        }

DomainDefinedAttributeList ::=
        SEQUENCE OF DomainDefinedAttribute

DomainDefinedAttribute ::=
        SEQUENCE {
            type
                PrintableString,

            value
                PrintableString
        }

X121Address ::=
        NumericString

TerminalID ::=
        PrintableString

OrganizationName ::=
        PrintableString

UniqueUAIdentifier ::=
        NumericString

PersonalName ::=
        SET {
            surName [0]
                IMPLICIT PrintableString,

            givenName [1]
                IMPLICIT PrintableString
                OPTIONAL,

            initials [2]
                IMPLICIT PrintableString
                OPTIONAL,

            generalQualifier [3]
                IMPLICIT PrintableString
                OPTIONAL
    }

OrganizationalUnit ::=
        PrintableString

PrivateDomainName ::=
        CHOICE {
            NumericString,
            PrintableString
        }


-- encoded information types

EncodedInformationTypes ::=
        [APPLICATION 5] IMPLICIT SET {
            bitstring [0]
                IMPLICIT BITSTRING {
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
                OPTIONAL,

            [2]
                IMPLICIT TeletexNonBasicParams
                OPTIONAL,

            [3]
                IMPLICIT PresentationCapabilities
                OPTIONAL

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
        T73.PresentationCapabilities


-- Delivery Report MPDU

DeliveryReportMPDU ::=
        SEQUENCE {
            envelope
                DeliveryReportEnvelope,

            content
                DeliveryReportContent
        }

DeliveryReportEnvelope ::=
        SET {
            report
                MPDUIdentifier,

            originator
                ORName,

                TraceInformation
        }

DeliveryReportContent ::=
        SET {
            original
                MPDUIdentifier,

            intermediate
                TraceInformation OPTIONAL,

            uacontentID
                UAContentId OPTIONAL,

            reprecip [0]
                IMPLICIT SEQUENCE OF ReportedRecipientInfo,

            returned [1]
                IMPLICIT UMPDUContent OPTIONAL,

            billingInformation [2]
                ANY
                OPTIONAL
        }

ReportedRecipientInfo ::=
        SET {
            recipient[0]
                IMPLICIT ORName,

            extensionID [1]
                IMPLICIT ExtensionIdentifier,

            perecipflag [2]
                IMPLICIT PerRecipientFlag,

            lastrace [3]
                IMPLICIT LastTraceInformation,

            intendedRecipient[4]
                IMPLICIT ORName
                OPTIONAL,

            additional [5]
                IMPLICIT SupplementaryInformation OPTIONAL
        }


-- last trace information

LastTraceInformation ::=
        SET {
            arrival [0]
                IMPLICIT Time,

            encodedtypes
                EncodedInformationTypes
                OPTIONAL,

            report [1]
                Report
        }

Report ::=
        CHOICE {
            delivered [0]
                IMPLICIT DeliveredInfo,

            nondelivered [1]
                IMPLICIT NonDeliveredInfo
        }

DeliveredInfo ::=
        SET {
            delivery [0]
                IMPLICIT Time,

            typeOfUA [1]
                IMPLICIT INTEGER {
                    public(0),
                    private(1)
                } DEFAULT public
        }

NonDeliveredInfo::=
        SET {
            reason [0]
                IMPLICIT ReasonCode,

            diagnostic [1]
                IMPLICIT DiagnosticCode OPTIONAL
        }

ReasonCode ::=
        INTEGER {
            transferFailure(0),
            unableToTransfer(1),
            conversionNotPerformed(2)
        }

DiagnosticCode  ::=
        INTEGER {
            unrecognizedORName(0),

            ambiguousORName(1),

            mtaCongestion(2),

            loopDetected(3),

            uaUnavailable(4),

            maximumTimeExpired(5),

            encodedInformationTypesUnsupported(6),

            contentTooLong(7),

            conversionImpractical(8),

            conversionProhibited(9),

            implicitConversionNotResgistered(10),

            invalidParameters(11)
        }


-- supplementary information

SupplementaryInformation ::=
        PrintableString -- length limited and for further study


-- Probe MPDU

ProbeMPDU ::=
        ProbeEnvelope

ProbeEnvelope ::=
        SET {
            probeid
                MPDUIdentifier,

            originator
                ORName,

                ContentType,

                UAContentId OPTIONAL,

            encodedtypes
                EncodedInformationTypes
                OPTIONAL,

                TraceInformation,

                PerMessageFlag DEFAULT {},

            contentLength[0]
                IMPLICIT INTEGER
                OPTIONAL,

            [1]
                IMPLICIT SEQUENCE OF PerDomainBilateralInfo
                OPTIONAL,

            [2]
                IMPLICIT SEQUENCE OF RecipientInfo
}

END

%{

void    adios ();


void    parse_p2 (octstr, len)
char    *octstr;
int     len;
{
    PS      ps;
    PE      pe;

    if ((ps = ps_alloc (str_open)) == NULLPS)
        adios (NULLCP, "ps_alloc");
    if (str_setup (ps, octstr, len, 0) == NOTOK)
        adios (NULLCP, "str_setup (%s)", ps_error (ps -> ps_errno));

    if ((pe = ps2pe (ps)) == NULLPE)
        adios (NULLCP, "ps2pe (%s)", ps_error (ps -> ps_errno));

    (void) print_P2_UAPDU (pe, 1, NULLIP, NULLVP, NullParm);

    pe_free (pe);
    ps_free (ps);
}

%}
