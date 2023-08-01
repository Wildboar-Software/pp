-- externalbp.py: Externally defined Body Parts

-- @(#) $Header: /xtel/pp/pp-beta/Format/asn/asn1/RCS/extdefbp.py,v 6.0 1991/12/18 20:16:32 jpo Rel $
--
-- $Log: extdefbp.py,v $
-- Revision 6.0  1991/12/18  20:16:32  jpo
-- Release 6.0
--
--
--

--
--
-- This is EXTENDED-BODY-PART-TYPE START 
--
--


ExtDef DEFINITIONS IMPLICIT TAGS ::=

%{
#ifndef	lint
static char *Rcsid = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/asn1/RCS/extdefbp.py,v 6.0 1991/12/18 20:16:32 jpo Rel $";
#endif	lint
%}

BEGIN

ExtDefBodyPart ::= SEQUENCE {
	parameters   [0] ExtDefParameters OPTIONAL,
	data		 ExtDefData
	}

ExtDefParameters ::= EXTERNAL

ExtDefData ::= EXTERNAL


END
