# Channel master makefile
#
############################################################
#
# @(#) $Header$
#
# $Log$
#
############################################################
#
# Definitions
#
############################################################

SUBDIRS	= master LOCALSMTP INTERNET JANET

############################################################
#
# Building Rules
#
############################################################

target:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE)); done

tidy define depend clean lint:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

install:;

############################################################
#
# End of Building Rules
#
############################################################
