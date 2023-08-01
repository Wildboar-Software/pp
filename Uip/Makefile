# User interface programs
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Uip/RCS/Makefile,v 6.0 1991/12/18 20:42:20 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:42:20  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS = mail rcvalert rcvtrip sendmail resend misc

############################################################
#
# Building Rules
#
############################################################

target:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE)); done

lint tidy install define depend clean:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

############################################################
#
# End of Building Rules
#
############################################################
