# tables programs master makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/tables/RCS/Makefile,v 6.0 1991/12/18 20:35:38 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:35:38  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS = abbrev tjoin
ALLDIRS	= $(SUBDIRS) domain compat

############################################################
#
# Building Rules
#
############################################################

target:
	for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE)); done

install:
	for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

tidy define depend clean lint:
	for i in ${ALLDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

############################################################
#
# End of Building Rules
#
############################################################
