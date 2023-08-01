# splat Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/splat/RCS/Makefile,v 6.0 1991/12/18 20:32:50 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:32:50  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  splat.c ps2pe.c
OBJS =  splat.o ps2pe.o
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

PROGS = xsplat

default:        $(PROGS)
all:            default
lint:           l-splat

install:        inst-dir inst-splat
inst-dir:       $(TOOLDIR)


splat: xsplat
xsplat: $(OBJS) $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

l-splat: $(SRCS)
		$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIBS)


inst-splat: $(TOOLDIR)/splat
$(TOOLDIR)/splat: xsplat
		-$(BACKUP) $@ zxsplat
		rm -f $@
		$(INSTALL) xsplat $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "splat tool installed normally"; echo ""


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
splat.o: splat.c
splat.o: ../../h/util.h
splat.o: ../../h/config.h
splat.o: ../../h/ll_log.h
splat.o: splat.c
splat.o: ../../h/util.h
splat.o: ../../h/config.h
splat.o: ../../h/ll_log.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
