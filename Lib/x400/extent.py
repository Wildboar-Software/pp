-- extent.py:


-- @(#) $Header: /xtel/pp/pp-beta/Lib/x400/RCS/extent.py,v 6.0 1991/12/18 20:25:37 jpo Rel $
--
-- $Log: extent.py,v $
-- Revision 6.0  1991/12/18  20:25:37  jpo
-- Release 6.0
--
--
--


Ext DEFINITIONS IMPLICIT TAGS ::=

BEGIN

IMPORTS
	ActualRecipientName,
	NonDeliveryReasonCode,
	NonDeliveryDiagnosticCode,
	ORAddress,
	ORAddressAndOrDirectoryName,
	ORAddressAndOptionalDirectoryName,
	MessageDeliveryTime,
	TypeOfMTSUser,
	OriginallyIntendedRecipientName,
	RecipientCertificate,
	GlobalDomainIdentifier,
	RoutingAction,
	ArrivalTime,
	DeferredTime,
	OtherActions,
	MTAName
		FROM MTA
	ProofOfDelivery,
	Certificates,
	SecurityLabel,
	Token
		FROM Toks
	AlgorithmIdentifier,
	Signature
		FROM Auth;

RecipientReassignmentProhibited ::= ENUMERATED {
	recipient-reassignment-allowed (0),
	recipient-reassignment-prohibited (1) }

OriginatorRequestedAlternateRecipient ::= ORAddressAndOrDirectoryName

DLExpansionProhibited ::= ENUMERATED {
	dl-expansion-allowed (0),
	dl-expansion-prohibited (1) }

ConversionWithLossProhibited ::= ENUMERATED {
	conversion-with-loss-allowed (0),
	conversion-with-loss-prohibited (1) }

LatestDeliveryTime ::= Time

RequestedDeliveryMethod ::= SEQUENCE OF INTEGER {	-- each different in order of preference, most preferred first
	any-delivery-method (0),
	mhs-delivery (1),
	physical-delivery (2),
	telex-delivery (3),
	teletex-delivery (4),
	g3-facsimile-delivery (5),
	g4-facsimile-delivery (6),
	ia5-terminal-delivery (7),
	videotex-delivery (8),
	telephone-delivery (9) } (0..ub-integer-options)

PhysicalForwardingProhibited ::= ENUMERATED {
	physical-forwarding-allowed (0),
	physical-forwarding-prohibited (1) }

PhysicalForwardingAddressRequest ::= ENUMERATED {
	physical-forwarding-address-not-requested (0),
	physical-forwarding-address-requested (1) }

PhysicalDeliveryModes ::= BIT STRING {
	ordinary-mail (0),
	special-delivery (1),
	express-mail (2),
	counter-collection (3),
	counter-collection-with-telephone-advice (4),
	counter-collection-with-telex-advice (5),
	counter-collection-with-teletex-advice (6),
	bureau-fax-delivery (7)
	-- bits 0 to 6 are mutually exclusive
	-- bit 7 can be set with any of bits 0 to 6 -- } (SIZE (0..ub-bit-options)) 

RegisteredMailType ::= INTEGER {
	non-registered-mail (0),
	registered-mail (1),
	registered-mail-to-addressee-in-person (2) } (0..ub-integer-options)

RecipientNumberForAdvice ::= TeletexString (SIZE (1..ub-recipient-number-for-advice-length))

PhysicalRenditionAttributes ::= OBJECT IDENTIFIER

OriginatorReturnAddress ::= ORAddress

PhysicalDeliveryReportRequest ::= INTEGER {
	return-of-undeliverable-mail-by-PDS (0),
	return-of-notification-by-PDS (1),
	return-of-notification-by-MHS (2),
	return-of-notification-by-MHS-and-PDS (3) } (0..ub-integer-options)

OriginatorCertificate ::= Certificates

MessageToken ::= Token

ContentConfidentialityAlgorithmIdentifier ::= AlgorithmIdentifier

ContentIntegrityCheck ::= Signature
--	SIGNATURE SEQUENCE {
--	algorithm-identifier ContentIntegrityAlgorithmIdentifier,
--	content Content }

ContentIntegrityAlgorithmIdentifier ::= AlgorithmIdentifier


MessageOriginAuthenticationCheck ::= Signature
-- SIGNATURE SEQUENCE {
--	algorithm-identifier MessageOriginAuthenticationAlgorithmIdentifier,
--	content Content,
--	content-identifier ContentIdentifier OPTIONAL,
--	message-security-label MessageSecurityLabel OPTIONAL }

MessageOriginAuthenticationAlgorithmIdentifier ::= AlgorithmIdentifier

MessageSecurityLabel ::= SecurityLabel

ProofOfSubmissionRequest ::= ENUMERATED {
	proof-of-submission-not-requested (0),
	proof-of-submission-requested (1) }

SequenceNumber ::= INTEGER

ProofOfDeliveryRequest ::= ENUMERATED {
	proof-of-delivery-not-requested (0),
	proof-of-delivery-requested (1) }

ContentCorrelator ::= ANY	-- maximum ub-content-correlator-length octets including all encoding

ProbeOriginAuthenticationCheck ::= Signature
-- SIGNATURE SEQUENCE {
--	algorithm-identifier ProbeOriginAuthenticationAlgorithmIdentifier,
--	content-identifier ContentIdentifier OPTIONAL,
--	message-security-label MessageSecurityLabel OPTIONAL }

ProbeOriginAuthenticationAlgorithmIdentifier ::= AlgorithmIdentifier

RedirectionHistory ::= SEQUENCE SIZE (1..ub-redirections) OF Redirection

Redirection ::= SEQUENCE {
	intended-recipient-name IntendedRecipientName,
	redirection-reason RedirectionReason }

IntendedRecipientName ::= SEQUENCE {
	address	ORAddressAndOptionalDirectoryName,
	redirection-time Time }

RedirectionReason ::= ENUMERATED {
	recipient-assigned-alternate-recipient (0),
	originator-requested-alternate-recipient (1),
	recipient-MD-assigned-alternate-recipient (2) }

DLExpansionHistory ::= SEQUENCE SIZE (1..ub-dl-expansions) OF DLExpansion

DLExpansion ::= SEQUENCE {
	address	ORAddressAndOptionalDirectoryName,
	dl-expansion-time Time }

PhysicalForwardingAddress ::= ORAddressAndOptionalDirectoryName


OriginatorAndDLExpansionHistory ::= SEQUENCE SIZE (0..ub-dl-expansions) OF OriginatorAndDLExpansion

OriginatorAndDLExpansion ::= SEQUENCE {
	originator-or-dl-name ORAddressAndOptionalDirectoryName,
	origination-or-expansion-time Time }

ReportingDLName ::= ORAddressAndOptionalDirectoryName

ReportingMTACertificate ::= Certificates

ReportOriginAuthenticationCheck ::= Signature
-- SIGNATURE SEQUENCE {
--	algorithm-identifier ReportOriginAuthenticationAlgorithmIdentifier,
--	content-identifier ContentIdentifier OPTIONAL,
--	message-security-label MessageSecurityLabel OPTIONAL,
--	per-recipient SEQUENCE SIZE (1..ub-recipients) OF PerRecipientReportFields }

ReportOriginAuthenticationAlgorithmIdentifier ::= AlgorithmIdentifier

PerRecipientReportFields ::= SEQUENCE {
	actual-recipient-name ActualRecipientName,
	originally-intended-recipient-name OriginallyIntendedRecipientName OPTIONAL,
	report CHOICE {
		delivery [0] PerRecipientDeliveryReportFields,
		non-delivery [1] PerRecipientNonDeliveryReportFields } }

PerRecipientDeliveryReportFields ::= SEQUENCE {
	message-delivery-time MessageDeliveryTime,
	type-of-MTS-user TypeOfMTSUser,
	recipient-certificate [0] RecipientCertificate OPTIONAL,
	proof-of-delivery [1] ProofOfDelivery OPTIONAL }

PerRecipientNonDeliveryReportFields ::= SEQUENCE {
	non-delivery-reason-code NonDeliveryReasonCode,
	non-delivery-diagnostic-code NonDeliveryDiagnosticCode OPTIONAL }

OriginatingMTACertificate ::= Certificates

ProofOfSubmission ::= Signature
-- SIGNATURE SEQUENCE {
--	algorithm-identifier ProofOfSubmissionAlgorithmIdentifier,
--	message-submission-envelope MessageSubmissionEnvelope,
--	message-submission-identifier MessageSubmissionIdentifier,
--	message-submission-time MessageSubmissionTime }

ProofOfSubmissionAlgorithmIdentifier ::= AlgorithmIdentifier

InternalTraceInformation ::= SEQUENCE OF InternalTraceInformationElement

InternalTraceInformationElement ::= SEQUENCE {
        global-domain-identifier GlobalDomainIdentifier,
        mta-name MTAName,
        mta-supplied-information MTASuppliedInformation }

MTASuppliedInformation ::= SET {
        arrival-time [0] ArrivalTime,
        routing-action [2] RoutingAction,
        attempted CHOICE {
                mta MTAName,
                domain GlobalDomainIdentifier } OPTIONAL,
	deferred-time [1] DeferredTime OPTIONAL,
	other-actions[3] BIT STRING DEFAULT {} }
--	other-actions[3] OtherActions DEFAULT {} }


