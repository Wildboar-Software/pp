# Preliminary comments
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/RCS/Makefile,v 6.0 1991/12/18 20:15:10 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:15:10  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS = p2explode p2flatten rfc1148 rfc822norm p2norm rfc934 $(OPTIONALFILTERS)

ALLDIRS = $(SUBDIRS) ascii2fax asn charset

############################################################
#
# Building Rules
#
############################################################

target:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE)); done

install define lint:
	@for i in ${SUBDIRS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

clean tidy depend:
	@for i in $(ALLDIRS); \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

############################################################
#
# End of Building Rules
#
############################################################
