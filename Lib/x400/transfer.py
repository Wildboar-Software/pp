-- transfer.py:


-- @(#) $Header: /xtel/pp/pp-beta/Lib/x400/RCS/transfer.py,v 6.0 1991/12/18 20:25:37 jpo Rel $
--
-- $Log: transfer.py,v $
-- Revision 6.0  1991/12/18  20:25:37  jpo
-- Release 6.0
--
--
--


Trans
-- {iso identified-organisation(3) locator(999) modules(0) transfer(8)}
DEFINITIONS IMPLICIT TAGS ::=
BEGIN

IMPORTS 
	MessageTransferEnvelope, Content,
	ReportTransferEnvelope, ReportTransferContent,
	ProbeTransferEnvelope
-- Extras
	,
	MTABindArgument, MTABindResult, MTABindError
-- End Extras
	FROM MTA;

MtsAPDU ::=
	CHOICE {
		message [0] MessageAPDU,
		report  [1] ReportAPDU,
		probe   [2] ProbeTransferEnvelope}

MessageAPDU ::=
	SEQUENCE {
		envelope MessageTransferEnvelope,
		content  Content}

ReportAPDU ::=
	SEQUENCE {
		envelope ReportTransferEnvelope,
		content  ReportTransferContent}

ProbeAPDU ::=
	ProbeTransferEnvelope

--- additions for 1988 BIND macro

Bind1988Argument ::= [16] EXPLICIT MTABindArgument

Bind1988Result ::= [17] EXPLICIT MTABindResult

Bind1988Error ::= [18] EXPLICIT MTABindError

END
