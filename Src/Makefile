# Makefile governing the 'core' pp programs
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Src/RCS/Makefile,v 6.0 1991/12/18 20:28:20 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:28:20  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS = submit qmgr pptsapd MTAconsole LINEconsole lconsole

target:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE)); done

all define depend clean tidy install lint:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done
