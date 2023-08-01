# Channel master makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/RCS/Makefile,v 6.0 1991/12/18 20:03:15 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:03:15  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



BASIC   = dr2rfc filtercontrol msg-clean qmgr-load timeout trashman \
	splitter warnings
SUBDIRS = $(BASIC) $(OPTIONALCHANS)
ALLDIRS	= $(BASIC) 822-local grey lists shell slocal \
	smtp uucp x40084 x40088 fax decnet testchan

############################################################
#
# Building Rules
#
############################################################

target:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE)); done

install:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done


tidy define depend clean lint:
	@for i in ${ALLDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

############################################################
#
# End of Building Rules
#
############################################################
