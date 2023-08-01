# Tools building Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/RCS/Makefile,v 6.0 1991/12/18 20:35:46 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:35:46  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS = bin2hex ckadr ckchan ckconfig ckmail  dbmbuild \
	dbmedit dbmunbuild hex2bin mkpasswd mlist mpp84 mpp88 \
	nsapexpand ntail probe splat tables statgen
ALLDIRS = $(SUBDIRS) bin2ascii ckor misc mu ean2real ppls ppbm

default: make-bits

make-bits:
	@for x in ${SUBDIRS} ; \
	do (echo "cd $$x ; $(MAKE)"; cd $$x ; $(MAKE)); done

clean tidy:
	@for x in ${ALLDIRS} ; \
	do (echo "cd $$x ; $(MAKE) $@"; cd $$x ; $(MAKE) $@ ); done
	rm -f core *.old *.BAK

depend lint install:
	@for x in ${SUBDIRS} ; \
	do (echo "cd $$x ; $(MAKE) $@"; cd $$x ; $(MAKE) $@ ); done
