# Manual page installation
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/man/RCS/Makefile,v 6.0 1991/12/18 20:43:36 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:43:36  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS	= man1 man3 man8


############################################################
#
# Building Rules
#
############################################################

default: 
	@for i in $(SUBDIRS); \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE) );\
	done

install clean tidy:
	@for i in $(SUBDIRS); \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@);\
	done
lint:;

depend:;

############################################################
#
# End of Building Rules
#
############################################################
