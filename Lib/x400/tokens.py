-- tokens.py: 


-- @(#) $Header: /xtel/pp/pp-beta/Lib/x400/RCS/tokens.py,v 6.0 1991/12/18 20:25:37 jpo Rel $
--
-- $Log: tokens.py,v $
-- Revision 6.0  1991/12/18  20:25:37  jpo
-- Release 6.0
--
--
--

-- Security related definitions from X.400
-- Version 1 December 1988
-- Author: M.Roe, University College London


Toks 
-- {iso identified-organisation(3) locator(99) modules(0) tokens(2)}
DEFINITIONS IMPLICIT TAGS ::=
BEGIN

IMPORTS 
   Encrypted, SignedType, Signature, Protected, AlgorithmIdentifier, 
   Certificate, EncryptionKey
   FROM Auth;


--	Security Label

SecurityContext ::= SET SIZE (1..ub-security-labels) OF SecurityLabel

SecurityLabel ::= SET {
	security-policy-identifier SecurityPolicyIdentifier OPTIONAL,
	security-classification SecurityClassification OPTIONAL,
	privacy-mark PrivacyMark OPTIONAL,
	security-categories SecurityCategories OPTIONAL }

SecurityPolicyIdentifier ::= OBJECT IDENTIFIER

SecurityClassification ::= INTEGER {
	unmarked (0),
	unclassified (1),
	restricted (2),
	confidential (3),
	secret (4),
	top-secret (5) } (0..ub-integer-options)

PrivacyMark ::= PrintableString (SIZE (1..ub-privacy-mark-length))

SecurityCategories ::= SET SIZE (1..ub-security-categories) OF SecurityCategory

SecurityCategory ::= SEQUENCE {
	type [0] OBJECT IDENTIFIER,
	value [1] ANY}

-- Proof of Delivery

ProofOfDelivery ::=
   SET {
      certificates [0] Certificates OPTIONAL,
      signature    [1] Signature OPTIONAL}

ProofOfDeliveryBody ::=
   SEQUENCE {
       algorithm AlgorithmIdentifier,
       time UTCTime,
       recipient  ANY,
       content OCTET STRING}
       
-- We don't handle certification paths at the moment
Certificates ::= 
    SEQUENCE {
        certificate Certificate}

-- Tokens

Token ::=
   SEQUENCE {
      type  OBJECT IDENTIFIER,
      token [1] ANY}

TokenData ::=
    SEQUENCE {
       type  [0] INTEGER,
       value [1] ANY}

AsymmetricToken ::= SignedType

AsymmetricTokenBody ::=
   SEQUENCE {
      signature-algorithm-identifier	AlgorithmIdentifier,
      time				Nonce,
      signed-data			[0] TokenData OPTIONAL,
      encryption-algorithm-identifier	[1] AlgorithmIdentifier OPTIONAL,
      encrypted-data			[2] Encrypted OPTIONAL}

SymmetricToken ::= 
   SEQUENCE {
      algorithm		AlgorithmIdentifier,
      encrypted-data	Encrypted}

SymmetricTokenBody ::=
   SEQUENCE {
      time		Nonce,
      data		TokenData}
      
Nonce ::=
   CHOICE {
      time	Time,
      random	RandomNumber}
      
RandomNumber ::=
   BIT STRING

MessageTokenSignedData ::=
   SEQUENCE {
      content-confidentiality-algorithm [0] AlgorithmIdentifier	OPTIONAL,
      content-integrity-check		[1] Signature		OPTIONAL,
      message-sequence-number		[4] INTEGER		DEFAULT 0}

MessageTokenEncryptedData ::=
   SEQUENCE {
      content-confidentiality-key	[0] EncryptionKey	OPTIONAL,
      content-integrity-check		[1] Signature		OPTIONAL,
      content-integrity-key		[3] EncryptionKey	OPTIONAL,
      message-sequence-number		[4] INTEGER		DEFAULT 0}

Time ::= UTCTime

END

