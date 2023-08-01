-- authen.py:


-- @(#) $Header: /xtel/pp/pp-beta/Lib/x400/RCS/authen.py,v 6.0 1991/12/18 20:25:37 jpo Rel $
--
-- $Log: authen.py,v $
-- Revision 6.0  1991/12/18  20:25:37  jpo
-- Release 6.0
--
--
--

-- Authentication Framework for LOCATOR project.
-- Based on Geneva X.509 Authentication Framework
-- Written by M.Roe, University College London.

-- Version 1 April    1988
-- Version 2 August   1988 (for ISODE-4.0)
-- Version 3 November 1988 (Copenhagen IS)
-- Version 4 June     1989 (Minor change to revocation list)



Auth
-- {iso identified-organization(3) locator(999) modules(0) authentication(1)}
DEFINITIONS ::=

BEGIN
-- IMPORTS
-- Change
--        Name
--                FROM IF
--                        {
--                        joint-iso-ccitt
--                        ds(5)
--                        modules(1)
--                        informationFramework(1)
--                        };

-- Declare forward references to other modules as ANY

Name ::=
    ANY

External ::=
    ANY

-- Use this type to represent the ENCRYPTED macro.
-- Should be BIT STRING, but use OCTET STRING for the moment.

Encrypted ::=
   BIT STRING

-- Use this type to represent the SIGNED macro.

SignedType ::=
   SEQUENCE {
    tobesigned External,
    algorithm AlgorithmIdentifier,
    signature Encrypted}

-- Use this type to represent the SIGNATURE macro.

Signature ::=
   SEQUENCE {
    algorithm AlgorithmIdentifier,
    signature Encrypted}

-- Use this type to represent the PROTECTED macro.

Protected ::=
    Signature

-- These types are created by the SIGNED macro.

CertificateList ::=
    SignedType

Certificate ::=
    SignedType

HotList ::=
    SignedType

-- This is signed to make CertificateList

ListBody ::=
   SEQUENCE {
     signature 			AlgorithmIdentifier,
     issuer    			Name,
     lastUpdate 		UTCTime,
     revokedCertificates	HotList OPTIONAL}

-- This is signed to make Certificate

CertificateBody ::=
    SEQUENCE {
      version 			[0] Version DEFAULT 0,
      serialNumber		SerialNumber,
      signature			AlgorithmIdentifier,
      issuer			Name,
      validity  		Validity,
      subject			Name,
      subjectPublicKeyInfo	SubjectPublicKeyInfo}

-- This is signed to revoke a certificate

RevokedCertficate ::=
    SEQUENCE {
      signature			AlgorithmIdentifier,
      issuer			Name,
      serialNumber              SerialNumber,
      revocationDate		UTCTime}

Version ::= 
    INTEGER

SerialNumber ::=
    INTEGER

Validity ::=
    SEQUENCE {
	notBefore	UTCTime,
	notAfter	UTCTime}

SubjectPublicKeyInfo ::=
     SEQUENCE {
	algorithm		AlgorithmIdentifier,
	subjectPublicKey	EncryptionKey}

EncryptionKey ::=
     BIT STRING

AlgorithmIdentifier ::=
     SEQUENCE {
	algorithm	OBJECT IDENTIFIER,
 	parameters	INTEGER} 

-- These are the possible parameters

KeySize ::= 
    INTEGER

BlockSize ::=
    INTEGER

KeyAndBlockSize ::=
    INTEGER


-- Attribute Syntaxes

-- CertificateSyntax ::= ATTRIBUTE-SYNTAX
--	Certificate
--	MULTI-VALUED
--	MATCHES FOR EQUALITY
--
-- CertificateListSyntax ::= ATTRIBUTE-SYNTAX
--	CertificateList
--	SINGLE-VALUED
--	MATCHES FOR EQUALITY
--
-- PasswordSyntax ::= ATTRIBUTE-SYNTAX
--	Password
--	SINGLE-VALUED
--	MATCHES FOR EQUALITY
--
-- UserCertificate ::= ATTRIBUTE 
--	WITH ATTRIBUTE-SYNTAX CertificateSyntax
--
-- CACertificate ::= ATTRIBUTE
--	WITH ATTRIBUTE-SYNTAX CertificateSyntax
--
-- CertificateRevocationList ::= ATTRIBUTE
--	WITH ATTRIBUTE-SYNTAX CertificateListSyntax
--
-- AuthorityRevocationList ::= ATTRIBUTE
--	WITH ATTRIBUTE-SYNTAX CertificateListSyntax
--
-- UserPassword ::= ATTRIBUTE
--	WITh ATTRIBUTE-SYNTAX PasswordSyntax
--
-- userPassword UserPassword ::= 
--	{attributeType 35}
--
-- userCertificate UserCertificate ::= 
--	{attributeType 36}
--
-- caCertificate CACertificate ::= 
--	{attributeType 37}
--
-- authorityRevocationList AuthorityRevocationList ::=
--	{attributeType 38}
--
-- certificateRevocationList CertificateRevocationList ::=
--	{attributeType 39}
--

END
