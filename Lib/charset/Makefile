# Character Set specific routines
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/charset/RCS/Makefile,v 6.0 1991/12/18 20:21:39 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:21:39  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  charset_conf.c gc.c strcnv.c
OBJS =  charset_conf.o gc.o strcnv.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


LIBSRCS		= charset_conf.c strcnv.c
LIBOBJS		= charset_conf.o strcnv.o
HEADERS         = ../../h

CFLAGS          = $(LIBCCOPTIONS) -I$(HEADERS) $(LCF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = charset-made 

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
		  ../parse/llib-lparse.ln \
		  ../pp/llib-lpp.ln \
		  ../qmgr/llib-lqmgr.ln \
		  ../table/llib-ltable.ln \
		  ../tai/llib-ltai.ln \
		  ../util/llib-lutil.ln \
		  ../x400/llib-lx400.ln



############################################################
#
# Building Rules
#
############################################################



default: ${SHADOW} xgc

${SHADOW}: $(LIBOBJS)
	$(AR) r$(ARFLAGS) ../libpp.a $?
	touch $@




charset_conf.o: charset_conf.c ../../Make.defs Make.defs
	$(CC) $(CFLAGS) -c \
		'-DFILE_CHARSETS="$(FILE_CHARSETS)"' \
		'-DFILE_CHARDEFS="$(FILE_CHARDEFS)"' \
		'-DFILE_CHARMAP="$(FILE_CHARMAP)"' \
		'-DFILE_CHARMNEM="$(FILE_CHARMNEM)"' \
		'-DCSDIR="$(CSDIR)"' \
		charset_conf.c

gc:	xgc
xgc:	charset_conf.o gc.o
	$(CC) $(CCFLAGS) charset_conf.o gc.o -o $@


lintlib:llib-lcs.ln
llib-lcs.ln: ${LIBSRCS}
	$(LINT) -Ccs $(LLFLAGS) ${LIBSRCS}

lint: $(LIBSRCS)
	$(LINT) $(LLFLAGS) $(LIBSRCS) $(LINTLIBS) $(LINTISODE)

install: $(CSDIR) $(CSDIR)/$(FILE_CHARSETS) \
		$(CSDIR)/$(FILE_CHARDEFS) inst-gc $(CSDIR)/LATIN1
	@echo "All installed"

inst-gc: $(CSDIR)/gc
$(CSDIR)/gc: xgc
	-$(BACKUP) $@ zxgc
	rm -f $@
	$(INSTALL) xgc $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "gc program installed normally"; echo ""

$(CSDIR)/LATIN1: $(CSDIR)/$(FILE_CHARSETS)
	(cd $(CSDIR); ./gc)
	(cd  $(CSDIR); $(CHOWN) $(PPUSER) * )

$(CSDIR):
	mkdir $(CSDIR)
	$(CHOWN) $(PPUSER) $(CSDIR)

$(CSDIR)/$(FILE_CHARSETS): $(MASTER_CHARSETS)
	cat $(MASTER_CHARSETS) > $(CSDIR)/$(FILE_CHARSETS)

$(CSDIR)/$(FILE_CHARDEFS): $(MASTER_CHARDEFS)
	cat $(MASTER_CHARDEFS) > $(CSDIR)/$(FILE_CHARDEFS)

clean: tidy
	rm -f $(OBJS) llib-lcs.ln 

tidy:
	rm -f core ${SHADOW} charset-table-made xgc *.old a.out *.BAK

depend:
	$(DEPEND) $(SRCS)

define:
	$(DEFINE) Makefile



############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
charset_conf.o: charset_conf.c
gc.o: gc.c
gc.o: ../../h/charset.h
strcnv.o: strcnv.c
strcnv.o: ../../h/charset.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
