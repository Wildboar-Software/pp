# ean2real Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ean2real/RCS/Makefile,v 6.0 1991/12/18 20:30:18 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:30:18  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ean2real.c 
OBJS =  ean2real.o 
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a

LDFLAGS		= $(LDOPTIONS) $(LLF)
CFLAGS          =  $(CCOPTIONS) $(LCF) -I$(HEADERS)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS = xean2real

default:        $(PROGS)
all:            default
lint:           l-ean2real

install:        inst-dir inst-ean2real
inst-dir:       $(TOOLDIR)


ean2real: xean2real
xean2real: $(OBJS) $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

l-ean2real: $(SRCS)
		$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIBS)


inst-ean2real: $(TOOLDIR)/ean2real
$(TOOLDIR)/ean2real: xean2real
		-$(BACKUP) $@ zxean2real
		rm -f $@
		$(INSTALL) xean2real $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "ean2real tool installed normally"; echo ""


clean:  tidy
		rm -f $(OBJS)
tidy:
		rm -f core.* $(PROGS) zx* *.old *.BAK

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
ean2real.o: ean2real.c
ean2real.o: ../../h/util.h
ean2real.o: ../../h/config.h
ean2real.o: ../../h/ll_log.h
ean2real.o: ean2real.c
ean2real.o: ../../h/util.h
ean2real.o: ../../h/config.h
ean2real.o: ../../h/ll_log.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
