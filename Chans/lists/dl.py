-- dl.py

-- @(#) $Header: /xtel/pp/pp-beta/Chans/lists/RCS/dl.py,v 6.0 1991/12/18 20:10:43 jpo Rel $
--
-- $Log: dl.py,v $
-- Revision 6.0  1991/12/18  20:10:43  jpo
-- Release 6.0
--
--

DL

DEFINITIONS ::=

PREFIXES encode decode print

BEGIN

DlPolicy [[P struct dl_policy *]]
        ::=
	SET
        {
	furtherExpansionPermitted [0] 
		BOOLEAN [[b dp_expand]]
		DEFAULT TRUE,
	conversionProhibited [1] 
		ENUMERATED [[i dp_convert]] 
		{
			original (1),
			false (2),
			true (3)
		} 
		DEFAULT original,
	priority [2]
                ENUMERATED [[i dp_priority]]
		{
			original (1),
			low (2),
			normal (3),
			high (4)
		} 
		DEFAULT low
	}		
END
