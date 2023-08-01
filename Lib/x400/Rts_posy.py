-- Rts_posy.py:


-- @(#) $Header: /xtel/pp/pp-beta/Lib/x400/RCS/Rts_posy.py,v 6.0 1991/12/18 20:25:37 jpo Rel $
--
-- $Log: Rts_posy.py,v $
-- Revision 6.0  1991/12/18  20:25:37  jpo
-- Release 6.0
--
--
--


RTS84 DEFINITIONS ::=

BEGIN

Request ::=
    CHOICE {
        NULL,

        [1] IMPLICIT SET {
                mTAName[0]
                        IMPLICIT IA5String,
                password[1]
                        ANY
        }
    }

END