--	Extension Attributes

CommonName ::= PrintableString (SIZE (1..ub-common-name-length))

TeletexCommonName ::= TeletexString (SIZE (1..ub-common-name-length))

TeletexOrganizationName ::= TeletexString (SIZE (1..ub-organization-name-length))

TeletexPersonalName ::= SET {
	surname [0] TeletexString (SIZE (1..ub-surname-length)),
	given-name [1] TeletexString (SIZE (1..ub-given-name-length)) OPTIONAL,
	initials [2] TeletexString (SIZE (1..ub-initials-length)) OPTIONAL,
	generation-qualifier [3] TeletexString (SIZE (1..ub-generation-qualifier-length)) OPTIONAL }

TeletexOrganizationalUnitNames ::= SEQUENCE SIZE (1..ub-organizational-units) OF 
	TeletexOrganizationalUnitName 

TeletexOrganizationalUnitName ::= TeletexString (SIZE (1..ub-organizational-unit-name-length))

TeletexDomainDefinedAttributes ::= SEQUENCE SIZE (1..ub-domain-defined-attributes) OF 
	TeletexDomainDefinedAttribute 

TeletexDomainDefinedAttribute ::= SEQUENCE {
	type TeletexString (SIZE (1..ub-domain-defined-attribute-type-length)),
	value TeletexString (SIZE (1..ub-domain-defined-attribute-value-length)) }

