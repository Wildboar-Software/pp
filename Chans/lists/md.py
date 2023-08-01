-- md.py

-- @(#) $Header: /xtel/pp/pp-beta/Chans/lists/RCS/md.py,v 6.0 1991/12/18 20:10:43 jpo Rel $
--
-- $Log: md.py,v $
-- Revision 6.0  1991/12/18  20:10:43  jpo
-- Release 6.0
--
--

MD
	{
	joint-iso-ccitt
	mhs-motis(6)
	arch (5)
	modules(0)
	directory(1)
	}

DEFINITIONS IMPLICIT TAGS ::=

%{
static orname_decode (pparm, pe)
struct dl_permit **pparm;
PE pe;
{
	PElementClass class;
	PElementID id;
	int result = OK;

	class = pe -> pe_class;
	pe -> pe_class = PE_CLASS_APPL;
	id = pe -> pe_id;
	pe -> pe_id = 0;
	if (((*pparm)->dp_or = pe2orn(pe)) == NULL)
	    result = NOTOK;
	pe -> pe_id = id;
	pe -> pe_class = class;
	return result;
}

static orname_encode (parm, ppe)
struct dl_permit *parm;
PE *ppe;
{
	if ((*ppe = orn2pe (parm->dp_or)) == NULL)
	    return NOTOK;
	return OK;
}

%}

PREFIXES encode decode print

BEGIN

IMPORTS
	Name
		FROM IF
			{
			joint-iso-ccitt
			ds(5)
			modules(1)
			informationFramework(1)
			}
	ORName
		FROM IOB
			{
			joint-iso-ccitt
			mts (3)
			modules(0)
			mts-abstract-service(1)
			};

ORNamePattern [[P ORName *]] ::= ORName [[p *]]

DLSubmitPermission [[P struct dl_permit *]]
        ::=
	CHOICE <<dp_type>>
        {
	individual [0]
		--ORName [[p dp_or]],
		*ANY [[a dp_or]] [[D orname_decode]] [[E orname_encode]],
	member-of-dl [1]
		-- ORName [[p dp_or]],
		*ANY [[a dp_or]] [[D orname_decode]] [[E orname_encode]],
	pattern-match [2]
		--ORNamePattern [[p dp_or]],
		*ANY [[a dp_or]] [[D orname_decode]] [[E orname_encode]],
	member-of-group [3]
		Name [[p dp_dn]]
	}		
END
