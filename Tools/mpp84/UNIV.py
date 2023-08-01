-- ASN.1 UNIVERSAL defined types

-- @(#) $Header: /xtel/pp/pp-beta/Tools/mpp84/RCS/UNIV.py,v 6.0 1991/12/18 20:31:34 jpo Rel $
--
-- $Log: UNIV.py,v $
-- Revision 6.0  1991/12/18  20:31:34  jpo
-- Release 6.0
--
--
--


UNIV DEFINITIONS ::=

%{
#ifndef	lint
static char *rcsid = "$Header: /xtel/pp/pp-beta/Tools/mpp84/RCS/UNIV.py,v 6.0 1991/12/18 20:31:34 jpo Rel $";
#endif	lint

#undef	DEBUG
#define	testdebug(pe,s)
%}

BEGIN


			-- ISO 646-1983
IA5String ::=
	[UNIVERSAL 22] IMPLICIT OCTETSTRING

ISO646String ::=
	IA5String

NumericString ::=
	[UNIVERSAL 18] IMPLICIT IA5String

PrintableString	::=
	[UNIVERSAL 19] IMPLICIT IA5String


			-- ISO 6937/2-1983
T61String ::=
	[UNIVERSAL 20] IMPLICIT OCTETSTRING

TeletexString ::=
	T61String

			-- ISO 6937/2-1983
VideotexString ::=
	[UNIVERSAL 21] IMPLICIT OCTETSTRING


			-- ISO 2014, 3307, 4031
			--     date, time, zone
GeneralizedTime	::=
	[UNIVERSAL 24] IMPLICIT VisibleString

GeneralisedTime	::=
	GeneralizedTime


UTCTime ::=
	[UNIVERSAL 23] IMPLICIT VisibleString

UniversalTime ::=
	UTCTime

			-- ISO 2375
GraphicString ::=
	[UNIVERSAL 25] IMPLICIT OCTETSTRING

VisibleString ::=
	[UNIVERSAL 26] IMPLICIT OCTETSTRING

GeneralString ::=
	[UNIVERSAL 27] IMPLICIT OCTETSTRING


			-- ISO 8824
EXTERNAL ::=
	[UNIVERSAL 8]
	    IMPLICIT SEQUENCE {
		data-value-identifier CHOICE {
			-- only for prior argreement of transfer syntax
		    direct-reference OBJECT IDENTIFIER,

			-- only for presentation layer negotiation
		    indirect-reference INTEGER
		},

		data-value-descriptor ObjectDescriptor OPTIONAL,

		encodings CHOICE {
		    single-encoding Encoding-choice,

		    multiple-encoding SEQUENCE OF SEQUENCE {
			encoding-identifier OBJECT IDENTIFIER,

			encoding Encoding-choice
		    }
		}
	    }

Encoding-choice	::=
	CHOICE {
	    single-ASN1-type[0]
		ANY,

	    octet-aligned[1]
		IMPLICIT OCTETSTRING,

	    arbitrary[2]
		IMPLICIT BITSTRING
	}


			-- ISO 8824
ObjectDescriptor ::=
	[UNIVERSAL 7] IMPLICIT GraphicString

END
