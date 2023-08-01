# Library building Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/RCS/Makefile,v 6.0 1991/12/18 20:26:14 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:26:14  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS = qmgr  x400 addr format io or parse pp table tai charset util
SHADOW  = ranlib-made
LIB     = libpp.a
LINTLIB = llib-lpp.ln
CFLAGS  = $(LIBCCOPTIONS) $(LCF)

default: bits-made ${SHADOW}

install:
	@for x in ${SUBDIRS}; \
	do (echo "cd $$x; $(MAKE) $@";cd $$x; $(MAKE) $@); done

bits-made:
	@for x in ${SUBDIRS} ; \
	do (echo "cd $$x; $(MAKE)"; cd $$x ; $(MAKE) ) ; done

${SHADOW}: $(LIB)
	@$(MAKE) ppversion.o
	$(AR) r$(ARFLAGS) $(LIB) ppversion.o
	$(RANLIB) ${LIB}
	touch $@

ppversion.c: $(LIB) version.major version.minor
	@./version.sh pp > $@

lintlib: lint-bits concat-lint

lint-bits:
	@for x in ${SUBDIRS}; \
	do (echo "cd $$x; $(MAKE) lintlib"; cd $$x ; $(MAKE) lintlib); done

concat-lint:
	rm -f ${LINTLIB}
	@for x in ${SUBDIRS}; \
	do cat $$x/*.ln >> ${LINTLIB}; done

clean:
	@for x in ${SUBDIRS} ; \
	do (echo "cd $$x; $(MAKE) $@"; cd $$x ; $(MAKE) $@ ); done
	rm -f core *.old *.BAK ${SHADOW} *.o ${LIB} $(LINTLIB)

tidy:
	@for x in ${SUBDIRS} ; \
	do (echo "cd $$x; $(MAKE) $@"; cd $$x ; $(MAKE) $@ ); done
	rm -f core *.old *.BAK ${SHADOW} *.o

depend lint:
	@for x in ${SUBDIRS} ; \
	do (echo "cd $$x; $(MAKE)"; cd $$x ; $(MAKE) $@ ); done
