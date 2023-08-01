# hex2bin Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/hex2bin/RCS/Makefile,v 6.0 1991/12/18 20:30:25 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:30:25  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  hex2bin.c hex2bin.c
OBJS =  hex2bin.o
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

PROGS = xhex2bin

default:        $(PROGS)
all:            default
lint:           l-hex2bin

install:        inst-dir inst-hex2bin
inst-dir:       $(TOOLDIR)


hex2bin: xhex2bin
xhex2bin: hex2bin.o $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ hex2bin.o $(LIBPP) $(LIBSYS)

l-hex2bin: hex2bin.c
		$(LINT) $(LINTFLAGS) hex2bin.c $(LINTLIBS)


inst-hex2bin: $(TOOLDIR)/hex2bin
$(TOOLDIR)/hex2bin: xhex2bin
		-$(BACKUP) $@ zxhex2bin
		rm -f $@
		$(INSTALL) xhex2bin $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "hex2bin tool inst-talled normally"; echo ""


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
hex2bin.o: hex2bin.c
hex2bin.o: hex2bin.c

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
