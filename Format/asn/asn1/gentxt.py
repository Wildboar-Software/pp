-- gentxt.py:

-- @(#) $Header: /xtel/pp/pp-beta/Format/asn/asn1/RCS/gentxt.py,v 6.0 1991/12/18 20:16:32 jpo Rel $
--
-- $Log: gentxt.py,v $
-- Revision 6.0  1991/12/18  20:16:32  jpo
-- Release 6.0
--
--
--

--
--
-- This is an EXTENDED-BODY-PART-TYPE FOR GENERAL TEXT
--
--


GenTxt DEFINITIONS IMPLICIT TAGS ::=

%{
#ifndef	lint
static char *Rcsid = "@(#)$Header: /xtel/pp/pp-beta/Format/asn/asn1/RCS/gentxt.py,v 6.0 1991/12/18 20:16:32 jpo Rel $";
#endif	lint
%}

BEGIN

GenTxtBodyPart ::= SEQUENCE {
		parameters	GenTxtParameters,
		data		GenTxtData
		}

GenTxtParameters ::= SET {
		g0-designator [0] CharacterSetDesignator OPTIONAL, 
		g1-designator [1] CharacterSetDesignator OPTIONAL, 
		g2-designator [2] CharacterSetDesignator OPTIONAL, 
		g3-designator [3] CharacterSetDesignator OPTIONAL, 
		c0-designator [4] CharacterSetDesignator OPTIONAL, 
		c1-designator [5] CharacterSetDesignator OPTIONAL
		}

CharacterSetDesignator ::= GeneralString (SIZE (3..5))

GenTxtData ::= GeneralString
END