PDSName ::= PrintableString (SIZE (1..ub-pds-name-length))

PhysicalDeliveryCountryName ::= CHOICE {
	x121-dcc-code NumericString (SIZE (ub-country-name-numeric-length)),
	iso-3166-alpha2-code PrintableString (SIZE (ub-country-name-alpha-length)) }

PostalCode ::= CHOICE {
	numeric-code NumericString (SIZE (1..ub-postal-code-length)),
	printable-code PrintableString (SIZE (1..ub-postal-code-length)) }

PhysicalDeliveryOfficeName ::= PDSParameter

PhysicalDeliveryOfficeNumber ::= PDSParameter

ExtensionORAddressComponents ::= PDSParameter

PhysicalDeliveryPersonalName ::= PDSParameter

PhysicalDeliveryOrganizationName ::= PDSParameter

ExtensionPhysicalDeliveryAddressComponents ::= PDSParameter

UnformattedPostalAddress ::= SET {
	printable-address SEQUENCE SIZE (1..ub-physical-address-lines) OF
		PrintableString (SIZE (1..ub-pds-parameter-length)) OPTIONAL,
	teletex-string TeletexString (SIZE (1..ub-unformatted-address-length)) OPTIONAL }

StreetAddress ::= PDSParameter

PostOfficeBoxAddress ::= PDSParameter

PosteRestanteAddress ::= PDSParameter

UniquePostalName ::= PDSParameter

LocalPostalAttributes ::= PDSParameter

PDSParameter ::= SET {
	printable-string PrintableString (SIZE(1..ub-pds-parameter-length)) OPTIONAL,
	teletex-string TeletexString (SIZE(1..ub-pds-parameter-length)) OPTIONAL }

ExtendedNetworkAddress ::= CHOICE {
	e163-4-address SEQUENCE {
		number [0] NumericString (SIZE (1..ub-e163-4-number-length)),
		sub-address [1] NumericString (SIZE (1..ub-e163-4-sub-address-length)) OPTIONAL }
	, psap-address PresentationAddress }
-- quick hack
PresentationAddress ::= ANY

TerminalType ::= INTEGER {
	telex (3),
	teletex (4),
	g3-facsimile (5),
	g4-facsimile (6),
	ia5-terminal (7),
	videotex (8) } (0..ub-integer-options)

Time ::= UTCTime

END
