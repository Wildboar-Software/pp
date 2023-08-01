#  ntail - log watcher...
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ntail/RCS/Makefile,v 6.0 1991/12/18 20:32:10 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:32:10  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  entryfuncs.c miscfuncs.c ntail.c
OBJS =  entryfuncs.o miscfuncs.o ntail.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS		= $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS = xntail

default: $(PROGS)

ntail: xntail
	@true

xntail: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)


install: inst-dir inst-ntail
inst-ntail: $(TOOLDIR)/ntail
$(TOOLDIR)/ntail: xntail
	-$(BACKUP) $@ zxntail
	rm -f $@
	$(INSTALL) xntail $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "ntail installed correctly"; echo ""
inst-dir: $(TOOLDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core* $(PROGS) zxntail xntail *.old *.BAK

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile

############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
entryfuncs.o: entryfuncs.c
entryfuncs.o: ./ntail.h
miscfuncs.o: miscfuncs.c
miscfuncs.o: ./ntail.h
ntail.o: ntail.c
ntail.o: ./ntail.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
