-- pe2or: P1 O/R to OR_ptr conversion routines


-- @(#) $Header: /xtel/pp/pp-beta/Lib/or/RCS/pe2or.py,v 6.0 1991/12/18 20:23:08 jpo Rel $
--
-- $Log: pe2or.py,v $
-- Revision 6.0  1991/12/18  20:23:08  jpo
-- Release 6.0
--
--
--


-- SEK add application 1 tag back into PE

OR DEFINITIONS ::=

%{
#ifndef lint
static char Rcsid[] = "@(#)$Header";
#endif  lint

#include "util.h"
#include "or.h"

extern char or_error[];
static int or_concat ();

#define PEPYPARM        OR_ptr *

OR_ptr  pe2or (pe)
PE      pe;
{
        OR_ptr  or = NULLOR;
	PE  tpe;

		/* SEK put in an extra tage */
	tpe = pe_alloc (PE_CLASS_APPL, PE_FORM_CONS, 0);
	tpe -> pe_cons = pe;
        if (decode_OR_ORName (tpe, 1, NULLIP, NULLVP, &or) == NOTOK) {
                if (or)
                        or_free (or);
		or = NULLOR;
        }
	tpe -> pe_cons = NULLPE;
	pe_free (tpe);
        return or;
}

#undef DEBUG
%}

BEGIN

DECODER decode

-- O/R name

ORName ::=
        [APPLICATION 0]
            IMPLICIT SEQUENCE {
                standard
                    StandardAttributeList [[p parm]],

                domaindefined
                    DomainDefinedAttributeList [[p parm]] OPTIONAL
            }

StandardAttributeList 
        %{
                char    *str;
        %} ::=
        SEQUENCE {
                CountryName [[p parm]] OPTIONAL,

                AdministrationDomainName [[p parm]] OPTIONAL,

            [0]
                IMPLICIT X121Address [[s str]]
                %{ if (or_concat (parm, OR_X121, NULLCP, str) == NOTOK)
			return NOTOK;
		%}
                OPTIONAL,

            [1]
                IMPLICIT TerminalID [[s str]]
                %{ if (or_concat (parm, OR_TID, NULLCP, str) == NOTOK)
			return NOTOK;
		%}
                OPTIONAL,

            [2]
                PrivateDomainName [[p parm]] OPTIONAL,

            [3]
                IMPLICIT OrganizationName [[s str]]
                %{ if (or_concat (parm, OR_O, NULLCP, str) == NOTOK)
			return NOTOK;
		%}
                OPTIONAL,

            [4]
                IMPLICIT UniqueUAIdentifier [[s str]]
                %{ if (or_concat (parm, OR_UAID, NULLCP, str) == NOTOK)
			return NOTOK;
		%}
                OPTIONAL,

            [5]
                IMPLICIT PersonalName [[p parm]] OPTIONAL,

            [6]
                IMPLICIT SEQUENCE OF OrganizationalUnit [[s str]]
                %{
                        if (or_concat (parm, OR_OU, NULLCP, str) == NOTOK)
				return NOTOK;
                %}
                OPTIONAL
        }


DomainDefinedAttributeList ::=
        SEQUENCE OF DomainDefinedAttribute [[p parm]]


DomainDefinedAttribute
        %{
                char    *dd_type, *dd_val;
        %}
        ::=
        SEQUENCE {
            type
                PrintableString [[s dd_type]],

            value
                PrintableString [[s dd_val]]
        }
        %{
                if (or_concat (parm, OR_DD, dd_type, dd_val) == NOTOK)
			return NOTOK;
        %}


CountryName
        %{
                char    *country;
        %} ::=
        [APPLICATION 1]
            CHOICE {
                NumericString [[s country]],

                PrintableString [[s country]]
            }
        %{
                if (or_concat (parm, OR_C, NULLCP, country) == NOTOK)
			return NOTOK;
        %}


AdministrationDomainName
        %{
                char    *admd;
        %} ::=
        [APPLICATION 2]
            CHOICE {
                NumericString [[s admd]],
                PrintableString [[s admd]]
            }
        %{
                if (or_concat (parm, OR_ADMD, NULLCP, admd) == NOTOK)
			return NOTOK;
        %}


PrivateDomainIdentifier
        %{
                char    *prmd;
        %} ::=
        CHOICE {
            NumericString [[s prmd]],

            PrintableString [[s prmd]]
        }
        %{
                if (or_concat (parm, OR_PRMD, NULLCP, prmd) == NOTOK)
			return NOTOK;
        %}

X121Address ::=
        NumericString

TerminalID ::=
        PrintableString

OrganizationName ::=
        PrintableString

UniqueUAIdentifier ::=
        NumericString

PersonalName
        %{
                char    *str;
        %} ::=
        SET {
            surName[0]
                IMPLICIT PrintableString [[s str]]
                %{
                        if (or_concat (parm, OR_S, NULLCP, str) == NOTOK)
				return NOTOK;
                %},

            givenName[1]
                IMPLICIT PrintableString [[s str]]
                %{
                        if (or_concat (parm, OR_G, NULLCP, str) == NOTOK)
				return NOTOK;
                %}
                OPTIONAL,

            initials[2]
                IMPLICIT PrintableString [[s str]]
                %{
                        if (or_concat (parm, OR_I, NULLCP, str) == NOTOK)
				return NOTOK;
                %}
                OPTIONAL,

            generalQualifier[3]
                IMPLICIT PrintableString [[s str]]
                %{
                        if (or_concat (parm, OR_GQ, NULLCP, str) == NOTOK)
				return NOTOK;
                %}
                OPTIONAL
    }


OrganizationalUnit ::=
        PrintableString

PrivateDomainName
        %{
                char *prmd;
        %} ::=
        CHOICE {
            NumericString [[s prmd]],

            PrintableString [[s prmd]]
        }
        %{ if (or_concat (parm, OR_PRMD, NULLCP, prmd) == NOTOK)
		return NOTOK;
	%}




END

%{



static int    or_concat (orp, type, str1, str2)
OR_ptr  *orp;
int     type;
char    *str1, *str2;
{
        OR_ptr  or1, 
		or2;

        if (*orp == NULLOR) {
                *orp = or_new (type, str1, str2);
		if (*orp == NULLOR)
			return NOTOK;
	}
        else {
                or1 = or_new (type, str1, str2);
		if (or1 == NULLOR) {
			(void) strcpy (PY_pepy, or_error);
			return NOTOK;
		}

		if (type == OR_OU)
			or2 = or_add (*orp, or1, FALSE);
		else 
			or2 = or_add (*orp, or1, TRUE);

		if (or2 != NULLOR)
			*orp = or2;
		else {
			PP_LOG (LLOG_EXCEPTIONS, ("or_add failed on %s %s",
				str1 ? str1 : "", str2 ? str2 : "" ));
			return NOTOK;
		}
        }
	return OK;
}

%}
