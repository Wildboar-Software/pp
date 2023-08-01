# bin2hex Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/bin2hex/RCS/Makefile,v 6.0 1991/12/18 20:28:39 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:28:39  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  bin2hex.c template.c
OBJS =  bin2hex.o template.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
LDFLAGS		= $(LDOPTIONS) $(LLF)
CFLAGS          =  $(CCOPTIONS) $(LCF) -DDEBUG -I$(HEADERS)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS = xbin2hex

default:        $(PROGS)
all:            default
lint:           l-bin2hex

install:        inst-dir inst-bin2hex
inst-dir:       $(TOOLDIR)


bin2hex: xbin2hex
xbin2hex: bin2hex.o $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ bin2hex.o $(LIBPP) $(LIBSYS)

l-bin2hex: bin2hex.c
		$(LINT) $(LINTFLAGS) bin2hex.c $(LINTLIBS)


inst-bin2hex: $(TOOLDIR)/bin2hex
$(TOOLDIR)/bin2hex: xbin2hex
		-$(BACKUP) $@ zxbin2hex
		rm -f $@
		$(INSTALL) xbin2hex $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "bin2hex tool inst-talled normally"; echo ""


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

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
