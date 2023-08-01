-- iob.py - MHS P22 definitions
-- starting p602 in blue book x400(88)

-- @(#) $Header: /xtel/pp/pp-beta/Lib/x400/RCS/iob.py,v 6.0 1991/12/18 20:25:37 jpo Rel $
--
-- $Log: iob.py,v $
-- Revision 6.0  1991/12/18  20:25:37  jpo
-- Release 6.0
--
--

IOB 
--	{
--	joint-iso-ccitt
--	mhs-motis(6)
--	ipms(1)
--	modules(0)
--	information-objects(2)
--	}
DEFINITIONS IMPLICIT TAGS ::=

BEGIN
IMPORTS
	ub-auto-forward-comment, ub-free-form-name,
	ub-ipm-identifier-suffix, ub-local-imp-identifier,
	ub-subject-field, ub-telephone-number
		FROM UB
		     {
		     joint-iso-ccitt
		     mhs-motis(6)
		     ipms(1)
		     modules(0)
		     upper-bounds(10)
		     }

	EncodedInformationTypes, G3FacsimileNonBasicParameters,
	MessageDeliveryTime, ORAddress, 
--	ORName but not right yet
	StandardAttributes, DomainDefinedAttributes, ExtensionAttributes,
	OtherMessageDeliveryFields, SupplementaryInformation,
	TeletexNonBasicParameters
		FROM MTA
		{
		joint-iso-ccitt
		mhs-motis(6)
		mts(3)
		modules(0)
		mts-abstract-service(1)
		};
--		Change
--        Name
--                FROM IF
--                        {
--                        joint-iso-ccitt
--                        ds(5)
--                        modules(1)
--                        informationFramework(1)
--                        };

Time ::= UTCTime

-- Information object

InformationObject ::= CHOICE {
		  ipm [0] IPM,
		  ipn [1] IPN
		  }
-- IPM

IPM ::= SEQUENCE {
    heading	 			Heading,
    body				Body
    }

-- Heading

Heading ::= SET {
	this-IPM			ThisIPMField,
	originator		[0]	OriginatorField OPTIONAL,
	authorizing-users	[1]	AuthorizingUsersField OPTIONAL,
	primary-recipients	[2]	PrimaryRecipientsField DEFAULT {},
	copy-recipients		[3]	CopyRecipientsField OPTIONAL,
	blind-copy-recipients	[4]	BlindCopyRecipientsField OPTIONAL,
	replied-to-IPM		[5]	RepliedToIPMField OPTIONAL,
	obsoleted-IPMs		[6]	ObsoletedIPMsField DEFAULT {},
	related-IPMs		[7]	RelatedIPMsField DEFAULT {},
	subject			[8]	EXPLICIT SubjectField OPTIONAL,
	expiry-time		[9]	ExpiryTimeField OPTIONAL,
	reply-time		[10]	ReplyTimeField OPTIONAL,
	reply-recipients	[11]	ReplyRecipientsField OPTIONAL,
	importance		[12]	ImportanceField DEFAULT normal,
	sensitivity		[13]	SensitivityField OPTIONAL,
	auto-forwarded		[14]	AutoForwardedField DEFAULT FALSE,
	extensions		[15]	ExtensionsField DEFAULT {}
	}

-- Heading components types

IPMIdentifierSequence ::= SEQUENCE OF IPMIdentifier

IPMIdentifier ::= [APPLICATION 11] SET {
	user			       ORName OPTIONAL,
	user-relative-identifier       LocalIPMIdentifier
	}

LocalIPMIdentifier ::= PrintableString
        (SIZE (0..ub-local-ipm-identifier))

RecipientSequence ::= SEQUENCE OF RecipientSpecifier

RecipientSpecifier ::= SET {
	recipient	   	[0]	ORDescriptor,
	notification-requests	[1]	NotificationRequests DEFAULT {},
	reply-requested		[2]	BOOLEAN DEFAULT FALSE
	}

NotificationRequests ::= BIT STRING {
	rn(0),
	nrn(1),
	ipm-return(2)
	}

ORDescriptorSequence ::= SEQUENCE OF ORDescriptor

ORDescriptor ::= SET {
	formal-name  			ORName OPTIONAL,
	free-form-name		[0]	FreeFormName OPTIONAL,
	telephone-number	[1]	TelephoneNumber OPTIONAL
	}

FreeFormName ::= TeletexString (SIZE (0..ub-free-form-name))

TelephoneNumber ::= PrintableString (SIZE (0..ub-telephone-number))

-- This IPM heading field

ThisIPMField ::= IPMIdentifier

-- Originating heading field

OriginatorField ::= ORDescriptor

--Authorizing Users heading field

--AuthorizingUsersField ::= SEQUENCE OF AuthorizingUsersSubfield
	     
--AuthorizingUsersSubfield ::= ORDescriptor

-- To simplify coding

AuthorizingUsersField ::= ORDescriptorSequence

-- Primary Recipients heading field

--PrimaryRecipientsField ::= SEQUENCE OF PrimaryRecipientsSubfield

--PrimaryRecipientsSubfield ::= RecipientSpecifier

-- To simplify coding

PrimaryRecipientsField ::= RecipientSequence

-- Copy Recipients heading field

--CopyRecipientsField ::= SEQUENCE OF CopyRecipientsSubfield

--CopyRecipientsSubfield ::= RecipientSpecifier

-- To simplify coding

CopyRecipientsField ::= RecipientSequence

-- Blind Copy Recipients heading field

--BlindCopyRecipientsField ::= SEQUENCE OF BlindCopyRecipientsSubfield

--BlindCopyRecipientsSubfield ::= RecipientSpecifier

BlindCopyRecipientsField ::= RecipientSequence

-- Replied-to IPM heading field

RepliedToIPMField ::= IPMIdentifier

-- Obsoleted IPMs heading field

--ObsoletedIPMsField ::= SEQUENCE OF ObsoletedIPMsSubfield

--ObsoletedIPMsSubfield ::= IPMIdentifier

-- To simplify coding

ObsoletedIPMsField ::= IPMIdentifierSequence

-- Related IPMs heading field

--RelatedIPMsField ::= SEQUENCE OF RelatedIPMsSubfield

--RelatedIPMsSubfield ::= IPMIdentifier

-- To simplify coding

RelatedIPMsField ::= IPMIdentifierSequence

-- Subject heading field

SubjectField ::= TeletexString (SIZE (0..ub-subject-field))

-- Expiry Time heading field

ExpiryTimeField ::= Time

-- Reply Time heading field

ReplyTimeField ::= Time

-- Reply Recipients heading field

--ReplyRecipientsField ::= SEQUENCE OF ReplyRecipientsSubfield

--ReplyRecipientsSubfield ::= ORDescriptor

-- To simplify coding

ReplyRecipientsField ::= ORDescriptorSequence

-- Importance heading field

ImportanceField ::= ENUMERATED {
	low		       		(0),
	normal				(1),
	high				(2)
	}

-- Sensitivity heading field

SensitivityField ::= ENUMERATED {
	personal	       		(1),
	private				(2),
	company-confidential		(3)
	}
	
-- Auto-forwarded heading field

AutoForwardedField ::= BOOLEAN

-- Extensions heading field

ExtensionsField ::= SET OF HeadingExtension

HeadingExtension ::= SEQUENCE {
	type		      		OBJECT IDENTIFIER,
	value				ANY 
}

-- Incomplete Copy

IncompleteCopy ::= NULL

Languages ::= SET OF Language

Language ::= PrintableString (SIZE (2..2))

RFC822FieldList ::= SEQUENCE OF RFC822Field

RFC822Field ::= IA5String

-- Body

Body ::=
        SEQUENCE OF BodyPart

BodyPart ::=

        CHOICE {
            ia5-text		[0] IA5TextBodyPart,

            voice		[2] VoiceBodyPart,

            g3-facsimile	[3] G3FacsimileBodyPart,

            g4-class1		[4] G4Class1BodyPart,

            teletex		[5] TeletexBodyPart,

            videotex		[6] VideotexBodyPart,

            encrypted		[8] EncryptedBodyPart,

            message		[9] MessageBodyPart,

            mixed-mode		[11] MixedModeBodyPart,

	    bilaterally-defined	[14] BilaterallyDefinedBodyPart,

            nationally-defined	[7] NationallyDefinedBodyPart,

	    externally-defined	[15] ExternallyDefinedBodyPart,

	    -- not in x400 88 ?

            tlx			[1] TLXBodyPart,

            sfd			[10] SFDBodyPart,

            odif		[12] ODIFBodyPart,

	    iso6937Text		[13] ISO6937TextBodyPart
        }


-- body part types

IA5TextBodyPart ::= SEQUENCE {
		parameters   IA5TextParameters,
		data	     IA5TextData
		}

IA5TextParameters ::= SET {
	      repertoire [0] Repertoire DEFAULT ia5
	      }

IA5TextData ::= IA5String

Repertoire ::= ENUMERATED {
	   ita2 (2),
	   ia5	(5)
	   }

TLXBodyPart ::= ANY

-- Voice body part

VoiceBodyPart ::= SEQUENCE {
	      parameters   VoiceParameters,
	      data	   VoiceData
	      }

VoiceParameters ::= SET {}  -- for further study

VoiceData ::= BIT STRING -- for further study

-- G3 Facsimile body part

G3FacsimileBodyPart ::= SEQUENCE {
      parameters    G3FacsimileParameters,
      data	    G3FacsimileData
      }

G3FacsimileParameters ::= SET {
      number-of-pages	      [0] INTEGER	OPTIONAL,
      non-basic-parameters    [1] G3FacsimileNonBasicParameters OPTIONAL
      }

G3FacsimileData ::= SEQUENCE OF BIT STRING

-- G4 class 1 and mixed-mode body parts

--G4Class1BodyPart ::= SEQUENCE OF ProtocolElement

G4Class1BodyPart ::= ANY

-- MixedModeBodyPart ::= SEQUENCE OF ProtocolElement

MixedModeBodyPart ::= ANY

-- Teletex body part

TeletexBodyPart ::= SEQUENCE {
		parameters   TeletexParameters,
		data	     TeletexData
		}

TeletexParameters ::= SET {
		  number-of-pages	[0] INTEGER OPTIONAL,
		  telex-compatible	[1] BOOLEAN DEFAULT FALSE,
		  non-basic-parameters	[2] TeletexNonBasicParameters OPTIONAL
		  }

TeletexData ::= SEQUENCE OF TeletexString

-- Videotex body part

VideotexBodyPart ::= SEQUENCE {
		parameters   VideotexParameters,
		data	     VideotexData
		}

VideotexParameters ::= SET {
		  syntax   [0] VideotexSyntax OPTIONAL
		  }
VideotexSyntax ::= INTEGER {
	       ids 		(0),
	       data-syntax1	(1),
	       data-syntax2	(2),
	       data-syntax3	(3)}

VideotexData ::= SEQUENCE OF VideotexString

-- Encrypted body part

EncryptedBodyPart ::= SEQUENCE {
		  parameters   EncryptedParameters,
		  data	       EncryptedData
		  }

EncryptedParameters ::= SET {}-- for further study

EncryptedData ::= BIT STRING -- for further study

-- Message body part

MessageBodyPart ::= SEQUENCE {
		parameters	MessageParameters,
		data		MessageData
		}

MessageParameters ::= SET {
		  delivery-time		[0] MessageDeliveryTime OPTIONAL,
		  delivery-envelope	[1] OtherMessageDeliveryFields OPTIONAL
		  }

MessageData ::= IPM

SFDBodyPart ::= ANY

ODIFBodyPart ::=
        -- from appendix A minutes of the PODA Munich meeting July 6th 1988.
        OCTET STRING


-- motis-86-6937 (extra body part) 
ISO6937TextBodyPart ::= SEQUENCE {
	parameters      ISO6937Parameters,
	data            ISO6937Data}

ISO6937Parameters ::= SET {
		repertoire [0] ISO6937Repertoire DEFAULT part1and2}

ISO6937Repertoire ::= ENUMERATED {
		--
		-- values are assigned by the ISO registration
		-- authority. Other values are for further study,
		-- and shall be relayed.
		--
		part1and2 (0),
		teletexSubRepertoire (3)}

ISO6937Data ::= SEQUENCE OF ISO6937Line
		-- sequence may contain zero elements 

ISO6937Line ::= [0] IMPLICIT OCTET STRING
		-- additional protocol element to those defined by CCITT


BilaterallyDefinedBodyPart ::= OCTET STRING

NationallyDefinedBodyPart ::= ANY

-- Externally defined body part

--ExternallyDefinedBodyPart ::= SEQUENCE {
--			  parameters   [0] ExternallyDefinedParameters OPTIONAL
--			  data		   ExternallyDefinedData
--			  }

--ExternallyDefinedParameters ::= EXTERNAL

--ExternallyDefinedData ::= EXTERNAL

ExternallyDefinedBodyPart ::= ANY

-- IPN

IPN ::= SET {
	-- common-fields -- COMPONENTS OF CommonFields,

	choice [0] EXPLICIT CHOICE {
	       non-receipt-fields	[0]	NonReceiptFields,
	       receipt-fields		[1]	ReceiptFields
	       }
	}
RN ::= IPN -- with receipt-fields chosen

NRN ::= IPN -- with non-receipt-fields chosen

CommonFields ::= SET {
	subject-ipm			SubjectIPMField,
	ipn-originator		[1]	IPNOriginatorField OPTIONAL,
	ipn-preferred-recipient	[2]	IPNPreferredRecipientField OPTIONAL,
	conversion-eits			ConversionEITsField OPTIONAL
	}

NonReceiptFields ::= SET {
	non-receipt-reason	[0]	NonReceiptReasonField,
	discard-reason		[1]	DiscardReasonField OPTIONAL,
	auto-forward-comment	[2]	AutoForwardCommentField OPTIONAL,
	returned-ipm		[3]	ReturnedIPMField OPTIONAL
	}

ReceiptFields ::= SET {
	receipt-time  		[0] 	ReceiptTimeField,
	acknowledgment-mode	[1]	AcknowledgmentModeField DEFAULT manual,
	suppl-receipt-info	[2] 	SupplReceiptInfoField DEFAULT ""
	}

-- Common fields

SubjectIPMField ::= IPMIdentifier

IPNOriginatorField ::= ORDescriptor

IPNPreferredRecipientField ::= ORDescriptor

ConversionEITsField ::= EncodedInformationTypes

-- Non-receipt fields

NonReceiptReasonField ::= ENUMERATED {
	ipm-discarded		     	(0),
	ipm-auto-forward		(1)
	}

DiscardReasonField ::= ENUMERATED {
	ipm-expired		  	(0),
	ipm-obsoleted			(1),
	user-subscription-terminated	(2)
	}

AutoForwardCommentField ::= AutoForwardComment

AutoForwardComment ::= PrintableString
	(SIZE (0..ub-auto-forward-comment))

ReturnedIPMField ::= IPM

-- Receipt fields

ReceiptTimeField ::= Time

AcknowledgmentModeField ::= ENUMERATED {
	manual			       (0),
	automatic		       (1)
	}

-- SupplReceiptInfoField ::= SupplementaryInformation

SupplReceiptInfoField ::= PrintableString
	(SIZE (1..ub-supplementary-info-length))

-- Message store realization

ForwardedInfo ::= SET {
	      auto-forwarded-comment [0]
	           AutoForwardComment OPTIONAL,
	      cover-note [1]
	           IA5TextBodyPart OPTIONAL,
	      this-ipm-prefix [2]
	           PrintableString (SIZE(1..ub-ipm-identifier-suffix))
		        OPTIONAL
	      }

-- Have to have as mta.py not right 
ORName ::= [APPLICATION 0] SEQUENCE {
	standard-attributes StandardAttributes,
	domain-defined DomainDefinedAttributes OPTIONAL,
	extension-attributes ExtensionAttributes OPTIONAL,
	directory-name [0] ANY OPTIONAL }
--	directory-name [0] EXPLICIT ANY OPTIONAL }
--	Change
--	directory-name [0] EXPLICIT Name OPTIONAL }

END -- of IPMSInformationObjects
