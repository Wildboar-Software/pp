-- or2pe: OR structure to P1 OR name


-- @(#) $Header: /xtel/pp/pp-beta/Lib/or/RCS/or2pe.py,v 6.0 1991/12/18 20:23:08 jpo Rel $
--
-- $Log: or2pe.py,v $
-- Revision 6.0  1991/12/18  20:23:08  jpo
-- Release 6.0
--
--
--


OR DEFINITIONS ::=

%{
#ifndef lint
static char *Rcsid = "@(#)$Header: /xtel/pp/pp-beta/Lib/or/RCS/or2pe.py,v 6.0 1991/12/18 20:23:08 jpo Rel $";
#endif  lint

#include "util.h"
#include "or.h"
#include <ctype.h>

# define PEPYPARM OR_ptr

# define or_present(or, type) (or_locate(or, type) != NULLOR)

PE      or2pe (or)
OR_ptr  or;
{
        PE      pe = NULLPE;
        PE tpe;

        if (build_OR_ORName (&pe, 1, 0, NULLCP, or) == NOTOK) {
                if (pe)
                        pe_free (pe);
                return NULLPE;
        }
                        /* SEK - strip off the APPLICATION TAG */
        tpe = pe -> pe_cons;
        pe -> pe_cons = NULLPE;
        pe_free (pe);
        return tpe;
}

#undef DEBUG
%}

BEGIN

ENCODER build
-- O/R name

ORName ::=
        [APPLICATION 0]
            IMPLICIT SEQUENCE {
                standard
                    StandardAttributeList [[p parm]],

                domaindefined
                    DomainDefinedAttributeList [[p parm]] 
                    OPTIONAL <<or_present (parm, OR_DD)>>
            }

StandardAttributeList 
        %{
                OR_ptr op;
        %} ::=
        SEQUENCE {
                CountryName
                [[p or_locate (parm, OR_C)]]
                        OPTIONAL
                        <<or_present (parm, OR_C) >>,

                AdministrationDomainName
                [[p or_locate (parm, OR_ADMD)]]
                        OPTIONAL
                        << or_present (parm, OR_ADMD) >>,

            [0]
                IMPLICIT X121Address
                [[s or_value (parm, OR_X121)]]
                        OPTIONAL
                        << or_present (parm, OR_X121) >>,

            [1]
                IMPLICIT TerminalID [[s or_value (parm, OR_TID) ]]
                OPTIONAL << or_present (parm, OR_TID) >>,

            [2]
                PrivateDomainName
                [[p or_locate (parm, OR_PRMD)]]
                OPTIONAL << or_present (parm, OR_PRMD) >>,

            [3]
                IMPLICIT OrganizationName [[s or_value (parm, OR_O)]]
                OPTIONAL <<or_present (parm, OR_O) >>,

            [4]
                IMPLICIT UniqueUAIdentifier
                [[p or_locate (parm, OR_UAID)]]
                OPTIONAL << or_present (parm, OR_UAID) >>,

            [5]
                IMPLICIT PersonalName [[p parm]]
                OPTIONAL <<or_present (parm, OR_S)>>,

            [6]
                IMPLICIT SEQUENCE OF
                        <<op = or_locate (parm, OR_OU);
                                op && op -> or_type == OR_OU;
                                        op = op -> or_next >>
                OrganizationalUnit [[s op -> or_value]]
                OPTIONAL <<or_present (parm, OR_OU) >>
        }

DomainDefinedAttributeList 
%{
        OR_ptr op;
%} ::=
        SEQUENCE OF << op = or_lastpart (parm);
                        op && op -> or_type == OR_DD;
                                op = op -> or_prev>>
                DomainDefinedAttribute [[p op]]

DomainDefinedAttribute ::=
        SEQUENCE {
            type
                PrintableString [[s parm -> or_ddname]],

            value
                PrintableString [[s parm -> or_value]]
        }

CountryName ::=
        [APPLICATION 1]
            CHOICE  << or_str_isns (parm -> or_value) ? 1 : 2 >> {
                NumericString [[s parm -> or_value ]],
                PrintableString [[s parm -> or_value ]]
            }

AdministrationDomainName ::=
        [APPLICATION 2]
            CHOICE << or_str_isns (parm -> or_value) ? 1 : 2 >> {
                NumericString [[s parm -> or_value]],
                PrintableString [[s parm -> or_value]]
            }

PrivateDomainIdentifier ::=
        CHOICE << or_str_isns (parm -> or_value) ? 1 : 2 >> {
            NumericString [[s parm -> or_value]],

            PrintableString [[s parm -> or_value]]
        }


X121Address ::=
        NumericString

TerminalID ::=
        PrintableString

OrganizationName ::=
        PrintableString

UniqueUAIdentifier ::=
        NumericString [[s parm -> or_value]]

PersonalName ::=
        SET {
            surName[0]
                IMPLICIT PrintableString [[s or_value (parm, OR_S)]],

            givenName[1]
                IMPLICIT PrintableString [[s or_value (parm, OR_G)]]
                OPTIONAL <<or_present (parm, OR_G)>>,

            initials[2]
                IMPLICIT PrintableString [[s or_value (parm, OR_I)]]
                OPTIONAL <<or_present (parm, OR_I)>>,

            generalQualifier[3]
                IMPLICIT PrintableString [[s or_value (parm, OR_GQ)]]
                OPTIONAL <<or_present (parm, OR_GQ)>>
    }

OrganizationalUnit ::=
        PrintableString

PrivateDomainName ::=
        CHOICE << or_str_isns (parm -> or_value) ? 1 : 2 >> {
            NumericString [[s parm -> or_value]],

            PrintableString [[s parm -> or_value]]
        }

END
